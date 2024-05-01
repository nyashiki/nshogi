#include <CUnit/CUnit.h>

#include <CUnit/TestDB.h>
#include <iostream>
#include <unistd.h>

#include "../core/bitboard.h"
#include "../core/types.h"
#include "../io/bitboard.h"
#include "test_common.hpp"

namespace {

void testZeroBB() {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        CU_ASSERT_FALSE(nshogi::core::bitboard::Bitboard::ZeroBB().isSet(Sq));
    }
}

void testAllBB() {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        CU_ASSERT_TRUE(nshogi::core::bitboard::Bitboard::AllBB().isSet(Sq));
    }
}

void testSquareBB() {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        CU_ASSERT_EQUAL(nshogi::core::bitboard::SquareBB[Sq].getOne(), Sq);
    }
}

void testDoubleFlip() {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        CU_ASSERT_EQUAL(nshogi::core::bitboard::SquareBB[Sq] ^ nshogi::core::bitboard::SquareBB[Sq],
                nshogi::core::bitboard::Bitboard::ZeroBB());
    }
}

void testNotEqual() {
    for (nshogi::core::Square Sq1 : nshogi::core::Squares) {
        for (nshogi::core::Square Sq2 : nshogi::core::Squares) {
            if (Sq1 == Sq2) {
                CU_ASSERT_EQUAL(nshogi::core::bitboard::SquareBB[Sq1],
                    nshogi::core::bitboard::SquareBB[Sq2]);
            } else {
                CU_ASSERT_NOT_EQUAL(nshogi::core::bitboard::SquareBB[Sq1],
                    nshogi::core::bitboard::SquareBB[Sq2]);
            }
        }
    }
}


void testShift() {
    using namespace nshogi::core;
    using namespace nshogi::core::bitboard;

    CU_ASSERT_EQUAL(SquareBB[Sq7G].getLeftShiftEpi64<1>(), SquareBB[Sq7F]);
    CU_ASSERT_EQUAL(SquareBB[Sq7F].getRightShiftEpi64<1>(), SquareBB[Sq7G]);
    CU_ASSERT_EQUAL(SquareBB[Sq7G].getLeftShiftEpi64(1), SquareBB[Sq7F]);
    CU_ASSERT_EQUAL(SquareBB[Sq7F].getRightShiftEpi64(1), SquareBB[Sq7G]);
}

void testPrint() {
    using namespace nshogi::test;
    using namespace nshogi::core;
    using namespace nshogi::core::bitboard;

    // clang-format off
    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print, Bitboard::ZeroBB());

    nshogi::test::testStdOut(
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n"
            " 1  1  1  1  1  1  1  1  1 \n",
            nshogi::io::bitboard::print, Bitboard::AllBB());

    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  1 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print, SquareBB[Sq1A]);

    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  1 \n",
            nshogi::io::bitboard::print, SquareBB[Sq1I]);

    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 1  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print, SquareBB[Sq9I]);

    nshogi::test::testStdOut(
            " 1  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print, SquareBB[Sq9A]);

    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  1  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print, SquareBB[Sq5E]);

    nshogi::test::testStdOut(
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  1  0  1  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  1  0  1  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n"
            " 0  0  0  0  0  0  0  0  0 \n",
            nshogi::io::bitboard::print,
            SquareBB[Sq4D] | SquareBB[Sq4F] | SquareBB[Sq6D] | SquareBB[Sq6F]);

    // clang-format on
}

} // namespace

int setupTestBitboard() {
    CU_pSuite suite = CU_add_suite("Bitboard test", 0, 0);

    CU_add_test(suite, "ZeroBB", testZeroBB);
    CU_add_test(suite, "AllBB", testAllBB);
    CU_add_test(suite, "Double Flip", testDoubleFlip);
    CU_add_test(suite, "Not Equal", testNotEqual);
    CU_add_test(suite, "SquareBB getOne", testSquareBB);
    CU_add_test(suite, "Shift", testShift);
    CU_add_test(suite, "Print", testPrint);

    return CUE_SUCCESS;
}
