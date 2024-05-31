// #include <CUnit/Basic.h>
// #include <CUnit/CUnit.h>
// #include <CUnit/TestDB.h>

// int setupTestTypes();
// int setupTestBitboard();
// int setupTestHuffman();
// int setupTestSfen();
// int setupTestCSA();
// int setupTestMoveGeneration();
// int setupTestUtils();
// int setupTestPosition();
// int setupTestState();
// int setupTestML();
// int setupTestSquareIterator();
// int setupTestSolver();

#include "common.h"
#include "../core/initializer.h"

int main() {
    // CU_initialize_registry();

    nshogi::core::initializer::initializeAll();
    const auto Statistics = nshogi::test::common::TestBody::getInstance().run();

    std::printf("Ran %llu test cases: ", Statistics.NumSuccess + Statistics.NumFail);
    std::printf("%llu succeeded, %llu failed.\n", Statistics.NumSuccess, Statistics.NumFail);

    return Statistics.NumFail == 0;

    // // Test body begin.
    // setupTestSolver();
    // setupTestTypes();
    // setupTestBitboard();
    // setupTestHuffman();
    // setupTestSfen();
    // setupTestCSA();
    // setupTestUtils();
    // setupTestML();
    // setupTestSquareIterator();
    // setupTestMoveGeneration();
    // setupTestPosition();
    // setupTestState();
    // // Test body end.

    // CU_basic_set_mode(CU_BRM_VERBOSE);
    // CU_basic_run_tests();

    // CU_cleanup_registry();

    // return CU_get_error();
    return 0;
}
