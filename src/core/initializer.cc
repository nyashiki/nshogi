#include "initializer.h"
#include "internal/bitboard.h"

namespace nshogi {
namespace core {
namespace initializer {

void initializeAll() {
    static bool IsCalled = false;

    if (IsCalled) {
        return;
    }

    IsCalled = true;

    internal::bitboard::initializeBitboards();
}

} // namespace initializer
} // namespace core
} // namespace nshogi
