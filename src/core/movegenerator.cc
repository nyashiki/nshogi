#include "movegenerator.h"
#include "../io/bitboard.h"
#include "bitboard.h"
#include "movelist.h"
#include "position.h"
#include "types.h"

#include "../io/sfen.h"

namespace nshogi {
namespace core {

namespace {

using namespace bitboard;

enum struct GenerateType {
    Normal, Check, Evasion
};

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardOneStepPawnMovesImpl(const State& S, Move32* Moves,
                                               const Bitboard& TargetSquares, const Bitboard& OccupiedBB) {
    Bitboard ToBB = S.getBitboard<C, PTK_Pawn>();

    if constexpr (C == Black) {
        ToBB = ToBB.getLeftShiftEpi64<1>();
    } else {
        ToBB = ToBB.getRightShiftEpi64<1>();
    }

    ToBB &= TargetSquares;

    ToBB.forEach([&](Square To) {
        const Square From = (C == Black) ? (To + South) : (To + North);

        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);
        if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
            return;
        }

        if (PromotableBB[C].isSet(To)) {
            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(
                    From, To, PTK_Pawn, CaptureType);
            } else {
                *Moves++ =
                    Move32::boardPromotingMove(From, To, PTK_Pawn);
            }

            if (WilyPromote) {
                return;
            }
        }

        // A move that can cause no-future-move is prohibitted by the rule.
        if (!FurthermostBB[C].isSet(To)) {
            if constexpr (C == Black) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(To + South, To, PTK_Pawn,
                                                 CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(To + South, To, PTK_Pawn);
                }
            } else {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(To + North, To, PTK_Pawn,
                                                 CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(To + North, To, PTK_Pawn);
                }
            }
        }
    });

    return Moves;
}

template <Color C, bool Capture>
inline Move32* generateOnBoardOneStepGoldKindsMovesImpl(const State& S, Move32* Moves, const Bitboard& TargetSquares, const Bitboard& OccupiedBB) {
    const Bitboard FromBB = (S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>()
                        | S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>()
                        | S.getBitboard<PTK_ProSilver>()) & S.getBitboard<C>();

    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        const PieceTypeKind FromPieceType = getPieceType(S.getPosition().pieceOn(From));

        const Bitboard ToBB = TargetSquares & getAttackBB<C, PTK_Gold>(From);
        ToBB.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardMove(From, To, FromPieceType, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, FromPieceType);
            }
        });
    });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture>
inline Move32* generateOnBoardOneStepMovesImpl(const State& S, Move32* Moves,
                                           const Bitboard& TargetSquares, const Bitboard& OccupiedBB) {
    static_assert(Type != PTK_Pawn,
                  "Pawn moves are handled in another function. "
                  "Do not use this function for pawns.");
    static_assert(
        Type != PTK_Bishop,
        "Bishop moves are handled in another function. Do not use this "
        "function for bishops.");
    static_assert(Type != PTK_Rook,
                  "Rook moves are handled in another function. "
                  "Do not use this function for rooks.");

    // Extracting the bitboard for pieces of specified color and type.
    const Bitboard FromBB = S.getBitboard<C, Type>();

    // Loop through each piece of the specified type.
    FromBB.forEach([&](Square From) {
        const bool IsPromotableFrom = Type != PTK_Pawn && Type != PTK_Knight &&
                                      PromotableBB[C].isSet(From);
        assert(checkRange(From));

        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        const Bitboard ToBB = TargetSquares & getAttackBB<C, Type>(From);
        ToBB.forEach([&](Square To) {
            if constexpr (Type == PTK_King) {
                if (S.isAttacked<C>(To, From)) {
                    return;
                }
            }
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            // Handling regular promotion cases for all piece types.
            if constexpr (Type != PTK_King) {
                if constexpr (Type == PTK_Knight) {
                    // The above pieces cannot go back so forward
                    // promotion check is sufficient enough.
                    if (PromotableBB[C].isSet(To)) {
                        // If the move starts or ends on a promotable square,
                        // promote the piece.
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ = Move32::boardPromotingMove(
                                From, To, Type, CaptureType);
                        } else {
                            *Moves++ =
                                Move32::boardPromotingMove(From, To, Type);
                        }
                    }
                } else if constexpr (!isPromoted(Type) && Type != PTK_Gold) {
                    if (IsPromotableFrom || PromotableBB[C].isSet(To)) {
                        // If the move starts or ends on a promotable square,
                        // promote the piece.
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ = Move32::boardPromotingMove(
                                From, To, Type, CaptureType);
                        } else {
                            *Moves++ =
                                Move32::boardPromotingMove(From, To, Type);
                        }
                    }
                }
            }

            // Add a non-promoting move.
            // Note: be careful when move pawn or knight which can cause
            //      no-possible-moves that is prohibitted by the rule.
            //      However pawn move generation is done in another function
            //      so process the case of only knight here.
            if (!(Type == PTK_Knight &&
                  FirstAndSecondFurthestBB[C].isSet(To))) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            }
        });
    });

    // Return the pointer to the next empty slot in the Moves array.
    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardLanceMovesImpl(const State& S, Move32* Moves,
                                         const Bitboard& TargetSquares,
                                         const Bitboard& OccupiedBB) {
    const Bitboard FromBB = S.getBitboard<C, PTK_Lance>();
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        const Bitboard ToBB = getLanceAttackBB<C>(From, OccupiedBB) & TargetSquares;
        ToBB.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (WilyPromote) {
                if (FirstAndSecondFurthestBB[C].isSet(To)) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(
                            From, To, PTK_Lance, CaptureType);
                    } else {
                        *Moves++ =
                            Move32::boardPromotingMove(From, To, PTK_Lance);
                    }
                    return;
                }

                if (PromotableBB[C].isSet(To)) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(
                            From, To, PTK_Lance, CaptureType);
                    } else {
                        *Moves++ =
                            Move32::boardPromotingMove(From, To, PTK_Lance);
                    }
                }

                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardMove(From, To, PTK_Lance, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, PTK_Lance);
                }
            } else {
                if (PromotableBB[C].isSet(To)) {
                    // If the move starts or ends on a promotable square,
                    // promote the piece.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(
                            From, To, PTK_Lance, CaptureType);
                    } else {
                        *Moves++ =
                            Move32::boardPromotingMove(From, To, PTK_Lance);
                    }
                }

                // Add a non-promoting move.
                if (!FurthermostBB[C].isSet(To)) {
                    // Filter out going the furthermost squares without
                    // promotion, because it violates the No-Unmovable-Pieces
                    // rule.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, PTK_Lance, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, PTK_Lance);
                    }
                }
            }
        });
    });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardBishopMovesImpl(const State& S, Move32* Moves,
                                          const Bitboard& TargetSquares,
                                          const Bitboard& OccupiedBB) {
    static_assert(Type == PTK_Bishop || Type == PTK_ProBishop,
                  "Type should be PTK_Bishop or PTK_ProBishop.");

    const Bitboard FromBB = S.getBitboard<C, Type>();
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);

        const Bitboard ToBB =
            getBishopAttackBB<Type>(From, OccupiedBB) & TargetSquares;

        ToBB.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Type == PTK_Bishop) {
                if (IsPromotableFrom || PromotableBB[C].isSet(To)) {
                    // If the move starts or ends on a promotable square,
                    // promote the piece.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(From, To, Type,
                                                              CaptureType);
                    } else {
                        *Moves++ = Move32::boardPromotingMove(From, To, Type);
                    }

                    if constexpr (WilyPromote) {
                        return;
                    }
                }
            }

            // Add a non-promoting move.
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

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardRookMovesImpl(const State& S, Move32* Moves,
                                        const Bitboard& TargetSquares,
                                        const Bitboard& OccupiedBB) {
    static_assert(Type == PTK_Rook || Type == PTK_ProRook,
                  "Type should be PTK_Rook or PTK_ProRook.");

    const Bitboard FromBB = S.getBitboard<C, Type>();
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);

        const Bitboard ToBB = getRookAttackBB<Type>(From, OccupiedBB) & TargetSquares;

        ToBB.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Type == PTK_Rook) {
                if (IsPromotableFrom || PromotableBB[C].isSet(To)) {
                    // If the move starts or ends on a promotable square,
                    // promote the piece.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(From, To, Type,
                                                              CaptureType);
                    } else {
                        *Moves++ = Move32::boardPromotingMove(From, To, Type);
                    }

                    if constexpr (WilyPromote) {
                        return;
                    }
                }
            }

            // Add a non-promoting move.
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

template <Color C>
inline Move32* generateDroppingMovesImpl(const State& S, Move32* List,
                                     const Bitboard& TargetSquares) {
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

    const bool LanceExists = getStandCount<PTK_Lance>(St) > 0;
    const bool KnightExists = getStandCount<PTK_Knight>(St) > 0;
    const bool SilverExists = getStandCount<PTK_Silver>(St) > 0;
    const bool GoldExists = getStandCount<PTK_Gold>(St) > 0;
    const bool BishopExists = getStandCount<PTK_Bishop>(St) > 0;
    const bool RookExists = getStandCount<PTK_Rook>(St) > 0;

    /* if (Stands exist) */ {
        uint32_t DroppingCandidates[6];
        int MoveCount = 0;
        if (LanceExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Lance).value();
            ++MoveCount;
        }
        if (KnightExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Knight).value();
            ++MoveCount;
        }
        if (SilverExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Silver).value();
            ++MoveCount;
        }
        if (GoldExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Gold).value();
            ++MoveCount;
        }
        if (BishopExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Bishop).value();
            ++MoveCount;
        }
        if (RookExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Rook).value();
            ++MoveCount;
        }

        const Bitboard ToBB = FirstAndSecondFurthestBB[C].andNot(TargetSquares);
        switch (MoveCount) {
            case 1:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                });
                break;
            case 2:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                });
                break;
            case 3:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                });
                break;
            case 4:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                });
                break;
            case 5:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
                });
                break;
            case 6:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[5] | (uint32_t)To);
                });
                break;
        }
    }

    {
        uint32_t DroppingCandidates[5];
        int MoveCount = 0;
        if (LanceExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Lance).value();
            ++MoveCount;
        }
        if (SilverExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Silver).value();
            ++MoveCount;
        }
        if (GoldExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Gold).value();
            ++MoveCount;
        }
        if (BishopExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Bishop).value();
            ++MoveCount;
        }
        if (RookExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Rook).value();
            ++MoveCount;
        }

        const Bitboard ToBB = TargetSquares & SecondFurthestBB[C];
        switch (MoveCount) {
            case 1:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                });
                break;
            case 2:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                });
                break;
            case 3:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                });
                break;
            case 4:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                });
                break;
            case 5:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[4] | (uint32_t)To);
                });
                break;
        }
    }

    {
        // Dropping move onto the furthermost squares.
        uint32_t DroppingCandidates[4];
        int MoveCount = 0;
        if (SilverExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Silver).value();
            ++MoveCount;
        }
        if (GoldExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Gold).value();
            ++MoveCount;
        }
        if (BishopExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Bishop).value();
            ++MoveCount;
        }
        if (RookExists) {
            DroppingCandidates[MoveCount] = Move32::droppingMove((Square)0, PTK_Rook).value();
            ++MoveCount;
        }

        const Bitboard ToBB = TargetSquares & FurthermostBB[C];

        switch (MoveCount) {
            case 1:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                });
                break;
            case 2:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                });
                break;
            case 3:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                });
                break;
            case 4:
                ToBB.forEach([&](Square To) {
                    *List++ = Move32::fromValue(DroppingCandidates[0] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[1] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[2] | (uint32_t)To);
                    *List++ = Move32::fromValue(DroppingCandidates[3] | (uint32_t)To);
                });
                break;
        }
    }

    return List;
}

template <Color C>
inline Move32* generateDroppingStepCheckMovesImpl(const State& S, const Square OpKingSq, Move32* List,
                                     const Bitboard& TargetBB) {
    const Stands St = S.getPosition().getStand<C>();

    if (getStandCount<PTK_Pawn>(St) > 0) {
        const Bitboard PawnBB = S.getBitboard<C, PTK_Pawn>();

        if ((PawnBB & FileBB[squareToFile(OpKingSq)]).isZero()) {
            if constexpr (C == Black) {
                const Bitboard ToBB = TargetBB & ((FurthermostBB[White].andNot(SquareBB[OpKingSq])).getRightShiftEpi64<1>());
                ToBB.forEach([&](Square To) {
                    assert(checkRange(To));
                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                });
            } else {
                const Bitboard ToBB = TargetBB & ((FurthermostBB[Black].andNot(SquareBB[OpKingSq])).getLeftShiftEpi64<1>());
                ToBB.forEach([&](Square To) {
                    assert(checkRange(To));
                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                });
            }
        }
    }

    if (getStandCount<PTK_Knight>(St) > 0) {
        const Bitboard ToBB = TargetBB & KnightAttackBB[~C][OpKingSq];
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Knight);
        });
    }

    if (getStandCount<PTK_Silver>(St) > 0) {
        const Bitboard ToBB = TargetBB & SilverAttackBB[~C][OpKingSq];
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Silver);
        });
    }

    if (getStandCount<PTK_Gold>(St) > 0) {
        const Bitboard ToBB = TargetBB & GoldAttackBB[~C][OpKingSq];
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Gold);
        });
    }

    return List;
}

template <Color C>
inline Move32* generateDroppingSliderCheckMovesImpl(const State& S, const Square OpKingSq, Move32* List,
                                     const Bitboard& TargetBB, const Bitboard& OccupiedBB) {
    const Stands St = S.getPosition().getStand<C>();

    if (getStandCount<PTK_Lance>(St) > 0) {
        const Bitboard ToBB = getLanceAttackBB<~C>(OpKingSq, OccupiedBB) & TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Lance);
        });
    }

    if (getStandCount<PTK_Rook>(St) > 0) {
        const Bitboard ToBB = getRookAttackBB<PTK_Rook>(OpKingSq, OccupiedBB) & TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Rook);
        });
    }

    if (getStandCount<PTK_Bishop>(St) > 0) {
        const Bitboard ToBB = getBishopAttackBB<PTK_Bishop>(OpKingSq, OccupiedBB) & TargetBB;
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Bishop);
        });
    }

    return List;
}

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardOneStepNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type != PTK_Lance,     "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Bishop,    "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Rook,      "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook,   "PTK_ProRook must not be passed as `Type` in generateCheckStepMoves().");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;

    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, Type>(OpKingSq))
                                      : getAttackBB<~C, Type>(OpKingSq))
                             & getAttackBB<C, Type>(From) & TargetBB;

        if constexpr (Type == PTK_Pawn) {
            if constexpr (WilyPromote) {
                TargetBB2 = PromotableBB[C].andNot(TargetBB2);
            } else {
                TargetBB2 = FurthermostBB[C].andNot(TargetBB2);
            }
        }

        if constexpr (Type == PTK_Knight) {
            TargetBB2 = FirstAndSecondFurthestBB[C].andNot(TargetBB2);
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if constexpr (Type == PTK_King) {
                if (S.isAttacked<C>(To, From)) {
                    return;
                }
            }

            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
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

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardOneStepNoPromoteCheckGoldKindsMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    const Bitboard FromBB = S.getBitboard<C>() & SourceFilter
                        & (S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>()
                            | S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>() | S.getBitboard<PTK_ProSilver>());

    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        const PieceTypeKind Type = getPieceType(S.getPosition().pieceOn(From));

        const Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, PTK_Gold>(OpKingSq))
                                      : getAttackBB<~C, PTK_Gold>(OpKingSq))
                             & getAttackBB<C, PTK_Gold>(From) & TargetBB;

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
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

template<Color C, bool Capture, bool WilyPromote, bool Pinned>
inline Move32* generateOnBoardLanceNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        Bitboard TargetBB2 = (Pinned)? ~LineBB[From][OpKingSq] : getLanceAttackBB<~C>(OpKingSq, OccupiedBB);

        if constexpr (WilyPromote) {
            TargetBB2 = FirstAndSecondFurthestBB[C].andNot(TargetBB2);
        } else {
            TargetBB2 = FurthermostBB[C].andNot(TargetBB2);
        }

        TargetBB2 &= getLanceAttackBB<C>(From, OccupiedBB) & TargetBB;

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
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

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardBishopNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type == PTK_Bishop || Type == PTK_ProBishop, "Only PTK_Bishop or PTK_ProBishop can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        if constexpr (WilyPromote && Type == PTK_Bishop) {
            if (PromotableBB[C].isSet(From)) {
                return;
            }
        }

        // clang-format off
        Bitboard TargetBB2;

        if constexpr (Pinned) {
            TargetBB2 = ~LineBB[From][OpKingSq] | getBishopAttackBB<Type>(OpKingSq, OccupiedBB);
        } else {
            TargetBB2 = getBishopAttackBB<Type>(OpKingSq, OccupiedBB);
        }
        TargetBB2 = TargetBB2 & getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if constexpr (WilyPromote && Type == PTK_Bishop) {
            TargetBB2 = PromotableBB[C].andNot(TargetBB2);
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
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

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardRookNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type == PTK_Rook || Type == PTK_ProRook, "Only PTK_Rook or PTK_ProRook can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        if constexpr (WilyPromote && Type == PTK_Rook) {
            if (PromotableBB[C].isSet(From)) {
                return;
            }
        }

        // clang-format off
        Bitboard TargetBB2;

        if constexpr (Pinned) {
            TargetBB2 = ~LineBB[From][OpKingSq] | getRookAttackBB<Type>(OpKingSq, OccupiedBB);
        } else {
            TargetBB2 = getRookAttackBB<Type>(OpKingSq, OccupiedBB);
        }
        TargetBB2 = TargetBB2 & getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if constexpr (WilyPromote && Type == PTK_Rook) {
            TargetBB2 = PromotableBB[C].andNot(TargetBB2);
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
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


template<Color C, PieceTypeKind Type, bool Capture, bool Pinned>
inline Move32* generateOnBoardOneStepPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type != PTK_Lance,     "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Bishop,    "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Rook,      "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook,   "PTK_ProRook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(!isPromoted(Type),     "Type must be an unpromoted piece type.");

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, PTK_Gold>(OpKingSq))
                                    : getAttackBB<~C, PTK_Gold>(OpKingSq))
                             & getAttackBB<C, Type>(From) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        });
    });

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardLancePromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        const Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, PTK_Gold>(OpKingSq))
                                    : getAttackBB<~C, PTK_Gold>(OpKingSq))
                             & getLanceAttackBB<C>(From, OccupiedBB) & TargetBB & PromotableBB[C];

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        });
    });

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardBishopPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Bishop;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getBishopAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq))
                                    : (getBishopAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq)))
                             & getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        });
    });

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardRookPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Rook;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    FromBB.forEach([&](Square From) {
        const bool IsDefendingPiece = S.getDefendingOpponentSliderBB<C>().isSet(From);

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getRookAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq))
                                    : (getRookAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq)))
                             & getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }
        // clang-format on

        TargetBB2.forEach([&](Square To) {
            if (IsDefendingPiece && S.isAttackedBySlider<C>(S.getKingSquare<C>(), OccupiedBB, From, To)) {
                return;
            }

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        });
    });

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardOneStepMovesImpl(const State& S, Move32* Moves,
                                           const Bitboard& TargetSquares, const Bitboard& OccupiedBB) {
    Moves = generateOnBoardOneStepPawnMovesImpl<C, Capture, WilyPromote>(S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, Capture>(S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, Capture>(S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, Capture>(S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardOneStepGoldKindsMovesImpl<C, Capture>(S, Moves, TargetSquares, OccupiedBB);

    return Moves;
}

template <Color C, bool Capture, bool Pinned, bool WilyPromote, bool SkipKing = false>
inline Move32* generateOnBoardOneStepCheckMovesImpl(const State& S, Move32* Moves, const Square OpKingSq, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& FromMask) {
    if constexpr (!SkipKing) {
        Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    }

    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Pawn, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Knight, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Silver, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckGoldKindsMovesImpl<C, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);

    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Pawn, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Knight, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Silver, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);

    return Moves;
}

template <Color C, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardSliderCheckMovesImpl(const State& S, Move32* Moves, const Square OpKingSq, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& FromMask) {
    Moves = generateOnBoardLanceNoPromoteCheckMovesImpl<C, Capture, WilyPromote, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_Bishop, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_ProBishop, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_Rook, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_ProRook, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);

    Moves = generateOnBoardLancePromoteCheckMovesImpl<C, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopPromoteCheckMovesImpl<C, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookPromoteCheckMovesImpl<C, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, OccupiedBB, FromMask);

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardSliderMovesImpl(const State& S, Move32* Moves,
                                          const Bitboard& TargetSquares,
                                          const Bitboard& OccupiedBB) {
    Moves = generateOnBoardLanceMovesImpl<C, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardBishopMovesImpl<C, PTK_ProBishop, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardBishopMovesImpl<C, PTK_Bishop, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardRookMovesImpl<C, PTK_ProRook, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves = generateOnBoardRookMovesImpl<C, PTK_Rook, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);

    return Moves;
}

template <Color C, bool WilyPromote = true>
inline Move32* generateLegalEvasionMovesImpl(const State& S, Move32* Moves,
                                                 const Bitboard& CheckerBB,
                                                 const Bitboard& OpponentBB,
                                                 const Bitboard& OccupiedBB) {
    // Moving the king.
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, true>(S, Moves, OpponentBB, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, false>(S, Moves, ~OccupiedBB, OccupiedBB);

    // If there are more than one checkers,
    // no moves available but moving the king.
    if (CheckerBB.popCount() >= 2) {
        return Moves;
    }

    // Capturing a checker.
    Moves = generateOnBoardOneStepPawnMovesImpl<C, true, WilyPromote>(S, Moves, CheckerBB, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, true>(S, Moves, CheckerBB, OccupiedBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, true>(S, Moves, CheckerBB, OccupiedBB);
    Moves = generateOnBoardOneStepGoldKindsMovesImpl<C, true>(S, Moves, CheckerBB, OccupiedBB);

    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(S, Moves, CheckerBB,
                                                            OccupiedBB);

    const Square CheckerSq = CheckerBB.getOne();
    const Bitboard BetweenBB = getBetweenBB(S.getKingSquare<C>(), CheckerSq);

    if (BetweenBB.isZero()) {
        return Moves;
    }

    // Moving a piece.
    Moves =
        generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves, BetweenBB, OccupiedBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(S, Moves, BetweenBB,
                                                             OccupiedBB);

    // Dropping a piece (a.k.a. aigoma).
    Moves = generateDroppingMovesImpl<C>(S, Moves, BetweenBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const State& S, Move32* Moves,
                                              const Bitboard& OpponentBB,
                                              const Bitboard& OccupiedBB) {
    // Captures.
    Moves =
        generateOnBoardOneStepMovesImpl<C, true, WilyPromote>(S, Moves, OpponentBB, OccupiedBB);
    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(S, Moves, OpponentBB,
                                                            OccupiedBB);

    const Bitboard EmptyBB = ~OccupiedBB;
    // Droppings.
    Moves = generateDroppingMovesImpl<C>(S, Moves, EmptyBB);

    // No captures.
    Moves = generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves, EmptyBB, OccupiedBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(S, Moves, EmptyBB,
                                                             OccupiedBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const State& S, Move32* Moves) {
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard BlackBB = S.getBitboard<Black>();
    const Bitboard WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;

    if constexpr (C == Black) {
        if (!CheckerBB.isZero()) {
            Moves = generateLegalEvasionMovesImpl<Black, WilyPromote>(
                S, Moves, CheckerBB, WhiteBB, OccupiedBB);
        } else {
            Moves = generateLegalMovesImpl<Black, WilyPromote>(
                S, Moves, WhiteBB, OccupiedBB);
        }
    } else {
        if (!CheckerBB.isZero()) {
            Moves = generateLegalEvasionMovesImpl<White, WilyPromote>(
                S, Moves, CheckerBB, BlackBB, OccupiedBB);
        } else {
            Moves = generateLegalMovesImpl<White, WilyPromote>(
                S, Moves, BlackBB, OccupiedBB);
        }
    }

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalCheckMovesImpl(const State& S, Move32* Moves) {
    const Bitboard BlackBB = S.getBitboard<Black>();
    const Bitboard WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;
    const Bitboard EmptyBB = ~OccupiedBB;
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard PinnedBB = S.getDefendingOpponentSliderBB<~C>();
    const Bitboard NoPinnedBB = ~PinnedBB;
    const Square MyKingSq = S.getKingSquare<C>();
    const Square OpKingSq = S.getKingSquare<~C>();

    if (CheckerBB.popCount() >= 2) {
        if (PinnedBB.isSet(S.getKingSquare<C>())) {
            const Bitboard KingLineBB = LineBB[MyKingSq][OpKingSq];
            const Bitboard UnpinEmptyBB = KingLineBB.andNot(EmptyBB);
            const Bitboard UnpinCaptureBB = KingLineBB.andNot(S.getBitboard<~C>());

            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, false, true, WilyPromote>(S, OpKingSq, Moves, UnpinEmptyBB, PinnedBB, OccupiedBB);
            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, true, true, WilyPromote>(S, OpKingSq, Moves, UnpinCaptureBB, PinnedBB, OccupiedBB);
        }

        return Moves;
    }

    const bool PinnedExists = !PinnedBB.isZero();
    const Stands St = S.getPosition().getStand<C>();

    if (!CheckerBB.isZero()) {
        const Square CheckerSq = CheckerBB.getOne();
        const Bitboard BetweenTarget = BetweenBB[CheckerSq][MyKingSq];

        if (St != 0) {
            Moves = generateDroppingStepCheckMovesImpl<C>(S, OpKingSq, Moves, BetweenTarget);
            Moves = generateDroppingSliderCheckMovesImpl<C>(S, OpKingSq, Moves, BetweenTarget, OccupiedBB);
        }

        // Capturing moves.

        const Bitboard CheckerMyKingBetweenBB = BetweenBB[CheckerSq][MyKingSq];

        if (PinnedExists) {
            if (PinnedBB.isSet(MyKingSq)) {
                const Bitboard KingLineBB = LineBB[MyKingSq][OpKingSq];
                const Bitboard UnpinEmptyBB = KingLineBB.andNot(EmptyBB);
                const Bitboard UnpinCaptureBB = KingLineBB.andNot(S.getBitboard<~C>());

                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, false, true, WilyPromote>(S, OpKingSq, Moves, UnpinEmptyBB, PinnedBB, OccupiedBB);
                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, true, true, WilyPromote>(S, OpKingSq, Moves, UnpinCaptureBB, PinnedBB, OccupiedBB);
            }

            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true, WilyPromote, true>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true, WilyPromote, true>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, PinnedBB);

            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, PinnedBB);
        }

        Moves = generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, NoPinnedBB);
    } else {
        if (St != 0) {
            Moves = generateDroppingStepCheckMovesImpl<C>(S, OpKingSq, Moves, EmptyBB);
            Moves = generateDroppingSliderCheckMovesImpl<C>(S, OpKingSq, Moves, EmptyBB, OccupiedBB);
        }

        if (PinnedExists) {
            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, PinnedBB);
        }

        Moves = generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, NoPinnedBB);
    }

    return Moves;
}

template <GenerateType GenType, Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const State& S, Move32* Moves) {
    if constexpr (GenType == GenerateType::Normal) {
        Moves = generateLegalMovesImpl<C, WilyPromote>(S, Moves);
    } else if constexpr (GenType == GenerateType::Check) {
        Moves = generateLegalCheckMovesImpl<C, WilyPromote>(S, Moves);
    }

    return Moves;
}

} // namespace

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) {
    MoveList List;
    List.Tail = generateLegalMovesImpl<GenerateType::Normal, C, WilyPromote>(S, List.Tail);
    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) {
    return (S.getPosition().sideToMove() == Black)
        ? generateLegalMoves<Black, WilyPromote>(S)
        : generateLegalMoves<White, WilyPromote>(S);
}

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) {
    MoveList List;
    List.Tail = generateLegalMovesImpl<GenerateType::Check, C, WilyPromote>(S, List.Tail);
    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) {
    return (S.getPosition().sideToMove() == Black)
        ? generateLegalCheckMoves<Black, WilyPromote>(S)
        : generateLegalCheckMoves<White, WilyPromote>(S);
}

template MoveList MoveGenerator::generateLegalMoves<Black, false>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<Black, true>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<White, false>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<White, true>(const State& S);

template MoveList MoveGenerator::generateLegalCheckMoves<Black, false>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<Black, true>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<White, false>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<White, true>(const State& S);

template MoveList MoveGenerator::generateLegalMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<true>(const State& S);

template MoveList MoveGenerator::generateLegalCheckMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<true>(const State& S);


} // namespace core
} // namespace nshogi
