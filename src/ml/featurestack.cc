//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "featurestack.h"
#include "../core/internal/stateadapter.h"
#include "featuretype.h"
#include "internal/featurebitboardutil.h"

#include <cassert>

namespace nshogi {
namespace ml {

template <>
FeatureBitboard processIsBlackTurn<core::Black>() {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), 1, false);
}

template <>
FeatureBitboard processIsBlackTurn<core::White>() {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template <>
FeatureBitboard processIsWhiteTurn<core::Black>() {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template <>
FeatureBitboard processIsWhiteTurn<core::White>() {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), 1, false);
}

FeatureBitboard processCheck(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    if (!Adapter->getCheckerBB().isZero()) {
        return internal::FeatureBitboardUtil::makeFeatureBitboard(
            core::internal::bitboard::Bitboard::AllBB(), 1, false);
    }

    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template <core::Color MyColor, core::Color TargetColor>
FeatureBitboard processNoPawnFile(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    const core::internal::bitboard::Bitboard PawnBB =
        Adapter->getBitboard<TargetColor, core::PTK_Pawn>();
    core::internal::bitboard::Bitboard PawnExistFilesBB =
        core::internal::bitboard::Bitboard::ZeroBB();

    if (!(PawnBB & core::internal::bitboard::FileBB[core::File1]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File1];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File2]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File2];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File3]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File3];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File4]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File4];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File5]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File5];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File6]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File6];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File7]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File7];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File8]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File8];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File9]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File9];
    }

    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        ~PawnExistFilesBB, 1, MyColor == core::White);
}

template FeatureBitboard
processNoPawnFile<core::Black, core::Black>(const core::State& State);
template FeatureBitboard
processNoPawnFile<core::Black, core::White>(const core::State& State);
template FeatureBitboard
processNoPawnFile<core::White, core::Black>(const core::State& State);
template FeatureBitboard
processNoPawnFile<core::White, core::White>(const core::State& State);

FeatureBitboard processProgress(const core::State& State,
                                const core::StateConfig& Config) {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(),
        (float)State.getPly() / Config.MaxPly, false);
}

FeatureBitboard processProgressUnit(const core::StateConfig& Config) {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), 1.0f / Config.MaxPly,
        false);
}

template <core::Color MyColor, core::Color PieceColor>
FeatureBitboard processPiece(const core::State& State,
                             core::PieceTypeKind Type) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        Adapter->getBitboard<PieceColor>(Type), 1, MyColor == core::White);
}

template <core::Color MyColor, core::Color PieceColor>
FeatureBitboard processPiece(const core::State& State,
                             core::PieceTypeKind Type1,
                             core::PieceTypeKind Type2) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        (Adapter->getBitboard(Type1) | Adapter->getBitboard(Type2)) &
            Adapter->getBitboard<PieceColor>(),
        1, MyColor == core::White);
}

template FeatureBitboard
processPiece<core::Black, core::Black>(const core::State& State,
                                       core::PieceTypeKind Type);
template FeatureBitboard
processPiece<core::Black, core::White>(const core::State& State,
                                       core::PieceTypeKind Type);
template FeatureBitboard
processPiece<core::White, core::Black>(const core::State& State,
                                       core::PieceTypeKind Type);
template FeatureBitboard
processPiece<core::White, core::White>(const core::State& State,
                                       core::PieceTypeKind Type);

template FeatureBitboard
processPiece<core::Black, core::Black>(const core::State& State,
                                       core::PieceTypeKind Type1,
                                       core::PieceTypeKind Type2);
template FeatureBitboard
processPiece<core::Black, core::White>(const core::State& State,
                                       core::PieceTypeKind Type1,
                                       core::PieceTypeKind Type2);
template FeatureBitboard
processPiece<core::White, core::Black>(const core::State& State,
                                       core::PieceTypeKind Type1,
                                       core::PieceTypeKind Type2);
template FeatureBitboard
processPiece<core::White, core::White>(const core::State& State,
                                       core::PieceTypeKind Type1,
                                       core::PieceTypeKind Type2);

template <core::Color MyColor>
FeatureBitboard processStand(const core::State& State, core::PieceTypeKind Type,
                             uint8_t Count) {
    uint8_t TypeStandCount = State.getPosition().getStandCount<MyColor>(Type);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), TypeStandCount >= Count,
        false);
}

template FeatureBitboard processStand<core::Black>(const core::State& State,
                                                   core::PieceTypeKind Type,
                                                   uint8_t Count);
template FeatureBitboard processStand<core::White>(const core::State& State,
                                                   core::PieceTypeKind Type,
                                                   uint8_t Count);

template <core::EndingRule Rule>
FeatureBitboard processRule(const core::StateConfig& Config) {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), Config.Rule == Rule,
        false);
}

template FeatureBitboard
processRule<core::EndingRule::ER_NoRule>(const core::StateConfig& Config);
template FeatureBitboard
processRule<core::EndingRule::ER_Draw24>(const core::StateConfig& Config);
template FeatureBitboard
processRule<core::EndingRule::ER_Trying>(const core::StateConfig& Config);
template FeatureBitboard
processRule<core::EndingRule::ER_Declare27>(const core::StateConfig& Config);

template <>
FeatureBitboard processDrawValue<core::Black>(const core::StateConfig& Config) {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), Config.BlackDrawValue,
        false);
}

template <>
FeatureBitboard processDrawValue<core::White>(const core::StateConfig& Config) {
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(), Config.WhiteDrawValue,
        false);
}

template <>
FeatureBitboard processDeclarationScore<core::Black>(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(),
        (float)Adapter->computeDeclarationScore<core::Black>() / 28.0f, false);
}

template <>
FeatureBitboard processDeclarationScore<core::White>(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(),
        (float)Adapter->computeDeclarationScore<core::White>() / 27.0f, false);
}

template <core::Color C>
FeatureBitboard processPieceScore(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(),
        (float)Adapter->computePieceScore<C>(5, 1, false) / 54.0f, false);
}

template FeatureBitboard
processPieceScore<core::Black>(const core::State& State);
template FeatureBitboard
processPieceScore<core::White>(const core::State& State);

template <core::Color MyColor, core::Color TargetColor>
FeatureBitboard processAttack(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        Adapter->getAttackBB<TargetColor>(), 1.0f, MyColor == core::White);
}

template FeatureBitboard
processAttack<core::Black, core::Black>(const core::State& State);
template FeatureBitboard
processAttack<core::Black, core::White>(const core::State& State);
template FeatureBitboard
processAttack<core::White, core::Black>(const core::State& State);
template FeatureBitboard
processAttack<core::White, core::White>(const core::State& State);

template <core::Color MyColor>
FeatureBitboard processDeclarationRemaining(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    const int32_t TargetScore = (MyColor == core::Black) ? 28 : 27;
    const int32_t RemainingScore =
        TargetScore - (int32_t)Adapter->computeDeclarationScore<MyColor>();
    return internal::FeatureBitboardUtil::makeFeatureBitboard(
        core::internal::bitboard::Bitboard::AllBB(),
        (float)RemainingScore / (float)TargetScore, false);
}

template FeatureBitboard
processDeclarationRemaining<core::Black>(const core::State& State);
template FeatureBitboard
processDeclarationRemaining<core::White>(const core::State& State);

FeatureStackRuntime::FeatureStackRuntime(const std::vector<FeatureType>& Types,
                                         const core::State& State,
                                         const core::StateConfig& Config) {
    if (State.getPosition().sideToMove() == core::Black) {
        process<core::Black>(Types, State, Config);
    } else {
        process<core::White>(Types, State, Config);
    }
}

const FeatureBitboard& FeatureStackRuntime::get(std::size_t Index) const {
    return Features[Index];
}

template <core::Color C>
void FeatureStackRuntime::process(const std::vector<FeatureType>& Types,
                                  const core::State& State,
                                  const core::StateConfig& Config) {
    Features.resize(Types.size());

    for (size_t I = 0; I < Types.size(); ++I) {
        const auto& Type = Types[I];

        if (Type == FeatureType::FT_Black) {
            Features[I] = processIsBlackTurn<C>();
        } else if (Type == FeatureType::FT_White) {
            Features[I] = processIsWhiteTurn<C>();
        } else if (Type == FeatureType::FT_Check) {
            Features[I] = processCheck(State);
        } else if (Type == FeatureType::FT_NoMyPawnFile) {
            Features[I] = processNoPawnFile<C, C>(State);
        } else if (Type == FeatureType::FT_NoOpPawnFile) {
            Features[I] = processNoPawnFile<C, ~C>(State);
        } else if (Type == FeatureType::FT_Progress) {
            Features[I] = processProgress(State, Config);
        } else if (Type == FeatureType::FT_ProgressUnit) {
            Features[I] = processProgressUnit(Config);
        } else if (Type == FeatureType::FT_MyPawn) {
            Features[I] = processPiece<C, C>(State, core::PTK_Pawn);
        } else if (Type == FeatureType::FT_MyLance) {
            Features[I] = processPiece<C, C>(State, core::PTK_Lance);
        } else if (Type == FeatureType::FT_MyKnight) {
            Features[I] = processPiece<C, C>(State, core::PTK_Knight);
        } else if (Type == FeatureType::FT_MySilver) {
            Features[I] = processPiece<C, C>(State, core::PTK_Silver);
        } else if (Type == FeatureType::FT_MyGold) {
            Features[I] = processPiece<C, C>(State, core::PTK_Gold);
        } else if (Type == FeatureType::FT_MyKing) {
            Features[I] = processPiece<C, C>(State, core::PTK_King);
        } else if (Type == FeatureType::FT_MyBishop) {
            Features[I] = processPiece<C, C>(State, core::PTK_Bishop);
        } else if (Type == FeatureType::FT_MyRook) {
            Features[I] = processPiece<C, C>(State, core::PTK_Rook);
        } else if (Type == FeatureType::FT_MyProPawn) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProPawn);
        } else if (Type == FeatureType::FT_MyProLance) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProLance);
        } else if (Type == FeatureType::FT_MyProKnight) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProKnight);
        } else if (Type == FeatureType::FT_MyProSilver) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProSilver);
        } else if (Type == FeatureType::FT_MyProBishop) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProBishop);
        } else if (Type == FeatureType::FT_MyProRook) {
            Features[I] = processPiece<C, C>(State, core::PTK_ProRook);
        } else if (Type == FeatureType::FT_MyBishopAndProBishop) {
            Features[I] = processPiece<C, C>(State, core::PTK_Bishop,
                                             core::PTK_ProBishop);
        } else if (Type == FeatureType::FT_MyRookAndProRook) {
            Features[I] =
                processPiece<C, C>(State, core::PTK_Rook, core::PTK_ProRook);
        } else if (Type == FeatureType::FT_OpPawn) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Pawn);
        } else if (Type == FeatureType::FT_OpLance) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Lance);
        } else if (Type == FeatureType::FT_OpKnight) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Knight);
        } else if (Type == FeatureType::FT_OpSilver) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Silver);
        } else if (Type == FeatureType::FT_OpGold) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Gold);
        } else if (Type == FeatureType::FT_OpKing) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_King);
        } else if (Type == FeatureType::FT_OpBishop) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Bishop);
        } else if (Type == FeatureType::FT_OpRook) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Rook);
        } else if (Type == FeatureType::FT_OpProPawn) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProPawn);
        } else if (Type == FeatureType::FT_OpProLance) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProLance);
        } else if (Type == FeatureType::FT_OpProKnight) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProKnight);
        } else if (Type == FeatureType::FT_OpProSilver) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProSilver);
        } else if (Type == FeatureType::FT_OpProBishop) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProBishop);
        } else if (Type == FeatureType::FT_OpProRook) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_ProRook);
        } else if (Type == FeatureType::FT_OpBishopAndProBishop) {
            Features[I] = processPiece<C, ~C>(State, core::PTK_Bishop,
                                              core::PTK_ProBishop);
        } else if (Type == FeatureType::FT_OpRookAndProRook) {
            Features[I] =
                processPiece<C, ~C>(State, core::PTK_Rook, core::PTK_ProRook);
        } else if (Type == FeatureType::FT_MyStandPawn1) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 1);
        } else if (Type == FeatureType::FT_MyStandPawn2) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 2);
        } else if (Type == FeatureType::FT_MyStandPawn3) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 3);
        } else if (Type == FeatureType::FT_MyStandPawn4) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 4);
        } else if (Type == FeatureType::FT_MyStandPawn5) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 5);
        } else if (Type == FeatureType::FT_MyStandPawn6) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 6);
        } else if (Type == FeatureType::FT_MyStandPawn7) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 7);
        } else if (Type == FeatureType::FT_MyStandPawn8) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 8);
        } else if (Type == FeatureType::FT_MyStandPawn9) {
            Features[I] = processStand<C>(State, core::PTK_Pawn, 9);
        } else if (Type == FeatureType::FT_MyStandLance1) {
            Features[I] = processStand<C>(State, core::PTK_Lance, 1);
        } else if (Type == FeatureType::FT_MyStandLance2) {
            Features[I] = processStand<C>(State, core::PTK_Lance, 2);
        } else if (Type == FeatureType::FT_MyStandLance3) {
            Features[I] = processStand<C>(State, core::PTK_Lance, 3);
        } else if (Type == FeatureType::FT_MyStandLance4) {
            Features[I] = processStand<C>(State, core::PTK_Lance, 4);
        } else if (Type == FeatureType::FT_MyStandKnight1) {
            Features[I] = processStand<C>(State, core::PTK_Knight, 1);
        } else if (Type == FeatureType::FT_MyStandKnight2) {
            Features[I] = processStand<C>(State, core::PTK_Knight, 2);
        } else if (Type == FeatureType::FT_MyStandKnight3) {
            Features[I] = processStand<C>(State, core::PTK_Knight, 3);
        } else if (Type == FeatureType::FT_MyStandKnight4) {
            Features[I] = processStand<C>(State, core::PTK_Knight, 4);
        } else if (Type == FeatureType::FT_MyStandSilver1) {
            Features[I] = processStand<C>(State, core::PTK_Silver, 1);
        } else if (Type == FeatureType::FT_MyStandSilver2) {
            Features[I] = processStand<C>(State, core::PTK_Silver, 2);
        } else if (Type == FeatureType::FT_MyStandSilver3) {
            Features[I] = processStand<C>(State, core::PTK_Silver, 3);
        } else if (Type == FeatureType::FT_MyStandSilver4) {
            Features[I] = processStand<C>(State, core::PTK_Silver, 4);
        } else if (Type == FeatureType::FT_MyStandGold1) {
            Features[I] = processStand<C>(State, core::PTK_Gold, 1);
        } else if (Type == FeatureType::FT_MyStandGold2) {
            Features[I] = processStand<C>(State, core::PTK_Gold, 2);
        } else if (Type == FeatureType::FT_MyStandGold3) {
            Features[I] = processStand<C>(State, core::PTK_Gold, 3);
        } else if (Type == FeatureType::FT_MyStandGold4) {
            Features[I] = processStand<C>(State, core::PTK_Gold, 4);
        } else if (Type == FeatureType::FT_MyStandBishop1) {
            Features[I] = processStand<C>(State, core::PTK_Bishop, 1);
        } else if (Type == FeatureType::FT_MyStandBishop2) {
            Features[I] = processStand<C>(State, core::PTK_Bishop, 2);
        } else if (Type == FeatureType::FT_MyStandRook1) {
            Features[I] = processStand<C>(State, core::PTK_Rook, 1);
        } else if (Type == FeatureType::FT_MyStandRook2) {
            Features[I] = processStand<C>(State, core::PTK_Rook, 2);
        } else if (Type == FeatureType::FT_OpStandPawn1) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 1);
        } else if (Type == FeatureType::FT_OpStandPawn2) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 2);
        } else if (Type == FeatureType::FT_OpStandPawn3) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 3);
        } else if (Type == FeatureType::FT_OpStandPawn4) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 4);
        } else if (Type == FeatureType::FT_OpStandPawn5) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 5);
        } else if (Type == FeatureType::FT_OpStandPawn6) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 6);
        } else if (Type == FeatureType::FT_OpStandPawn7) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 7);
        } else if (Type == FeatureType::FT_OpStandPawn8) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 8);
        } else if (Type == FeatureType::FT_OpStandPawn9) {
            Features[I] = processStand<~C>(State, core::PTK_Pawn, 9);
        } else if (Type == FeatureType::FT_OpStandLance1) {
            Features[I] = processStand<~C>(State, core::PTK_Lance, 1);
        } else if (Type == FeatureType::FT_OpStandLance2) {
            Features[I] = processStand<~C>(State, core::PTK_Lance, 2);
        } else if (Type == FeatureType::FT_OpStandLance3) {
            Features[I] = processStand<~C>(State, core::PTK_Lance, 3);
        } else if (Type == FeatureType::FT_OpStandLance4) {
            Features[I] = processStand<~C>(State, core::PTK_Lance, 4);
        } else if (Type == FeatureType::FT_OpStandKnight1) {
            Features[I] = processStand<~C>(State, core::PTK_Knight, 1);
        } else if (Type == FeatureType::FT_OpStandKnight2) {
            Features[I] = processStand<~C>(State, core::PTK_Knight, 2);
        } else if (Type == FeatureType::FT_OpStandKnight3) {
            Features[I] = processStand<~C>(State, core::PTK_Knight, 3);
        } else if (Type == FeatureType::FT_OpStandKnight4) {
            Features[I] = processStand<~C>(State, core::PTK_Knight, 4);
        } else if (Type == FeatureType::FT_OpStandSilver1) {
            Features[I] = processStand<~C>(State, core::PTK_Silver, 1);
        } else if (Type == FeatureType::FT_OpStandSilver2) {
            Features[I] = processStand<~C>(State, core::PTK_Silver, 2);
        } else if (Type == FeatureType::FT_OpStandSilver3) {
            Features[I] = processStand<~C>(State, core::PTK_Silver, 3);
        } else if (Type == FeatureType::FT_OpStandSilver4) {
            Features[I] = processStand<~C>(State, core::PTK_Silver, 4);
        } else if (Type == FeatureType::FT_OpStandGold1) {
            Features[I] = processStand<~C>(State, core::PTK_Gold, 1);
        } else if (Type == FeatureType::FT_OpStandGold2) {
            Features[I] = processStand<~C>(State, core::PTK_Gold, 2);
        } else if (Type == FeatureType::FT_OpStandGold3) {
            Features[I] = processStand<~C>(State, core::PTK_Gold, 3);
        } else if (Type == FeatureType::FT_OpStandGold4) {
            Features[I] = processStand<~C>(State, core::PTK_Gold, 4);
        } else if (Type == FeatureType::FT_OpStandBishop1) {
            Features[I] = processStand<~C>(State, core::PTK_Bishop, 1);
        } else if (Type == FeatureType::FT_OpStandBishop2) {
            Features[I] = processStand<~C>(State, core::PTK_Bishop, 2);
        } else if (Type == FeatureType::FT_OpStandRook1) {
            Features[I] = processStand<~C>(State, core::PTK_Rook, 1);
        } else if (Type == FeatureType::FT_OpStandRook2) {
            Features[I] = processStand<~C>(State, core::PTK_Rook, 2);
        } else if (Type == FeatureType::FT_RuleDeclare27) {
            Features[I] = processRule<core::EndingRule::ER_Declare27>(Config);
        } else if (Type == FeatureType::FT_RuleDraw24) {
            Features[I] = processRule<core::EndingRule::ER_Draw24>(Config);
        } else if (Type == FeatureType::FT_RuleTrying) {
            Features[I] = processRule<core::EndingRule::ER_Trying>(Config);
        } else if (Type == FeatureType::FT_MyDrawValue) {
            Features[I] = processDrawValue<C>(Config);
        } else if (Type == FeatureType::FT_OpDrawValue) {
            Features[I] = processDrawValue<~C>(Config);
        } else if (Type == FeatureType::FT_MyDeclarationScore) {
            Features[I] = processDeclarationScore<C>(State);
        } else if (Type == FeatureType::FT_OpDeclarationScore) {
            Features[I] = processDeclarationScore<~C>(State);
        } else if (Type == FeatureType::FT_MyPieceScore) {
            Features[I] = processPieceScore<C>(State);
        } else if (Type == FeatureType::FT_OpPieceScore) {
            Features[I] = processPieceScore<~C>(State);
        } else if (Type == FeatureType::FT_MyAttack) {
            Features[I] = processAttack<C, C>(State);
        } else if (Type == FeatureType::FT_OpAttack) {
            Features[I] = processAttack<C, ~C>(State);
        } else if (Type == FeatureType::FT_MyDeclarationRemaining) {
            Features[I] = processDeclarationRemaining<C>(State);
        } else if (Type == FeatureType::FT_OpDeclarationRemaining) {
            Features[I] = processDeclarationRemaining<~C>(State);
        } else {
            assert(false);
        }
    }
}

template <core::IterateOrder Order, bool ChannelsFirst>
    requires(!ChannelsFirst)
void FeatureStackRuntime::extract(float* Dest) const {
    core::SquareIterator<Order> SquareIt;
    for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
        for (std::size_t Channel = 0; Channel < Features.size(); ++Channel) {
            const core::internal::bitboard::Bitboard* BB =
                reinterpret_cast<const core::internal::bitboard::Bitboard*>(
                    Features[Channel].data());

            const bool IsRotated = Features[Channel].isRotated();

            if (!IsRotated) {
                Dest[(std::size_t)*It * Features.size() + Channel] =
                    BB->isSet(*It) ? Features[Channel].getValue() : 0.0f;
            } else {
                Dest[(std::size_t)(core::NumSquares - 1 - *It) *
                         Features.size() +
                     Channel] =
                    BB->isSet(*It) ? Features[Channel].getValue() : 0.0f;
            }
        }
    }
}

template void FeatureStackRuntime::extract<core::IterateOrder::ESWN, false>(
    float* Dest) const;
template void FeatureStackRuntime::extract<core::IterateOrder::NWSE, false>(
    float* Dest) const;
template void FeatureStackRuntime::extract<core::IterateOrder::SENW, false>(
    float* Dest) const;
template void FeatureStackRuntime::extract<core::IterateOrder::WNES, false>(
    float* Dest) const;

} // namespace ml
} // namespace nshogi
