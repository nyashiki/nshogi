//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_STATEHELPER_H
#define NSHOGI_CORE_STATEHELPER_H

#include "../position.h"
#include "../types.h"
#include "bitboard.h"
#include "hash.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace nshogi {
namespace core {
namespace internal {

struct StepHelper {
    StepHelper()
        : Move(Move32::MoveNone()) {
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
    StateHelper(const Position& Pos, uint16_t PlyOffset);

    StateHelper(const StateHelper&) = delete;
    StateHelper& operator=(const StateHelper&) = delete;
    StateHelper& operator=(StateHelper&&) = delete;

    StateHelper(StateHelper&&) noexcept;

    ~StateHelper();

    void proceedOneStep(Move32 Move, uint64_t BoardHash, Stands BlackStand,
                        Stands WhiteStand) noexcept;
    Move32 goBackOneStep();

    inline const StepHelper& getCurrentStepHelper() const noexcept {
        return getStepHelper(Ply);
    }

    inline const StepHelper& getStepHelper(uint16_t TargetPly) const {
        return SHelper[TargetPly];
    }

    inline const Position& getInitialPosition() const noexcept {
        return InitialPosition;
    }

    inline uint16_t getPly(bool IncludeOffset = true) const noexcept {
        if (IncludeOffset) {
            return InitialPosition.getPlyOffset() + Ply;
        }

        return Ply;
    }

 private:
    constexpr static std::size_t DefaultReserveSize = 1024;

    const Position InitialPosition;
    uint16_t Ply;

    // Incrementally updatable variables.
    bitboard::Bitboard ColorBB[NumColors];
    bitboard::Bitboard TypeBB[NumPieceType];
    Square KingSquare[NumColors];

    // Store them since they are computationally heavy.
    std::vector<StepHelper> SHelper;

    friend class StateImpl;
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATEHELPER_H
