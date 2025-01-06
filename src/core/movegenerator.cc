//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "movegenerator.h"
#include "internal/movegenerator.h"
#include "internal/stateadapter.h"

namespace nshogi {
namespace core {

using namespace internal;

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) {
    return MoveGeneratorInternal::generateLegalMoves<C, WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalMoves(const State& S) {
    return MoveGeneratorInternal::generateLegalMoves<WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <Color C, bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) {
    return MoveGeneratorInternal::generateLegalCheckMoves<C, WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template <bool WilyPromote>
MoveList MoveGenerator::generateLegalCheckMoves(const State& S) {
    return MoveGeneratorInternal::generateLegalCheckMoves<WilyPromote>(
        *ImmutableStateAdapter(S).get());
}

template MoveList
MoveGenerator::generateLegalMoves<Black, false>(const State& S);
template MoveList
MoveGenerator::generateLegalMoves<Black, true>(const State& S);
template MoveList
MoveGenerator::generateLegalMoves<White, false>(const State& S);
template MoveList
MoveGenerator::generateLegalMoves<White, true>(const State& S);

template MoveList
MoveGenerator::generateLegalCheckMoves<Black, false>(const State& S);
template MoveList
MoveGenerator::generateLegalCheckMoves<Black, true>(const State& S);
template MoveList
MoveGenerator::generateLegalCheckMoves<White, false>(const State& S);
template MoveList
MoveGenerator::generateLegalCheckMoves<White, true>(const State& S);

template MoveList MoveGenerator::generateLegalMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalMoves<true>(const State& S);

template MoveList MoveGenerator::generateLegalCheckMoves<false>(const State& S);
template MoveList MoveGenerator::generateLegalCheckMoves<true>(const State& S);

} // namespace core
} // namespace nshogi
