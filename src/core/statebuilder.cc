//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "statebuilder.h"
#include "positionbuilder.h"
#include "internal/stateadapter.h"

namespace nshogi {
namespace core {

StateBuilder::StateBuilder(const Position& Pos)
    : Instance(Pos) {
    internal::MutableStateAdapter Adapter(Instance);
    Adapter->refresh();
}

StateBuilder::StateBuilder(const Position& Pos, uint16_t Ply)
    : Instance(Pos, Ply) {
    internal::MutableStateAdapter Adapter(Instance);
    Adapter->refresh();
}

State StateBuilder::build() {
    return std::move(Instance);
}

State StateBuilder::getInitialState() {
    StateBuilder Builder(PositionBuilder::getInitialPosition());

    return Builder.build();
}

State StateBuilder::newState(const Position& Pos) {
    StateBuilder Builder(Pos);
    return Builder.build();
}

State StateBuilder::newState(const Position& Pos, uint16_t Ply) {
    StateBuilder Builder(Pos, Ply);
    return Builder.build();
}

} // namespace core
} // namespace nshogi
