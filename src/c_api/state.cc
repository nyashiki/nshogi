//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "state.h"

#include "../core/state.h"
#include "../core/movegenerator.h"
#include "../io/sfen.h"

using namespace nshogi;

namespace {

void stateApiDestroyState(nshogi_state_t* CState) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    delete State;
}

int stateApiGenerateMoves(nshogi_state_t* CState, int WilyPromote, nshogi_move_t* Moves) {
    core::State* State = reinterpret_cast<core::State*>(CState);

    auto generateLegalMoves = (WilyPromote)
        ? core::MoveGenerator::generateLegalMoves<true>
        : core::MoveGenerator::generateLegalMoves<false>;

    const core::MoveList MoveList = generateLegalMoves(*State);

    for (std::size_t I = 0; I < MoveList.size(); ++I) {
        Moves[I] = MoveList[I].value();
    }

    return (int)MoveList.size();
}

void stateApiDoMove(nshogi_state_t* CState, nshogi_move_t CMove) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    core::Move32 Move = core::Move32::fromValue(CMove);

    State->doMove(Move);
}

void stateApiUndoMove(nshogi_state_t* CState) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    State->undoMove();
}

nshogi_repetition_t stateApiGetRepetition(nshogi_state_t* CState) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    return static_cast<nshogi_repetition_t>(State->getRepetitionStatus());
}

int stateApiCanDeclare(nshogi_state_t* CState) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    return static_cast<int>(State->canDeclare());
}

nshogi_state_config_t* stateApiCreateStateConfig(void) {
    core::StateConfig* StateConfig = new core::StateConfig();
    return reinterpret_cast<nshogi_state_config_t*>(StateConfig);
}

void stateApiDestroyStateConfig(nshogi_state_config_t* CStateConfig) {
    core::StateConfig* StateConfig = reinterpret_cast<core::StateConfig*>(CStateConfig);
    delete StateConfig;
}

} // namespace

nshogi_state_api_t* c_api::state::getApi() {
    static bool Initialized = false;
    static nshogi_state_api_t Api;

    if (!Initialized) {
        Api.destroyState = stateApiDestroyState;
        Api.generateMoves = stateApiGenerateMoves;
        Api.doMove = stateApiDoMove;
        Api.undoMove = stateApiUndoMove;
        Api.getRepetition = stateApiGetRepetition;
        Api.canDeclare = stateApiCanDeclare;

        Api.createStateConfig = stateApiCreateStateConfig;
        Api.destroyStateConfig = stateApiDestroyStateConfig;

        Initialized = true;
    }

    return &Api;
}
