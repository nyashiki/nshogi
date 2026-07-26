//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "stateimpl.h"
#include "../types.h"
#include "bitboard.h"
#include "statehelper.h"
#include "utils.h"

#include <cassert>
#include <cstring>
#include <utility>

namespace nshogi {
namespace core {
namespace internal {

namespace {

template <Color C>
bool canDeclareImpl(const StateImpl& S) noexcept {
    if (!bitboard::PromotableBB[C].isSet(S.getKingSquare<C>())) {
        return false;
    }

    const bitboard::Bitboard EnteringPiecesBB =
        (S.getBitboard<C>() ^ S.getBitboard<C, PTK_King>()) &
        bitboard::PromotableBB[C];

    if (EnteringPiecesBB.popCount() < 10) {
        return false;
    }

    if (!S.getCheckerBB().isZero()) {
        return false;
    }

    constexpr uint8_t Criteria = (C == Black) ? 28 : 27;

    if (S.computeDeclarationScore<C>() >= Criteria) {
        return true;
    }

    return false;
}

} // namespace

StateImpl::StateImpl(const Position& P)
    : Pos(P)
    , Helper(P) {
}

StateImpl::StateImpl(const Position& P, uint16_t Ply)
    : Pos(P, Ply)
    , Helper(P, Ply) {
}

StateImpl::StateImpl(const Position& CurrentP, const Position& InitP)
    : Pos(CurrentP)
    , Helper(InitP) {
}

StateImpl::StateImpl(StateImpl&& S) noexcept
    : Pos(S.Pos)
    , Helper(std::move(S.Helper))
    , HashValue(S.HashValue) {
}

StateImpl StateImpl::clone() const {
    StateImpl S(Pos, getInitialPosition());

    S.HashValue = HashValue;

    // Clone helper.
    S.Helper.Ply = Helper.Ply;

    std::memcpy(reinterpret_cast<char*>(S.Helper.ColorBB),
                reinterpret_cast<const char*>(Helper.ColorBB),
                NumColors * sizeof(bitboard::Bitboard));

    std::memcpy(reinterpret_cast<char*>(S.Helper.TypeBB),
                reinterpret_cast<const char*>(Helper.TypeBB),
                NumPieceType * sizeof(bitboard::Bitboard));

    std::memcpy(reinterpret_cast<char*>(S.Helper.KingSquare),
                reinterpret_cast<const char*>(Helper.KingSquare),
                NumColors * sizeof(Square));

    S.Helper.SHelper.resize(Helper.SHelper.size());
    std::memcpy(reinterpret_cast<char*>(S.Helper.SHelper.data()),
                reinterpret_cast<const char*>(Helper.SHelper.data()),
                Helper.SHelper.size() * sizeof(StepHelper));

    return S;
}

template <Color C>
inline void StateImpl::doMove(Move32 Move) noexcept {
    Helper.proceedOneStep(Move, HashValue.getValue(),
                          getPosition().getStand<Black>(),
                          getPosition().getStand<White>());

    const StepHelper* PrevStepHelper = &Helper.getStepHelper(Helper.Ply - 1);
    StepHelper* CurrentStepHelper = &Helper.SHelper[Helper.Ply];

    const Square To = Move.to();
    const Square From = Move.from();
    const PieceTypeKind Type = Move.pieceType();

    if (Move.drop()) { // When the move is dropping.
        // Decrement the number of the standing piece one has.
        Pos.decrementStand(C, Type);

        // And put the piece on the board.
        Pos.putPiece(To, makePiece<C>(Type));

        Helper.ColorBB[C].toggleBit(To);
        Helper.TypeBB[Type].toggleBit(To);

        HashValue.update<C>(Type, To);

        if (Type == PTK_Pawn) {
            CurrentStepHelper->IsLastMoveDroppingAPawn = true;
        }
    } else { // Move a piece on the board, which means not a dropping move.
        const PieceTypeKind CaptureType = Move.capturePieceType();

        if (CaptureType != PTK_Empty) {
            assert(CaptureType != PTK_King);

            // Capturing an opponent piece occurs.
            // Then increment the number of standing piece.
            Pos.incrementStand(C, demotePieceType(CaptureType));
            Pos.removePiece(To);

            Helper.ColorBB[~C].toggleBit(To);
            Helper.TypeBB[CaptureType].toggleBit(To);

            HashValue.update<~C>(CaptureType, To);
        }

        Helper.ColorBB[C].toggleBit(From);
        Helper.TypeBB[Type].toggleBit(From);

        HashValue.update<C>(Type, From);

        const PieceKind Piece = Pos.removePiece(From);

        if (Move.promote()) {
            const PieceKind PromotedPiece = promotePiece(Piece);
            const PieceTypeKind PromotedType = promotePieceType(Type);

            Pos.putPiece(To, PromotedPiece);

            Helper.TypeBB[PromotedType].toggleBit(To);
            HashValue.update<C>(PromotedType, To);
        } else {
            Pos.putPiece(To, Piece);

            Helper.TypeBB[Type].toggleBit(To);
            HashValue.update<C>(Type, To);
        }
        Helper.ColorBB[C].toggleBit(To);

        if (Type == PTK_King) {
            Helper.KingSquare[C] = To;
        }
    }

    HashValue.updateColor();

    const bitboard::Bitboard OccupiedBB =
        getBitboard<Black>() | getBitboard<White>();

    // Update opponent's CheckerBB as it will be the opponent's turn next.
    // Instead of calling `setStepCheckerBB<~C>()`,
    // we can update CheckerBB by only considering the move just made,
    // which is more efficient.
    if (Move.promote()) {
        CurrentStepHelper->CheckerBB =
            bitboard::getStepAttackBB<~C>(promotePieceType(Type),
                                          getKingSquare<~C>()) &
            bitboard::SquareBB[To];
    } else {
        CurrentStepHelper->CheckerBB =
            bitboard::getStepAttackBB<~C>(Type, getKingSquare<~C>()) &
            bitboard::SquareBB[To];
    }
    setDefendingOpponentSliderBBAndSliderCheckerBB<C, false>(CurrentStepHelper,
                                                             OccupiedBB);
    setDefendingOpponentSliderBBAndSliderCheckerBB<~C, true>(CurrentStepHelper,
                                                             OccupiedBB);

    if (!CurrentStepHelper->CheckerBB.isZero()) {
        CurrentStepHelper->ContinuousCheckCounts[C] =
            PrevStepHelper->ContinuousCheckCounts[C] + 1;
    } else {
        CurrentStepHelper->ContinuousCheckCounts[C] = 0;
    }
    CurrentStepHelper->ContinuousCheckCounts[~C] =
        PrevStepHelper->ContinuousCheckCounts[~C];

    // And now it is opponent's turn.
    Pos.changeSideToMove();
}

void StateImpl::doMove(Move32 Move) noexcept {
    if (getPosition().sideToMove() == Black) {
        doMove<Black>(Move);
    } else {
        doMove<White>(Move);
    }
}

template <Color C>
inline void StateImpl::undoMove() {
    const Move32 PrevMove = Helper.goBackOneStep();
    Pos.changeSideToMove();

    const Square To = PrevMove.to();
    const Square From = PrevMove.from();

    if (PrevMove.drop()) {
        const PieceTypeKind Type = PrevMove.pieceType();

        Pos.incrementStand(~C, Type);
        Pos.removePiece(To);

        Helper.ColorBB[~C].toggleBit(To);
        Helper.TypeBB[Type].toggleBit(To);

        HashValue.update<~C>(Type, To);
    } else { // Move a piece on the board, which means not a dropping move.
        const PieceTypeKind CaptureType = PrevMove.capturePieceType();
        const PieceTypeKind Type = PrevMove.pieceType();

        Pos.removePiece(To);
        if (CaptureType != PTK_Empty) {
            // Capturing occurs.
            Pos.decrementStand(~C, demotePieceType(CaptureType));
            Pos.putPiece(To, makePiece<C>(CaptureType));

            Helper.ColorBB[C].toggleBit(To);
            Helper.TypeBB[CaptureType].toggleBit(To);

            HashValue.update<C>(CaptureType, To);
        }

        Helper.ColorBB[~C].toggleBit(To);
        if (PrevMove.promote()) {
            const PieceTypeKind PromotedType = promotePieceType(Type);

            Helper.TypeBB[PromotedType].toggleBit(To);
            HashValue.update<~C>(PromotedType, To);
        } else {
            Helper.TypeBB[Type].toggleBit(To);
            HashValue.update<~C>(Type, To);
        }

        Pos.putPiece(From, makePiece<~C>(Type));
        Helper.TypeBB[Type].toggleBit(From);
        Helper.ColorBB[~C].toggleBit(From);

        HashValue.update<~C>(Type, From);

        if (Type == PTK_King) {
            Helper.KingSquare[~C] = From;
        }
    }

    HashValue.updateColor();

    assert((getBitboard<Black>() & getBitboard<White>()).isZero());
}

void StateImpl::undoMove() {
    if (getPosition().sideToMove() == Black) {
        undoMove<Black>();
    } else {
        undoMove<White>();
    }
}

void StateImpl::refresh() noexcept {
    Helper.Ply = 0;
    std::memset(static_cast<void*>(Helper.ColorBB), 0,
                NumColors * sizeof(bitboard::Bitboard));
    std::memset(static_cast<void*>(Helper.TypeBB), 0,
                NumPieceType * sizeof(bitboard::Bitboard));
    Helper.KingSquare[Black] = Sq1I;
    Helper.KingSquare[White] = Sq1I;

    StepHelper* CurrentStepHelper = &Helper.SHelper[0];

    // First, clear all bitboards.
    std::memset(static_cast<void*>(CurrentStepHelper), 0, sizeof(StepHelper));

    // Set bitboards.
    for (Square Sq : Squares) {
        if (Pos.OnBoard[Sq] != PK_Empty) {
            Color C = getColor(Pos.OnBoard[Sq]);
            PieceTypeKind Type = getPieceType(Pos.OnBoard[Sq]);

            Helper.ColorBB[C].toggleBit(Sq);
            Helper.TypeBB[Type].toggleBit(Sq);
        }

        if (Pos.OnBoard[Sq] == PK_BlackKing) {
            Helper.KingSquare[Black] = Sq;
        } else if (Pos.OnBoard[Sq] == PK_WhiteKing) {
            Helper.KingSquare[White] = Sq;
        }
    }

    // Non-trivial bitboards.
    const bitboard::Bitboard OccupiedBB =
        getBitboard<Black>() | getBitboard<White>();

    if (getPosition().sideToMove() == Black) {
        setStepCheckerBB<Black>(CurrentStepHelper);
        setDefendingOpponentSliderBBAndSliderCheckerBB<Black, true>(
            CurrentStepHelper, OccupiedBB);
        setDefendingOpponentSliderBBAndSliderCheckerBB<White, false>(
            CurrentStepHelper, OccupiedBB);
    } else {
        setStepCheckerBB<White>(CurrentStepHelper);
        setDefendingOpponentSliderBBAndSliderCheckerBB<Black, false>(
            CurrentStepHelper, OccupiedBB);
        setDefendingOpponentSliderBBAndSliderCheckerBB<White, true>(
            CurrentStepHelper, OccupiedBB);
    }

    HashValue.refresh(getPosition());

    CurrentStepHelper->ContinuousCheckCounts[Black] = 0;
    CurrentStepHelper->ContinuousCheckCounts[White] = 0;
    CurrentStepHelper->EachStand[Black] = getPosition().getStand<Black>();
    CurrentStepHelper->EachStand[White] = getPosition().getStand<White>();

    if (!CurrentStepHelper->CheckerBB.isZero()) {
        CurrentStepHelper->ContinuousCheckCounts[getPosition().sideToMove()] =
            1;
    }
}

template <Color C>
bool StateImpl::isLegal(Move16 Move) const noexcept {
    assert(!Move.isNone());
    assert(!Move.isNull());

    const auto& P = getPosition();

    const bitboard::Bitboard& CheckerBB = getCheckerBB();

    if (Move.drop()) {
        const PieceTypeKind Type =
            (PieceTypeKind)(Move.from() - NumSquares + 1);

        if (Type >= NumPieceType) {
            return false;
        }

        if (Move.to() >= NumSquares) {
            return false;
        }

        // Check we have the piece to drop.
        if (P.getStandCount<C>(Type) == 0) {
            return false;
        }

        // Check the destination square is empty.
        if (P.pieceOn(Move.to()) != PK_Empty) {
            return false;
        }

        // When dropping a pawn, check the "nifu" rule.
        if (Type == PTK_Pawn) {
            const core::File File = squareToFile(Move.to());
            if (!(bitboard::FileBB[File] & getBitboard<C>(PTK_Pawn)).isZero()) {
                return false;
            }
        }

        // CHeck no-move-available rule.
        if (Type == PTK_Pawn || Type == PTK_Lance || Type == PTK_Knight) {
            if (bitboard::Bitboard::FurthermostBB<C>().isSet(Move.to())) {
                return false;
            }
            if (Type == PTK_Knight) {
                if (bitboard::Bitboard::SecondFurthestBB<C>().isSet(
                        Move.to())) {
                    return false;
                }
            }
        }

        // If in check, check if the drop move can evade the check.
        if (!CheckerBB.isZero()) {
            // If the checker is on an adjacent square to the king,
            // any dropping move is illegal.
            if (!(bitboard::KingAttackBB[getKingSquare<C>()] & CheckerBB)
                     .isZero()) {
                return false;
            }

            if (CheckerBB.popCount() == 1) {
                const Square CheckerSq = CheckerBB.getOne();

                // If the checker is a knight,
                // any dropping move is illegal.
                if (getPieceType(P.pieceOn(CheckerSq)) == PTK_Knight) {
                    return false;
                }

                // Here, we only need to check if the dropping move can
                // block the check by a slider.
                // A move blocks the ckeck iff the king square,
                // the checker square, and the move's destination square are
                // aligned on the same line and the destination square is
                // between the king square and the
                return bitboard::BetweenBB[CheckerSq][getKingSquare<C>()].isSet(
                    Move.to());
            } else { // CheckerBB.popCount() >= 2.
                // If there are two or more checkers, the only legal moves are
                // king moves. Therefore, any dropping move is illegal.
                return false;
            }
        }
    } else { // A move on the board.
        if (Move.from() >= NumSquares || Move.to() >= NumSquares) {
            return false;
        }

        const core::PieceKind Piece = P.pieceOn(Move.from());
        const core::PieceTypeKind Type = getPieceType(Piece);

        if (Type >= NumPieceType) {
            return false;
        }

        // Check if the source square has a piece of the side to move.
        if (Type == PTK_Empty || getColor(Piece) != C) {
            return false;
        }

        // Check if the destination square has no piece or an opponent's piece.
        if (P.pieceOn(Move.to()) != PK_Empty &&
            getColor(P.pieceOn(Move.to())) == C) {
            return false;
        }

        // Promotion is only possible if the either move's from or move's to
        // is in the promotion zone.
        if (Move.promote()) {
            // Check if the piece type is promotable.
            if (isPromoted(Type) || Type == PTK_King || Type == PTK_Gold) {
                return false;
            }

            const auto& PromotableBB = bitboard::PromotableBB[C];
            if ((PromotableBB & (bitboard::SquareBB[Move.from()] |
                                 bitboard::SquareBB[Move.to()]))
                    .isZero()) {
                return false;
            }
        } else {
            // CHeck no-move-available rule.
            if (Type == PTK_Pawn || Type == PTK_Lance || Type == PTK_Knight) {
                if (bitboard::Bitboard::FurthermostBB<C>().isSet(Move.to())) {
                    return false;
                }
                if (Type == PTK_Knight) {
                    if (bitboard::Bitboard::SecondFurthestBB<C>().isSet(
                            Move.to())) {
                        return false;
                    }
                }
            }
        }

        // Check if the piece can move to the destination from the source
        // square.
        if (!bitboard::getStepAttackBB<C>(Type, Move.from()).isSet(Move.to())) {
            // Check slider moves.
            if (Type == PTK_Bishop || Type == PTK_ProBishop ||
                Type == PTK_Rook || Type == PTK_ProRook || Type == PTK_Lance) {
                const bitboard::Bitboard OccupiedBB =
                    getBitboard<Black>() | getBitboard<White>();
                if (!bitboard::getSliderAttackBB<C>(Type, Move.from(),
                                                    OccupiedBB)
                         .isSet(Move.to())) {
                    return false;
                }
            } else {
                return false;
            }
        }

        // Check if the king is not in check after the move.
        if (!CheckerBB.isZero()) {
            if (CheckerBB.popCount() >= 2) {
                if (Type != PTK_King) {
                    // If there are two or more checkers, the only legal moves
                    // are king moves. Therefore, if the move is not a king
                    // move, it is illegal.
                    return false;
                }
                // Checking if the move can evade the check by moving the king
                // later (*).
            }

            // If the move captures the checker, the move is legal.
            // If not, the king must evade the check from step check
            // or block slider check.
            if (!CheckerBB.isSet(Move.to())) {
                if (Type != PTK_King) {
                    // Otherwise, if checked by a step piece from a adjacent
                    // square or a knight, the move is illegal.
                    if (!((bitboard::KingAttackBB[getKingSquare<C>()] |
                           bitboard::getAttackBB<C, PTK_Knight>(
                               getKingSquare<C>())) &
                          CheckerBB)
                             .isZero()) {
                        return false;
                    }

                    // If checked by a slider, the move is legal only if it can
                    // block the check.
                    const bitboard::Bitboard OccupiedBB =
                        getBitboard<Black>() | getBitboard<White>();
                    if (isAttackedBySlider<C>(getKingSquare<C>(), OccupiedBB,
                                              Move.to())) {
                        return false;
                    }
                }
            }
        }

        // (*) Check not in check after moving the king.
        if (Type == PTK_King) {
            return !isAttacked<C>(Move.to(), Move.from());
        }

        // Check if the king is not in check after moving a pinned piece.
        if (getDefendingOpponentSliderBB<C>().isSet(Move.from())) {
            // When moving a pinned piece, the move is legal only if
            // the source square, the destination square, and the king square
            // are aligned.
            return utils::isSameLine(getKingSquare<C>(), Move.from(),
                                     Move.to());
        }
    }

    return true;
}

template <Color C>
bool StateImpl::isLegal(Move32 Move) const noexcept {
    if (!Move.drop()) {
        if (Move.from() >= NumSquares || Move.to() >= NumSquares) {
            return false;
        }
        if (Move.pieceType() >= NumPieceType) {
            return false;
        }
        if (Move.capturePieceType() >= NumPieceType) {
            return false;
        }

        // Check if the source square has the piece type.
        // Note: piece color check will be done in isLegal(Move16).
        if (getPieceType(getPosition().pieceOn(Move.from())) !=
            Move.pieceType()) {
            return false;
        }

        if (Move.capturePieceType() != PTK_Empty) {
            // Check if the destination square has the opponent's piece type.
            const PieceKind CapturePiece = getPosition().pieceOn(Move.to());
            if (getColor(CapturePiece) == getSideToMove() ||
                getPieceType(CapturePiece) != Move.capturePieceType()) {
                return false;
            }
        }
    }

    return isLegal<C>(core::Move16(Move));
}

void StateImpl::doNullMove() noexcept {
    // This function must not be called when the king is in check.
    assert(getCheckerBB().isZero());

    Helper.proceedOneStep(Move32::MoveNull(), HashValue.getValue(),
                          getPosition().getStand<Black>(),
                          getPosition().getStand<White>());

    // Reset continuous check counts as the null move is not a checking move.
    const StepHelper* PrevStepHelper = &Helper.getStepHelper(Helper.Ply - 1);
    StepHelper* CurrentStepHelper = &Helper.SHelper[Helper.Ply];

    CurrentStepHelper->ContinuousCheckCounts[Black] = 0;
    CurrentStepHelper->ContinuousCheckCounts[White] = 0;
    CurrentStepHelper->CheckerBB.clear();

    CurrentStepHelper->DefendingOpponentSliderBB[Black] =
        PrevStepHelper->DefendingOpponentSliderBB[Black];
    CurrentStepHelper->DefendingOpponentSliderBB[White] =
        PrevStepHelper->DefendingOpponentSliderBB[White];
    Pos.changeSideToMove();
    HashValue.updateColor();
}

void StateImpl::undoNullMove() {
    assert(getPly(false) > 0);
    assert(getLastMove().isNull());

    Helper.goBackOneStep();
    Pos.changeSideToMove();
    HashValue.updateColor();
}

namespace {

// The order in which computeSEE() picks the attackers. It must be kept
// consistent with the order in which generateLegalSmallestMove() tries
// the piece types.
// clang-format off
constexpr int32_t SEEAttackerOrder[NumPieceType] = {
    /* PTK_Empty */     14,
    /* PTK_Pawn */       0, /* PTK_Lance */     2, /* PTK_Knight */     4, /* PTK_Silver */     6,
    /* PTK_Bishop */     9, /* PTK_Rook */     10, /* PTK_Gold */       8, /* PTK_King */      13,
    /* PTK_ProPawn */    1, /* PTK_ProLance */  3, /* PTK_ProKnight */  5, /* PTK_ProSilver */  7,
    /* PTK_ProBishop */ 11, /* PTK_ProRook */  12,
};
// clang-format on

// The largest promotion bonus a single capture in the exchange can gain.
// Never negative: the promoted piece types must not be valued less than
// their corresponding raw piece types (see computeSEE()).
int32_t maxPromotionGain(const int32_t* const PieceValues) noexcept {
    int32_t Gain = 0;

    Gain = std::max(Gain, PieceValues[PTK_ProPawn] - PieceValues[PTK_Pawn]);
    Gain = std::max(Gain, PieceValues[PTK_ProLance] - PieceValues[PTK_Lance]);
    Gain = std::max(Gain, PieceValues[PTK_ProKnight] - PieceValues[PTK_Knight]);
    Gain = std::max(Gain, PieceValues[PTK_ProSilver] - PieceValues[PTK_Silver]);
    Gain = std::max(Gain, PieceValues[PTK_ProBishop] - PieceValues[PTK_Bishop]);
    Gain = std::max(Gain, PieceValues[PTK_ProRook] - PieceValues[PTK_Rook]);

    return Gain;
}

} // namespace

// When `GEMode` is false, returns the SEE value of the move.
// When `GEMode` is true, returns 1 if the SEE value is greater than or
// equal to `Threshold` and 0 otherwise. Instead of folding the whole
// `Gains` array back at the end, `Bound` carries the threshold through
// the exchange so that the loop can stop as soon as the comparison
// outcome is decided:
//  - after an even-depth capture (by the moving side), `Bound` is the
//    largest value the next opponent capture may gain while keeping
//    the SEE value at least `Threshold`, and
//  - after an odd-depth capture (by the opponent), `Bound` is the
//    smallest value the next own capture must gain to keep the SEE
//    value at least `Threshold`.
template <bool GEMode>
int32_t StateImpl::computeSEEImpl(const Move32 Move,
                                  const int32_t* const PieceValues,
                                  const int32_t Threshold) const noexcept {
    assert(!Move.isNone() && !Move.isNull());

    const bool IsDrop = Move.drop();
    const bool IsCapture = Move.capturePieceType() != PTK_Empty;

    const StepHelper* CurrentStepHelper = &Helper.getCurrentStepHelper();

    const Square To = Move.to();
    Color C = getSideToMove();

    int32_t Gains[64];

    // The exchange starts with the given move, which is assumed to be legal.
    // A non-capture (a drop or a quiet board move) captures nothing: the
    // exchange value consists only of what the opponent wins back on To.
    Gains[0] = IsCapture ? PieceValues[Move.capturePieceType()] : 0;

    PieceTypeKind TargetType = Move.pieceType();
    if (Move.promote()) {
        // The moving side gains the promotion bonus, and in exchange the
        // opponent can now win the promoted piece back.
        TargetType = promotePieceType(Move.pieceType());
        Gains[0] += PieceValues[TargetType] - PieceValues[Move.pieceType()];
    }

    [[maybe_unused]] int32_t Bound = 0;
    [[maybe_unused]] int32_t MaxPromotionGain = 0;
    if constexpr (GEMode) {
        // The SEE value never exceeds Gains[0] because the opponent may
        // simply stop capturing.
        Bound = Gains[0] - Threshold;
        if (Bound < 0) {
            return 0;
        }

        // Conversely, a capture at depth D wins at most the piece sitting
        // on To plus the promotion bonus, and the deeper captures cannot
        // make it more profitable (each side may stop capturing instead),
        // so the SEE value is at least Gains[0] minus that maximum. Both
        // exits skip building the attacker set below.
        MaxPromotionGain = maxPromotionGain(PieceValues);
        if (Bound >= PieceValues[TargetType] + MaxPromotionGain) {
            return 1;
        }
    }

    bitboard::Bitboard BBs[2] = {getBitboard(Black), getBitboard(White)};

    // Move my piece. Only From squares get vacated during the exchange:
    // To remains occupied all the time (a non-capture occupies it with
    // this very move), so its ownership is not tracked.
    if (!IsDrop) {
        // My piece must exist.
        assert(BBs[C].isSet(Move.from()));
        BBs[C].toggleBit(Move.from());
    }
    if (IsCapture) {
        // The opponent's piece must exist.
        assert(BBs[~C].isSet(Move.to()));
        BBs[~C].toggleBit(Move.to());
    }
    BBs[C].toggleBit(Move.to());

    bitboard::Bitboard OccupiedBB = BBs[Black] | BBs[White];

    // All pieces of both colors attacking To under the current occupancy.
    // The set may keep bits of squares whose piece has already been
    // exchanged away; such stale bits are always masked by BBs[] below.
    bitboard::Bitboard AttackersBB = computeSEEAttackersBB(To, BBs, OccupiedBB);

    // When a capture delivers a discovered check, capturing on To cannot
    // resolve the check because To never lies on the discovered line
    // (for a capture, To has been occupied since the root position, so a
    // line containing To would have had two blockers and would not have
    // been recorded). Hence the only possible recapture is by the king.
    // A direct check, on the other hand, is always resolved by capturing
    // the checker on To and needs no special handling.
    // When the first move is a non-capture, To was empty at the root and a
    // recorded line may pass through To; the piece sitting on To keeps such
    // a line blocked, so the test below can produce a false positive. That
    // only suppresses a legal recapture and cuts the exchange short: the
    // SEE value can only be overestimated by it, never underestimated.
    // A drop vacates no square and cannot deliver a discovered check.
    bool OnlyKingCanCapture =
        !IsDrop &&
        seeGivesDiscoveredCheck(C, Move.from(), BBs[C], CurrentStepHelper);

    C = ~C;

    // Keep the two color bitboards in locals so that they stay in
    // registers across the exchange loop: BBs[] with a runtime color
    // index would be spilled to the stack on every update.
    bitboard::Bitboard MyBB = BBs[C];
    bitboard::Bitboard OppBB = BBs[~C];

    int32_t Depth;

    for (Depth = 1;; ++Depth) {
        assert(Depth < 64);

        const bitboard::Bitboard CandidatesBB = AttackersBB & MyBB;

        if (CandidatesBB.isZero()) {
            // No capture is possible.
            break;
        }

        // The pinners that still remain on their original squares.
        const bitboard::Bitboard AlivePinnersBB =
            OppBB & CurrentStepHelper->Pinners[~C];

        Square FromSq = SqInvalid;
        PieceTypeKind AttackerType = PTK_Empty;

        if (!OnlyKingCanCapture) {
            // Search the least valuable attacker. Scanning the candidate
            // squares directly is cheaper than testing each piece type
            // because there are only a few attackers at a time.
            int32_t BestOrder = SEEAttackerOrder[PTK_King];

            bitboard::Bitboard ScanBB = CandidatesBB;

            while (!ScanBB.isZero()) {
                const Square Sq = ScanBB.popOne();
                const PieceTypeKind Type = getPieceType(Pos.pieceOn(Sq));
                const int32_t Order = SEEAttackerOrder[Type];

                if (Order >= BestOrder) {
                    // This also skips the king: it is handled separately
                    // below because moving it needs a legality check.
                    continue;
                }

                // Is this piece pinned?
                // A pin disappears once its pinner moves or is captured
                // during the exchange, so check that the pinner of this
                // piece still remains on its original square.
                if (getDefendingOpponentSliderBB(C).isSet(Sq)) {
                    const Square MyKingSq = getKingSquare(C);

                    bool Pinned = false;
                    (AlivePinnersBB & bitboard::LineBB[Sq][MyKingSq])
                        .forEach([&](Square PinnerSq) {
                            if (bitboard::getBetweenBB(PinnerSq, MyKingSq)
                                    .isSet(Sq)) {
                                Pinned = true;
                            }
                        });

                    // If pinned, the piece can only move to a square which
                    // is aligned with the king and the piece's current
                    // square.
                    if (Pinned && !bitboard::LineBB[Sq][MyKingSq].isSet(To)) {
                        continue;
                    }
                }

                BestOrder = Order;
                FromSq = Sq;
                AttackerType = Type;
            }
        }

        if (AttackerType == PTK_Empty) {
            // No piece but possibly the king can capture.
            if ((CandidatesBB & getBitboard<PTK_King>()).isZero()) {
                break;
            }

            const Square KingSq = getKingSquare(C);
            assert(CandidatesBB.isSet(KingSq));

            // Moving the king is legal only if To is not defended.
            OccupiedBB.toggleBit(KingSq);
            AttackersBB.toggleBit(KingSq);
            updateSEEAttackersBB(&AttackersBB, To, KingSq, OccupiedBB);

            if (!(AttackersBB & OppBB).isZero()) {
                // To is defended: the king cannot capture.
                break;
            }

            assert(TargetType != PTK_King);
            Gains[Depth] = PieceValues[TargetType];

            if constexpr (GEMode) {
                Bound = Gains[Depth] - Bound;
                if ((Depth & 1) != 0) {
                    // The opponent's capture is not profitable enough:
                    // the SEE value stays at least Threshold.
                    if (Bound <= 0) {
                        return 1;
                    }
                } else {
                    // Even this capture cannot bring the SEE value back
                    // to Threshold.
                    if (Bound < 0) {
                        return 0;
                    }
                }
            }

            MyBB.toggleBit(KingSq);
            TargetType = PTK_King;

            // No opponent attacker remains (verified right above), so the
            // exchange ends at the next iteration.
            OnlyKingCanCapture = false;
        } else {
            assert(checkRange(FromSq));
            assert(TargetType != PTK_King);

            const bool Promotes =
                !isPromoted(AttackerType) && AttackerType != PTK_Gold &&
                !(bitboard::PromotableBB[C] &
                  (bitboard::SquareBB[FromSq] | bitboard::SquareBB[To]))
                     .isZero();

            Gains[Depth] = PieceValues[TargetType];
            if (Promotes) {
                TargetType = promotePieceType(AttackerType);
                Gains[Depth] +=
                    PieceValues[TargetType] - PieceValues[AttackerType];
            } else {
                TargetType = AttackerType;
            }

            if constexpr (GEMode) {
                Bound = Gains[Depth] - Bound;
                if ((Depth & 1) != 0) {
                    // The opponent's capture is not profitable enough: the
                    // SEE value stays at least Threshold.
                    if (Bound <= 0) {
                        return 1;
                    }
                    // The recapture at the next depth cannot gain enough
                    // even with the largest possible victim: the SEE value
                    // stays below Threshold whether it is played or not.
                    if (PieceValues[TargetType] + MaxPromotionGain < Bound) {
                        return 0;
                    }
                } else {
                    // Even this capture cannot bring the SEE value back to
                    // Threshold.
                    if (Bound < 0) {
                        return 0;
                    }
                    // The recapture at the next depth cannot win enough
                    // even with the largest possible victim: the SEE value
                    // stays at least Threshold whether it is played or not.
                    if (PieceValues[TargetType] + MaxPromotionGain <= Bound) {
                        return 1;
                    }
                }
            }

            MyBB.toggleBit(FromSq);
            OccupiedBB.toggleBit(FromSq);
            AttackersBB.toggleBit(FromSq);
            updateSEEAttackersBB(&AttackersBB, To, FromSq, OccupiedBB);

            OnlyKingCanCapture =
                seeGivesDiscoveredCheck(C, FromSq, MyBB, CurrentStepHelper);
        }

        C = ~C;
        std::swap(MyBB, OppBB);
    }

    if constexpr (GEMode) {
        // The side to move at Depth cannot capture anymore. If it is the
        // opponent (odd depth), no capture was profitable enough to drop
        // the SEE value below Threshold; if it is the moving side (even
        // depth), the last opponent capture dropped the SEE value below
        // Threshold and there is no way to recover.
        return Depth & 1;
    } else {
        int32_t Gain = 0;

        for (int32_t I = Depth - 1; I >= 0; --I) {
            Gain = Gains[I] - std::max(Gain, 0);
        }

        return Gain;
    }
}

int32_t StateImpl::computeSEE(const Move32 Move,
                              const int32_t* const PieceValues) const noexcept {
    return computeSEEImpl<false>(Move, PieceValues, 0);
}

bool StateImpl::computeSEEGE(const Move32 Move,
                             const int32_t* const PieceValues,
                             const int32_t Threshold) const noexcept {
    return computeSEEImpl<true>(Move, PieceValues, Threshold) != 0;
}

bitboard::Bitboard StateImpl::computeSEEAttackersBB(
    const Square To, const bitboard::Bitboard* BBs,
    const bitboard::Bitboard& OccupiedBB) const noexcept {
    const bitboard::Bitboard GoldsBB =
        getBitboard<PTK_Gold>() | getBitboard<PTK_ProPawn>() |
        getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
        getBitboard<PTK_ProSilver>();

    // The rook attacks are reused for the lances: the file component of
    // the rook attacks from To is exactly the union of the two lance rays
    // (both go up to and including the first occupied square), so masking
    // it with the forward squares of To from the lance's point of view
    // gives the same bitboard as getLanceAttackBB() computes.
    const bitboard::Bitboard RookAttackBB =
        bitboard::getRookAttackBB<PTK_Rook>(To, OccupiedBB);

    // A piece of color C on Sq attacks To if and only if Sq is attacked
    // by the same piece type of color ~C placed on To.
    const bitboard::Bitboard BlackAttackersBB =
        ((bitboard::getAttackBB<White, PTK_Pawn>(To) &
          getBitboard<PTK_Pawn>()) |
         (bitboard::getAttackBB<White, PTK_Knight>(To) &
          getBitboard<PTK_Knight>()) |
         (bitboard::getAttackBB<White, PTK_Silver>(To) &
          getBitboard<PTK_Silver>()) |
         (bitboard::getAttackBB<White, PTK_Gold>(To) & GoldsBB) |
         (RookAttackBB & bitboard::getForwardBB<White>(To) &
          getBitboard<PTK_Lance>())) &
        BBs[Black];

    const bitboard::Bitboard WhiteAttackersBB =
        ((bitboard::getAttackBB<Black, PTK_Pawn>(To) &
          getBitboard<PTK_Pawn>()) |
         (bitboard::getAttackBB<Black, PTK_Knight>(To) &
          getBitboard<PTK_Knight>()) |
         (bitboard::getAttackBB<Black, PTK_Silver>(To) &
          getBitboard<PTK_Silver>()) |
         (bitboard::getAttackBB<Black, PTK_Gold>(To) & GoldsBB) |
         (RookAttackBB & bitboard::getForwardBB<Black>(To) &
          getBitboard<PTK_Lance>())) &
        BBs[White];

    // The attacks of the king, the sliding parts of the bishop and the rook,
    // and the step parts of the promoted bishop and the promoted rook are
    // symmetric in color, and pieces on the vacated squares have already
    // been filtered out through OccupiedBB.
    const bitboard::Bitboard SymmetricAttackersBB =
        ((bitboard::KingAttackBB[To] &
          (getBitboard<PTK_King>() | getBitboard<PTK_ProBishop>() |
           getBitboard<PTK_ProRook>())) |
         (bitboard::getBishopAttackBB<PTK_Bishop>(To, OccupiedBB) &
          (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>())) |
         (RookAttackBB &
          (getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()))) &
        OccupiedBB;

    return BlackAttackersBB | WhiteAttackersBB | SymmetricAttackersBB;
}

void StateImpl::updateSEEAttackersBB(
    bitboard::Bitboard* AttackersBB, const Square To, const Square FromSq,
    const bitboard::Bitboard& OccupiedBB) const noexcept {
    const bitboard::Bitboard& XRayLineBB = bitboard::LineBB[To][FromSq];

    if (XRayLineBB.isZero()) {
        // Not aligned with To (a knight): no slider can be revealed.
        return;
    }

    if (squareToFile(To) == squareToFile(FromSq)) {
        // Vertical: lances, rooks and dragons can be revealed.
        const bitboard::Bitboard RookDragonBB =
            getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>();

        *AttackersBB |= (bitboard::getLanceAttackBB<Black>(To, OccupiedBB) &
                         (RookDragonBB |
                          (getBitboard<PTK_Lance>() & getBitboard<White>()))) |
                        (bitboard::getLanceAttackBB<White>(To, OccupiedBB) &
                         (RookDragonBB |
                          (getBitboard<PTK_Lance>() & getBitboard<Black>())));
    } else if (squareToRank(To) == squareToRank(FromSq)) {
        // Horizontal: rooks and dragons can be revealed.
        *AttackersBB |= bitboard::getRookAttackBB<PTK_Rook>(To, OccupiedBB) &
                        XRayLineBB &
                        (getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>());
    } else {
        // Diagonal: bishops and horses can be revealed.
        *AttackersBB |=
            bitboard::getBishopAttackBB<PTK_Bishop>(To, OccupiedBB) &
            XRayLineBB &
            (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>());
    }
}

bool StateImpl::seeGivesDiscoveredCheck(
    const Color C, const Square FromSq, const bitboard::Bitboard& MyBB,
    const StepHelper* SHelper) const noexcept {
    // Was the piece on FromSq shielding the opponent's king from one of
    // my sliders at the root position?
    if (!getDefendingOpponentSliderBB(~C).isSet(FromSq)) {
        return false;
    }

    // A discovered line becomes a check once FromSq gets vacated: pieces
    // can enter only To during the exchange and To is never on the line.
    // Verify that my slider still remains on its original square.
    const Square OpKingSq = getKingSquare(~C);

    bool Discovered = false;
    (MyBB & SHelper->Pinners[C] & bitboard::LineBB[FromSq][OpKingSq])
        .forEach([&](Square SliderSq) {
            if (bitboard::getBetweenBB(SliderSq, OpKingSq).isSet(FromSq)) {
                Discovered = true;
            }
        });

    return Discovered;
}

template <Color C, bool UpdateCheckerBySliders>
inline void StateImpl::setDefendingOpponentSliderBBAndSliderCheckerBB(
    StepHelper* SHelper, const bitboard::Bitboard& OccupiedBB) noexcept {
    SHelper->DefendingOpponentSliderBB[C].clear();
    SHelper->Pinners[~C].clear();

    const bitboard::Bitboard Candidates =
        ((getBitboard<PTK_Lance>() &
          bitboard::getForwardBB<C>(getKingSquare<C>())) |
         ((getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>()) &
          bitboard::getDiagBB(getKingSquare<C>())) |
         ((getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()) &
          bitboard::getCrossBB(getKingSquare<C>()))) &
        getBitboard<~C>();

    Candidates.forEach([&](Square Sq) {
        const bitboard::Bitboard BetweenOccupiedBB =
            bitboard::getBetweenBB(Sq, getKingSquare<C>()) & OccupiedBB;

        const uint8_t PopCount = BetweenOccupiedBB.popCount();
        if (PopCount == 0) {
            if constexpr (UpdateCheckerBySliders) {
                SHelper->CheckerBB |= bitboard::SquareBB[Sq];
            }
        } else if (PopCount == 1) {
            SHelper->DefendingOpponentSliderBB[C] |= BetweenOccupiedBB;
            SHelper->Pinners[~C].toggleBit(Sq);
        }
    });
}

template <Color C>
inline void StateImpl::setStepCheckerBB(StepHelper* SHelper) noexcept {
    const Square KingSq = getKingSquare<C>();
    assert(checkRange(KingSq));

    SHelper->CheckerBB.clear();

    SHelper->CheckerBB |=
        bitboard::getAttackBB<C, PTK_Pawn>(KingSq) & getBitboard<PTK_Pawn>();
    SHelper->CheckerBB |= bitboard::getAttackBB<C, PTK_Knight>(KingSq) &
                          getBitboard<PTK_Knight>();
    SHelper->CheckerBB |= bitboard::getAttackBB<C, PTK_Silver>(KingSq) &
                          getBitboard<PTK_Silver>();
    SHelper->CheckerBB |=
        bitboard::getAttackBB<C, PTK_Gold>(KingSq) &
        (getBitboard<PTK_Gold>() | getBitboard<PTK_ProPawn>() |
         getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
         getBitboard<PTK_ProSilver>());
    SHelper->CheckerBB |=
        bitboard::getAttackBB<C, PTK_King>(KingSq) &
        (getBitboard<PTK_ProBishop>() | getBitboard<PTK_ProRook>());

    SHelper->CheckerBB &= getBitboard<~C>();
}

bool StateImpl::canDeclare() const noexcept {
    if (getSideToMove() == Black) {
        return canDeclareImpl<Black>(*this);
    } else {
        return canDeclareImpl<White>(*this);
    }
}

template void StateImpl::doMove<Black>(Move32 Move) noexcept;
template void StateImpl::doMove<White>(Move32 Move) noexcept;
template void StateImpl::undoMove<Black>();
template void StateImpl::undoMove<White>();
template bool StateImpl::isLegal<Black>(Move16 Move) const noexcept;
template bool StateImpl::isLegal<White>(Move16 Move) const noexcept;
template bool StateImpl::isLegal<Black>(Move32 Move) const noexcept;
template bool StateImpl::isLegal<White>(Move32 Move) const noexcept;

} // namespace internal
} // namespace core
} // namespace nshogi
