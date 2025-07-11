//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../c_api.h"
#include "move.h"
#include "io.h"
#include "ml.h"
#include "position.h"
#include "solver.h"
#include "state.h"

#include "../core/initializer.h"
#include "../core/types.h"

using namespace nshogi;

extern "C" {

nshogi_api_t* nshogiApi(void) {
    static bool Initialized = false;
    static nshogi_api_t Api;

    if (!Initialized) {
        core::initializer::initializeAll();

        Api.moveApi = c_api::move::getApi;
        Api.positionApi = c_api::position::getApi;
        Api.stateApi = c_api::state::getApi;
        Api.solverApi = c_api::solver::getApi;
        Api.mlApi = c_api::ml::getApi;
        Api.ioApi = c_api::io::getApi;

        Initialized = true;
    }

    return &Api;
}

} // extern "C"
