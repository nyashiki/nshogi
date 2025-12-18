//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_STATE_H
#define NSHOGI_CORE_STATE_H

#include <memory>

#include <cassert>
#include <cstdint>

#include "position.h"
#include "stateconfig.h"
#include "types.h"

namespace nshogi {
namespace core {

namespace internal {

class StateImpl;
class ImmutableStateAdapter;
class MutableStateAdapter;

} // namespace internal

///
/// @class State
/// @brief Manages game states in Shogi.
///
/// This class is responsible for controlling and maintaining
/// the game state in Shogi. It includes functionality for tracking
/// the side to move, the current piece locations (refer to the
/// `Position` class for more details), applying a legal move,
/// maintaining the game history, and handling the declaration rule.
///
class State {
 public:
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = delete;

    State(State&&) noexcept;

    ~State();

    ///
    /// @brief Make a clone.
    ///
    State clone() const;

    ///
    /// @brief Get the current color to play.
    ///
    Color getSideToMove() const noexcept;

    ///
    /// @brief Get the current position.
    ///
    const Position& getPosition() const noexcept;

    ///
    /// @brief Get the initial position of the state.
    ///
    const Position& getInitialPosition() const noexcept;

    ///
    /// @brief Get the current ply (i.e., how many moves are applied).
    ///
    uint16_t getPly(bool IncludeOffset = true) const noexcept;

    ///
    /// @brief Get the history move.
    /// @param Ply The specific ply to retrieve.
    ///
    Move32 getHistoryMove(uint16_t Ply) const;

    ///
    /// @brief Get the last history move.
    ///
    Move32 getLastMove() const;

    ///
    /// @brief Get the hash value of the state.
    ///
    uint64_t getHash() const noexcept;

    ///
    /// @brief Apply a legal move.
    ///
    void doMove(Move32 Move) noexcept;

    ///
    /// @brief Undo the last move.
    ///
    void undoMove();

    ///
    /// @brief Get the repetition status of the state.
    /// @param Strict If false, detecting one repeated position is sufficient.
    ///               If true, the detection strictly follows the repetition
    ///               rules.
    ///
    RepetitionStatus getRepetitionStatus(bool Strict = false) const noexcept;

    ///
    /// @brief Return true if the king of the current turn is in check.
    ///
    bool isInCheck() const noexcept;

    ///
    /// @brief Get the square location of the king piece.
    /// @param C The color of the king.
    ///
    Square getKingSquare(Color C) const noexcept;

    ///
    /// @brief Convert a `Move16` object to a `Move32` object.
    /// @param M16 The `Move16` object to be converted.
    ///
    Move32 getMove32FromMove16(Move16 M16) const noexcept;

    ///
    /// @brief Compute the piece score used for the declaration rule.
    /// @param C The color of the player.
    /// @param SliderScoreUnit The score assigned to each slider piece.
    /// @param StepScoreUnit The score assigned to each step piece.
    /// @param OnlyInPromotableZone If true, only pieces within the promotable
    /// zone are considered.
    ///
    uint8_t computePieceScore(Color C, uint8_t SliderScoreUnit,
                              uint8_t StepScoreUnit,
                              bool OnlyInPromotableZone) const noexcept;

    ///
    /// @brief Check if the current player can win by declaration.
    ///
    bool canDeclare() const noexcept;

 private:
    State() = delete;
    State(const Position& P);
    State(const Position& P, uint16_t Ply);
    State(const Position& CurrentP, const Position& InitP);

    State(internal::StateImpl* SI);

    internal::StateImpl* Impl;
    friend class StateBuilder;
    friend class internal::ImmutableStateAdapter;
    friend class internal::MutableStateAdapter;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATE_H
