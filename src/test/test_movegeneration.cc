#include "common.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <random>

#include "../core/movegenerator.h"
#include "../core/internal/bitboard.h"
#include "../io/sfen.h"

namespace {

template <bool WilyPromote>
void testMoveGenerationNum(const std::string& Sfen, size_t Count) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);
    const auto Moves =
        nshogi::core::MoveGenerator::generateLegalMoves<WilyPromote>(
            State);

    TEST_ASSERT_EQ(Moves.size(), Count);
}

} // namespace

TEST(MoveGeneration, InitialPosition) {
    testMoveGenerationNum<true>(
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1", 30);
}

TEST(MoveGeneration, MaxMoveCountPosition) {
    testMoveGenerationNum<false>(
        "R8/2K1S1SSk/4B4/9/9/9/9/9/1L1L1L3 b RBGSNLP3g3n17p 1", 593);
}

TEST(MoveGeneration, MatsuriPosition) {
    testMoveGenerationNum<true>(
        "l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1",
        198);
}

TEST(MoveGeneration, EvasionPosition1) {
    nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::newState("4l4/9/9/9/9/9/9/9/4K4 b P 1");
    const auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(State);

    TEST_ASSERT_EQ(Moves.size(), (std::size_t)11);
}

TEST(MoveGeneration, FindMoves1) {
    const std::string Sfens[] = {
        "1g1f", "2g2f", "3g3f", "4g4f", "5g5f", "6g6f", "7g7f", "8g8f", "9g9f",
        "3i3h", "3i4h", "7i6h", "7i7h", "4i3h", "4i4h", "4i5h", "6i5h", "6i6h",
        "6i7h", "5i4h", "5i5h", "5i6h", "1i1h", "9i9h", "2h1h", "2h3h", "2h4h",
        "2h5h", "2h6h", "2h7h",
    };

    nshogi::core::State State = nshogi::core::StateBuilder::getInitialState();
    const auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(State);

    for (const auto& Sfen : Sfens) {
        nshogi::core::Move32 Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), Sfen);
        TEST_ASSERT_TRUE(Moves.find(Move) != Moves.end());
    }

    nshogi::core::Move32 IllegalMove = nshogi::io::sfen::sfenToMove32(State.getPosition(), "G*5e");
    TEST_ASSERT_TRUE(Moves.find(IllegalMove) == Moves.end());
}

TEST(MoveGeneration, ExamplePositions) {
    const uint16_t  NumTestRandomMove = 2;
    std::mt19937_64 mt(20230704);

    std::ifstream Ifs("./res/test/legal-move-examples.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Line);

        uint16_t NumMoves = 0;

        Ifs >> NumMoves;

        std::vector<nshogi::core::Move32> LegalMoves;
        LegalMoves.reserve(NumMoves);

        for (uint16_t I = 0; I < NumMoves; ++I) {
            std::string SfenMove;
            Ifs >> SfenMove;

            LegalMoves.emplace_back(nshogi::io::sfen::sfenToMove32(State.getPosition(), SfenMove));
        }

        Ifs >> std::ws;

        auto GeneratedMoves = nshogi::core::MoveGenerator::generateLegalMoves(State);

        TEST_ASSERT_EQ(GeneratedMoves.size(), LegalMoves.size());

        for (uint16_t I = 0; I < NumTestRandomMove; ++I) {
            const auto& RandomLegalMove = LegalMoves[mt() % LegalMoves.size()];
            TEST_ASSERT_TRUE(GeneratedMoves.find(RandomLegalMove) != GeneratedMoves.end());
        }
    }
}

TEST(MoveGeneration, CheckPositions) {
    std::mt19937_64 mt(20230704);

    std::ifstream Ifs("./res/test/legal-check-examples.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Line);

        uint16_t NumMoves = 0;

        Ifs >> NumMoves;

        std::vector<nshogi::core::Move32> LegalMoves;
        LegalMoves.reserve(NumMoves);

        for (uint16_t I = 0; I < NumMoves; ++I) {
            std::string SfenMove;
            Ifs >> SfenMove;

            auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), SfenMove);

            // Filtering-out some moves if necessary.
            // No-promote pawn moves.
            if (!Move.drop() && Move.pieceType() == nshogi::core::PTK_Pawn && !Move.promote()) {
                auto PromotableBB = (State.getPosition().sideToMove() == nshogi::core::Black)
                                        ? nshogi::core::internal::bitboard::PromotableBB[nshogi::core::Black]
                                        : nshogi::core::internal::bitboard::PromotableBB[nshogi::core::White];

                if (PromotableBB.isSet(Move.to())) {
                    continue;
                }
            }

            // No-promote lance moves.
            if (!Move.drop() && Move.pieceType() == nshogi::core::PTK_Lance && !Move.promote()) {
                auto FirstAndSecondFurthestBB = (State.getPosition().sideToMove() == nshogi::core::Black)
                                        ? nshogi::core::internal::bitboard::FirstAndSecondFurthestBB[nshogi::core::Black]
                                        : nshogi::core::internal::bitboard::FirstAndSecondFurthestBB[nshogi::core::White];

                if (FirstAndSecondFurthestBB.isSet(Move.to())) {
                    continue;
                }
            }

            LegalMoves.emplace_back(Move);
        }

        Ifs >> std::ws;

        auto GeneratedMoves = nshogi::core::MoveGenerator::generateLegalCheckMoves(State);
        TEST_ASSERT_EQ(GeneratedMoves.size(), LegalMoves.size());

        for (uint16_t I = 0; I < LegalMoves.size(); ++I) {
            TEST_ASSERT_TRUE(GeneratedMoves.find(LegalMoves[I]) != GeneratedMoves.end());
        }
    }
}
