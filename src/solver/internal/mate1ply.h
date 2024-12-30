#ifndef NSHOGI_SOLVER_INTERNAL_MATE1PLY_H
#define NSHOGI_SOLVER_INTERNAL_MATE1PLY_H

#include "../../core/types.h"
#include "../../core/internal/stateimpl.h"

namespace nshogi {
namespace solver {
namespace internal {
namespace mate1ply {

template <core::Color C>
core::Move32 solve(const core::internal::StateImpl& S);

} // namespace mate1ply
} // namespace internal
} // namespace solver
} // namespace nshogi


#endif // #ifndef NSHOGI_SOLVER_MATE1PLY_H
