//
// Copyright (c) 2025 @nyashiki
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

#include <cassert>
#include <cstring>

namespace nshogi {
namespace core {
namespace internal {

namespace {

template <Color C>
bool canDeclareImpl(const StateImpl& S) {
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
    , HashValue(S.HashValue)
    , Helper(std::move(S.Helper)) {
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
inline void StateImpl::doMove(Move32 Move) {
    Helper.proceedOneStep(Move, HashValue.getValue(),
                          getPosition().getStand<Black>(),
                          getPosition().getStand<White>());
    StepHelper* CurrentStepHelper = &Helper.SHelper[Helper.Ply];

    const Square To = Move.to();
    const Square From = Move.from();

    if (Move.drop()) { // When the move is dropping.
        PieceTypeKind Type = Move.pieceType();

        // Decrement the number of the standing piece one has.
        Pos.decrementStand(C, Type);

        // And put the piece on the board.
        Pos.putPiece(To, makePiece(C, Type));

        Helper.ColorBB[C].toggleBit(To);
        Helper.TypeBB[Type].toggleBit(To);

        HashValue.update<C>(Type, To);
    } else { // Move a piece on the board, which means not a dropping move.
        const PieceTypeKind Type = Move.pieceType();
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
            const PieceTypeKind PromotedType = getPieceType(PromotedPiece);

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
    setCheckerBB<~C>(CurrentStepHelper);
    setDefendingOpponentSliderBB<C, false>(CurrentStepHelper, OccupiedBB);
    setDefendingOpponentSliderBB<~C, true>(CurrentStepHelper, OccupiedBB);

    const StepHelper& PrevStepHelper = Helper.getStepHelper(Helper.Ply - 1);
    if (!CurrentStepHelper->CheckerBB.isZero()) {
        CurrentStepHelper->ContinuousCheckCounts[C] =
            PrevStepHelper.ContinuousCheckCounts[C] + 1;
    } else {
        CurrentStepHelper->ContinuousCheckCounts[C] = 0;
    }
    CurrentStepHelper->ContinuousCheckCounts[~C] =
        PrevStepHelper.ContinuousCheckCounts[~C];

    // And now it is opponent's turn.
    Pos.changeSideToMove();
}

void StateImpl::doMove(Move32 Move) {
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
            Pos.putPiece(To, makePiece(C, CaptureType));

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

        Pos.putPiece(From, makePiece(~C, Type));
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

void StateImpl::refresh() {
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
        setCheckerBB<Black>(CurrentStepHelper);
        setDefendingOpponentSliderBB<Black, true>(CurrentStepHelper,
                                                  OccupiedBB);
        setDefendingOpponentSliderBB<White, false>(CurrentStepHelper,
                                                   OccupiedBB);
    } else {
        setCheckerBB<White>(CurrentStepHelper);
        setDefendingOpponentSliderBB<Black, false>(CurrentStepHelper,
                                                   OccupiedBB);
        setDefendingOpponentSliderBB<White, true>(CurrentStepHelper,
                                                  OccupiedBB);
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

template <Color C, bool UpdateCheckerBySliders>
inline void
StateImpl::setDefendingOpponentSliderBB(StepHelper* SHelper,
                                        const bitboard::Bitboard& OccupiedBB) {
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

        if (BetweenOccupiedBB.isZero()) {
            if constexpr (UpdateCheckerBySliders) {
                SHelper->CheckerBB |= bitboard::SquareBB[Sq];
            }
        } else if (BetweenOccupiedBB.popCount() == 1) {
            SHelper->DefendingOpponentSliderBB[C] |= BetweenOccupiedBB;
        }
    });
}

template <Color C>
inline void StateImpl::setCheckerBB(StepHelper* SHelper) {
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

bool StateImpl::canDeclare() const {
    if (getSideToMove() == Black) {
        return canDeclareImpl<Black>(*this);
    } else {
        return canDeclareImpl<White>(*this);
    }
}

template void StateImpl::doMove<Black>(Move32 Move);
template void StateImpl::doMove<White>(Move32 Move);
template void StateImpl::undoMove<Black>();
template void StateImpl::undoMove<White>();

} // namespace internal
} // namespace core
} // namespace nshogi
