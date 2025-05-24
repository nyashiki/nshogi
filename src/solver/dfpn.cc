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

Solver::Solver(std::size_t MemoryMB, uint64_t MaxDepth, uint64_t MaxNodeCount)
    : Impl(std::make_unique<internal::dfpn::SolverImpl>(MemoryMB, MaxDepth, MaxNodeCount)) {
}

Solver::~Solver() {
}

core::Move32 Solver::solve(core::State* S) {
    return Impl->solve(S);
}

std::vector<core::Move32> Solver::solveWithPV(core::State* S) {
    return Impl->solveWithPV(S);
}

uint64_t Solver::searchedNodeCount() const {
    return Impl->searchedNodeCount();
}

} // namespace dfpn
} // namespace solver
} // namespace nshogi
