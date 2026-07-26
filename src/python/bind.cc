//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include <pybind11/pybind11.h>

#include "core.h"
#include "feature.h"
#include "io.h"
#include "solver.h"
#include "teacher.h"

#include "../core/initializer.h"

PYBIND11_MODULE(nshogi, Module) {
    Module.doc() = "";
    Module.attr("__version__") = VERSION;

    nshogi::core::initializer::initializeAll();

    nshogi::python::core::bind(Module);

    auto SolverModule = Module.def_submodule("solver");
    nshogi::python::solver::bind(SolverModule);

    auto IOModule = Module.def_submodule("io");
    nshogi::python::io::bind(IOModule);

    auto MLModule = Module.def_submodule("ml");
    nshogi::python::feature::bind(MLModule);
    nshogi::python::teacher::bind(MLModule);
}
