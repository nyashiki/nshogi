//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_IO_CSA_H
#define NSHOGI_IO_CSA_H

#include "../core/position.h"
#include "../core/positionbuilder.h"
#include "../core/state.h"
#include "../core/statebuilder.h"
#include "../core/types.h"

#include <string>

namespace nshogi {
namespace io {
namespace csa {

char fileToChar(core::File File);
char rankToChar(core::File File);
core::Rank charToRank(char C);
core::File charToFile(char C);

std::string pieceToCSA(core::PieceKind Piece);
core::PieceKind charToPiece(char C, bool Promote) noexcept;

std::string move32ToCSA(core::Move32 Move, core::Color Color);
core::Move32 CSAToMove32(const core::Position& Pos, const std::string& CSA);

std::string squareToCSA(core::Square Sq);

std::string positionToCSA(const core::Position& Pos);
std::string stateToCSA(const core::State& State);

class PositionBuilder : public core::PositionBuilder {
 public:
    static core::Position newPosition(const std::string& CSA);

 private:
    PositionBuilder() = default;
    PositionBuilder(const core::Position& Position);
};

class StateBuilder : public core::StateBuilder {
 public:
    static core::State newState(const std::string& CSA);

 private:
    StateBuilder(const std::string& CSA);
};

} // namespace csa
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_CSA_H
