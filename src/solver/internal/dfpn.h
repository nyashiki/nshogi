//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../../core/internal/stateimpl.h"
#include "../../core/types.h"

#ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
#define NSHOGI_SOLVER_INTERNAL_DFPN_H

namespace nshogi {
namespace solver {
namespace internal {
namespace dfpn {

template <core::Color C>
core::Move32 solve(core::internal::StateImpl* S);

} // namespace dfpn
} // namespaec internal
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
