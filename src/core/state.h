#ifndef NSHOGI_CORE_STATE_H
#define NSHOGI_CORE_STATE_H

#include <memory>

#include <cassert>
#include <cstdint>

#include "position.h"
#include "stateconfig.h"
#include "types.h"

namespace nshogi {
namespace core {

namespace internal {

class StateImpl;
class ImmutableStateAdapter;
class MutableStateAdapter;

} // namespace internal

class State {
 public:
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = delete;

    State(State&&) noexcept;

    ~State();

    State clone() const;

    Color getSideToMove() const;
    const Position& getPosition() const;
    const Position& getInitialPosition() const;
    uint16_t getPly(bool IncludeOffset = true) const;
    Move32 getHistoryMove(uint16_t Ply) const;
    Move32 getLastMove() const;

    void doMove(Move32 Move);
    Move32 undoMove();

    // Re-compute "second" variables, where "second" means
    // any variables that can be computed by "first" variables
    // that are neccesary to stand for the game representation.
    void refresh();

    // Helper functions.
    RepetitionStatus getRepetitionStatus(bool Strict) const;
    uint8_t getStandCount(Color C, PieceTypeKind Type) const;
    Square getKingSquare(Color C) const;
    Move32 getMove32FromMove16(Move16 M16) const;

    uint8_t computePieceScore(Color C, uint8_t SliderScoreUnit, uint8_t StepScoreUnit, bool OnlyInPromotableZone) const;
    bool canDeclare() const;

 private:
    State() = delete;
    State(const Position& P);
    State(const Position& P, uint16_t Ply);
    State(const Position& CurrentP, const Position& InitP);

    State(internal::StateImpl* SI);

    internal::StateImpl* Impl;
    friend class StateBuilder;
    friend class internal::ImmutableStateAdapter;
    friend class internal::MutableStateAdapter;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATE_H
