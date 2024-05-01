#ifndef NSHOGI_CORE_STATEBUILDER_H
#define NSHOGI_CORE_STATEBUILDER_H

#include "state.h"
#include "types.h"
#include <initializer_list>
#include <utility>

namespace nshogi {
namespace core {

class StateBuilder {
 public:
    static State getInitialState();

    State build() {
        return std::move(Instance);
    }

    static core::State newState(const Position&);

 protected:
    StateBuilder(const Position&);
    State Instance;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATEBUILDER_H
