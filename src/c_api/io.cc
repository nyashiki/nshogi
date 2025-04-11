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

nshogi_state_t* ioApiCreateStateFromSfen(char* Sfen) {
    core::State* State = new core::State(io::sfen::StateBuilder::newState(Sfen));
    return reinterpret_cast<nshogi_state_t*>(State);
}

char* ioApiMoveToSfen(nshogi_move_t CMove) {
    core::Move32 Move = core::Move32::fromValue(CMove);
    std::string Sfen = io::sfen::move32ToSfen(Move);

    char* CSfen = static_cast<char*>(std::malloc((Sfen.size() + 1) * sizeof(char)));
    std::strcpy(CSfen, Sfen.c_str());
    return CSfen;
}


} // namespace

nshogi_io_api_t* c_api::io::getApi() {
    static bool Initialized = false;
    static nshogi_io_api_t Api;

    if (!Initialized) {
        Api.createStateFromSfen = ioApiCreateStateFromSfen;
        Api.moveToSfen = ioApiMoveToSfen;

        Initialized = true;
    }

    return &Api;
}
