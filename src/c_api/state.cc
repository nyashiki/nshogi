#include "../core/state.h"

extern "C" {

#include "state.h"

Color nshogi_state_get_side_to_move(const State* St) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return (Color)S->getSideToMove();
}

uint16_t nshogi_state_get_ply(const State* St) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return S->getPly();
}

PieceKind nshogi_state_get_piece_on_square(const State* St, int Sq) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return (PieceKind)S->getPosition().pieceOn((nshogi::core::Square)Sq);
}

RepetitionStatus nshogi_state_get_repetition_status(const State* St) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return (RepetitionStatus)S->getRepetitionStatus();
}

uint8_t nshogi_state_get_stand_count(const State* St, Color C, PieceTypeKind Kind) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return S->getStandCount((nshogi::core::Color)C, (nshogi::core::PieceTypeKind)Kind);
}

int8_t nshogi_state_can_declare(const State* St) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(St);
    return (int8_t)S->canDeclare();
}

void nshogi_state_do_move(State* St, Move32 Move) {
    auto* S = reinterpret_cast<nshogi::core::State*>(St);
    S->doMove(nshogi::core::Move32::fromValue(Move));
}

void nshogi_state_undo_move(State* St, Move32* LastMove) {
    auto* S = reinterpret_cast<nshogi::core::State*>(St);
    if (LastMove == nullptr) {
        S->undoMove();
    } else {
        const auto M = S->undoMove();
        *LastMove = M.value();
    }
}

void nshogi_free_state(State** St) {
    auto* S = reinterpret_cast<nshogi::core::State*>(*St);

    if (S != nullptr) {
        delete S;
    }

    *St = nullptr;
}

} // extern "C"
