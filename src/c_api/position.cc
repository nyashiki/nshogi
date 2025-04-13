//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "position.h"

#include "../core/position.h"

using namespace nshogi;

namespace {

nshogi_color_t positionApiGetSideToMove(nshogi_position_t* CPosition) {
    core::Position* Position = reinterpret_cast<core::Position*>(CPosition);
    return static_cast<nshogi_color_t>(Position->sideToMove());
}

nshogi_piece_t positionApiPieceOn(nshogi_position_t* CPosition, nshogi_square_t CSquare) {
    core::Position* Position = reinterpret_cast<core::Position*>(CPosition);
    core::Square Square = static_cast<core::Square>(CSquare);
    return static_cast<nshogi_piece_t>(Position->pieceOn(Square));
}

uint8_t positionApiGetStandCount(nshogi_position_t* CPosition, nshogi_color_t CColor, nshogi_piece_type_t CPieceType) {
    core::Position* Position = reinterpret_cast<core::Position*>(CPosition);
    core::Color Color = static_cast<core::Color>(CColor);
    core::PieceTypeKind PieceType = static_cast<core::PieceTypeKind>(CPieceType);
    return Position->getStandCount(Color, PieceType);
}

} // namespace

nshogi_position_api_t* c_api::position::getApi() {
    static bool Initialized = false;
    static nshogi_position_api_t Api;


    if (!Initialized) {
        Api.getSideToMove = positionApiGetSideToMove;
        Api.pieceOn = positionApiPieceOn;
        Api.getStandCount = positionApiGetStandCount;

        Initialized = true;
    }

    return &Api;
}
