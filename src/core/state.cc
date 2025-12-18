//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "state.h"
#include "internal/stateimpl.h"
#include "position.h"
#include "types.h"

#include <cassert>
#include <cstring>

namespace nshogi {
namespace core {

State::State(State&& S) noexcept
    : Impl(S.Impl) {
    S.Impl = nullptr;
}

State::State(const Position& P)
    : Impl(new internal::StateImpl(P)) {
}

State::State(const Position& P, uint16_t Ply)
    : Impl(new internal::StateImpl(P, Ply)) {
}

State::State(const Position& CurrentP, const Position& InitP)
    : Impl(new internal::StateImpl(CurrentP, InitP)) {
}

State::State(internal::StateImpl* SI)
    : Impl(SI) {
}

State::~State() {
    if (Impl != nullptr) {
        delete Impl;
    }
}

State State::clone() const {
    return State(new internal::StateImpl(Impl->clone()));
}

Color State::getSideToMove() const noexcept {
    return Impl->getSideToMove();
}

const Position& State::getPosition() const noexcept {
    return Impl->getPosition();
}

const Position& State::getInitialPosition() const noexcept {
    return Impl->getInitialPosition();
}

uint16_t State::getPly(bool IncludeOffset) const noexcept {
    return Impl->getPly(IncludeOffset);
}

Move32 State::getHistoryMove(uint16_t Ply) const {
    return Impl->getHistoryMove(Ply);
}

Move32 State::getLastMove() const {
    return Impl->getLastMove();
}

uint64_t State::getHash() const noexcept {
    return Impl->getHash();
}

void State::doMove(Move32 Move) noexcept {
    Impl->doMove(Move);
}

void State::undoMove() {
    Impl->undoMove();
}

RepetitionStatus State::getRepetitionStatus(bool Strict) const noexcept {
    if (Strict) {
        return Impl->getRepetitionStatus<true>();
    } else {
        return Impl->getRepetitionStatus<false>();
    }
}

bool State::isInCheck() const noexcept {
    return !Impl->getCheckerBB().isZero();
}

Square State::getKingSquare(Color C) const noexcept {
    return Impl->getKingSquare(C);
}

Move32 State::getMove32FromMove16(Move16 M16) const noexcept {
    return Impl->getMove32FromMove16(M16);
}

uint8_t State::computePieceScore(Color C, uint8_t SliderScoreUnit,
                                 uint8_t StepScoreUnit,
                                 bool OnlyInPromotableZone) const noexcept {

    if (C == Black) {
        return Impl->computePieceScore<Black>(SliderScoreUnit, StepScoreUnit,
                                              OnlyInPromotableZone);
    } else {
        return Impl->computePieceScore<White>(SliderScoreUnit, StepScoreUnit,
                                              OnlyInPromotableZone);
    }
}

bool State::canDeclare() const noexcept {
    return Impl->canDeclare();
}

} // namespace core
} // namespace nshogi
