#include "../solver/mate1ply.h"
#include <vector>

void benchMate1ply(const std::vector<nshogi::core::State>& States) {
    for (const auto& State : States) {
        [[maybe_unused]]
        volatile const auto CheckmateMove = nshogi::solver::mate1ply::solve(State);
    }
}
