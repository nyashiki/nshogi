#include "movegenerator.h"
#include "../io/bitboard.h"
#include "bitboard.h"
#include "bititerator.h"
#include "movelist.h"
#include "position.h"
#include "types.h"

namespace nshogi {
namespace core {

namespace {

using namespace bitboard;

enum struct GenerateType {
    Normal, Check, Evasion
};

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardOneStepPawnMovesImpl(const State& S, Move32* Moves,
                                               const Bitboard& TargetSquares) {
    Bitboard ToBB = S.getBitboard<C, PTK_Pawn>();

    if constexpr (C == Black) {
        ToBB = ToBB.getLeftShiftEpi64<1>();
    } else {
        ToBB = ToBB.getRightShiftEpi64<1>();
    }

    BitIterator ToIt(ToBB & TargetSquares);

    while (!ToIt.end()) {
        const Square To = ToIt.next();
        assert(checkRange(To));

        if (PromotableBB[C].isSet(To)) {
            if constexpr (C == Black) {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardPromotingMove(
                        To + South, To, PTK_Pawn, CaptureType);
                } else {
                    *Moves++ =
                        Move32::boardPromotingMove(To + South, To, PTK_Pawn);
                }
            } else {
                if constexpr (Capture) {
                    const PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(To));
                    *Moves++ = Move32::boardPromotingMove(
                        To + North, To, PTK_Pawn, CaptureType);
                } else {
                    *Moves++ =
                        Move32::boardPromotingMove(To + North, To, PTK_Pawn);
                }
            }

            if (WilyPromote) {
                continue;
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
    }

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture>
inline Move32* generateOnBoardOneStepMovesImpl(const State& S, Move32* Moves,
                                           const Bitboard& TargetSquares) {
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
    Bitboard FromBB = S.getBitboard<C, Type>();

    if (FromBB.isZero()) {
        return Moves;
    }

    BitIterator FromIt(FromBB);

    // // Loop through each piece of the specified type.
    do {
        // Get the position of a piece.
        const Square From = FromIt.next();
        const bool IsPromotableFrom = Type != PTK_Pawn && Type != PTK_Knight &&
                                      PromotableBB[C].isSet(From);
        assert(checkRange(From));

        BitIterator ToIt(TargetSquares & getAttackBB<C, Type>(From));
        while (!ToIt.end()) {
            // Get a possible destination.
            const Square To = ToIt.next();
            assert(checkRange(To));

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
        }
    } while (!FromIt.end());

    // Return the pointer to the next empty slot in the Moves array.
    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardLanceMovesImpl(const State& S, Move32* Moves,
                                         const Bitboard& TargetSquares,
                                         const Bitboard& OccupiedBB) {
    BitIterator FromIt(S.getBitboard<C, PTK_Lance>());
    while (!FromIt.end()) {
        const Square From = FromIt.next();
        assert(checkRange(From));

        BitIterator ToIt(getLanceAttackBB<C>(From, OccupiedBB) & TargetSquares);
        while (!ToIt.end()) {
            const Square To = ToIt.next();
            assert(checkRange(To));

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
                    continue;
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
        }
    }

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardBishopMovesImpl(const State& S, Move32* Moves,
                                          const Bitboard& TargetSquares,
                                          const Bitboard& OccupiedBB) {
    static_assert(Type == PTK_Bishop || Type == PTK_ProBishop,
                  "Type should be PTK_Bishop or PTK_ProBishop.");

    BitIterator FromIt(S.getBitboard<C, Type>());
    while (!FromIt.end()) {
        const Square From = FromIt.next();
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);
        assert(checkRange(From));

        // Get the bitboard of possible move destinations for the current piece.
        Bitboard ToBB =
            getBishopAttackBB<Type>(From, OccupiedBB) & TargetSquares;

        // Loop through each possible destination.
        BitIterator ToIt(std::move(ToBB));
        while (!ToIt.end()) {
            const Square To = ToIt.next();
            assert(checkRange(To));

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
                        continue;
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
        }
    }

    return Moves;
}

template <Color C, PieceTypeKind Type, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardRookMovesImpl(const State& S, Move32* Moves,
                                        const Bitboard& TargetSquares,
                                        const Bitboard& OccupiedBB) {
    static_assert(Type == PTK_Rook || Type == PTK_ProRook,
                  "Type should be PTK_Rook or PTK_ProRook.");

    BitIterator FromIt(S.getBitboard<C, Type>());
    while (!FromIt.end()) {
        const Square From = FromIt.next();
        const bool IsPromotableFrom = PromotableBB[C].isSet(From);
        assert(checkRange(From));

        // Get the bitboard of possible move destinations for the current piece.
        Bitboard ToBB = getRookAttackBB<Type>(From, OccupiedBB) & TargetSquares;

        // Loop through each possible destination.
        BitIterator ToIt(std::move(ToBB));
        while (!ToIt.end()) {
            const Square To = ToIt.next();
            assert(checkRange(To));

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
                        continue;
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
        }
    }

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

        Bitboard PawnExistFilesBB = Bitboard::ZeroBB();
        if (!(PawnBB & FileBB[File1]).isZero()) {
            PawnExistFilesBB |= FileBB[File1];
        }
        if (!(PawnBB & FileBB[File2]).isZero()) {
            PawnExistFilesBB |= FileBB[File2];
        }
        if (!(PawnBB & FileBB[File3]).isZero()) {
            PawnExistFilesBB |= FileBB[File3];
        }
        if (!(PawnBB & FileBB[File4]).isZero()) {
            PawnExistFilesBB |= FileBB[File4];
        }
        if (!(PawnBB & FileBB[File5]).isZero()) {
            PawnExistFilesBB |= FileBB[File5];
        }
        if (!(PawnBB & FileBB[File6]).isZero()) {
            PawnExistFilesBB |= FileBB[File6];
        }
        if (!(PawnBB & FileBB[File7]).isZero()) {
            PawnExistFilesBB |= FileBB[File7];
        }
        if (!(PawnBB & FileBB[File8]).isZero()) {
            PawnExistFilesBB |= FileBB[File8];
        }
        if (!(PawnBB & FileBB[File9]).isZero()) {
            PawnExistFilesBB |= FileBB[File9];
        }

        BitIterator It(~PawnExistFilesBB & ~FurthermostBB[C] & TargetSquares);
        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Pawn);
        }
    }

    const bool LanceExists = getStandCount<PTK_Lance>(St) > 0;
    const bool KnightExists = getStandCount<PTK_Knight>(St) > 0;
    const bool SilverExists = getStandCount<PTK_Silver>(St) > 0;
    const bool GoldExists = getStandCount<PTK_Gold>(St) > 0;
    const bool BishopExists = getStandCount<PTK_Bishop>(St) > 0;
    const bool RookExists = getStandCount<PTK_Rook>(St) > 0;

    /* if (Stands exist) */ {
        // Dropping move onto squares where no rule violation occurs.
        BitIterator It(TargetSquares & ~FirstAndSecondFurthestBB[C]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            if (LanceExists) {
                *List++ = Move32::droppingMove(To, PTK_Lance);
            }
            if (KnightExists) {
                *List++ = Move32::droppingMove(To, PTK_Knight);
            }
            if (SilverExists) {
                *List++ = Move32::droppingMove(To, PTK_Silver);
            }
            if (GoldExists) {
                *List++ = Move32::droppingMove(To, PTK_Gold);
            }
            if (BishopExists) {
                *List++ = Move32::droppingMove(To, PTK_Bishop);
            }
            if (RookExists) {
                *List++ = Move32::droppingMove(To, PTK_Rook);
            }
        }
    }

    if (LanceExists || SilverExists || GoldExists || BishopExists ||
        RookExists) {
        // Dropping move onto the second-furthest squares.
        BitIterator It(TargetSquares & SecondFurthestBB[C]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            // The piece type knight cannot be placed on the second-furthermost
            // squares. if (KnightExists) {
            //     *List++ = Move32::droppingMove(To, PTK_Knight);
            // }
            if (LanceExists) {
                *List++ = Move32::droppingMove(To, PTK_Lance);
            }
            if (SilverExists) {
                *List++ = Move32::droppingMove(To, PTK_Silver);
            }
            if (GoldExists) {
                *List++ = Move32::droppingMove(To, PTK_Gold);
            }
            if (BishopExists) {
                *List++ = Move32::droppingMove(To, PTK_Bishop);
            }
            if (RookExists) {
                *List++ = Move32::droppingMove(To, PTK_Rook);
            }
        }
    }

    if (SilverExists || GoldExists || BishopExists || RookExists) {
        // Dropping move onto the furthermost squares.
        BitIterator It(TargetSquares & FurthermostBB[C]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            // The piece type knight or lance cannot be
            // placed on the second-furthermost squares.
            // if (KnightExists) {
            //     *List++ = Move32::droppingMove(To, PTK_Knight);
            // }
            // if (LanceExists) {
            //     *List++ = Move32::droppingMove(To, PTK_Lance);
            // }
            if (SilverExists) {
                *List++ = Move32::droppingMove(To, PTK_Silver);
            }
            if (GoldExists) {
                *List++ = Move32::droppingMove(To, PTK_Gold);
            }
            if (BishopExists) {
                *List++ = Move32::droppingMove(To, PTK_Bishop);
            }
            if (RookExists) {
                *List++ = Move32::droppingMove(To, PTK_Rook);
            }
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
                BitIterator It(TargetBB & ((~FurthermostBB[White] & SquareBB[OpKingSq]).getRightShiftEpi64<1>()));
                while (!It.end()) {
                    Square To = It.next();
                    assert(checkRange(To));

                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                }
            } else {
                BitIterator It(TargetBB & ((~FurthermostBB[Black] & SquareBB[OpKingSq]).getLeftShiftEpi64<1>()));
                while (!It.end()) {
                    Square To = It.next();
                    assert(checkRange(To));

                    *List++ = Move32::droppingMove(To, PTK_Pawn);
                }
            }
        }
    }

    if (getStandCount<PTK_Knight>(St) > 0) {
        BitIterator It(TargetBB & KnightAttackBB[~C][OpKingSq]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Knight);
        }
    }

    if (getStandCount<PTK_Silver>(St) > 0) {
        BitIterator It(TargetBB & SilverAttackBB[~C][OpKingSq]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Silver);
        }
    }

    if (getStandCount<PTK_Gold>(St) > 0) {
        BitIterator It(TargetBB & GoldAttackBB[~C][OpKingSq]);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Gold);
        }
    }

    return List;
}

template <Color C>
inline Move32* generateDroppingSliderCheckMovesImpl(const State& S, const Square OpKingSq, Move32* List,
                                     const Bitboard& TargetBB, const Bitboard& OccupiedBB) {
    const Stands St = S.getPosition().getStand<C>();

    if (getStandCount<PTK_Lance>(St) > 0) {
        BitIterator It(getLanceAttackBB<~C>(OpKingSq, OccupiedBB) & TargetBB);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Lance);
        }
    }

    if (getStandCount<PTK_Rook>(St) > 0) {
        BitIterator It(getRookAttackBB<PTK_Rook>(OpKingSq, OccupiedBB) & TargetBB);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Rook);
        }
    }

    if (getStandCount<PTK_Bishop>(St) > 0) {
        BitIterator It(getBishopAttackBB<PTK_Bishop>(OpKingSq, OccupiedBB) & TargetBB);

        while (!It.end()) {
            Square To = It.next();
            assert(checkRange(To));

            *List++ = Move32::droppingMove(To, PTK_Bishop);
        }
    }

    return List;
}

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardOneStepNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& SourceFilter) {
    static_assert(Type != PTK_Lance,     "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Bishop,    "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Rook,      "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook,   "PTK_ProRook must not be passed as `Type` in generateCheckStepMoves().");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, Type>(OpKingSq))
                                      : getAttackBB<~C, Type>(OpKingSq))
                             & getAttackBB<C, Type>(From) & TargetBB;

        if constexpr (Type == PTK_Pawn) {
            if constexpr (WilyPromote) {
                TargetBB2 &= ~PromotableBB[C];
            } else {
                TargetBB2 &= ~FurthermostBB[C];
            }
        }

        if constexpr (Type == PTK_Knight) {
            TargetBB2 &= ~FirstAndSecondFurthestBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, bool Capture, bool WilyPromote, bool Pinned>
inline Move32* generateOnBoardLanceNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        Bitboard TargetBB2 = (Pinned)? ~LineBB[From][OpKingSq] : getLanceAttackBB<~C>(OpKingSq, OccupiedBB);

        if constexpr (WilyPromote) {
            TargetBB2 &= ~FirstAndSecondFurthestBB[C];
        } else {
            TargetBB2 &= ~FurthermostBB[C];
        }

        TargetBB2 = TargetBB2 & getLanceAttackBB<C>(From, OccupiedBB) & TargetBB;

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardBishopNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type == PTK_Bishop || Type == PTK_ProBishop, "Only PTK_Bishop or PTK_ProBishop can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        if constexpr (WilyPromote && Type == PTK_Bishop) {
            if (PromotableBB[C].isSet(From)) {
                continue;
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
            TargetBB2 &= ~PromotableBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, PieceTypeKind Type, bool Capture, bool Pinned, bool WilyPromote>
inline Move32* generateOnBoardRookNoPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    static_assert(Type == PTK_Rook || Type == PTK_ProRook, "Only PTK_Rook or PTK_ProRook can be processed in this function.");

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        if constexpr (WilyPromote && Type == PTK_Rook) {
            if (PromotableBB[C].isSet(From)) {
                continue;
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
            TargetBB2 &= ~PromotableBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardMove(From, To, Type);
            }
        }
    }

    return Moves;
}


template<Color C, PieceTypeKind Type, bool Capture, bool Pinned>
inline Move32* generateOnBoardOneStepPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& SourceFilter) {
    static_assert(Type != PTK_Lance,     "PTK_Lance must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Bishop,    "PTK_Bishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_Rook,      "PTK_Rook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProBishop, "PTK_ProBishop must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(Type != PTK_ProRook,   "PTK_ProRook must not be passed as `Type` in generateCheckStepMoves().");
    static_assert(!isPromoted(Type),     "Type must be an unpromoted piece type.");

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, PTK_Gold>(OpKingSq))
                                    : getAttackBB<~C, PTK_Gold>(OpKingSq))
                             & getAttackBB<C, Type>(From) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardLancePromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Lance;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getAttackBB<~C, PTK_Gold>(OpKingSq))
                                    : getAttackBB<~C, PTK_Gold>(OpKingSq))
                             & getLanceAttackBB<C>(From, OccupiedBB) & TargetBB & PromotableBB[C];

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardBishopPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Bishop;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getBishopAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq))
                                    : (getBishopAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq)))
                             & getBishopAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template<Color C, bool Capture, bool Pinned>
inline Move32* generateOnBoardRookPromoteCheckMovesImpl(const State& S, const Square OpKingSq, Move32* Moves, const Bitboard& TargetBB, const Bitboard& OccupiedBB, const Bitboard& SourceFilter) {
    constexpr PieceTypeKind Type = PTK_Rook;

    if ((S.getBitboard<C, Type>() & SourceFilter).isZero()) {
        return Moves;
    }

    BitIterator FromIt(S.getBitboard<C, Type>() & SourceFilter);

    while (!FromIt.end()) {
        const Square From = FromIt.next();

        // clang-format off
        Bitboard TargetBB2 = (Pinned? (~LineBB[From][OpKingSq] | getRookAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq))
                                    : (getRookAttackBB<Type>(OpKingSq, OccupiedBB) | getAttackBB<~C, PTK_King>(OpKingSq)))
                             & getRookAttackBB<Type>(From, OccupiedBB) & TargetBB;

        if (!PromotableBB[C].isSet(From)) {
            TargetBB2 &= PromotableBB[C];
        }

        // clang-format on
        BitIterator ToIt(std::move(TargetBB2));

        while (!ToIt.end()) {
            const Square To = ToIt.next();

            if constexpr (Capture) {
                const PieceTypeKind CaptureType =
                    getPieceType(S.getPosition().pieceOn(To));
                *Moves++ = Move32::boardPromotingMove(From, To, Type, CaptureType);
            } else {
                *Moves++ = Move32::boardPromotingMove(From, To, Type);
            }
        }
    }

    return Moves;
}

template <Color C, bool Capture, bool WilyPromote = true>
inline Move32* generateOnBoardOneStepMovesImpl(const State& S, Move32* Moves,
                                           const Bitboard& TargetSquares) {
    Moves = generateOnBoardOneStepPawnMovesImpl<C, Capture, WilyPromote>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Gold, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProPawn, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProLance, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProKnight, Capture>(S, Moves, TargetSquares);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProSilver, Capture>(S, Moves, TargetSquares);

    return Moves;
}

template <Color C, bool Capture, bool Pinned, bool WilyPromote, bool SkipKing = false>
inline Move32* generateOnBoardOneStepCheckMovesImpl(const State& S, Move32* Moves, const Square OpKingSq, const Bitboard& TargetBB, const Bitboard& FromMask) {
    if constexpr (!SkipKing) {
        Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    }

    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Pawn, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Knight, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Silver, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_Gold, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_ProPawn, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_ProLance, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_ProKnight, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_ProSilver, Capture, Pinned, WilyPromote>(S, OpKingSq, Moves, TargetBB, FromMask);

    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Pawn, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Knight, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, FromMask);
    Moves = generateOnBoardOneStepPromoteCheckMovesImpl<C, PTK_Silver, Capture, Pinned>(S, OpKingSq, Moves, TargetBB, FromMask);

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
inline Move32* generatePossiblyLegalEvasionMovesImpl(const State& S, Move32* Moves,
                                                 const Bitboard& CheckerBB,
                                                 const Bitboard& OpponentBB,
                                                 const Bitboard& OccupiedBB) {
    // Moving the king.
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, true>(S, Moves, OpponentBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_King, false>(S, Moves, ~OccupiedBB);

    // If there are more than one checkers,
    // no moves available but moving the king.
    if (CheckerBB.popCount() >= 2) {
        return Moves;
    }

    // Capturing a checker.
    Moves = generateOnBoardOneStepPawnMovesImpl<C, true, WilyPromote>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Knight, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Silver, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_Gold, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProPawn, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProLance, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProKnight, true>(S, Moves, CheckerBB);
    Moves = generateOnBoardOneStepMovesImpl<C, PTK_ProSilver, true>(S, Moves, CheckerBB);

    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(S, Moves, CheckerBB,
                                                            OccupiedBB);

    const Square CheckerSq = CheckerBB.getOne();
    const Bitboard BetweenBB = getBetweenBB(S.getKingSquare<C>(), CheckerSq);

    if (BetweenBB.isZero()) {
        return Moves;
    }

    // Moving a piece.
    Moves =
        generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves, BetweenBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(S, Moves, BetweenBB,
                                                             OccupiedBB);

    // Dropping a piece (a.k.a. aigoma).
    Moves = generateDroppingMovesImpl<C>(S, Moves, BetweenBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generatePossiblyLegalMovesImpl(const State& S, Move32* Moves,
                                              const Bitboard& OpponentBB,
                                              const Bitboard& OccupiedBB) {
    // Captures.
    Moves =
        generateOnBoardOneStepMovesImpl<C, true, WilyPromote>(S, Moves, OpponentBB);
    Moves = generateOnBoardSliderMovesImpl<C, true, WilyPromote>(S, Moves, OpponentBB,
                                                            OccupiedBB);

    const Bitboard EmptyBB = ~OccupiedBB;
    // Droppings.
    Moves = generateDroppingMovesImpl<C>(S, Moves, EmptyBB);

    // No captures.
    Moves = generateOnBoardOneStepMovesImpl<C, false, WilyPromote>(S, Moves, EmptyBB);
    Moves = generateOnBoardSliderMovesImpl<C, false, WilyPromote>(S, Moves, EmptyBB,
                                                             OccupiedBB);

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generatePossiblyLegalMovesImpl(const State& S, Move32* Moves) {
    const Bitboard& CheckerBB = S.getCheckerBB();
    const Bitboard& BlackBB = S.getBitboard<Black>();
    const Bitboard& WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;

    if constexpr (C == Black) {
        if (!CheckerBB.isZero()) {
            Moves = generatePossiblyLegalEvasionMovesImpl<Black, WilyPromote>(
                S, Moves, CheckerBB, WhiteBB, OccupiedBB);
        } else {
            Moves = generatePossiblyLegalMovesImpl<Black, WilyPromote>(
                S, Moves, WhiteBB, OccupiedBB);
        }
    } else {
        if (!CheckerBB.isZero()) {
            Moves = generatePossiblyLegalEvasionMovesImpl<White, WilyPromote>(
                S, Moves, CheckerBB, BlackBB, OccupiedBB);
        } else {
            Moves = generatePossiblyLegalMovesImpl<White, WilyPromote>(
                S, Moves, BlackBB, OccupiedBB);
        }
    }

    return Moves;
}

template <Color C, bool WilyPromote>
inline Move32* generatePossiblyLegalCheckMovesImpl(const State& S, Move32* Moves) {
    const Bitboard& BlackBB = S.getBitboard<Black>();
    const Bitboard& WhiteBB = S.getBitboard<White>();
    const Bitboard OccupiedBB = BlackBB | WhiteBB;
    const Bitboard EmptyBB = ~OccupiedBB;
    const Bitboard& CheckerBB = S.getCheckerBB();
    const Bitboard& PinnedBB = S.getDefendingOpponentSliderBB<~C>();
    const Bitboard NoPinnedBB = ~PinnedBB;
    const Square MyKingSq = S.getKingSquare<C>();
    const Square OpKingSq = S.getKingSquare<~C>();

    if (CheckerBB.popCount() >= 2) {
        if (PinnedBB.isSet(S.getKingSquare<C>())) {
            const Bitboard& KingLineBB = LineBB[MyKingSq][OpKingSq];
            const Bitboard UnpinEmptyBB = EmptyBB & ~KingLineBB;
            const Bitboard UnpinCaptureBB = S.getBitboard<~C>() & ~KingLineBB;

            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, false, true, WilyPromote>(S, OpKingSq, Moves, UnpinEmptyBB, PinnedBB);
            Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, true, true, WilyPromote>(S, OpKingSq, Moves, UnpinCaptureBB, PinnedBB);
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

        const Bitboard& CheckerMyKingBetweenBB = BetweenBB[CheckerSq][MyKingSq];

        if (PinnedExists) {
            if (PinnedBB.isSet(MyKingSq)) {
                const Bitboard& KingLineBB = LineBB[MyKingSq][OpKingSq];
                const Bitboard UnpinEmptyBB = EmptyBB & ~KingLineBB;
                const Bitboard UnpinCaptureBB = S.getBitboard<~C>() & ~KingLineBB;

                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, false, true, WilyPromote>(S, OpKingSq, Moves, UnpinEmptyBB, PinnedBB);
                Moves = generateOnBoardOneStepNoPromoteCheckMovesImpl<C, PTK_King, true, true, WilyPromote>(S, OpKingSq, Moves, UnpinCaptureBB, PinnedBB);
            }

            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true, WilyPromote, true>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true, WilyPromote, true>(S, Moves, OpKingSq, CheckerBB, PinnedBB);

            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, PinnedBB);
        }

        Moves = generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, NoPinnedBB);
        Moves = generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, CheckerBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, CheckerMyKingBetweenBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, CheckerBB, OccupiedBB, NoPinnedBB);
    } else {
        if (St != 0) {
            Moves = generateDroppingStepCheckMovesImpl<C>(S, OpKingSq, Moves, EmptyBB);
            Moves = generateDroppingSliderCheckMovesImpl<C>(S, OpKingSq, Moves, EmptyBB, OccupiedBB);
        }

        if (PinnedExists) {
            Moves = generateOnBoardOneStepCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, EmptyBB, PinnedBB);
            Moves = generateOnBoardOneStepCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, false, true, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, PinnedBB);
            Moves = generateOnBoardSliderCheckMovesImpl<C, true, true, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, PinnedBB);
        }

        Moves = generateOnBoardOneStepCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, EmptyBB, NoPinnedBB);
        Moves = generateOnBoardOneStepCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, false, false, WilyPromote>(S, Moves, OpKingSq, EmptyBB, OccupiedBB, NoPinnedBB);
        Moves = generateOnBoardSliderCheckMovesImpl<C, true, false, WilyPromote>(S, Moves, OpKingSq, S.getBitboard<~C>(), OccupiedBB, NoPinnedBB);
    }

    return Moves;
}

template <GenerateType GenType, Color C, bool WilyPromote>
inline Move32* generateLegalMovesImpl(const State& S, Move32* Moves) {
    Move32* const MovesHead = Moves;

    if constexpr (GenType == GenerateType::Normal) {
        Moves = generatePossiblyLegalMovesImpl<C, WilyPromote>(S, Moves);
    } else if constexpr (GenType == GenerateType::Check) {
        Moves = generatePossiblyLegalCheckMovesImpl<C, WilyPromote>(S, Moves);
    }

    Move32* Reader = MovesHead;
    Move32* Writer = MovesHead;

    while (Reader != Moves) {
        if (S.isLegal<C>(*Reader)) {
            *Writer = *Reader;
            ++Writer;
        }

        ++Reader;
    }

    return Writer;
}

} // namespace

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generatePossiblyLegalCheckMoves(const State& S) {
    MoveList List;
    List.Tail = generatePossiblyLegalCheckMovesImpl<C, WilyPromote>(S, List.Tail);
    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generatePossiblyLegalMoves(const State& S) {
    MoveList List;

    List.Tail = (S.getPosition().sideToMove() == Black)
                    ? generatePossiblyLegalMovesImpl<Black, WilyPromote>(S, List.Tail)
                    : generatePossiblyLegalMovesImpl<White, WilyPromote>(S, List.Tail);

    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generatePossiblyLegalCheckMoves(const State& S) {
    return (S.getPosition().sideToMove() == Black)
                ? generatePossiblyLegalCheckMoves<Black, WilyPromote>(S)
                : generatePossiblyLegalCheckMoves<White, WilyPromote>(S);
}

template <bool WilyPromote>
MoveList MoveGenerator::generatePossiblyLegalEvasionMoves(const State& S) {
    return (S.getPosition().sideToMove() == Black)
                ? generatePossiblyLegalEvasionMoves<Black, WilyPromote>(S)
                : generatePossiblyLegalEvasionMoves<White, WilyPromote>(S);
}

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generatePossiblyLegalEvasionMoves(const State& S) {
    MoveList List;
    const Bitboard& CheckerBB = S.getCheckerBB();
    const Bitboard& MyBB = S.getBitboard<C>();
    const Bitboard& OpponentBB = S.getBitboard<~C>();
    const Bitboard OccupiedBB = MyBB | OpponentBB;

    List.Tail = generatePossiblyLegalEvasionMovesImpl<C, WilyPromote>(S, List.Tail, CheckerBB, OpponentBB, OccupiedBB);
    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) {
    MoveList List;

    List.Tail = (S.getPosition().sideToMove() == Black)
                    ? generateLegalMovesImpl<GenerateType::Normal, Black, WilyPromote>(S, List.Tail)
                    : generateLegalMovesImpl<GenerateType::Normal, White, WilyPromote>(S, List.Tail);

    return List;
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) {
    MoveList List;

    List.Tail = (S.getPosition().sideToMove() == Black)
                    ? generateLegalMovesImpl<GenerateType::Check, Black, WilyPromote>(S, List.Tail)
                    : generateLegalMovesImpl<GenerateType::Check, White, WilyPromote>(S, List.Tail);

    return List;
}

template MoveList
MoveGenerator::generatePossiblyLegalMoves<false>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalMoves<true>(const State& S);

template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<false>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<core::Black, true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<core::White, true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<core::Black, false>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalCheckMoves<core::White, false>(const State& S);

template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<false>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<core::Black, true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<core::White, true>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<core::Black, false>(const State& S);
template MoveList
MoveGenerator::generatePossiblyLegalEvasionMoves<core::White, false>(const State& S);

template MoveList MoveGenerator::generateLegalMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<true>(const State& S);

template MoveList MoveGenerator::generateLegalCheckMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<true>(const State& S);


} // namespace core
} // namespace nshogi
