//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "statehelper.h"
#include "../types.h"

#include <cstdlib>
#include <cstring>

namespace nshogi {
namespace core {
namespace internal {

StateHelper::StateHelper(const Position& Pos)
    : InitialPosition(Pos)
    , Ply(0) {
    SHelper.reserve(DefaultReserveSize);
    SHelper.emplace_back();
    assert(SHelper.size() == 1);
}

StateHelper::StateHelper(const Position& Pos, uint16_t PlyOffset)
    : InitialPosition(Pos, PlyOffset)
    , Ply(0) {
    SHelper.reserve(DefaultReserveSize);
    SHelper.emplace_back();
    assert(SHelper.size() == 1);
}

StateHelper::StateHelper(StateHelper&& Helper) noexcept
    : InitialPosition(Helper.InitialPosition), Ply(Helper.Ply),
      SHelper(Helper.SHelper) {

    std::memcpy(static_cast<void*>(ColorBB),
                static_cast<const void*>(Helper.ColorBB),
                NumColors * sizeof(bitboard::Bitboard));

    std::memcpy(static_cast<void*>(TypeBB),
                static_cast<const void*>(Helper.TypeBB),
                NumPieceType * sizeof(bitboard::Bitboard));

    std::memcpy(static_cast<void*>(KingSquare),
                static_cast<const void*>(Helper.KingSquare),
                NumColors * sizeof(Square));
}

StateHelper::~StateHelper() {
}

void StateHelper::proceedOneStep(Move32 Move, uint64_t BoardHash, Stands BlackStand, Stands WhiteStand) {
    SHelper.emplace_back();

    assert(Ply < SHelper.size());

    // Record the move and increment Ply.
    SHelper[Ply].Move = Move;
    SHelper[Ply].BoardHash = BoardHash;
    SHelper[Ply].EachStand[Black] = BlackStand;
    SHelper[Ply].EachStand[White] = WhiteStand;

    Ply++;
}

Move32 StateHelper::goBackOneStep() {
    assert(Ply > 0);

    SHelper.pop_back();

    --Ply;
    const Move32 PrevMove = SHelper[Ply].Move;

    return PrevMove;
}

} // namepsace internal
} // namespace core
} // namespace nshogi
