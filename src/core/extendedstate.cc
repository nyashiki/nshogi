#include "extendedstate.h"
#include "internal/stateimpl.h"

namespace nshogi {
namespace core {

ExtendedState::ExtendedState(State&& S) noexcept
    : State(std::move(S)) {
}

ExtendedState::~ExtendedState() {
}

void ExtendedState::doNullMove() noexcept {
    Impl->doNullMove();
}

void ExtendedState::undoNullMove() {
    Impl->undoNullMove();
}

int32_t
ExtendedState::computeSEE(Move32 Move,
                          const int32_t* const PieceValues) const noexcept {
    return Impl->computeSEE(Move, PieceValues);
}

bool ExtendedState::computeSEEGE(Move32 Move, const int32_t* const PieceValues,
                                 int32_t Threshold) const noexcept {
    return Impl->computeSEEGE(Move, PieceValues, Threshold);
}

} // namespace core
} // namespace nshogi
