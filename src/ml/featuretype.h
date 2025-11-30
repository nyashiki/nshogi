//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_FEATURETYPE_H
#define NSHOGI_ML_FEATURETYPE_H

namespace nshogi {
namespace ml {

// clang-format off
enum class FeatureType {
    FT_Black, FT_White,

    FT_MyPawn, FT_MyLance, FT_MyKnight, FT_MySilver, FT_MyGold, FT_MyKing, FT_MyBishop, FT_MyRook,
    FT_MyProPawn, FT_MyProLance, FT_MyProKnight, FT_MyProSilver, FT_MyProBishop, FT_MyProRook,
    FT_MyBishopAndProBishop, FT_MyRookAndProRook,

    FT_OpPawn, FT_OpLance, FT_OpKnight, FT_OpSilver, FT_OpGold, FT_OpKing, FT_OpBishop, FT_OpRook,
    FT_OpProPawn, FT_OpProLance, FT_OpProKnight, FT_OpProSilver, FT_OpProBishop, FT_OpProRook,
    FT_OpBishopAndProBishop, FT_OpRookAndProRook,

    FT_MyStandPawn1,   FT_MyStandPawn2,   FT_MyStandPawn3,   FT_MyStandPawn4,
    FT_MyStandPawn5,   FT_MyStandPawn6,   FT_MyStandPawn7,   FT_MyStandPawn8, FT_MyStandPawn9,
    FT_MyStandLance1,  FT_MyStandLance2,  FT_MyStandLance3,  FT_MyStandLance4,
    FT_MyStandKnight1, FT_MyStandKnight2, FT_MyStandKnight3, FT_MyStandKnight4,
    FT_MyStandSilver1, FT_MyStandSilver2, FT_MyStandSilver3, FT_MyStandSilver4,
    FT_MyStandGold1,   FT_MyStandGold2,   FT_MyStandGold3,   FT_MyStandGold4,
    FT_MyStandBishop1, FT_MyStandBishop2,
    FT_MyStandRook1,   FT_MyStandRook2,

    FT_OpStandPawn1,   FT_OpStandPawn2,   FT_OpStandPawn3,   FT_OpStandPawn4,
    FT_OpStandPawn5,   FT_OpStandPawn6,   FT_OpStandPawn7,   FT_OpStandPawn8, FT_OpStandPawn9,
    FT_OpStandLance1,  FT_OpStandLance2,  FT_OpStandLance3,  FT_OpStandLance4,
    FT_OpStandKnight1, FT_OpStandKnight2, FT_OpStandKnight3, FT_OpStandKnight4,
    FT_OpStandSilver1, FT_OpStandSilver2, FT_OpStandSilver3, FT_OpStandSilver4,
    FT_OpStandGold1,   FT_OpStandGold2,   FT_OpStandGold3,   FT_OpStandGold4,
    FT_OpStandBishop1, FT_OpStandBishop2,
    FT_OpStandRook1,   FT_OpStandRook2,

    FT_Check, FT_NoMyPawnFile, FT_NoOpPawnFile, FT_Progress, FT_ProgressUnit,

    FT_RuleDeclare27, FT_RuleDraw24, FT_RuleTrying,
    FT_MyDrawValue, FT_OpDrawValue,

    FT_MyDeclarationScore, FT_OpDeclarationScore,
    FT_MyPieceScore, FT_OpPieceScore,

    FT_MyAttack, FT_OpAttack,

    FT_MyDeclarationRemaining, FT_OpDeclarationRemaining,
};
// clang-format on

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_FEATURETYPE_H
