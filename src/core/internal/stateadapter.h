//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_INTERNAL_STATEADAPTER_H
#define NSHOGI_CORE_INTERNAL_STATEADAPTER_H

#include "../state.h"
#include "stateimpl.h"

namespace nshogi {
namespace core {
namespace internal {

class ImmutableStateAdapter {
 public:
    ImmutableStateAdapter(const State& S) noexcept
        : Impl(S.Impl) {
    }

    const StateImpl* operator->() const noexcept {
        return Impl;
    }

    const StateImpl* get() const noexcept {
        return Impl;
    }

 private:
    const StateImpl* const Impl;
};

class MutableStateAdapter {
 public:
    MutableStateAdapter(State& S) noexcept
        : Impl(S.Impl) {
    }

    StateImpl* operator->() const noexcept {
        return Impl;
    }

    StateImpl* get() const noexcept {
        return Impl;
    }

 private:
    StateImpl* const Impl;
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_STATEADAPTER_H
