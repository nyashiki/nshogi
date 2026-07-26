//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "move.h"

#include "../core/types.h"

using namespace nshogi;

namespace {

int8_t moveApiTo(nshogi_move_t CMove) {
    return core::Move32::fromValue(CMove).to();
}

int8_t moveApiFrom(nshogi_move_t CMove) {
    return core::Move32::fromValue(CMove).from();
}

int moveApiDrop(nshogi_move_t CMove) {
    return static_cast<int>(core::Move32::fromValue(CMove).drop());
}

int moveApiPromote(nshogi_move_t CMove) {
    return static_cast<int>(core::Move32::fromValue(CMove).promote());
}

uint8_t moveApiPieceType(nshogi_move_t CMove) {
    return static_cast<uint8_t>(core::Move32::fromValue(CMove).pieceType());
}

uint8_t moveApiCapturePieceType(nshogi_move_t CMove) {
    return static_cast<uint8_t>(
        core::Move32::fromValue(CMove).capturePieceType());
}

int moveApiIsNone(nshogi_move_t CMove) {
    return static_cast<int>(core::Move32::fromValue(CMove).isNone());
}

int moveApiIsWin(nshogi_move_t CMove) {
    return static_cast<int>(core::Move32::fromValue(CMove).isWin());
}

nshogi_move_t moveApiNoneMove() {
    return static_cast<nshogi_move_t>(core::Move32::MoveNone().value());
}

nshogi_move_t moveApiWinMove() {
    return static_cast<nshogi_move_t>(core::Move32::MoveWin().value());
}

} // namespace

nshogi_move_api_t* c_api::move::getApi() {
    static nshogi_move_api_t Api = [] {
        nshogi_move_api_t A;

        A.to = moveApiTo;
        A.from = moveApiFrom;
        A.drop = moveApiDrop;
        A.promote = moveApiPromote;
        A.pieceType = moveApiPieceType;
        A.capturePieceType = moveApiCapturePieceType;
        A.isNone = moveApiIsNone;
        A.isWin = moveApiIsWin;

        A.noneMove = moveApiNoneMove;
        A.winMove = moveApiWinMove;

        return A;
    }();

    return &Api;
}
