#include "mate1ply.h"

namespace nshogi {
namespace solver {
namespace internal {
namespace mate1ply {

namespace {

template<core::Color C>
bool isEvadable(core::Square KingSq, const core::internal::bitboard::Bitboard& AttackedBB, const core::internal::bitboard::Bitboard& MyOccupiedBB) {
    const core::internal::bitboard::Bitboard& KingAttackBB = core::internal::bitboard::KingAttackBB[KingSq];
    const core::internal::bitboard::Bitboard PossibleEvadableBB = MyOccupiedBB.andNot(KingAttackBB);

    // If all possibly-evadable squares are attacked, the king is captured.
    return (PossibleEvadableBB & AttackedBB) != PossibleEvadableBB;
}

template<core::Color C, core::PieceTypeKind Type>
core::Move32 checkmateByDrop(const core::internal::StateImpl& S, core::Stands St, core::internal::bitboard::Bitboard ToBB, core::Square OpKingSq, const core::internal::bitboard::Bitboard& StepAttackBB, const core::internal::bitboard::Bitboard& SliderAttackBB, const core::internal::bitboard::Bitboard& StepOrSliderAttackBB) {
    static_assert(Type != core::PTK_Pawn, "PTK_Pawn must not be passed as `Type` because"
                                          "one-ply checkmate is prohibitted by the rule.");

    static_assert(Type == core::PTK_Lance || Type == core::PTK_Knight || Type == core::PTK_Silver ||
            Type == core::PTK_Gold || Type == core::PTK_Bishop || Type == core::PTK_Rook,
            "Passed invalid `Type`.");

    if (core::getStandCount<Type>(St) == 0) {
        return core::Move32::MoveNone();
    }

    if constexpr (Type == core::PTK_Gold) {
        ToBB &= core::internal::bitboard::getAttackBB<~C, core::PTK_Gold>(OpKingSq);
    } else if constexpr (Type == core::PTK_Silver) {
        ToBB &= core::internal::bitboard::getAttackBB<~C, core::PTK_Silver>(OpKingSq);
    } else if constexpr (Type == core::PTK_Knight) {
        ToBB &= core::internal::bitboard::getAttackBB<~C, core::PTK_Knight>(OpKingSq);
    } else if constexpr (Type == core::PTK_Bishop) {
        ToBB &= core::internal::bitboard::DiagStepAttackBB[OpKingSq];
    } else if constexpr (Type == core::PTK_Rook) {
        ToBB &= core::internal::bitboard::CrossStepAttackBB[OpKingSq];
    } else if constexpr (Type == core::PTK_Lance) {
        ToBB = core::internal::bitboard::FurthermostBB[C].andNot(ToBB);
        ToBB &= (C == core::Black)? (S.getBitboard<~C, core::PTK_King>().template getRightShiftEpi64<1>()) : (S.getBitboard<~C, core::PTK_King>().template getLeftShiftEpi64<1>());
    }

    while (!ToBB.isZero()) {
        const core::Square ToSq = ToBB.popOne();
        core::internal::bitboard::Bitboard NewAttackBB = (SliderAttackBB.isSet(ToSq))? (StepAttackBB | S.getSliderAttackBB<C>(/* ExcludeSq = */ core::SqInvalid, /* VirtualSq = */ ToSq))
                                                                           : StepOrSliderAttackBB;

        if constexpr (Type == core::PTK_Gold) {
            NewAttackBB |= core::internal::bitboard::GoldAttackBB[C][ToSq];
        } else if constexpr (Type == core::PTK_Silver) {
            NewAttackBB |= core::internal::bitboard::SilverAttackBB[C][ToSq];
        } else if constexpr (Type == core::PTK_Knight) {
            NewAttackBB |= core::internal::bitboard::KnightAttackBB[C][ToSq];
        } else if constexpr (Type == core::PTK_Bishop) {
            const core::internal::bitboard::Bitboard NewOccupiedBB = (S.getBitboard<core::Black>() | S.getBitboard<core::White>() | core::internal::bitboard::SquareBB[ToSq]) ^ S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |= core::internal::bitboard::getBishopAttackBB<core::PTK_Bishop>(ToSq, NewOccupiedBB);
        } else if constexpr (Type == core::PTK_Rook) {
            const core::internal::bitboard::Bitboard NewOccupiedBB = (S.getBitboard<core::Black>() | S.getBitboard<core::White>() | core::internal::bitboard::SquareBB[ToSq]) ^ S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |= core::internal::bitboard::getRookAttackBB<core::PTK_Rook>(ToSq, NewOccupiedBB);
        } else if constexpr (Type == core::PTK_Lance) {
            const core::internal::bitboard::Bitboard NewOccupiedBB = (S.getBitboard<core::Black>() | S.getBitboard<core::White>() | core::internal::bitboard::SquareBB[ToSq]) ^ S.getBitboard<~C, core::PTK_King>();
            NewAttackBB |= core::internal::bitboard::getLanceAttackBB<C>(ToSq, NewOccupiedBB);
        }

        if (!isEvadable<~C>(OpKingSq, NewAttackBB, S.getBitboard<~C>())) {
            return core::Move32::droppingMove(ToSq, Type);
        }
    }

    return core::Move32::MoveNone();
}

template<core::Color C>
core::Move32 checkmateByDrops(const core::internal::StateImpl&, core::Stands, const core::internal::bitboard::Bitboard&, core::Square, const core::internal::bitboard::Bitboard&, const core::internal::bitboard::Bitboard&, const core::internal::bitboard::Bitboard&) {
    return core::Move32::MoveNone();
}

template<core::Color C, core::PieceTypeKind Type, core::PieceTypeKind... Types>
core::Move32 checkmateByDrops(const core::internal::StateImpl& S, core::Stands St, const core::internal::bitboard::Bitboard& ToBB, core::Square OpKingSq, const core::internal::bitboard::Bitboard& StepAttackBB, const core::internal::bitboard::Bitboard& SliderAttackBB, const core::internal::bitboard::Bitboard& StepOrSliderAttackBB) {
    const core::Move32 PossiblyCheckmateMove = checkmateByDrop<C, Type>(S, St, ToBB, OpKingSq, StepAttackBB, SliderAttackBB, StepOrSliderAttackBB);

    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    return checkmateByDrops<C, Types...>(S, St, ToBB, OpKingSq, StepAttackBB, SliderAttackBB, StepOrSliderAttackBB);
}

template<core::Color C, bool Capture, bool Promote, core::PieceTypeKind Type>
core::Move32 checkmateByOneStepMove(const core::internal::StateImpl& S, const core::internal::bitboard::Bitboard& ToBB, core::Square OpKingSq, const core::internal::bitboard::Bitboard& OccupiedBB, const core::internal::bitboard::Bitboard& FromBB) {
    core::internal::bitboard::Bitboard PossiblyCheckmateToBB = ToBB;

    const core::internal::bitboard::Bitboard BishopsBB = S.getBitboard<~C, core::PTK_Bishop>() | S.getBitboard<~C, core::PTK_ProBishop>();
    const core::internal::bitboard::Bitboard RooksBB = S.getBitboard<~C, core::PTK_Rook>() | S.getBitboard<~C, core::PTK_ProRook>();
    const core::internal::bitboard::Bitboard LancesBB = S.getBitboard<~C, core::PTK_Lance>();

    if constexpr (Promote) {
        PossiblyCheckmateToBB &= core::internal::bitboard::getAttackBB<~C, core::promotePieceType(Type)>(OpKingSq);
    } else {
        PossiblyCheckmateToBB &= core::internal::bitboard::getAttackBB<~C, Type>(OpKingSq);
        if constexpr (Type == core::PTK_Pawn) {
            PossiblyCheckmateToBB = core::internal::bitboard::PromotableBB[C].andNot(PossiblyCheckmateToBB);
        }
    }

    while (!PossiblyCheckmateToBB.isZero()) {
        const core::Square PossiblyCheckmateToSq = PossiblyCheckmateToBB.popOne();

        core::internal::bitboard::Bitboard PossiblyCheckmateFromBB =
            FromBB & core::internal::bitboard::getAttackBB<~C, Type>(PossiblyCheckmateToSq);

        if constexpr (Promote) {
            // Given Promote == true, all generated moves must be promotion moves,
            // so by the rule, ToSq or FromSq must be in promotable squares.
            if (!core::internal::bitboard::PromotableBB[C].isSet(PossiblyCheckmateToSq)) {
                PossiblyCheckmateFromBB &= core::internal::bitboard::PromotableBB[C];
            }
        }

        while (!PossiblyCheckmateFromBB.isZero()) {
            const core::Square PossiblyCheckmateFromSq = PossiblyCheckmateFromBB.popOne();

            // Check if it is defenced by opponent's sliders.
            {
                const core::internal::bitboard::Bitboard TempOccupiedBB = OccupiedBB ^ core::internal::bitboard::SquareBB[PossiblyCheckmateFromSq] | core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

                if (!(core::internal::bitboard::LineBB[PossiblyCheckmateToSq][PossiblyCheckmateFromSq] & BishopsBB).isZero()) {
                    if (!(core::internal::bitboard::getBishopAttackBB<core::PTK_Bishop>(PossiblyCheckmateToSq, TempOccupiedBB) & BishopsBB).isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::LineBB[PossiblyCheckmateToSq][PossiblyCheckmateFromSq] & RooksBB).isZero()) {
                    if (!(core::internal::bitboard::getRookAttackBB<core::PTK_Rook>(PossiblyCheckmateToSq, TempOccupiedBB) & RooksBB).isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::FileBB[core::squareToFile(PossiblyCheckmateToSq)] & LancesBB).isZero()) {
                    if (!(core::internal::bitboard::getLanceAttackBB<C>(PossiblyCheckmateToSq, TempOccupiedBB) & LancesBB).isZero()) {
                        continue;
                    }
                }
            }

            // Compute the new attack bitboard.
            // Remove pre-move attacks.
            core::internal::bitboard::Bitboard NewAttackedBB = S.getStepAttackBB<C>(/* ExcludeSq = */ PossiblyCheckmateFromSq) |
                                                     S.getSliderAttackBB<C>(/* ExcludeSq = */ PossiblyCheckmateFromSq, /* VirtualSq = */ PossiblyCheckmateToSq);

            // If the piece type is not knight, the target square
            // must be attacked by other my piece(s).
            if constexpr (Type != core::PTK_Knight) {
                if (!NewAttackedBB.isSet(PossiblyCheckmateToSq)) {
                    continue;
                }
            }

            // Even if the type is knight, if the target square is
            // adjacent to the opponent king (it is if and only if
            // Promote == true), the square must be
            // attacked by other my piece(s).
            if constexpr (Type == core::PTK_Knight && Promote) {
                if (!NewAttackedBB.isSet(PossiblyCheckmateToSq)) {
                    continue;
                }
            }

            // Add post-move attacks.
            if constexpr (Promote) {
                NewAttackedBB |= core::internal::bitboard::getAttackBB<C, core::promotePieceType(Type)>(PossiblyCheckmateToSq);
            } else {
                NewAttackedBB |= core::internal::bitboard::getAttackBB<C, Type>(PossiblyCheckmateToSq);
            }

            if (!isEvadable<~C>(OpKingSq, NewAttackedBB, S.getBitboard<~C>())) {
                if constexpr (Capture) {
                    const core::PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(PossiblyCheckmateToSq));

                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type, CaptureType);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type, CaptureType);
                    }
                } else {
                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type);
                    }
                }
            }
        }
    }

    return core::Move32::MoveNone();
}

template<core::Color C, core::PieceTypeKind Type>
core::Move32 checkmateByOneStepMove(const core::internal::StateImpl& S, core::Square OpKingSq, const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OccupiedBB, const core::internal::bitboard::Bitboard& NotPinnedBB) {
    const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, Type>() & NotPinnedBB;
    if (FromBB.isZero()) {
        return core::Move32::MoveNone();
    }

    core::Move32 PossiblyCheckmateMove = checkmateByOneStepMove<C, false, true, Type>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, true, true, Type>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, Type>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, Type>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    return core::Move32::MoveNone();
}

template<core::Color C>
core::Move32 checkmateByOneStepMove(const core::internal::StateImpl& S, core::Square OpKingSq, const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OccupiedBB, const core::internal::bitboard::Bitboard& NotPinnedBB) {
    core::Move32 PossiblyCheckmateMove = checkmateByOneStepMove<C, core::PTK_Pawn>(S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB, OccupiedBB, NotPinnedBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, core::PTK_Knight>(S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB, OccupiedBB, NotPinnedBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    PossiblyCheckmateMove = checkmateByOneStepMove<C, core::PTK_Silver>(S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB, OccupiedBB, NotPinnedBB);
    if (!PossiblyCheckmateMove.isNone()) {
        return PossiblyCheckmateMove;
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_Gold>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, core::PTK_Gold>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, core::PTK_Gold>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProPawn>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, core::PTK_ProPawn>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, core::PTK_ProPawn>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProLance>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, core::PTK_ProLance>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, core::PTK_ProLance>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProKnight>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, core::PTK_ProKnight>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, core::PTK_ProKnight>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProSilver>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            PossiblyCheckmateMove = checkmateByOneStepMove<C, false, false, core::PTK_ProSilver>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateByOneStepMove<C, true, false, core::PTK_ProSilver>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);
            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    return core::Move32::MoveNone();
}

template<core::Color C, bool Capture, bool Promote, core::PieceTypeKind Type>
core::Move32 checkmateBySliderMove(const core::internal::StateImpl& S, const core::internal::bitboard::Bitboard& ToBB, core::Square OpKingSq, const core::internal::bitboard::Bitboard& OccupiedBB, const core::internal::bitboard::Bitboard& FromBB) {
    core::internal::bitboard::Bitboard PossiblyCheckmateToBB = ToBB;

    const core::internal::bitboard::Bitboard BishopsBB = S.getBitboard<~C, core::PTK_Bishop>() | S.getBitboard<~C, core::PTK_ProBishop>();
    const core::internal::bitboard::Bitboard RooksBB = S.getBitboard<~C, core::PTK_Rook>() | S.getBitboard<~C, core::PTK_ProRook>();
    const core::internal::bitboard::Bitboard LancesBB = S.getBitboard<~C, core::PTK_Lance>();

    if constexpr (Type == core::PTK_Lance && Promote) {
        PossiblyCheckmateToBB &= core::internal::bitboard::GoldAttackBB[~C][OpKingSq];
    } else if constexpr (Promote || core::isPromoted(Type)) {
        PossiblyCheckmateToBB &= core::internal::bitboard::KingAttackBB[OpKingSq];
    } else {
        if constexpr (Type == core::PTK_Lance) {
            PossiblyCheckmateToBB &= (C == core::Black)? (S.getBitboard<~C, core::PTK_King>().template getRightShiftEpi64<1>())
                                                       : (S.getBitboard<~C, core::PTK_King>().template getLeftShiftEpi64<1>());
        } else if constexpr (Type == core::PTK_Bishop) {
            PossiblyCheckmateToBB &= core::internal::bitboard::DiagStepAttackBB[OpKingSq];
            if constexpr (!Promote) {
                PossiblyCheckmateToBB = core::internal::bitboard::PromotableBB[C].andNot(PossiblyCheckmateToBB);
            }
        } else if constexpr (Type == core::PTK_Rook) {
            PossiblyCheckmateToBB &= core::internal::bitboard::CrossStepAttackBB[OpKingSq];
            if constexpr (!Promote) {
                PossiblyCheckmateToBB = core::internal::bitboard::PromotableBB[C].andNot(PossiblyCheckmateToBB);
            }
        }
    }

    while (!PossiblyCheckmateToBB.isZero()) {
        const core::Square PossiblyCheckmateToSq = PossiblyCheckmateToBB.popOne();

        if constexpr (Type == core::PTK_Lance && !Promote) {
            if (core::internal::bitboard::SecondFurthestBB[C].isSet(PossiblyCheckmateToSq)) {
                continue;
            }
        }

        core::internal::bitboard::Bitboard PossiblyCheckmateFromBB;

        if constexpr (Type == core::PTK_Lance) {
            PossiblyCheckmateFromBB = FromBB & core::internal::bitboard::getLanceAttackBB<~C>(PossiblyCheckmateToSq, OccupiedBB);
        } else if constexpr (Type == core::PTK_Bishop || Type == core::PTK_ProBishop) {
            PossiblyCheckmateFromBB = FromBB & core::internal::bitboard::getBishopAttackBB<Type>(PossiblyCheckmateToSq, OccupiedBB);
        } else if constexpr (Type == core::PTK_Rook || Type == core::PTK_ProRook) {
            PossiblyCheckmateFromBB = FromBB & core::internal::bitboard::getRookAttackBB<Type>(PossiblyCheckmateToSq, OccupiedBB);
        }

        if constexpr (Promote) {
            // Given Promote == true, all generated moves must be promotion moves,
            // so by the rule, ToSq or FromSq must be in promotable squares.
            if (!core::internal::bitboard::PromotableBB[C].isSet(PossiblyCheckmateToSq)) {
                PossiblyCheckmateFromBB &= core::internal::bitboard::PromotableBB[C];
            }
        }

        while (!PossiblyCheckmateFromBB.isZero()) {
            const core::Square PossiblyCheckmateFromSq = PossiblyCheckmateFromBB.popOne();

            // Check if it is defenced by opponent's sliders.
            {
                const core::internal::bitboard::Bitboard TempOccupiedBB = OccupiedBB ^ core::internal::bitboard::SquareBB[PossiblyCheckmateFromSq] | core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

                if (!(core::internal::bitboard::LineBB[PossiblyCheckmateToSq][PossiblyCheckmateFromSq] & BishopsBB).isZero()) {
                    if (!(core::internal::bitboard::getBishopAttackBB<core::PTK_Bishop>(PossiblyCheckmateToSq, TempOccupiedBB) & BishopsBB).isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::LineBB[PossiblyCheckmateToSq][PossiblyCheckmateFromSq] & RooksBB).isZero()) {
                    if (!(core::internal::bitboard::getRookAttackBB<core::PTK_Rook>(PossiblyCheckmateToSq, TempOccupiedBB) & RooksBB).isZero()) {
                        continue;
                    }
                }

                if (!(core::internal::bitboard::FileBB[core::squareToFile(PossiblyCheckmateToSq)] & LancesBB).isZero()) {
                    if (!(core::internal::bitboard::getLanceAttackBB<C>(PossiblyCheckmateToSq, TempOccupiedBB) & LancesBB).isZero()) {
                        continue;
                    }
                }
            }

            // Compute the new attack bitboard.
            // Remove pre-move attacks.
            core::internal::bitboard::Bitboard NewAttackedBB = S.getStepAttackBB<C>(/* ExcludeSq = */ PossiblyCheckmateFromSq) |
                                                     S.getSliderAttackBB<C>(/* ExcludeSq = */ PossiblyCheckmateFromSq, /* Virtual Sq = */PossiblyCheckmateToSq);

            if (!NewAttackedBB.isSet(PossiblyCheckmateToSq)) {
                continue;
            }

            // Add post-move attacks.
            const core::internal::bitboard::Bitboard NewOccupiedBB = (OccupiedBB ^ core::internal::bitboard::SquareBB[PossiblyCheckmateFromSq] ^ S.getBitboard<~C, core::PTK_King>()) |
                                                           core::internal::bitboard::SquareBB[PossiblyCheckmateToSq];

            if constexpr (Promote) {
                if constexpr (Type == core::PTK_Lance) {
                    NewAttackedBB |= core::internal::bitboard::GoldAttackBB[C][PossiblyCheckmateToSq];
                } else if constexpr (Type == core::PTK_Bishop) {
                    NewAttackedBB |= core::internal::bitboard::getBishopAttackBB<core::PTK_ProBishop>(PossiblyCheckmateToSq, NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Rook) {
                    NewAttackedBB |= core::internal::bitboard::getRookAttackBB<core::PTK_ProRook>(PossiblyCheckmateToSq, NewOccupiedBB);
                }
            } else {
                if constexpr (Type == core::PTK_Lance) {
                    NewAttackedBB |= core::internal::bitboard::getLanceAttackBB<C>(PossiblyCheckmateToSq, NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Bishop || Type == core::PTK_ProBishop) {
                    NewAttackedBB |= core::internal::bitboard::getBishopAttackBB<Type>(PossiblyCheckmateToSq, NewOccupiedBB);
                } else if constexpr (Type == core::PTK_Rook || Type == core::PTK_ProRook) {
                    NewAttackedBB |= core::internal::bitboard::getRookAttackBB<Type>(PossiblyCheckmateToSq, NewOccupiedBB);
                }
            }

            if (!isEvadable<~C>(OpKingSq, NewAttackedBB, S.getBitboard<~C>())) {
                if constexpr (Capture) {
                    const core::PieceTypeKind CaptureType =
                        getPieceType(S.getPosition().pieceOn(PossiblyCheckmateToSq));

                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type, CaptureType);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type, CaptureType);
                    }
                } else {
                    if constexpr (Promote) {
                        return core::Move32::boardPromotingMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type);
                    } else {
                        return core::Move32::boardMove(PossiblyCheckmateFromSq, PossiblyCheckmateToSq, Type);
                    }
                }
            }
        }
    }

    return core::Move32::MoveNone();
}

template<core::Color C>
core::Move32 checkmateBySliderMove(const core::internal::StateImpl& S, core::Square OpKingSq, const core::internal::bitboard::Bitboard& EmptyAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OpOccupiedAndNotOpAttackBB, const core::internal::bitboard::Bitboard& OccupiedBB, const core::internal::bitboard::Bitboard& NotPinnedBB) {
    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProBishop>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove = checkmateBySliderMove<C, false, false, core::PTK_ProBishop>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, false, core::PTK_ProBishop>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_ProRook>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove = checkmateBySliderMove<C, false, false, core::PTK_ProRook>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, false, core::PTK_ProRook>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_Bishop>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove = checkmateBySliderMove<C, false, true, core::PTK_Bishop>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, true, core::PTK_Bishop>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, false, false, core::PTK_Bishop>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, false, core::PTK_Bishop>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_Rook>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove = checkmateBySliderMove<C, false, true, core::PTK_Rook>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, true, core::PTK_Rook>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }


            PossiblyCheckmateMove = checkmateBySliderMove<C, false, false, core::PTK_Rook>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }


            PossiblyCheckmateMove = checkmateBySliderMove<C, true, false, core::PTK_Rook>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    {
        const core::internal::bitboard::Bitboard FromBB = S.getBitboard<C, core::PTK_Lance>() & NotPinnedBB;
        if (!FromBB.isZero()) {
            core::Move32 PossiblyCheckmateMove = checkmateBySliderMove<C, false, false, core::PTK_Lance>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, false, true, core::PTK_Lance>(S, EmptyAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, false, core::PTK_Lance>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }

            PossiblyCheckmateMove = checkmateBySliderMove<C, true, true, core::PTK_Lance>(S, OpOccupiedAndNotOpAttackBB, OpKingSq, OccupiedBB, FromBB);

            if (!PossiblyCheckmateMove.isNone()) {
                return PossiblyCheckmateMove;
            }
        }
    }

    return core::Move32::MoveNone();
}

} // namespace

template<core::Color C>
core::Move32 solve(const core::internal::StateImpl& S) {
    if (!S.getCheckerBB().isZero()) {
        return core::Move32::MoveNone();
    }

    const core::Square OpKingSq = S.getKingSquare<~C>();
    const core::internal::bitboard::Bitboard OpAttackBB = S.getStepAttackBB<~C>(OpKingSq) | S.getSliderAttackBB<~C>();
    const core::internal::bitboard::Bitboard OccupiedBB = S.getBitboard<core::Black>() | S.getBitboard<core::White>();

    const core::internal::bitboard::Bitboard EmptyAndNotOpAttackBB = OccupiedBB.andNot(~OpAttackBB);

    const core::Stands St = S.getPosition().getStand<C>();

    // Checkmate by dropping.
    if (St != 0) {
        const core::internal::bitboard::Bitboard MyStepAttackBB = S.getStepAttackBB<C>();
        const core::internal::bitboard::Bitboard MySliderAttackBB = S.getSliderAttackBB<C>();
        const core::internal::bitboard::Bitboard MyStepOrSliderAttackBB = MyStepAttackBB | MySliderAttackBB;
        const core::internal::bitboard::Bitboard EmptyAndAttackBB = OccupiedBB.andNot(MyStepAttackBB | MySliderAttackBB);
        const core::internal::bitboard::Bitboard EmptyAndAttackAndNotOpAttackBB = OpAttackBB.andNot(EmptyAndAttackBB);

        const core::Move32 CheckmateByKnightDrop =
            checkmateByDrop<C, core::PTK_Knight>(S, St, EmptyAndNotOpAttackBB, OpKingSq, MyStepAttackBB, MySliderAttackBB, MyStepOrSliderAttackBB);

        if (!CheckmateByKnightDrop.isNone()) {
            return CheckmateByKnightDrop;
        }

        const core::Move32 CheckmateByDrop =
            checkmateByDrops<C, core::PTK_Gold, core::PTK_Lance, core::PTK_Bishop, core::PTK_Rook, core::PTK_Silver>
            (S, St, EmptyAndAttackAndNotOpAttackBB, OpKingSq, MyStepAttackBB, MySliderAttackBB, MyStepOrSliderAttackBB);

        if (!CheckmateByDrop.isNone()) {
            return CheckmateByDrop;
        }
    }

    const core::internal::bitboard::Bitboard OpOccupiedAndNotOpAttackBB = OpAttackBB.andNot(S.getBitboard<~C>());
    const core::internal::bitboard::Bitboard NotPinnedBB = ~S.getDefendingOpponentSliderBB<C>();

    // Checkmate by on-board moves.
    const core::Move32 PossiblyCheckmateOneStepMove =
        checkmateByOneStepMove<C>(S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB, OccupiedBB, NotPinnedBB);

    if (!PossiblyCheckmateOneStepMove.isNone()) {
        return PossiblyCheckmateOneStepMove;
    }

    const core::Move32 PossiblyCheckmateSliderMove =
        checkmateBySliderMove<C>(S, OpKingSq, EmptyAndNotOpAttackBB, OpOccupiedAndNotOpAttackBB, OccupiedBB, NotPinnedBB);

    if (!PossiblyCheckmateSliderMove.isNone()) {
        return PossiblyCheckmateSliderMove;
    }

    return core::Move32::MoveNone();
}

template
core::Move32 solver::internal::mate1ply::solve<core::Color::Black>(const core::internal::StateImpl&);
template
core::Move32 solver::internal::mate1ply::solve<core::Color::White>(const core::internal::StateImpl&);

} // namespace mate1ply
} // namespace internal
} // namespace solver
} // namespace nshogi
