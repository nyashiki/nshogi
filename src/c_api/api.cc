//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../c_api.h"
#include "position.h"
#include "state.h"
#include "ml.h"
#include "io.h"

#include "../core/types.h"
#include "../core/state.h"
#include "../core/initializer.h"
#include "../ml/common.h"
#include "../io/sfen.h"

using namespace nshogi;

namespace {

nshogi_position_api_t* positionApiImpl() {
    return c_api::position::getApi();
}

nshogi_state_api_t* stateApiImpl() {
    return c_api::state::getApi();
}

nshogi_ml_api_t* mlApiImpl() {
    return c_api::ml::getApi();
}

nshogi_io_api_t* ioApiImpl() {
    return c_api::io::getApi();
}

int apiIsDroppingPawn(nshogi_move_t CMove) {
    core::Move32 Move = core::Move32::fromValue(CMove);
    return Move.drop() && Move.pieceType() == core::PTK_Pawn;
}

int apiMoveToIndex(const nshogi_state_t* CState, nshogi_move_t CMove) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    core::Move32 Move = core::Move32::fromValue(CMove);
    return static_cast<int>(ml::getMoveIndex(State->getSideToMove(), Move));
}

nshogi_move_t apiWinDeclarationMove() {
    core::Move32 Move = core::Move32::MoveWin();
    return static_cast<nshogi_move_t>(Move.value());
}

} // namespace

extern "C" {

nshogi_api_t* nshogiApi(void) {
    static bool Initialized = false;
    static nshogi_api_t Api;

    if (!Initialized) {
        core::initializer::initializeAll();

        Api.positionApi = positionApiImpl;
        Api.stateApi = stateApiImpl;
        Api.mlApi = mlApiImpl;
        Api.ioApi = ioApiImpl;

        Api.isDroppingPawn = apiIsDroppingPawn;
        Api.moveToIndex = apiMoveToIndex;
        Api.winDeclarationMove = apiWinDeclarationMove;

        Initialized = true;
    }

    return &Api;
}

} // extern "C"
