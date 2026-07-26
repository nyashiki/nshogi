//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_PYTHON_CORE_H
#define NSHOGI_PYTHON_CORE_H

#include <pybind11/pybind11.h>

namespace nshogi {
namespace python {
namespace core {

void bind(pybind11::module_& Module);

} // namespace core
} // namespace python
} // namespace nshogi

#endif // #ifndef NSHOGI_PYTHON_CORE_H
