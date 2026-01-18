//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "utils.h"

#include <algorithm>
#include <random>

namespace nshogi {
namespace ml {
namespace utils {

PermutationGenerator::PermutationGenerator(uint64_t Seed, std::size_t Size) {
    std::mt19937_64 MT(Seed);
    Indices.resize(Size);
    for (std::size_t I = 0; I < Size; ++I) {
        Indices[I] = I;
    }
    std::shuffle(Indices.begin(), Indices.end(), MT);
}

std::size_t PermutationGenerator::operator()(std::size_t X) const {
    return Indices[X];
}

} // namespace utils
} // namespace ml
} // namespace nshogi
