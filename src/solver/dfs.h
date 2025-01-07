//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_SOLVER_DFS_H
#define NSHOGI_SOLVER_DFS_H

#include "../core/state.h"
#include "../core/types.h"

namespace nshogi {
namespace solver {
namespace dfs {

core::Move32 solve(core::State* S, int limit);

} // namespace dfs
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_DFS_H
