#include "common.h"
#include "../core/initializer.h"

int main() {
    nshogi::core::initializer::initializeAll();
    const auto Statistics = nshogi::test::common::TestBody::getInstance().run();

    std::printf("Ran %llu test cases: ", Statistics.NumSuccess + Statistics.NumFail);
    std::printf("%llu succeeded, %llu failed.\n", Statistics.NumSuccess, Statistics.NumFail);

    return Statistics.NumFail != 0;
}
