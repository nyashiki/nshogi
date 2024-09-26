#include "../core/initializer.h"

extern "C" {

#include "c_api.h"

void nshogi_initialize(void) {
    nshogi::core::initializer::initializeAll();
}

} // extern "C"
