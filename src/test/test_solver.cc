#include <CUnit/CUnit.h>

#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"
#include "../io/bitboard.h"
#include "../io/sfen.h"
#include "../solver/mate1ply.h"
#include "../solver/dfs.h"

#include <CUnit/TestDB.h>
#include <fstream>

namespace {

void testMate1PlyHandmade1() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "G*5b");
}

void testMate1PlyHandmade2() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b S 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "S*5b");
}

void testMate1PlyHandmade3() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gkg3/9/3pG4/9/9/9/9/9/4K4 b N 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "N*4c");
}

void testMate1PlyHandmade4() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gkg3/9/4Gp3/9/9/9/9/9/4K4 b N 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "N*6c");
}

void testMate1PlyHandmade5() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3sk4/9/4G4/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "G*4b");
}

void testMate1PlyHandmade6() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4G4/9/9/9/9/9/4K4 b B 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "B*6b");
}

void testMate1PlyHandmade7() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4P4/9/9/9/9/9/4K4 b R 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "R*5b");
}

void testMate1PlyHandmade8() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3nkl3/9/4S4/9/9/9/9/9/4K4 b L 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "L*5b");
}

void testMate1PlyHandmade9() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/9/9/9/9/4g4/9/4K4 w g 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "G*5h");
}

void testMate1PlyHandmade10() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/3B5/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "G*5b");
}

void testMate1PlyHandmade11() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/2B6/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "5c5b");
}

void testMate1PlyHandmade12() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "3gk4/9/4SG3/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "4c4b");
}

void testMate1PlyHandmade13() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/4L4/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "5c5b");
}

void testMate1PlyHandmade14() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4kg1RR/3s5/9/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "2a4a+");
}

void testMate1PlyHandmade15() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/4G4/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "resign");
}

void testMate1PlyHandmade16() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4kg2R/3s5/9/9/9/9/9/9/4K4 b - 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "resign");
}

void testMate1PlyHandmade17() {
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(
        "4k4/9/9/9/9/9/9/9/4K4 b G 1");

    const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    const auto CheckmateMoveSfen = nshogi::io::sfen::move32ToSfen(CheckmateMove);

    CU_ASSERT_STRING_EQUAL(CheckmateMoveSfen.c_str(), "resign");
}

void testMate1Ply() {
    std::ifstream Ifs("./res/test/mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::mate1ply::solve(State);

        CU_ASSERT_FALSE(CheckmateMove.isNone());
    }
}

void testNoMate1Ply() {
    std::ifstream Ifs("./res/test/no-mate-1-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::mate1ply::solve(State);

        CU_ASSERT_TRUE(CheckmateMove.isNone());
    }
}

void testMate3Ply() {
    std::ifstream Ifs("./res/test/mate-3-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 3);

        CU_ASSERT_TRUE(!CheckmateMove.isNone());
    }
}

void testMate5Ply() {
    std::ifstream Ifs("./res/test/mate-5-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 5);

        CU_ASSERT_TRUE(!CheckmateMove.isNone());
    }
}

void testMate7Ply() {
    std::ifstream Ifs("./res/test/mate-7-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 7);

        CU_ASSERT_TRUE(!CheckmateMove.isNone());
    }
}

void testMate9Ply() {
    std::ifstream Ifs("./res/test/mate-9-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 9);

        CU_ASSERT_TRUE(!CheckmateMove.isNone());
    }
}

void testMate11Ply() {
    std::ifstream Ifs("./res/test/mate-11-ply.txt");

    std::string Line;
    while (std::getline(Ifs, Line)) {
        auto State = nshogi::io::sfen::StateBuilder::newState(Line);
        auto CheckmateMove = nshogi::solver::dfs::solve(&State, 11);

        CU_ASSERT_TRUE(!CheckmateMove.isNone());
    }
}

} // namespace

int setupTestSolver() {
    CU_pSuite suite = CU_add_suite("solver test", 0, 0);

    CU_add_test(suite, "Mate1Ply Handmade 1", testMate1PlyHandmade1);
    CU_add_test(suite, "Mate1Ply Handmade 2", testMate1PlyHandmade2);
    CU_add_test(suite, "Mate1Ply Handmade 3", testMate1PlyHandmade3);
    CU_add_test(suite, "Mate1Ply Handmade 4", testMate1PlyHandmade4);
    CU_add_test(suite, "Mate1Ply Handmade 5", testMate1PlyHandmade5);
    CU_add_test(suite, "Mate1Ply Handmade 6", testMate1PlyHandmade6);
    CU_add_test(suite, "Mate1Ply Handmade 7", testMate1PlyHandmade7);
    CU_add_test(suite, "Mate1Ply Handmade 8", testMate1PlyHandmade8);
    CU_add_test(suite, "Mate1Ply Handmade 9", testMate1PlyHandmade9);
    CU_add_test(suite, "Mate1Ply Handmade 10", testMate1PlyHandmade10);
    CU_add_test(suite, "Mate1Ply Handmade 11", testMate1PlyHandmade11);
    CU_add_test(suite, "Mate1Ply Handmade 12", testMate1PlyHandmade12);
    CU_add_test(suite, "Mate1Ply Handmade 13", testMate1PlyHandmade13);
    CU_add_test(suite, "Mate1Ply Handmade 14", testMate1PlyHandmade14);
    CU_add_test(suite, "Mate1Ply Handmade 15", testMate1PlyHandmade15);
    CU_add_test(suite, "Mate1Ply Handmade 16", testMate1PlyHandmade16);
    CU_add_test(suite, "Mate1Ply Handmade 17", testMate1PlyHandmade17);
    CU_add_test(suite, "Mate1Ply Positive Example Positions", testMate1Ply);
    CU_add_test(suite, "Mate1Ply Negative Example Positions", testNoMate1Ply);
    CU_add_test(suite, "Mate3Ply Positive Example Positions", testMate3Ply);
    CU_add_test(suite, "Mate5Ply Positive Example Positions", testMate5Ply);
    CU_add_test(suite, "Mate7Ply Positive Example Positions", testMate7Ply);
    CU_add_test(suite, "Mate9Ply Positive Example Positions", testMate9Ply);
    CU_add_test(suite, "Mate11Ply Positive Example Positions", testMate11Ply);

    return CUE_SUCCESS;
}
