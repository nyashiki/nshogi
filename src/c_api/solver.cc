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

nshogi_solver_dfpn_t* solverApiCreateDfPnSolver(uint64_t MemoryMB) {
    auto Solver = new solver::dfpn::Solver((std::size_t)MemoryMB);
    return reinterpret_cast<nshogi_solver_dfpn_t*>(Solver);
}

void solverApiDestroyDfPnSolver(nshogi_solver_dfpn_t* CSolver) {
    auto Solver = reinterpret_cast<solver::dfpn::Solver*>(CSolver);
    delete Solver;
}

nshogi_move_t solverApiSolveByDfPn(nshogi_state_t* CState,
                                   nshogi_solver_dfpn_t* CSolver,
                                   uint64_t MaxNodeCount, int MaxDepth,
                                   int Strict) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    auto Solver = reinterpret_cast<solver::dfpn::Solver*>(CSolver);

    auto CheckmateMove =
        Solver->solve(State, MaxNodeCount, (uint64_t)MaxDepth, (bool)Strict);
    return CheckmateMove.value();
}

int solverApiSolveWithPVByDfPn(nshogi_state_t* CState,
                               nshogi_solver_dfpn_t* CSolver,
                               uint64_t MaxNodeCount, int MaxDepth, int Strict,
                               nshogi_move_t* Buffer, int BufferSize) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    auto Solver = reinterpret_cast<solver::dfpn::Solver*>(CSolver);

    auto CheckmateSequence = Solver->solveWithPV(
        State, MaxNodeCount, (uint64_t)MaxDepth, (bool)Strict);

    int WriteCount = 0;

    for (const auto Move : CheckmateSequence) {
        if (WriteCount >= BufferSize) {
            break;
        }

        Buffer[WriteCount] = Move.value();
        ++WriteCount;
    }

    return WriteCount;
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
        Api.solveWithPVByDfPn = solverApiSolveWithPVByDfPn;

        Initialized = true;
    }

    return &Api;
}
