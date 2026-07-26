//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../core/internal/bitboard.h"
#include "../core/types.h"
#include "../io/internal/bitboard.h"
#include "common.h"
#include "utils.h"

#include <random>
#include <unistd.h>

TEST(Bitboard, ZeroBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_FALSE(
            nshogi::core::internal::bitboard::Bitboard::ZeroBB().isSet(Sq));
    }
}

TEST(Bitboard, AllBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_TRUE(
            nshogi::core::internal::bitboard::Bitboard::AllBB().isSet(Sq));
    }
}

TEST(Bitboard, SquareBB) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_EQ(nshogi::core::internal::bitboard::SquareBB[Sq].getOne(),
                       Sq);
    }
}

TEST(Bitboard, DoubleFlip) {
    for (nshogi::core::Square Sq : nshogi::core::Squares) {
        TEST_ASSERT_EQ(nshogi::core::internal::bitboard::SquareBB[Sq] ^
                           nshogi::core::internal::bitboard::SquareBB[Sq],
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
                TEST_ASSERT_NEQ(
                    nshogi::core::internal::bitboard::SquareBB[Sq1],
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

namespace {

nshogi::core::internal::bitboard::Bitboard computeSliderAttackNaive(
    nshogi::core::Square Sq,
    const nshogi::core::internal::bitboard::Bitboard& OccupiedBB,
    const int (&FileSteps)[4], const int (&RankSteps)[4]) {
    using namespace nshogi::core;
    using nshogi::core::internal::bitboard::Bitboard;
    using nshogi::core::internal::bitboard::SquareBB;

    Bitboard AttackBB = Bitboard::ZeroBB();

    for (int D = 0; D < 4; ++D) {
        int F = (int)squareToFile(Sq) + FileSteps[D];
        int R = (int)squareToRank(Sq) + RankSteps[D];

        while (0 <= F && F < (int)NumFiles && 0 <= R && R < (int)NumRanks) {
            const Square To = makeSquare((Rank)R, (File)F);
            AttackBB |= SquareBB[To];

            if (OccupiedBB.isSet(To)) {
                break;
            }

            F += FileSteps[D];
            R += RankSteps[D];
        }
    }

    return AttackBB;
}

nshogi::core::internal::bitboard::Bitboard
generateRandomOccupancy(std::mt19937_64& Mt) {
    return nshogi::core::internal::bitboard::Bitboard(
        Mt() & 0x3ffffULL, Mt() & 0x7fffffffffffffffULL);
}

} // namespace

TEST(Bitboard, BishopAttack) {
    using namespace nshogi::core;
    using namespace nshogi::core::internal::bitboard;

    const int FileSteps[4] = {1, 1, -1, -1};
    const int RankSteps[4] = {1, -1, 1, -1};

    std::mt19937_64 Mt(0x20260712ULL);

    for (Square Sq : Squares) {
        for (int I = 0; I < 1000; ++I) {
            Bitboard OccupiedBB = (I == 0)   ? Bitboard::ZeroBB()
                                  : (I == 1) ? Bitboard::AllBB()
                                             : generateRandomOccupancy(Mt);
            if (I % 2 == 0) {
                OccupiedBB |= SquareBB[Sq];
            }

            const Bitboard ExpectedBB =
                computeSliderAttackNaive(Sq, OccupiedBB, FileSteps, RankSteps);

            TEST_ASSERT_EQ(getBishopAttackBB<PTK_Bishop>(Sq, OccupiedBB),
                           ExpectedBB);
            TEST_ASSERT_EQ(getBishopAttackBB<PTK_ProBishop>(Sq, OccupiedBB),
                           ExpectedBB | KingAttackBB[Sq]);
        }
    }
}

TEST(Bitboard, RookAttack) {
    using namespace nshogi::core;
    using namespace nshogi::core::internal::bitboard;

    const int FileSteps[4] = {1, -1, 0, 0};
    const int RankSteps[4] = {0, 0, 1, -1};

    std::mt19937_64 Mt(0x20260712ULL);

    for (Square Sq : Squares) {
        for (int I = 0; I < 1000; ++I) {
            Bitboard OccupiedBB = (I == 0)   ? Bitboard::ZeroBB()
                                  : (I == 1) ? Bitboard::AllBB()
                                             : generateRandomOccupancy(Mt);
            if (I % 2 == 0) {
                OccupiedBB |= SquareBB[Sq];
            }

            const Bitboard ExpectedBB =
                computeSliderAttackNaive(Sq, OccupiedBB, FileSteps, RankSteps);

            TEST_ASSERT_EQ(getRookAttackBB<PTK_Rook>(Sq, OccupiedBB),
                           ExpectedBB);
            TEST_ASSERT_EQ(getRookAttackBB<PTK_ProRook>(Sq, OccupiedBB),
                           ExpectedBB | KingAttackBB[Sq]);
        }
    }
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
