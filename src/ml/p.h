//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_P_H
#define NSHOGI_ML_P_H

#include "../core/state.h"

#include <cstdint>
#include <vector>

namespace nshogi {
namespace ml {

class PFeatureExtractor {
 public:
    PFeatureExtractor();

    static void idsAt(
        int32_t* DestMyIds,
        int32_t* DestOpIds,
        int32_t* DestMyIdsCount,
        int32_t* DestOpIdsCount,
        const core::State& S
    );

    auto ids(const core::State&) const
        -> std::pair<std::vector<int32_t>, std::vector<int32_t>>;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_P_H
