#include "capability.h"

namespace nshogi {
namespace build_info {
namespace capability {

bool sse2Available() {
#if defined(USE_SSE2)
    return true;
#endif
    return false;
}

bool sse41Available() {
#if defined(USE_SSE41)
    return true;
#endif
    return false;
}

bool avxAvailable() {
#if defined(USE_AVX)
    return true;
#endif
    return false;
}

bool avx2Available() {
#if defined(USE_AVX2)
    return true;
#endif
    return false;
}

bool neonAvailable() {
#if defined(USE_NEON)
    return true;
#endif
    return false;
}

} // namespace build_info
} // namespace capability
} // namespace nshogi