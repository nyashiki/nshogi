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

///
/// @class MoveGenerator
/// @brief Generates all legal moves for a given state.
///
/// This class is responsible for enumerating all possible moves in a game state,
/// ensuring that each move complies with the rules of Shogi.
///

class MoveGenerator {
 public:
    MoveGenerator() = delete;

    ///
    /// @brief Generate all legal moves for a given state.
    /// @tparam C The color of the side for which moves are begin generated.
    ///         It must match the side to move in the provided state.
    /// @tparam WilyPromote If true, trivial non-promoting moves are ommited
    ///         to reduce the move set.
    ///
    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    ///
    /// @brief Generate all legal check moves for a given state.
    /// @tparam C The color of the side for which moves are begin generated.
    ///         It must match the side to move in the provided state.
    /// @tparam WilyPromote If true, trivial non-promoting moves are ommited
    ///         to reduce the move set.
    ///
    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);

    ///
    /// @brief Generate all legal moves for a given state.
    /// @tparam WilyPromote If true, trivial non-promoting moves are ommited
    ///         to reduce the move set.
    ///
    template <bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    ///
    /// @brief Generate all legal check moves for a given state.
    /// @tparam WilyPromote If true, trivial non-promoting moves are ommited
    ///         to reduce the move set.
    ///
    template <bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_MOVEGENERATOR_H
