//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "position.h"
#include "types.h"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace nshogi {
namespace core {

Position::Position() {
    std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

Position::Position(const Position& Pos) {
    std::memcpy(static_cast<void*>(this), static_cast<const void*>(&Pos),
                sizeof(*this));
}

Position::Position(const Position& Pos, uint16_t Offset) {
    std::memcpy(static_cast<void*>(this), static_cast<const void*>(&Pos),
                sizeof(*this));
    PlyOffset = Offset;
}

bool Position::equals(const Position& Pos, bool IgnorePlyOffset) const {
    if (SideToMove != Pos.SideToMove) {
        return false;
    }

    if (!IgnorePlyOffset && PlyOffset != Pos.PlyOffset) {
        return false;
    }

    if (!std::equal(OnBoard, OnBoard + NumSquares, Pos.OnBoard)) {
        return false;
    }

    if (!std::equal(EachStands, EachStands + NumColors, Pos.EachStands)) {
        return false;
    }

    return true;
}

} // namespace core
} // namespace nshogi
