//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "solver.h"
#include "../core/state.h"
#include "../solver/dfpn.h"
#include "../solver/dfs.h"

using namespace nshogi;

namespace {

nshogi_move_t solverApiDFS(nshogi_state_t* CState, int Depth) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    const auto CheckmateMove = solver::dfs::solve(State, Depth);
    return CheckmateMove.value();
}

nshogi_solver_dfpn_t* solverApiCreateDfPnSolver(long MemoryMB) {
    auto Solver = new solver::dfpn::Solver((std::size_t)MemoryMB);
    return reinterpret_cast<nshogi_solver_dfpn_t*>(Solver);
}

void solverApiDestroyDfPnSolver(nshogi_solver_dfpn_t* CSolver) {
    auto Solver = reinterpret_cast<solver::dfpn::Solver*>(CSolver);
    delete Solver;
}

nshogi_move_t solverApiSolveByDfPn(nshogi_state_t* CState,
                                   nshogi_solver_dfpn_t* CSolver,
                                   long MaxNodeCount, int MaxDepth) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    auto Solver = reinterpret_cast<solver::dfpn::Solver*>(CSolver);

    auto CheckmateMove =
        Solver->solve(State, (uint64_t)MaxNodeCount, (uint64_t)MaxDepth);
    return CheckmateMove.value();
}

} // namespace

nshogi_solver_api_t* c_api::solver::getApi() {
    static bool Initialized = false;
    static nshogi_solver_api_t Api;

    if (!Initialized) {
        Api.dfs = solverApiDFS;
        Api.createDfPnSolver = solverApiCreateDfPnSolver;
        Api.destroyDfPnSolver = solverApiDestroyDfPnSolver;
        Api.solveByDfPn = solverApiSolveByDfPn;

        Initialized = true;
    }

    return &Api;
}
