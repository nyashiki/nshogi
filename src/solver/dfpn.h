//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include <memory>
#include <vector>

#ifndef NSHOGI_SOLVER_DFPN_H
#define NSHOGI_SOLVER_DFPN_H

#include "../core/state.h"
#include "../core/types.h"

namespace nshogi {
namespace solver {
namespace dfpn {

class Solver {
 public:
    Solver(std::size_t MemoryMB, uint64_t MaxDepth = 64, uint64_t MaxNodeCount = 1000000);
    ~Solver();

    core::Move32 solve(core::State* S);
    std::vector<core::Move32> solveWithPV(core::State* S);

    uint64_t searchedNodeCount() const;

 private:
    class SolverImpl;
    std::unique_ptr<SolverImpl> Impl;
};

} // namespace dfpn
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_DFPN_H
