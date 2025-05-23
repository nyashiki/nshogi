//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include <memory>

#ifndef NSHOGI_SOLVER_DFPN_H
#define NSHOGI_SOLVER_DFPN_H

#include "../core/state.h"
#include "../core/types.h"

namespace nshogi {
namespace solver {
namespace dfpn {

class Solver {
 public:
    Solver();
    ~Solver();

    core::Move32 solve(core::State* S);

 private:
    class SolverImpl;
    std::unique_ptr<SolverImpl> Impl;
};

} // namespace dfpn
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_DFPN_H
