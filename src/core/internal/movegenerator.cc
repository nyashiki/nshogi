//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "movegenerator.h"
#include "bitboard.h"

namespace nshogi {
namespace core {
namespace internal {

namespace {

using namespace internal::bitboard;

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32*
generateOnBoardOneStepPawnMovesImpl(const StateImpl& S,
                                    Move32* __restrict Moves,
                                    const Bitboard& TargetSquares) noexcept {
    Bitboard ToBB = S.getBitboard<C, PTK_Pawn>();

    if constexpr (C == Black) {
        ToBB = ToBB.getLeftShiftEpi64<1>();
    } else {
        ToBB = ToBB.getRightShiftEpi64<1>();
    }

    ToBB &= TargetSquares;

    (ToBB & PromotableBB[C]).forEach([&](Square To) {
        const Square From = (C == Black) ? (To + South) : (To + North);

        const bool IsDefendingPiece =
            S.getDefendingOpponentSliderBB<C>().isSet(From);

        // If this pawn is defending a slider piece along a diagonal or
        // horizontal, this pawn cannot move.
        if (IsDefendingPiece) {
            const std::size_t KingSq = (std::size_t)S.getKingSquare<C>();
            if (SquareDirection[(std::size_t)From][KingSq] !=
                    Direction::North &&
                SquareDirection[(std::size_t)From][KingSq] !=
                    Direction::South) {
                return;
            }
        }

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
        const Square From = (C == Black) ? (To + South) : (To + North);

        const bool IsDefendingPiece =
            S.getDefendingOpponentSliderBB<C>().isSet(From);

        if (IsDefendingPiece) {
            const std::size_t KingSq = (std::size_t)S.getKingSquare<C>();
            if (SquareDirection[(std::size_t)From][KingSq] !=
                    Direction::North &&
                SquareDirection[(std::size_t)From][KingSq] !=
                    Direction::South) {
                return;
            }
        }

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
         S.getBitboard<PTK_ProSilver>()) &
        S.getBitboard<C>();

    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const PieceTypeKind FromPieceType =
            getPieceType(S.getPosition().pieceOn(From));

        const Bitboard ToBB = TargetSquares & getAttackBB<C, PTK_Gold>(From) &
                              LineBB[From][S.getKingSquare<C>()];
        ToBB.forEach([&](Square To) {
            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ =
                    Move32::boardMove(From, To, FromPieceType, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, FromPieceType);
            }
        });
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const PieceTypeKind FromPieceType =
                getPieceType(S.getPosition().pieceOn(From));

            const Bitboard ToBB =
                TargetSquares & getAttackBB<C, PTK_Gold>(From);
            ToBB.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardMove(From, To, FromPieceType, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, FromPieceType);
                }
            });
        });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture>
inline Move32*
generateOnBoardOneStepMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                                const Bitboard& TargetSquares) noexcept {
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

    // Note: if a knight is defending a slider piece, it cannot move anywhere.
    if constexpr (Type != PTK_Knight) {
        (FromBB & S.getDefendingOpponentSliderBB<C>())
            .forEach([&](Square From) {
                assert(checkRange(From));

                const Bitboard ToBB =
                    (Type == PTK_King)
                        ? TargetSquares & getAttackBB<C, Type>(From)
                        : (TargetSquares & getAttackBB<C, Type>(From) &
                           LineBB[From][S.getKingSquare<C>()]);

                // No promotion only.
                ToBB.forEach([&](Square To) {
                    if constexpr (Type == PTK_King) {
                        if (S.isAttacked<C>(To, From)) {
                            return;
                        }
                    }

                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });

                // Promotion only.
                if constexpr (Type != PTK_King && Type != PTK_Gold &&
                              !isPromoted(Type)) {
                    const Bitboard PromotableToBB =
                        PromotableBB[C].isSet(From) ? ToBB
                                                    : (ToBB & PromotableBB[C]);

                    PromotableToBB.forEach([&](Square To) {
                        if constexpr (Type == PTK_King) {
                            if (S.isAttacked<C>(To, From)) {
                                return;
                            }
                        }

                        // Handling regular promotion cases for all piece types.
                        if constexpr (Type != PTK_King) {
                            if constexpr (!isPromoted(Type) &&
                                          Type != PTK_Gold) {
                                if constexpr (Capture) {
                                    const PieceTypeKind CaptureType =
                                        getPieceType(
                                            S.getPosition().pieceOn(To));
                                    *Moves++ = Move32::boardPromotingMove(
                                        From, To, Type, CaptureType);
                                } else {
                                    *Moves++ = Move32::boardPromotingMove(
                                        From, To, Type);
                                }
                            }
                        }
                    });
                }
            });
    }

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            assert(checkRange(From));

            const Bitboard ToBB = TargetSquares & getAttackBB<C, Type>(From);

            // No promotion only.
            if constexpr (Type == PTK_Knight) {
                FirstAndSecondFurthestBB[C].andNot(ToBB).forEach(
                    [&](Square To) {
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ =
                                Move32::boardMove(From, To, Type, CaptureType);
                        } else {
                            *Moves++ = Move32::boardMove(From, To, Type);
                        }
                    });
            } else {
                ToBB.forEach([&](Square To) {
                    if constexpr (Type == PTK_King) {
                        if (S.isAttacked<C>(To, From)) {
                            return;
                        }
                    }

                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });
            }

            // Promotion only.
            if constexpr (Type != PTK_King && Type != PTK_Gold &&
                          !isPromoted(Type)) {
                const Bitboard PromotableToBB = PromotableBB[C].isSet(From)
                                                    ? ToBB
                                                    : (ToBB & PromotableBB[C]);

                PromotableToBB.forEach([&](Square To) {
                    if constexpr (Type == PTK_King) {
                        if (S.isAttacked<C>(To, From)) {
                            return;
                        }
                    }

                    // Handling regular promotion cases for all piece types.
                    if constexpr (Type != PTK_King) {
                        if constexpr (Type == PTK_Knight) {
                            if constexpr (Capture) {
                                const PieceTypeKind CaptureType =
                                    getPieceType(S.getPosition().pieceOn(To));
                                *Moves++ = Move32::boardPromotingMove(
                                    From, To, Type, CaptureType);
                            } else {
                                *Moves++ =
                                    Move32::boardPromotingMove(From, To, Type);
                            }
                        } else if constexpr (!isPromoted(Type) &&
                                             Type != PTK_Gold) {
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
                });
            }
        });

    // Return the pointer to the next empty slot in the Moves array.
    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32*
generateOnBoardLanceMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                              const Bitboard& TargetSquares,
                              const Bitboard& OccupiedBB) noexcept {
    // Note: using bitboard `and` operations that divide into cases
    // of promotion and non-promotion is slower, so here we do not
    // do that and check if the destination square is promotable
    // or not inside the loop.

    const Bitboard FromBB = S.getBitboard<C, PTK_Lance>();
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const Bitboard ToBB = getLanceAttackBB<C>(From, OccupiedBB) &
                              TargetSquares &
                              LineBB[From][S.getKingSquare<C>()];

        ToBB.forEach([&](Square To) {
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
                if (!Bitboard::FurthermostBB<C>().isSet(To)) {
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

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const Bitboard ToBB =
                getLanceAttackBB<C>(From, OccupiedBB) & TargetSquares;
            ToBB.forEach([&](Square To) {
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
                    if (!Bitboard::FurthermostBB<C>().isSet(To)) {
                        // Filter out going the furthermost squares without
                        // promotion, because it violates the
                        // No-Unmovable-Pieces rule.
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ = Move32::boardMove(From, To, PTK_Lance,
                                                         CaptureType);
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
inline Move32*
generateOnBoardBishopMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                               const Bitboard& TargetSquares,
                               const Bitboard& OccupiedBB) noexcept {
    static_assert(Type == PTK_Bishop || Type == PTK_ProBishop,
                  "Type should be PTK_Bishop or PTK_ProBishop.");

    const Bitboard FromBB = S.getBitboard<C, Type>();
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);

        const Bitboard ToBB = getBishopAttackBB<Type>(From, OccupiedBB) &
                              TargetSquares &
                              LineBB[From][S.getKingSquare<C>()];

        if constexpr (Type == PTK_Bishop) {
            const Bitboard PromotableToBB =
                IsPromotableFrom ? ToBB : (ToBB & PromotableBB[C]);

            PromotableToBB.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }

                if constexpr (!WilyPromote) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                }
            });

            PromotableToBB.andNot(ToBB).forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        } else {
            ToBB.forEach([&](Square To) {
                // Add a non-promoting move.
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        }
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const bool IsPromotableFrom = PromotableBB[C].isSet(From);

            const Bitboard ToBB =
                getBishopAttackBB<Type>(From, OccupiedBB) & TargetSquares;

            if constexpr (Type == PTK_Bishop) {
                const Bitboard PromotableToBB =
                    IsPromotableFrom ? ToBB : (ToBB & PromotableBB[C]);

                PromotableToBB.forEach([&](Square To) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(From, To, Type,
                                                              CaptureType);
                    } else {
                        *Moves++ = Move32::boardPromotingMove(From, To, Type);
                    }

                    if constexpr (!WilyPromote) {
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ =
                                Move32::boardMove(From, To, Type, CaptureType);
                        } else {
                            *Moves++ = Move32::boardMove(From, To, Type);
                        }
                    }
                });

                PromotableToBB.andNot(ToBB).forEach([&](Square To) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });
            } else {
                ToBB.forEach([&](Square To) {
                    // Add a non-promoting move.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });
            }
        });

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote = true>
inline Move32*
generateOnBoardRookMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                             const Bitboard& TargetSquares,
                             const Bitboard& OccupiedBB) noexcept {
    static_assert(Type == PTK_Rook || Type == PTK_ProRook,
                  "Type should be PTK_Rook or PTK_ProRook.");

    const Bitboard FromBB = S.getBitboard<C, Type>();
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);

        const Bitboard ToBB = getRookAttackBB<Type>(From, OccupiedBB) &
                              TargetSquares &
                              LineBB[From][S.getKingSquare<C>()];

        if constexpr (Type == PTK_Rook) {
            const Bitboard PromotableToBB =
                IsPromotableFrom ? ToBB : (ToBB & PromotableBB[C]);

            PromotableToBB.forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ =
                        Move32::boardPromotingMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardPromotingMove(From, To, Type);
                }

                if constexpr (!WilyPromote) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                }
            });

            PromotableToBB.andNot(ToBB).forEach([&](Square To) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        } else {
            ToBB.forEach([&](Square To) {
                // Add a non-promoting move.
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
                } else {
                    *Moves++ = Move32::boardMove(From, To, Type);
                }
            });
        }
    });

    S.getDefendingOpponentSliderBB<C>().andNot(FromBB).forEach(
        [&](Square From) {
            const bool IsPromotableFrom = PromotableBB[C].isSet(From);

            const Bitboard ToBB =
                getRookAttackBB<Type>(From, OccupiedBB) & TargetSquares;

            if constexpr (Type == PTK_Rook) {
                const Bitboard PromotableToBB =
                    IsPromotableFrom ? ToBB : (ToBB & PromotableBB[C]);

                PromotableToBB.forEach([&](Square To) {
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ = Move32::boardPromotingMove(From, To, Type,
                                                              CaptureType);
                    } else {
                        *Moves++ = Move32::boardPromotingMove(From, To, Type);
                    }

                    if constexpr (!WilyPromote) {
                        if constexpr (Capture) {
                            const PieceTypeKind CaptureType =
                                getPieceType(S.getPosition().pieceOn(To));
                            *Moves++ =
                                Move32::boardMove(From, To, Type, CaptureType);
                        } else {
                            *Moves++ = Move32::boardMove(From, To, Type);
                        }
                    }
                });

                PromotableToBB.andNot(ToBB).forEach([&](Square To) {
                    // Add a non-promoting move.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });
            } else {
                ToBB.forEach([&](Square To) {
                    // Add a non-promoting move.
                    if constexpr (Capture) {
                        const PieceTypeKind CaptureType =
                            getPieceType(S.getPosition().pieceOn(To));
                        *Moves++ =
                            Move32::boardMove(From, To, Type, CaptureType);
                    } else {
                        *Moves++ = Move32::boardMove(From, To, Type);
                    }
                });
            }
        });

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

    const bool LanceExists = getStandCount<PTK_Lance>(St) > 0;
    const bool KnightExists = getStandCount<PTK_Knight>(St) > 0;
    const bool SilverExists = getStandCount<PTK_Silver>(St) > 0;
    const bool GoldExists = getStandCount<PTK_Gold>(St) > 0;
    const bool BishopExists = getStandCount<PTK_Bishop>(St) > 0;
    const bool RookExists = getStandCount<PTK_Rook>(St) > 0;

    alignas(32) uint32_t Pack32[8] = {};

    int MoveCount = 0;

    if (SilverExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Silver).value();
        ++MoveCount;
    }
    if (GoldExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Gold).value();
        ++MoveCount;
    }
    if (BishopExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Bishop).value();
        ++MoveCount;
    }
    if (RookExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Rook).value();
        ++MoveCount;
    }
    if (LanceExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Lance).value();
        ++MoveCount;
    }
    if (KnightExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Knight).value();
        ++MoveCount;
    }

    const __m256i Pack256 = _mm256_load_si256((const __m256i*)Pack32);

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

    const bool LanceExists = getStandCount<PTK_Lance>(St) > 0;
    const bool KnightExists = getStandCount<PTK_Knight>(St) > 0;
    const bool SilverExists = getStandCount<PTK_Silver>(St) > 0;
    const bool GoldExists = getStandCount<PTK_Gold>(St) > 0;
    const bool BishopExists = getStandCount<PTK_Bishop>(St) > 0;
    const bool RookExists = getStandCount<PTK_Rook>(St) > 0;

    alignas(32) uint32_t Pack32[8] = {};

    int MoveCount = 0;

    if (SilverExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Silver).value();
        ++MoveCount;
    }
    if (GoldExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Gold).value();
        ++MoveCount;
    }
    if (BishopExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Bishop).value();
        ++MoveCount;
    }
    if (RookExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Rook).value();
        ++MoveCount;
    }
    if (LanceExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Lance).value();
        ++MoveCount;
    }
    if (KnightExists) {
        Pack32[MoveCount] = Move32::droppingMove((Square)0, PTK_Knight).value();
        ++MoveCount;
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
            TargetBB & KnightAttackBB[~C][S.getKingSquare<~C>()];
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Knight);
        });
    }

    if (getStandCount<PTK_Silver>(St) > 0) {
        const Bitboard ToBB =
            TargetBB & SilverAttackBB[~C][S.getKingSquare<~C>()];
        ToBB.forEach([&](Square To) {
            assert(checkRange(To));
            *List++ = Move32::droppingMove(To, PTK_Silver);
        });
    }

    if (getStandCount<PTK_Gold>(St) > 0) {
        const Bitboard ToBB =
            TargetBB & GoldAttackBB[~C][S.getKingSquare<~C>()];
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

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;

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
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned) ? ~LineBB[From][S.getKingSquare<~C>()]
                     : getLanceAttackBB<~C>(S.getKingSquare<~C>(), OccupiedBB);

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
                    : getLanceAttackBB<~C>(S.getKingSquare<~C>(), OccupiedBB);

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
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
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
                getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
        } else {
            TargetBB2 =
                getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
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
                    getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
            } else {
                TargetBB2 =
                    getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
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
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
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
                getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
        } else {
            TargetBB2 =
                getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
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
                    getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
            } else {
                TargetBB2 =
                    getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB);
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
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
    constexpr PieceTypeKind Type = PTK_Bishop;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned
                 ? (~LineBB[From][S.getKingSquare<~C>()] |
                    getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB) |
                    getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))
                 : (getBishopAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB) |
                    getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))) &
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
                           getBishopAttackBB<Type>(S.getKingSquare<~C>(),
                                                   OccupiedBB) |
                           getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))
                        : (getBishopAttackBB<Type>(S.getKingSquare<~C>(),
                                                   OccupiedBB) |
                           getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))) &
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
    const Bitboard& OccupiedBB, const Bitboard& SourceFilter) noexcept {
    constexpr PieceTypeKind Type = PTK_Rook;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    const Bitboard FromBB = S.getBitboard<C, Type>() & SourceFilter;
    (FromBB & S.getDefendingOpponentSliderBB<C>()).forEach([&](Square From) {
        Bitboard TargetBB2 =
            (Pinned
                 ? (~LineBB[From][S.getKingSquare<~C>()] |
                    getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB) |
                    getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))
                 : (getRookAttackBB<Type>(S.getKingSquare<~C>(), OccupiedBB) |
                    getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))) &
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
                           getRookAttackBB<Type>(S.getKingSquare<~C>(),
                                                 OccupiedBB) |
                           getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))
                        : (getRookAttackBB<Type>(S.getKingSquare<~C>(),
                                                 OccupiedBB) |
                           getAttackBB<~C, PTK_King>(S.getKingSquare<~C>()))) &
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

template <Color C, bool Capture, bool WilyPromote = true>
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
    if constexpr (!SkipKing) {
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
    const Bitboard& OccupiedBB, const Bitboard& FromMask) noexcept {
    Moves = generateOnBoardLanceNoPromoteCheckMovesImpl<C, Capture, WilyPromote,
                                                        Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_Bishop, Capture,
                                                         Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves =
        generateOnBoardBishopNoPromoteCheckMovesImpl<C, PTK_ProBishop, Capture,
                                                     Pinned, WilyPromote>(
            S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_Rook, Capture,
                                                       Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookNoPromoteCheckMovesImpl<C, PTK_ProRook, Capture,
                                                       Pinned, WilyPromote>(
        S, Moves, TargetBB, OccupiedBB, FromMask);

    Moves = generateOnBoardLancePromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardBishopPromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask);
    Moves = generateOnBoardRookPromoteCheckMovesImpl<C, Capture, Pinned>(
        S, Moves, TargetBB, OccupiedBB, FromMask);

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32*
generateOnBoardSliderMovesImpl(const StateImpl& S, Move32* __restrict Moves,
                               const Bitboard& TargetSquares,
                               const Bitboard& OccupiedBB) noexcept {
    Moves = generateOnBoardLanceMovesImpl<C, Capture, WilyPromote>(
        S, Moves, TargetSquares, OccupiedBB);
    Moves =
        generateOnBoardBishopMovesImpl<C, PTK_ProBishop, Capture, WilyPromote>(
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
inline Move32* generateLegalEvasionMovesImpl(
    const StateImpl& S, Move32* __restrict Moves, const Bitboard& CheckerBB,
    const Bitboard& OpponentBB, const Bitboard& OccupiedBB) noexcept {
    // Moving the king.
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, true>(S, Moves,
                                                               OpponentBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, false>(S, Moves,
                                                                ~OccupiedBB);

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

    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(
        S, Moves, CheckerBB, OccupiedBB);

    const Square CheckerSq = CheckerBB.getOne();
    const Bitboard BetweenBB = getBetweenBB(S.getKingSquare<C>(), CheckerSq);

    if (BetweenBB.isZero()) {
        return Moves;
    }

    // Moving a piece.
    Moves = generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves,
                                                                   BetweenBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(
        S, Moves, BetweenBB, OccupiedBB);

    // Dropping a piece (a.k.a. aigoma).
    Moves = generateDroppingMovesImpl<C>(S, Moves, BetweenBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const StateImpl& S,
                                      Move32* __restrict Moves,
                                      const Bitboard& OpponentBB,
                                      const Bitboard& OccupiedBB) noexcept {
    // Captures.
    Moves = generateOnBoardOneStepMovesImpl<C, true, WilyPromote>(S, Moves,
                                                                  OpponentBB);
    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(
        S, Moves, OpponentBB, OccupiedBB);

    const Bitboard EmptyBB = ~OccupiedBB;
    // Droppings.
    Moves = generateDroppingMovesImpl<C>(S, Moves, EmptyBB);

    // No captures.
    Moves = generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves,
                                                                   EmptyBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(
        S, Moves, EmptyBB, OccupiedBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const StateImpl& S,
                                      Move32* Moves) noexcept {
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
inline Move32* generateLegalCheckMovesImpl(const StateImpl& S,
                                           Move32* Moves) noexcept {
    const Bitboard BlackBB = S.getBitboard<Black>();
    const Bitboard WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;
    const Bitboard EmptyBB = ~OccupiedBB;
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard PinnedBB = S.getDefendingOpponentSliderBB<~C>();
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
                S, Moves, CheckerMyKingBetweenBB, OccupiedBB, PinnedBB);
            Moves =
                generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(
                    S, Moves, CheckerBB, OccupiedBB, PinnedBB);
        }

        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, CheckerMyKingBetweenBB, NoPinnedBB);
        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, CheckerBB, NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, CheckerMyKingBetweenBB, OccupiedBB, NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, CheckerBB, OccupiedBB, NoPinnedBB);
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
                S, Moves, EmptyBB, OccupiedBB, PinnedBB);
            Moves =
                generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(
                    S, Moves, S.getBitboard<~C>(), OccupiedBB, PinnedBB);
        }

        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, EmptyBB, NoPinnedBB);
        Moves =
            generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, S.getBitboard<~C>(), NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(
                S, Moves, EmptyBB, OccupiedBB, NoPinnedBB);
        Moves =
            generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(
                S, Moves, S.getBitboard<~C>(), OccupiedBB, NoPinnedBB);
    }

    return Moves;
}

} // namespace

template <Color C, bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalMoves(const StateImpl& S) noexcept {
    MoveList List;
    List.Tail = generateLegalMovesImpl<C, WilyPromote>(S, List.Tail);
    return List;
}

template <bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalMoves(const StateImpl& S) noexcept {
    return (S.getPosition().sideToMove() == Black)
               ? generateLegalMoves<Black, WilyPromote>(S)
               : generateLegalMoves<White, WilyPromote>(S);
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
MoveGeneratorInternal::generateLegalEvasionMoves(const StateImpl& S) noexcept {
    const Bitboard CheckerBB = S.getCheckerBB();
    const Bitboard MyBB = S.getBitboard<C>();
    const Bitboard OpBB = S.getBitboard<~C>();
    const Bitboard OccupiedBB = MyBB | OpBB;

    assert(!CheckerBB.isZero());

    MoveList List;
    List.Tail = generateLegalEvasionMovesImpl<C, WilyPromote>(
        S, List.Tail, CheckerBB, OpBB, OccupiedBB);

    return List;
}

template <bool WilyPromote>
MoveList
MoveGeneratorInternal::generateLegalCheckMoves(const StateImpl& S) noexcept {
    return (S.getPosition().sideToMove() == Black)
               ? generateLegalCheckMoves<Black, WilyPromote>(S)
               : generateLegalCheckMoves<White, WilyPromote>(S);
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
MoveGeneratorInternal::generateLegalEvasionMoves<Black, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalEvasionMoves<Black, true>(
    const StateImpl& S) noexcept;
template MoveList
MoveGeneratorInternal::generateLegalEvasionMoves<White, false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalEvasionMoves<White, true>(
    const StateImpl& S) noexcept;

template MoveList
MoveGeneratorInternal::generateLegalMoves<false>(const StateImpl& S) noexcept;
template MoveList
MoveGeneratorInternal::generateLegalMoves<true>(const StateImpl& S) noexcept;

template MoveList MoveGeneratorInternal::generateLegalCheckMoves<false>(
    const StateImpl& S) noexcept;
template MoveList MoveGeneratorInternal::generateLegalCheckMoves<true>(
    const StateImpl& S) noexcept;

} // namespace internal
} // namespace core
} // namespace nshogi
