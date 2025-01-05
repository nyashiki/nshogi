//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfs.h"
#include "internal/mate1ply.h"
#include "../core/internal/movegenerator.h"
#include "../core/internal/stateimpl.h"
#include "../core/internal/stateadapter.h"

namespace nshogi {
namespace solver {
namespace dfs {

namespace {

template<core::Color C>
core::Move32 defence(core::internal::StateImpl* S, int Limit);

template<core::Color C>
core::Move32 attack(core::internal::StateImpl* S, int Limit) {
    if (Limit < 1) {
        return core::Move32::MoveNone();
    }

    const auto Move1Ply = internal::mate1ply::solve<C>(*S);

    if (!Move1Ply.isNone()) {
        return Move1Ply;
    }

    const auto CheckMoves = [&]() {
        if (Limit > 3 && S->getStandCount<C, core::PTK_Pawn>() > 0) {
            // Generate non-promoting moves to avoid utifu-dume rule.
            return core::internal::MoveGeneratorInternal::generateLegalCheckMoves<C, false>(*S);
        } else {
            return core::internal::MoveGeneratorInternal::generateLegalCheckMoves<C, true>(*S);
        }
    }();

    for (const auto& Move : CheckMoves) {
        S->doMove<C>(Move);

        const auto CounterMove = defence<~C>(S, Limit - 1);

        S->undoMove<~C>();

        if (CounterMove.isNone()) {
            return Move;
        }
    }

    return core::Move32::MoveNone();
}

template<core::Color C>
core::Move32 defence(core::internal::StateImpl* S, int Limit) {
    const auto DefenceMoves = core::internal::MoveGeneratorInternal::generateLegalMoves<C, true>(*S);

    bool IsCheckmatedBy1Ply = true;
    for (const auto& Move : DefenceMoves) {
        S->doMove<C>(Move);

        const auto CheckMove = attack<~C>(S, Limit - 1);

        S->undoMove<~C>();

        if (CheckMove.isNone()) {
            return Move;
        }
        IsCheckmatedBy1Ply = false;
    }

    const core::Move32 LastMove = S->getLastMove();
    if (IsCheckmatedBy1Ply && LastMove.drop() && LastMove.pieceType() == core::PTK_Pawn) {
        // Rule: utchifu-dume.
        return core::Move32::MoveWin();
    }

    return core::Move32::MoveNone();
}

} // namespace

core::Move32 solve(core::State* S, int Limit) {
    core::internal::MutableStateAdapter Adapter(*S);
    if (S->getSideToMove() == core::Black) {
        return attack<core::Black>(Adapter.get(), Limit);
    } else {
        return attack<core::White>(Adapter.get(), Limit);
    }
}

} // namespace dfs
} // namespace solver
} // namespace nshogi
