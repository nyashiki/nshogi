//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "io.h"

#include "../core/state.h"
#include "../io/sfen.h"

#include <cstring>
#include <memory>
#include <stdexcept>

using namespace nshogi;

namespace {

int ioApiCanCreateStateFromSfen(const char* Sfen) {
    try {
        std::unique_ptr<core::State> State = std::make_unique<core::State>(
            io::sfen::StateBuilder::newState(Sfen));
    } catch (const std::runtime_error&) {
        return 0;
    }

    return 1;
}

int ioApiCanCreateMoveFromSfen(const nshogi_state_t* CState,
                               const char* CSfen) {
    try {
        const core::State* State = reinterpret_cast<const core::State*>(CState);
        io::sfen::sfenToMove32(State->getPosition(), CSfen);
    } catch (const std::runtime_error&) {
        return 0;
    }

    return 1;
}

nshogi_state_t* ioApiCreateStateFromSfen(const char* Sfen) {
    core::State* State =
        new core::State(io::sfen::StateBuilder::newState(Sfen));
    return reinterpret_cast<nshogi_state_t*>(State);
}

nshogi_move_t ioApiCreateMoveFromSfen(const nshogi_state_t* CState,
                                      const char* CSfen) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    core::Move32 Move = io::sfen::sfenToMove32(State->getPosition(), CSfen);
    return static_cast<nshogi_move_t>(Move.value());
}

int32_t ioApiMoveToSfen(char* Dest, nshogi_move_t CMove) {
    core::Move32 Move = core::Move32::fromValue(CMove);
    std::string Sfen = io::sfen::move32ToSfen(Move);
    std::strcpy(Dest, Sfen.c_str());
    return static_cast<int32_t>(Sfen.size());
}

int32_t ioApiStateToSfen(char* Dest, int32_t MaxLength,
                         const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    std::string Sfen = io::sfen::stateToSfen(*State);

    if ((std::size_t)MaxLength <= Sfen.size()) {
        return 0;
    }

    std::strcpy(Dest, Sfen.c_str());
    return static_cast<int32_t>(Sfen.size());
}

int32_t ioApiPositionToSfen(char* Dest, int32_t MaxLength,
                            const nshogi_position_t* CPosition) {
    const core::Position* Position =
        reinterpret_cast<const core::Position*>(CPosition);
    std::string Sfen = io::sfen::positionToSfen(*Position);

    if ((std::size_t)MaxLength <= Sfen.size()) {
        return 0;
    }

    std::strcpy(Dest, Sfen.c_str());
    return static_cast<int32_t>(Sfen.size());
}

} // namespace

nshogi_io_api_t* c_api::io::getApi() {
    static bool Initialized = false;
    static nshogi_io_api_t Api;

    if (!Initialized) {
        Api.canCreateStateFromSfen = ioApiCanCreateStateFromSfen;
        Api.canCreateMoveFromSfen = ioApiCanCreateMoveFromSfen;
        Api.createStateFromSfen = ioApiCreateStateFromSfen;
        Api.createMoveFromSfen = ioApiCreateMoveFromSfen;
        Api.moveToSfen = ioApiMoveToSfen;
        Api.stateToSfen = ioApiStateToSfen;
        Api.positionToSfen = ioApiPositionToSfen;

        Initialized = true;
    }

    return &Api;
}
