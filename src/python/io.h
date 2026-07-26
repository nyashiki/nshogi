//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_PYTHON_IO_H
#define NSHOGI_PYTHON_IO_H

#include <pybind11/pybind11.h>

namespace nshogi {
namespace python {
namespace io {

void bind(pybind11::module_& Module);

} // namespace io
} // namespace python
} // namespace nshogi

#endif // #ifndef NSHOGI_PYTHON_IO_H
