#ifndef NSHOGI_CORE_STATECONFIG_H
#define NSHOGI_CORE_STATECONFIG_H

#include "types.h"
#include <cstdint>

namespace nshogi {
namespace core {

struct StateConfig {
    EndingRule Rule = NoRule_ER;
    uint16_t MaxPly = 256;
    float BlackDrawValue = 0.5;
    float WhiteDrawValue = 0.5;
};

} // namespace core
} // namespace nshogi


#endif // #ifndef NSHOGI_CORE_STATECONFIG_H
