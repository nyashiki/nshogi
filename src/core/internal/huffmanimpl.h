//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_INTERNAL_HUFFMANIMPL_H
#define NSHOGI_CORE_INTERNAL_HUFFMANIMPL_H

#include "../position.h"
#include <cinttypes>
#include <cstring>

#ifdef USE_AVX2

#include <immintrin.h>

#endif // #ifdef USE_AVX2

namespace nshogi {
namespace core {
namespace internal {

struct alignas(32) HuffmanCodeImpl {
 public:
    HuffmanCodeImpl(const HuffmanCodeImpl& HC) {
#ifdef USE_AVX2
        C = HC.C;
#else
        std::memcpy(Data, HC.Data, HC.size());
#endif // #ifdef USE_AVX2
    }

    HuffmanCodeImpl(uint64_t Code3, uint64_t Code2, uint64_t Code1,
                    uint64_t Code0) {
        Data[0] = Code0;
        Data[1] = Code1;
        Data[2] = Code2;
        Data[3] = Code3;
    }

    HuffmanCodeImpl operator=(const HuffmanCodeImpl& HC) {
#ifdef USE_AVX2
        C = HC.C;
#else
        if (this != &HC) {
            std::memcpy(Data, HC.Data, HC.size());
        }
#endif // #ifdef USE_AVX2
        return *this;
    }

    static HuffmanCodeImpl zero() {
        return HuffmanCodeImpl();
    }

    static HuffmanCodeImpl encode(const Position&);
    static HuffmanCodeImpl encode(const Position&, Square BlackKingSquare,
                                  Square WhiteKingSquare);
    static Position decode(const HuffmanCodeImpl&);

    bool operator==(const HuffmanCodeImpl& HC) const {
#ifdef USE_AVX2
        __m256i Comp = _mm256_cmpeq_epi8(C, HC.C);
        uint32_t BitMask = (uint32_t)_mm256_movemask_epi8(Comp);
        return BitMask == 0xffffffffU;
#else
        for (std::size_t I = 0; I < 4; ++I) {
            if (Data[I] != HC.Data[I]) {
                return false;
            }
        }
        return true;
#endif // #ifdef USE_AVX2
    }

    bool operator!=(const HuffmanCodeImpl& HC) const {
#ifdef USE_AVX2
        __m256i Comp = _mm256_cmpeq_epi8(C, HC.C);
        uint32_t BitMask = (uint32_t)_mm256_movemask_epi8(Comp);
        return BitMask != 0xffffffffU;
#else
        for (std::size_t I = 0; I < 4; ++I) {
            if (Data[I] != HC.Data[I]) {
                return true;
            }
        }
        return false;
#endif // #ifdef USE_AVX2
    }

    static constexpr std::size_t size() {
        return 4 * sizeof(uint64_t);
    }

    const char* data() const {
        return reinterpret_cast<const char*>(Data);
    }

 protected:
    HuffmanCodeImpl() {
#ifdef USE_AVX2
        C = _mm256_setzero_si256();
#else
        std::memset(reinterpret_cast<char*>(Data), 0, 4 * sizeof(uint64_t));
#endif // #ifdef USE_AVX2
    }

    union {
#ifdef USE_AVX2
        __m256i C;
#endif // #ifdef USE_AVX2
        uint64_t Data[4];
    };
};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_HUFFMANIMPL_H
