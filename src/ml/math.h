//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_MATH_H
#define NSHOGI_ML_MATH_H

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace nshogi {
namespace ml {
namespace math {

template <typename DType>
void softmax_(DType* X, std::size_t N, DType Temperature = 1.0) {
    const DType MaxValue = *std::max_element(X, X + N);
    DType Sum = 0;

    const DType InvTemperature = (DType)1.0 / Temperature;

    for (std::size_t I = 0; I < N; ++I) {
        X[I] = std::exp((X[I] - MaxValue) * InvTemperature);
        Sum += X[I];
    }

    const DType InvSum = (DType)1.0 / Sum;
    for (size_t I = 0; I < N; ++I) {
        X[I] *= InvSum;
    }
}

template <typename DType>
DType sigmoid(DType X) {
    return (DType)1.0 / ((DType)1.0 + std::exp(-X));
}

} // namespace math
} // namespace ml
} // namespace nshogi

#endif // #define NSHOGI_ML_MATH_H
