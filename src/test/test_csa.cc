//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"

#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"
#include "../io/csa.h"
#include "../io/sfen.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


TEST(CSA, InitialPosition) {
    auto Position = nshogi::core::PositionBuilder::getInitialPosition();

    const std::string& InitCSA = nshogi::io::csa::positionToCSA(Position);

    // clang-format off
    TEST_ASSERT_STREQ(InitCSA.c_str(),
        "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
        "P2 * -HI *  *  *  *  * -KA * \n"
        "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
        "P4 *  *  *  *  *  *  *  *  * \n"
        "P5 *  *  *  *  *  *  *  *  * \n"
        "P6 *  *  *  *  *  *  *  *  * \n"
        "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
        "P8 * +KA *  *  *  *  * +HI * \n"
        "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
        "+\n");
    // clang-format on
}

TEST(CSA, Example1) {
    std::ifstream Ifs("./res/test/example-csa.txt");

    std::string Contents;
    std::string Line;

    while (std::getline(Ifs, Line)) {
        Contents += Line + "\n";
    }

    auto State = nshogi::io::csa::StateBuilder::newState(Contents);
    const std::string& CSA = nshogi::io::csa::stateToCSA(State);
    TEST_ASSERT_STREQ(CSA.c_str(), Contents.c_str());
}

TEST(CSA, Handmade1) {
    const std::string Sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7g7f 3c3d 8h2b+ 3a2b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    const std::string& CSA = nshogi::io::csa::stateToCSA(State);

    // clang-format off
    TEST_ASSERT_STREQ(CSA.c_str(),
        "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
        "P2 * -HI *  *  *  *  * -KA * \n"
        "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
        "P4 *  *  *  *  *  *  *  *  * \n"
        "P5 *  *  *  *  *  *  *  *  * \n"
        "P6 *  *  *  *  *  *  *  *  * \n"
        "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
        "P8 * +KA *  *  *  *  * +HI * \n"
        "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
        "+\n"
        "+7776FU\n"
        "-3334FU\n"
        "+8822UM\n"
        "-3122GI\n");
    // clang-format on
}
