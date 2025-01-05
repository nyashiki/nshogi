//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_STATEBUILDER_H
#define NSHOGI_CORE_STATEBUILDER_H

#include "state.h"
#include "types.h"
#include <memory>
#include <utility>

namespace nshogi {
namespace core {

class StateBuilder {
 public:
    static State getInitialState();

    State build() {
        return std::move(Instance);
    }

    static core::State newState(const Position&);
    static core::State newState(const Position&, uint16_t Ply);

 protected:
    StateBuilder(const Position&);
    StateBuilder(const Position&, uint16_t Ply);
    State Instance;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATEBUILDER_H
