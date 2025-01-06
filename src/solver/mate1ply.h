//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_SOLVER_MATE1PLY_H
#define NSHOGI_SOLVER_MATE1PLY_H

#include "../core/state.h"
#include "../core/types.h"

namespace nshogi {
namespace solver {
namespace mate1ply {

template <core::Color C>
core::Move32 solve(const core::State& S);

/// Returns Move32::MoveNone() if there is any checkmate moves
/// within one ply. Otherwise, returns an one-ply-checkmate move.
core::Move32 solve(const core::State& S);

} // namespace mate1ply
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_MATE1PLY_H
