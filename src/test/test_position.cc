#include "common.h"
#include "../core/position.h"
#include "../core/positionbuilder.h"


TEST(Position, Constructor1) {
    nshogi::core::Position Pos =
        nshogi::core::PositionBuilder::getInitialPosition();

    nshogi::core::Position Pos2 =
        nshogi::core::PositionBuilder::newPosition(Pos);

    TEST_ASSERT_TRUE(Pos.equals(Pos2));
}

TEST(Position, Constructor2) {
    nshogi::core::Position Pos =
        nshogi::core::PositionBuilder::getInitialPosition();
    for (uint16_t I = 0; I < 1024; ++I) {
        nshogi::core::Position Pos2 =
            nshogi::core::PositionBuilder::newPosition(Pos, I);
        nshogi::core::Position Pos3 =
            nshogi::core::PositionBuilder::newPosition(Pos2);

        TEST_ASSERT_TRUE(Pos2.equals(Pos3));
    }
}
