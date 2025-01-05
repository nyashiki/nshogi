//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CAPABILITY_H
#define NSHOGI_CAPABILITY_H

namespace nshogi {
namespace build_info {
namespace capability {

bool sse2Available();
bool sse41Available();
bool avxAvailable();
bool avx2Available();
bool neonAvailable();

} // namespace build_info
} // namespace capability
} // namespace

#endif // #ifndef NSHOGI_CAPABILITY_H
