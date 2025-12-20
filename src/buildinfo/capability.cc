//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "capability.h"

namespace nshogi {
namespace build_info {
namespace capability {

bool sse41Available() {
#if defined(USE_SSE41)
    return true;
#endif
    return false;
}

bool avxAvailable() {
#if defined(USE_AVX)
    return true;
#endif
    return false;
}

bool avx2Available() {
#if defined(USE_AVX2)
    return true;
#endif
    return false;
}

bool neonAvailable() {
#if defined(USE_NEON)
    return true;
#endif
    return false;
}

} // namespace capability
} // namespace build_info
} // namespace nshogi
