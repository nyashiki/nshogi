#ifndef NSHOGI_CORE_INTERNAL_MOVEGENERATOR_H
#define NSHOGI_CORE_INTERNAL_MOVEGENERATOR_H

#include "stateimpl.h"
#include "../movelist.h"

namespace nshogi {
namespace core {
namespace internal {

class MoveGeneratorInternal {
 public:
    MoveGeneratorInternal() = delete;

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalMoves(const internal::StateImpl& S);

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const internal::StateImpl& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalMoves(const internal::StateImpl& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const internal::StateImpl& S);
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_MOVEGENERATOR_H
