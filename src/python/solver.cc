//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "solver.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../core/state.h"
#include "../solver/dfpn.h"
#include "../solver/dfs.h"

namespace nshogi {
namespace python {
namespace solver {

void bind(pybind11::module_& Module) {
    Module.def("dfs", &nshogi::solver::dfs::solve);

    pybind11::class_<nshogi::solver::dfpn::Solver>(Module, "DfPn")
        .def(pybind11::init<std::size_t>(), pybind11::arg("memory_mb"))
        .def(
            "solve",
            [](nshogi::solver::dfpn::Solver& Solver, nshogi::core::State& State,
               bool WithPV, uint64_t MaxNodeCount, uint64_t MaxDepth) {
                if (WithPV) {
                    return pybind11::cast(
                        Solver.solveWithPV(&State, MaxNodeCount, MaxDepth));
                } else {
                    return pybind11::cast(
                        Solver.solve(&State, MaxNodeCount, MaxDepth));
                }
            },
            pybind11::arg("state"), pybind11::arg("with_pv") = true,
            pybind11::arg("max_node_count") = 0,
            pybind11::arg("max_depth") = 0);
}

} // namespace solver
} // namespace python
} // namespace nshogi
