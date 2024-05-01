#ifndef NSHOGI_CORE_STATEHELPER_H
#define NSHOGI_CORE_STATEHELPER_H

#include "bitboard.h"
#include "hash.h"
#include "position.h"
#include "types.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <xmmintrin.h>

namespace nshogi {
namespace core {

struct StepHelper {
    StepHelper() : Move(Move32::MoveNone()) {
    }

    Move32 Move;
    uint64_t BoardHash;
    uint8_t ContinuousCheckCounts[NumColors];
    Stands EachStand[NumColors];

    bitboard::Bitboard DefendingOpponentSliderBB[NumColors];
    bitboard::Bitboard CheckerBB;
    // __m128i Hash;
};

struct StateHelper {
 public:
    StateHelper(const Position& Pos);

    StateHelper(const StateHelper&) = delete;
    StateHelper& operator=(const StateHelper&) = delete;
    StateHelper& operator=(StateHelper&&) = delete;

    StateHelper(StateHelper&&) noexcept;

    ~StateHelper();

    void proceedOneStep(const Move32& Move, uint64_t BoardHash, Stands BlackStand, Stands WhiteStand);
    Move32 goBackOneStep();

    inline const StepHelper& getCurrentStepHelper() const {
        return getStepHelper(Ply);
    }

    inline const StepHelper& getStepHelper(uint16_t TargetPly) const {
        return SHelper[TargetPly];
    }

    inline const Position& getInitialPosition() const {
        return InitialPosition;
    }

    inline uint16_t getPly() const {
        return Ply;
    }

 private:
    const Position InitialPosition;
    uint16_t Ply;

    // Incrementally updatable variables.
    bitboard::Bitboard ColorBB[NumColors];
    bitboard::Bitboard TypeBB[NumPieceType];
    Square KingSquare[NumColors];

    // Store them since they are computationally heavy.
    std::vector<StepHelper> SHelper;

    friend class State;
    friend class StateBuilder;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATEHELPER_H
