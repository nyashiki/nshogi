#include "statebuilder.h"
#include "positionbuilder.h"

namespace nshogi {
namespace core {

StateBuilder::StateBuilder(const Position& Pos)
    : Instance(Pos) {
    Instance.refresh();
}

StateBuilder::StateBuilder(const Position& Pos, uint16_t Ply)
    : Instance(Pos, Ply) {
    Instance.refresh();
}

State StateBuilder::getInitialState() {
    StateBuilder Builder(PositionBuilder::getInitialPosition());

    return Builder.build();
}

State StateBuilder::newState(const Position& Pos) {
    StateBuilder Builder(Pos);
    return Builder.build();
}

State StateBuilder::newState(const Position& Pos, uint16_t Ply) {
    StateBuilder Builder(Pos, Ply);
    return Builder.build();
}

} // namespace core
} // namespace nshogi
