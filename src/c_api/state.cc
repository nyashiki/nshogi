//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "state.h"

#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../io/sfen.h"

using namespace nshogi;

namespace {

void stateApiDestroyState(nshogi_state_t* CState) {
    core::State* State = reinterpret_cast<core::State*>(CState);
    delete State;
}

nshogi_state_t* stateApiClone(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return reinterpret_cast<nshogi_state_t*>(new core::State(State->clone()));
}

int stateApiGenerateMoves(const nshogi_state_t* CState, int WilyPromote,
                          nshogi_move_t* Moves) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);

    auto generateLegalMoves =
        (WilyPromote) ? core::MoveGenerator::generateLegalMoves<true>
                      : core::MoveGenerator::generateLegalMoves<false>;

    const core::MoveList MoveList = generateLegalMoves(*State);

    for (std::size_t I = 0; I < MoveList.size(); ++I) {
        Moves[I] = MoveList[I].value();
    }

    return (int)MoveList.size();
}

nshogi_color_t stateApiGetSideToMove(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<nshogi_color_t>(State->getSideToMove());
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

nshogi_repetition_t stateApiGetRepetition(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<nshogi_repetition_t>(State->getRepetitionStatus());
}

int stateApiGetPly(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return State->getPly();
}

const nshogi_position_t* stateApiGetPosition(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return reinterpret_cast<const nshogi_position_t*>(&State->getPosition());
}

const nshogi_position_t*
stateApiGetInitialPosition(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return reinterpret_cast<const nshogi_position_t*>(
        &State->getInitialPosition());
}

int stateApiCanDeclare(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<int>(State->canDeclare());
}

nshogi_move_t stateApiGetHistoryMove(const nshogi_state_t* CState,
                                     uint16_t Ply) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<nshogi_move_t>(State->getHistoryMove(Ply).value());
}

nshogi_move_t stateApiGetLastMove(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<nshogi_move_t>(State->getLastMove().value());
}

uint64_t stateApiGetHash(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return State->getHash();
}

int stateApiIsInCheck(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    return static_cast<bool>(State->isInCheck());
}

nshogi_state_config_t* stateApiCreateStateConfig(void) {
    core::StateConfig* StateConfig = new core::StateConfig();
    return reinterpret_cast<nshogi_state_config_t*>(StateConfig);
}

void stateApiDestroyStateConfig(nshogi_state_config_t* CStateConfig) {
    core::StateConfig* StateConfig =
        reinterpret_cast<core::StateConfig*>(CStateConfig);
    delete StateConfig;
}

uint16_t stateApiGetMaxPly(const nshogi_state_config_t* CStateConfig) {
    const core::StateConfig* StateConfig =
        reinterpret_cast<const core::StateConfig*>(CStateConfig);
    return StateConfig->MaxPly;
}

float stateApiGetBlackDrawValue(const nshogi_state_config_t* CStateConfig) {
    const core::StateConfig* StateConfig =
        reinterpret_cast<const core::StateConfig*>(CStateConfig);
    return StateConfig->BlackDrawValue;
}

float stateApiGetWhiteDrawValue(const nshogi_state_config_t* CStateConfig) {
    const core::StateConfig* StateConfig =
        reinterpret_cast<const core::StateConfig*>(CStateConfig);
    return StateConfig->WhiteDrawValue;
}

void stateApiSetMaxPly(nshogi_state_config_t* CStateConfig, uint16_t MaxPly) {
    core::StateConfig* StateConfig =
        reinterpret_cast<core::StateConfig*>(CStateConfig);
    StateConfig->MaxPly = MaxPly;
}

void stateApiSetBlackDrawValue(nshogi_state_config_t* CStateConfig,
                               float Value) {
    core::StateConfig* StateConfig =
        reinterpret_cast<core::StateConfig*>(CStateConfig);
    StateConfig->BlackDrawValue = Value;
}

void stateApiSetWhiteDrawValue(nshogi_state_config_t* CStateConfig,
                               float Value) {
    core::StateConfig* StateConfig =
        reinterpret_cast<core::StateConfig*>(CStateConfig);
    StateConfig->WhiteDrawValue = Value;
}

} // namespace

nshogi_state_api_t* c_api::state::getApi() {
    static bool Initialized = false;
    static nshogi_state_api_t Api;

    if (!Initialized) {
        Api.destroyState = stateApiDestroyState;

        Api.clone = stateApiClone;

        Api.getSideToMove = stateApiGetSideToMove;
        Api.getPly = stateApiGetPly;
        Api.getPosition = stateApiGetPosition;
        Api.getInitialPosition = stateApiGetInitialPosition;
        Api.getRepetition = stateApiGetRepetition;
        Api.canDeclare = stateApiCanDeclare;
        Api.getHistoryMove = stateApiGetHistoryMove;
        Api.getLastMove = stateApiGetLastMove;
        Api.getHash = stateApiGetHash;
        Api.isInCheck = stateApiIsInCheck;

        Api.generateMoves = stateApiGenerateMoves;
        Api.doMove = stateApiDoMove;
        Api.undoMove = stateApiUndoMove;

        Api.createStateConfig = stateApiCreateStateConfig;
        Api.destroyStateConfig = stateApiDestroyStateConfig;

        Api.getMaxPly = stateApiGetMaxPly;
        Api.getBlackDrawValue = stateApiGetBlackDrawValue;
        Api.getWhiteDrawValue = stateApiGetWhiteDrawValue;

        Api.setMaxPly = stateApiSetMaxPly;
        Api.setBlackDrawValue = stateApiSetBlackDrawValue;
        Api.setWhiteDrawValue = stateApiSetWhiteDrawValue;

        Initialized = true;
    }

    return &Api;
}
