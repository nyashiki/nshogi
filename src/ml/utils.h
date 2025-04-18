//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_UTILS_H
#define NSHOGI_ML_UTILS_H

#include <cinttypes>

namespace nshogi {
namespace ml {
namespace utils {

class PermutationGenerator {
 public:
    PermutationGenerator(uint64_t Seed, uint64_t MaxValue);

    uint64_t generate(uint64_t X) const;
    uint64_t operator()(uint64_t X) const;

 private:
    const uint64_t Seed_;
    const uint64_t MaxValue_;
    const uint64_t Multiplier_;

    bool isPrime(uint64_t N) const;
    uint64_t findPrimeNumber() const;
};

} // namespace utils
} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_UTILS_H
