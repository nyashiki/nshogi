#include "dfs.h"
#include "mate1ply.h"
#include "../core/movegenerator.h"

namespace nshogi {
namespace solver {
namespace dfs {

namespace {

template<core::Color C>
core::Move32 defence(core::State* S, int Limit);

template<core::Color C>
core::Move32 attack(core::State* S, int Limit) {
    if (Limit < 1) {
        return core::Move32::MoveNone();
    }

    const auto Move1Ply = mate1ply::solve<C>(*S);

    if (!Move1Ply.isNone()) {
        return Move1Ply;
    }

    const auto CheckMoves = [&]() {
        if (Limit > 3 && S->getStandCount<C, core::PTK_Pawn>() > 0) {
            // Generate non-promoting moves to avoid utifu-dume rule.
            return core::MoveGenerator::generateLegalCheckMoves<C, false>(*S);
        } else {
            return core::MoveGenerator::generateLegalCheckMoves<C, true>(*S);
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
core::Move32 defence(core::State* S, int Limit) {
    const auto DefenceMoves = core::MoveGenerator::generateLegalMoves<C, true>(*S);

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
    if (S->getSideToMove() == core::Black) {
        return attack<core::Black>(S, Limit);
    } else {
        return attack<core::White>(S, Limit);
    }
}

} // namespace dfs
} // namespace solver
} // namespace nshogi
