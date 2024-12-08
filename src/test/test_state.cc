#include "common.h"

#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"
#include "../core/movegenerator.h"
#include "../core/internal/stateadapter.h"
#include "../core/internal/hash.h"
#include "../io/sfen.h"

#include <iostream>
#include <random>
#include <map>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

template <nshogi::core::Color C>
void testDefenderImpl(const std::string& Sfen, nshogi::core::Square Defender) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);
    if (Defender == nshogi::core::SqInvalid) {
        TEST_ASSERT_TRUE(Adapter->getDefendingOpponentSliderBB<C>().isZero());
    } else {
        TEST_ASSERT_TRUE(Adapter->getDefendingOpponentSliderBB<C>().isSet(Defender));
    }
}

void testRecomputeHelper(const nshogi::core::State& State) {
    nshogi::core::State DummyState =
        nshogi::core::StateBuilder::newState(State.getPosition());

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);
    nshogi::core::internal::ImmutableStateAdapter DummyAdapter(DummyState);

    TEST_ASSERT_EQ(Adapter->getKingSquare<nshogi::core::Black>(),
                    DummyAdapter->getKingSquare<nshogi::core::Black>());

    TEST_ASSERT_EQ(Adapter->getKingSquare<nshogi::core::White>(),
                    DummyAdapter->getKingSquare<nshogi::core::White>());

    TEST_ASSERT_EQ(Adapter->getDefendingOpponentSliderBB<nshogi::core::Black>(),
                    DummyAdapter->getDefendingOpponentSliderBB<nshogi::core::Black>());

    TEST_ASSERT_EQ(Adapter->getDefendingOpponentSliderBB<nshogi::core::White>(),
                    DummyAdapter->getDefendingOpponentSliderBB<nshogi::core::White>());

    TEST_ASSERT_EQ(Adapter->getCheckerBB(), DummyAdapter->getCheckerBB());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::Black>(),
                    DummyAdapter->getBitboard<nshogi::core::Black>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::White>(),
                    DummyAdapter->getBitboard<nshogi::core::White>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Pawn>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Pawn>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Lance>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Lance>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Knight>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Knight>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Silver>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Silver>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Gold>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Gold>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_King>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_King>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Bishop>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Bishop>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_Rook>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_Rook>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProPawn>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProPawn>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProLance>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProLance>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProKnight>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProKnight>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProSilver>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProSilver>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProBishop>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProBishop>());

    TEST_ASSERT_EQ(Adapter->getBitboard<nshogi::core::PTK_ProRook>(),
                    DummyAdapter->getBitboard<nshogi::core::PTK_ProRook>());
}

void testDoMoveAndUndoMove(nshogi::core::State& State,
                           nshogi::core::Move32 Move) {
    const std::string OriginalSfen =
        nshogi::io::sfen::positionToSfen(State.getPosition());

    State.doMove(Move);
    State.undoMove();

    const std::string DoMoveAndUndoMoveSfen =
        nshogi::io::sfen::positionToSfen(State.getPosition());
    TEST_ASSERT_STREQ(OriginalSfen.c_str(), DoMoveAndUndoMoveSfen.c_str());
    testRecomputeHelper(State);
}

void testHashDoMoveAndUndoMove(nshogi::core::State& State,
                           nshogi::core::Move32 Move) {

    nshogi::core::internal::ImmutableStateAdapter Adapter(State);

    uint64_t OriginalHashValue = Adapter->getHash();

    State.doMove(Move);
    State.undoMove();

    TEST_ASSERT_EQ(OriginalHashValue, Adapter->getHash());
}

void testPieceScore(const nshogi::core::State& S, uint8_t BlackScore, uint8_t WhiteScore) {
    nshogi::core::internal::ImmutableStateAdapter Adapter(S);
    const uint8_t BlackScoreComputed = Adapter->computePieceScore<nshogi::core::Black>(5, 1, false);
    const uint8_t WhiteScoreComputed = Adapter->computePieceScore<nshogi::core::White>(5, 1, false);

    if (BlackScoreComputed != BlackScore) {
        std::cout << (int)BlackScoreComputed << ", " << (int)BlackScore << std::endl;
    }
    if (WhiteScoreComputed != WhiteScore) {
        std::cout << (int)WhiteScoreComputed << ", " << (int)WhiteScore << std::endl;
    }

    TEST_ASSERT_EQ(BlackScoreComputed, BlackScore);
    TEST_ASSERT_EQ(WhiteScoreComputed, WhiteScore);
}

void testEntryingScore(const nshogi::core::State& S, uint8_t BlackScore, uint8_t WhiteScore) {
    nshogi::core::internal::ImmutableStateAdapter Adapter(S);
    TEST_ASSERT_EQ(Adapter->computeDeclarationScore<nshogi::core::Black>(), BlackScore);
    TEST_ASSERT_EQ(Adapter->computeDeclarationScore<nshogi::core::White>(), WhiteScore);
}

} // namespace

TEST(State, Defender) {
    testDefenderImpl<nshogi::core::Black>(
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
        nshogi::core::SqInvalid);
    testDefenderImpl<nshogi::core::White>(
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
        nshogi::core::SqInvalid);
    testDefenderImpl<nshogi::core::Black>("l8/9/9/9/9/9/9/P8/K8 b - 1",
                                          nshogi::core::Sq9H);
    testDefenderImpl<nshogi::core::Black>("1l7/9/9/9/9/9/1G7/9/1K7 b - 1",
                                          nshogi::core::Sq8G);
    testDefenderImpl<nshogi::core::White>("k8/p8/9/9/9/9/9/9/L8 b - 1",
                                          nshogi::core::Sq9B);
    testDefenderImpl<nshogi::core::White>("9/2k6/2p6/2L6/9/9/9/9/L8 b - 1",
                                          nshogi::core::Sq7C);
    testDefenderImpl<nshogi::core::Black>("9/9/KP6r/9/9/9/9/9/9 b - 1",
                                          nshogi::core::Sq8C);
    testDefenderImpl<nshogi::core::Black>("9/9/KPP5r/9/9/9/9/9/9 b - 1",
                                          nshogi::core::SqInvalid);
    testDefenderImpl<nshogi::core::White>("4p4/5k3/9/9/B8/9/9/9/9 b - 1",
                                          nshogi::core::SqInvalid);
    testDefenderImpl<nshogi::core::White>("p7k/9/9/9/9/9/9/9/R8 b - 1",
                                          nshogi::core::SqInvalid);
    testDefenderImpl<nshogi::core::Black>("8b/7R1/9/9/9/9/9/9/K8 b - 1",
                                          nshogi::core::Sq2B);
    testDefenderImpl<nshogi::core::Black>("8b/9/6R2/9/9/9/9/9/K8 b - 1",
                                          nshogi::core::Sq3C);
    testDefenderImpl<nshogi::core::Black>("8b/9/9/5R3/9/9/9/9/K8 b - 1",
                                          nshogi::core::Sq4D);
    testDefenderImpl<nshogi::core::Black>("8b/9/9/9/4R4/9/9/9/K8 b - 1",
                                          nshogi::core::Sq5E);
    testDefenderImpl<nshogi::core::Black>("8b/9/9/9/9/3R5/9/9/K8 b - 1",
                                          nshogi::core::Sq6F);
    testDefenderImpl<nshogi::core::Black>("8b/9/9/9/9/9/2R6/9/K8 b - 1",
                                          nshogi::core::Sq7G);
    testDefenderImpl<nshogi::core::Black>("8b/9/9/9/9/9/9/1R7/K8 b - 1",
                                          nshogi::core::Sq8H);
    testDefenderImpl<nshogi::core::White>("B8/1b7/9/9/9/9/9/9/8k b - 1",
                                          nshogi::core::Sq8B);
    testDefenderImpl<nshogi::core::White>("B8/9/2b6/9/9/9/9/9/8k b - 1",
                                          nshogi::core::Sq7C);
    testDefenderImpl<nshogi::core::White>("B8/9/9/3b5/9/9/9/9/8k b - 1",
                                          nshogi::core::Sq6D);
    testDefenderImpl<nshogi::core::White>("B8/9/9/9/4b4/9/9/9/8k b - 1",
                                          nshogi::core::Sq5E);
    testDefenderImpl<nshogi::core::White>("B8/9/9/9/9/5b3/9/9/8k b - 1",
                                          nshogi::core::Sq4F);
    testDefenderImpl<nshogi::core::White>("B8/9/9/9/9/9/6b2/9/8k b - 1",
                                          nshogi::core::Sq3G);
    testDefenderImpl<nshogi::core::White>("B8/9/9/9/9/9/9/7b1/8k b - 1",
                                          nshogi::core::Sq2H);
    testDefenderImpl<nshogi::core::Black>("3k1l3/3p5/9/9/9/9/9/5P3/3L1K3 b - 1",
                                          nshogi::core::Sq4H);
    testDefenderImpl<nshogi::core::White>("3k1l3/3p5/9/9/9/9/9/5P3/3L1K3 b - 1",
                                          nshogi::core::Sq6B);
}

TEST(State, Move1) {
    nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::getInitialState();

    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "7g7f");
    State.doMove(Move);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL w - 1");
}

TEST(State, Move2) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5b1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL b - 1");

    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "8h2b+");
    State.doMove(Move);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 1");
}

TEST(State, Move3) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkg1nl/1r5s1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL b Bb 1");

    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "B*4e");
    State.doMove(Move);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkg1nl/1r5s1/pppppp1pp/6p2/5B3/2P6/PP1PPPPPP/7R1/LNSGKGSNL w b 1");
}

TEST(State, Move4) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 1");

    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "3a2b");
    State.doMove(Move);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkg1nl/1r5s1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL b Bb 1");
}

TEST(State, Ply1) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 1");
    TEST_ASSERT_EQ(State.getPly(), 0);
}

TEST(State, Ply2) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 1 moves 3a2b");
    TEST_ASSERT_EQ(State.getPly(), 1);
}

TEST(State, Ply3) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 127");
    TEST_ASSERT_EQ(State.getPly(), 126);
}

TEST(State, Ply4) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 127 moves 3a2b");
    TEST_ASSERT_EQ(State.getPly(), 127);
}

TEST(State, DoAndUndo1) {
    nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::getInitialState();
    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "7g7f");

    testDoMoveAndUndoMove(State, Move);
}

TEST(State, DoAndUndo2) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5b1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL b - 1");
    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "8h2b+");

    testDoMoveAndUndoMove(State, Move);
}

TEST(State, DoAndUndo3) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkg1nl/1r5s1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL b Bb 1");
    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "B*4e");

    testDoMoveAndUndoMove(State, Move);
}

TEST(State, DoAndUndo4) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "lnsgkgsnl/1r5+B1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w B 1");
    auto Move = nshogi::io::sfen::sfenToMove32(State.getPosition(), "3a2b");

    testDoMoveAndUndoMove(State, Move);
}

TEST(State, HelperRandom) {
    const int N = 1000;
    std::mt19937_64 mt(20230620);

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

            TEST_ASSERT_EQ(Ply + 1, State.getPly());
            testRecomputeHelper(State);
        }
    }
}

TEST(State, DoAndUndoRandom) {
    const int N = 100;
    std::mt19937_64 mt(20230620);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 512; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            for (const auto& Move : Moves) {
                testDoMoveAndUndoMove(State, Move);

                const auto M16 = nshogi::core::Move16(Move);
                const auto RestoredM32 = State.getMove32FromMove16(M16);

                TEST_ASSERT_EQ(Move, RestoredM32);
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(State, HashDoAndUndoRandom) {
    const int N = 100;
    std::mt19937_64 mt(20230622);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 256; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            for (const auto& Move : Moves) {
                testHashDoMoveAndUndoMove(State, Move);
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(State, GetLastMoveRandom) {
    const int N = 100;
    std::mt19937_64 mt(20230825);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 256; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);

            TEST_ASSERT_EQ(State.getLastMove(), RandomMove);
        }
    }
}

TEST(State, HashRecomputeRandom) {
    const int N = 100;
    std::mt19937_64 mt(20230622);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        nshogi::core::internal::ImmutableStateAdapter Adapter(State);

        for (uint16_t Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);

            const std::string CurrentSfen = nshogi::io::sfen::positionToSfen(State.getPosition());
            const auto DummyState = nshogi::io::sfen::StateBuilder::newState(CurrentSfen);
            nshogi::core::internal::ImmutableStateAdapter DummyAdapter(DummyState);

            TEST_ASSERT_EQ(Adapter->getHash(), DummyAdapter->getHash());
        }
    }
}

TEST(State, CloneRandom) {
    const int N = 1000;
    std::mt19937_64 mt(20230730);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();
        nshogi::core::internal::ImmutableStateAdapter Adapter(State);

        for (uint16_t Ply = 0; Ply < 256; ++Ply) {
            const auto StateCloned = State.clone();
            nshogi::core::internal::ImmutableStateAdapter ClonedAdapter(StateCloned);

            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            const auto MovesByCloned =
                nshogi::core::MoveGenerator::generateLegalMoves(StateCloned);

            TEST_ASSERT_EQ(Moves.size(), MovesByCloned.size());
            TEST_ASSERT_EQ(Adapter->getHash(), ClonedAdapter->getHash());

            if (Moves.size() == 0) {
                break;
            }

            for (std::size_t J = 0; J < Moves.size(); ++J) {
                TEST_ASSERT_EQ(Moves[J], MovesByCloned[J]);
            }

            for (const auto& Move : Moves) {
                testHashDoMoveAndUndoMove(State, Move);
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(State, RepetitionHandmade1) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h3h 8b7b 3h2h 7b8b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::Repetition);
}

TEST(State, RepetitionHandmade2) {
    nshogi::core::State State = nshogi::core::StateBuilder::getInitialState();
    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionHandmade3) {
    const std::string Sfen =
      "l3k2Bl/1r1sg4/1l1pps2p/2P1np3/1P4p2/2G1RP1N1/+p1KPP3P/3S5/1+n2G2+bL b GPsn5p 1 moves G*4a "
      "5a6a 4a5a 6a5a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::InferiorRepetition);
}

TEST(State, RepetitionHandmade4) {
    const std::string Sfen =
      "+Bn1g1g2l/2s1ks3/p1Ppppn1p/2+BP3r1/2pN5/1p4ppP/P2gPP3/8K/L5GNL w RL2s3p 1 moves S*2g 1h1g "
      "2g1h+ 1g1h S*2g";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::SuperiorRepetition);
}

TEST(State, RepetitionHandmade5) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7g7f 8c8d 7i6h 3c3d "
      "6h7g 7c7d 2g2f 8d8e 2f2e 8a7c 6i7h 2b3c 8h7i 8e8f 8g8f 7c6e 7g6f 8b8f P*8h 6c6d 5g5f 8f7f "
      "7i4f 7a6b 5f5e P*8f 8i7g 5a4b 4i5h 3a3b 5h6h 6a5b 3i4h 4b3a 3g3f 3c4d 2e2d 2c2d 2h2d P*2c "
      "2d2h 1c1d 5i4i 2a3c 1g1f 6e7g 6h7g 7f7g+ 7h7g N*2d 2h3h G*5f P*2h 5f4f 4g4f 8f8g+ 8h8g 6d6e "
      "6f6e 4d5e 7g6f B*7g 3f3e 7g9i+ 3e3d L*3f N*3g 3f3g+ 3h3g N*3f 6f5e 9i5e 3d3c+ 3b3c G*5f "
      "3f4h+ 4i4h P*3f 3g2g 5e7g P*3d P*6f 5f6f 7g6h P*5h S*3g 2i3g 3f3g+ 2g3g P*3f 3g1g G*3g 4h4i "
      "N*4g S*4h 3g4h 4i4h S*3g 1g3g 3f3g+ 4h3g R*3f 3g4g 3f4f 4g3h 6h5h 3h2i 4f4i+ 2i1h 5h3f S*2g "
      "3f2g 2h2g S*3g R*9h 4i8i 9h5h 8i4i 5h7h 4i8i 7h5h 8i4i 5h7h 4i8i 7h5h";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::Repetition);
}

TEST(State, RepetitionHandmade6) {
    const std::string Sfen = "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::WinRepetition);
}

TEST(State, RepetitionHandmade7) {
    const std::string Sfen = "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::LossRepetition);
}

TEST(State, RepetitionHandmade8) {
    const std::string Sfen = "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);
    State.undoMove();
    State.undoMove();

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionHandmade9) {
    const std::string Sfen = "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    State.undoMove();
    State.doMove(nshogi::io::sfen::sfenToMove32(State.getPosition(), "8c7c"));

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::WinRepetition);
}

TEST(State, RepetitionHandmade10) {
    const std::string Sfen = "9/9/9/9/9/9/1rk6/9/K8 w - 1 moves 8g9g 9i8i 9g8g 8i9i";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::LossRepetition);
}

TEST(State, RepetitionHandmade11) {
    const std::string Sfen = "9/9/9/9/9/9/1rk6/9/K8 w - 1 moves 8g9g 9i8i 9g8g 8i9i 8g9g";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::WinRepetition);
}

TEST(State, RepetitionHandmade12) {
    const std::string Sfen = "9/9/9/9/9/9/1rk6/9/K8 w - 1 moves 8g9g 9i8i 9g8g 8i9i 8g8a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionHandmade13) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 8c8d 2f2e 4a3b "
      "6i7h 8d8e 3i3h 7a7b 9g9f 5a5b 4g4f 9c9d 3h4g 7c7d 7g7f 3c3d 8h7g 2b7g+ 7h7g 8a7c 4g5f 3a2b "
      "2e2d 2c2d 2h2d 2a3c 2d2h 1c1d 1g1f 8b8a 7f7e 8a8d 1f1e P*2f 1e1d 7d7e 7g6f 3d3e 2h2f P*2e "
      "2f2h 8e8f 8g8f 8d8f P*8h 8f8d 5i6i 8d3d 5f4g 3d5d P*7d 5d7d B*5f 7d2d P*7d 7c8e 6f7e P*7g "
      "7e8e 2e2f P*2e 3c2e P*2c 2b3a 8i7g P*7f 7g6e 7f7g+ 2c2b+ 3a2b 7d7c+ 2f2g+ 2h2g 2e3g 2g3g "
      "2d2i+ 4g3h 3e3f 3g3f B*2e 5f4g 2e3f 4g3f 2i1i 7c7b R*8i 6e5c+ 5b5c N*6e 5c5b S*5c 5b4a "
      "3f6c+ 4a3a 6i5h N*5a 6c6d P*6c 6d5e 8i7i+ N*2d N*4d 5c4d 4c4d 2d3b+ 3a3b 5e4d S*4c G*4b "
      "3b2c 4b4c S*3f 4d2b 2c1d P*1e 1d1e P*1f 1i1f S*2d 1e2d 2b3c 2d2e S*3d 2e2f B*4d P*3e 4d3e "
      "2f3e 3c4d 3e2d 4d3c 2d3e";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::LossRepetition);
}

TEST(State, RepetitionHandmade14) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 3c3d 7g7f 8c8d 2f2e 8d8e 2e2d 2c2d 6i7h 2b8h+ 7i8h 8b2b B*7g B*3c 7g3c+ 2a3c B*8c 2d2e 8c5f+ 2b2d 3i3h 2e2f 5f4f 2d2a 4f3f 5a6b 8h7g 9c9d 5i6h 7a8b 3f2f 3a3b P*2b P*2g 2h2g 2a2b 2f5c 6b5c 2g2b+ P*5f 5g5f B*3e 6h5i 5c6b 2b1a 3c4e P*2c 3e4d 2c2b+ 4e5g+ L*6f B*3c 2b3b 3c1a 3b4a 6b7b R*3b 6a6b S*5a 8b7a G*6h 5g6h 7g6h 7c7d 5a6b+ 7a6b N*5d S*5c 5d6b+ 5c6b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(false), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade1) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h3h 8b7b 3h2h 7b8b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade2) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h3h 8b7b 3h2h 7b8b 2h3h 8b7b 3h2h 7b8b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade3) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h3h 8b7b 3h2h 7b8b 2h3h 8b7b 3h2h 7b8b 2h3h 8b7b 3h2h 7b8b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::Repetition);
}

TEST(State, RepetitionStrictHandmade4) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade5) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a4a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade6) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a7a 8c7c";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::WinRepetition);
}

TEST(State, RepetitionStrictHandmade7) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade8) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a7a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::NoRepetition);
}

TEST(State, RepetitionStrictHandmade9) {
    const std::string Sfen =
        "2k6/9/KR7/9/9/9/9/9/9 b - 1 moves 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a7a 8c7c 7a8a 7c8c 8a7a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::LossRepetition);
}

TEST(State, RepetitionStrictHandmade10) {
    const std::string Sfen =
      "l3k2Bl/1r1sg4/1l1pps2p/2P1np3/1P4p2/2G1RP1N1/+p1KPP3P/3S5/1+n2G2+bL b GPsn5p 1 moves G*4a "
      "5a6a 4a5a 6a5a";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::InferiorRepetition);
}

TEST(State, RepetitionStrictHandmade11) {
    const std::string Sfen =
      "+Bn1g1g2l/2s1ks3/p1Ppppn1p/2+BP3r1/2pN5/1p4ppP/P2gPP3/8K/L5GNL w RL2s3p 1 moves S*2g 1h1g "
      "2g1h+ 1g1h S*2g";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getRepetitionStatus(true), nshogi::core::RepetitionStatus::SuperiorRepetition);
}

TEST(State, PlyOffset1) {
    const std::string Sfen =
      "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 100";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_EQ(State.getPly(), 100 - 1);
}

TEST(State, PlyOffset2) {
    nshogi::core::State State =
        nshogi::core::StateBuilder::newState(nshogi::core::PositionBuilder::getInitialPosition(), 100);

    TEST_ASSERT_EQ(State.getPly(), 100);
}

TEST(State, EqualsItself) {
    const int N = 100;
    std::mt19937_64 mt(20230622);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            TEST_ASSERT_TRUE(State.getPosition().equals(State.getPosition()));

            if (Moves.size() == 0) {
                break;
            }

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);
        }
    }
}

TEST(State, NotEqualsOneMove) {
    const int N = 100;
    std::mt19937_64 mt(20230622);

    for (int I = 0; I < N; ++I) {
        nshogi::core::State State =
            nshogi::core::StateBuilder::getInitialState();

        for (uint16_t Ply = 0; Ply < 1024; ++Ply) {
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            if (Moves.size() == 0) {
                break;
            }

            nshogi::core::Position Pos(State.getPosition());

            const auto RandomMove = Moves[mt() % Moves.size()];
            State.doMove(RandomMove);

            TEST_ASSERT_FALSE(Pos.equals(State.getPosition()));
            TEST_ASSERT_FALSE(State.getPosition().equals(Pos));
        }
    }
}

TEST(State, EnteringScoreInitialPosition) {
    auto State = nshogi::core::StateBuilder::getInitialState();
    testEntryingScore(State, 0, 0);
}

TEST(State, PieceScoreExamplePositions) {
    std::map<std::string, std::pair<uint8_t, uint8_t>> TestCases = {
        {"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1", {27, 27}},
        {"2BR4K/1GBG2+P+P+P/3GN4/9/9/9/9/9/6k2 b RG4S3N4L15P 1", {54, 0}},
        {"8K/1G1G2+P+P+P/3GN1PPP/9/9/9/9/9/6k2 b 2r2bg4s3n4l12p 1", {10, 44}},
        {"+PK2+S4/1PpP+BP3/2+P1+L4/N4p3/Pp4+rp1/7sk/9/6+s1+p/9 b B4GS3N3L4Pr3p 1", {34, 20}},
        {"+PK2+S4/1PpP1P+r2/2+P1+L4/N4p3/Pp5k1/7s1/9/6+s1+p/9 b B4GS3N3L4Prb4p 1", {29, 25}},
    };

    for (const auto& [K, V] : TestCases) {
        auto State = nshogi::io::sfen::StateBuilder::newState(K);
        testPieceScore(State, V.first, V.second);
    }
}

TEST(State, EnteringScoreExamplePositions) {
    std::map<std::string, std::pair<uint8_t, uint8_t>> TestCases = {
        {"2BR4K/1GBG2+P+P+P/3GN4/9/9/9/9/9/6k2 b RG4S3N4L15P 1", {54, 0}},
        {"8K/1G1G2+P+P+P/3GN1PPP/9/9/9/9/9/6k2 b 2r2bg4s3n4l12p 1", {10, 44}},
        {"+PK2+S4/1PpP+BP3/2+P1+L4/N4p3/Pp4+rp1/7sk/9/6+s1+p/9 b B4GS3N3L4Pr3p 1", {32, 10}},
        {"+PK2+S4/1PpP1P+r2/2+P1+L4/N4p3/Pp5k1/7s1/9/6+s1+p/9 b B4GS3N3L4Prb4p 1", {27, 16}},
    };

    for (const auto& [K, V] : TestCases) {
        auto State = nshogi::io::sfen::StateBuilder::newState(K);
        testEntryingScore(State, V.first, V.second);
    }
}

TEST(State, DeclarationExamplePositions) {
    std::map<std::string, bool> TestCases = {
        {"2BR4K/1GBG2+P+P+P/3GN4/9/9/9/9/9/6k2 b RG4S3N4L15P 1", true},
        {"2BR4K/1GBG2+P+P+P/3GN4/9/9/9/9/9/6k2 b RG4S3N4L15P 1", false},
        {"8K/1G1G2+P+P+P/3GN1PPP/9/9/9/9/9/6k2 w 2r2bg4s3n4l12p 1", false},
        {"8K/1G1G2+P+P+P/3GN1PPP/9/9/9/9/9/6k2 w 2r2bg4s3n4l12p 1", false},
    };

    for (const auto& [K, V] : TestCases) {
        auto State = nshogi::io::sfen::StateBuilder::newState(K);
        TEST_ASSERT_EQ(State.canDeclare(), V);
    }
}
