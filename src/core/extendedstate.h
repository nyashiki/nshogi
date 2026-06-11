//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_EXTENDEDSTATE_H
#define NSHOGI_CORE_EXTENDEDSTATE_H

#include "state.h"

namespace nshogi {
namespace core {

///
/// @class ExtendedState
/// @brief An extended version of the `State` class with additional
/// functionalities.
///
/// This class is an extended version of the `State` class,
/// providing additional functionalities that are not essential for representing
/// the game state but may be useful in certain contexts. For example, it
/// includes methods for performing and undoing null moves, which can be
/// beneficial for search algorithms in game engines.
///
class ExtendedState : public State {
 public:
    ExtendedState(State&&) noexcept;
    ~ExtendedState();

    ///
    /// @brief Perform a null move.
    ///
    void doNullMove() noexcept;

    ///
    /// @brief Undo a null move.
    ///
    void undoNullMove();

    ///
    /// @brief Compute the Static Exchange Evaluation (SEE) for a given move.
    ///
    /// @param Move The move to evaluate. It must be a legal move in the
    /// current position.
    /// @param PieceValues An array of piece values used for the SEE calculation.
    /// The array should be indexed by `PieceTypeKind` and contain the values of
    /// each piece type (i.e., `PieceValues[PTK_Pawn]` should give the value of a pawn).
    ///
    /// @return The computed SEE value for the move.
    ///
    int32_t computeSEE(Move32 Move, const int32_t* const PieceValues) const noexcept;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_EXTENDEDSTATE_H
