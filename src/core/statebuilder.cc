#include "statebuilder.h"
#include "positionbuilder.h"

namespace nshogi {
namespace core {

StateBuilder::StateBuilder(const Position& Pos) : Instance(Pos) {
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

} // namespace core
} // namespace nshogi
