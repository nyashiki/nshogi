#ifndef NSHOGI_CORE_HUFFMAN_H
#define NSHOGI_CORE_HUFFMAN_H

#include "position.h"
#include <cinttypes>
#include <cstring>

#ifdef USE_AVX2

#include <immintrin.h>

#endif // #ifdef USE_AVX2

namespace nshogi {
namespace core {

struct alignas(32) HuffmanCode {
 public:
    HuffmanCode(const HuffmanCode& HC) {
#ifdef USE_AVX2
        C = HC.C;
#else
        std::memcpy(Data, HC.Data, HC.size());
#endif // #ifdef USE_AVX2
    }

    HuffmanCode operator=(const HuffmanCode& HC) {
#ifdef USE_AVX2
        C = HC.C;
#else
        std::memcpy(Data, HC.Data, HC.size());
#endif // #ifdef USE_AVX2
        return *this;
    }

    static HuffmanCode zero() {
        return HuffmanCode();
    }

    static HuffmanCode encode(const Position&);
    static HuffmanCode encode(const Position&, Square BlackKingSquare, Square WhiteKingSquare);
    static Position decode(const HuffmanCode&);

    bool operator==(const HuffmanCode& HC) const {
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

    bool operator!=(const HuffmanCode& HC) const {
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

    constexpr std::size_t size() const {
        return 4 * sizeof(uint64_t);
    }

    char* data() {
        return reinterpret_cast<char*>(Data);
    }

 protected:
    HuffmanCode() {
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

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_HUFFMAN_H
