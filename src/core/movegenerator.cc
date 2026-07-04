//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "movegenerator.h"
#include "internal/movegenerator.h"
#include "internal/stateadapter.h"

#include <cassert>

namespace nshogi {
namespace core {

using namespace internal;

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) noexcept {
    return MoveGeneratorInternal::generateLegalMoves<C, WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) noexcept {
    if (S.getSideToMove() == Black) {
        return generateLegalMoves<Black, WilyPromote>(S);
    } else {
        return generateLegalMoves<White, WilyPromote>(S);
    }
}

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) noexcept {
    return MoveGeneratorInternal::generateLegalCheckMoves<C, WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <Color C>
Move32 MoveGenerator::generateLegalSmallestMove(const State& S,
                                                Square To) noexcept {
    // The destination square must be empty or occupied by an opponent's piece.
    assert(S.getPosition().pieceOn(To) == PK_Empty ||
           getColor(S.getPosition().pieceOn(To)) == ~S.getSideToMove());

    return MoveGeneratorInternal::generateLegalSmallestMove<C>(
        *ImmutableStateAdapter(S).get(), To);
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) noexcept {
    if (S.getSideToMove() == Black) {
        return generateLegalCheckMoves<Black, WilyPromote>(S);
    } else {
        return generateLegalCheckMoves<White, WilyPromote>(S);
    }
}

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalCaptureMoves(const State& S) noexcept {
    return MoveGeneratorInternal::generateLegalCaptureMoves<C, WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalCaptureMoves(const State& S) noexcept {
    if (S.getSideToMove() == Black) {
        return generateLegalCaptureMoves<Black, WilyPromote>(S);
    } else {
        return generateLegalCaptureMoves<White, WilyPromote>(S);
    }
}

Move32 MoveGenerator::generateLegalSmallestMove(const State& S,
                                                Square To) noexcept {
    if (S.getSideToMove() == Black) {
        return generateLegalSmallestMove<Black>(S, To);
    } else {
        return generateLegalSmallestMove<White>(S, To);
    }
}

template MoveList
MoveGenerator::generateLegalMoves<Black, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalMoves<Black, true>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalMoves<White, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalMoves<White, true>(const State& S) noexcept;

template MoveList
MoveGenerator::generateLegalCheckMoves<Black, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCheckMoves<Black, true>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCheckMoves<White, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCheckMoves<White, true>(const State& S) noexcept;

template MoveList
MoveGenerator::generateLegalCaptureMoves<Black, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCaptureMoves<Black, true>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCaptureMoves<White, false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCaptureMoves<White, true>(const State& S) noexcept;

template MoveList
MoveGenerator::generateLegalMoves<false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalMoves<true>(const State& S) noexcept;

template MoveList
MoveGenerator::generateLegalCheckMoves<false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCheckMoves<true>(const State& S) noexcept;

template MoveList
MoveGenerator::generateLegalCaptureMoves<false>(const State& S) noexcept;
template MoveList
MoveGenerator::generateLegalCaptureMoves<true>(const State& S) noexcept;

template Move32
MoveGenerator::generateLegalSmallestMove<Black>(const State& S,
                                                Square To) noexcept;
template Move32
MoveGenerator::generateLegalSmallestMove<White>(const State& S,
                                                Square To) noexcept;

} // namespace core
} // namespace nshogi
