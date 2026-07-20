//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_INTERNAL_STATEIMPL_H
#define NSHOGI_CORE_INTERNAL_STATEIMPL_H

#include "../position.h"
#include "bitboard.h"
#include "hash.h"
#include "statehelper.h"

namespace nshogi {
namespace core {
namespace internal {

// One directional Kogge-Stone flood: extends GeneratorBB over PropagateBB
// squares along the direction of `Shift` bits, by doubling steps of
// 1 + 2 + 4 (a ray is at most eight squares long). PropagateBB must
// exclude every square a ray cannot pass through: the occupied squares
// and, when the direction has one, its edge rank; the caller also removes
// the edge rank from GeneratorBB so that no shift can wrap into a
// neighboring file. Shifting the flood one more step yields the attacks.

template <int Shift>
inline bitboard::Bitboard
koggeStoneRightSi128(bitboard::Bitboard GeneratorBB,
                     bitboard::Bitboard PropagateBB) noexcept {
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftSi128<Shift>();
    PropagateBB &= PropagateBB.getRightShiftSi128<Shift>();
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftSi128<2 * Shift>();
    PropagateBB &= PropagateBB.getRightShiftSi128<2 * Shift>();
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftSi128<4 * Shift>();
    return GeneratorBB;
}

template <int Shift>
inline bitboard::Bitboard
koggeStoneLeftSi128(bitboard::Bitboard GeneratorBB,
                    bitboard::Bitboard PropagateBB) noexcept {
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftSi128<Shift>();
    PropagateBB &= PropagateBB.getLeftShiftSi128<Shift>();
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftSi128<2 * Shift>();
    PropagateBB &= PropagateBB.getLeftShiftSi128<2 * Shift>();
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftSi128<4 * Shift>();
    return GeneratorBB;
}

inline bitboard::Bitboard
koggeStoneRightEpi64(bitboard::Bitboard GeneratorBB,
                     bitboard::Bitboard PropagateBB) noexcept {
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftEpi64<1>();
    PropagateBB &= PropagateBB.getRightShiftEpi64<1>();
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftEpi64<2>();
    PropagateBB &= PropagateBB.getRightShiftEpi64<2>();
    GeneratorBB |= PropagateBB & GeneratorBB.getRightShiftEpi64<4>();
    return GeneratorBB;
}

inline bitboard::Bitboard
koggeStoneLeftEpi64(bitboard::Bitboard GeneratorBB,
                    bitboard::Bitboard PropagateBB) noexcept {
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftEpi64<1>();
    PropagateBB &= PropagateBB.getLeftShiftEpi64<1>();
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftEpi64<2>();
    PropagateBB &= PropagateBB.getLeftShiftEpi64<2>();
    GeneratorBB |= PropagateBB & GeneratorBB.getLeftShiftEpi64<4>();
    return GeneratorBB;
}

#if defined(USE_AVX2)

// AVX2 variants that flood two directions at once, one direction per
// 128-bit half of a 256-bit register. `ShiftN`/`FunnelN` describe the
// direction placed in half N: the shift width in bits, and whether the
// shift funnels over the 63/18-bit lane border of a bitboard (see
// getRightShiftSi128(); the one-bit rank-wise step never leaves its
// lane, so it must not funnel). A funnel-free half kills the carry with
// a saturating shift count of 64.

template <int Shift0, bool Funnel0, int Shift1, bool Funnel1>
inline __attribute__((always_inline)) __m256i
getRightShiftPair(__m256i X) noexcept {
    const __m256i MainCounts =
        _mm256_set_epi64x(Shift1, Shift1, Shift0, Shift0);
    const __m256i CarryCounts = _mm256_set_epi64x(
        Funnel1 ? 63 - Shift1 : 64, Funnel1 ? 63 - Shift1 : 64,
        Funnel0 ? 63 - Shift0 : 64, Funnel0 ? 63 - Shift0 : 64);
    return _mm256_or_si256(
        _mm256_srlv_epi64(X, MainCounts),
        _mm256_sllv_epi64(_mm256_srli_si256(X, 8), CarryCounts));
}

template <int Shift0, bool Funnel0, int Shift1, bool Funnel1>
inline __attribute__((always_inline)) __m256i
getLeftShiftPair(__m256i X) noexcept {
    const __m256i MainCounts =
        _mm256_set_epi64x(Shift1, Shift1, Shift0, Shift0);
    const __m256i CarryCounts = _mm256_set_epi64x(
        Funnel1 ? 63 - Shift1 : 64, Funnel1 ? 63 - Shift1 : 64,
        Funnel0 ? 63 - Shift0 : 64, Funnel0 ? 63 - Shift0 : 64);
    return _mm256_or_si256(
        _mm256_sllv_epi64(X, MainCounts),
        _mm256_srlv_epi64(_mm256_slli_si256(X, 8), CarryCounts));
}

// The paired counterparts of the koggeStone*() helpers above. Unlike
// them, these shift the flood one more step and return the attacks.

template <int Shift0, bool Funnel0, int Shift1, bool Funnel1>
inline __attribute__((always_inline)) __m256i
koggeStoneRightPairAttack(__m256i GeneratorBB, __m256i PropagateBB) noexcept {
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(
            PropagateBB,
            getRightShiftPair<Shift0, Funnel0, Shift1, Funnel1>(GeneratorBB)));
    PropagateBB = _mm256_and_si256(
        PropagateBB,
        getRightShiftPair<Shift0, Funnel0, Shift1, Funnel1>(PropagateBB));
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(PropagateBB,
                         getRightShiftPair<2 * Shift0, Funnel0, 2 * Shift1,
                                           Funnel1>(GeneratorBB)));
    PropagateBB = _mm256_and_si256(
        PropagateBB,
        getRightShiftPair<2 * Shift0, Funnel0, 2 * Shift1, Funnel1>(
            PropagateBB));
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(PropagateBB,
                         getRightShiftPair<4 * Shift0, Funnel0, 4 * Shift1,
                                           Funnel1>(GeneratorBB)));
    return getRightShiftPair<Shift0, Funnel0, Shift1, Funnel1>(GeneratorBB);
}

template <int Shift0, bool Funnel0, int Shift1, bool Funnel1>
inline __attribute__((always_inline)) __m256i
koggeStoneLeftPairAttack(__m256i GeneratorBB, __m256i PropagateBB) noexcept {
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(
            PropagateBB,
            getLeftShiftPair<Shift0, Funnel0, Shift1, Funnel1>(GeneratorBB)));
    PropagateBB = _mm256_and_si256(
        PropagateBB,
        getLeftShiftPair<Shift0, Funnel0, Shift1, Funnel1>(PropagateBB));
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(PropagateBB,
                         getLeftShiftPair<2 * Shift0, Funnel0, 2 * Shift1,
                                          Funnel1>(GeneratorBB)));
    PropagateBB = _mm256_and_si256(
        PropagateBB,
        getLeftShiftPair<2 * Shift0, Funnel0, 2 * Shift1, Funnel1>(
            PropagateBB));
    GeneratorBB = _mm256_or_si256(
        GeneratorBB,
        _mm256_and_si256(PropagateBB,
                         getLeftShiftPair<4 * Shift0, Funnel0, 4 * Shift1,
                                          Funnel1>(GeneratorBB)));
    return getLeftShiftPair<Shift0, Funnel0, Shift1, Funnel1>(GeneratorBB);
}

#endif

class StateImpl {
 public:
    StateImpl() = delete;
    StateImpl(const StateImpl&) = delete;
    StateImpl(StateImpl&&) noexcept;
    StateImpl(const Position& P);
    StateImpl(const Position& P, uint16_t Ply);
    StateImpl(const Position& CurrentP, const Position& InitP);

    StateImpl& operator=(const StateImpl&) = delete;
    StateImpl& operator=(StateImpl&&) = delete;

    ~StateImpl() = default;

    StateImpl clone() const;

    // Getters.
    inline constexpr const Position& getPosition() const noexcept {
        return Pos;
    }

    template <Color C>
    inline constexpr const bitboard::Bitboard getBitboard() const noexcept {
        return Helper.ColorBB[C];
    }

    template <PieceTypeKind Type>
    inline constexpr const bitboard::Bitboard getBitboard() const noexcept {
        return Helper.TypeBB[Type];
    }

    template <Color C, PieceTypeKind Type>
    inline constexpr bitboard::Bitboard getBitboard() const noexcept {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    inline const bitboard::Bitboard getBitboard(Color C) const noexcept {
        return Helper.ColorBB[C];
    }

    inline bitboard::Bitboard getBitboard(Color C,
                                          PieceTypeKind Type) const noexcept {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    template <PieceTypeKind Type>
    inline bitboard::Bitboard getBitboard(Color C) const noexcept {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    template <Color C>
    inline bitboard::Bitboard getBitboard(PieceTypeKind Type) const noexcept {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    inline bitboard::Bitboard getBitboard(PieceTypeKind Type) const noexcept {
        return Helper.TypeBB[Type];
    }

    inline const bitboard::Bitboard getCheckerBB() const noexcept {
        return Helper.getCurrentStepHelper().CheckerBB;
    }

    inline uint64_t getBoardHash() const noexcept {
        return HashValue.getValue();
    }

    inline uint64_t getHash() const noexcept {
        return HashValue.getValue() ^
               ((uint64_t)(getPosition().getStand<Black>()) << 33) ^
               ((uint64_t)(getPosition().getStand<White>()) << 1);
    }

    const Position& getInitialPosition() const noexcept {
        return Helper.getInitialPosition();
    }

    uint16_t getPly(bool IncludeOffset = true) const noexcept {
        return Helper.getPly(IncludeOffset);
    }

    Move32 getHistoryMove(uint16_t Ply) const {
        return Helper.getStepHelper(Ply).Move;
    }

    Move32 getLastMove() const {
        return getHistoryMove(getPly(false) - 1);
    }

    // Manipulations.
    template <Color C>
    void doMove(Move32 Move) noexcept;
    void doMove(Move32 Move) noexcept;

    template <Color C>
    void undoMove();
    void undoMove();

    // Re-compute "second" variables, where "second" means
    // any variables that can be computed by "first" variables
    // that are neccesary to stand for the game representation.
    void refresh() noexcept;

    // Helper functions.

    template <bool Strict = false>
    inline RepetitionStatus getRepetitionStatus() const noexcept {
        const Color SideToMove = getPosition().sideToMove();
        const StepHelper& CurrentStepHelper = Helper.getCurrentStepHelper();

        const Stands MyStand = getPosition().getStand(SideToMove);
        const Stands OpStand = getPosition().getStand(~SideToMove);

        uint16_t RepetitionCount = 0;
        for (int Ply = Helper.Ply - 4; Ply >= 0; Ply -= 2) {
            const StepHelper& SHelper = Helper.getStepHelper((uint16_t)Ply);
            const Stands MyStepStand = SHelper.EachStand[SideToMove];
            const Stands OpStepStand = SHelper.EachStand[~SideToMove];

            if (HashValue.getValue() == SHelper.BoardHash) {
                if (MyStand == MyStepStand && OpStand == OpStepStand) {
                    if constexpr (Strict) {
                        if (CurrentStepHelper
                                    .ContinuousCheckCounts[~SideToMove] >= 6 &&
                            CurrentStepHelper
                                        .ContinuousCheckCounts[~SideToMove] *
                                    2 >=
                                Helper.Ply - Ply) {
                            return RepetitionStatus::WinRepetition;
                        }
                    } else {
                        if (CurrentStepHelper
                                    .ContinuousCheckCounts[~SideToMove] *
                                2 >=
                            Helper.Ply - Ply) {
                            return RepetitionStatus::WinRepetition;
                        }
                    }

                    if constexpr (Strict) {
                        if (CurrentStepHelper
                                    .ContinuousCheckCounts[SideToMove] >= 6 &&
                            CurrentStepHelper
                                        .ContinuousCheckCounts[SideToMove] *
                                    2 >=
                                Helper.Ply - Ply) {
                            return RepetitionStatus::LossRepetition;
                        }
                    } else {
                        if (CurrentStepHelper
                                    .ContinuousCheckCounts[SideToMove] *
                                2 >=
                            Helper.Ply - Ply) {
                            return RepetitionStatus::LossRepetition;
                        }
                    }

                    if constexpr (Strict) {
                        if (RepetitionCount == 2) {
                            return RepetitionStatus::Repetition;
                        }
                        ++RepetitionCount;
                        Ply -= 2;
                    } else {
                        return RepetitionStatus::Repetition;
                    }
                } else {
                    if (isSuperiorOrEqual(MyStand, MyStepStand)) {
                        return RepetitionStatus::SuperiorRepetition;
                    }

                    if (isSuperiorOrEqual(MyStepStand, MyStand)) {
                        return RepetitionStatus::InferiorRepetition;
                    }
                }
            }
        }

        return RepetitionStatus::NoRepetition;
    }

    inline constexpr Color getSideToMove() const noexcept {
        return getPosition().sideToMove();
    }

    template <Color C, PieceTypeKind Type>
    uint8_t getStandCount() const noexcept {
        return getPosition().getStandCount<C, Type>();
    }

    template <Color C>
    uint8_t getStandCount(PieceTypeKind Type) const noexcept {
        return getPosition().getStandCount<C>(Type);
    }

    uint8_t getStandCount(Color C, PieceTypeKind Type) const noexcept {
        return getPosition().getStandCount(C, Type);
    }

    template <Color C>
    inline constexpr Square getKingSquare() const noexcept {
        return Helper.KingSquare[C];
    }

    inline constexpr Square getKingSquare(Color C) const noexcept {
        return Helper.KingSquare[C];
    }

    Move32 getMove32FromMove16(Move16 M16) const noexcept {
        if (M16.drop()) {
            const PieceTypeKind Type =
                (PieceTypeKind)(M16.from() - NumSquares + 1);
            return Move32::droppingMove(M16.to(), Type);
        }

        const Square From = M16.from();
        const Square To = M16.to();
        const PieceTypeKind Type = getPieceType(getPosition().pieceOn(From));
        const PieceTypeKind CaptureType =
            getPieceType(getPosition().pieceOn(To));

        if (M16.promote()) {
            return Move32::boardPromotingMove(From, To, Type, CaptureType);
        }

        return Move32::boardMove(From, To, Type, CaptureType);
    }

    template <Color C>
    uint8_t computePieceScore(uint8_t SliderScoreUnit, uint8_t StepScoreUnit,
                              bool OnlyInPromotableZone) const noexcept {
        uint8_t SliderScore = 0;
        uint8_t StepScore = 0;

        bitboard::Bitboard SlideresBB =
            (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>() |
             getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()) &
            getBitboard<C>();

        bitboard::Bitboard StepsBB =
            getBitboard<C>() ^ getBitboard<C, PTK_King>() ^ SlideresBB;

        if (OnlyInPromotableZone) {
            SlideresBB &= bitboard::PromotableBB[C];
            StepsBB &= bitboard::PromotableBB[C];
        }

        SliderScore = SlideresBB.popCount();
        StepScore = StepsBB.popCount();

        const Stands St = getPosition().getStand<C>();

        StepScore += (uint8_t)(core::getStandCount<PTK_Pawn>(St) +
                               core::getStandCount<PTK_Lance>(St) +
                               core::getStandCount<PTK_Knight>(St) +
                               core::getStandCount<PTK_Silver>(St) +
                               core::getStandCount<PTK_Gold>(St));

        SliderScore += (uint8_t)(core::getStandCount<PTK_Bishop>(St) +
                                 core::getStandCount<PTK_Rook>(St));
        return (uint8_t)(SliderScore * SliderScoreUnit +
                         StepScore * StepScoreUnit);
    }

    template <Color C>
    inline constexpr uint8_t
    computeDeclarationScore(uint8_t SliderScoreUnit = 5,
                            uint8_t StepScoreUnit = 1) const noexcept {
        return computePieceScore<C>(SliderScoreUnit, StepScoreUnit, true);
    }

    bool canDeclare() const noexcept;

    template <Color C>
    inline constexpr const bitboard::Bitboard
    getDefendingOpponentSliderBB() const noexcept {
        return Helper.getCurrentStepHelper().DefendingOpponentSliderBB[C];
    }

    inline const bitboard::Bitboard
    getDefendingOpponentSliderBB(Color C) const noexcept {
        return Helper.getCurrentStepHelper().DefendingOpponentSliderBB[C];
    }

    template <Color C>
    inline bitboard::Bitboard
    getStepAttackBB(Square ExcludeSq = SqInvalid) const noexcept {
        bitboard::Bitboard StepAttackBB = bitboard::Bitboard::ZeroBB();

        // North attacks.
        bitboard::Bitboard NorthAttackCandidateBB =
            getBitboard<C>() &
            ((C == Black)
                 ? (getBitboard<PTK_Pawn>() | getBitboard<PTK_Silver>() |
                    getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() | getBitboard<PTK_ProBishop>())

                 : (getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() |
                    getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            NorthAttackCandidateBB =
                bitboard::SquareBB[ExcludeSq].andNot(NorthAttackCandidateBB);
        }

        StepAttackBB |= bitboard::RankBB[RankA]
                            .andNot(NorthAttackCandidateBB)
                            .getLeftShiftEpi64<1>();

        // NorthEast attacks and NorthWest attacks.
        bitboard::Bitboard NorthEastAndNorthWestAttackCandidateBB =
            getBitboard<C>() &
            ((C == Black)
                 ? (getBitboard<PTK_Silver>() | getBitboard<PTK_Gold>() |
                    getBitboard<PTK_King>() | getBitboard<PTK_ProPawn>() |
                    getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() | getBitboard<PTK_ProRook>())

                 : (getBitboard<PTK_King>() | getBitboard<PTK_Silver>() |
                    getBitboard<PTK_ProRook>()));

        if (ExcludeSq != SqInvalid) {
            NorthEastAndNorthWestAttackCandidateBB =
                bitboard::SquareBB[ExcludeSq].andNot(
                    NorthEastAndNorthWestAttackCandidateBB);
        }

        StepAttackBB |=
            (bitboard::FileBB[File1].andNot(bitboard::RankBB[RankA].andNot(
                 NorthEastAndNorthWestAttackCandidateBB)))
                .getRightShiftSi128<8>();
        StepAttackBB |=
            (bitboard::FileBB[File9].andNot(bitboard::RankBB[RankA].andNot(
                 NorthEastAndNorthWestAttackCandidateBB)))
                .getLeftShiftSi128<10>();

        // East attacks and West attacks.
        bitboard::Bitboard EastAndWestAttackCandidateBB =
            getBitboard<C>() &
            ((C == Black)
                 ? (getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() | getBitboard<PTK_ProBishop>())

                 : (getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() |
                    getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            EastAndWestAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(
                EastAndWestAttackCandidateBB);
        }

        StepAttackBB |=
            (bitboard::FileBB[File1].andNot(EastAndWestAttackCandidateBB))
                .getRightShiftSi128<9>();
        StepAttackBB |=
            (bitboard::FileBB[File9].andNot(EastAndWestAttackCandidateBB))
                .getLeftShiftSi128<9>();

        // SouthEast attacks and SouthWest attacks.
        bitboard::Bitboard SouthEastAndSouthWestAttackCandidateBB =
            getBitboard<C>() &
            ((C == Black)
                 ? (getBitboard<PTK_King>() | getBitboard<PTK_Silver>() |
                    getBitboard<PTK_ProRook>())

                 : (getBitboard<PTK_Silver>() | getBitboard<PTK_Gold>() |
                    getBitboard<PTK_King>() | getBitboard<PTK_ProPawn>() |
                    getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() | getBitboard<PTK_ProRook>()));

        if (ExcludeSq != SqInvalid) {
            SouthEastAndSouthWestAttackCandidateBB =
                bitboard::SquareBB[ExcludeSq].andNot(
                    SouthEastAndSouthWestAttackCandidateBB);
        }

        StepAttackBB |=
            (bitboard::FileBB[File1].andNot(bitboard::RankBB[RankI].andNot(
                 SouthEastAndSouthWestAttackCandidateBB)))
                .getRightShiftSi128<10>();
        StepAttackBB |=
            (bitboard::FileBB[File9].andNot(bitboard::RankBB[RankI].andNot(
                 SouthEastAndSouthWestAttackCandidateBB)))
                .getLeftShiftSi128<8>();

        // South attacks.
        bitboard::Bitboard SouthAttackCandidateBB =
            getBitboard<C>() &
            ((C == Black)
                 ? (getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() | getBitboard<PTK_ProBishop>())

                 : (getBitboard<PTK_Pawn>() | getBitboard<PTK_Silver>() |
                    getBitboard<PTK_Gold>() | getBitboard<PTK_King>() |
                    getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
                    getBitboard<PTK_ProKnight>() |
                    getBitboard<PTK_ProSilver>() |
                    getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            SouthAttackCandidateBB =
                bitboard::SquareBB[ExcludeSq].andNot(SouthAttackCandidateBB);
        }

        StepAttackBB |= (bitboard::RankBB[RankI].andNot(SouthAttackCandidateBB))
                            .getRightShiftEpi64<1>();

        // Knight attacks.
        bitboard::Bitboard KnightBB = getBitboard<C, PTK_Knight>();

        if (ExcludeSq != SqInvalid) {
            KnightBB = bitboard::SquareBB[ExcludeSq].andNot(KnightBB);
        }

        if constexpr (C == Black) {
            StepAttackBB |=
                (bitboard::FileBB[File1].andNot(
                     bitboard::FirstAndSecondFurthestBB[C].andNot(KnightBB)))
                    .getRightShiftSi128<7>();
            StepAttackBB |=
                (bitboard::FileBB[File9].andNot(
                     bitboard::FirstAndSecondFurthestBB[C].andNot(KnightBB)))
                    .getLeftShiftSi128<11>();
        } else {
            StepAttackBB |=
                (bitboard::FileBB[File1].andNot(
                     bitboard::FirstAndSecondFurthestBB[White].andNot(
                         KnightBB)))
                    .getRightShiftSi128<11>();
            StepAttackBB |=
                (bitboard::FileBB[File9].andNot(
                     bitboard::FirstAndSecondFurthestBB[White].andNot(
                         KnightBB)))
                    .getLeftShiftSi128<7>();
        }

        return StepAttackBB;
    }

    /// Compute Sliders' attacks by the Kogge-Stone algorithm.
    template <Color C>
    bitboard::Bitboard
    getSliderAttackBB(Square ExcludeSq = SqInvalid,
                      Square VirtualSq = SqInvalid) const noexcept {
        const bitboard::Bitboard OccupiedBB =
            (VirtualSq == SqInvalid)
                ? (getBitboard<Black>() | getBitboard<White>())
                : (getBitboard<Black>() | getBitboard<White>() |
                   bitboard::SquareBB[VirtualSq]);

        const bitboard::Bitboard EmptyBB =
            (ExcludeSq == SqInvalid)
                ? ~OccupiedBB
                : (~OccupiedBB | bitboard::SquareBB[ExcludeSq]);

#if defined(USE_AVX2)
        __m256i AttackPairBB = _mm256_setzero_si256();
#else
        bitboard::Bitboard SliderAttackBB = bitboard::Bitboard::ZeroBB();
#endif

        // Bishops.
        const bitboard::Bitboard BishopBB =
            getBitboard<C>() &
            ((ExcludeSq == SqInvalid)
                 ? (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>())
                 : (bitboard::SquareBB[ExcludeSq].andNot(
                       getBitboard<PTK_Bishop>() |
                       getBitboard<PTK_ProBishop>())));

        const bitboard::Bitboard NotRankAAndEmptyBB =
            bitboard::RankBB[RankA].andNot(EmptyBB);
        const bitboard::Bitboard NotRankIAndEmptyBB =
            bitboard::RankBB[RankI].andNot(EmptyBB);

        if (!BishopBB.isZero()) {
            // The generators are pre-masked by the edge rank of each
            // direction so that no shift can wrap into a neighboring file.
            const bitboard::Bitboard NoRankABishopBB =
                bitboard::RankBB[RankA].andNot(BishopBB);
            const bitboard::Bitboard NoRankIBishopBB =
                bitboard::RankBB[RankI].andNot(BishopBB);

#if defined(USE_AVX2)
            // NorthEast (low half) and SouthEast (high half).
            AttackPairBB = _mm256_or_si256(
                AttackPairBB,
                koggeStoneRightPairAttack<8, true, 10, true>(
                    _mm256_set_m128i(NoRankIBishopBB.getRaw(),
                                     NoRankABishopBB.getRaw()),
                    _mm256_set_m128i(NotRankIAndEmptyBB.getRaw(),
                                     NotRankAAndEmptyBB.getRaw())));
            // SouthWest (low half) and NorthWest (high half).
            AttackPairBB = _mm256_or_si256(
                AttackPairBB,
                koggeStoneLeftPairAttack<8, true, 10, true>(
                    _mm256_set_m128i(NoRankABishopBB.getRaw(),
                                     NoRankIBishopBB.getRaw()),
                    _mm256_set_m128i(NotRankAAndEmptyBB.getRaw(),
                                     NotRankIAndEmptyBB.getRaw())));
#else
            // NorthEast.
            SliderAttackBB |=
                koggeStoneRightSi128<8>(NoRankABishopBB, NotRankAAndEmptyBB)
                    .getRightShiftSi128<8>();
            // SouthEast.
            SliderAttackBB |=
                koggeStoneRightSi128<10>(NoRankIBishopBB, NotRankIAndEmptyBB)
                    .getRightShiftSi128<10>();
            // SouthWest.
            SliderAttackBB |=
                koggeStoneLeftSi128<8>(NoRankIBishopBB, NotRankIAndEmptyBB)
                    .getLeftShiftSi128<8>();
            // NorthWest.
            SliderAttackBB |=
                koggeStoneLeftSi128<10>(NoRankABishopBB, NotRankAAndEmptyBB)
                    .getLeftShiftSi128<10>();
#endif
        }

        const bitboard::Bitboard RookBB =
            getBitboard<C>() &
            ((ExcludeSq == SqInvalid)
                 ? (getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())
                 : (bitboard::SquareBB[ExcludeSq].andNot(
                       getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())));

        const bitboard::Bitboard LanceBB =
            getBitboard<C>() &
            ((ExcludeSq == SqInvalid) ? getBitboard<PTK_Lance>()
                                      : (bitboard::SquareBB[ExcludeSq].andNot(
                                            getBitboard<PTK_Lance>())));

        const bitboard::Bitboard ForwardBB =
            (C == Black) ? (RookBB | LanceBB) : RookBB;
        const bitboard::Bitboard BackwardBB =
            (C == White) ? (RookBB | LanceBB) : RookBB;

#if defined(USE_AVX2)
        // ForwardBB and BackwardBB always contain RookBB, so one gate
        // covers both halves of each pair. A rank has no edge rank to wrap
        // over, so the east and west generators need no pre-mask.
        if (!BackwardBB.isZero()) {
            // East (low half) and South (high half).
            AttackPairBB = _mm256_or_si256(
                AttackPairBB,
                koggeStoneRightPairAttack<9, true, 1, false>(
                    _mm256_set_m128i(
                        bitboard::RankBB[RankI].andNot(BackwardBB).getRaw(),
                        RookBB.getRaw()),
                    _mm256_set_m128i(NotRankIAndEmptyBB.getRaw(),
                                     EmptyBB.getRaw())));
        }

        if (!ForwardBB.isZero()) {
            // West (low half) and North (high half).
            AttackPairBB = _mm256_or_si256(
                AttackPairBB,
                koggeStoneLeftPairAttack<9, true, 1, false>(
                    _mm256_set_m128i(
                        bitboard::RankBB[RankA].andNot(ForwardBB).getRaw(),
                        RookBB.getRaw()),
                    _mm256_set_m128i(NotRankAAndEmptyBB.getRaw(),
                                     EmptyBB.getRaw())));
        }

        const bitboard::Bitboard SliderAttackBB(
            _mm_or_si128(_mm256_castsi256_si128(AttackPairBB),
                         _mm256_extracti128_si256(AttackPairBB, 1)));
#else
        if (!ForwardBB.isZero()) {
            // North.
            SliderAttackBB |=
                koggeStoneLeftEpi64(bitboard::RankBB[RankA].andNot(ForwardBB),
                                    NotRankAAndEmptyBB)
                    .getLeftShiftEpi64<1>();
        }

        if (!BackwardBB.isZero()) {
            // South.
            SliderAttackBB |=
                koggeStoneRightEpi64(bitboard::RankBB[RankI].andNot(BackwardBB),
                                     NotRankIAndEmptyBB)
                    .getRightShiftEpi64<1>();
        }

        if (!RookBB.isZero()) {
            // East and West. A rank has no edge rank to wrap over, so the
            // generators need no pre-mask.
            SliderAttackBB |= koggeStoneRightSi128<9>(RookBB, EmptyBB)
                                  .getRightShiftSi128<9>();
            SliderAttackBB |=
                koggeStoneLeftSi128<9>(RookBB, EmptyBB).getLeftShiftSi128<9>();
        }
#endif

        // The final one-step shifts may spill outside the board (the spare
        // low-lane bit and the bits above the last square), so trim the
        // result to the board.
        return SliderAttackBB & bitboard::Bitboard::AllBB();
    }

    template <Color C>
    inline bitboard::Bitboard
    getAttackBB(Square ExcludeSq = SqInvalid) const noexcept {
        return getStepAttackBB<C>(ExcludeSq) | getSliderAttackBB<C>(ExcludeSq);
    }

    template <Color C>
    inline bool isAttackedByOneStep(Square Sq) const noexcept {
        const bitboard::Bitboard UnderAttackedBB =
            (bitboard::getAttackBB<C, PTK_Pawn>(Sq) & getBitboard<PTK_Pawn>()) |
            (bitboard::getAttackBB<C, PTK_Knight>(Sq) &
             getBitboard<PTK_Knight>()) |
            (bitboard::getAttackBB<C, PTK_Silver>(Sq) &
             getBitboard<PTK_Silver>()) |
            (bitboard::getAttackBB<C, PTK_King>(Sq) &
             (getBitboard<PTK_King>() | getBitboard<PTK_ProBishop>() |
              getBitboard<PTK_ProRook>())) |
            (bitboard::getAttackBB<C, PTK_Gold>(Sq) &
             (getBitboard<PTK_Gold>() | getBitboard<PTK_ProPawn>() |
              getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
              getBitboard<PTK_ProSilver>()));

        return !(UnderAttackedBB & getBitboard<~C>()).isZero();
    }

    template <Color C, PieceTypeKind Type>
    inline bool
    isAttackedBySlider(Square Sq, const bitboard::Bitboard& OccupiedBB,
                       Square VirtualSq = SqInvalid) const noexcept {
        static_assert(Type == PTK_Lance || Type == PTK_Bishop ||
                          Type == PTK_ProBishop || Type == PTK_Rook ||
                          Type == PTK_ProRook,
                      "possibly caused by calling isAttackedBySlider() with a "
                      "template parameter "
                      "that must not be passed in.");

        if constexpr (Type == PTK_Lance) {
            return (getBitboard<~C, PTK_Lance>() &
                    bitboard::getForwardBB<C>(Sq))
                .any([&](Square LanceSq) {
                    if (VirtualSq != SqInvalid && LanceSq == VirtualSq) {
                        return false;
                    }
                    const bitboard::Bitboard BetweenOccupiedBB =
                        bitboard::getBetweenBB(Sq, LanceSq) & OccupiedBB;

                    return BetweenOccupiedBB.isZero();
                });
        } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
            if (((getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>()) &
                 getBitboard<~C>() & bitboard::DiagBB[Sq])
                    .any([&](Square BishopSq) {
                        if (VirtualSq != SqInvalid && BishopSq == VirtualSq) {
                            return false;
                        }
                        const bitboard::Bitboard BetweenOccupiedBB =
                            bitboard::getBetweenBB(Sq, BishopSq) & OccupiedBB;

                        return BetweenOccupiedBB.isZero();
                    })) {
                return true;
            }

            return false;
        } else if constexpr (Type == PTK_Rook || Type == PTK_ProRook) {
            if (((getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()) &
                 getBitboard<~C>() & bitboard::CrossBB[Sq])
                    .any([&](Square RookSq) {
                        if (VirtualSq != SqInvalid && RookSq == VirtualSq) {
                            return false;
                        }
                        const bitboard::Bitboard BetweenOccupiedBB =
                            bitboard::getBetweenBB(Sq, RookSq) & OccupiedBB;

                        return BetweenOccupiedBB.isZero();
                    })) {
                return true;
            }

            return false;
        } else {
            // Should not be reached.
            []<bool Flag = false>() {
                static_assert(Flag, "Unhandled PieceTypeKind.");
            }();
            return false;
        }
    }

    template <Color C>
    inline bool
    isAttackedBySlider(Square Sq, const bitboard::Bitboard& OccupiedBB,
                       Square ExcludeSq = SqInvalid,
                       Square VirtualSq = SqInvalid) const noexcept {
        bitboard::Bitboard OccupiedBB2 = OccupiedBB;
        if (ExcludeSq != SqInvalid) {
            assert(OccupiedBB.isSet(ExcludeSq));
            OccupiedBB2.toggleBit(ExcludeSq);
        }
        if (VirtualSq != SqInvalid) {
            OccupiedBB2 |= bitboard::SquareBB[VirtualSq];
        }
        return isAttackedBySlider<C, PTK_Bishop>(Sq, OccupiedBB2, VirtualSq) ||
               isAttackedBySlider<C, PTK_Rook>(Sq, OccupiedBB2, VirtualSq) ||
               isAttackedBySlider<C, PTK_Lance>(Sq, OccupiedBB2, VirtualSq);
    }

    template <Color C>
    inline bool isAttacked(Square Sq, Square ExcludeSq = SqInvalid,
                           Square VirtualSq = SqInvalid) const noexcept {
        if (isAttackedByOneStep<C>(Sq)) {
            return true;
        }

        const bitboard::Bitboard OccupiedBB =
            getBitboard<Black>() | getBitboard<White>();
        return isAttackedBySlider<C>(Sq, OccupiedBB, ExcludeSq, VirtualSq);
    }

    bool isLastMoveDroppingAPawn() const noexcept {
        return Helper.Ply > 0 && Helper.SHelper.back().IsLastMoveDroppingAPawn;
    }

    template <core::Color C>
    bool isLegal(Move16 Move) const noexcept;

    template <core::Color C>
    bool isLegal(Move32 Move) const noexcept;

    //  The functions below are for `ExtendedState` class,
    //  which means that they are not necessary for the game representation but
    //  are possibly useful.

    void doNullMove() noexcept;

    void undoNullMove();

    int32_t computeSEE(const Move32 Move,
                       const int32_t* const PieceValues) const noexcept;

    bool computeSEEGE(const Move32 Move, const int32_t* const PieceValues,
                      int32_t Threshold) const noexcept;

 protected:
    Position Pos;
    StateHelper Helper;
    Hash<uint64_t> HashValue;

 private:
    template <Color C, bool UpdateCheckerBySliders>
    void setDefendingOpponentSliderBBAndSliderCheckerBB(
        StepHelper* SHelper, const bitboard::Bitboard& OccupiedBB) noexcept;

    template <Color C>
    void setStepCheckerBB(StepHelper* SHelper) noexcept;

    template <bool GEMode>
    int32_t computeSEEImpl(const Move32 Move, const int32_t* const PieceValues,
                           int32_t Threshold) const noexcept;

    bitboard::Bitboard
    computeSEEAttackersBB(Square To, const bitboard::Bitboard* BBs,
                          const bitboard::Bitboard& OccupiedBB) const noexcept;

    void
    updateSEEAttackersBB(bitboard::Bitboard* AttackersBB, Square To,
                         Square FromSq,
                         const bitboard::Bitboard& OccupiedBB) const noexcept;

    bool seeGivesDiscoveredCheck(Color C, Square FromSq,
                                 const bitboard::Bitboard& MyBB,
                                 const StepHelper* SHelper) const noexcept;
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_STATEIMPL_H
