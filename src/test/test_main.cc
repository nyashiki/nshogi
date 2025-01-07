//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../core/initializer.h"
#include "common.h"
#include <cinttypes>

int main() {
    nshogi::core::initializer::initializeAll();
    const auto Statistics = nshogi::test::common::TestBody::getInstance().run();

    std::printf("Ran %" PRIu64 " test cases: ",
                Statistics.NumSuccess + Statistics.NumFail);
    std::printf("%" PRIu64 " succeeded, %" PRIu64 " failed.\n",
                Statistics.NumSuccess, Statistics.NumFail);

    return Statistics.NumFail != 0;
}
