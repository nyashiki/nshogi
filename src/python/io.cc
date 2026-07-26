//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "io.h"

#include <pybind11/pybind11.h>

#include "../core/state.h"
#include "../io/csa.h"
#include "../io/sfen.h"

#include <string>

namespace nshogi {
namespace python {
namespace io {

void bind(pybind11::module_& Module) {
    auto SfenModule = Module.def_submodule("sfen");

    SfenModule.def("make_state_from_sfen", [](const std::string& Sfen) {
        return nshogi::io::sfen::StateBuilder::newState(Sfen);
    });

    SfenModule.def("stringify", [](nshogi::core::Move32 Move) {
        return nshogi::io::sfen::move32ToSfen(Move);
    });

    SfenModule.def("move_from_sfen", [](const nshogi::core::State& State,
                                        const std::string& Sfen) {
        return nshogi::io::sfen::sfenToMove32(State.getPosition(), Sfen);
    });

    auto CSAModule = Module.def_submodule("csa");

    CSAModule.def("make_state_from_csa", [](const std::string& CSA) {
        return nshogi::io::csa::StateBuilder::newState(CSA);
    });

    CSAModule.def("stringify", [](nshogi::core::Move32 Move,
                                  const nshogi::core::Color Color) {
        return nshogi::io::csa::move32ToCSA(Move, Color);
    });

    CSAModule.def("move_from_csa", [](const nshogi::core::State& State,
                                      const std::string& CSA) {
        return nshogi::io::csa::CSAToMove32(State.getPosition(), CSA);
    });
}

} // namespace io
} // namespace python
} // namespace nshogi
