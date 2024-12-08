#ifndef NSHOGI_CORE_INTERNAL_STATEADAPTER_H
#define NSHOGI_CORE_INTERNAL_STATEADAPTER_H

#include "../state.h"
#include "stateimpl.h"

namespace nshogi {
namespace core {
namespace internal {

class ImmutableStateAdapter {
 public:
    ImmutableStateAdapter(const State& S)
        : Impl(S.Impl) {
    }

    const StateImpl* operator->() const {
        return Impl;
    }

    const StateImpl* get() const {
        return Impl;
    }

 private:
    const StateImpl* const Impl;
};

class MutableStateAdapter {
 public:
    MutableStateAdapter(State& S)
        : Impl(S.Impl) {
    }

    StateImpl* operator->() const {
        return Impl;
    }

    StateImpl* get() const {
        return Impl;
    }

 private:
    StateImpl* const Impl;
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_STATEADAPTER_H
