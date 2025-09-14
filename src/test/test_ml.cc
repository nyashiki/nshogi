//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include <cstddef>

#include "../core/internal/stateadapter.h"
#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/statebuilder.h"
#include "../core/stateconfig.h"
#include "../io/file.h"
#include "../io/sfen.h"
#include "../ml/azteacher.h"
#include "../ml/common.h"
#include "../ml/featurestack.h"
#include "../ml/internal/featurebitboardutil.h"
#include "../ml/simpleteacher.h"
#include "../ml/utils.h"
#include "common.h"

#include <filesystem>
#include <fstream>
#include <random>

namespace {

using FeatureBitboardUtil = nshogi::ml::internal::FeatureBitboardUtil;

void checkFeatureTypeColor(const nshogi::core::State& State,
                           const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<nshogi::ml::FeatureType::FT_Black,
                                           nshogi::ml::FeatureType::FT_White>
        Features(State, Config);

    if (State.getPosition().sideToMove() == nshogi::core::Black) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
    } else {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
        TEST_ASSERT_EQ(Features.get(1).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
    }

    for (std::size_t I = 0; I < Features.size(); ++I) {
        const auto Copy = nshogi::ml::FeatureBitboard(Features.get(I));
        TEST_ASSERT_TRUE(Features.get(I) == Copy);
    }
}

void checkFeatureTypeColorRuntime(const nshogi::core::State& State,
                                  const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {
            nshogi::ml::FeatureType::FT_Black,
            nshogi::ml::FeatureType::FT_White,
        },
        State, Config);

    if (State.getPosition().sideToMove() == nshogi::core::Black) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
    } else {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
        TEST_ASSERT_EQ(Features.get(1).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
    }
}

void checkFeatureTypeOnBoard(const nshogi::core::State& State,
                             const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_MyPawn, nshogi::ml::FeatureType::FT_MyLance,
        nshogi::ml::FeatureType::FT_MyKnight,
        nshogi::ml::FeatureType::FT_MySilver,
        nshogi::ml::FeatureType::FT_MyBishop,
        nshogi::ml::FeatureType::FT_MyRook, nshogi::ml::FeatureType::FT_MyGold,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_MyProPawn,
        nshogi::ml::FeatureType::FT_MyProLance,
        nshogi::ml::FeatureType::FT_MyProKnight,
        nshogi::ml::FeatureType::FT_MyProSilver,
        nshogi::ml::FeatureType::FT_MyProBishop,
        nshogi::ml::FeatureType::FT_MyProRook,
        nshogi::ml::FeatureType::FT_OpPawn, nshogi::ml::FeatureType::FT_OpLance,
        nshogi::ml::FeatureType::FT_OpKnight,
        nshogi::ml::FeatureType::FT_OpSilver,
        nshogi::ml::FeatureType::FT_OpBishop,
        nshogi::ml::FeatureType::FT_OpRook, nshogi::ml::FeatureType::FT_OpGold,
        nshogi::ml::FeatureType::FT_OpKing,
        nshogi::ml::FeatureType::FT_OpProPawn,
        nshogi::ml::FeatureType::FT_OpProLance,
        nshogi::ml::FeatureType::FT_OpProKnight,
        nshogi::ml::FeatureType::FT_OpProSilver,
        nshogi::ml::FeatureType::FT_OpProBishop,
        nshogi::ml::FeatureType::FT_OpProRook>
        Features(State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type : nshogi::core::PieceTypes) {
            TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
            TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(I)),
                           Adapter->getBitboard(C, Type));

            if (State.getPosition().sideToMove() == nshogi::core::Black) {
                TEST_ASSERT_FALSE(Features.get(I).isRotated());
            } else {
                TEST_ASSERT_TRUE(Features.get(I).isRotated());
            }

            ++I;
        }
    }
}

void checkFeatureTypeOnBoardRuntime(const nshogi::core::State& State,
                                    const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_MyPawn,
         nshogi::ml::FeatureType::FT_MyLance,
         nshogi::ml::FeatureType::FT_MyKnight,
         nshogi::ml::FeatureType::FT_MySilver,
         nshogi::ml::FeatureType::FT_MyBishop,
         nshogi::ml::FeatureType::FT_MyRook,
         nshogi::ml::FeatureType::FT_MyGold,
         nshogi::ml::FeatureType::FT_MyKing,
         nshogi::ml::FeatureType::FT_MyProPawn,
         nshogi::ml::FeatureType::FT_MyProLance,
         nshogi::ml::FeatureType::FT_MyProKnight,
         nshogi::ml::FeatureType::FT_MyProSilver,
         nshogi::ml::FeatureType::FT_MyProBishop,
         nshogi::ml::FeatureType::FT_MyProRook,
         nshogi::ml::FeatureType::FT_OpPawn,
         nshogi::ml::FeatureType::FT_OpLance,
         nshogi::ml::FeatureType::FT_OpKnight,
         nshogi::ml::FeatureType::FT_OpSilver,
         nshogi::ml::FeatureType::FT_OpBishop,
         nshogi::ml::FeatureType::FT_OpRook,
         nshogi::ml::FeatureType::FT_OpGold,
         nshogi::ml::FeatureType::FT_OpKing,
         nshogi::ml::FeatureType::FT_OpProPawn,
         nshogi::ml::FeatureType::FT_OpProLance,
         nshogi::ml::FeatureType::FT_OpProKnight,
         nshogi::ml::FeatureType::FT_OpProSilver,
         nshogi::ml::FeatureType::FT_OpProBishop,
         nshogi::ml::FeatureType::FT_OpProRook},
        State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type : nshogi::core::PieceTypes) {
            TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
            TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(I)),
                           Adapter->getBitboard(C, Type));

            if (State.getPosition().sideToMove() == nshogi::core::Black) {
                TEST_ASSERT_FALSE(Features.get(I).isRotated());
            } else {
                TEST_ASSERT_TRUE(Features.get(I).isRotated());
            }

            ++I;
        }
    }
}

void checkFeatureTypeSmallStand(const nshogi::core::State& State,
                                const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_MyStandPawn1,
        nshogi::ml::FeatureType::FT_MyStandPawn2,
        nshogi::ml::FeatureType::FT_MyStandPawn3,
        nshogi::ml::FeatureType::FT_MyStandPawn4,
        nshogi::ml::FeatureType::FT_MyStandPawn5,
        nshogi::ml::FeatureType::FT_MyStandPawn6,
        nshogi::ml::FeatureType::FT_MyStandPawn7,
        nshogi::ml::FeatureType::FT_MyStandPawn8,
        nshogi::ml::FeatureType::FT_MyStandPawn9,
        nshogi::ml::FeatureType::FT_MyStandLance1,
        nshogi::ml::FeatureType::FT_MyStandLance2,
        nshogi::ml::FeatureType::FT_MyStandLance3,
        nshogi::ml::FeatureType::FT_MyStandLance4,
        nshogi::ml::FeatureType::FT_MyStandKnight1,
        nshogi::ml::FeatureType::FT_MyStandKnight2,
        nshogi::ml::FeatureType::FT_MyStandKnight3,
        nshogi::ml::FeatureType::FT_MyStandKnight4,
        nshogi::ml::FeatureType::FT_MyStandSilver1,
        nshogi::ml::FeatureType::FT_MyStandSilver2,
        nshogi::ml::FeatureType::FT_MyStandSilver3,
        nshogi::ml::FeatureType::FT_MyStandSilver4,
        nshogi::ml::FeatureType::FT_MyStandGold1,
        nshogi::ml::FeatureType::FT_MyStandGold2,
        nshogi::ml::FeatureType::FT_MyStandGold3,
        nshogi::ml::FeatureType::FT_MyStandGold4,
        nshogi::ml::FeatureType::FT_OpStandPawn1,
        nshogi::ml::FeatureType::FT_OpStandPawn2,
        nshogi::ml::FeatureType::FT_OpStandPawn3,
        nshogi::ml::FeatureType::FT_OpStandPawn4,
        nshogi::ml::FeatureType::FT_OpStandPawn5,
        nshogi::ml::FeatureType::FT_OpStandPawn6,
        nshogi::ml::FeatureType::FT_OpStandPawn7,
        nshogi::ml::FeatureType::FT_OpStandPawn8,
        nshogi::ml::FeatureType::FT_OpStandPawn9,
        nshogi::ml::FeatureType::FT_OpStandLance1,
        nshogi::ml::FeatureType::FT_OpStandLance2,
        nshogi::ml::FeatureType::FT_OpStandLance3,
        nshogi::ml::FeatureType::FT_OpStandLance4,
        nshogi::ml::FeatureType::FT_OpStandKnight1,
        nshogi::ml::FeatureType::FT_OpStandKnight2,
        nshogi::ml::FeatureType::FT_OpStandKnight3,
        nshogi::ml::FeatureType::FT_OpStandKnight4,
        nshogi::ml::FeatureType::FT_OpStandSilver1,
        nshogi::ml::FeatureType::FT_OpStandSilver2,
        nshogi::ml::FeatureType::FT_OpStandSilver3,
        nshogi::ml::FeatureType::FT_OpStandSilver4,
        nshogi::ml::FeatureType::FT_OpStandGold1,
        nshogi::ml::FeatureType::FT_OpStandGold2,
        nshogi::ml::FeatureType::FT_OpStandGold3,
        nshogi::ml::FeatureType::FT_OpStandGold4>
        Features(State, Config);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type :
             {nshogi::core::PTK_Pawn, nshogi::core::PTK_Lance,
              nshogi::core::PTK_Knight, nshogi::core::PTK_Silver,
              nshogi::core::PTK_Gold}) {
            const uint8_t CountMax =
                (Type == nshogi::core::PieceTypeKind::PTK_Pawn) ? 9 : 4;
            for (uint8_t Count = 1; Count <= CountMax; ++Count) {
                if (State.getPosition().getStandCount(C, Type) >= Count) {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                } else {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 0.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                }

                ++I;
            }
        }
    }
}

void checkFeatureTypeSmallStandRuntime(
    const nshogi::core::State& State, const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_MyStandPawn1,
         nshogi::ml::FeatureType::FT_MyStandPawn2,
         nshogi::ml::FeatureType::FT_MyStandPawn3,
         nshogi::ml::FeatureType::FT_MyStandPawn4,
         nshogi::ml::FeatureType::FT_MyStandPawn5,
         nshogi::ml::FeatureType::FT_MyStandPawn6,
         nshogi::ml::FeatureType::FT_MyStandPawn7,
         nshogi::ml::FeatureType::FT_MyStandPawn8,
         nshogi::ml::FeatureType::FT_MyStandPawn9,
         nshogi::ml::FeatureType::FT_MyStandLance1,
         nshogi::ml::FeatureType::FT_MyStandLance2,
         nshogi::ml::FeatureType::FT_MyStandLance3,
         nshogi::ml::FeatureType::FT_MyStandLance4,
         nshogi::ml::FeatureType::FT_MyStandKnight1,
         nshogi::ml::FeatureType::FT_MyStandKnight2,
         nshogi::ml::FeatureType::FT_MyStandKnight3,
         nshogi::ml::FeatureType::FT_MyStandKnight4,
         nshogi::ml::FeatureType::FT_MyStandSilver1,
         nshogi::ml::FeatureType::FT_MyStandSilver2,
         nshogi::ml::FeatureType::FT_MyStandSilver3,
         nshogi::ml::FeatureType::FT_MyStandSilver4,
         nshogi::ml::FeatureType::FT_MyStandGold1,
         nshogi::ml::FeatureType::FT_MyStandGold2,
         nshogi::ml::FeatureType::FT_MyStandGold3,
         nshogi::ml::FeatureType::FT_MyStandGold4,
         nshogi::ml::FeatureType::FT_OpStandPawn1,
         nshogi::ml::FeatureType::FT_OpStandPawn2,
         nshogi::ml::FeatureType::FT_OpStandPawn3,
         nshogi::ml::FeatureType::FT_OpStandPawn4,
         nshogi::ml::FeatureType::FT_OpStandPawn5,
         nshogi::ml::FeatureType::FT_OpStandPawn6,
         nshogi::ml::FeatureType::FT_OpStandPawn7,
         nshogi::ml::FeatureType::FT_OpStandPawn8,
         nshogi::ml::FeatureType::FT_OpStandPawn9,
         nshogi::ml::FeatureType::FT_OpStandLance1,
         nshogi::ml::FeatureType::FT_OpStandLance2,
         nshogi::ml::FeatureType::FT_OpStandLance3,
         nshogi::ml::FeatureType::FT_OpStandLance4,
         nshogi::ml::FeatureType::FT_OpStandKnight1,
         nshogi::ml::FeatureType::FT_OpStandKnight2,
         nshogi::ml::FeatureType::FT_OpStandKnight3,
         nshogi::ml::FeatureType::FT_OpStandKnight4,
         nshogi::ml::FeatureType::FT_OpStandSilver1,
         nshogi::ml::FeatureType::FT_OpStandSilver2,
         nshogi::ml::FeatureType::FT_OpStandSilver3,
         nshogi::ml::FeatureType::FT_OpStandSilver4,
         nshogi::ml::FeatureType::FT_OpStandGold1,
         nshogi::ml::FeatureType::FT_OpStandGold2,
         nshogi::ml::FeatureType::FT_OpStandGold3,
         nshogi::ml::FeatureType::FT_OpStandGold4},
        State, Config);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type :
             {nshogi::core::PTK_Pawn, nshogi::core::PTK_Lance,
              nshogi::core::PTK_Knight, nshogi::core::PTK_Silver,
              nshogi::core::PTK_Gold}) {
            const uint8_t CountMax =
                (Type == nshogi::core::PieceTypeKind::PTK_Pawn) ? 9 : 4;
            for (uint8_t Count = 1; Count <= CountMax; ++Count) {
                if (State.getPosition().getStandCount(C, Type) >= Count) {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                } else {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 0.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                }

                ++I;
            }
        }
    }
}

void checkFeatureTypeLargeStand(const nshogi::core::State& State,
                                const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_MyStandBishop1,
        nshogi::ml::FeatureType::FT_MyStandBishop2,
        nshogi::ml::FeatureType::FT_MyStandRook1,
        nshogi::ml::FeatureType::FT_MyStandRook2,
        nshogi::ml::FeatureType::FT_OpStandBishop1,
        nshogi::ml::FeatureType::FT_OpStandBishop2,
        nshogi::ml::FeatureType::FT_OpStandRook1,
        nshogi::ml::FeatureType::FT_OpStandRook2>
        Features(State, Config);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type :
             {nshogi::core::PTK_Bishop, nshogi::core::PTK_Rook}) {
            for (uint8_t Count = 1; Count <= 2; ++Count) {
                if (State.getPosition().getStandCount(C, Type) >= Count) {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                } else {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 0.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                }

                ++I;
            }
        }
    }
}

void checkFeatureTypeLargeStandRuntime(
    const nshogi::core::State& State, const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_MyStandBishop1,
         nshogi::ml::FeatureType::FT_MyStandBishop2,
         nshogi::ml::FeatureType::FT_MyStandRook1,
         nshogi::ml::FeatureType::FT_MyStandRook2,
         nshogi::ml::FeatureType::FT_OpStandBishop1,
         nshogi::ml::FeatureType::FT_OpStandBishop2,
         nshogi::ml::FeatureType::FT_OpStandRook1,
         nshogi::ml::FeatureType::FT_OpStandRook2},
        State, Config);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        for (nshogi::core::PieceTypeKind Type :
             {nshogi::core::PTK_Bishop, nshogi::core::PTK_Rook}) {
            for (uint8_t Count = 1; Count <= 2; ++Count) {
                if (State.getPosition().getStandCount(C, Type) >= Count) {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                } else {
                    TEST_ASSERT_EQ(Features.get(I).getValue(), 0.0f);
                    TEST_ASSERT_EQ(
                        FeatureBitboardUtil::getBitboard(Features.get(I)),
                        nshogi::core::internal::bitboard::Bitboard::AllBB());
                }

                ++I;
            }
        }
    }
}

void checkFeatureTypeCheck(const nshogi::core::State& State,
                           const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<nshogi::ml::FeatureType::FT_Check>
        Features(State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    if (Adapter->getCheckerBB().isZero()) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
    } else {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
    }
}

void checkFeatureTypeCheckRuntime(const nshogi::core::State& State,
                                  const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_Check}, State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    if (Adapter->getCheckerBB().isZero()) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::ZeroBB());
    } else {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                       nshogi::core::internal::bitboard::Bitboard::AllBB());
    }
}

void checkFeatureTypeNoPawnFile(const nshogi::core::State& State,
                                const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_NoMyPawnFile,
        nshogi::ml::FeatureType::FT_NoOpPawnFile>
        Features(State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        nshogi::core::internal::bitboard::Bitboard PawnFileBB =
            nshogi::core::internal::bitboard::Bitboard::ZeroBB();

        const auto PawnBB = Adapter->getBitboard(C, nshogi::core::PTK_Pawn);

        for (nshogi::core::File F : nshogi::core::Files) {
            if (!(PawnBB & nshogi::core::internal::bitboard::FileBB[F])
                     .isZero()) {
                PawnFileBB |= nshogi::core::internal::bitboard::FileBB[F];
            }
        }

        TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(I)),
                       ~PawnFileBB);

        if (State.getPosition().sideToMove() == nshogi::core::Black) {
            TEST_ASSERT_FALSE(Features.get(I).isRotated());
        } else {
            TEST_ASSERT_TRUE(Features.get(I).isRotated());
        }

        ++I;
    }
}

void checkFeatureTypeNoPawnFileRuntime(
    const nshogi::core::State& State, const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_NoMyPawnFile,
         nshogi::ml::FeatureType::FT_NoOpPawnFile},
        State, Config);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    std::size_t I = 0;
    for (nshogi::core::Color C : {State.getPosition().sideToMove(),
                                  ~State.getPosition().sideToMove()}) {
        nshogi::core::internal::bitboard::Bitboard PawnFileBB =
            nshogi::core::internal::bitboard::Bitboard::ZeroBB();

        const auto PawnBB = Adapter->getBitboard(C, nshogi::core::PTK_Pawn);

        for (nshogi::core::File F : nshogi::core::Files) {
            if (!(PawnBB & nshogi::core::internal::bitboard::FileBB[F])
                     .isZero()) {
                PawnFileBB |= nshogi::core::internal::bitboard::FileBB[F];
            }
        }

        TEST_ASSERT_EQ(Features.get(I).getValue(), 1.0f);
        TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(I)),
                       ~PawnFileBB);

        if (State.getPosition().sideToMove() == nshogi::core::Black) {
            TEST_ASSERT_FALSE(Features.get(I).isRotated());
        } else {
            TEST_ASSERT_TRUE(Features.get(I).isRotated());
        }

        ++I;
    }
}

void checkFeatureTypeProgress(const nshogi::core::State& State,
                              const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<nshogi::ml::FeatureType::FT_Progress>
        Features(State, Config);

    TEST_ASSERT_FLOAT_EQ(Features.get(0).getValue(),
                         (double)State.getPly() / Config.MaxPly, 1e-6);
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
}

void checkFeatureTypeProgressRuntime(const nshogi::core::State& State,
                                     const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_Progress}, State, Config);

    TEST_ASSERT_FLOAT_EQ(Features.get(0).getValue(),
                         (double)State.getPly() / Config.MaxPly, 1e-6);
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
}

void checkFeatureTypeProgressUnit(const nshogi::core::State& State,
                                  const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_ProgressUnit>
        Features(State, Config);

    TEST_ASSERT_FLOAT_EQ(Features.get(0).getValue(), 1.0f / Config.MaxPly,
                         1e-6);
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
}

void checkFeatureTypeProgressUnitRuntime(
    const nshogi::core::State& State, const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_ProgressUnit}, State, Config);

    TEST_ASSERT_FLOAT_EQ(Features.get(0).getValue(), 1.0f / Config.MaxPly,
                         1e-6);
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
}

void checkFeatureTypeRule(const nshogi::core::State& State,
                          const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_RuleDeclare27,
        nshogi::ml::FeatureType::FT_RuleDraw24,
        nshogi::ml::FeatureType::FT_RuleTrying>
        Features(State, Config);

    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(2)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());

    if (Config.Rule == nshogi::core::EndingRule::ER_Declare27) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 0.0f);
    } else if (Config.Rule == nshogi::core::EndingRule::ER_Draw24) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 1.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 0.0f);
    } else if (Config.Rule == nshogi::core::EndingRule::ER_Trying) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 1.0f);
    }
}

void checkFeatureTypeRuleRuntime(const nshogi::core::State& State,
                                 const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackRuntime Features(
        {nshogi::ml::FeatureType::FT_RuleDeclare27,
         nshogi::ml::FeatureType::FT_RuleDraw24,
         nshogi::ml::FeatureType::FT_RuleTrying},
        State, Config);

    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(0)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(1)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());
    TEST_ASSERT_EQ(FeatureBitboardUtil::getBitboard(Features.get(2)),
                   nshogi::core::internal::bitboard::Bitboard::AllBB());

    if (Config.Rule == nshogi::core::EndingRule::ER_Declare27) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 1.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 0.0f);
    } else if (Config.Rule == nshogi::core::EndingRule::ER_Draw24) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 1.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 0.0f);
    } else if (Config.Rule == nshogi::core::EndingRule::ER_Trying) {
        TEST_ASSERT_EQ(Features.get(0).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(1).getValue(), 0.0f);
        TEST_ASSERT_EQ(Features.get(2).getValue(), 1.0f);
    }
}

void checkFeatureType(const nshogi::core::State& State,
                      const nshogi::core::StateConfig& Config) {
    checkFeatureTypeColor(State, Config);
    checkFeatureTypeOnBoard(State, Config);
    checkFeatureTypeSmallStand(State, Config);
    checkFeatureTypeLargeStand(State, Config);
    checkFeatureTypeCheck(State, Config);
    checkFeatureTypeNoPawnFile(State, Config);
    checkFeatureTypeProgress(State, Config);
    checkFeatureTypeProgressUnit(State, Config);
    checkFeatureTypeRule(State, Config);
}

void checkFeatureTypeRuntime(const nshogi::core::State& State,
                             const nshogi::core::StateConfig& Config) {
    checkFeatureTypeColorRuntime(State, Config);
    checkFeatureTypeOnBoardRuntime(State, Config);
    checkFeatureTypeSmallStandRuntime(State, Config);
    checkFeatureTypeLargeStandRuntime(State, Config);
    checkFeatureTypeCheckRuntime(State, Config);
    checkFeatureTypeNoPawnFileRuntime(State, Config);
    checkFeatureTypeProgressRuntime(State, Config);
    checkFeatureTypeProgressUnitRuntime(State, Config);
    checkFeatureTypeRuleRuntime(State, Config);
}

void checkEqualComptimeAndRuntime(const nshogi::core::State& State,
                                  const nshogi::core::StateConfig& Config) {
    const nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_Black, nshogi::ml::FeatureType::FT_White,
        nshogi::ml::FeatureType::FT_MyPawn, nshogi::ml::FeatureType::FT_MyLance,
        nshogi::ml::FeatureType::FT_MyKnight,
        nshogi::ml::FeatureType::FT_MySilver,
        nshogi::ml::FeatureType::FT_MyBishop,
        nshogi::ml::FeatureType::FT_MyRook, nshogi::ml::FeatureType::FT_MyGold,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_MyProPawn,
        nshogi::ml::FeatureType::FT_MyProLance,
        nshogi::ml::FeatureType::FT_MyProKnight,
        nshogi::ml::FeatureType::FT_MyProSilver,
        nshogi::ml::FeatureType::FT_MyProBishop,
        nshogi::ml::FeatureType::FT_MyProRook,
        nshogi::ml::FeatureType::FT_OpPawn, nshogi::ml::FeatureType::FT_OpLance,
        nshogi::ml::FeatureType::FT_OpKnight,
        nshogi::ml::FeatureType::FT_OpSilver,
        nshogi::ml::FeatureType::FT_OpBishop,
        nshogi::ml::FeatureType::FT_OpRook, nshogi::ml::FeatureType::FT_OpGold,
        nshogi::ml::FeatureType::FT_OpKing,
        nshogi::ml::FeatureType::FT_OpProPawn,
        nshogi::ml::FeatureType::FT_OpProLance,
        nshogi::ml::FeatureType::FT_OpProKnight,
        nshogi::ml::FeatureType::FT_OpProSilver,
        nshogi::ml::FeatureType::FT_OpProBishop,
        nshogi::ml::FeatureType::FT_OpProRook,
        nshogi::ml::FeatureType::FT_MyStandPawn1,
        nshogi::ml::FeatureType::FT_MyStandPawn2,
        nshogi::ml::FeatureType::FT_MyStandPawn3,
        nshogi::ml::FeatureType::FT_MyStandPawn4,
        nshogi::ml::FeatureType::FT_MyStandLance1,
        nshogi::ml::FeatureType::FT_MyStandLance2,
        nshogi::ml::FeatureType::FT_MyStandLance3,
        nshogi::ml::FeatureType::FT_MyStandLance4,
        nshogi::ml::FeatureType::FT_MyStandKnight1,
        nshogi::ml::FeatureType::FT_MyStandKnight2,
        nshogi::ml::FeatureType::FT_MyStandKnight3,
        nshogi::ml::FeatureType::FT_MyStandKnight4,
        nshogi::ml::FeatureType::FT_MyStandSilver1,
        nshogi::ml::FeatureType::FT_MyStandSilver2,
        nshogi::ml::FeatureType::FT_MyStandSilver3,
        nshogi::ml::FeatureType::FT_MyStandSilver4,
        nshogi::ml::FeatureType::FT_MyStandGold1,
        nshogi::ml::FeatureType::FT_MyStandGold2,
        nshogi::ml::FeatureType::FT_MyStandGold3,
        nshogi::ml::FeatureType::FT_MyStandGold4,
        nshogi::ml::FeatureType::FT_OpStandPawn1,
        nshogi::ml::FeatureType::FT_OpStandPawn2,
        nshogi::ml::FeatureType::FT_OpStandPawn3,
        nshogi::ml::FeatureType::FT_OpStandPawn4,
        nshogi::ml::FeatureType::FT_OpStandLance1,
        nshogi::ml::FeatureType::FT_OpStandLance2,
        nshogi::ml::FeatureType::FT_OpStandLance3,
        nshogi::ml::FeatureType::FT_OpStandLance4,
        nshogi::ml::FeatureType::FT_OpStandKnight1,
        nshogi::ml::FeatureType::FT_OpStandKnight2,
        nshogi::ml::FeatureType::FT_OpStandKnight3,
        nshogi::ml::FeatureType::FT_OpStandKnight4,
        nshogi::ml::FeatureType::FT_OpStandSilver1,
        nshogi::ml::FeatureType::FT_OpStandSilver2,
        nshogi::ml::FeatureType::FT_OpStandSilver3,
        nshogi::ml::FeatureType::FT_OpStandSilver4,
        nshogi::ml::FeatureType::FT_OpStandGold1,
        nshogi::ml::FeatureType::FT_OpStandGold2,
        nshogi::ml::FeatureType::FT_OpStandGold3,
        nshogi::ml::FeatureType::FT_OpStandGold4,
        nshogi::ml::FeatureType::FT_MyStandBishop1,
        nshogi::ml::FeatureType::FT_MyStandBishop2,
        nshogi::ml::FeatureType::FT_MyStandRook1,
        nshogi::ml::FeatureType::FT_MyStandRook2,
        nshogi::ml::FeatureType::FT_OpStandBishop1,
        nshogi::ml::FeatureType::FT_OpStandBishop2,
        nshogi::ml::FeatureType::FT_OpStandRook1,
        nshogi::ml::FeatureType::FT_OpStandRook2,
        nshogi::ml::FeatureType::FT_Check,
        nshogi::ml::FeatureType::FT_NoMyPawnFile,
        nshogi::ml::FeatureType::FT_NoOpPawnFile,
        nshogi::ml::FeatureType::FT_Progress,
        nshogi::ml::FeatureType::FT_ProgressUnit,
        nshogi::ml::FeatureType::FT_RuleDeclare27,
        nshogi::ml::FeatureType::FT_RuleDraw24,
        nshogi::ml::FeatureType::FT_RuleTrying>
        FeaturesComptime(State, Config);

    const nshogi::ml::FeatureStackRuntime FeaturesRuntime(
        {nshogi::ml::FeatureType::FT_Black,
         nshogi::ml::FeatureType::FT_White,
         nshogi::ml::FeatureType::FT_MyPawn,
         nshogi::ml::FeatureType::FT_MyLance,
         nshogi::ml::FeatureType::FT_MyKnight,
         nshogi::ml::FeatureType::FT_MySilver,
         nshogi::ml::FeatureType::FT_MyBishop,
         nshogi::ml::FeatureType::FT_MyRook,
         nshogi::ml::FeatureType::FT_MyGold,
         nshogi::ml::FeatureType::FT_MyKing,
         nshogi::ml::FeatureType::FT_MyProPawn,
         nshogi::ml::FeatureType::FT_MyProLance,
         nshogi::ml::FeatureType::FT_MyProKnight,
         nshogi::ml::FeatureType::FT_MyProSilver,
         nshogi::ml::FeatureType::FT_MyProBishop,
         nshogi::ml::FeatureType::FT_MyProRook,
         nshogi::ml::FeatureType::FT_OpPawn,
         nshogi::ml::FeatureType::FT_OpLance,
         nshogi::ml::FeatureType::FT_OpKnight,
         nshogi::ml::FeatureType::FT_OpSilver,
         nshogi::ml::FeatureType::FT_OpBishop,
         nshogi::ml::FeatureType::FT_OpRook,
         nshogi::ml::FeatureType::FT_OpGold,
         nshogi::ml::FeatureType::FT_OpKing,
         nshogi::ml::FeatureType::FT_OpProPawn,
         nshogi::ml::FeatureType::FT_OpProLance,
         nshogi::ml::FeatureType::FT_OpProKnight,
         nshogi::ml::FeatureType::FT_OpProSilver,
         nshogi::ml::FeatureType::FT_OpProBishop,
         nshogi::ml::FeatureType::FT_OpProRook,
         nshogi::ml::FeatureType::FT_MyStandPawn1,
         nshogi::ml::FeatureType::FT_MyStandPawn2,
         nshogi::ml::FeatureType::FT_MyStandPawn3,
         nshogi::ml::FeatureType::FT_MyStandPawn4,
         nshogi::ml::FeatureType::FT_MyStandLance1,
         nshogi::ml::FeatureType::FT_MyStandLance2,
         nshogi::ml::FeatureType::FT_MyStandLance3,
         nshogi::ml::FeatureType::FT_MyStandLance4,
         nshogi::ml::FeatureType::FT_MyStandKnight1,
         nshogi::ml::FeatureType::FT_MyStandKnight2,
         nshogi::ml::FeatureType::FT_MyStandKnight3,
         nshogi::ml::FeatureType::FT_MyStandKnight4,
         nshogi::ml::FeatureType::FT_MyStandSilver1,
         nshogi::ml::FeatureType::FT_MyStandSilver2,
         nshogi::ml::FeatureType::FT_MyStandSilver3,
         nshogi::ml::FeatureType::FT_MyStandSilver4,
         nshogi::ml::FeatureType::FT_MyStandGold1,
         nshogi::ml::FeatureType::FT_MyStandGold2,
         nshogi::ml::FeatureType::FT_MyStandGold3,
         nshogi::ml::FeatureType::FT_MyStandGold4,
         nshogi::ml::FeatureType::FT_OpStandPawn1,
         nshogi::ml::FeatureType::FT_OpStandPawn2,
         nshogi::ml::FeatureType::FT_OpStandPawn3,
         nshogi::ml::FeatureType::FT_OpStandPawn4,
         nshogi::ml::FeatureType::FT_OpStandLance1,
         nshogi::ml::FeatureType::FT_OpStandLance2,
         nshogi::ml::FeatureType::FT_OpStandLance3,
         nshogi::ml::FeatureType::FT_OpStandLance4,
         nshogi::ml::FeatureType::FT_OpStandKnight1,
         nshogi::ml::FeatureType::FT_OpStandKnight2,
         nshogi::ml::FeatureType::FT_OpStandKnight3,
         nshogi::ml::FeatureType::FT_OpStandKnight4,
         nshogi::ml::FeatureType::FT_OpStandSilver1,
         nshogi::ml::FeatureType::FT_OpStandSilver2,
         nshogi::ml::FeatureType::FT_OpStandSilver3,
         nshogi::ml::FeatureType::FT_OpStandSilver4,
         nshogi::ml::FeatureType::FT_OpStandGold1,
         nshogi::ml::FeatureType::FT_OpStandGold2,
         nshogi::ml::FeatureType::FT_OpStandGold3,
         nshogi::ml::FeatureType::FT_OpStandGold4,
         nshogi::ml::FeatureType::FT_MyStandBishop1,
         nshogi::ml::FeatureType::FT_MyStandBishop2,
         nshogi::ml::FeatureType::FT_MyStandRook1,
         nshogi::ml::FeatureType::FT_MyStandRook2,
         nshogi::ml::FeatureType::FT_OpStandBishop1,
         nshogi::ml::FeatureType::FT_OpStandBishop2,
         nshogi::ml::FeatureType::FT_OpStandRook1,
         nshogi::ml::FeatureType::FT_OpStandRook2,
         nshogi::ml::FeatureType::FT_Check,
         nshogi::ml::FeatureType::FT_NoMyPawnFile,
         nshogi::ml::FeatureType::FT_NoOpPawnFile,
         nshogi::ml::FeatureType::FT_Progress,
         nshogi::ml::FeatureType::FT_ProgressUnit,
         nshogi::ml::FeatureType::FT_RuleDeclare27,
         nshogi::ml::FeatureType::FT_RuleDraw24,
         nshogi::ml::FeatureType::FT_RuleTrying},
        State, Config);

    for (std::size_t I = 0; I < 86; ++I) {
        TEST_ASSERT_EQ(FeaturesComptime.get(I).getValue(),
                       FeaturesRuntime.get(I).getValue());
        TEST_ASSERT_EQ(
            FeatureBitboardUtil::getBitboard(FeaturesComptime.get(I)),
            FeatureBitboardUtil::getBitboard(FeaturesRuntime.get(I)));
        TEST_ASSERT_EQ(FeaturesComptime.get(I).isRotated(),
                       FeaturesRuntime.get(I).isRotated());
    }

    const auto ExtractedComptime = FeaturesComptime.extract();
    const auto ExtractedRuntime = FeaturesRuntime.extract();

    TEST_ASSERT_EQ(ExtractedComptime.size(), ExtractedRuntime.size());

    for (std::size_t I = 0; I < ExtractedComptime.size(); ++I) {
        TEST_ASSERT_EQ(ExtractedComptime[I], ExtractedRuntime[I]);
    }
}

} // namespace

TEST(ML, FeatureType) {
    const std::vector<std::string> Sfens({
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1",
        "lnsgk1snl/1r4gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BG4R1/LNS1KGSNL b - 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/P8/1PPPPPP1P/1BG3SR1/LNS1KG1NL w "
        "- 1",
        "ln1gk1snl/1rs3gb1/p1pppppp1/8p/7P1/P5P2/1PPPPP2P/1BG1K1SR1/LNS2G1NL w "
        "p 1",
        "ln1g2snl/1rs1k1gb1/p1pppp1p1/6p1p/9/P1P3PR1/1P1PPP2P/1BG1K1S2/"
        "LNS2G1NL w Pp 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/8P/PPPPPPP2/1BG3SR1/LNS1KG1NL w - "
        "1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BGK2SR1/LNS2G1NL w - "
        "1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BG1K1SR1/LNS2G1NL w "
        "- 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/7P1/1p7/9/PPPPPPP1P/1BG3SR1/LNS1KG1NL w - "
        "1",
        "ln1gk1snl/2s3gb1/p1pppp2p/6pR1/9/1rP3P2/P2PPP2P/1BG1K4/LNS2GSNL w "
        "2P2p 1",
        "ln1g2snl/2s1k1g2/2pppp2p/p5pp1/5N3/PrP3P2/B2PPP2P/1SGK2S+b1/LN3G2L b "
        "Pr2p 1",
        "ln4knl/1r2g1g2/p1s1ppsp1/b2P2p1p/1p6P/2S1SP3/PP2P1P2/2GK3R1/LN3G1NL b "
        "B3Pp 1",
        "ln1gk1snl/1rs3gb1/p1p1pppp1/3R4p/9/P5P2/1PPPPP2P/1BG1K1S2/LNS2G1NL w "
        "2Pp 1",
        "ln1gk1s+Rl/2s3g2/p1pppp3/6p1p/1r7/PPP3P2/3PPP2P/1+b2K1S2/LNS2G1NL w "
        "BNPg2p 1",
        "ln1gk1snl/2s3gb1/p1pppp1pp/6p2/7P1/1rP1P4/P2P1PP1P/1BG2S1R1/LNSK1G1NL "
        "w Pp 1",
        "ln1g3n+B/1r1s2k2/p2ppp1pp/2p6/1p3s3/2P3P2/PPSPPP2P/3K2S2/LN1G1G2+b b "
        "GL2Prnl 1",
        "lnsgk1snl/6gb1/p1pppp2p/6R2/9/1rP6/P2PPPP1P/1BG6/LNS1KGSNL w 3P2p 1",
        "ln1gk1snl/1rs3gb1/p1p1pp1pp/1p1p2p2/7P1/2P6/PPSPPPP1P/1B5R1/LN1GKGSNL "
        "b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2P6/PPSPPPPPP/1B5R1/LN1GKGSNL w - 1",
        "ln1gk2nl/1r1s1sgb1/p1pppp1pp/1p4p2/9/2P4P1/PPSPPPP1P/1BG4R1/LN2KGSNL "
        "b - 1",
        "ln1gkg1nl/1r1s3b1/p1ppppspp/1p4p2/7P1/2P6/PPSPPPP1P/1BG4R1/LN2KGSNL w "
        "- 1",
        "lnsgk2nl/1r4gb1/p1ppppspp/1p4p2/7P1/2P6/PPSPPPP1P/1B5R1/LN1GKGSNL b - "
        "1",
        "lnsgk2nl/1r4gs1/p1pppp1pp/6p2/1p7/2P4P1/PPSPPPP1P/2G4R1/LN2KGSNL b Bb "
        "1",
        "lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p7/2P3PP1/PPBPPP2P/1S5R1/LN1GKGSNL w "
        "- 1",
        "lr5nl/3g1kg2/2n1p1sp1/p1pp2p1p/1p2sS1P1/P1PP2P1P/1PS1P1N2/2GK1G3/"
        "LN5RL b BPbp 1",
        "lr5nl/2g2kg2/2n1ppsp1/p1pps1p1p/1p7/P1PPSPPPP/1PS1P1N2/2G2G3/LNK4RL w "
        "Bb 1",
        "l2r3nl/2g2kg2/2n1ppsp1/p1p3p1p/1p1s3P1/P1P1SPP1P/1PS1P1N2/2G2G3/"
        "LNKR4L w BPbp 1",
        "lB5nl/2g2kg2/2n1ppsp1/p1p3p1p/1p1r3P1/P4PP1P/1PP1P1N2/2GS1G3/LNKR4L w "
        "SPbsp 1",
        "l2gk2nl/1r1s2g2/2npppsp1/p1p3p1p/1p5P1/P1P2PP1P/1PSPP1N2/2G3SR1/"
        "LN2KG2L w Bb 1",
        "l2gk2nl/1r1s2g2/2npppspp/p1p3p2/1p7/P1P2PPPP/1PSPP4/2GK2SR1/LN3G1NL w "
        "Bb 1",
        "ln5nl/4gkg2/p2pp1spp/5pp2/7P1/1rpP1PP2/P3PSN1P/2G1K1G2/LN5RL b "
        "BS2Pbsp 1",
        "ln1g3nl/1r3kg2/2psppsp1/p2p4p/1p3NpP1/P1P2P2P/1PSPP4/3K2SR1/LN1G1G2L "
        "w Bbp 1",
        "ln1g3n1/1r3kg2/2pspp1s1/p2p5/1p3NpRl/P1P2P3/1PSPP4/3K2S2/LN1G1G3 w "
        "B2Pbl3p 1",
        "ln1gk2nl/1r1s2g2/p2pppsp1/2p3p1p/1p5P1/P1P3P1P/1PSPPP3/5S1R1/"
        "LN1GKG1NL b Bb 1",
        "ln1g4l/1r2k1g2/6np1/p1pp1p2p/1p5b1/P1P2PP1P/1PSPPS3/2G2S3/LN2KG1PL w "
        "BS2Prn 1",
        "lnsgk3l/1r4gs1/p1ppppnp1/6p1p/1p5P1/2P3P1P/PPSPPP3/7R1/LN1GKGSNL b Bb "
        "1",
        "l6nl/3g2kg1/4ppb2/p1PPs1ppp/1r3P3/P1p1S1P1P/1S2P4/1PG2G3/LNK4RL b "
        "BN3Psn 1",
        "l1S4nl/1r1g1kgs1/2n1p2p1/p2p2p1p/2p1Sp1P1/PPn2PP1P/3PP4/2GK1G1R1/LN6L "
        "b BPbsp 1",
        "lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p5P1/2PP5/PPB1PPP1P/7R1/LNSGKGSNL w - "
        "1",
        "ln4knl/1r2gbg2/p1sp2spp/2p1ppp2/1p5PP/2PPPPP2/PP1S2N2/2GBGS1R1/LN2K3L "
        "w - 1",
        "ln2k2nl/1r1sg1gb1/p1pp1s1pp/4ppp2/1p7/2P1SPP2/PPBPP1NPP/2GS3R1/"
        "LN1K1G2L w - 1",
        "ln5nl/1r2gkg2/p1pps2pp/4ppp2/1p7/2P6/PPSPP1PPP/2G2R3/LNK2G1NL w BSPbs "
        "1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/6p2/1p7/2PP5/PPB1PPPPP/1R7/LNSGKGSNL w - 1",
        "lnsgkg1nl/7r1/1p1pppspp/2p3p2/p6P1/2P6/PP1PPPP1P/1SK1GS1R1/LN1G3NL b "
        "Bb 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/2P6/9/PP1PPPPPP/1BR6/LNSGKGSNL w - 1",
        "lnsgkg1nl/5srb1/ppppppppp/9/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2PP5/PP2PPPPP/1BR6/LNSGKGSNL w - 1",
        "lnsgkgsnl/6rb1/pppppp1pp/6p2/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2PP5/PP2PPPPP/1B1R5/LNSGKGSNL w - 1",
        "lnsgkgsnl/5r1b1/pppppp1pp/6p2/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/5r3/ppppp1bpp/5pp2/7P1/2P6/PP1PPPP1P/1B3S1R1/LNSGKG1NL b - "
        "1",
        "lnsgkgsnl/4r2b1/pppp1p1pp/4p1p2/7P1/2P6/PP1PPPP1P/1B5R1/LNSGKGSNL b - "
        "1",
        "ln1gkgsnl/1r1s3b1/p1pppp1pp/1p4p2/4P4/2P6/PP1P1PPPP/1B2R4/LNSGKGSNL w "
        "- 1",
        "lnsg1g1nl/3k1srb1/pppppp1pp/9/6p2/2PP5/PPBSPPPPP/1R7/LN1GKGSNL w - 1",
        "ln1g3nl/1r1kP2g1/3p1ps1p/p3p1p2/8P/P4P3/1P1P+b1PR1/2G1S4/LN2KG1NL w "
        "BS3Ps2p 1",
        "4k4/9/9/9/9/9/9/9/4K4 b 2R2B4G4S4N4L18P 1",
        "4k4/9/9/9/9/9/9/9/4K4 w 2r2b4g4s4n4l18p 1",
        "ln1g3R1/1ks1g4/1ppp4p/p3P1p2/3P+Bp1n1/P1P3P1P/1PNGSP3/1+bKG5/2S2+r3 b "
        "L3Psn2l 1",
    });

    for (const auto& Sfen : Sfens) {
        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::newState(Sfen);
        nshogi::core::StateConfig Config;

        checkFeatureType(State, Config);
        checkFeatureTypeRuntime(State, Config);
    }
}

TEST(ML, FeatureTypeRandom) {
    std::mt19937_64 mt(20230627);

    const int N = 10;

    auto Config = nshogi::core::StateConfig();

    for (int Count = 0; Count < N; ++Count) {
        auto State = nshogi::core::StateBuilder::getInitialState();

        Config.Rule = (nshogi::core::EndingRule)(mt() % 3);

        for (uint16_t Ply = 0; Ply < Config.MaxPly; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto NextMove = Moves[mt() % Moves.size()];
            State.doMove(NextMove);

            checkEqualComptimeAndRuntime(State, Config);
        }
    }
}

TEST(ML, AZTeacherSaveAndLoad) {
    nshogi::ml::AZTeacher T1;

    const int N = 100;

    const std::string Path = std::filesystem::temp_directory_path().string() +
                             "/az_teacher_test.bin";

    for (int I = 0; I < N; ++I) {
        T1.corruptMyself();

        {
            std::ofstream Ofs(Path, std::ios::out | std::ios::binary);
            nshogi::io::file::save(Ofs, T1);
        }

        {
            std::ifstream Ifs(Path, std::ios::in | std::ios::binary);

            auto T2 = nshogi::io::file::load<nshogi::ml::AZTeacher>(Ifs);

            TEST_ASSERT_TRUE(T1.equals(T2));
            TEST_ASSERT_TRUE(T2.equals(T1));
        }
    }
}

TEST(ML, AZTeacherHandmade1) {
    nshogi::ml::AZTeacher T1;
    T1.corruptMyself();

    nshogi::ml::AZTeacher T2(T1);

    TEST_ASSERT_TRUE(T1.equals(T2));
    TEST_ASSERT_TRUE(T2.equals(T1));

    // Change helpers.
    T1.Declared = true;
    T2.Declared = false;

    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    // Change configs.
    T2.WhiteDrawValue = T2.WhiteDrawValue * 0.20230711f;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T2.BlackDrawValue = T2.BlackDrawValue * 0.1613f;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T1.EndingRule = nshogi::core::ER_NoRule;
    T2.EndingRule = nshogi::core::ER_Declare27;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T2.MaxPly = T2.MaxPly + 1;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    // Change targets.
    T2.Visits[0] = T2.Visits[0] + 1;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T2.NumMoves = T2.NumMoves + 2;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T1.SideToMove = nshogi::core::Color::White;
    T2.SideToMove = nshogi::core::Color::Black;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    T1.Winner = nshogi::core::Color::White;
    T2.Winner = nshogi::core::Color::Black;
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));

    std::strcpy(T1.Sfen,
                "lnsgkgsnl/9/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1");
    std::strcpy(
        T2.Sfen,
        "lnsgkgsnl/1b5r1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    TEST_ASSERT_FALSE(T1.equals(T2));
    TEST_ASSERT_FALSE(T2.equals(T1));
}

TEST(ML, SimpleTeacherCopyConstructor) {
    const int N = 100;
    std::mt19937_64 mt(20240525);

    nshogi::core::StateConfig Config;
    nshogi::ml::SimpleTeacher SimpleTeacher;

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 1; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];

            SimpleTeacher.setState(State)
                .setConfig(Config)
                .setNextMove(nshogi::core::Move16(RandomMove))
                .setWinner(State.getSideToMove());

            nshogi::ml::SimpleTeacher SimpleTeacher2(SimpleTeacher);

            TEST_ASSERT_TRUE(SimpleTeacher.equals(SimpleTeacher2));

            State.doMove(RandomMove);
        }
    }
}

TEST(ML, SimpleTeacherPosition) {
    const int N = 100;
    std::mt19937_64 mt(20240525);

    nshogi::ml::SimpleTeacher SimpleTeacher;

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            SimpleTeacher.setState(State);
            TEST_ASSERT_TRUE(
                State.getPosition().equals(SimpleTeacher.getPosition(), true));

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(ML, SimpleTeacherState) {
    const int N = 100;
    std::mt19937_64 mt(20240525);

    nshogi::ml::SimpleTeacher SimpleTeacher;

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            SimpleTeacher.setState(State);
            TEST_ASSERT_TRUE(State.getPosition().equals(
                SimpleTeacher.getState().getPosition(), true));
            TEST_ASSERT_EQ(State.getPly(), SimpleTeacher.getState().getPly());

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(ML, SimpleTeacherConfig) {
    nshogi::ml::SimpleTeacher SimpleTeacher;

    for (uint16_t MaxPly = 1; MaxPly < 10000; ++MaxPly) {
        for (float BlackDrawValue = 0.0f; BlackDrawValue <= 1.0f;
             BlackDrawValue += 0.01f) {
            for (float WhiteDrawValue = 0.0f; WhiteDrawValue <= 1.0f;
                 WhiteDrawValue += 0.01f) {
                nshogi::core::StateConfig Config;
                Config.MaxPly = MaxPly;
                Config.BlackDrawValue = BlackDrawValue;
                Config.WhiteDrawValue = WhiteDrawValue;

                SimpleTeacher.setConfig(Config);

                TEST_ASSERT_EQ(SimpleTeacher.getConfig().MaxPly, MaxPly);
                TEST_ASSERT_EQ(SimpleTeacher.getConfig().BlackDrawValue,
                               BlackDrawValue);
                TEST_ASSERT_EQ(SimpleTeacher.getConfig().WhiteDrawValue,
                               WhiteDrawValue);
            }
        }
    }
}

TEST(ML, SimpleTeacherNextMove) {
    nshogi::ml::SimpleTeacher SimpleTeacher;

    for (uint32_t Val = 0; Val < (1 << 16); ++Val) {
        auto Move = nshogi::core::Move16::fromValue((uint16_t)Val);
        SimpleTeacher.setNextMove(Move);

        TEST_ASSERT_EQ(SimpleTeacher.getNextMove(), Move);
    }
}

TEST(ML, SimpleTeacherWinnerBlack) {
    nshogi::ml::SimpleTeacher SimpleTeacher;
    SimpleTeacher.setWinner(nshogi::core::Black);
    TEST_ASSERT_EQ(SimpleTeacher.getWinner(), nshogi::core::Black);
}

TEST(ML, SimpleTeacherWinnerWhite) {
    nshogi::ml::SimpleTeacher SimpleTeacher;
    SimpleTeacher.setWinner(nshogi::core::White);
    TEST_ASSERT_EQ(SimpleTeacher.getWinner(), nshogi::core::White);
}

TEST(ML, SimpleTeacherEqualsItself) {
    nshogi::ml::SimpleTeacher SimpleTeacher;
    TEST_ASSERT_TRUE(SimpleTeacher.equals(SimpleTeacher));
}

TEST(ML, SimpleTeacherSaveAndLoad) {
    nshogi::ml::SimpleTeacher T1;

    const int N = 100;

    const std::string Path = std::filesystem::temp_directory_path().string() +
                             "/simple_teacher_test.bin";

    for (int I = 0; I < N; ++I) {
        T1.corruptMyself();

        {
            std::ofstream Ofs(Path, std::ios::out | std::ios::binary);
            nshogi::io::file::save(Ofs, T1);
        }

        {
            std::ifstream Ifs(Path, std::ios::in | std::ios::binary);

            auto T2 = nshogi::io::file::load<nshogi::ml::SimpleTeacher>(Ifs);

            TEST_ASSERT_TRUE(T1.equals(T2));
            TEST_ASSERT_TRUE(T2.equals(T1));
        }
    }
}

TEST(ML, PermutationGenerator) {
    for (std::size_t N = 1; N < 1000; ++N) {
        std::vector<bool> Filled(N, false);
        for (uint64_t Seed = 0; Seed < 10; ++Seed) {
            nshogi::ml::utils::PermutationGenerator PG(Seed, N);
            for (uint64_t I = 0; I < (uint64_t)N; ++I) {
                Filled[PG(I)] = true;
            }

            for (uint64_t I = 0; I < (uint64_t)N; ++I) {
                TEST_ASSERT_TRUE(Filled[I]);
            }
        }
    }
}

TEST(ML, MoveIndexChannelsFirstAndLast) {
    auto State = nshogi::core::StateBuilder::getInitialState();

    while (State.getPly() < 1024) {
        const auto Moves =
            nshogi::core::MoveGenerator::generateLegalMoves(State);

        if (Moves.size() == 0) {
            break;
        }

        for (const auto& Move : Moves) {
            const auto IndexChannelsFirst =
                nshogi::ml::getMoveIndex<true>(State.getSideToMove(), Move);
            const auto IndexChannelsLast =
                nshogi::ml::getMoveIndex<false>(State.getSideToMove(), Move);

            const auto To = IndexChannelsFirst % 81;
            const auto Channel = IndexChannelsFirst / 81;

            TEST_ASSERT_EQ(IndexChannelsLast, To * 27 + Channel);
        }

        State.doMove(Moves[0]);
    }
}

TEST(ML, ChannelsFirstComptimeAndRuntime) {
    auto State = nshogi::core::StateBuilder::getInitialState();

    using Features = nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_Black,
        nshogi::ml::FeatureType::FT_White,
        nshogi::ml::FeatureType::FT_MyPawn,
        nshogi::ml::FeatureType::FT_OpPawn,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_OpKing>;

    std::vector<nshogi::ml::FeatureType> FeatureTypes = {
        nshogi::ml::FeatureType::FT_Black,
        nshogi::ml::FeatureType::FT_White,
        nshogi::ml::FeatureType::FT_MyPawn,
        nshogi::ml::FeatureType::FT_OpPawn,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_OpKing
    };

    nshogi::core::StateConfig StateConfig;

    while (State.getPly() < 1024) {
        const auto Moves =
            nshogi::core::MoveGenerator::generateLegalMoves(State);

        if (Moves.size() == 0) {
            break;
        }

        Features FSC(State, StateConfig);
        nshogi::ml::FeatureStackRuntime FSR(
            FeatureTypes, State, StateConfig);

        const auto FSCExtracted = FSC.extract<nshogi::core::IterateOrder::Fastest, true>();
        const auto FSRExtracted = FSR.extract<nshogi::core::IterateOrder::Fastest, true>();

        for (std::size_t I = 0; I < FSCExtracted.size(); ++I) {
            TEST_ASSERT_FLOAT_EQ(FSCExtracted[I], FSRExtracted[I], 1e-6f);
        }

        State.doMove(Moves[0]);
    }
}

TEST(ML, ChannelsLastComptimeAndRuntime) {
    auto State = nshogi::core::StateBuilder::getInitialState();

    using Features = nshogi::ml::FeatureStackComptime<
        nshogi::ml::FeatureType::FT_Black,
        nshogi::ml::FeatureType::FT_White,
        nshogi::ml::FeatureType::FT_MyPawn,
        nshogi::ml::FeatureType::FT_OpPawn,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_OpKing>;

    std::vector<nshogi::ml::FeatureType> FeatureTypes = {
        nshogi::ml::FeatureType::FT_Black,
        nshogi::ml::FeatureType::FT_White,
        nshogi::ml::FeatureType::FT_MyPawn,
        nshogi::ml::FeatureType::FT_OpPawn,
        nshogi::ml::FeatureType::FT_MyKing,
        nshogi::ml::FeatureType::FT_OpKing
    };

    nshogi::core::StateConfig StateConfig;

    while (State.getPly() < 1024) {
        const auto Moves =
            nshogi::core::MoveGenerator::generateLegalMoves(State);

        if (Moves.size() == 0) {
            break;
        }

        Features FSC(State, StateConfig);
        nshogi::ml::FeatureStackRuntime FSR(
            FeatureTypes, State, StateConfig);

        const auto FSCExtracted = FSC.extract<nshogi::core::IterateOrder::Fastest, false>();
        const auto FSRExtracted = FSR.extract<nshogi::core::IterateOrder::Fastest, false>();

        for (std::size_t I = 0; I < FSCExtracted.size(); ++I) {
            TEST_ASSERT_FLOAT_EQ(FSCExtracted[I], FSRExtracted[I], 1e-6f);
        }

        State.doMove(Moves[0]);
    }
}
