#include "initializer.h"
#include "bitboard.h"

namespace nshogi {
namespace core {
namespace initializer {

void initializeAll() {
    static bool IsCalled = false;

    if (IsCalled) {
        return;
    }

    IsCalled = true;

    bitboard::initializeBitboards();
}

} // namespace initializer
} // namespace core
} // namespace nshogi
