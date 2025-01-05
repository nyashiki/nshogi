//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_HUFFMAN_H
#define NSHOGI_CORE_HUFFMAN_H

#include "position.h"

namespace nshogi {
namespace core {

namespace internal {

struct HuffmanCodeImpl;

} // namespace

struct alignas(32) HuffmanCode {
 public:
    HuffmanCode(const HuffmanCode& HC);
    HuffmanCode(uint64_t Code3, uint64_t Code2, uint64_t Code1, uint64_t Code0);

    ~HuffmanCode();

    HuffmanCode operator=(const HuffmanCode& HC);

    bool operator==(const HuffmanCode& HC) const;
    bool operator!=(const HuffmanCode& HC) const;

    static std::size_t size();

    const char* data() const;

    static HuffmanCode zero();

    static HuffmanCode encode(const Position&);
    static HuffmanCode encode(const Position&, Square BlackKingSquare, Square WhiteKingSquare);
    static Position decode(const HuffmanCode&);

 private:
    HuffmanCode(internal::HuffmanCodeImpl* I)
        : Impl(I) {
    }

    internal::HuffmanCodeImpl* Impl;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_HUFFMAN_H
