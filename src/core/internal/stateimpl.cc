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
        const core::PieceKind Piece = P.pieceOn(Move.from());
        const core::PieceTypeKind Type = getPieceType(Piece);

        if (Type >= NumPieceType) {
            return false;
        }

        if (Move.from() >= NumSquares || Move.to() >= NumSquares) {
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

int32_t StateImpl::computeSEE(Square To, const int32_t* const PieceValues) const noexcept {
    Color C = getSideToMove();

    bitboard::Bitboard BBs[2] = { getBitboard(Black), getBitboard(White) };
    int32_t Gains[64];

    const PieceKind TargetPiece = getPosition().pieceOn(To);
    assert(TargetPiece == PK_Empty || getColor(TargetPiece) != C);

    PieceTypeKind TargetType = getPieceType(TargetPiece);

    int32_t Depth;

    for (Depth = 0; ; ++Depth, C = ~C) {
        assert(Depth < 64);

        bitboard::Bitboard& MyBB = BBs[C];
        bitboard::Bitboard& OpBB = BBs[~C];

        if (Depth <= 1) {
            // A pawn attacker to To, if any, is fixed by color and square.
            // Since pieces only leave their source squares during SEE, no new pawn attacker
            // can be revealed after depth >= 2.
            if (processSEEPiece<PTK_Pawn>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
                assert(TargetType != PTK_King);
                continue;
            }
        }

        if (processSEEPiece<PTK_ProPawn>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Lance>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_ProLance>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Knight>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_ProKnight>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Silver>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_ProSilver>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Gold>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Bishop>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_Rook>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_ProBishop>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_ProRook>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }
        if (processSEEPiece<PTK_King>(C, To, &MyBB, &OpBB, &TargetType, &Gains[Depth], PieceValues)) {
            assert(TargetType != PTK_King);
            continue;
        }

        // No capture is possible.
        break;
    }

    int32_t Gain = 0;

    for (int32_t I = Depth - 1; I >= 0; --I) {
        Gain = Gains[I] - std::max(Gain, 0);
    }

    return Gain;
}

template <PieceTypeKind Type>
bool StateImpl::processSEEPiece(
    Color C,
    Square To,
    bitboard::Bitboard* MyBB,
    bitboard::Bitboard* OpBB,
    PieceTypeKind* TargetType,
    int32_t* Gain,
    const int32_t* const PieceValues
) const noexcept {
    bitboard::Bitboard FromBB =
        getBitboard<Type>()
        & *MyBB;

    if (FromBB.isZero()) {
        return false;
    }

    const bitboard::Bitboard OccupiedBB = *MyBB | *OpBB;
    bitboard::Bitboard MovableFromBB = bitboard::Bitboard::ZeroBB();

    if constexpr (Type != PTK_Lance && Type != PTK_Bishop && Type != PTK_Rook) {
        MovableFromBB |= bitboard::getAttackBB<Type>(~C, To);
    }
    if constexpr (Type == PTK_Lance) {
        MovableFromBB |= bitboard::getLanceAttackBB(~C, To, OccupiedBB);
    } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
        MovableFromBB |= bitboard::getBishopAttackBB<PTK_Bishop>(To, OccupiedBB);
    } else if constexpr (Type == PTK_Rook || Type == PTK_ProRook) {
        MovableFromBB |= bitboard::getRookAttackBB<PTK_Rook>(To, OccupiedBB);
    }

    FromBB &= MovableFromBB;

    while (!FromBB.isZero()) {
        const Square FromSq = FromBB.popOne();

        // Is this piece pinned?
        if (getDefendingOpponentSliderBB(C).isSet(FromSq)) {
            // If pinned, the piece must move to a square which
            // is aligned with the king and the piece's current square.
            if (bitboard::LineBB[FromSq][getKingSquare(C)].isSet(To)) {
                // Promotion.
                if (!isPromoted(Type) && Type != PTK_Gold && Type != PTK_King &&
                        !(bitboard::PromotableBB[C]
                            & (bitboard::SquareBB[FromSq] | bitboard::SquareBB[To])
                        ).isZero()) {
                    *Gain = PieceValues[*TargetType];
                    MyBB->toggleBit(FromSq);
                    *TargetType = promotePieceType(Type);
                    return true;
                } else {
                    *Gain = PieceValues[*TargetType];
                    MyBB->toggleBit(FromSq);
                    *TargetType = Type;
                    return true;
                }
            }
        } else { // Not pinned.
            // Promotion.
            if (!isPromoted(Type) && Type != PTK_Gold && Type != PTK_King &&
                    !(bitboard::PromotableBB[C]
                        & (bitboard::SquareBB[FromSq] | bitboard::SquareBB[To])
                    ).isZero()) {
                *Gain = PieceValues[*TargetType];
                MyBB->toggleBit(FromSq);
                *TargetType = promotePieceType(Type);
                return true;
            } else {
                if constexpr (Type == PTK_King) {
                    // When moving the king, we need to check if the move is legal.

                    // Is attacked by opponent's step piece?
                    const bitboard::Bitboard StepAttackersBB =
                        ((bitboard::getAttackBB<PTK_Pawn>(C, To) & getBitboard<PTK_Pawn>()) |
                         (bitboard::getAttackBB<PTK_Knight>(C, To) & getBitboard<PTK_Knight>()) |
                         (bitboard::getAttackBB<PTK_Silver>(C, To) & getBitboard<PTK_Silver>()) |
                         (bitboard::getAttackBB<PTK_Gold>(C, To) &
                          (getBitboard<PTK_Gold>() | getBitboard<PTK_ProPawn>() |
                           getBitboard<PTK_ProLance>() | getBitboard<PTK_ProKnight>() |
                           getBitboard<PTK_ProSilver>())) |
                         (bitboard::getAttackBB<PTK_King>(C, To) &
                          (getBitboard<PTK_King>() | getBitboard<PTK_ProBishop>() | getBitboard<PTK_ProRook>())))
                        & *OpBB;

                    if (!StepAttackersBB.isZero()) {
                        return false;
                    }

                    // Is attacked by opponent's slider piece?
                    const bitboard::Bitboard AfterMoveOccupiedBB = OccupiedBB ^ bitboard::SquareBB[FromSq];
                    const bitboard::Bitboard SliderAttackersBB =
                        ((bitboard::getLanceAttackBB(C, To, AfterMoveOccupiedBB) & getBitboard<PTK_Lance>()) |
                         (bitboard::getBishopAttackBB<PTK_Bishop>(To, AfterMoveOccupiedBB) & (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>())) |
                         (bitboard::getRookAttackBB<PTK_Rook>(To, AfterMoveOccupiedBB) & (getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())))
                        & *OpBB;

                    if (!SliderAttackersBB.isZero()) {
                        return false;
                    }
                }

                *Gain = PieceValues[*TargetType];
                MyBB->toggleBit(FromSq);
                *TargetType = Type;
                return true;
            }
        }
    }

    return false;
}

template <Color C, bool UpdateCheckerBySliders>
inline void StateImpl::setDefendingOpponentSliderBBAndSliderCheckerBB(
    StepHelper* SHelper, const bitboard::Bitboard& OccupiedBB) noexcept {
    SHelper->DefendingOpponentSliderBB[C].clear();

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
