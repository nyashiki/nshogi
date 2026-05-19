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

int32_t ExtendedState::computeSEE(
    Square To, const int32_t* const PieceValues) const noexcept {
    return Impl->computeSEE(To, PieceValues);
}

} // namespace core
} // namespace nshogi
