#ifndef NSHOGI_SOLVER_DFS_H
#define NSHOGI_SOLVER_DFS_H

#include "../core/types.h"
#include "../core/state.h"

namespace nshogi {
namespace solver {
namespace dfs {

core::Move32 solve(core::State* S, int limit);

} // namespace dfs
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_DFS_H
