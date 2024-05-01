#include <CUnit/CUnit.h>

#include "../core/types.h"
#include "../core/utils.h"

namespace {

void testIsSameLine2() {
    using namespace nshogi::core;

    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1B));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1C));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1D));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1E));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1F));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1G));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1H));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq1I));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq2B));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq3C));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq4D));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq5E));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq6F));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq7G));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq8H));
    CU_ASSERT_TRUE(utils::isSameLine(Sq1A, Sq9I));

    CU_ASSERT_FALSE(utils::isSameLine(Sq1A, Sq1A));
    CU_ASSERT_FALSE(utils::isSameLine(Sq1A, Sq2C));
    CU_ASSERT_FALSE(utils::isSameLine(Sq2B, Sq4C));
    CU_ASSERT_FALSE(utils::isSameLine(Sq5E, Sq1B));
    CU_ASSERT_FALSE(utils::isSameLine(Sq9E, Sq4B));
}

} // namespace

int setupTestUtils() {
    CU_pSuite suite = CU_add_suite("utils.h test", 0, 0);
    CU_add_test(suite, "isSameLine2", testIsSameLine2);

    return CUE_SUCCESS;
}
