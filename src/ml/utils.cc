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
#include <stdexcept>

namespace nshogi {
namespace ml {
namespace utils {

PermutationGenerator::PermutationGenerator(uint64_t Seed, uint64_t MaxValue)
    : Seed_([Seed, MaxValue] {
        if (MaxValue == 0) {
            throw std::runtime_error("MaxValue must not be 0.");
        }
        return Seed % MaxValue;
    }())
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
    if (N == 0 || N == 1) {
        return false;
    }

    if (N == 2) {
        return true;
    }

    for (uint64_t I = 2; I * I <= N; ++I) {
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

    if (MaxValue_ <= 3) {
        return 1;
    }

    for (uint64_t I = 0; I < 10000; ++I) {
        uint64_t N = MT() % MaxValue_;
        if (N % 2 == 0) {
            ++N;
        }

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

    if (Candidate == 0) {
        throw std::runtime_error("Failed to find a prime number().");
    }
    return Candidate;
}

} // namespace utils
} // namespace ml
} // namespace nshogi
