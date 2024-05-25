#include <CUnit/CUnit.h>

#include "../core/position.h"
#include "../core/positionbuilder.h"

namespace {

void testConstructor1() {
    nshogi::core::Position Pos =
        nshogi::core::PositionBuilder::getInitialPosition();

    nshogi::core::Position Pos2 =
        nshogi::core::PositionBuilder::newPosition(Pos);

    CU_ASSERT_TRUE(Pos.equals(Pos2));
}

void testConstructor2() {
    nshogi::core::Position Pos =
        nshogi::core::PositionBuilder::getInitialPosition();
    for (uint16_t I = 0; I < 1024; ++I) {
        nshogi::core::Position Pos2 =
            nshogi::core::PositionBuilder::newPosition(Pos, I);
        nshogi::core::Position Pos3 =
            nshogi::core::PositionBuilder::newPosition(Pos2);

        CU_ASSERT_TRUE(Pos2.equals(Pos3));
    }
}

} // namespace

int setupTestPosition() {
    CU_pSuite suite = CU_add_suite("position test", 0, 0);

    CU_add_test(suite, "Constructor 1", testConstructor1);
    CU_add_test(suite, "Constructor 2", testConstructor2);

    return CUE_SUCCESS;
}
