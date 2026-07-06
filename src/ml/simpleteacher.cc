//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "simpleteacher.h"
#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"

#include <cstring>
#include <random>

namespace nshogi {
namespace ml {

SimpleTeacher::SimpleTeacher()
    : HuffmanCode(core::HuffmanCode::zero())
    , Ply(0)
    , MaxPly(256)
    , DrawValues{0.5f, 0.5f}
    , NextMove(core::Move16::MoveNone())
    , Winner(core::NoColor)
    , V(0.0f)
    , Q(0.0f)
    , GamePly(0)
    , Declared(false) {
}

SimpleTeacher::SimpleTeacher(const SimpleTeacher& ST)
    : HuffmanCode(ST.HuffmanCode)
    , Ply(ST.Ply)
    , MaxPly(ST.MaxPly)
    , DrawValues{ST.DrawValues[0], ST.DrawValues[1]}
    , NextMove(ST.NextMove)
    , Winner(ST.Winner)
    , V(ST.V)
    , Q(ST.Q)
    , GamePly(ST.GamePly)
    , Declared(ST.Declared) {
}

SimpleTeacher& SimpleTeacher::operator=(const SimpleTeacher& ST) {
    if (this != &ST) {
        HuffmanCode = ST.HuffmanCode;
        Ply = ST.Ply;
        MaxPly = ST.MaxPly;
        DrawValues[0] = ST.DrawValues[0];
        DrawValues[1] = ST.DrawValues[1];
        NextMove = ST.NextMove;
        Winner = ST.Winner;
        V = ST.V;
        Q = ST.Q;
        GamePly = ST.GamePly;
        Declared = ST.Declared;
    }
    return *this;
}

const core::HuffmanCode& SimpleTeacher::getHuffmanCode() const {
    return HuffmanCode;
}

core::Position SimpleTeacher::getPosition() const {
    return core::PositionBuilder::newPosition(
        core::HuffmanCode::decode(HuffmanCode), Ply);
}

core::State SimpleTeacher::getState() const {
    return core::StateBuilder::newState(core::HuffmanCode::decode(HuffmanCode),
                                        Ply);
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

float SimpleTeacher::v() const {
    return V;
}

float SimpleTeacher::q() const {
    return Q;
}

uint16_t SimpleTeacher::gamePly() const {
    return GamePly;
}

bool SimpleTeacher::declared() const {
    return Declared;
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

SimpleTeacher& SimpleTeacher::setV(float Value) {
    V = Value;
    return *this;
}

SimpleTeacher& SimpleTeacher::setQ(float QValue) {
    Q = QValue;
    return *this;
}

SimpleTeacher& SimpleTeacher::setGamePly(uint16_t PlyValue) {
    GamePly = PlyValue;
    return *this;
}

SimpleTeacher& SimpleTeacher::setDeclared(bool DeclaredValue) {
    Declared = DeclaredValue;
    return *this;
}

bool SimpleTeacher::equals(const SimpleTeacher& ST) const {
    return HuffmanCode == ST.HuffmanCode && Ply == ST.Ply &&
           MaxPly == ST.MaxPly && DrawValues[0] == ST.DrawValues[0] &&
           DrawValues[1] == ST.DrawValues[1] && NextMove == ST.NextMove &&
           Winner == ST.Winner && V == ST.V && Q == ST.Q &&
           GamePly == ST.GamePly && Declared == ST.Declared;
}

void SimpleTeacher::corruptMyself() {
    std::mt19937_64 Mt(20250413);

    HuffmanCode = core::HuffmanCode(Mt(), Mt(), Mt(), Mt());
    Ply = (uint16_t)Mt();

    MaxPly = (uint16_t)Mt();
    uint32_t Buffer[2] = {
        (uint32_t)Mt(),
        (uint32_t)Mt(),
    };
    std::memcpy((void*)&DrawValues[0], &Buffer[0], sizeof(float));
    std::memcpy((void*)&DrawValues[1], &Buffer[1], sizeof(float));

    NextMove = core::Move16::fromValue((uint16_t)Mt());
    Winner = (core::Color)Mt();

    V = std::bit_cast<float>((int32_t)Mt());
    Q = std::bit_cast<float>((int32_t)Mt());
    GamePly = (uint16_t)Mt();
    Declared = (bool)(Mt() % 2);
}

} // namespace ml
} // namespace nshogi
