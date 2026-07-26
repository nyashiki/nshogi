#include "../core/internal/stateimpl.h"
#include "../core/movegenerator.h"
#include "../core/position.h"
#include "../io/sfen.h"
#include "common.hpp"

#include <iostream>
#include <string>
#include <vector>

void benchMoveGeneration(const nshogi::core::State& State) {
    const auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(State);
    nshogi::bench::doNotOptimize(Moves);

    // const auto Moves =
    // nshogi::core::MoveGenerator::generateLegalCheckMoves(State); for (const
    // auto& Move : Moves) {
    //     std::cout << nshogi::io::sfen::move32ToSfen(Move) << ", ";
    // }
    // std::cout << std::endl;
    // std::cout << "Moves.size(): " << Moves.size() << std::endl;
    // std::cout << nshogi::io::sfen::positionToSfen(State.getPosition())
    //           << std::endl;
}

void benchMoveGenerationSet(const std::vector<nshogi::core::State>& States) {
    for (const auto& State : States) {
        const auto Moves =
            nshogi::core::MoveGenerator::generateLegalMoves(State);
        nshogi::bench::doNotOptimize(Moves);
    }
}

// void benchMoveGenerationInternal(const nshogi::core::internal::StateImpl&
// State) {
//     [[maybe_unused]]
//     volatile const auto Moves =
//     nshogi::core::MoveGeneratorInternal::generateLegalMoves(State);
//
//     // const auto Moves =
//     nshogi::core::MoveGenerator::generateLegalCheckMoves(State);
//     // for (const auto& Move : Moves) {
//     //     std::cout << nshogi::io::sfen::move32ToSfen(Move) << ", ";
//     // }
//     // std::cout << std::endl;
//     // std::cout << "Moves.size(): " << Moves.size() << std::endl;
//     // std::cout << nshogi::io::sfen::positionToSfen(State.getPosition())
//     //           << std::endl;
// }
