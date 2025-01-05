//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_INTERNAL_FEATUREBITBOARDUTIL_H
#define NSHOGI_ML_INTERNAL_FEATUREBITBOARDUTIL_H

#include "../../core/internal/bitboard.h"
#include "../featurebitboard.h"

namespace nshogi {
namespace ml {
namespace internal {

class FeatureBitboardUtil {
 public:
    FeatureBitboardUtil() = delete;

    inline
    static FeatureBitboard makeFeatureBitboard(const core::internal::bitboard::Bitboard& BB, float Value, bool Rotation) {
        FeatureBitboard FB;

        *reinterpret_cast<core::internal::bitboard::Bitboard*>(FB.C) = BB;
        FB.F[3] = Value;
        FB.C[11] = Rotation;

        return FB;
    }

    inline
    static core::internal::bitboard::Bitboard getBitboard(const FeatureBitboard& FB) {
        const core::internal::bitboard::Bitboard* BB = reinterpret_cast<const core::internal::bitboard::Bitboard*>(FB.C);
        return *BB & core::internal::bitboard::Bitboard::AllBB();
    }
};

} // namespace internal
} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_INTERNAL_FEATUREBITBOARDUTIL_H
