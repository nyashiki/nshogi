//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "mate1ply.h"
#include "internal/mate1ply.h"
#include "../core/internal/bitboard.h"
#include "../core/internal/stateimpl.h"
#include "../core/internal/stateadapter.h"

namespace nshogi {

template <core::Color C>
core::Move32 solver::mate1ply::solve(const core::State& S) {
    core::internal::ImmutableStateAdapter Adapter(S);
    return internal::mate1ply::solve<C>(*Adapter.get());
}

core::Move32 solver::mate1ply::solve(const core::State& S) {
    core::internal::ImmutableStateAdapter Adapter(S);

    if (S.getSideToMove() == core::Black) {
        return internal::mate1ply::solve<core::Black>(*Adapter.get());
    } else {
        return internal::mate1ply::solve<core::White>(*Adapter.get());
    }
}

template
core::Move32 solver::mate1ply::solve<core::Color::Black>(const core::State&);
template
core::Move32 solver::mate1ply::solve<core::Color::White>(const core::State&);

}  // namespace nshogi
