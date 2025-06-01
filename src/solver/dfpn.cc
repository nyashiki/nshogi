//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfpn.h"
#include "internal/dfpn.h"

namespace nshogi {
namespace solver {
namespace dfpn {

Solver::Solver(std::size_t MemoryMB)
    : Impl(std::make_unique<internal::dfpn::SolverImpl>(MemoryMB)) {
}

Solver::~Solver() {
}

core::Move32 Solver::solve(core::State* S, uint64_t MaxNodeCount, uint64_t MaxDepth) {
    return Impl->solve(S, MaxNodeCount, MaxDepth);
}

std::vector<core::Move32> Solver::solveWithPV(core::State* S, uint64_t MaxNodeCount, uint64_t MaxDepth) {
    return Impl->solveWithPV(S, MaxNodeCount, MaxDepth);
}

uint64_t Solver::searchedNodeCount() const {
    return Impl->searchedNodeCount();
}

} // namespace dfpn
} // namespace solver
} // namespace nshogi
