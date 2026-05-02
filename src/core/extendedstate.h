//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

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
};

} // namespace core
} // namespace nshogi
