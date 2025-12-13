#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/statebuilder.h"
#include <cstdio>

namespace {

uint64_t perftImpl(nshogi::core::State& State, int Limit) {
    if (Limit == 0) {
        return 1;
    }

    const auto Moves =
        nshogi::core::MoveGenerator::generateLegalMoves<false>(State);

    if (Limit == 1) {
        return (uint64_t)Moves.size();
    }

    uint64_t Sum = 0;
    for (const auto& Move : Moves) {
        State.doMove(Move);
        Sum += perftImpl(State, Limit - 1);
        State.undoMove();
    }

    return Sum;
}

} // namespace

void benchPerft(int Ply) {
    nshogi::core::State State = nshogi::core::StateBuilder::getInitialState();

    [[maybe_unused]]
    volatile const auto Result = perftImpl(State, Ply);

    // printf("%d: %lu\n", Ply, Result);
}
