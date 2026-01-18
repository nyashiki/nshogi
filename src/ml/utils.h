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
#include <vector>

namespace nshogi {
namespace ml {
namespace utils {

class PermutationGenerator {
 public:
    PermutationGenerator(uint64_t Seed, std::size_t Size);
    uint64_t operator()(uint64_t X) const;

 private:
    std::vector<uint64_t> Indices;
};

} // namespace utils
} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_UTILS_H
