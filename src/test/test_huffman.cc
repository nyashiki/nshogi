//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"
#include "../core/positionbuilder.h"
#include "../io/sfen.h"
#include "../core/huffman.h"
#include "../core/movegenerator.h"

#include <random>
#include <cstring>

namespace {

bool testPositionEquality(const nshogi::core::Position& Pos) {
    nshogi::core::HuffmanCode HC = nshogi::core::HuffmanCode::encode(Pos);
    const auto DecodedPos = nshogi::core::HuffmanCode::decode(HC);

    return Pos.equals(DecodedPos);
}

bool testHuffmanEquality(const nshogi::core::Position& Pos) {
    nshogi::core::HuffmanCode HC = nshogi::core::HuffmanCode::encode(Pos);
    nshogi::core::HuffmanCode HC2 = nshogi::core::HuffmanCode::encode(Pos);

    return (HC == HC2) && !(HC != HC2);
}

bool testDiffHuffmanCodeOneMoveAfter(const nshogi::core::Position& Pos) {
    nshogi::core::State S = nshogi::io::sfen::StateBuilder::newState(
            nshogi::io::sfen::positionToSfen(Pos));

    nshogi::core::HuffmanCode OriginalHC = nshogi::core::HuffmanCode::encode(S.getPosition());

    const auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(S);

    for (const auto& Move : Moves) {
        S.doMove(Move);

        const auto HC = nshogi::core::HuffmanCode::encode(S.getPosition());
        if ((OriginalHC == HC) || !(OriginalHC != HC)) {
            return false;
        }

        S.undoMove();
    }

    return true;
}

} // namespace

TEST(Huffman, Size) {
    nshogi::core::HuffmanCode HC = nshogi::core::HuffmanCode::zero();
    TEST_ASSERT_EQ(HC.size(), 4 * sizeof(uint64_t));
    TEST_ASSERT_EQ(sizeof(nshogi::core::HuffmanCode), 4 * sizeof(uint64_t));
}

TEST(Huffman, Constructor) {
    const int N = 200;
    std::mt19937_64 mt(20240216);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::getInitialState();

        for (int Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];

            State.doMove(RandomMove);

            nshogi::core::HuffmanCode HC1 = nshogi::core::HuffmanCode::encode(State.getPosition());
            nshogi::core::HuffmanCode HC2(HC1);

            TEST_ASSERT_EQ(HC1, HC2);
        }
    }
}

TEST(Huffman, CopyConstructor) {
    const int N = 200;
    std::mt19937_64 mt(20240216);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::getInitialState();

        for (int Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];

            State.doMove(RandomMove);

            nshogi::core::HuffmanCode HC1 = nshogi::core::HuffmanCode::encode(State.getPosition());
            nshogi::core::HuffmanCode HC2 = nshogi::core::HuffmanCode::zero();
            HC2 = HC1;

            TEST_ASSERT_EQ(HC1, HC2);
        }
    }
}

TEST(Huffman, InitialPosition) {
    nshogi::core::Position Pos = nshogi::core::PositionBuilder::getInitialPosition();
    TEST_ASSERT_TRUE(testPositionEquality(Pos));
    TEST_ASSERT_TRUE(testHuffmanEquality(Pos));
    TEST_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

TEST(Huffman, AllStandsBlack) {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("4k4/9/9/9/9/9/9/9/4K4 b 2R2B4G4S4N4L18P 1");
    TEST_ASSERT_TRUE(testPositionEquality(Pos));
    TEST_ASSERT_TRUE(testHuffmanEquality(Pos));
    TEST_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

TEST(Huffman, AllStandsWhite) {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("4k4/9/9/9/9/9/9/9/4K4 b 2r2b4g4s4n4l18p 1");
    TEST_ASSERT_TRUE(testPositionEquality(Pos));
    TEST_ASSERT_TRUE(testHuffmanEquality(Pos));
    TEST_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

TEST(Huffman, Handmade1) {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1");
    TEST_ASSERT_TRUE(testPositionEquality(Pos));
    TEST_ASSERT_TRUE(testHuffmanEquality(Pos));
    TEST_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

TEST(Huffman, RandomMove) {
    const int N = 200;
    std::mt19937_64 mt(20240216);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::getInitialState();

        for (int Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];

            State.doMove(RandomMove);

            TEST_ASSERT_TRUE(testPositionEquality(State.getPosition()));
            TEST_ASSERT_TRUE(testHuffmanEquality(State.getPosition()));
        }
    }
}

TEST(Huffman, Memcpy) {
    const int N = 200;
    std::mt19937_64 mt(20240522);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::getInitialState();

        for (int Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];

            State.doMove(RandomMove);

            uint64_t Codes[4];
            const auto HC = nshogi::core::HuffmanCode::encode(State.getPosition());
            std::memcpy(reinterpret_cast<char*>(Codes), HC.data(), HC.size());
            const auto HC2 = nshogi::core::HuffmanCode(Codes[3], Codes[2], Codes[1], Codes[0]);

            TEST_ASSERT_EQ(std::memcmp(HC.data(), HC2.data(), HC.size()), 0);
        }
    }
}
