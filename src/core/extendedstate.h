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
    /// The exchange is simulated by letting both sides repeatedly capture
    /// on the destination square with their least valuable attacker, where
    /// each side may also stop capturing at any point. A capture promotes
    /// whenever possible, and the promotion gain is included in the score.
    /// Pins and discovered checks are respected as long as they arise from
    /// the slider lines that are already effective at the current position:
    /// a piece pinned at the current position stays pinned until its pinner
    /// leaves its original square, and when a capture delivers a discovered
    /// check along such a line, only the king may recapture.
    ///
    /// @note This function knows only the slider lines that are effective
    /// at the current position. A slider line toward either king that is
    /// blocked by two or more pieces at the current position may become an
    /// effective pin or check line during the exchange once the blocking
    /// pieces leave their squares; such lines are not taken into account.
    /// Therefore, when an exchange makes such a line effective, this
    /// function may not return the correct value (this is exactly the
    /// condition under which the equivalence test
    /// `ExtendedState.ComputeSEEMatchesSmallestMoveIteration` tolerates
    /// a mismatch).
    ///
    /// @param Move The move to evaluate. It must be a legal capture move
    /// (a board move that captures an opponent's piece) in the current
    /// position.
    /// @param PieceValues An array of piece values used for the SEE
    /// calculation. The array should be indexed by `PieceTypeKind` and contain
    /// the values of each piece type (i.e., `PieceValues[PTK_Pawn]` should give
    /// the value of a pawn). The promoted piece types must not be valued less
    /// than their corresponding raw piece types.
    ///
    /// @return The computed SEE value for the move.
    ///
    int32_t computeSEE(Move32 Move,
                       const int32_t* const PieceValues) const noexcept;

    ///
    /// @brief Test whether the Static Exchange Evaluation (SEE) of a given
    /// move is greater than or equal to a threshold.
    ///
    /// This function is equivalent to
    /// `computeSEE(Move, PieceValues) >= Threshold`, but it stops
    /// simulating the exchange as soon as the outcome of the comparison
    /// is decided, so it can be faster than computing the exact SEE value
    /// when only the comparison result is needed (e.g. for pruning
    /// decisions in a search). All the notes on `computeSEE()` apply to
    /// this function as well.
    ///
    /// @param Move The move to evaluate. It must be a legal capture move
    /// (a board move that captures an opponent's piece) in the current
    /// position.
    /// @param PieceValues An array of piece values used for the SEE
    /// calculation. See `computeSEE()` for the requirements.
    /// @param Threshold The threshold to compare the SEE value against.
    ///
    /// @return True if the SEE value of the move is greater than or equal
    /// to `Threshold`, and false otherwise.
    ///
    bool computeSEEGE(Move32 Move, const int32_t* const PieceValues,
                      int32_t Threshold) const noexcept;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_EXTENDEDSTATE_H
