#include "common.h"
#include "../core/initializer.h"
#include <cinttypes>

int main() {
    nshogi::core::initializer::initializeAll();
    const auto Statistics = nshogi::test::common::TestBody::getInstance().run();

    std::printf("Ran %" PRIu64 " test cases: ", Statistics.NumSuccess + Statistics.NumFail);
    std::printf("%" PRIu64 " succeeded, %" PRIu64 " failed.\n", Statistics.NumSuccess, Statistics.NumFail);

    return Statistics.NumFail != 0;
}
