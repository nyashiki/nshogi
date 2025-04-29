//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "utils.h"

#include <random>

namespace nshogi {
namespace ml {
namespace utils {

PermutationGenerator::PermutationGenerator(uint64_t Seed, uint64_t MaxValue)
    : Seed_(Seed % MaxValue)
    , MaxValue_(MaxValue)
    , Multiplier_(findPrimeNumber()) {
}

uint64_t PermutationGenerator::generate(uint64_t X) const {
    return (X * Multiplier_ + Seed_) % MaxValue_;
}

uint64_t PermutationGenerator::operator()(uint64_t X) const {
    return generate(X);
}

bool PermutationGenerator::isPrime(uint64_t N) const {
    if (N == 1) {
        return false;
    }

    if (N == 2) {
        return true;
    }

    for (uint64_t I = 3; I * I <= N; ++I) {
        if (N % I == 0) {
            return false;
        }
    }

    return true;
}

uint64_t PermutationGenerator::findPrimeNumber() const {
    uint64_t DistanceMax = 0;
    uint64_t Candidate = 0;

    std::mt19937_64 MT(Seed_);

    for (uint64_t I = 0; I < 10000; ++I) {
        const uint64_t N = MT() % MaxValue_;

        if (!isPrime(N) || MaxValue_ % N == 0) {
            continue;
        }

        const uint64_t Distance =
            (N > MaxValue_ / 2) ? (N - MaxValue_ / 2) : (MaxValue_ / 2 - N);

        if (Distance > DistanceMax) {
            DistanceMax = Distance;
            Candidate = N;
        }
    }

    return Candidate;
}

} // namespace utils
} // namespace ml
} // namespace nshogi
