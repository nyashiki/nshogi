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

namespace internal {
namespace dfpn {

class SolverImpl;

} // namespace dfpn
} // namespace internal

namespace dfpn {

class Solver {
 public:
    Solver(std::size_t MemoryMB);
    ~Solver();

    core::Move32 solve(core::State* S, uint64_t MaxNodeCount = 1000000, uint64_t MaxDepth = 64);
    std::vector<core::Move32> solveWithPV(core::State* S, uint64_t MaxNodeCount = 1000000, uint64_t MaxDepth = 64);

    uint64_t searchedNodeCount() const;

 private:
    std::unique_ptr<internal::dfpn::SolverImpl> Impl;
};

} // namespace dfpn
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_DFPN_H
