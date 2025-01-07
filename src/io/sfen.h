//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_IO_SFEN_H
#define NSHOGI_IO_SFEN_H

#include "../core/position.h"
#include "../core/positionbuilder.h"
#include "../core/state.h"
#include "../core/statebuilder.h"
#include "../core/types.h"

#include <string>

namespace nshogi {
namespace io {
namespace sfen {

char fileToChar(core::File File);
char rankToChar(core::Rank Rank);
core::Rank charToRank(char C);
core::File charToFile(char C);

std::string pieceToSfen(core::PieceKind Piece);
core::PieceKind charToPiece(char C, bool Promote) noexcept;

std::string squareToSfen(core::Square Sq);

std::string positionToSfen(const core::Position& Pos, uint16_t Ply = 0);
std::string stateToSfen(const core::State& State);

std::string move16ToSfen(core::Move16 Move);
std::string move32ToSfen(core::Move32 Move);
core::Move32 sfenToMove32(const core::Position& Pos, const std::string& Sfen);

class PositionBuilder : public core::PositionBuilder {
 public:
    static core::Position newPosition(const std::string& Sfen);
};

class StateBuilder : public core::StateBuilder {
 public:
    static core::State newState(const std::string& Sfen);

 private:
    StateBuilder(const std::string& Sfen);
};

} // namespace sfen
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_SFEN_H
