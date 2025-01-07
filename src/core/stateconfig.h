//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_STATECONFIG_H
#define NSHOGI_CORE_STATECONFIG_H

#include "types.h"
#include <cstdint>

namespace nshogi {
namespace core {

struct StateConfig {
    EndingRule Rule = ER_NoRule;
    uint16_t MaxPly = 256;
    float BlackDrawValue = 0.5f;
    float WhiteDrawValue = 0.5f;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATECONFIG_H
