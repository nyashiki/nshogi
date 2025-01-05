//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "simpleteacher.h"
#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"

namespace nshogi {
namespace ml {

SimpleTeacher::SimpleTeacher()
    : HuffmanCode(core::HuffmanCode::zero()) {
}

SimpleTeacher::SimpleTeacher(const SimpleTeacher& ST)
    : HuffmanCode(ST.HuffmanCode)
    , Ply(ST.Ply)
    , MaxPly(ST.MaxPly)
    , DrawValues { ST.DrawValues[0], ST.DrawValues[1] }
    , NextMove(ST.NextMove)
    , Winner(ST.Winner) {
}

core::Position SimpleTeacher::getPosition() const {
    return core::PositionBuilder::newPosition(core::HuffmanCode::decode(HuffmanCode), Ply);
}

core::State SimpleTeacher::getState() const {
    return core::StateBuilder::newState(core::HuffmanCode::decode(HuffmanCode), Ply);
}

core::StateConfig SimpleTeacher::getConfig() const {
    core::StateConfig Config;
    Config.Rule = core::EndingRule::ER_Declare27;
    Config.MaxPly = MaxPly;
    Config.BlackDrawValue = DrawValues[0];
    Config.WhiteDrawValue = DrawValues[1];

    return Config;
}

core::Move16 SimpleTeacher::getNextMove() const {
    return NextMove;
}

core::Color SimpleTeacher::getWinner() const {
    return Winner;
}

SimpleTeacher& SimpleTeacher::setState(const core::State& State) {
    HuffmanCode = core::HuffmanCode::encode(State.getPosition());
    Ply = State.getPly();
    return *this;
}

SimpleTeacher& SimpleTeacher::setConfig(const core::StateConfig& Config) {
    MaxPly = Config.MaxPly;
    DrawValues[0] = Config.BlackDrawValue;
    DrawValues[1] = Config.WhiteDrawValue;
    return *this;
}

SimpleTeacher& SimpleTeacher::setNextMove(core::Move16 Move) {
    NextMove = Move;
    return *this;
}

SimpleTeacher& SimpleTeacher::setWinner(core::Color C) {
    Winner = C;
    return *this;
}

bool SimpleTeacher::equals(const SimpleTeacher& ST) const {
    return HuffmanCode == ST.HuffmanCode
        && Ply == ST.Ply
        && MaxPly == ST.MaxPly
        && DrawValues[0] == ST.DrawValues[0]
        && DrawValues[1] == ST.DrawValues[1]
        && NextMove == ST.NextMove
        && Winner == ST.Winner;
}

} // namespace ml
} // namespace nshogi
