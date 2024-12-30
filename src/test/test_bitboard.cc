#include "common.h"
#include "utils.h"
#include "../core/internal/bitboard.h"
#include "../core/types.h"
#include "../io/internal/bitboard.h"

#include <unistd.h>

TEST(Bitboard, ZeroBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_FALSE(nshogi::core::internal::bitboard::Bitboard::ZeroBB().isSet(Sq));
    }
}

TEST(Bitboard, AllBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_TRUE(nshogi::core::internal::bitboard::Bitboard::AllBB().isSet(Sq));
    }
}

TEST(Bitboard, SquareBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_EQ(nshogi::core::internal::bitboard::SquareBB[Sq].getOne(), Sq);
    }
}

TEST(Bitboard, DoubleFlip) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_EQ(nshogi::core::internal::bitboard::SquareBB[Sq] ^ nshogi::core::internal::bitboard::SquareBB[Sq],
                nshogi::core::internal::bitboard::Bitboard::ZeroBB());
    }
}

TEST(Bitboard, NotEqual) {
    for (nshogi::core::Square Sq1 : nshogi::core::Squares) {
        for (nshogi::core::Square Sq2 : nshogi::core::Squares) {
            if (Sq1 == Sq2) {
                TEST_ASSERT_EQ(nshogi::core::internal::bitboard::SquareBB[Sq1],
                    nshogi::core::internal::bitboard::SquareBB[Sq2]);
            } else {
                TEST_ASSERT_NEQ(nshogi::core::internal::bitboard::SquareBB[Sq1],
                    nshogi::core::internal::bitboard::SquareBB[Sq2]);
            }
        }
    }
}

TEST(Bitboard, Shift) {
    using namespace nshogi::core;
    using namespace nshogi::core::internal::bitboard;

    TEST_ASSERT_EQ(SquareBB[Sq7G].getLeftShiftEpi64<1>(), SquareBB[Sq7F]);
    TEST_ASSERT_EQ(SquareBB[Sq7F].getRightShiftEpi64<1>(), SquareBB[Sq7G]);
    TEST_ASSERT_EQ(SquareBB[Sq7G].getLeftShiftEpi64(1), SquareBB[Sq7F]);
    TEST_ASSERT_EQ(SquareBB[Sq7F].getRightShiftEpi64(1), SquareBB[Sq7G]);
}

TEST(Bitboard, Print) {
    using namespace nshogi::test;
    using namespace nshogi::core;
    using namespace nshogi::core::internal::bitboard;

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
            nshogi::io::internal::bitboard::print, Bitboard::ZeroBB());

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
            nshogi::io::internal::bitboard::print, Bitboard::AllBB());

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
            nshogi::io::internal::bitboard::print, SquareBB[Sq1A]);

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
            nshogi::io::internal::bitboard::print, SquareBB[Sq1I]);

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
            nshogi::io::internal::bitboard::print, SquareBB[Sq9I]);

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
            nshogi::io::internal::bitboard::print, SquareBB[Sq9A]);

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
            nshogi::io::internal::bitboard::print, SquareBB[Sq5E]);

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
            nshogi::io::internal::bitboard::print,
            SquareBB[Sq4D] | SquareBB[Sq4F] | SquareBB[Sq6D] | SquareBB[Sq6F]);

    // clang-format on
}
