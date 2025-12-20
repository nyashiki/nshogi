//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_UTILS_H
#define NSHOGI_CORE_UTILS_H

#include "../types.h"
#include "bitboard.h"

#include <sstream>
#include <vector>

namespace nshogi {
namespace core {
namespace internal {
namespace utils {

inline bool isSameLine(Square Sq1, Square Sq2) noexcept {
    return !bitboard::LineBB[Sq1][Sq2].isZero();
}

inline bool isSameLine(Square Sq1, Square Sq2, Square Sq3) noexcept {
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
} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_UTILS_H
