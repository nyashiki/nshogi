#ifndef NSHOGI_CORE_MOVEGENERATOR_H
#define NSHOGI_CORE_MOVEGENERATOR_H

#include "movelist.h"
#include "state.h"

namespace nshogi {
namespace core {

namespace internal {

class StateImpl;

} // namespace internal

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

class MoveGenerator {
 public:
    MoveGenerator() = delete;

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    template <Color C, bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalMoves(const State& S);

    template <bool WilyPromote = true>
    static MoveList generateLegalCheckMoves(const State& S);
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_MOVEGENERATOR_H
