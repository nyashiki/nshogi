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

bool Position::equals(const Position &Pos, bool IgnorePlyOffset) const {
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
