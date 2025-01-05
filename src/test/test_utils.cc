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
#include "../core/internal/utils.h"

TEST(Utils, IsSameLine) {
    using namespace nshogi::core;

    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1B));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1C));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1D));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1E));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1F));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1G));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1H));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq1I));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq2B));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq3C));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq4D));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq5E));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq6F));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq7G));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq8H));
    TEST_ASSERT_TRUE(internal::utils::isSameLine(Sq1A, Sq9I));

    TEST_ASSERT_FALSE(internal::utils::isSameLine(Sq1A, Sq1A));
    TEST_ASSERT_FALSE(internal::utils::isSameLine(Sq1A, Sq2C));
    TEST_ASSERT_FALSE(internal::utils::isSameLine(Sq2B, Sq4C));
    TEST_ASSERT_FALSE(internal::utils::isSameLine(Sq5E, Sq1B));
    TEST_ASSERT_FALSE(internal::utils::isSameLine(Sq9E, Sq4B));
}
