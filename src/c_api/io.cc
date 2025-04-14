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

using namespace nshogi;

namespace {

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

char* ioApiMoveToSfen(nshogi_move_t CMove) {
    core::Move32 Move = core::Move32::fromValue(CMove);
    std::string Sfen = io::sfen::move32ToSfen(Move);

    char* CSfen =
        static_cast<char*>(std::malloc((Sfen.size() + 1) * sizeof(char)));
    std::strcpy(CSfen, Sfen.c_str());
    return CSfen;
}

char* ioApiStateToSfen(const nshogi_state_t* CState) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);

    std::string Sfen = io::sfen::stateToSfen(*State);

    char* CSfen = static_cast<char*>(malloc((Sfen.size() + 1) * sizeof(char)));
    std::strcpy(CSfen, Sfen.c_str());

    return CSfen;
}

char* ioApiPositionToSfen(const nshogi_position_t* CPosition) {
    const core::Position* Position =
        reinterpret_cast<const core::Position*>(CPosition);

    std::string Sfen = io::sfen::positionToSfen(*Position);

    char* CSfen = static_cast<char*>(malloc((Sfen.size() + 1) * sizeof(char)));
    std::strcpy(CSfen, Sfen.c_str());

    return CSfen;
}

} // namespace

nshogi_io_api_t* c_api::io::getApi() {
    static bool Initialized = false;
    static nshogi_io_api_t Api;

    if (!Initialized) {
        Api.createStateFromSfen = ioApiCreateStateFromSfen;
        Api.createMoveFromSfen = ioApiCreateMoveFromSfen;
        Api.moveToSfen = ioApiMoveToSfen;
        Api.stateToSfen = ioApiStateToSfen;
        Api.positionToSfen = ioApiPositionToSfen;

        Initialized = true;
    }

    return &Api;
}
