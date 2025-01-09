//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "featurebitboard.h"

#include "../core/internal/bitboard.h"

namespace nshogi {
namespace ml {

namespace {

template <typename IterateType>
inline void extractImpl(float* Dest,
                        const core::internal::bitboard::Bitboard& Bitboard,
                        float Value) {
    IterateType SquareIt;

    int Counter = 0;
    for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
        Dest[Counter] = (Bitboard.isSet(*It)) ? Value : 0;
        ++Counter;
    }
}

} // namespace

template <core::IterateOrder Order>
void FeatureBitboard::extract(float* Dest) const {
    const core::internal::bitboard::Bitboard Bitboard =
        *reinterpret_cast<const core::internal::bitboard::Bitboard*>(C);

    if (C[11] == 0) { // no rotation.
        extractImpl<core::SquareIterator<Order>>(Dest, Bitboard, F[3]);
    } else {
        extractImpl<core::SquareIterator<core::reverseOrder<Order>()>>(
            Dest, Bitboard, F[3]);
    }
}

template void
FeatureBitboard::extract<core::IterateOrder::ESWN>(float* Dest) const;
template void
FeatureBitboard::extract<core::IterateOrder::NWSE>(float* Dest) const;
template void
FeatureBitboard::extract<core::IterateOrder::SENW>(float* Dest) const;
template void
FeatureBitboard::extract<core::IterateOrder::WNES>(float* Dest) const;

} // namespace ml
} // namespace nshogi
