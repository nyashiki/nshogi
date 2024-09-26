#include "../core/state.h"
#include "../core/movegenerator.h"

extern "C" {

#include "movegenerator.h"

uint16_t nshogi_generate_legal_moves(const State* St, Move32* Ms) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);

    const auto LegalMoves = nshogi::core::MoveGenerator::generateLegalMoves<false>(*S);
    for (nshogi::core::Move32 Move : LegalMoves) {
        *Ms++ = Move.value();
    }

    return (uint16_t)LegalMoves.size();
}

uint16_t nshogi_generate_legal_moves_pruned(const State* St, Move32* Ms) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);

    const auto LegalMoves = nshogi::core::MoveGenerator::generateLegalMoves<true>(*S);
    for (nshogi::core::Move32 Move : LegalMoves) {
        *Ms++ = Move.value();
    }

    return (uint16_t)LegalMoves.size();
}

} // extern "C"
