//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_MOVEGENERATOR_H
#define NSHOGI_CORE_MOVEGENERATOR_H

#include "movelist.h"
#include "state.h"

namespace nshogi {
namespace core {

class MoveGenerator {
 public:
    MoveGenerator() = delete;

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_MOVEGENERATOR_H
