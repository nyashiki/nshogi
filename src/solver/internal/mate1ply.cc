//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "mate1ply.h"

namespace nshogi {
namespace solver {
namespace internal {
namespace mate1ply {

namespace {

// Only adjacent checks and knight checks are handled by this solver. The
// masks ignore blockers but include promotion restrictions, so they can
// reject impossible origins before generating any candidate moves.
struct CheckOrigins {
    core::internal::bitboard::Bitboard BB[core::NumSquares][core::NumPieceType];
    core::internal::bitboard::Bitboard Bishops[core::NumSquares];
    core::internal::bitboard::Bitboard Rooks[core::NumSquares];
};

template <core::Color C, core::PieceTypeKind Type, bool Promote>
void addOrigins(CheckOrigins& Origins) {
    using namespace core;
    using namespace core::internal::bitboard;
    constexpr PieceTypeKind AfterType = Promote ? promotePieceType(Type) : Type;
    for (Square KingSq : Squares) {
        Bitboard ToBB;
        if constexpr (AfterType == PTK_Bishop) {
            ToBB = PromotableBB[C].andNot(DiagStepAttackBB[KingSq]);
        } else if constexpr (AfterType == PTK_Rook) {
            ToBB = PromotableBB[C].andNot(CrossStepAttackBB[KingSq]);
        } else if constexpr (AfterType == PTK_Lance) {
            ToBB = Bitboard::SecondFurthestBB<C>().andNot(
                getAttackBB<~C, PTK_Pawn>(KingSq));
        } else {
            ToBB = getAttackBB<~C, AfterType>(KingSq);
            if constexpr (AfterType == PTK_Pawn) {
                ToBB = PromotableBB[C].andNot(ToBB);
            }
        }
        // Pawn/lance shifts are lane-local and can wrap at a file edge or
        // set a padding bit. Never use those bits as attack-table indices.
        if constexpr (AfterType != PTK_Knight) {
            ToBB &= KingAttackBB[KingSq];
        }
        while (!ToBB.isZero()) {
            const Square ToSq = ToBB.popOne();
            Bitboard FromBB = getStepAttackBB<~C>(Type, ToSq) |
                             getSliderAttackBB<~C>(Type, ToSq,
                                                   Bitboard::ZeroBB());
            if constexpr (Promote) {
                if (!PromotableBB[C].isSet(ToSq)) {
                    FromBB &= PromotableBB[C];
                }
            }
            Origins.BB[KingSq][Type] |= FromBB;
        }
    }
}

template <core::Color C, core::PieceTypeKind Type>
void initializeOrigins(CheckOrigins& Origins) {
    addOrigins<C, Type, false>(Origins);
    if constexpr (Type == core::PTK_Pawn || Type == core::PTK_Lance ||
                  Type == core::PTK_Knight || Type == core::PTK_Silver ||
                  Type == core::PTK_Bishop || Type == core::PTK_Rook) {
        addOrigins<C, Type, true>(Origins);
    }
}

template <core::Color C>
const CheckOrigins& checkOrigins() {
    // initializeAll() has already populated the shared attack tables when
    // solve() is called. Function-local initialization is thread-safe.
    static const CheckOrigins Origins = [] {
        CheckOrigins Result{};
        using namespace core;
        using namespace core::internal::bitboard;
        for (Square KingSq : Squares) {
            Bitboard TargetsBB = KingAttackBB[KingSq] |
                                 getAttackBB<~C, PTK_Knight>(KingSq);
            while (!TargetsBB.isZero()) {
                const Square Sq = TargetsBB.popOne();
                Result.Bishops[KingSq] |= DiagBB[Sq];
                Result.Rooks[KingSq] |= CrossBB[Sq];
            }
        }
        initializeOrigins<C, PTK_Pawn>(Result);
        initializeOrigins<C, PTK_Lance>(Result);
        initializeOrigins<C, PTK_Knight>(Result);
        initializeOrigins<C, PTK_Silver>(Result);
        initializeOrigins<C, PTK_Gold>(Result);
        initializeOrigins<C, PTK_Bishop>(Result);
        initializeOrigins<C, PTK_Rook>(Result);
        initializeOrigins<C, PTK_ProPawn>(Result);
        initializeOrigins<C, PTK_ProLance>(Result);
        initializeOrigins<C, PTK_ProKnight>(Result);
        initializeOrigins<C, PTK_ProSilver>(Result);
        initializeOrigins<C, PTK_ProBishop>(Result);
        initializeOrigins<C, PTK_ProRook>(Result);
        return Result;
    }();
    return Origins;
}

// Attacks outside the checking and escape squares are irrelevant. Skip
// sliders whose rays cannot reach any of those squares.
template <core::Color C, core::Color MateC>
core::internal::bitboard::Bitboard sliderAttacks(
    const core::internal::StateImpl& S, core::Square KingSq,
    core::Square VirtualSq = core::SqInvalid) {
    using namespace core;
    using namespace core::internal::bitboard;
    const auto& Origins = checkOrigins<MateC>();
    Bitboard OccupiedBB = S.getBitboard<Black>() | S.getBitboard<White>();
    if (VirtualSq != SqInvalid) {
        OccupiedBB |= SquareBB[VirtualSq];
    }
    Bitboard Result = Bitboard::ZeroBB();
    Bitboard BishopsBB = Origins.Bishops[KingSq] & S.getBitboard<C>() &
        (S.getBitboard<PTK_Bishop>() | S.getBitboard<PTK_ProBishop>());
    while (!BishopsBB.isZero()) {
        Result |= getBishopAttackBB<PTK_Bishop>(BishopsBB.popOne(), OccupiedBB);
    }
    Bitboard RooksBB = Origins.Rooks[KingSq] & S.getBitboard<C>() &
        (S.getBitboard<PTK_Rook>() | S.getBitboard<PTK_ProRook>());
    while (!RooksBB.isZero()) {
        Result |= getRookAttackBB<PTK_Rook>(RooksBB.popOne(), OccupiedBB);
    }
    Bitboard LancesBB = Origins.Rooks[KingSq] & S.getBitboard<C, PTK_Lance>();
    while (!LancesBB.isZero()) {
        Result |= getLanceAttackBB<C>(LancesBB.popOne(), OccupiedBB);
    }
    return Result;
}

// Only the checking square and the king's escape squares need to be covered.
// Query attacks backwards from those squares instead of rebuilding attacks
// over the entire board for every candidate move.
template <core::Color C>
bool covers(const core::internal::StateImpl& S,
            core::internal::bitboard::Bitboard RequiredBB,
            const core::internal::bitboard::Bitboard& OccupiedBB,
            core::Square FromSq) {
    using namespace core;
    using namespace core::internal::bitboard;

    const Bitboard PiecesBB = SquareBB[FromSq].andNot(S.getBitboard<C>());
    const Bitboard GoldsBB =
        S.getBitboard<PTK_Gold>() | S.getBitboard<PTK_ProPawn>() |
        S.getBitboard<PTK_ProLance>() | S.getBitboard<PTK_ProKnight>() |
        S.getBitboard<PTK_ProSilver>();
    const Bitboard BishopsBB =
        PiecesBB & (S.getBitboard<PTK_Bishop>() | S.getBitboard<PTK_ProBishop>());
    const Bitboard RooksBB =
        PiecesBB & (S.getBitboard<PTK_Rook>() | S.getBitboard<PTK_ProRook>());
    const Bitboard LancesBB = PiecesBB & S.getBitboard<PTK_Lance>();

    while (!RequiredBB.isZero()) {
        const Square Sq = RequiredBB.popOne();
        const Bitboard StepBB =
            (getAttackBB<~C, PTK_Pawn>(Sq) & S.getBitboard<PTK_Pawn>()) |
            (getAttackBB<~C, PTK_Knight>(Sq) & S.getBitboard<PTK_Knight>()) |
            (getAttackBB<~C, PTK_Silver>(Sq) & S.getBitboard<PTK_Silver>()) |
            (getAttackBB<~C, PTK_Gold>(Sq) & GoldsBB) |
            (KingAttackBB[Sq] & S.getBitboard<PTK_King>()) |
            (CrossStepAttackBB[Sq] & S.getBitboard<PTK_ProBishop>()) |
            (DiagStepAttackBB[Sq] & S.getBitboard<PTK_ProRook>());
        if (!(StepBB & PiecesBB).isZero()) {
            continue;
        }
        Bitboard SlidersBB = (DiagBB[Sq] & BishopsBB) |
                            (CrossBB[Sq] & RooksBB) |
                            (getForwardBB<~C>(Sq) & LancesBB);
        bool Attacked = false;
        while (!SlidersBB.isZero()) {
            const Square SliderSq = SlidersBB.popOne();
            if ((getBetweenBB(Sq, SliderSq) & OccupiedBB).isZero()) {
                Attacked = true;
                break;
            }
        }
        if (!Attacked) {
            return false;
        }
    }
    return true;
}

template <core::Color C>
bool isEvadable(core::Square KingSq,
                const core::internal::bitboard::Bitboard& AttackedBB,
                const core::internal::bitboard::Bitboard& MyOccupiedBB) {
    const core::internal::bitboard::Bitboard& KingAttackBB =
        core::internal::bitboard::KingAttackBB[KingSq];
    const core::internal::bitboard::Bitboard PossibleEvadableBB =
        MyOccupiedBB.andNot(KingAttackBB);

    // If all possibly-evadable squares are attacked, the king is captured.
    return (PossibleEvadableBB & AttackedBB) != PossibleEvadableBB;
}

template <core::Color C, core::PieceTypeKind Type>
core::Move32 checkmateByDrop(
    const core::internal::StateImpl& S, core::Stands St,
    core::internal::bitboard::Bitboard ToBB, core::Square OpKingSq,
    const core::internal::bitboard::Bitboard& StepAttackBB,
    const core::internal::bitboard::Bitboard& SliderAttackBB,
    const core::internal::bitboard::Bitboard& StepOrSliderAttackBB) {
    static_assert(Type != core::PTK_Pawn,
                  "PTK_Pawn must not be passed as `Type` because"
                  "one-ply checkmate is prohibitted by the rule.");

    static_assert(Type == core::PTK_Lance || Type == core::PTK_Knight ||
                      Type == core::PTK_Silver || Type == core::PTK_Gold ||
                      Type == core::PTK_Bishop || Type == core::PTK_Rook,
                  "Passed invalid `Type`.");

    if (core::getStandCount<Type>(St) == 0) {
        return core::Move32::MoveNone();
    }

    if constexpr (Type == core::PTK_Gold) {
        ToBB &=
            core::internal::bitboard::getAttackBB<~C, core::PTK_Gold>(OpKingSq);
    } else if constexpr (Type == core::PTK_Silver) {
        ToBB &= core::internal::bitboard::getAttackBB<~C, core::PTK_Silver>(
            OpKingSq);
    } else if constexpr (Type == core::PTK_Knight) {
        ToBB &= core::internal::bitboard::getAttackBB<~C, core::PTK_Knight>(
            OpKingSq);
    } else if constexpr (Type == core::PTK_Bishop) {
        ToBB &= core::internal::bitboard::DiagStepAttackBB[OpKingSq];
    } else if constexpr (Type == core::PTK_Rook) {
        ToBB &= core::internal::bitboard::CrossStepAttackBB[OpKingSq];
    } else if constexpr (Type == core::PTK_Lance) {
        ToBB =
            core::internal::bitboard::Bitboard::FurthermostBB<C>().andNot(ToBB);
        ToBB &= (C == core::Black) ? (S.getBitboard<~C, core::PTK_King>()
                                          .template getRightShiftEpi64<1>())
                                   : (S.getBitboard<~C, core::PTK_King>()
                                          .template getLeftShiftEpi64<1>());
    }

    while (!ToBB.isZero()) {
        const core::Square ToSq = ToBB.popOne();
        core::internal::bitboard::Bitboard NewAttackBB =
            core::internal::bitboard::Bitboard::ZeroBB();

        if constexpr (Type == core::PTK_Gold || Type == core::PTK_Silver ||
                      Type == core::PTK_Knight) {
            NewAttackBB |= core::internal::bitboard::getAttackBB<C, Type>(ToSq);
        } else if constexpr (Type == core::PTK_Bishop) {
            const core::internal::bitboard::Bitboard NewOccupiedBB =
                (S.getBitboard<core::Black>() | S.getBitboard<core::White>() |
                 core::internal::bitboard::SquareBB[ToSq]) ^
                S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |=
                core::internal::bitboard::getBishopAttackBB<core::PTK_Bishop>(
                    ToSq, NewOccupiedBB);
        } else if constexpr (Type == core::PTK_Rook) {
            const core::internal::bitboard::Bitboard NewOccupiedBB =
                (S.getBitboard<core::Black>() | S.getBitboard<core::White>() |
                 core::internal::bitboard::SquareBB[ToSq]) ^
                S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |=
                core::internal::bitboard::getRookAttackBB<core::PTK_Rook>(
                    ToSq, NewOccupiedBB);
        } else if constexpr (Type == core::PTK_Lance) {
            const core::internal::bitboard::Bitboard NewOccupiedBB =
                (S.getBitboard<core::Black>() | S.getBitboard<core::White>() |
                 core::internal::bitboard::SquareBB[ToSq]) ^
                S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |= core::internal::bitboard::getLanceAttackBB<C>(
                ToSq, NewOccupiedBB);
        }

        // A drop can only block existing attacks, never uncover new ones.
        if (isEvadable<~C>(OpKingSq, NewAttackBB | StepOrSliderAttackBB,
                          S.getBitboard<~C>())) {
            continue;
        }
        NewAttackBB |= (SliderAttackBB.isSet(ToSq))
            ? (StepAttackBB | sliderAttacks<C, C>(S, OpKingSq, ToSq))
            : StepOrSliderAttackBB;

        if (!isEvadable<~C>(OpKingSq, NewAttackBB, S.getBitboard<~C>())) {
            return core::Move32::droppingMove(ToSq, Type);
        }
    }

    return core::Move32::MoveNone();
}

template <core::Color C>
core::Move32 checkmateByDrops(const core::internal::StateImpl&, core::Stands,
                              const core::internal::bitboard::Bitboard&,
                              core::Square,
                              const core::internal::bitboard::Bitboard&,
                              const core::internal::bitboard::Bitboard&,
                              const core::internal::bitboard::Bitboard&) {
    return core::Move32::MoveNone();
}

template <core::Color C, core::PieceTypeKind Type, core::PieceTypeKind... Types>
core::Move32 checkmateByDrops(
    const core::internal::StateImpl& S, core::Stands St,
    const core::internal::bitboard::Bitboard& ToBB, core::Square OpKingSq,
    const core::internal::bitboard::Bitboard& StepAttackBB,
    const core::internal::bitboard::Bitboard& SliderAttackBB,
    const core::internal::bitboard::Bitboard& StepOrSliderAttackBB) {
    const core::Move32 PossiblyCheckmateMove =
        checkmateByDrop<C, Type>(S, St, ToBB, OpKingSq, StepAttackBB,
                                 SliderAttackBB, StepOrSliderAttackBB);

    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    return checkmateByDrops<C, Types...>(S, St, ToBB, OpKingSq, StepAttackBB,
                                         SliderAttackBB, StepOrSliderAttackBB);
}

template <core::Color C, bool Capture, bool Promote, core::PieceTypeKind Type>
core::Move32
checkmateByOneStepMove(const core::internal::StateImpl& S,
                       const core::internal::bitboard::Bitboard& ToBB,
                       core::Square OpKingSq,
                       const core::internal::bitboard::Bitboard& OccupiedBB,
                       const core::internal::bitboard::Bitboard& FromBB) {
    core::internal::bitboard::Bitboard PossiblyCheckmateToBB = ToBB;

    const core::internal::bitboard::Bitboard BishopsBB =
        S.getBitboard<~C, core::PTK_Bishop>() |
        S.getBitboard<~C, core::PTK_ProBishop>();
    const core::internal::bitboard::Bitboard RooksBB =
        S.getBitboard<~C, core::PTK_Rook>() |
        S.getBitboard<~C, core::PTK_ProRook>();
    const core::internal::bitboard::Bitboard LancesBB =
        S.getBitboard<~C, core::PTK_Lance>();

    if constexpr (Promote) {
        PossiblyCheckmateToBB &=
            core::internal::bitboard::getAttackBB<~C, core::promotePieceType(
                                                          Type)>(OpKingSq);
    } else {
        PossiblyCheckmateToBB &=
            core::internal::bitboard::getAttackBB<~C, Type>(OpKingSq);
        if constexpr (Type == core::PTK_Pawn) {
            PossiblyCheckmateToBB =
                core::internal::bitboard::PromotableBB[C].andNot(
                    PossiblyCheckmateToBB);
        }
    }

    while (!PossiblyCheckmateToBB.isZero()) {
        const core::Square PossiblyCheckmateToSq =
            PossiblyCheckmateToBB.popOne();

        core::internal::bitboard::Bitboard PossiblyCheckmateFromBB =
            FromBB & core::internal::bitboard::getAttackBB<~C, Type>(
                         PossiblyCheckmateToSq);

        if constexpr (Promote) {
            // Given Promote == true, all generated moves must be promotion
            // moves, so by the rule, ToSq or FromSq must be in promotable
            // squares.
            if (!core::internal::bitboard::PromotableBB[C].isSet(
                    PossiblyCheckmateToSq)) {
                PossiblyCheckmateFromBB &=
                    core::internal::bitboard::PromotableBB[C];
            }
        }

        while (!PossiblyCheckmateFromBB.isZero()) {
            const core::Square PossiblyCheckmateFromSq =
                PossiblyCheckmateFromBB.popOne();

            // Check if it is defenced by opponent's sliders.
            {
                const core::internal::bitboard::Bitboard TempOccupiedBB =
                    OccupiedBB ^ core::internal::bitboard::SquareBB
                                     [PossiblyCheckmateFromSq] |
                    core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

                if (!(core::internal::bitboard::LineBB
                          [PossiblyCheckmateToSq][PossiblyCheckmateFromSq] &
                      BishopsBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getBishopAttackBB<
                              core::PTK_Bishop>(PossiblyCheckmateToSq,
                                                TempOccupiedBB) &
                          BishopsBB)
                             .isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::LineBB
                          [PossiblyCheckmateToSq][PossiblyCheckmateFromSq] &
                      RooksBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getRookAttackBB<
                              core::PTK_Rook>(PossiblyCheckmateToSq,
                                              TempOccupiedBB) &
                          RooksBB)
                             .isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::FileBB[core::squareToFile(
                          PossiblyCheckmateToSq)] &
                      LancesBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getLanceAttackBB<C>(
                              PossiblyCheckmateToSq, TempOccupiedBB) &
                          LancesBB)
                             .isZero()) {
                        continue;
                    }
                }
            }

            core::internal::bitboard::Bitboard NewAttackedBB =
                core::internal::bitboard::Bitboard::ZeroBB();

            // Add post-move attacks.
            if constexpr (Promote) {
                NewAttackedBB |= core::internal::bitboard::getAttackBB<
                    C, core::promotePieceType(Type)>(PossiblyCheckmateToSq);
            } else {
                NewAttackedBB |= core::internal::bitboard::getAttackBB<C, Type>(
                    PossiblyCheckmateToSq);
            }

            core::internal::bitboard::Bitboard RequiredBB =
                (S.getBitboard<~C>() | NewAttackedBB).andNot(
                    core::internal::bitboard::KingAttackBB[OpKingSq]);
            if constexpr (Type != core::PTK_Knight || Promote) {
                RequiredBB |=
                    core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];
            }
            const core::internal::bitboard::Bitboard PostOccupiedBB =
                (OccupiedBB ^ core::internal::bitboard::SquareBB
                                  [PossiblyCheckmateFromSq]) |
                core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];
            if (covers<C>(S, RequiredBB, PostOccupiedBB,
                          PossiblyCheckmateFromSq)) {
                if constexpr (Capture) {
                    const core::PieceTypeKind CaptureType = getPieceType(
                        S.getPosition().pieceOn(PossiblyCheckmateToSq));

                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(
                            PossiblyCheckmateFromSq, PossiblyCheckmateToSq,
                            Type, CaptureType);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq,
                                                       PossiblyCheckmateToSq,
                                                       Type, CaptureType);
                    }
                } else {
                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(
                            PossiblyCheckmateFromSq, PossiblyCheckmateToSq,
                            Type);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq,
                                                       PossiblyCheckmateToSq,
                                                       Type);
                    }
                }
            }
        }
    }

    return core::Move32::MoveNone();
}

template <core::Color C, core::PieceTypeKind Type>
core::Move32 checkmateByOneStepMove(
    const core::internal::StateImpl& S, core::Square OpKingSq,
    const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OccupiedBB,
    const core::internal::bitboard::Bitboard& NotPinnedBB,
    const CheckOrigins& Origins) {
    const core::internal::bitboard::Bitboard FromBB =
        S.getBitboard<C, Type>() & NotPinnedBB & Origins.BB[OpKingSq][Type];
    if (FromBB.isZero()) {
        return core::Move32::MoveNone();
    }

    core::Move32 PossiblyCheckmateMove =
        checkmateByOneStepMove<C, false, true, Type>(
            S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, true, true, Type>(
        S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, Type>(
        S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, Type>(
        S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    return core::Move32::MoveNone();
}

template <core::Color C>
core::Move32 checkmateByOneStepMove(
    const core::internal::StateImpl& S, core::Square OpKingSq,
    const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OccupiedBB,
    const core::internal::bitboard::Bitboard& NotPinnedBB,
    const CheckOrigins& Origins) {
    core::Move32 PossiblyCheckmateMove =
        checkmateByOneStepMove<C, core::PTK_Pawn>(
            S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB,
            OccupiedBB, NotPinnedBB, Origins);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, core::PTK_Knight>(
        S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB,
        OccupiedBB, NotPinnedBB, Origins);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, core::PTK_Silver>(
        S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB,
        OccupiedBB, NotPinnedBB, Origins);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_Gold>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_Gold];
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, false, false, core::PTK_Gold>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, true, false, core::PTK_Gold>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProPawn>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProPawn];
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, false, false, core::PTK_ProPawn>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, true, false, core::PTK_ProPawn>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProLance>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProLance];
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, false, false, core::PTK_ProLance>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, true, false, core::PTK_ProLance>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProKnight>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProKnight];
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, false, false, core::PTK_ProKnight>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, true, false, core::PTK_ProKnight>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProSilver>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProSilver];
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, false, false, core::PTK_ProSilver>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateByOneStepMove<C, true, false, core::PTK_ProSilver>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    return core::Move32::MoveNone();
}

template <core::Color C, bool Capture, bool Promote, core::PieceTypeKind Type>
core::Move32
checkmateBySliderMove(const core::internal::StateImpl& S,
                      const core::internal::bitboard::Bitboard& ToBB,
                      core::Square OpKingSq,
                      const core::internal::bitboard::Bitboard& OccupiedBB,
                      const core::internal::bitboard::Bitboard& FromBB) {
    core::internal::bitboard::Bitboard PossiblyCheckmateToBB = ToBB;

    const core::internal::bitboard::Bitboard BishopsBB =
        S.getBitboard<~C, core::PTK_Bishop>() |
        S.getBitboard<~C, core::PTK_ProBishop>();
    const core::internal::bitboard::Bitboard RooksBB =
        S.getBitboard<~C, core::PTK_Rook>() |
        S.getBitboard<~C, core::PTK_ProRook>();
    const core::internal::bitboard::Bitboard LancesBB =
        S.getBitboard<~C, core::PTK_Lance>();

    if constexpr (Type == core::PTK_Lance && Promote) {
        PossiblyCheckmateToBB &=
            core::internal::bitboard::getAttackBB<~C, core::PTK_Gold>(OpKingSq);
    } else if constexpr (Promote || core::isPromoted(Type)) {
        PossiblyCheckmateToBB &=
            core::internal::bitboard::KingAttackBB[OpKingSq];
    } else {
        if constexpr (Type == core::PTK_Lance) {
            PossiblyCheckmateToBB &=
                (C == core::Black) ? (S.getBitboard<~C, core::PTK_King>()
                                          .template getRightShiftEpi64<1>())
                                   : (S.getBitboard<~C, core::PTK_King>()
                                          .template getLeftShiftEpi64<1>());
        } else if constexpr (Type == core::PTK_Bishop) {
            PossiblyCheckmateToBB &=
                core::internal::bitboard::DiagStepAttackBB[OpKingSq];
            if constexpr (!Promote) {
                PossiblyCheckmateToBB =
                    core::internal::bitboard::PromotableBB[C].andNot(
                        PossiblyCheckmateToBB);
            }
        } else if constexpr (Type == core::PTK_Rook) {
            PossiblyCheckmateToBB &=
                core::internal::bitboard::CrossStepAttackBB[OpKingSq];
            if constexpr (!Promote) {
                PossiblyCheckmateToBB =
                    core::internal::bitboard::PromotableBB[C].andNot(
                        PossiblyCheckmateToBB);
            }
        }
    }

    while (!PossiblyCheckmateToBB.isZero()) {
        const core::Square PossiblyCheckmateToSq =
            PossiblyCheckmateToBB.popOne();

        if constexpr (Type == core::PTK_Lance && !Promote) {
            if (core::internal::bitboard::Bitboard::SecondFurthestBB<C>().isSet(
                    PossiblyCheckmateToSq)) {
                continue;
            }
        }

        core::internal::bitboard::Bitboard PossiblyCheckmateFromBB;

        // There are few sliders. Filter their rays first, then inspect
        // blockers only for origins on a ray to this checking square.
        if constexpr (Type == core::PTK_Lance) {
            PossiblyCheckmateFromBB = FromBB &
                core::internal::bitboard::getForwardBB<~C>(PossiblyCheckmateToSq);
        } else if constexpr (Type == core::PTK_Bishop ||
                             Type == core::PTK_ProBishop) {
            PossiblyCheckmateFromBB = FromBB &
                core::internal::bitboard::DiagBB[PossiblyCheckmateToSq];
        } else {
            PossiblyCheckmateFromBB = FromBB &
                core::internal::bitboard::CrossBB[PossiblyCheckmateToSq];
        }
        if constexpr (Type == core::PTK_ProBishop || Type == core::PTK_ProRook) {
            PossiblyCheckmateFromBB |= FromBB &
                core::internal::bitboard::KingAttackBB[PossiblyCheckmateToSq];
        }

        if constexpr (Promote) {
            // Given Promote == true, all generated moves must be promotion
            // moves, so by the rule, ToSq or FromSq must be in promotable
            // squares.
            if (!core::internal::bitboard::PromotableBB[C].isSet(
                    PossiblyCheckmateToSq)) {
                PossiblyCheckmateFromBB &=
                    core::internal::bitboard::PromotableBB[C];
            }
        }

        while (!PossiblyCheckmateFromBB.isZero()) {
            const core::Square PossiblyCheckmateFromSq =
                PossiblyCheckmateFromBB.popOne();

            if (!(core::internal::bitboard::getBetweenBB(
                      PossiblyCheckmateFromSq, PossiblyCheckmateToSq) &
                  OccupiedBB).isZero()) {
                continue;
            }

            // Check if it is defenced by opponent's sliders.
            {
                const core::internal::bitboard::Bitboard TempOccupiedBB =
                    OccupiedBB ^ core::internal::bitboard::SquareBB
                                     [PossiblyCheckmateFromSq] |
                    core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

                if (!(core::internal::bitboard::LineBB
                          [PossiblyCheckmateToSq][PossiblyCheckmateFromSq] &
                      BishopsBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getBishopAttackBB<
                              core::PTK_Bishop>(PossiblyCheckmateToSq,
                                                TempOccupiedBB) &
                          BishopsBB)
                             .isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::LineBB
                          [PossiblyCheckmateToSq][PossiblyCheckmateFromSq] &
                      RooksBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getRookAttackBB<
                              core::PTK_Rook>(PossiblyCheckmateToSq,
                                              TempOccupiedBB) &
                          RooksBB)
                             .isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::FileBB[core::squareToFile(
                          PossiblyCheckmateToSq)] &
                      LancesBB)
                         .isZero()) {
                    if (!(core::internal::bitboard::getLanceAttackBB<C>(
                              PossiblyCheckmateToSq, TempOccupiedBB) &
                          LancesBB)
                             .isZero()) {
                        continue;
                    }
                }
            }

            core::internal::bitboard::Bitboard NewAttackedBB =
                core::internal::bitboard::Bitboard::ZeroBB();

            // Add post-move attacks.
            const core::internal::bitboard::Bitboard NewOccupiedBB =
                (OccupiedBB ^
                 core::internal::bitboard::SquareBB[PossiblyCheckmateFromSq] ^
                 S.getBitboard<~C, core::PTK_King>()) |
                core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

            if constexpr (Promote) {
                if constexpr (Type == core::PTK_Lance) {
                    NewAttackedBB |= core::internal::bitboard::getAttackBB<
                        C, core::PTK_Gold>(PossiblyCheckmateToSq);
                } else if constexpr (Type == core::PTK_Bishop) {
                    NewAttackedBB |=
                        core::internal::bitboard::getBishopAttackBB<
                            core::PTK_ProBishop>(PossiblyCheckmateToSq,
                                                 NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Rook) {
                    NewAttackedBB |= core::internal::bitboard::getRookAttackBB<
                        core::PTK_ProRook>(PossiblyCheckmateToSq,
                                           NewOccupiedBB);
                }
            } else {
                if constexpr (Type == core::PTK_Lance) {
                    NewAttackedBB |=
                        core::internal::bitboard::getLanceAttackBB<C>(
                            PossiblyCheckmateToSq, NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Bishop ||
                                     Type == core::PTK_ProBishop) {
                    NewAttackedBB |=
                        core::internal::bitboard::getBishopAttackBB<Type>(
                            PossiblyCheckmateToSq, NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Rook ||
                                     Type == core::PTK_ProRook) {
                    NewAttackedBB |=
                        core::internal::bitboard::getRookAttackBB<Type>(
                            PossiblyCheckmateToSq, NewOccupiedBB);
                }
            }

            core::internal::bitboard::Bitboard RequiredBB =
                (S.getBitboard<~C>() | NewAttackedBB).andNot(
                    core::internal::bitboard::KingAttackBB[OpKingSq]);
            RequiredBB |=
                core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];
            const core::internal::bitboard::Bitboard PostOccupiedBB =
                (OccupiedBB ^ core::internal::bitboard::SquareBB
                                  [PossiblyCheckmateFromSq]) |
                core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];
            if (covers<C>(S, RequiredBB, PostOccupiedBB,
                          PossiblyCheckmateFromSq)) {
                if constexpr (Capture) {
                    const core::PieceTypeKind CaptureType = getPieceType(
                        S.getPosition().pieceOn(PossiblyCheckmateToSq));

                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(
                            PossiblyCheckmateFromSq, PossiblyCheckmateToSq,
                            Type, CaptureType);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq,
                                                       PossiblyCheckmateToSq,
                                                       Type, CaptureType);
                    }
                } else {
                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(
                            PossiblyCheckmateFromSq, PossiblyCheckmateToSq,
                            Type);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq,
                                                       PossiblyCheckmateToSq,
                                                       Type);
                    }
                }
            }
        }
    }

    return core::Move32::MoveNone();
}

template <core::Color C>
core::Move32 checkmateBySliderMove(
    const core::internal::StateImpl& S, core::Square OpKingSq,
    const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB,
    const core::internal::bitboard::Bitboard& OccupiedBB,
    const core::internal::bitboard::Bitboard& NotPinnedBB,
    const CheckOrigins& Origins) {
    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProBishop>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProBishop];
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, false, core::PTK_ProBishop>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, false, core::PTK_ProBishop>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_ProRook>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_ProRook];
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, false, core::PTK_ProRook>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, false, core::PTK_ProRook>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_Bishop>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_Bishop];
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, true, core::PTK_Bishop>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, true, core::PTK_Bishop>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, false, core::PTK_Bishop>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, false, core::PTK_Bishop>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_Rook>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_Rook];
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, true, core::PTK_Rook>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, true, core::PTK_Rook>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, false, core::PTK_Rook>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, false, core::PTK_Rook>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB =
            S.getBitboard<C, core::PTK_Lance>() & NotPinnedBB &
            Origins.BB[OpKingSq][core::PTK_Lance];
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, false, core::PTK_Lance>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, false, true, core::PTK_Lance>(
                    S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, false, core::PTK_Lance>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove =
                checkmateBySliderMove<C, true, true, core::PTK_Lance>(
                    S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB,
                    FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    return core::Move32::MoveNone();
}

} // namespace

template <core::Color C>
core::Move32 solve(const core::internal::StateImpl& S) {
    if (!S.getCheckerBB().isZero()) {
        return core::Move32::MoveNone();
    }

    const auto& Origins = checkOrigins<C>();
    const core::Square OpKingSq = S.getKingSquare<~C>();
    const core::internal::bitboard::Bitboard OccupiedBB =
        S.getBitboard<core::Black>() | S.getBitboard<core::White>();
    const core::Stands St = S.getPosition().getStand<C>();
    const core::internal::bitboard::Bitboard MyStepAttackBB =
        S.getStepAttackBB<C>();
    const core::internal::bitboard::Bitboard MySliderAttackBB =
        sliderAttacks<C, C>(S, OpKingSq);
    const core::internal::bitboard::Bitboard MyStepOrSliderAttackBB =
        MyStepAttackBB | MySliderAttackBB;
    const core::internal::bitboard::Bitboard KnightToBB =
        core::internal::bitboard::getAttackBB<~C, core::PTK_Knight>(OpKingSq);
    const core::internal::bitboard::Bitboard TargetsBB =
        core::internal::bitboard::KingAttackBB[OpKingSq] | KnightToBB;
    // A board move must already attack its destination. Adjacent drops
    // need support as well; only a knight drop can mate without support.
    if (S.getBitboard<C>().andNot(TargetsBB & MyStepOrSliderAttackBB).isZero() &&
        (core::getStandCount<core::PTK_Knight>(St) == 0 ||
         OccupiedBB.andNot(KnightToBB).isZero())) {
        return core::Move32::MoveNone();
    }
    const core::internal::bitboard::Bitboard OpAttackBB =
        S.getStepAttackBB<~C>(OpKingSq) | sliderAttacks<~C, C>(S, OpKingSq);
    // Restrict drops to real checking squares. In particular, shifting a
    // king on 7a must not turn the unused low-lane bit into a lance drop on 8i.
    const core::internal::bitboard::Bitboard EmptyAndNotOpAttackBB =
        (OccupiedBB | OpAttackBB).andNot(TargetsBB);

    // Checkmate by dropping.
    if (St != 0) {
        const core::internal::bitboard::Bitboard EmptyAndAttackBB =
            OccupiedBB.andNot(MyStepAttackBB | MySliderAttackBB);
        const core::internal::bitboard::Bitboard
            EmptyAndAttackAndNotOpAttackBB =
                OpAttackBB.andNot(EmptyAndAttackBB) & TargetsBB;

        const core::Move32 CheckmateByKnightDrop =
            checkmateByDrop<C, core::PTK_Knight>(
                S, St, EmptyAndNotOpAttackBB, OpKingSq, MyStepAttackBB,
                MySliderAttackBB, MyStepOrSliderAttackBB);

        if (!CheckmateByKnightDrop.isNone()) {
            return CheckmateByKnightDrop;
        }

        const core::Move32 CheckmateByDrop =
            checkmateByDrops<C, core::PTK_Gold, core::PTK_Lance,
                             core::PTK_Bishop, core::PTK_Rook,
                             core::PTK_Silver>(
                S, St, EmptyAndAttackAndNotOpAttackBB, OpKingSq, MyStepAttackBB,
                MySliderAttackBB, MyStepOrSliderAttackBB);

        if (!CheckmateByDrop.isNone()) {
            return CheckmateByDrop;
        }
    }

    const core::internal::bitboard::Bitboard OpOccupiedAndNotOpAttackBB =
        OpAttackBB.andNot(S.getBitboard<~C>() & TargetsBB);
    if (((EmptyAndNotOpAttackBB | OpOccupiedAndNotOpAttackBB) &
         MyStepOrSliderAttackBB).isZero()) {
        return core::Move32::MoveNone();
    }
    const core::internal::bitboard::Bitboard NotPinnedBB =
        ~S.getDefendingOpponentSliderBB<C>();

    // Checkmate by on-board moves.
    const core::Move32 PossiblyCheckmateOneStepMove = checkmateByOneStepMove<C>(
        S, OpKingSq, EmptyAndNotOpAttackBB & MyStepOrSliderAttackBB,
        OpOccupiedAndNotOpAttackBB & MyStepOrSliderAttackBB, OccupiedBB,
        NotPinnedBB, Origins);

    if (!PossiblyCheckmateOneStepMove.isNone()) {
        return PossiblyCheckmateOneStepMove;
    }

    const core::Move32 PossiblyCheckmateSliderMove = checkmateBySliderMove<C>(
        S, OpKingSq, EmptyAndNotOpAttackBB & MyStepOrSliderAttackBB,
        OpOccupiedAndNotOpAttackBB & MyStepOrSliderAttackBB, OccupiedBB,
        NotPinnedBB, Origins);

    if (!PossiblyCheckmateSliderMove.isNone()) {
        return PossiblyCheckmateSliderMove;
    }

    return core::Move32::MoveNone();
}

template core::Move32 solver::internal::mate1ply::solve<core::Color::Black>(
    const core::internal::StateImpl&);
template core::Move32 solver::internal::mate1ply::solve<core::Color::White>(
    const core::internal::StateImpl&);

} // namespace mate1ply
} // namespace internal
} // namespace solver
} // namespace nshogi
