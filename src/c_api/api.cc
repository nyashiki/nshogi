//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../c_api.h"
#include "io.h"
#include "ml.h"
#include "move.h"
#include "position.h"
#include "solver.h"
#include "state.h"

#include "../core/initializer.h"
#include "../core/types.h"

using namespace nshogi;

extern "C" {

nshogi_api_t* nshogiApi(void) {
    static nshogi_api_t Api = [] {
        core::initializer::initializeAll();

        nshogi_api_t A;

        A.moveApi = c_api::move::getApi;
        A.positionApi = c_api::position::getApi;
        A.stateApi = c_api::state::getApi;
        A.solverApi = c_api::solver::getApi;
        A.mlApi = c_api::ml::getApi;
        A.ioApi = c_api::io::getApi;

        return A;
    }();

    return &Api;
}

} // extern "C"
