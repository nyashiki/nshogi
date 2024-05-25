#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

int setupTestTypes();
int setupTestBitboard();
int setupTestHuffman();
int setupTestSfen();
int setupTestCSA();
int setupTestMoveGeneration();
int setupTestUtils();
int setupTestPosition();
int setupTestState();
int setupTestML();
int setupTestSquareIterator();
int setupTestSolver();

#include "../core/initializer.h"
#include "../core/bitboard.h"
#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/statebuilder.h"
#include "../io/bitboard.h"
#include "../io/sfen.h"
#include <iostream>


int main() {
    CU_initialize_registry();

    nshogi::core::initializer::initializeAll();

    // Test body begin.
    setupTestSolver();
    setupTestTypes();
    setupTestBitboard();
    setupTestHuffman();
    setupTestSfen();
    setupTestCSA();
    setupTestUtils();
    setupTestML();
    setupTestSquareIterator();
    setupTestMoveGeneration();
    setupTestPosition();
    setupTestState();
    // Test body end.

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();

    return CU_get_error();
}
