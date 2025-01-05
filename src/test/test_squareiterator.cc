//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"
#include "../core/types.h"
#include "../core/squareiterator.h"

namespace {

template<nshogi::core::IterateOrder Order>
void testReverseOrder() {
    nshogi::core::SquareIterator<Order> SqIt;
    nshogi::core::SquareIterator<nshogi::core::reverseOrder<Order>()> RvSqIt;

    while (true) {
        auto Sq1 = *SqIt;
        auto Sq2 = *RvSqIt;

        TEST_ASSERT_EQ(Sq1, nshogi::core::getInversed(Sq2));

        ++SqIt;
        ++RvSqIt;

        if (SqIt == SqIt.end()) {
            TEST_ASSERT_TRUE(RvSqIt == RvSqIt.end());
            break;
        }
    }
}

} // namespace

TEST(SquareIterator, ReverseESWN) {
    testReverseOrder<nshogi::core::IterateOrder::ESWN>();
}

TEST(SquareIterator, ReverseWNES) {
    testReverseOrder<nshogi::core::IterateOrder::WNES>();
}

TEST(SquareIterator, ReverseNWSE) {
    testReverseOrder<nshogi::core::IterateOrder::NWSE>();
}

TEST(SquareIterator, ReverseSENW) {
    testReverseOrder<nshogi::core::IterateOrder::SENW>();
}
