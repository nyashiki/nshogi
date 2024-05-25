#include <CUnit/CUnit.h>

#include "../core/positionbuilder.h"
#include "../io/sfen.h"
#include "../core/huffman.h"
#include "../core/movegenerator.h"

#include <random>

namespace {

void testHuffmanSize() {
    nshogi::core::HuffmanCode HC = nshogi::core::HuffmanCode::zero();
    CU_ASSERT_EQUAL(HC.size(), 4 * sizeof(uint64_t));
    CU_ASSERT_EQUAL(sizeof(nshogi::core::HuffmanCode), 4 * sizeof(uint64_t));
}

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

void testInitialPosition() {
    nshogi::core::Position Pos = nshogi::core::PositionBuilder::getInitialPosition();
    CU_ASSERT_TRUE(testPositionEquality(Pos));
    CU_ASSERT_TRUE(testHuffmanEquality(Pos));
    CU_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

void testAllStandsBlack() {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("4k4/9/9/9/9/9/9/9/4K4 b 2R2B4G4S4N4L18P 1");
    CU_ASSERT_TRUE(testPositionEquality(Pos));
    CU_ASSERT_TRUE(testHuffmanEquality(Pos));
    CU_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

void testAllStandsWhite() {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("4k4/9/9/9/9/9/9/9/4K4 b 2r2b4g4s4n4l18p 1");
    CU_ASSERT_TRUE(testPositionEquality(Pos));
    CU_ASSERT_TRUE(testHuffmanEquality(Pos));
    CU_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

void testHandmade1() {
    nshogi::core::Position Pos = nshogi::io::sfen::PositionBuilder::newPosition("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1");
    CU_ASSERT_TRUE(testPositionEquality(Pos));
    CU_ASSERT_TRUE(testHuffmanEquality(Pos));
    CU_ASSERT_TRUE(testDiffHuffmanCodeOneMoveAfter(Pos));
}

void testRandomMove() {
    const int N = 20000;
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

            CU_ASSERT_TRUE(testPositionEquality(State.getPosition()));
            CU_ASSERT_TRUE(testHuffmanEquality(State.getPosition()));
        }
    }
}

void testMemcpy() {
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

            CU_ASSERT_EQUAL(std::memcmp(HC.data(), HC2.data(), HC.size()), 0);
        }
    }
}

void testConstructor() {
    const int N = 20000;
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

            CU_ASSERT_EQUAL(HC1, HC2);
        }
    }
}

void testCopyConstructor() {
    const int N = 20000;
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

            CU_ASSERT_EQUAL(HC1, HC2);
        }
    }
}

} // namespace

int setupTestHuffman() {
    CU_pSuite suite = CU_add_suite("huffman test", 0, 0);

    CU_add_test(suite, "Huffman Size", testHuffmanSize);
    CU_add_test(suite, "Huffman Constructor", testConstructor);
    CU_add_test(suite, "Huffman Copy Constructor", testCopyConstructor);
    CU_add_test(suite, "Huffman InitialPosition", testInitialPosition);
    CU_add_test(suite, "Huffman AllStandsBlack", testAllStandsBlack);
    CU_add_test(suite, "Huffman AllStandsWhite", testAllStandsWhite);
    CU_add_test(suite, "Huffman Handmade 1", testHandmade1);
    CU_add_test(suite, "Huffman RandomMove", testRandomMove);
    CU_add_test(suite, "Huffman Memcpy", testMemcpy);

    return CUE_SUCCESS;
}
