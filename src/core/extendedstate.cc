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

} // namespace core
} // namespace nshogi
