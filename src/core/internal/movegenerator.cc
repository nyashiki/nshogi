//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "movegenerator.h"
#include "bitboard.h"
#include <array>

namespace nshogi {
namespace core {
namespace internal {

namespace {

using namespace internal::bitboard;

// Upper bound of the bishops and rooks one side can have on the board:
// the standard piece set contains two bishops and two rooks in total.
constexpr std::size_t SliderAttackCacheSize = 4;

#if defined(USE_AVX2) || defined(USE_NEON)

inline constexpr unsigned getDroppingMoveMask(Stands St) noexcept {
    // Each count field has a zero guard bit immediately above it.
    // Adding all ones to each non-pawn field carries into that bit iff
    // the count is nonzero. No carry propagates past the guard bit.
    constexpr uint32_t CountMask = 0x06ddddc0U;
    constexpr uint32_t GuardMask = 0x09222200U;
    const uint32_t Present = (St + CountMask) & GuardMask;
    // Pack silver, gold, bishop, rook, lance, knight into bits 0..5.
    return ((Present >> 17) & 1U) | ((Present >> 20) & 2U) |
           ((Present >> 22) & 4U) | ((Present >> 24) & 8U) |
           ((Present >> 5) & 16U) | ((Present >> 8) & 32U);
}

// The six non-pawn hand kinds have only 64 possible presence masks.
struct DroppingMovePack {
    alignas(32) std::array<uint32_t, 8> Moves{};
    unsigned Count = 0;
};

constexpr auto DroppingMovePacks = [] {
    constexpr PieceTypeKind Types[] = {
        PTK_Silver, PTK_Gold, PTK_Bishop, PTK_Rook, PTK_Lance, PTK_Knight};
    std::array<DroppingMovePack, 64> Packs{};
    for (unsigned Mask = 0; Mask < Packs.size(); ++Mask) {
        for (unsigned I = 0; I < 6; ++I) {
            if ((Mask & (1U << I)) != 0) {
                Packs[Mask].Moves[Packs[Mask].Count++] =
                    Move32::droppingMove(static_cast<Square>(0), Types[I])
                        .value();
            }
        }
    }
    return Packs;
}();

#endif

// Sliding attacks from the opposing king, shared by all check-move phases.
struct CheckAttackCache {
    Bitboard Bishop;
    Bitboard Rook;
    Bitboard Lance;
    Bitboard King;

    template <PieceTypeKind Type>
    Bitboard getBishop() const noexcept {
        if constexpr (Type == PTK_ProBishop) {
            return Bishop | King;
        }
        return Bishop;
    }

    template <PieceTypeKind Type>
    Bitboard getRook() const noexcept {
        if constexpr (Type == PTK_ProRook) {
            return Rook | King;
        }
        return Rook;
    }
};

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote>
inline Move32* emitBoardMoves(const StateImpl& S, Move32* __restrict Moves,
                              Square From, const Bitboard& ToBB) noexcept {
    const PieceTypeKind ActualType =
        Type == PTK_Gold ? getPieceType(S.getPosition().pieceOn(From)) : Type;
    const auto Append = [&](Square To, bool Promote) {
        const PieceTypeKind Captured =
            Capture ? getPieceType(S.getPosition().pieceOn(To)) : PTK_Empty;
        *Moves++ =
            Promote ? Move32::boardPromotingMove(From, To, ActualType, Captured)
                    : Move32::boardMove(From, To, ActualType, Captured);
    };
    if constexpr (Type == PTK_Lance) {
        ToBB.forEach([&](Square To) {
            if (PromotableBB[C].isSet(To)) {
                Append(To, true);
            }

            if constexpr (WilyPromote) {
                if (!FirstAndSecondFurthestBB[C].isSet(To)) {
                    Append(To, false);
                }
            } else {
                if (!Bitboard::FurthermostBB<C>().isSet(To)) {
                    Append(To, false);
                }
            }
        });
    } else if constexpr (Type == PTK_Bishop || Type == PTK_Rook) {
        const Bitboard PromoteTo = PromotableBB[C].isSet(From)
            ? ToBB : (ToBB & PromotableBB[C]);

        PromoteTo.forEach([&](Square To) {
            Append(To, true);

            if constexpr (!WilyPromote) {
                Append(To, false);
            }
        });
        PromoteTo.andNot(ToBB).forEach([&](Square To) { Append(To, false); });
    } else {
        const Bitboard NormalTo = Type == PTK_Knight
            ? FirstAndSecondFurthestBB[C].andNot(ToBB) : ToBB;

        NormalTo.forEach([&](Square To) {
            Append(To, false);
        });

        if constexpr (Type == PTK_Knight || Type == PTK_Silver) {
            const Bitboard PromoteTo = PromotableBB[C].isSet(From)
                ? ToBB : (ToBB & PromotableBB[C]);

            PromoteTo.forEach([&](Square To) {
                Append(To, true);
            });
        }
    }
    return Moves;
}

template <Color C, bool Capture, bool WilyPromote>
inline Move32*
generateOnBoardOneStepPawnMovesImpl(const StateImpl& S,
                                    Move32* __restrict Moves,
                                    const Bitboard& TargetSquares) noexcept {
    Bitboard ToBB = S.getBitboard<C, PTK_Pawn>();
    const Bitboard PinnedPawns = ToBB & S.getDefendingOpponentSliderBB<C>();
    if (!PinnedPawns.isZero()) {
        // A pinned pawn can move only along the file of its king.
        ToBB = PinnedPawns.andNot(ToBB) |
               (PinnedPawns & FileBB[squareToFile(S.getKingSquare<C>())]);
    }

    if constexpr (C == Black) {
        ToBB = ToBB.getLeftShiftEpi64<1>();
    } else {
        ToBB = ToBB.getRightShiftEpi64<1>();
    }

    ToBB &= TargetSquares;

    (ToBB & PromotableBB[C]).forEach([&](Square To) {
        const Square From = (C == Black) ? (To + South) : (To + North);

        if constexpr (Capture) {
            const PieceTypeKind CaptureType =
                getPieceType(S.getPosition().pieceOn(To));
            *Moves++ =
                Move32::boardPromotingMove(From, To, PTK_Pawn, CaptureType);
        } else {
            *Moves++ = Move32::boardPromotingMove(From, To, PTK_Pawn);
        }

        if constexpr (!WilyPromote) {
            if (!Bitboard::FurthermostBB<C>().isSet(To)) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardMove(From, To, PTK_Pawn, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, PTK_Pawn);
                }
            }
        }
    });

    PromotableBB[C].andNot(ToBB).forEach([&](Square To) {
        if constexpr (C == Black) {
            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ =
                    Move32::boardMove(To + South, To, PTK_Pawn, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(To + South, To, PTK_Pawn);
            }
        } else {
            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ =
                    Move32::boardMove(To + North, To, PTK_Pawn, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(To + North, To, PTK_Pawn);
            }
        }
    });

    return Moves;
}

template <Color C, bool Capture>
inline Move32* generateOnBoardOneStepGoldKindsMovesImpl(
    const StateImpl& S, Move32* __restrict Moves,
    const Bitboard& TargetSquares) noexcept {
    const Bitboard FromBB =
        (S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>() |
         S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>() |
         S.getBitboard<PTK_ProSilver>()) & S.getBitboard<C>();
    const auto Emit = [&](Square From, bool Pinned) {
        Bitboard ToBB = TargetSquares & getAttackBB<C, PTK_Gold>(From);
        if (Pinned) {
            ToBB &= LineBB[From][S.getKingSquare<C>()];
        }
        Moves = emitBoardMoves<C, PTK_Gold, Capture, true>(S, Moves, From, ToBB);
    };
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) { Emit(From, true); });
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) { Emit(From, false); });
    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture>
inline Move32*
generateOnBoardOneStepMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                                const Bitboard& TargetSquares) noexcept {
    if constexpr (Type == PTK_King) {
        const Square From = S.getKingSquare<C>();
        if (From == SqInvalid) {
            return Moves;
        }
        const Bitboard ToBB = TargetSquares & getAttackBB<C, PTK_King>(From);
        const Bitboard OccupiedBB = SquareBB[From].andNot(
            S.getBitboard<Black>() | S.getBitboard<White>());
        ToBB.forEach([&](Square To) {
            if (S.isAttackedByOneStep<C>(To) ||
                S.isAttackedBySlider<C>(To, OccupiedBB)) {
                return;
            }
            if constexpr (Capture) {
                *Moves++ = Move32::boardMove(
                    From, To, PTK_King,
                    getPieceType(S.getPosition().pieceOn(To)));
            } else {
                *Moves++ = Move32::boardMove(From, To, PTK_King);
            }
        });
        return Moves;
    }

    static_assert(Type == PTK_King || Type == PTK_Knight || Type == PTK_Silver);
    const Bitboard FromBB = S.getBitboard<C, Type>();
    const auto Emit = [&](Square From, bool Pinned) {
        Bitboard ToBB = TargetSquares & getAttackBB<C, Type>(From);
        if (Pinned) {
            ToBB &= LineBB[From][S.getKingSquare<C>()];
        }
        Moves = emitBoardMoves<C, Type, Capture, true>(S, Moves, From, ToBB);
    };
    if constexpr (Type != PTK_Knight) {
        (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach(
            [&](Square From) { Emit(From, true); });
    }
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) { Emit(From, false); });
    return Moves;
}

template <Color C, bool Capture, bool WilyPromote>
inline Move32*
generateOnBoardLanceMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                              const Bitboard& TargetSquares,
                              const Bitboard& OccupiedBB) noexcept {
    const Bitboard FromBB = S.getBitboard<C, PTK_Lance>();
    const auto Emit = [&](Square From, bool Pinned) {
        Bitboard ToBB = getLanceAttackBB<C>(From, OccupiedBB) & TargetSquares;
        if (Pinned) {
            ToBB &= LineBB[From][S.getKingSquare<C>()];
        }
        Moves = emitBoardMoves<C, PTK_Lance, Capture, WilyPromote>(
            S, Moves, From, ToBB);
    };
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach(
        [&](Square From) { Emit(From, true); });
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) { Emit(From, false); });
    return Moves;
}

// The capture pass (Capture == true) computes the attacks of each bishop
// and stores them through `AttackCache`; the no-capture pass that follows
// with the same occupancy replays them instead of recomputing. Both passes
// enumerate the pieces in the same order, so a single cursor suffices.
template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote>
inline Move32* generateOnBoardBishopMovesImpl(const StateImpl& S,
                                              Move32* __restrict Moves,
                                              const Bitboard& TargetSquares,
                                              const Bitboard& OccupiedBB,
                                              Bitboard*& AttackCache) noexcept {
    const auto LoadAttackBB = [&](Square From) {
        if constexpr (Capture) {
            const Bitboard AttackBB = getBishopAttackBB<Type>(From, OccupiedBB);
            *AttackCache++ = AttackBB;
            return AttackBB;
        } else {
            return *AttackCache++;
        }
    };
    const Bitboard FromBB = S.getBitboard<C, Type>();
    const auto Emit = [&](Square From, bool Pinned) {
        Bitboard ToBB = LoadAttackBB(From) & TargetSquares;
        if (Pinned) {
            ToBB &= LineBB[From][S.getKingSquare<C>()];
        }
        Moves = emitBoardMoves<C, Type, Capture, WilyPromote>(
            S, Moves, From, ToBB);
    };
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach(
        [&](Square From) { Emit(From, true); });
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) { Emit(From, false); });
    return Moves;
}

// See the comment on generateOnBoardBishopMovesImpl() about `AttackCache`.
template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote>
inline Move32* generateOnBoardRookMovesImpl(const StateImpl& S,
                                            Move32* __restrict Moves,
                                            const Bitboard& TargetSquares,
                                            const Bitboard& OccupiedBB,
                                            Bitboard*& AttackCache) noexcept {
    const auto LoadAttackBB = [&](Square From) {
        if constexpr (Capture) {
            const Bitboard AttackBB = getRookAttackBB<Type>(From, OccupiedBB);
            *AttackCache++ = AttackBB;
            return AttackBB;
        } else {
            return *AttackCache++;
        }
    };
    const Bitboard FromBB = S.getBitboard<C, Type>();
    const auto Emit = [&](Square From, bool Pinned) {
        Bitboard ToBB = LoadAttackBB(From) & TargetSquares;
        if (Pinned) {
            ToBB &= LineBB[From][S.getKingSquare<C>()];
        }
        Moves = emitBoardMoves<C, Type, Capture, WilyPromote>(
            S, Moves, From, ToBB);
    };
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach(
        [&](Square From) { Emit(From, true); });
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) { Emit(From, false); });
    return Moves;
}

#if defined(USE_AVX2)

template <Color C>
inline Move32*
generateDroppingMovesImplAVX2(const StateImpl& S, Move32* __restrict List,
                              const Bitboard& TargetSquares) noexcept {
    const Stands St = S.getPosition().getStand<C>();

    if (St == 0) {
        return List;
    }

    const bool PawnExists = getStandCount<PTK_Pawn>(St) > 0;

    if (PawnExists) {
        const Bitboard PawnBB = S.getBitboard<C, PTK_Pawn>();

        // Figure out files on which a pawn does not exist.
        const Bitboard Temp = RankBB[RankA].subtract(PawnBB) & RankBB[RankA];
        Bitboard ToBB = Temp.subtract(Temp.getRightShiftEpi64(8));
        if constexpr (C == White) {
            ToBB = ToBB.getLeftShiftEpi64(1);
        }

        ToBB &= TargetSquares;
        ToBB.forEach([&List](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Pawn);
        });
    }

    const unsigned Mask = getDroppingMoveMask(St);
    const bool LanceExists = (Mask & (1U << 4)) != 0;
    const bool KnightExists = (Mask & (1U << 5)) != 0;
    const auto& Pack = DroppingMovePacks[Mask];
    const uint32_t* Pack32 = Pack.Moves.data();
    int MoveCount = static_cast<int>(Pack.Count);

    const __m256i Pack256 = _mm256_load_si256((const __m256i*)Pack32);

    if (MoveCount == 0) {
        return List;
    }

    /* if (Stands exist) */ {
        const Bitboard ToBB = FirstAndSecondFurthestBB[C].andNot(TargetSquares);

        ToBB.forEach([&](Square To) {
            const __m256i Tos = _mm256_set1_epi32((int)To);
            const __m256i V = _mm256_or_si256(Pack256, Tos);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(List), V);
            List += MoveCount;
        });
    }

    {
        if (KnightExists) {
            --MoveCount;
        }

        if (MoveCount == 0) {
            return List;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::SecondFurthestBB<C>();
        ToBB.forEach([&](Square To) {
            const __m256i Tos = _mm256_set1_epi32((int)To);
            const __m256i V = _mm256_or_si256(Pack256, Tos);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(List), V);
            List += MoveCount;
        });
    }

    {
        if (LanceExists) {
            --MoveCount;
        }

        const __m128i Pack128 = _mm256_castsi256_si128(Pack256);

        if (MoveCount == 0) {
            return List;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::FurthermostBB<C>();
        ToBB.forEach([&](Square To) {
            const __m128i Tos = _mm_set1_epi32((int)To);
            const __m128i V = _mm_or_si128(Pack128, Tos);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(List), V);
            List += MoveCount;
        });
    }

    return List;
}

#elif defined(USE_NEON)

template <Color C>
inline Move32*
generateDroppingMovesImplNeon(const StateImpl& S, Move32* __restrict List,
                              const Bitboard& TargetSquares) noexcept {
    const Stands St = S.getPosition().getStand<C>();

    if (St == 0) {
        return List;
    }

    const bool PawnExists = getStandCount<PTK_Pawn>(St) > 0;

    if (PawnExists) {
        const Bitboard PawnBB = S.getBitboard<C, PTK_Pawn>();

        // Figure out files on which a pawn does not exist.
        const Bitboard Temp = RankBB[RankA].subtract(PawnBB) & RankBB[RankA];
        Bitboard ToBB = Temp.subtract(Temp.getRightShiftEpi64(8));
        if constexpr (C == White) {
            ToBB = ToBB.getLeftShiftEpi64(1);
        }

        ToBB &= TargetSquares;
        ToBB.forEach([&List](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Pawn);
        });
    }

    const unsigned Mask = getDroppingMoveMask(St);
    const bool LanceExists = (Mask & (1U << 4)) != 0;
    const bool KnightExists = (Mask & (1U << 5)) != 0;
    const auto& Pack = DroppingMovePacks[Mask];
    const uint32_t* Pack32 = Pack.Moves.data();
    int MoveCount = static_cast<int>(Pack.Count);

    if (MoveCount == 0) {
        return List;
    }

    /* if (Stands exist) */ {
        const Bitboard ToBB = FirstAndSecondFurthestBB[C].andNot(TargetSquares);

        if (MoveCount <= 4) {
            const uint32x4_t Pack128 = vld1q_u32(Pack32);
            ToBB.forEach([&](Square To) {
                const uint32x4_t Tos = vdupq_n_u32((uint32_t)To);
                const uint32x4_t V = vorrq_u32(Pack128, Tos);
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V);
                List += MoveCount;
            });
        } else {
            const uint32x4_t Pack128[2] = {
                vld1q_u32(&Pack32[0]),
                vld1q_u32(&Pack32[4]),
            };
            ToBB.forEach([&](Square To) {
                const uint32x4_t Tos = vdupq_n_u32((uint32_t)To);
                const uint32x4_t V0 = vorrq_u32(Pack128[0], Tos);
                const uint32x4_t V1 = vorrq_u32(Pack128[1], Tos);
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V0);
                List += 4;
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V1);
                List += (MoveCount - 4);
            });
        }
    }

    {
        if (KnightExists) {
            --MoveCount;
        }

        if (MoveCount == 0) {
            return List;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::SecondFurthestBB<C>();

        if (MoveCount <= 4) {
            const uint32x4_t Pack128 = vld1q_u32(Pack32);
            ToBB.forEach([&](Square To) {
                const uint32x4_t Tos = vdupq_n_u32((uint32_t)To);
                const uint32x4_t V = vorrq_u32(Pack128, Tos);
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V);
                List += MoveCount;
            });
        } else {
            const uint32x4_t Pack128[2] = {
                vld1q_u32(&Pack32[0]),
                vld1q_u32(&Pack32[4]),
            };
            ToBB.forEach([&](Square To) {
                const uint32x4_t Tos = vdupq_n_u32((uint32_t)To);
                const uint32x4_t V0 = vorrq_u32(Pack128[0], Tos);
                const uint32x4_t V1 = vorrq_u32(Pack128[1], Tos);
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V0);
                List += 4;
                vst1q_u32(reinterpret_cast<uint32_t*>(List), V1);
                List += (MoveCount - 4);
            });
        }
    }

    {
        if (LanceExists) {
            --MoveCount;
        }

        if (MoveCount == 0) {
            return List;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::FurthermostBB<C>();

        const uint32x4_t Pack128 = vld1q_u32(Pack32);
        ToBB.forEach([&](Square To) {
            const uint32x4_t Tos = vdupq_n_u32((uint32_t)To);
            const uint32x4_t V = vorrq_u32(Pack128, Tos);
            vst1q_u32(reinterpret_cast<uint32_t*>(List), V);
            List += MoveCount;
        });
    }

    return List;
}

#endif

template <Color C>
inline Move32*
generateDroppingMovesImpl(const StateImpl& S, Move32* __restrict List,
                          const Bitboard& TargetSquares) noexcept {

#if defined(USE_AVX2)
    return generateDroppingMovesImplAVX2<C>(S, List, TargetSquares);
#elif defined(USE_NEON)
    return generateDroppingMovesImplNeon<C>(S, List, TargetSquares);
#endif

    const Stands St = S.getPosition().getStand<C>();

    if (St == 0) {
        return List;
    }

    const bool PawnExists = getStandCount<PTK_Pawn>(St) > 0;

    if (PawnExists) {
        const Bitboard PawnBB = S.getBitboard<C, PTK_Pawn>();

        // Figure out files on which a pawn does not exist.
        const Bitboard Temp = RankBB[RankA].subtract(PawnBB) & RankBB[RankA];
        Bitboard ToBB = Temp.subtract(Temp.getRightShiftEpi64(8));
        if constexpr (C == White) {
            ToBB = ToBB.getLeftShiftEpi64(1);
        }

        ToBB &= TargetSquares;
        ToBB.forEach([&List](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Pawn);
        });
    }

    const bool SilverExists = getStandCount<PTK_Silver>(St) > 0;
    const bool GoldExists = getStandCount<PTK_Gold>(St) > 0;
    const bool BishopExists = getStandCount<PTK_Bishop>(St) > 0;
    const bool RookExists = getStandCount<PTK_Rook>(St) > 0;
    const bool LanceExists = getStandCount<PTK_Lance>(St) > 0;
    const bool KnightExists = getStandCount<PTK_Knight>(St) > 0;

    uint32_t DroppingCandidates[6];
    int MoveCount = 0;
    if (SilverExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Silver).value();
        ++MoveCount;
    }
    if (GoldExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Gold).value();
        ++MoveCount;
    }
    if (BishopExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Bishop).value();
        ++MoveCount;
    }
    if (RookExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Rook).value();
        ++MoveCount;
    }
    if (LanceExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Lance).value();
        ++MoveCount;
    }
    if (KnightExists) {
        DroppingCandidates[MoveCount] =
            Move32::droppingMove((Square)0, PTK_Knight).value();
        ++MoveCount;
    }

    /* if (Stands exist) */ {
        const Bitboard ToBB = FirstAndSecondFurthestBB[C].andNot(TargetSquares);

        switch (MoveCount) {
        case 1: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
            });
            break;
        }
        case 2: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
            });
            break;
        }
        case 3: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
            });
            break;
        }
        case 4: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
            });
            break;
        }
        case 5: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
            });
            break;
        }
        case 6: {
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[5] | (uint32_t)To);
            });
            break;
        }
        }
    }

    {
        if (KnightExists) {
            --MoveCount;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::SecondFurthestBB<C>();
        switch (MoveCount) {
        case 1:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
            });
            break;
        case 2:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
            });
            break;
        case 3:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
            });
            break;
        case 4:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
            });
            break;
        case 5:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
            });
            break;
        }
    }

    {
        if (LanceExists) {
            --MoveCount;
        }

        const Bitboard ToBB = TargetSquares & Bitboard::FurthermostBB<C>();
        switch (MoveCount) {
        case 1:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
            });
            break;
        case 2:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
            });
            break;
        case 3:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
            });
            break;
        case 4:
            ToBB.forEach([&](Square To) {
                *List++ =
                    Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                *List++ =
                    Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
            });
            break;
        }
    }

    return List;
}

template <Color C>
inline Move32*
generateDroppingStepCheckMovesImpl(const StateImpl& S, Move32* __restrict List,
                                   const Bitboard& TargetBB) noexcept {
    const Stands St = S.getPosition().getStand<C>();

    if (getStandCount<PTK_Pawn>(St) > 0) {
        const Bitboard PawnBB = S.getBitboard<C, PTK_Pawn>();

        if ((PawnBB & FileBB[squareToFile(S.getKingSquare<~C>())]).isZero()) {
            if constexpr (C == Black) {
                const Bitboard ToBB =
                    TargetBB & ((Bitboard::FurthermostBB<White>().andNot(
                                     SquareBB[S.getKingSquare<~C>()]))
                                    .template getRightShiftEpi64<1>());
                ToBB.forEach([&](Square To) {
                    assert(checkRange(To));
                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                });
            } else {
                const Bitboard ToBB =
                    TargetBB & ((Bitboard::FurthermostBB<Black>().andNot(
                                     SquareBB[S.getKingSquare<~C>()]))
                                    .template getLeftShiftEpi64<1>());
                ToBB.forEach([&](Square To) {
                    assert(checkRange(To));
                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                });
            }
        }
    }

    if (getStandCount<PTK_Knight>(St) > 0) {
        const Bitboard ToBB =
            TargetBB & getAttackBB<~C, PTK_Knight>(S.getKingSquare<~C>());
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Knight);
        });
    }

    if (getStandCount<PTK_Silver>(St) > 0) {
        const Bitboard ToBB =
            TargetBB & getAttackBB<~C, PTK_Silver>(S.getKingSquare<~C>());
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Silver);
        });
    }

    if (getStandCount<PTK_Gold>(St) > 0) {
        const Bitboard ToBB =
            TargetBB & getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>());
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Gold);
        });
    }

    return List;
}

template <Color C>
inline Move32* generateDroppingSliderCheckMovesImpl(
    const StateImpl& S, Move32* __restrict List, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB) noexcept {
    const Stands St = S.getPosition().getStand<C>();

    if (getStandCount<PTK_Lance>(St) > 0) {
        const Bitboard ToBB =
            getLanceAttackBB<~C>(S.getKingSquare<~C>(), OccupiedBB) & TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Lance);
        });
    }

    if (getStandCount<PTK_Rook>(St) > 0) {
        const Bitboard ToBB =
            getRookAttackBB<PTK_Rook>(S.getKingSquare<~C>(), OccupiedBB) &
            TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Rook);
        });
    }

    if (getStandCount<PTK_Bishop>(St) > 0) {
        const Bitboard ToBB =
            getBishopAttackBB<PTK_Bishop>(S.getKingSquare<~C>(), OccupiedBB) &
            TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Bishop);
        });
    }

    return List;
}

template <Color C, PieceTypeKind Type, bool Capture, bool Pinned,
          bool WilyPromote>
inline Move32* generateOnBoardOneStepNoPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& SourceFilter) noexcept {
    static_assert(
        Type != PTK_Lance,
        "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(
        Type != PTK_Bishop,
        "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(
        Type != PTK_Rook,
        "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as "
                                         "`Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook, "PTK_ProRook must not be passed as "
                                       "`Type` in generateCheckStepMoves().");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    if constexpr (!Pinned) {
        const Bitboard CheckSquares =
            getAttackBB<~C, Type>(S.getKingSquare<~C>()) & TargetBB;
        if (CheckSquares.isZero()) {
            return Moves;
        }
        if constexpr (Type == PTK_Pawn) {
            if constexpr (C == Black) {
                FromBB &= RankBB[RankI]
                              .andNot(CheckSquares)
                              .getRightShiftEpi64<1>();
            } else {
                FromBB &= RankBB[RankA]
                              .andNot(CheckSquares)
                              .getLeftShiftEpi64<1>();
            }
        }
    }

    if constexpr (Type != PTK_Knight) {
        (FromBB & S.getDefendingOpponentSliderBB<C>())
            .forEach([&](Square From) {
                Bitboard TargetBB2 =
                    (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                               getAttackBB<~C, Type>(S.getKingSquare<~C>()))
                            : getAttackBB<~C, Type>(S.getKingSquare<~C>())) &
                    getAttackBB<C, Type>(From) & TargetBB;

                if constexpr (Type == PTK_Pawn) {
                    if constexpr (WilyPromote) {
                        TargetBB2 = PromotableBB[C].andNot(TargetBB2);
                    } else {
                        TargetBB2 =
                            Bitboard::FurthermostBB<C>().andNot(TargetBB2);
                    }
                }

                (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
                    .forEach([&](Square To) {
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ =
                                Move32::boardMove(From, To, Type, CaptureType);
                        } else {
                            *Moves++ = Move32::boardMove(From, To, Type);
                        }
                    });
            });
    }

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           getAttackBB<~C, Type>(S.getKingSquare<~C>()))
                        : getAttackBB<~C, Type>(S.getKingSquare<~C>())) &
                getAttackBB<C, Type>(From) & TargetBB;

            if constexpr (Type == PTK_Pawn) {
                if constexpr (WilyPromote) {
                    TargetBB2 = PromotableBB[C].andNot(TargetBB2);
                } else {
                    TargetBB2 = Bitboard::FurthermostBB<C>().andNot(TargetBB2);
                }
            }

            if constexpr (Type == PTK_Knight) {
                TargetBB2 = FirstAndSecondFurthestBB[C].andNot(TargetBB2);
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Type == PTK_King) {
                    if (S.isAttacked<C>(To, From)) {
                        return;
                    }
                }

                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardOneStepNoPromoteCheckGoldKindsMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& SourceFilter) noexcept {
    if constexpr (!Pinned) {
        if ((TargetBB & getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())).isZero()) {
            return Moves;
        }
    }
    const Bitboard FromBB =
        S.getBitboard<C>() & SourceFilter &
        (S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>() |
         S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>() |
         S.getBitboard<PTK_ProSilver>());

    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const PieceTypeKind Type = getPieceType(S.getPosition().pieceOn(From));

        const Bitboard TargetBB2 =
            (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                       getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                    : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
            getAttackBB<C, PTK_Gold>(From) & TargetBB;

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const PieceTypeKind Type =
                getPieceType(S.getPosition().pieceOn(From));

            const Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                        : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
                getAttackBB<C, PTK_Gold>(From) & TargetBB;

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote, bool Pinned>
inline Move32* generateOnBoardLanceNoPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter,
    const CheckAttackCache& CheckAttacks) noexcept {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned) ? ~LineBB[From][S.getKingSquare<~C>()]
                     : CheckAttacks.Lance;

        if constexpr (WilyPromote) {
            TargetBB2 = FirstAndSecondFurthestBB[C].andNot(TargetBB2);
        } else {
            TargetBB2 = Bitboard::FurthermostBB<C>().andNot(TargetBB2);
        }

        TargetBB2 &= getLanceAttackBB<C>(From, OccupiedBB) & TargetBB;

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            Bitboard TargetBB2 =
                (Pinned)
                    ? ~LineBB[From][S.getKingSquare<~C>()]
                    : CheckAttacks.Lance;

            if constexpr (WilyPromote) {
                TargetBB2 = FirstAndSecondFurthestBB[C].andNot(TargetBB2);
            } else {
                TargetBB2 = Bitboard::FurthermostBB<C>().andNot(TargetBB2);
            }

            TargetBB2 &= getLanceAttackBB<C>(From, OccupiedBB) & TargetBB;

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool Pinned,
          bool WilyPromote>
inline Move32* generateOnBoardBishopNoPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter,
    const CheckAttackCache& CheckAttacks) noexcept {
    static_assert(
        Type == PTK_Bishop || Type == PTK_ProBishop,
        "Only PTK_Bishop or PTK_ProBishop can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        if constexpr (WilyPromote && Type == PTK_Bishop) {
            if (PromotableBB[C].isSet(From)) {
                return;
            }
        }

        Bitboard TargetBB2;

        if constexpr (Pinned) {
            TargetBB2 =
                ~LineBB[From][S.getKingSquare<~C>()] |
                CheckAttacks.getBishop<Type>();
        } else {
            TargetBB2 =
                CheckAttacks.getBishop<Type>();
        }
        TargetBB2 =
            TargetBB2 & getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if constexpr (WilyPromote && Type == PTK_Bishop) {
            TargetBB2 = PromotableBB[C].andNot(TargetBB2);
        }

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            if constexpr (WilyPromote && Type == PTK_Bishop) {
                if (PromotableBB[C].isSet(From)) {
                    return;
                }
            }

            Bitboard TargetBB2;

            if constexpr (Pinned) {
                TargetBB2 =
                    ~LineBB[From][S.getKingSquare<~C>()] |
                    CheckAttacks.getBishop<Type>();
            } else {
                TargetBB2 =
                    CheckAttacks.getBishop<Type>();
            }
            TargetBB2 = TargetBB2 & getBishopAttackBB<Type>(From, OccupiedBB) &
                        TargetBB;

            if constexpr (WilyPromote && Type == PTK_Bishop) {
                TargetBB2 = PromotableBB[C].andNot(TargetBB2);
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool Pinned,
          bool WilyPromote>
inline Move32* generateOnBoardRookNoPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter,
    const CheckAttackCache& CheckAttacks) noexcept {
    static_assert(
        Type == PTK_Rook || Type == PTK_ProRook,
        "Only PTK_Rook or PTK_ProRook can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        if constexpr (WilyPromote && Type == PTK_Rook) {
            if (PromotableBB[C].isSet(From)) {
                return;
            }
        }

        Bitboard TargetBB2;

        if constexpr (Pinned) {
            TargetBB2 =
                ~LineBB[From][S.getKingSquare<~C>()] |
                CheckAttacks.getRook<Type>();
        } else {
            TargetBB2 =
                CheckAttacks.getRook<Type>();
        }
        TargetBB2 =
            TargetBB2 & getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if constexpr (WilyPromote && Type == PTK_Rook) {
            TargetBB2 = PromotableBB[C].andNot(TargetBB2);
        }

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            if constexpr (WilyPromote && Type == PTK_Rook) {
                if (PromotableBB[C].isSet(From)) {
                    return;
                }
            }

            Bitboard TargetBB2;

            if constexpr (Pinned) {
                TargetBB2 =
                    ~LineBB[From][S.getKingSquare<~C>()] |
                    CheckAttacks.getRook<Type>();
            } else {
                TargetBB2 =
                    CheckAttacks.getRook<Type>();
            }
            TargetBB2 =
                TargetBB2 & getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

            if constexpr (WilyPromote && Type == PTK_Rook) {
                TargetBB2 = PromotableBB[C].andNot(TargetBB2);
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool Pinned>
inline Move32* generateOnBoardOneStepPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& SourceFilter) noexcept {
    static_assert(
        Type != PTK_King,
        "PTK_King must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(
        Type != PTK_Lance,
        "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(
        Type != PTK_Bishop,
        "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(
        Type != PTK_Rook,
        "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as "
                                         "`Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook, "PTK_ProRook must not be passed as "
                                       "`Type` in generateCheckStepMoves().");
    static_assert(!isPromoted(Type), "Type must be an unpromoted piece type.");

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                       getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                    : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
            getAttackBB<C, Type>(From) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                        : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
                getAttackBB<C, Type>(From) & TargetBB;

            if (!PromotableBB[C].isSet(From)) {
                TargetBB2 &= PromotableBB[C];
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardLancePromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const Bitboard TargetBB2 =
            (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                       getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                    : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
            getLanceAttackBB<C>(From, OccupiedBB) & TargetBB & PromotableBB[C];

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>()))
                        : getAttackBB<~C, PTK_Gold>(S.getKingSquare<~C>())) &
                getLanceAttackBB<C>(From, OccupiedBB) & TargetBB &
                PromotableBB[C];

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardBishopPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter,
    const CheckAttackCache& CheckAttacks) noexcept {
    constexpr PieceTypeKind Type = PTK_Bishop;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned
                 ? (~LineBB[From][S.getKingSquare<~C>()] |
                    CheckAttacks.getBishop<Type>() |
                    CheckAttacks.King)
                 : (CheckAttacks.getBishop<Type>() |
                    CheckAttacks.King)) &
            getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           CheckAttacks.getBishop<Type>() |
                           CheckAttacks.King)
                        : (CheckAttacks.getBishop<Type>() |
                           CheckAttacks.King)) &
                getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

            if (!PromotableBB[C].isSet(From)) {
                TargetBB2 &= PromotableBB[C];
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardRookPromoteCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter,
    const CheckAttackCache& CheckAttacks) noexcept {
    constexpr PieceTypeKind Type = PTK_Rook;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned
                 ? (~LineBB[From][S.getKingSquare<~C>()] |
                    CheckAttacks.getRook<Type>() |
                    CheckAttacks.King)
                 : (CheckAttacks.getRook<Type>() |
                    CheckAttacks.King)) &
            getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        (TargetBB2 & LineBB[From][S.getKingSquare<C>()])
            .forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            Bitboard TargetBB2 =
                (Pinned ? (~LineBB[From][S.getKingSquare<~C>()] |
                           CheckAttacks.getRook<Type>() |
                           CheckAttacks.King)
                        : (CheckAttacks.getRook<Type>() |
                           CheckAttacks.King)) &
                getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

            if (!PromotableBB[C].isSet(From)) {
                TargetBB2 &= PromotableBB[C];
            }

            TargetBB2.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }
            });
        });

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote>
inline Move32*
generateOnBoardOneStepMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                                const Bitboard& TargetSquares) noexcept {
    Moves = generateOnBoardOneStepPawnMovesImpl<C, Capture, WilyPromote>(
        S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, Capture>(
        S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, Capture>(
        S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, Capture>(
        S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepGoldKindsMovesImpl<C, Capture>(S, Moves,
                                                                 TargetSquares);

    return Moves;
}

template <Color C, bool Capture, bool Pinned, bool WilyPromote,
          bool SkipKing = false>
inline Move32* generateOnBoardOneStepCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& FromMask) noexcept {
    if (TargetBB.isZero()) {
        return Moves;
    }

    // A king can give check only by uncovering a friendly slider.
    if constexpr (!SkipKing && Pinned) {
        Moves =
            generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, Capture,
                                                          Pinned, WilyPromote>(
                S, Moves, TargetBB, FromMask);
    }

    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Pawn, Capture,
                                                          Pinned, WilyPromote>(
        S, Moves, TargetBB, FromMask);
    Moves =
        generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Knight, Capture,
                                                      Pinned, WilyPromote>(
            S, Moves, TargetBB, FromMask);
    Moves =
        generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Silver, Capture,
                                                      Pinned, WilyPromote>(
            S, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckGoldKindsMovesImpl<C, Capture,
                                                                   Pinned>(
        S, Moves, TargetBB, FromMask);

    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Pawn, Capture,
                                                        Pinned>(
        S, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Knight, Capture,
                                                        Pinned>(
        S, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Silver, Capture,
                                                        Pinned>(
        S, Moves, TargetBB, FromMask);

    return Moves;
}

template <Color C, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardSliderCheckMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& TargetBB,
    const Bitboard& OccupiedBB, const Bitboard& FromMask,
    const CheckAttackCache& CheckAttacks) noexcept {
    if (TargetBB.isZero()) {
        return Moves;
    }

    Moves = generateOnBoardLanceNoPromoteCheckMovesImpl<C, Capture, WilyPromote,
                                                        Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);
    Moves = generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_Bishop, Capture,
                                                         Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);
    Moves =
        generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_ProBishop, Capture,
                                                     Pinned, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_Rook, Capture,
                                                       Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_ProRook, Capture,
                                                       Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);

    Moves = generateOnBoardLancePromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopPromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);
    Moves = generateOnBoardRookPromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask, CheckAttacks);

    return Moves;
}

// `AttackCache` must point to storage for the bishop/rook attack bitboards
// of side `C` (SliderAttackCacheSize entries). The capture pass fills it
// and the following no-capture pass, called with the same state and
// occupancy, consumes it.
template <Color C, bool Capture, bool WilyPromote>
inline Move32* generateOnBoardSliderMovesImpl(const StateImpl& S,
                                              Move32* __restrict Moves,
                                              const Bitboard& TargetSquares,
                                              const Bitboard& OccupiedBB,
                                              Bitboard* AttackCache) noexcept {
    Moves = generateOnBoardLanceMovesImpl<C, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves =
        generateOnBoardBishopMovesImpl<C, PTK_ProBishop, Capture, WilyPromote>(
            S, Moves, TargetSquares, OccupiedBB, AttackCache);
    Moves = generateOnBoardBishopMovesImpl<C, PTK_Bishop, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB, AttackCache);
    Moves = generateOnBoardRookMovesImpl<C, PTK_ProRook, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB, AttackCache);
    Moves = generateOnBoardRookMovesImpl<C, PTK_Rook, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB, AttackCache);

    return Moves;
}

template <Color C, PieceTypeKind Type, bool WilyPromote>
inline Move32* generateBoardMoves(const StateImpl& S, Move32* Moves,
                                  const Bitboard& TargetBB,
                                  const Bitboard& OccupiedBB) noexcept {
    const Bitboard FromBB = Type == PTK_Gold
        ? ((S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>() |
            S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>() |
            S.getBitboard<PTK_ProSilver>()) & S.getBitboard<C>())
        : S.getBitboard<C, Type>();
    const auto Attacks = [&](Square From) {
        if constexpr (Type == PTK_Lance) {
            return getLanceAttackBB<C>(From, OccupiedBB);
        } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
            return getBishopAttackBB<Type>(From, OccupiedBB);
        } else if constexpr (Type == PTK_Rook || Type == PTK_ProRook) {
            return getRookAttackBB<Type>(From, OccupiedBB);
        } else {
            return getAttackBB<C, Type>(From);
        }
    };
    const auto Emit = [&](Square From, const Bitboard& AttacksBB) {
        Moves = emitBoardMoves<C, Type, true, WilyPromote>(
            S, Moves, From, AttacksBB & TargetBB);
    };
    if constexpr (Type != PTK_Knight) {
        (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
            Emit(From, Attacks(From) & LineBB[From][S.getKingSquare<C>()]);
        });
    }
    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach([&](Square From) {
        Emit(From, Attacks(From));
    });
    return Moves;
}

template <Color C, bool CaptureOnly, bool WilyPromote>
inline Move32* generateLegalEvasionMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& CheckerBB,
    const Bitboard& OpponentBB, const Bitboard& OccupiedBB) noexcept {
    // Moving the king.
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, true>(S, Moves,
                                                               OpponentBB);
    if constexpr (!CaptureOnly) {
        Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, false>(
            S, Moves, ~OccupiedBB);
    }

    // If there are more than one checkers,
    // no moves available but moving the king.
    if (CheckerBB.popCount() >= 2) {
        return Moves;
    }

    // Capturing a checker.
    Moves = generateOnBoardOneStepPawnMovesImpl<C, true, WilyPromote>(
        S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, true>(S, Moves,
                                                                 CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, true>(S, Moves,
                                                                 CheckerBB);
    Moves =
        generateOnBoardOneStepGoldKindsMovesImpl<C, true>(S, Moves, CheckerBB);

    assert((S.getBitboard<C, PTK_Bishop>() | S.getBitboard<C, PTK_ProBishop>() |
            S.getBitboard<C, PTK_Rook>() | S.getBitboard<C, PTK_ProRook>())
               .popCount() <= SliderAttackCacheSize);
    Bitboard SliderAttackCache[SliderAttackCacheSize];
    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(
        S, Moves, CheckerBB, OccupiedBB, SliderAttackCache);

    const Square CheckerSq = CheckerBB.getOne();
    const Bitboard BetweenBB = getBetweenBB(S.getKingSquare<C>(), CheckerSq);

    if (BetweenBB.isZero()) {
        return Moves;
    }

    // Moving a piece.
    if constexpr (!CaptureOnly) {
        Moves = generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(
            S, Moves, BetweenBB);
        Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(
            S, Moves, BetweenBB, OccupiedBB, SliderAttackCache);
    }

    // Dropping a piece (a.k.a. aigoma).
    if constexpr (!CaptureOnly) {
        Moves = generateDroppingMovesImpl<C>(S, Moves, BetweenBB);
    }

    return Moves;
}

template <Color C, bool CaptureOnly, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const StateImpl& S,
                                      Move32* __restrict Moves,
                                      const Bitboard& OpponentBB,
                                      const Bitboard& OccupiedBB) noexcept {
    if constexpr (!CaptureOnly) {
        // Emit captures and quiets together, directly into the output list.
        // The capture-capable emitters also accept empty target squares.
        const Bitboard TargetBB = ~S.getBitboard<C>();
        Moves = generateOnBoardOneStepPawnMovesImpl<C, true, WilyPromote>(
            S, Moves, TargetBB);
        Moves = generateBoardMoves<C, PTK_Knight, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_Silver, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, true>(
            S, Moves, TargetBB);
        Moves = generateBoardMoves<C, PTK_Gold, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_Lance, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_ProBishop, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_Bishop, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_ProRook, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        Moves = generateBoardMoves<C, PTK_Rook, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB);
        return generateDroppingMovesImpl<C>(S, Moves, ~OccupiedBB);
    }

    // Captures.
    Moves = generateOnBoardOneStepMovesImpl<C, true, WilyPromote>(S, Moves,
                                                                  OpponentBB);
    assert((S.getBitboard<C, PTK_Bishop>() | S.getBitboard<C, PTK_ProBishop>() |
            S.getBitboard<C, PTK_Rook>() | S.getBitboard<C, PTK_ProRook>())
               .popCount() <= SliderAttackCacheSize);
    Bitboard SliderAttackCache[SliderAttackCacheSize];
    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(
        S, Moves, OpponentBB, OccupiedBB, SliderAttackCache);

    return Moves;
}

template <Color C, bool CaptureOnly, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const StateImpl& S,
                                      Move32* Moves) noexcept {
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard BlackBB = S.getBitboard<Black>();
    const Bitboard WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;

    if constexpr (C == Black) {
        if (!CheckerBB.isZero()) {
            Moves =
                generateLegalEvasionMovesImpl<Black, CaptureOnly, WilyPromote>(
                    S, Moves, CheckerBB, WhiteBB, OccupiedBB);
        } else {
            Moves = generateLegalMovesImpl<Black, CaptureOnly, WilyPromote>(
                S, Moves, WhiteBB, OccupiedBB);
        }
    } else {
        if (!CheckerBB.isZero()) {
            Moves =
                generateLegalEvasionMovesImpl<White, CaptureOnly, WilyPromote>(
                    S, Moves, CheckerBB, BlackBB, OccupiedBB);
        } else {
            Moves = generateLegalMovesImpl<White, CaptureOnly, WilyPromote>(
                S, Moves, BlackBB, OccupiedBB);
        }
    }

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalCheckMovesImpl(const StateImpl& S,
                                           Move32* __restrict Moves) noexcept {
    const Bitboard BlackBB = S.getBitboard<Black>();
    const Bitboard WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;
    const Bitboard EmptyBB = ~OccupiedBB;
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard PinnedBB =
        S.getDefendingOpponentSliderBB<~C>() & S.getBitboard<C>();
    const Bitboard NoPinnedBB = ~PinnedBB;

    if (CheckerBB.popCount() >= 2) {
        if (PinnedBB.isSet(S.getKingSquare<C>())) {
            const Bitboard KingLineBB =
                LineBB[S.getKingSquare<C>()][S.getKingSquare<~C>()];
            const Bitboard UnpinEmptyBB = KingLineBB.andNot(EmptyBB);
            const Bitboard UnpinCaptureBB =
                KingLineBB.andNot(S.getBitboard<~C>());

            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<
                C, PTK_King, false, true, WilyPromote>(S, Moves, UnpinEmptyBB,
                                                       OccupiedBB);
            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<
                C, PTK_King, true, true, WilyPromote>(S, Moves, UnpinCaptureBB,
                                                      OccupiedBB);
        }

        return Moves;
    }

    const Square OpponentKing = S.getKingSquare<~C>();
    const CheckAttackCache CheckAttacks{
        ((S.getBitboard<PTK_Bishop>() | S.getBitboard<PTK_ProBishop>()) &
         S.getBitboard<C>()).isZero()
            ? Bitboard::ZeroBB()
            : getBishopAttackBB<PTK_Bishop>(OpponentKing, OccupiedBB),
        ((S.getBitboard<PTK_Rook>() | S.getBitboard<PTK_ProRook>()) &
         S.getBitboard<C>()).isZero()
            ? Bitboard::ZeroBB()
            : getRookAttackBB<PTK_Rook>(OpponentKing, OccupiedBB),
        S.getBitboard<C, PTK_Lance>().isZero()
            ? Bitboard::ZeroBB()
            : getLanceAttackBB<~C>(OpponentKing, OccupiedBB),
        getAttackBB<~C, PTK_King>(OpponentKing)};

    const bool PinnedExists = !PinnedBB.isZero();
    const Stands St = S.getPosition().getStand<C>();

    if (!CheckerBB.isZero()) {
        const Square CheckerSq = CheckerBB.getOne();
        const Bitboard BetweenTarget =
            BetweenBB[CheckerSq][S.getKingSquare<C>()];

        if (St != 0) {
            Moves =
                generateDroppingStepCheckMovesImpl<C>(S, Moves, BetweenTarget);
            Moves = generateDroppingSliderCheckMovesImpl<C>(
                S, Moves, BetweenTarget, OccupiedBB);
        }

        // Capturing moves.

        const Bitboard CheckerMyKingBetweenBB =
            BetweenBB[CheckerSq][S.getKingSquare<C>()];

        if (PinnedExists) {
            if (PinnedBB.isSet(S.getKingSquare<C>())) {
                const Bitboard KingLineBB =
                    LineBB[S.getKingSquare<C>()][S.getKingSquare<~C>()];
                const Bitboard UnpinEmptyBB = KingLineBB.andNot(EmptyBB);
                const Bitboard UnpinCaptureBB =
                    KingLineBB.andNot(S.getBitboard<~C>());

                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<
                    C, PTK_King, false, true, WilyPromote>(
                    S, Moves, UnpinEmptyBB, OccupiedBB);
                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<
                    C, PTK_King, true, true, WilyPromote>(
                    S, Moves, UnpinCaptureBB, OccupiedBB);
            }

            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true,
                                                         WilyPromote, true>(
                S, Moves, CheckerMyKingBetweenBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true,
                                                         WilyPromote, true>(
                S, Moves, CheckerBB, PinnedBB);

            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true,
                                                        WilyPromote>(
                S, Moves, CheckerMyKingBetweenBB, OccupiedBB, PinnedBB, CheckAttacks);
            Moves =
                generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(
                    S, Moves, CheckerBB, OccupiedBB, PinnedBB, CheckAttacks);
        }

        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, CheckerMyKingBetweenBB, NoPinnedBB);
        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, CheckerBB, NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, CheckerMyKingBetweenBB, OccupiedBB, NoPinnedBB, CheckAttacks);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, CheckerBB, OccupiedBB, NoPinnedBB, CheckAttacks);
    } else {
        if (St != 0) {
            Moves = generateDroppingStepCheckMovesImpl<C>(S, Moves, EmptyBB);
            Moves = generateDroppingSliderCheckMovesImpl<C>(S, Moves, EmptyBB,
                                                            OccupiedBB);
        }

        if (PinnedExists) {
            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true,
                                                         WilyPromote>(
                S, Moves, EmptyBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true,
                                                         WilyPromote>(
                S, Moves, S.getBitboard<~C>(), PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true,
                                                        WilyPromote>(
                S, Moves, EmptyBB, OccupiedBB, PinnedBB, CheckAttacks);
            Moves =
                generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(
                    S, Moves, S.getBitboard<~C>(), OccupiedBB, PinnedBB, CheckAttacks);
        }

        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, EmptyBB, NoPinnedBB);
        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, S.getBitboard<~C>(), NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, EmptyBB, OccupiedBB, NoPinnedBB, CheckAttacks);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, S.getBitboard<~C>(), OccupiedBB, NoPinnedBB, CheckAttacks);
    }

    return Moves;
}

} // namespace

template <Color C, bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalMoves(const StateImpl& S) noexcept {
    MoveList List;
    List.Tail = generateLegalMovesImpl<C, false, WilyPromote>(S, List.Tail);
    return List;
}

template <Color C, bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalCheckMoves(const StateImpl& S) noexcept {
    MoveList List;
    List.Tail = generateLegalCheckMovesImpl<C, WilyPromote>(S, List.Tail);
    return List;
}

template <Color C, bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalCaptureMoves(const StateImpl& S) noexcept {
    MoveList List;
    List.Tail = generateLegalMovesImpl<C, true, WilyPromote>(S, List.Tail);
    return List;
}

template <Color C, bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalEvasionMoves(const StateImpl& S) noexcept {
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard MyBB = S.getBitboard<C>();
    const Bitboard OpBB = S.getBitboard<~C>();
    const Bitboard OccupiedBB = MyBB | OpBB;

    assert(!CheckerBB.isZero());

    MoveList List;
    List.Tail = generateLegalEvasionMovesImpl<C, false, WilyPromote>(
        S, List.Tail, CheckerBB, OpBB, OccupiedBB);

    return List;
}

template <Color C>
Move32
MoveGeneratorInternal::generateLegalSmallestMove(const internal::StateImpl& S,
                                                 Square To) noexcept {
    { // When the king is in check, we can only move the king or capture the
      // checker.
        const Bitboard CheckerBB = S.getCheckerBB();

        if (!CheckerBB.isZero()) {
            if (CheckerBB.popCount() >= 2 || !CheckerBB.isSet(To)) {
                // When there are more than one checkers, or the destination
                // square is not the square of the (single) checker, no
                // non-king move to the destination square can resolve the
                // check. The only candidate is moving the king to the
                // destination square so that the king escapes the check.
                const Square KingSq = S.getKingSquare<C>();
                if (getAttackBB<C, PTK_King>(KingSq).isSet(To) &&
                    !S.isAttacked<C>(To, KingSq)) {
                    return Move32::boardMove(
                        KingSq, To, PTK_King,
                        getPieceType(S.getPosition().pieceOn(To)));
                }
                return Move32::MoveNone();
            }
        }
    }

    // Order:
    //      1. Pawn
    //      2. ProPawn
    //      3. Lance
    //      4. ProLance
    //      5. Knight
    //      6. ProKnight
    //      7. Silver
    //      8. ProSilver
    //      9. Gold
    //     10. Bishop
    //     11. Rook
    //     12. ProBishop
    //     13. ProRook
    //     14. King
    //
    // Process Pawn and King separately because it needs additional check.

    // Pawn.
    if constexpr (C == Black) {
        if (S.getBitboard<C, PTK_Pawn>().getLeftShiftEpi64(1).isSet(To)) {
            if (S.getDefendingOpponentSliderBB<C>().isSet(To + South)) {
                // This pawn is pinned.
                // We have to check whether this move is legal or not.
                // This move is legal when the line (= file because this is a
                // pawn) between the king and the piece contains the destination
                // square.
                if (LineBB[To + South][S.getKingSquare<C>()].isSet(To)) {
                    if (PromotableBB[C].isSet(To)) {
                        return Move32::boardPromotingMove(
                            To + South, To, PTK_Pawn,
                            getPieceType(S.getPosition().pieceOn(To)));
                    } else {
                        return Move32::boardMove(
                            To + South, To, PTK_Pawn,
                            getPieceType(S.getPosition().pieceOn(To)));
                    }
                }
            } else {
                if (PromotableBB[C].isSet(To)) {
                    return Move32::boardPromotingMove(
                        To + South, To, PTK_Pawn,
                        getPieceType(S.getPosition().pieceOn(To)));
                } else {
                    return Move32::boardMove(
                        To + South, To, PTK_Pawn,
                        getPieceType(S.getPosition().pieceOn(To)));
                }
            }
        }
    } else {
        if (S.getBitboard<C, PTK_Pawn>().getRightShiftEpi64(1).isSet(To)) {
            if (S.getDefendingOpponentSliderBB<C>().isSet(To + North)) {
                if (LineBB[To + North][S.getKingSquare<C>()].isSet(To)) {
                    if (PromotableBB[C].isSet(To)) {
                        return Move32::boardPromotingMove(
                            To + North, To, PTK_Pawn,
                            getPieceType(S.getPosition().pieceOn(To)));
                    } else {
                        return Move32::boardMove(
                            To + North, To, PTK_Pawn,
                            getPieceType(S.getPosition().pieceOn(To)));
                    }
                }
            } else {
                if (PromotableBB[C].isSet(To)) {
                    return Move32::boardPromotingMove(
                        To + North, To, PTK_Pawn,
                        getPieceType(S.getPosition().pieceOn(To)));
                } else {
                    return Move32::boardMove(
                        To + North, To, PTK_Pawn,
                        getPieceType(S.getPosition().pieceOn(To)));
                }
            }
        }
    }

    const Bitboard MyBB = S.getBitboard<C>();
    const Bitboard OpBB = S.getBitboard<~C>();
    const Bitboard OccupiedBB = MyBB | OpBB;

    // clang-format off
#define NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(Type) {                         \
    Bitboard FromBB = S.getBitboard<C, Type>() & getAttackBB<~C, Type>(To);                \
                                                                                           \
    while (!FromBB.isZero()) {                                                             \
        const Square FromSq = FromBB.popOne();                                             \
        if (S.getDefendingOpponentSliderBB<C>().isSet(FromSq)) {                           \
            if (LineBB[FromSq][S.getKingSquare<C>()].isSet(To)) {                          \
                if (!isPromoted(Type) && Type != PTK_Gold &&                               \
                      !(PromotableBB[C] & (SquareBB[To] | SquareBB[FromSq])).isZero()) {   \
                    return Move32::boardPromotingMove(                                     \
                            FromSq, To, Type,                                              \
                            getPieceType(S.getPosition().pieceOn(To)));                    \
                } else {                                                                   \
                    return Move32::boardMove(                                              \
                            FromSq, To, Type,                                              \
                            getPieceType(S.getPosition().pieceOn(To)));                    \
                }                                                                          \
            }                                                                              \
        } else {                                                                           \
            if (!isPromoted(Type) && Type != PTK_Gold &&                                   \
                  !(PromotableBB[C] & (SquareBB[To] | SquareBB[FromSq])).isZero()) {       \
                return Move32::boardPromotingMove(                                         \
                        FromSq, To, Type,                                                  \
                        getPieceType(S.getPosition().pieceOn(To)));                        \
            } else {                                                                       \
                return Move32::boardMove(                                                  \
                        FromSq, To, Type,                                                  \
                        getPieceType(S.getPosition().pieceOn(To)));                        \
            }                                                                              \
        }                                                                                  \
    }                                                                                      \
}

#define NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(Type) {                       \
    Bitboard FromBB = S.getBitboard<C, Type>() &                                           \
        getSliderAttackBB<~C, Type>(To, OccupiedBB);                                       \
                                                                                           \
    while (!FromBB.isZero()) {                                                             \
        const Square FromSq = FromBB.popOne();                                             \
        if (S.getDefendingOpponentSliderBB<C>().isSet(FromSq)) {                           \
            if (LineBB[FromSq][S.getKingSquare<C>()].isSet(To)) {                          \
                if (!isPromoted(Type) &&                                                   \
                        !(PromotableBB[C] & (SquareBB[To] | SquareBB[FromSq])).isZero()) { \
                    return Move32::boardPromotingMove(                                     \
                            FromSq, To, Type,                                              \
                            getPieceType(S.getPosition().pieceOn(To)));                    \
                } else {                                                                   \
                    return Move32::boardMove(                                              \
                            FromSq, To, Type,                                              \
                            getPieceType(S.getPosition().pieceOn(To)));                    \
                }                                                                          \
            }                                                                              \
        } else {                                                                           \
            if (!isPromoted(Type) &&                                                       \
                    !(PromotableBB[C] & (SquareBB[To] | SquareBB[FromSq])).isZero()) {     \
                return Move32::boardPromotingMove(                                         \
                        FromSq, To, Type,                                                  \
                        getPieceType(S.getPosition().pieceOn(To)));                        \
            } else {                                                                       \
                return Move32::boardMove(                                                  \
                        FromSq, To, Type,                                                  \
                        getPieceType(S.getPosition().pieceOn(To)));                        \
            }                                                                              \
        }                                                                                  \
    }                                                                                      \
}
    // clang-format on

    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_ProPawn);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(PTK_Lance);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_ProLance);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_Knight);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_ProKnight);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_Silver);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_ProSilver);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP(PTK_Gold);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(PTK_Bishop);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(PTK_Rook);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(PTK_ProBishop);
    NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER(PTK_ProRook);

#undef NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_STEP
#undef NSHOGI_GENERATE_SMALLEST_CAPTURE_MOVE_PROCESS_SLIDER

    // King.
    const Square KingSq = S.getKingSquare<C>();
    if (getAttackBB<C, PTK_King>(KingSq).isSet(To)) {
        // We have to check moving the king to
        // `To` is legal or not.
        // If the square is attacked by opponent's piece,
        // it's not legal.
        assert(getPieceType(S.getPosition().pieceOn(To)) != PTK_Empty);
        if (!S.isAttacked<C>(To, KingSq)) {
            // The square is not attacked.
            // Thus, the king can capture the checker.
            return Move32::boardMove(KingSq, To, PTK_King,
                                     getPieceType(S.getPosition().pieceOn(To)));
        }
    }

    return Move32::MoveNone();
}

template MoveList MoveGeneratorInternal::generateLegalMoves<Black, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalMoves<Black, true>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalMoves<White, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalMoves<White, true>(
    const StateImpl& S) noexcept;

template MoveList MoveGeneratorInternal::generateLegalCheckMoves<Black, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCheckMoves<Black, true>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCheckMoves<White, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCheckMoves<White, true>(
    const StateImpl& S) noexcept;

template MoveList
MoveGeneratorInternal::generateLegalCaptureMoves<Black, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCaptureMoves<Black, true>(
    const StateImpl& S) noexcept;
template MoveList
MoveGeneratorInternal::generateLegalCaptureMoves<White, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCaptureMoves<White, true>(
    const StateImpl& S) noexcept;

template MoveList
MoveGeneratorInternal::generateLegalEvasionMoves<Black, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalEvasionMoves<Black, true>(
    const StateImpl& S) noexcept;
template MoveList
MoveGeneratorInternal::generateLegalEvasionMoves<White, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalEvasionMoves<White, true>(
    const StateImpl& S) noexcept;

template Move32 MoveGeneratorInternal::generateLegalSmallestMove<Black>(
    const internal::StateImpl& S, Square To) noexcept;
template Move32 MoveGeneratorInternal::generateLegalSmallestMove<White>(
    const internal::StateImpl& S, Square To) noexcept;

} // namespace internal
} // namespace core
} // namespace nshogi
