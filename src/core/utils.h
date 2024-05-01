#ifndef NSHOGI_CORE_UTILS_H
#define NSHOGI_CORE_UTILS_H

#include "bitboard.h"
#include "types.h"

#include <vector>
#include <sstream>

namespace nshogi {
namespace core {
namespace utils {

inline bool isSameLine(Square Sq1, Square Sq2) {
    return !bitboard::LineBB[Sq1][Sq2].isZero();
}

inline bool isSameLine(Square Sq1, Square Sq2, Square Sq3) {
    return bitboard::LineBB[Sq1][Sq2].isSet(Sq3);
}

inline std::vector<std::string> split(const std::string& Src, char Delim) {
    std::vector<std::string> Ret;

    std::stringstream SStream(Src);
    std::string Buffer;

    while (std::getline(SStream, Buffer, Delim)) {
        Ret.push_back(Buffer);
    }

    return Ret;
}

} // namespace utils
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_UTILS_H
