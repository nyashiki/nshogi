#include "../core/movegenerator.h"
#include "../core/position.h"
#include "../io/sfen.h"

#include <iostream>
#include <string>

void benchMoveGeneration(const nshogi::core::State& State) {
    [[maybe_unused]]
    volatile const auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(State);

    // const auto Moves = nshogi::core::MoveGenerator::generateLegalCheckMoves(State);
    // for (const auto& Move : Moves) {
    //     std::cout << nshogi::io::sfen::move32ToSfen(Move) << ", ";
    // }
    // std::cout << std::endl;
    // std::cout << "Moves.size(): " << Moves.size() << std::endl;
    // std::cout << nshogi::io::sfen::positionToSfen(State.getPosition())
    //           << std::endl;
}
