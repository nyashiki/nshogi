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

#include <mutex>

namespace nshogi {
namespace core {
namespace initializer {

void initializeAll() {
    static std::mutex Mutex;
    // Enter the critical section here to ensure that multiple threads
    // cannot call initializeAll() concurrently.
    // This can occur, for example, when `cargo test`
    // calls this C++ code through the Rust API.
    std::lock_guard<std::mutex> Lock(Mutex);

    static bool IsCalled = false;

    if (IsCalled) {
        return;
    }

    internal::bitboard::initializeBitboards();
    IsCalled = true;
}

} // namespace initializer
} // namespace core
} // namespace nshogi
