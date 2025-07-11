//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../c_api.h"
#include "../core/types.h"
#include "../io/sfen.h"
#include "../ml/common.h"
#include "../ml/featuretype.h"
#include "common.h"

#include <cstring>
#include <fstream>
#include <random>

TEST(CAPI, FeatureType) {
    using namespace nshogi::ml;

    TEST_ASSERT_EQ(NSHOGI_FT_BLACK, (int)FeatureType::FT_Black);
    TEST_ASSERT_EQ(NSHOGI_FT_WHITE, (int)FeatureType::FT_White);

    TEST_ASSERT_EQ(NSHOGI_FT_MYPAWN, (int)FeatureType::FT_MyPawn);
    TEST_ASSERT_EQ(NSHOGI_FT_MYLANCE, (int)FeatureType::FT_MyLance);
    TEST_ASSERT_EQ(NSHOGI_FT_MYKNIGHT, (int)FeatureType::FT_MyKnight);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSILVER, (int)FeatureType::FT_MySilver);
    TEST_ASSERT_EQ(NSHOGI_FT_MYGOLD, (int)FeatureType::FT_MyGold);
    TEST_ASSERT_EQ(NSHOGI_FT_MYKING, (int)FeatureType::FT_MyKing);
    TEST_ASSERT_EQ(NSHOGI_FT_MYBISHOP, (int)FeatureType::FT_MyBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_MYROOK, (int)FeatureType::FT_MyRook);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROPAWN, (int)FeatureType::FT_MyProPawn);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROLANCE, (int)FeatureType::FT_MyProLance);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROKNIGHT, (int)FeatureType::FT_MyProKnight);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROSILVER, (int)FeatureType::FT_MyProSilver);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROBISHOP, (int)FeatureType::FT_MyProBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPROROOK, (int)FeatureType::FT_MyProRook);
    TEST_ASSERT_EQ(NSHOGI_FT_MYBISHOP_AND_PROBISHOP,
                   (int)FeatureType::FT_MyBishopAndProBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_MYROOK_AND_PROROOK,
                   (int)FeatureType::FT_MyRookAndProRook);

    TEST_ASSERT_EQ(NSHOGI_FT_OPPAWN, (int)FeatureType::FT_OpPawn);
    TEST_ASSERT_EQ(NSHOGI_FT_OPLANCE, (int)FeatureType::FT_OpLance);
    TEST_ASSERT_EQ(NSHOGI_FT_OPKNIGHT, (int)FeatureType::FT_OpKnight);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSILVER, (int)FeatureType::FT_OpSilver);
    TEST_ASSERT_EQ(NSHOGI_FT_OPGOLD, (int)FeatureType::FT_OpGold);
    TEST_ASSERT_EQ(NSHOGI_FT_OPKING, (int)FeatureType::FT_OpKing);
    TEST_ASSERT_EQ(NSHOGI_FT_OPBISHOP, (int)FeatureType::FT_OpBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_OPROOK, (int)FeatureType::FT_OpRook);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROPAWN, (int)FeatureType::FT_OpProPawn);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROLANCE, (int)FeatureType::FT_OpProLance);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROKNIGHT, (int)FeatureType::FT_OpProKnight);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROSILVER, (int)FeatureType::FT_OpProSilver);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROBISHOP, (int)FeatureType::FT_OpProBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPROROOK, (int)FeatureType::FT_OpProRook);
    TEST_ASSERT_EQ(NSHOGI_FT_OPBISHOP_AND_PROBISHOP,
                   (int)FeatureType::FT_OpBishopAndProBishop);
    TEST_ASSERT_EQ(NSHOGI_FT_OPROOK_AND_PROROOK,
                   (int)FeatureType::FT_OpRookAndProRook);

    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN1, (int)FeatureType::FT_MyStandPawn1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN2, (int)FeatureType::FT_MyStandPawn2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN3, (int)FeatureType::FT_MyStandPawn3);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN4, (int)FeatureType::FT_MyStandPawn4);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN5, (int)FeatureType::FT_MyStandPawn5);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN6, (int)FeatureType::FT_MyStandPawn6);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN7, (int)FeatureType::FT_MyStandPawn7);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN8, (int)FeatureType::FT_MyStandPawn8);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDPAWN9, (int)FeatureType::FT_MyStandPawn9);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDLANCE1, (int)FeatureType::FT_MyStandLance1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDLANCE2, (int)FeatureType::FT_MyStandLance2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDLANCE3, (int)FeatureType::FT_MyStandLance3);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDLANCE4, (int)FeatureType::FT_MyStandLance4);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDKNIGHT1,
                   (int)FeatureType::FT_MyStandKnight1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDKNIGHT2,
                   (int)FeatureType::FT_MyStandKnight2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDKNIGHT3,
                   (int)FeatureType::FT_MyStandKnight3);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDKNIGHT4,
                   (int)FeatureType::FT_MyStandKnight4);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDSILVER1,
                   (int)FeatureType::FT_MyStandSilver1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDSILVER2,
                   (int)FeatureType::FT_MyStandSilver2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDSILVER3,
                   (int)FeatureType::FT_MyStandSilver3);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDSILVER4,
                   (int)FeatureType::FT_MyStandSilver4);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDGOLD1, (int)FeatureType::FT_MyStandGold1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDGOLD2, (int)FeatureType::FT_MyStandGold2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDGOLD3, (int)FeatureType::FT_MyStandGold3);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDGOLD4, (int)FeatureType::FT_MyStandGold4);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDBISHOP1,
                   (int)FeatureType::FT_MyStandBishop1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDBISHOP2,
                   (int)FeatureType::FT_MyStandBishop2);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDROOK1, (int)FeatureType::FT_MyStandRook1);
    TEST_ASSERT_EQ(NSHOGI_FT_MYSTANDROOK2, (int)FeatureType::FT_MyStandRook2);

    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN1, (int)FeatureType::FT_OpStandPawn1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN2, (int)FeatureType::FT_OpStandPawn2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN3, (int)FeatureType::FT_OpStandPawn3);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN4, (int)FeatureType::FT_OpStandPawn4);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN5, (int)FeatureType::FT_OpStandPawn5);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN6, (int)FeatureType::FT_OpStandPawn6);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN7, (int)FeatureType::FT_OpStandPawn7);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN8, (int)FeatureType::FT_OpStandPawn8);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDPAWN9, (int)FeatureType::FT_OpStandPawn9);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDLANCE1, (int)FeatureType::FT_OpStandLance1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDLANCE2, (int)FeatureType::FT_OpStandLance2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDLANCE3, (int)FeatureType::FT_OpStandLance3);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDLANCE4, (int)FeatureType::FT_OpStandLance4);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDKNIGHT1,
                   (int)FeatureType::FT_OpStandKnight1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDKNIGHT2,
                   (int)FeatureType::FT_OpStandKnight2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDKNIGHT3,
                   (int)FeatureType::FT_OpStandKnight3);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDKNIGHT4,
                   (int)FeatureType::FT_OpStandKnight4);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDSILVER1,
                   (int)FeatureType::FT_OpStandSilver1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDSILVER2,
                   (int)FeatureType::FT_OpStandSilver2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDSILVER3,
                   (int)FeatureType::FT_OpStandSilver3);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDSILVER4,
                   (int)FeatureType::FT_OpStandSilver4);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDGOLD1, (int)FeatureType::FT_OpStandGold1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDGOLD2, (int)FeatureType::FT_OpStandGold2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDGOLD3, (int)FeatureType::FT_OpStandGold3);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDGOLD4, (int)FeatureType::FT_OpStandGold4);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDBISHOP1,
                   (int)FeatureType::FT_OpStandBishop1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDBISHOP2,
                   (int)FeatureType::FT_OpStandBishop2);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDROOK1, (int)FeatureType::FT_OpStandRook1);
    TEST_ASSERT_EQ(NSHOGI_FT_OPSTANDROOK2, (int)FeatureType::FT_OpStandRook2);

    TEST_ASSERT_EQ(NSHOGI_FT_CHECK, (int)FeatureType::FT_Check);
    TEST_ASSERT_EQ(NSHOGI_FT_NOMYPAWNFILE, (int)FeatureType::FT_NoMyPawnFile);
    TEST_ASSERT_EQ(NSHOGI_FT_NOOPPAWNFILE, (int)FeatureType::FT_NoOpPawnFile);
    TEST_ASSERT_EQ(NSHOGI_FT_PROGRESS, (int)FeatureType::FT_Progress);
    TEST_ASSERT_EQ(NSHOGI_FT_PROGRESSUNIT, (int)FeatureType::FT_ProgressUnit);

    TEST_ASSERT_EQ(NSHOGI_FT_RULEDECLARE27, (int)FeatureType::FT_RuleDeclare27);
    TEST_ASSERT_EQ(NSHOGI_FT_RULEDRAW24, (int)FeatureType::FT_RuleDraw24);
    TEST_ASSERT_EQ(NSHOGI_FT_RULETRYING, (int)FeatureType::FT_RuleTrying);
    TEST_ASSERT_EQ(NSHOGI_FT_MYDRAWVALUE, (int)FeatureType::FT_MyDrawValue);
    TEST_ASSERT_EQ(NSHOGI_FT_OPDRAWVALUE, (int)FeatureType::FT_OpDrawValue);

    TEST_ASSERT_EQ(NSHOGI_FT_MYDECLARATIONSCORE,
                   (int)FeatureType::FT_MyDeclarationScore);
    TEST_ASSERT_EQ(NSHOGI_FT_OPDECLARATIONSCORE,
                   (int)FeatureType::FT_OpDeclarationScore);
    TEST_ASSERT_EQ(NSHOGI_FT_MYPIECESCORE, (int)FeatureType::FT_MyPieceScore);
    TEST_ASSERT_EQ(NSHOGI_FT_OPPIECESCORE, (int)FeatureType::FT_OpPieceScore);

    TEST_ASSERT_EQ(NSHOGI_FT_MYATTACK, (int)FeatureType::FT_MyAttack);
    TEST_ASSERT_EQ(NSHOGI_FT_OPATTACK, (int)FeatureType::FT_OpAttack);
}

TEST(CAPI, InitialPosition) {
    const char* Sfen = "startpos";
    nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen(Sfen);
    const nshogi_position_t* Position =
        nshogiApi()->stateApi()->getPosition(State);

    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getSideToMove(Position),
                   NSHOGI_BLACK);

    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1A),
        NSHOGI_PK_WHITELANCE);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1I),
        NSHOGI_PK_BLACKLANCE);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2A),
        NSHOGI_PK_WHITEKNIGHT);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2B),
        NSHOGI_PK_WHITEBISHOP);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2H),
        NSHOGI_PK_BLACKROOK);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2I),
        NSHOGI_PK_BLACKKNIGHT);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3A),
        NSHOGI_PK_WHITESILVER);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3I),
        NSHOGI_PK_BLACKSILVER);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4A),
        NSHOGI_PK_WHITEGOLD);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4I),
        NSHOGI_PK_BLACKGOLD);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5A),
        NSHOGI_PK_WHITEKING);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5I),
        NSHOGI_PK_BLACKKING);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6A),
        NSHOGI_PK_WHITEGOLD);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6I),
        NSHOGI_PK_BLACKGOLD);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7A),
        NSHOGI_PK_WHITESILVER);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7I),
        NSHOGI_PK_BLACKSILVER);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8A),
        NSHOGI_PK_WHITEKNIGHT);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8B),
        NSHOGI_PK_WHITEROOK);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8H),
        NSHOGI_PK_BLACKBISHOP);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8I),
        NSHOGI_PK_BLACKKNIGHT);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9A),
        NSHOGI_PK_WHITELANCE);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9B),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9C),
        NSHOGI_PK_WHITEPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9D),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9E),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9F),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9G),
        NSHOGI_PK_BLACKPAWN);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9H),
        NSHOGI_PK_EMPTY);
    TEST_ASSERT_EQ(
        nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9I),
        NSHOGI_PK_BLACKLANCE);

    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_PAWN),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_LANCE),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_KNIGHT),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_SILVER),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_GOLD),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_BISHOP),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_BLACK, NSHOGI_PTK_ROOK),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_PAWN),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_LANCE),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_KNIGHT),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_SILVER),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_GOLD),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_BISHOP),
                   0);
    TEST_ASSERT_EQ(nshogiApi()->positionApi()->getStandCount(
                       Position, NSHOGI_WHITE, NSHOGI_PTK_ROOK),
                   0);

    nshogiApi()->stateApi()->destroyState(State);
}

TEST(CAPI, InitialState) {
    const char* Sfen = "startpos";
    nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen(Sfen);

    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getSideToMove(State), NSHOGI_BLACK);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getPly(State), 0);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getRepetition(State),
                   NSHOGI_NO_REPETITION);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->canDeclare(State), 0);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->isInCheck(State), 0);

    nshogi_move_t Moves[600];
    int MoveCount = nshogiApi()->stateApi()->generateMoves(State, 1, Moves);
    TEST_ASSERT_EQ(MoveCount, 30);

    int MoveCountNoWilyPromote =
        nshogiApi()->stateApi()->generateMoves(State, 0, Moves);
    TEST_ASSERT_EQ(MoveCountNoWilyPromote, 30);

    TEST_ASSERT_EQ(
        nshogiApi()->stateApi()->getHash(State),
        nshogi::io::sfen::StateBuilder::newState("startpos").getHash());
}

TEST(CAPI, DoAndUndo) {
    const int N = 10;

    std::mt19937_64 Mt(20250413);
    nshogi_move_t Moves[600];
    nshogi_move_t MoveHistory[1024];
    constexpr int32_t MaxLength = 16384;
    char InitialPositionSfen[MaxLength];
    char StateSfen[MaxLength];
    for (int I = 0; I < N; ++I) {
        nshogi_state_t* State =
            nshogiApi()->ioApi()->createStateFromSfen("startpos");
        TEST_ASSERT_TRUE(nshogiApi()->ioApi()->positionToSfen(
                             InitialPositionSfen, MaxLength,
                             nshogiApi()->stateApi()->getPosition(State)) > 0);

        for (int Ply = 0; Ply < 1024; ++Ply) {
            int MoveCount =
                nshogiApi()->stateApi()->generateMoves(State, 1, Moves);

            for (int J = 0; J < Ply; ++J) {
                TEST_ASSERT_EQ(
                    nshogiApi()->stateApi()->getHistoryMove(State, (uint16_t)J),
                    MoveHistory[J]);
            }

            if (MoveCount == 0) {
                break;
            }

            TEST_ASSERT_TRUE(nshogiApi()->ioApi()->stateToSfen(
                                 StateSfen, MaxLength, State) > 0);

            for (int J = 0; J < MoveCount; ++J) {
                nshogi_move_t M = Moves[(std::size_t)J];

                char SfenMove[16];
                TEST_ASSERT_TRUE(nshogiApi()->ioApi()->moveToSfen(SfenMove, M) >
                                 0);
                nshogi_move_t M_ =
                    nshogiApi()->ioApi()->createMoveFromSfen(State, SfenMove);
                TEST_ASSERT_EQ(M, M_);

                if (std::strncmp(SfenMove, "P*", 2) == 0) {
                    TEST_ASSERT_TRUE(
                        nshogiApi()->moveApi()->drop(M) &&
                        (nshogiApi()->moveApi()->pieceType(M) == NSHOGI_PTK_PAWN)
                    );
                } else {
                    TEST_ASSERT_FALSE(
                        nshogiApi()->moveApi()->drop(M) &&
                        (nshogiApi()->moveApi()->pieceType(M) == NSHOGI_PTK_PAWN)
                    );
                }

                nshogiApi()->stateApi()->doMove(State, M);

                TEST_ASSERT_EQ(nshogiApi()->stateApi()->getLastMove(State), M);

                nshogiApi()->stateApi()->undoMove(State);

                char DoAndUndoSfen[MaxLength];
                TEST_ASSERT_TRUE(nshogiApi()->ioApi()->stateToSfen(
                                     DoAndUndoSfen, MaxLength, State) > 0);
                TEST_ASSERT_EQ(std::strcmp(StateSfen, DoAndUndoSfen), 0);

                char InitialPositionSfenCurrent[MaxLength];
                TEST_ASSERT_TRUE(
                    nshogiApi()->ioApi()->positionToSfen(
                        InitialPositionSfenCurrent, MaxLength,
                        nshogiApi()->stateApi()->getInitialPosition(State)) >
                    0);

                TEST_ASSERT_EQ(std::strcmp(InitialPositionSfen,
                                           InitialPositionSfenCurrent),
                               0);

                TEST_ASSERT_EQ(
                    (std::size_t)nshogiApi()->mlApi()->moveToIndex(State, M),
                    nshogi::ml::getMoveIndex(
                        static_cast<nshogi::core::Color>(
                            nshogiApi()->stateApi()->getSideToMove(State)),
                        nshogi::core::Move32::fromValue(M)));
                TEST_ASSERT_EQ(
                    (std::size_t)nshogiApi()->mlApi()->moveToIndex(State, M),
                    nshogi::ml::getMoveIndex(
                        static_cast<nshogi::core::Color>(
                            nshogiApi()->stateApi()->getSideToMove(State)),
                        nshogi::core::Move16(
                            nshogi::core::Move32::fromValue(M))));
            }

            nshogi_move_t RandomMove = Moves[(std::size_t)(
                (std::size_t)Mt() % (std::size_t)MoveCount)];
            nshogiApi()->stateApi()->doMove(State, RandomMove);

            MoveHistory[(std::size_t)Ply] = RandomMove;
        }

        nshogiApi()->stateApi()->destroyState(State);
    }
}

TEST(CAPI, Clone) {
    const int N = 100;

    std::mt19937_64 Mt(20250413);
    nshogi_move_t Moves[600];
    constexpr int32_t MaxLength = 16384;
    for (int I = 0; I < N; ++I) {
        nshogi_state_t* State =
            nshogiApi()->ioApi()->createStateFromSfen("startpos");

        for (int Ply = 0; Ply < 1024; ++Ply) {
            int MoveCount =
                nshogiApi()->stateApi()->generateMoves(State, 1, Moves);

            if (MoveCount == 0) {
                break;
            }

            nshogi_move_t RandomMove = Moves[(std::size_t)(
                (std::size_t)Mt() % (std::size_t)MoveCount)];
            nshogiApi()->stateApi()->doMove(State, RandomMove);

            nshogi_state_t* ClonedState = nshogiApi()->stateApi()->clone(State);

            char Sfen[MaxLength];
            TEST_ASSERT_TRUE(
                nshogiApi()->ioApi()->stateToSfen(Sfen, MaxLength, State) > 0);
            char ClonedSfen[MaxLength];
            TEST_ASSERT_TRUE(nshogiApi()->ioApi()->stateToSfen(
                                 ClonedSfen, MaxLength, ClonedState) > 0);

            TEST_ASSERT_EQ(std::strcmp(Sfen, ClonedSfen), 0);

            nshogiApi()->stateApi()->destroyState(ClonedState);
        }

        nshogiApi()->stateApi()->destroyState(State);
    }
}

TEST(CAPI, StateConfig) {
    nshogi_state_config_t* StateConfig =
        nshogiApi()->stateApi()->createStateConfig();

    for (uint16_t MaxPly = 1; MaxPly < 65535; ++MaxPly) {
        for (float BlackDrawValue = 0.0f; BlackDrawValue <= 1.0f;
             BlackDrawValue += 0.1f) {
            for (float WhiteDrawValue = 0.0f; WhiteDrawValue <= 1.0f;
                 WhiteDrawValue += 0.1f) {
                nshogiApi()->stateApi()->setMaxPly(StateConfig, MaxPly);
                nshogiApi()->stateApi()->setBlackDrawValue(StateConfig,
                                                           BlackDrawValue);
                nshogiApi()->stateApi()->setWhiteDrawValue(StateConfig,
                                                           WhiteDrawValue);

                TEST_ASSERT_EQ(MaxPly,
                               nshogiApi()->stateApi()->getMaxPly(StateConfig));
                TEST_ASSERT_FLOAT_EQ(
                    BlackDrawValue,
                    nshogiApi()->stateApi()->getBlackDrawValue(StateConfig),
                    1e-6);
                TEST_ASSERT_FLOAT_EQ(
                    WhiteDrawValue,
                    nshogiApi()->stateApi()->getWhiteDrawValue(StateConfig),
                    1e-6);
            }
        }
    }

    nshogiApi()->stateApi()->destroyStateConfig(StateConfig);
}

TEST(CAPI, InvalidState) {
    const char* InvalidSfen = "invalid example";
    TEST_ASSERT_TRUE(
        nshogiApi()->ioApi()->canCreateStateFromSfen(InvalidSfen) == 0);
}

TEST(CAPI, ValidState) {
    const char* ValidSfen = "startpos";
    TEST_ASSERT_TRUE(nshogiApi()->ioApi()->canCreateStateFromSfen(ValidSfen) >
                     0);
}

TEST(CAPI, InsufficientMemorySize) {
    char Sfen[16384];

    nshogi_state_t* State =
        nshogiApi()->ioApi()->createStateFromSfen("startpos");

    // MaxLength is not sufficient.
    TEST_ASSERT_TRUE(nshogiApi()->ioApi()->stateToSfen(
                         Sfen, /* MaxLength = */ 63, State) == 0);

    // MaxLength is sufficient.
    TEST_ASSERT_TRUE(nshogiApi()->ioApi()->stateToSfen(
                         Sfen, /* MaxLength = */ 64, State) > 0);

    nshogiApi()->stateApi()->destroyState(State);
}

TEST(CAPI, WinDeclarationMove) {
    nshogi_move_t WinMove = nshogiApi()->moveApi()->winMove();

    TEST_ASSERT_EQ(static_cast<uint32_t>(WinMove),
                   nshogi::core::Move32::MoveWin().value());
}

TEST(CAPI, DFS) {
    std::ifstream Ifs("./res/test/mate-5-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        nshogi_state_t* State =
            nshogiApi()->ioApi()->createStateFromSfen(Line.c_str());

        nshogi_move_t CheckmateMove = nshogiApi()->solverApi()->dfs(State, 5);

        TEST_ASSERT_FALSE(nshogiApi()->moveApi()->isNone(CheckmateMove));
    }
}

TEST(CAPI, DfPn) {
    std::ifstream Ifs("./res/test/mate-11-ply.txt");

    nshogi_solver_dfpn_t* Solver =
        nshogiApi()->solverApi()->createDfPnSolver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        nshogi_state_t* State =
            nshogiApi()->ioApi()->createStateFromSfen(Line.c_str());

        nshogi_move_t CheckmateMove =
            nshogiApi()->solverApi()->solveByDfPn(State, Solver, 0, 0);

        TEST_ASSERT_FALSE(nshogiApi()->moveApi()->isNone(CheckmateMove));
    }
    nshogiApi()->solverApi()->destroyDfPnSolver(Solver);
}

TEST(CAPI, MLFeatureVector) {
    float* Dest = static_cast<float*>(malloc(4 * 9 * 9 * sizeof(float)));

    nshogi_feature_type_t Features[] = {
        NSHOGI_FT_BLACK,
        NSHOGI_FT_WHITE,
        NSHOGI_FT_MYPAWN,
        NSHOGI_FT_OPPAWN,
    };

    int NumFeatures = sizeof(Features) / sizeof(Features[0]);

    nshogi_state_t* State =
        nshogiApi()->ioApi()->createStateFromSfen("startpos");
    nshogi_state_config_t* StateConfig =
        nshogiApi()->stateApi()->createStateConfig();

    nshogiApi()->mlApi()->makeFeatureVector(Dest, State, StateConfig, Features,
                                            NumFeatures);

    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)Square], 1.0f, 1e-6);
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(1 * 81 + Square)], 0.0f, 1e-6);
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        nshogi_piece_t Piece = nshogiApi()->positionApi()->pieceOn(
            nshogiApi()->stateApi()->getPosition(State), Square);
        if (Piece == NSHOGI_PK_BLACKPAWN) {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(2 * 81 + Square)], 1.0f,
                                 1e-6);
        } else {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(2 * 81 + Square)], 0.0f,
                                 1e-6);
        }
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        nshogi_piece_t Piece = nshogiApi()->positionApi()->pieceOn(
            nshogiApi()->stateApi()->getPosition(State), Square);
        if (Piece == NSHOGI_PK_WHITEPAWN) {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(3 * 81 + Square)], 1.0f,
                                 1e-6);
        } else {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(3 * 81 + Square)], 0.0f,
                                 1e-6);
        }
    }

    nshogiApi()->stateApi()->destroyStateConfig(StateConfig);
    nshogiApi()->stateApi()->destroyState(State);

    free(Dest);
}
