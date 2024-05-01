#include <CUnit/CUnit.h>

#include "../core/types.h"
#include "../core/squareiterator.h"


namespace {

template<nshogi::core::IterateOrder Order>
void testReverseOrder() {
    nshogi::core::SquareIterator<Order> SqIt;
    nshogi::core::SquareIterator<nshogi::core::reverseOrder<Order>()> RvSqIt;

    while (true) {
        auto Sq1 = *SqIt;
        auto Sq2 = *RvSqIt;

        CU_ASSERT_EQUAL(Sq1, nshogi::core::getInversed(Sq2));

        ++SqIt;
        ++RvSqIt;

        if (SqIt == SqIt.end()) {
            CU_ASSERT_TRUE(RvSqIt == RvSqIt.end());

            break;
        }
    }

}

void testReverseESWN() {
    testReverseOrder<nshogi::core::IterateOrder::ESWN>();
}

void testReverseWNES() {
    testReverseOrder<nshogi::core::IterateOrder::WNES>();
}

void testReverseNWSE() {
    testReverseOrder<nshogi::core::IterateOrder::NWSE>();
}

void testReverseSENW() {
    testReverseOrder<nshogi::core::IterateOrder::SENW>();
}

} // namespace


int setupTestSquareIterator() {
    CU_pSuite suite = CU_add_suite("squareiterator test", 0, 0);

    CU_add_test(suite, "Test Reverse ESWN", testReverseESWN);
    CU_add_test(suite, "Test Reverse WNES", testReverseWNES);
    CU_add_test(suite, "Test Reverse NWSE", testReverseNWSE);
    CU_add_test(suite, "Test Reverse SENW", testReverseSENW);

    return CUE_SUCCESS;
}
