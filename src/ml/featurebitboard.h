//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_FEATUREBITBOARD_H
#define NSHOGI_ML_FEATUREBITBOARD_H

#include "../core/squareiterator.h"

#include <cstring>

namespace nshogi {
namespace ml {

namespace internal {

class FeatureBitboardUtil;

} // namespace internal

struct FeatureBitboard {
 public:
    FeatureBitboard() {
    }

    FeatureBitboard(const FeatureBitboard& FB) {
        std::memcpy(reinterpret_cast<void*>(this), &FB, sizeof(*this));
    }

    FeatureBitboard& operator=(const FeatureBitboard& FB) {
        std::memcpy(reinterpret_cast<void*>(this), &FB, sizeof(*this));
        return *this;
    }

    float getValue() const {
        return F[3];
    }

    bool isRotated() const {
        return C[11];
    }

    const char* data() const {
        return reinterpret_cast<const char*>(C);
    }

    template <core::IterateOrder Order>
    void extract(float* Dest) const;

 private:
    union {
        float F[4];    // use F[3] for the value of a neural network channel.
        uint8_t C[16]; // use C[11] for the 180 degree rotation of the board.
    };

    friend class nshogi::ml::internal::FeatureBitboardUtil;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_FEATUREBITBOARD_H
