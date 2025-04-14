//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_C_API_STATE_H
#define NSHOGI_C_API_STATE_H

#include "../c_api.h"

namespace nshogi {
namespace c_api {
namespace state {

nshogi_state_api_t* getApi();

} // namespace state
} // namespace c_api
} // namespace nshogi

#endif // #ifndef NSHOGI_C_API_STATE_H
