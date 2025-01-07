//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "initializer.h"
#include "internal/bitboard.h"

namespace nshogi {
namespace core {
namespace initializer {

void initializeAll() {
    static bool IsCalled = false;

    if (IsCalled) {
        return;
    }

    IsCalled = true;

    internal::bitboard::initializeBitboards();
}

} // namespace initializer
} // namespace core
} // namespace nshogi
