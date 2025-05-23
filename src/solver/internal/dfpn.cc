//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfpn.h"

namespace nshogi {
namespace solver {
namespace internal {
namespace dfpn {

namespace {



} // namespace

template <core::Color C>
core::Move32 solve(const core::internal::StateImpl& S) {
}

template
core::Move32 solve<core::Color::Black>(core::internal::StateImpl*);

template
core::Move32 solve<core::Color::White>(core::internal::StateImpl&);

} // namespace dfpn
} // namespaec internal
} // namespace solver
} // namespace nshogi
