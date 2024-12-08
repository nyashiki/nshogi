#ifndef NSHOGI_SOLVER_MATE1PLY_H
#define NSHOGI_SOLVER_MATE1PLY_H


#include "../core/types.h"
#include "../core/state.h"

namespace nshogi {

namespace core {
namespace internal {

class StateImpl;

} // namespace internal
} // namespace core

namespace solver {
namespace mate1ply {

namespace internal {

template <core::Color C>
core::Move32 solve(const core::internal::StateImpl& S);

} // namespace internal

template <core::Color C>
core::Move32 solve(const core::State& S);

/// Returns Move32::MoveNone() if there is any checkmate move
/// within one ply. Otherwise, returns a one-ply-checkmate move.
core::Move32 solve(const core::State& S);


} // namespace mate1ply
} // namespace solver
} // namespace nshogi


#endif // #ifndef NSHOGI_SOLVER_MATE1PLY_H
