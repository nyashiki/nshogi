//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "huffman.h"
#include <cstdio>
#include <cstring>
#include <cinttypes>

namespace nshogi {
namespace io {
namespace huffman {

void print(const core::HuffmanCode& HuffmanCode) {
    uint64_t Codes[4];
    std::memcpy(static_cast<void*>(Codes), HuffmanCode.data(), sizeof(HuffmanCode));
    std::printf(
            "(0x%" PRIx64 ", 0x%" PRIx64 ", 0x%" PRIx64 ", 0x%" PRIx64 ")\n"
            , Codes[0]
            , Codes[1]
            , Codes[2]
            , Codes[3]);
}

} // namespace huffman
} // namespace io
} // namespace nshogi
