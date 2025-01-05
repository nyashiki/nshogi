//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_COMMON_H
#define NSHOGI_ML_COMMON_H

#include "../core/types.h"
#include <cstddef>


namespace nshogi {
namespace ml {

constexpr std::size_t MoveIndexMax = (std::size_t)(27 * core::NumSquares);

namespace {

template <core::Color C>
inline std::size_t getMoveIndex(core::Move16 M16) {
    const core::Square To = (C == core::Black)? M16.to() : core::getInversed(M16.to());

    if (M16.drop()) {
        const auto Type = (core::PieceTypeKind)(M16.from() - core::NumSquares + 1);
        return (std::size_t)(20 + (int)Type - 1) * (std::size_t)core::NumSquares + (std::size_t)To;
    }

    const core::Square From = (C == core::Black)? M16.from() : core::getInversed(M16.from());
    const uint8_t SerializedDirection = core::SquareSerializedDirection[(std::size_t)From][(std::size_t)To];

    const std::size_t Channel = (M16.promote())? (10 + SerializedDirection) : SerializedDirection;

    return Channel * (std::size_t)core::NumSquares + (std::size_t)To;
}

template <core::Color C>
inline std::size_t getMoveIndex(core::Move32 Move) {
    return getMoveIndex<C>(core::Move16(Move));
}

} // namespace

inline std::size_t getMoveIndex(core::Color C, const core::Move32 Move) {
    if (C == core::Black) {
        return getMoveIndex<core::Black>(Move);
    } else {
        return getMoveIndex<core::White>(Move);
    }
}

inline std::size_t getMoveIndex(core::Color C, const core::Move16 Move) {
    if (C == core::Black) {
        return getMoveIndex<core::Black>(Move);
    } else {
        return getMoveIndex<core::White>(Move);
    }
}


} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_COMMON_H
