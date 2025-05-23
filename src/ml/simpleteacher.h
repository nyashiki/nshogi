//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_SIMPLETEACHER_H
#define NSHOGI_ML_SIMPLETEACHER_H

#include "../core/huffman.h"
#include "../core/position.h"
#include "../core/state.h"
#include "../core/stateconfig.h"
#include "../core/types.h"

namespace nshogi {
namespace ml {

struct SimpleTeacher {
 public:
    SimpleTeacher();
    SimpleTeacher(const SimpleTeacher&);

    core::Position getPosition() const;
    core::State getState() const;
    core::StateConfig getConfig() const;
    core::Move16 getNextMove() const;
    core::Color getWinner() const;

    SimpleTeacher& setState(const core::State&);
    SimpleTeacher& setConfig(const core::StateConfig&);
    SimpleTeacher& setNextMove(core::Move16);
    SimpleTeacher& setWinner(core::Color);

    bool equals(const SimpleTeacher&) const;

    /// Generate completely random noisy data.
    /// This function is for test.
    [[maybe_unused]]
    void corruptMyself();

 protected:
    // State.
    core::HuffmanCode HuffmanCode;
    uint16_t Ply;

    // Config.
    uint16_t MaxPly;
    float DrawValues[2];

    // Result.
    core::Move16 NextMove;
    core::Color Winner;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_SIMPLETEACHER_H
