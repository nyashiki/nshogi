//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"

#include "../core/movegenerator.h"
#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"
#include "../io/sfen.h"
#include "../solver/dfpn.h"
#include "../solver/dfs.h"
#include "../solver/mate1ply.h"

#include <algorithm>
#include <fstream>
#include <random>

namespace {

void assertMateMove(nshogi::core::State& State, nshogi::core::Move32 Move) {
    const auto LegalMoves =
        nshogi::core::MoveGenerator::generateLegalMoves<false>(State);
    TEST_ASSERT_TRUE(std::find(LegalMoves.begin(), LegalMoves.end(), Move) !=
                     LegalMoves.end());
    const auto Hash = State.getHash();
    State.doMove(Move);
    TEST_ASSERT_TRUE(State.isInCheck());
    TEST_ASSERT_TRUE(
        nshogi::core::MoveGenerator::generateLegalMoves<false>(State).size() == 0);
    State.undoMove();
    TEST_ASSERT_EQ(State.getHash(), Hash);
}

} // namespace

TEST(Mate1Ply, Handmade1) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "G*5b");
}

TEST(Mate1Ply, Handmade2) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b S 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "S*5b");
}

TEST(Mate1Ply, Handmade3) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gkg3/9/3pG4/9/9/9/9/9/4K4 b N 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "N*4c");
}

TEST(Mate1Ply, Handmade4) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gkg3/9/4Gp3/9/9/9/9/9/4K4 b N 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "N*6c");
}

TEST(Mate1Ply, Handmade5) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3sk4/9/4G4/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "G*4b");
}

TEST(Mate1Ply, Handmade6) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4G4/9/9/9/9/9/4K4 b B 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "B*6b");
}

TEST(Mate1Ply, Handmade7) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4P4/9/9/9/9/9/4K4 b R 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "R*5b");
}

TEST(Mate1Ply, Handmade8) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4S4/9/9/9/9/9/4K4 b L 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "L*5b");
}

TEST(Mate1Ply, Handmade9) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/9/9/9/9/4g4/9/4K4 w g 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "G*5h");
}

TEST(Mate1Ply, Handmade10) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/3B5/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "G*5b");
}

TEST(Mate1Ply, Handmade11) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/2B6/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "5c5b");
}

TEST(Mate1Ply, Handmade12) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gk4/9/4SG3/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "4c4b");
}

TEST(Mate1Ply, Handmade13) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/4L4/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "5c5b");
}

TEST(Mate1Ply, Handmade14) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4kg1RR/3s5/9/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "2a4a+");
}

TEST(Mate1Ply, Handmade15) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "resign");
}

TEST(Mate1Ply, Handmade16) {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4kg2R/3s5/9/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "resign");
}

TEST(Mate1Ply, Handmade17) {
    nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::newState("4k4/9/9/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen =
        nshogi::io::sfen::move32ToSfen(CheckmateMove);

    TEST_ASSERT_STREQ(CheckmateMoveSfen.c_str(), "resign");
}

TEST(Mate1Ply, Problems) {
    std::ifstream Ifs("./res/test/mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::mate1ply::solve(State);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
        assertMateMove(State, CheckmateMove);
    }
}

TEST(Mate1Ply, KnightDropWithoutOtherAttacks) {
    // A knight drop does not need support. All escape squares are occupied
    // by the defending side, so no existing attack near its king is needed.
    for (const char* Sfen : {
             "3lkl3/3npn3/9/9/9/9/9/9/4K4 b N 1",
             "4k4/9/9/9/9/9/9/3NPN3/3LKL3 w n 1"}) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Sfen);
        const auto Move = nshogi::solver::mate1ply::solve(State);
        TEST_ASSERT_FALSE(Move.isNone());
        TEST_ASSERT_TRUE(Move.drop());
        assertMateMove(State, Move);
    }
}

TEST(Mate1Ply, LanceChecksMustNotWrapAtEdges) {
    // A reverse white-lance attack from a king on 7a crosses unused bit 63.
    // That padding bit must not be interpreted as the real square 8i.
    for (const char* Sfen : {
             "p+PK+NG4/1+L+PR+BG+P1p/1G7/6+S2/4p2P1/PPp1+lp2L/1pkn1P1p1/"
             "b2p2+r1S/+s+sg+n1+n1+p+p w Pl 1",
             "G+LK+P2s2/+P+P+R2s1p1/2bP5/3pk2+B1/1p2sPP1R/4gp1SP/"
             "Ng+p2N2N/L5+p1L/PP+p4NG w 2Pl 1",
             // The reverse black-lance direction from 6i wraps to 5a.
             "1+N1+P1G3/n+PPP2b1p/rNs1Lp1SR/3pP+S1PG/pp2plGp1/1+pp2+BP2/"
             "9/1+ng2K2+l/1P+pk2+lS1 b - 1"}) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Sfen);
        TEST_ASSERT_TRUE(nshogi::solver::mate1ply::solve(State).isNone());
    }
}

TEST(Mate1Ply, RandomGamesReturnedMovesAreCheckmate) {
    std::mt19937_64 Random(20260917);
    for (int Game = 0; Game < 128; ++Game) {
        auto State = nshogi::core::StateBuilder::getInitialState();
        for (int Ply = 0; Ply < 256; ++Ply) {
            const auto Move = nshogi::solver::mate1ply::solve(State);
            const auto ColorMove =
                State.getSideToMove() == nshogi::core::Black
                    ? nshogi::solver::mate1ply::solve<nshogi::core::Black>(State)
                    : nshogi::solver::mate1ply::solve<nshogi::core::White>(State);
            TEST_ASSERT_TRUE(Move == ColorMove);
            if (!Move.isNone()) {
                assertMateMove(State, Move);
            }
            const auto Moves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);
            if (Moves.size() == 0) {
                break;
            }
            State.doMove(Moves[Random() % Moves.size()]);
        }
    }
}

TEST(Mate1Ply, NoMates) {
    std::ifstream Ifs("./res/test/no-mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::mate1ply::solve(State);

        TEST_ASSERT_TRUE(CheckmateMove.isNone());
    }
}

TEST(Mate1Ply, ProblemsColorTemplate) {
    std::ifstream Ifs("./res/test/mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove =
            State.getSideToMove() == nshogi::core::Color::Black
                ? nshogi::solver::mate1ply::solve<nshogi::core::Color::Black>(
                      State)
                : nshogi::solver::mate1ply::solve<nshogi::core::Color::White>(
                      State);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(Mate1Ply, NoMatesColorTemplate) {
    std::ifstream Ifs("./res/test/no-mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove =
            State.getSideToMove() == nshogi::core::Color::Black
                ? nshogi::solver::mate1ply::solve<nshogi::core::Color::Black>(
                      State)
                : nshogi::solver::mate1ply::solve<nshogi::core::Color::White>(
                      State);

        TEST_ASSERT_TRUE(CheckmateMove.isNone());
    }
}

TEST(Mate3Ply, Problems) {
    std::ifstream Ifs("./res/test/mate-3-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 3);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(Mate5Ply, Problems) {
    std::ifstream Ifs("./res/test/mate-5-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 5);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(Mate7Ply, Problems) {
    std::ifstream Ifs("./res/test/mate-7-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 7);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, Mate1Ply) {
    std::ifstream Ifs("./res/test/mate-1-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, NoMate1Ply) {
    std::ifstream Ifs("./res/test/no-mate-1-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto PV = Solver.solveWithPV(&State, 100000);

        TEST_ASSERT_TRUE(PV.size() == 0 || PV.size() > 1);
    }
}

TEST(DfPn, Mate3Ply) {
    std::ifstream Ifs("./res/test/mate-3-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, Mate5Ply) {
    std::ifstream Ifs("./res/test/mate-5-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, Mate7Ply) {
    std::ifstream Ifs("./res/test/mate-7-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, Mate9Ply) {
    std::ifstream Ifs("./res/test/mate-9-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

TEST(DfPn, Mate11Ply) {
    std::ifstream Ifs("./res/test/mate-11-ply.txt");

    nshogi::solver::dfpn::Solver Solver(64);
    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = Solver.solve(&State, 100000);

        TEST_ASSERT_FALSE(CheckmateMove.isNone());
    }
}
