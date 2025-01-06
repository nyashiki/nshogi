//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_INTERNAL_HASH_H
#define NSHOGI_CORE_INTERNAL_HASH_H

#include "../position.h"
#include "../types.h"
#include <cinttypes>

namespace nshogi {
namespace core {
namespace internal {

template <typename HashValueType>
struct Hash {
 public:
    Hash();
    Hash(const Hash<HashValueType>&);

    Hash& operator=(const Hash& H) {
        Value = H.Value;
        return *this;
    }

    void clear();
    void refresh(const Position& Pos);

    template <Color C>
    void update(PieceTypeKind Type, Square Sq) {
        Value ^= OnBoardHash[C][Type][Sq];
    }

    inline void updateColor() {
        Value ^= ColorHash;
    }

    inline HashValueType getValue() const {
        return Value;
    }

 private:
    HashValueType Value;

    static bool IsTableGenerated;
    static HashValueType OnBoardHash[NumColors][NumPieceType][NumSquares];
    static HashValueType ColorHash;
};

template <typename HashValueType>
bool Hash<HashValueType>::IsTableGenerated = false;
template <typename HashValueType>
HashValueType Hash<HashValueType>::OnBoardHash[NumColors][NumPieceType]
                                              [NumSquares];
template <typename HashValueType>
HashValueType Hash<HashValueType>::ColorHash;

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_HASH_H
