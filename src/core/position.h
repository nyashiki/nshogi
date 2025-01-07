//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_POSITION_H
#define NSHOGI_CORE_POSITION_H

#include "types.h"
#include <cstdint>

namespace nshogi {
namespace core {

namespace internal {

class StateImpl;

} // namespace internal

///
/// @class Position
/// @brief Represents the positions of pieces in Shogi.
///
/// This class manages the positions of all pieces in Shogi,
/// including standing pieces (i.e., mochigoma in Japanese).
///
struct Position {
 public:
    Position();
    Position(const Position&);
    Position(const Position&, uint16_t Offset);

    ~Position() = default;

    ///
    /// @brief Get the current color to play.
    ///
    constexpr Color sideToMove() const {
        return SideToMove;
    }

    ///
    /// @brief Get the pieces on the specified square.
    /// @param Sq The square to check.
    ///
    constexpr PieceKind pieceOn(Square Sq) const {
        return OnBoard[Sq];
    }

    ///
    /// @brief Get the stands of the specified player.
    /// @tparam C The color of the player whose stands to get.
    ///
    template <Color C>
    constexpr Stands getStand() const {
        return EachStands[C];
    }

    ///
    /// @brief Get the stands of the specified player.
    /// @param C The color of the player whose stands to get.
    ///
    constexpr Stands getStand(Color C) const {
        return EachStands[C];
    }

    ///
    /// @brief Get the ply at the initial position.
    ///
    constexpr uint16_t getPlyOffset() const {
        return PlyOffset;
    }

    ///
    /// @brief Get the count of a stand piece.
    /// @tparam C The color of the player whose stands to get.
    /// @tparam Type The piece type to get.
    ///
    template <Color C, PieceTypeKind Type>
    constexpr uint8_t getStandCount() const {
        return core::getStandCount<Type>(getStand<C>());
    }

    ///
    /// @brief Get the count of a stand piece.
    /// @tparam C The color of the player whose stands to get.
    /// @param Type The piece type to get.
    ///
    template <Color C>
    uint8_t getStandCount(PieceTypeKind Type) const {
        return core::getStandCount(getStand<C>(), Type);
    }

    ///
    /// @brief Get the count of a stand piece.
    /// @param C The color of the player whose stands to get.
    /// @param Type The piece type to get.
    ///
    uint8_t getStandCount(Color C, PieceTypeKind Type) const {
        return core::getStandCount(getStand(C), Type);
    }

    ///
    /// @brief Check if two positions are equal.
    /// @param Pos The position to compare with.
    /// @param IgnorePlyOffset Whether to ignore the ply offset at the initial
    /// position.
    ///
    bool equals(const Position& Pos, bool IgnorePlyOffset = false) const;

 private:
    void changeSideToMove() {
        SideToMove = ~SideToMove;
    }

    void putPiece(Square Sq, PieceKind Piece) {
        OnBoard[Sq] = Piece;
    }

    PieceKind removePiece(Square Sq) {
        assert(OnBoard[Sq] != PK_Empty);

        PieceKind Piece = OnBoard[Sq];
        OnBoard[Sq] = PK_Empty;
        return Piece;
    }

    void incrementStand(Color C, PieceTypeKind Type) {
        EachStands[C] = nshogi::core::incrementStand(EachStands[C], Type);
    }

    void decrementStand(Color C, PieceTypeKind Type) {
        EachStands[C] = nshogi::core::decrementStand(EachStands[C], Type);
    }

    Color SideToMove;
    uint16_t PlyOffset;
    PieceKind OnBoard[NumSquares];
    Stands EachStands[NumColors];

    friend class internal::StateImpl;
    friend class PositionBuilder;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_POSITION_H
