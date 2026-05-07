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

#include <vector>

namespace nshogi {
namespace ml {

class PFeatureExtractor {
 public:
    PFeatureExtractor();

    auto ids(const core::State&) const -> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>;
    std::vector<int8_t> extract(const core::State&) const;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_P_H
