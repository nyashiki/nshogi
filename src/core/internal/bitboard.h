//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_INTERNAL_BITBOARD_H
#define NSHOGI_CORE_INTERNAL_BITBOARD_H

#include "../types.h"

#include <bit>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <functional>

#if defined(USE_SSE41)

#include <emmintrin.h>
#include <smmintrin.h>

#endif

#if defined(USE_BMI1) || defined(USE_BMI2) || defined(USE_AVX2)

#include <immintrin.h>

#endif

#if defined(USE_NEON)

#include <arm_neon.h>

#endif

#include <type_traits>

namespace nshogi {
namespace core {
namespace internal {
namespace bitboard {

struct Bitboard;

extern const Bitboard SquareBB[NumSquares];
extern const Bitboard FileBB[NumFiles];
extern const Bitboard RankBB[NumRanks];

// `StepPieceAttackBB` holds the attack bitboard of each color, piece type, and
// square. Slider attacks (bishop, rook, and lance) are not included.
// `PTK_Empty`, of course, does not have any attack,
// `PTK_Pawn`'s attack can be computed by bit operation,
// and `PTK_King`'s attack is stored in `KingAttackBB` for memory efficiency.
// As a result, only 3 piece types (knight, silver, and gold) are
// included in `StepPieceAttackBB`.
// Therefore, the second dimension of `StepPieceAttackBB` is 3.
extern Bitboard StepPieceAttackBB[NumColors][3][NumSquares];
// King attacks are irrelavant to its color.
// Therefore, `KingAttackBB` holds the attack bitboard of king for each square
// without color distinction for memory efficiency.
extern Bitboard KingAttackBB[NumSquares];

extern Bitboard DiagStepAttackBB[NumSquares];
extern Bitboard CrossStepAttackBB[NumSquares];

//  Useful bitboards.
extern const Bitboard FirstAndSecondFurthestBB[NumColors];
extern const Bitboard PromotableBB[NumColors];

extern Bitboard LineBB[NumSquares][NumSquares];
extern Bitboard DirectionBB[11 + NorthNorthWest + 1][NumSquares];
extern Bitboard BetweenBB[NumSquares][NumSquares];
extern Bitboard DiagBB[NumSquares];
extern Bitboard CrossBB[NumSquares];
extern Bitboard ForwardBB[NumSquares];
extern Bitboard BackwardBB[NumSquares];

struct alignas(16) Bitboard {
    Bitboard() noexcept = default;

#if defined(USE_SSE41)
    Bitboard(uint64_t High, uint64_t Low) noexcept
        : Bitboard_{_mm_set_epi64x((long long)High, (long long)Low)} {
    }
#elif defined(USE_NEON)
    Bitboard(uint64_t High, uint64_t Low) noexcept
        : Bitboard_{vcombine_u64(vcreate_u64(Low), vcreate_u64(High))} {
    }
#else
    Bitboard(uint64_t High, uint64_t Low) noexcept
        : Primitive{Low, High} {
    }
#endif

    static Bitboard ZeroBB() noexcept {
#if defined(USE_SSE41)
        return _mm_setzero_si128();
#elif defined(USE_NEON)
        return vdupq_n_u64(0);
#else
        return Bitboard(0, 0);
#endif
    }

    static Bitboard AllBB() noexcept {
#if defined(USE_SSE41)
        return _mm_set_epi64x(0x3ffff, 0x7fffffffffffffff);
#elif defined(USE_NEON)
        return vcombine_u64(vcreate_u64(0x7fffffffffffffff),
                            vcreate_u64(0x3ffff));
#else
        return Bitboard(0x3ffff, 0x7fffffffffffffff);
#endif
    }

    template <Color C>
    static Bitboard FurthermostBB() noexcept {
        if constexpr (C == Black) {
            return RankBB[RankA];
        } else {
            return RankBB[RankI];
        }
    }

    static Bitboard FurthermostBB(Color C) noexcept {
        return (C == Black) ? RankBB[RankA] : RankBB[RankI];
    }

    template <Color C>
    static Bitboard SecondFurthestBB() noexcept {
        if constexpr (C == Black) {
            return RankBB[RankB];
        } else {
            return RankBB[RankH];
        }
    }

    static Bitboard SecondFurthestBB(Color C) noexcept {
        return (C == Black) ? RankBB[RankB] : RankBB[RankH];
    }

#if defined(USE_SSE41)
    Bitboard(const __m128i& BB)
        : Bitboard_(BB) {
    }
#elif defined(USE_NEON)
    Bitboard(const uint64x2_t& BB)
        : Bitboard_(BB) {
    }
#endif

#if defined(USE_SSE41) || defined(USE_NEON)
    Bitboard(const Bitboard& BB) noexcept
        : Bitboard_(BB.Bitboard_) {
    }
#else
    Bitboard(const Bitboard& BB) noexcept
        : Primitive{BB.getPrimitive<false>(), BB.getPrimitive<true>()} {
    }
#endif

    Bitboard& operator=(Bitboard&& BB) noexcept {
#if defined(USE_SSE41) || defined(USE_NEON)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.Primitive[0];
        Primitive[1] = BB.Primitive[1];
#endif
        return *this;
    }

    Bitboard& operator=(const Bitboard& BB) noexcept {
#if defined(USE_SSE41) || defined(USE_NEON)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.Primitive[0];
        Primitive[1] = BB.Primitive[1];
#endif
        return *this;
    }

    void copyFrom(const Bitboard& BB) noexcept {
#if defined(USE_SSE41) || defined(USE_NEON)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.getPrimitive<false>();
        Primitive[1] = BB.getPrimitive<true>();
#endif
    }

    // Getters.
    inline __attribute__((always_inline)) bool isZero() const noexcept {
#if defined(USE_SSE41)
        return _mm_testz_si128(Bitboard_, Bitboard_);
#elif defined(USE_NEON)
        uint64x2_t Temp = vandq_u64(Bitboard_, Bitboard_);
        return (vgetq_lane_u64(Temp, 0) == 0) && (vgetq_lane_u64(Temp, 1) == 0);
#else
        return Primitive[0] == 0 && Primitive[1] == 0;
#endif
    }

    inline __attribute__((always_inline)) bool isSet(Square Sq) const noexcept {
#if defined(USE_SSE41)
        return !_mm_testz_si128(Bitboard_, SquareBB[Sq].Bitboard_);
#elif defined(USE_NEON)
        uint64x2_t Temp = vandq_u64(Bitboard_, SquareBB[Sq].Bitboard_);
        return (vgetq_lane_u64(Temp, 0) != 0) || (vgetq_lane_u64(Temp, 1) != 0);
#endif
        return !(*this & SquareBB[Sq]).isZero();
    }

    // Setters.
    inline __attribute__((always_inline)) void clear() noexcept {
#if defined(USE_SSE41)
        Bitboard_ = _mm_setzero_si128();
#elif defined(USE_NEON)
        Bitboard_ = vdupq_n_u64(0);
#else
        Primitive[0] = 0;
        Primitive[1] = 0;
#endif
    }

    inline __attribute__((always_inline)) void toggleBit(Square Sq) noexcept {
        (*this) ^= SquareBB[Sq];
    }

    // Bit operations.
    inline __attribute__((always_inline)) Bitboard&
    operator|=(const Bitboard& BB) noexcept {
#if defined(USE_SSE41)
        Bitboard_ = _mm_or_si128(Bitboard_, BB.Bitboard_);
#elif defined(USE_NEON)
        Bitboard_ = vorrq_u64(Bitboard_, BB.Bitboard_);
#else
        Primitive[1] |= BB.Primitive[1];
        Primitive[0] |= BB.Primitive[0];
#endif
        return *this;
    }

    inline __attribute__((always_inline)) Bitboard
    operator|(const Bitboard& BB) const noexcept {
        Bitboard Res = *this;
        Res |= BB;
        return Res;
    }

    inline __attribute__((always_inline)) Bitboard&
    operator&=(const Bitboard& BB) noexcept {
#if defined(USE_SSE41)
        Bitboard_ = _mm_and_si128(Bitboard_, BB.Bitboard_);
#elif defined(USE_NEON)
        Bitboard_ = vandq_u64(Bitboard_, BB.Bitboard_);
#else
        Primitive[1] &= BB.Primitive[1];
        Primitive[0] &= BB.Primitive[0];
#endif
        return *this;
    }

    inline __attribute__((always_inline)) Bitboard
    operator&(const Bitboard& BB) const noexcept {
        Bitboard Res = *this;
        Res &= BB;
        return Res;
    }

    inline __attribute__((always_inline)) Bitboard&
    operator^=(const Bitboard& BB) noexcept {
#if defined(USE_SSE41)
        Bitboard_ = _mm_xor_si128(Bitboard_, BB.Bitboard_);
#elif defined(USE_NEON)
        Bitboard_ = veorq_u64(Bitboard_, BB.Bitboard_);
#else
        Primitive[1] ^= BB.Primitive[1];
        Primitive[0] ^= BB.Primitive[0];
#endif
        return *this;
    }

    inline __attribute__((always_inline)) Bitboard
    operator^(const Bitboard& BB) const noexcept {
        Bitboard Res = *this;
        Res ^= BB;
        return Res;
    }

    inline __attribute__((always_inline)) Bitboard
    andNot(const Bitboard& BB) const noexcept {
#if defined(USE_SSE41)
        return _mm_andnot_si128(Bitboard_, BB.Bitboard_);
#elif defined(USE_NEON)
        return vbicq_u64(BB.Bitboard_, Bitboard_);
#else
        return {~Primitive[1] & BB.Primitive[1],
                ~Primitive[0] & BB.Primitive[0]};
#endif
    }

    inline __attribute__((always_inline)) Bitboard operator~() const noexcept {
        return andNot(AllBB());
    }

    template <int Shift>
    inline __attribute__((always_inline)) Bitboard
    getRightShiftEpi64() const noexcept {
#if defined(USE_SSE41)
        return _mm_srli_epi64(Bitboard_, Shift);
#elif defined(USE_NEON)
        return vshrq_n_u64(Bitboard_, Shift);
#else
        return Bitboard(Primitive[1] >> Shift, Primitive[0] >> Shift);
#endif
    }

    inline __attribute__((always_inline)) Bitboard
    getRightShiftEpi64(int Shift) const noexcept {
#if defined(USE_SSE41)
        return _mm_srli_epi64(Bitboard_, Shift);
#elif defined(USE_NEON)
        const int64x2_t ShiftVector = vdupq_n_s64(-Shift);
        return vshlq_u64(Bitboard_, ShiftVector);
#else
        return Bitboard(Primitive[1] >> Shift, Primitive[0] >> Shift);
#endif
    }

    template <int Shift>
    inline __attribute__((always_inline)) Bitboard
    getRightShiftSi128() const noexcept {
        // We can't use _mm_slli_si128 here even when Shift % 8 == 0 holds
        // because the bitboard is composed of two 64-bit variables. One of them
        // uses 63 bits to represent the first 7 files (7 files * 9 squares =
        // 63), and the other one uses 18 bits for the remaining 2 files (2
        // files * 9 squares = 18). Therefore, we must pay attention to the 1
        // bit of the former one. Using _mm_slli_si128 directly collapses this
        // configuration.

        static_assert(0 <= Shift && Shift <= 63, "Shift must be in [0, 63]");

        if constexpr (Shift == 0) {
            return *this;
        }

#if defined(USE_SSE41)
        const __m128i Shifted = _mm_srli_epi64(Bitboard_, Shift);
        const __m128i HiToLo = _mm_srli_si128(Bitboard_, 8);
        const __m128i Carry = _mm_slli_epi64(HiToLo, 63 - Shift);
        return _mm_or_si128(Shifted, Carry);
#elif defined(USE_NEON)
        const uint64x2_t Shifted = vshlq_u64(Bitboard_, vdupq_n_s64(-Shift));
        const uint8x16_t Bytes = vreinterpretq_u8_u64(Bitboard_);
        const uint8x16_t Zero = vdupq_n_u8(0);
        const uint8x16_t HiToLoBytes = vextq_u8(Bytes, Zero, 8);
        const uint64x2_t HiToLo = vreinterpretq_u64_u8(HiToLoBytes);
        const uint64x2_t Carry = vshlq_u64(HiToLo, vdupq_n_s64(63 - Shift));
        return vorrq_u64(Shifted, Carry);
#else
        return Bitboard(Primitive[1] >> Shift,
                        (Primitive[0] >> Shift) |
                            (Primitive[1] << (63 - Shift)));
#endif
    }

    template <int Shift>
    inline __attribute__((always_inline)) Bitboard
    getLeftShiftEpi64() const noexcept {
#if defined(USE_SSE41)
        return _mm_slli_epi64(Bitboard_, Shift);
#elif defined(USE_NEON)
        return vshlq_n_u64(Bitboard_, Shift);
#else
        return Bitboard(Primitive[1] << Shift, Primitive[0] << Shift);
#endif
    }

    inline __attribute__((always_inline)) Bitboard
    getLeftShiftEpi64(int Shift) const noexcept {
#if defined(USE_SSE41)
        return _mm_slli_epi64(Bitboard_, Shift);
#elif defined(USE_NEON)
        const int64x2_t ShiftVector = vdupq_n_s64(Shift);
        return vshlq_u64(Bitboard_, ShiftVector);
#else
        return Bitboard(Primitive[1] << Shift, Primitive[0] << Shift);
#endif
    }

    template <int Shift>
    inline __attribute__((always_inline)) Bitboard
    getLeftShiftSi128() const noexcept {
        static_assert(0 <= Shift && Shift <= 63, "Shift must be in [0, 63]");

        if constexpr (Shift == 0) {
            return *this;
        }

        // See the comment in getRightShiftSi128().
#if defined(USE_SSE41)
        const __m128i Shifted = _mm_slli_epi64(Bitboard_, Shift);
        const __m128i LoToHi = _mm_slli_si128(Bitboard_, 8);
        const __m128i Carry = _mm_srli_epi64(LoToHi, 63 - Shift);
        return _mm_or_si128(Shifted, Carry);
#elif defined(USE_NEON)
        const uint64x2_t Shifted = vshlq_n_u64(Bitboard_, Shift);
        const uint8x16_t Bytes = vreinterpretq_u8_u64(Bitboard_);
        const uint8x16_t Zero = vdupq_n_u8(0);
        const uint8x16_t LoToHiBytes = vextq_u8(Zero, Bytes, 8);
        const uint64x2_t LoToHi = vreinterpretq_u64_u8(LoToHiBytes);
        const uint64x2_t Carry = vshrq_n_u64(LoToHi, 63 - Shift);
        return vorrq_u64(Shifted, Carry);
#else
        return Bitboard((Primitive[1] << Shift) |
                            (Primitive[0] >> (63 - Shift)),
                        Primitive[0] << Shift);
#endif
    }

    bool operator==(const Bitboard& BB) const noexcept {
#if defined(USE_SSE41)
        __m128i Result = _mm_cmpeq_epi8(Bitboard_, BB.Bitboard_); // Compare
        return _mm_movemask_epi8(Result) == 0xFFFF;
#elif defined(USE_NEON)
        const uint64x2_t Compare = vceqq_u64(Bitboard_, BB.Bitboard_);
        return (vgetq_lane_u64(Compare, 0) & vgetq_lane_u64(Compare, 1)) ==
               ~0ULL;
#else
        return Primitive[0] == BB.Primitive[0] &&
               Primitive[1] == BB.Primitive[1];
#endif
    }

    bool operator!=(const Bitboard& BB) const noexcept {
#if defined(USE_SSE41)
        __m128i Result = _mm_cmpeq_epi8(Bitboard_, BB.Bitboard_); // Compare
        return _mm_movemask_epi8(Result) != 0xFFFF;
#elif defined(USE_NEON)
        const uint64x2_t Compare = vceqq_u64(Bitboard_, BB.Bitboard_);
        return (vgetq_lane_u64(Compare, 0) & vgetq_lane_u64(Compare, 1)) !=
               ~0ULL;
#else
        return (Primitive[0] != BB.Primitive[0]) ||
               (Primitive[1] != BB.Primitive[1]);
#endif
    }

    [[maybe_unused]] [[nodiscard]] inline
        __attribute__((always_inline)) Bitboard
        subtract(const Bitboard& BB) const noexcept {
#if defined(USE_SSE41)
        return _mm_sub_epi64(Bitboard_, BB.Bitboard_);
#elif defined(USE_NEON)
        return vsubq_u64(Bitboard_, BB.Bitboard_);
#else
        return Bitboard(Primitive[1] - BB.Primitive[1],
                        Primitive[0] - BB.Primitive[0]);
#endif
    }

    // Returns, in both lanes, all ones when the selected lane is zero and
    // zero otherwise. Note that "all ones" means all 128 bits, unlike
    // AllBB() which covers only the 81 board squares.
    template <bool High>
    [[nodiscard]] inline __attribute__((always_inline)) Bitboard
    getAllOnesIfLaneEmpty() const noexcept {
#if defined(USE_SSE41)
        const __m128i IsZero = _mm_cmpeq_epi64(Bitboard_, _mm_setzero_si128());
        return _mm_shuffle_epi32(IsZero, High ? 0xEE : 0x44);
#elif defined(USE_NEON)
        const uint64x2_t IsZero = vceqq_u64(Bitboard_, vdupq_n_u64(0));
        if constexpr (High) {
            return vdupq_laneq_u64(IsZero, 1);
        } else {
            return vdupq_laneq_u64(IsZero, 0);
        }
#else
        const uint64_t Mask = (Primitive[High ? 1 : 0] == 0) ? ~0ULL : 0ULL;
        return Bitboard(Mask, Mask);
#endif
    }

    template <bool High>
    [[nodiscard]] inline __attribute__((always_inline)) uint64_t
    getPrimitive() const noexcept {
#if defined(USE_SSE41)
        // _mm_cvtsi128_si64 is faster by one latency than _mm_extract_epi64
        // so use the function to extract the lower bits.
        return High ? (uint64_t)_mm_extract_epi64(Bitboard_, 1)
                    : (uint64_t)_mm_cvtsi128_si64(Bitboard_);
#elif defined(USE_NEON)
        return vgetq_lane_u64(Bitboard_, (int)High);
#else
        return High ? Primitive[1] : Primitive[0];
#endif
    }

    inline __attribute__((always_inline)) int
    countTrailingZero(uint64_t Value) const noexcept {
#if defined(USE_BMI1)
        return (int)_tzcnt_u64(Value);
#endif
        return std::countr_zero(Value);
    }

    [[maybe_unused]] [[nodiscard]] inline __attribute__((always_inline)) Square
    getOne() const noexcept {
        uint64_t Low = getPrimitive<false>();
        if (Low > 0) {
            return static_cast<Square>(countTrailingZero(Low));
        }

        uint64_t High = getPrimitive<true>();
        assert(High > 0);

        return static_cast<Square>(63 + countTrailingZero(High));
    }

    inline __attribute__((always_inline)) Square popOne() noexcept {
        const uint64_t Low = getPrimitive<false>();
        if (Low > 0) {
            Square Sq = static_cast<Square>(countTrailingZero(Low));

#if defined(USE_SSE41)
            Bitboard_ = _mm_and_si128(
                Bitboard_, _mm_sub_epi64(Bitboard_, _mm_set_epi64x(0, 1)));
#elif defined(USE_NEON)
            Bitboard_ = vandq_u64(
                Bitboard_, vsubq_u64(Bitboard_, vcombine_u64(vdup_n_u64(1),
                                                             vdup_n_u64(0))));
#else
            Primitive[0] &= (Primitive[0] - 1);
#endif
            return Sq;
        }

        const uint64_t High = getPrimitive<true>();
        Square Sq = static_cast<Square>(63 + countTrailingZero(High));

#if defined(USE_SSE41)
        Bitboard_ = _mm_and_si128(
            Bitboard_, _mm_sub_epi64(Bitboard_, _mm_set_epi64x(1, 0)));
#elif defined(USE_NEON)
        Bitboard_ = vandq_u64(
            Bitboard_,
            vsubq_u64(Bitboard_, vcombine_u64(vdup_n_u64(0), vdup_n_u64(1))));
#else
        Primitive[1] &= (Primitive[1] - 1);
#endif

        return Sq;
    }

    // Use template of a function since using
    // std::function<void(Square)> can have overhead.
    template <typename FuncType>
        requires std::is_invocable_v<FuncType, Square>
    inline __attribute__((always_inline)) void
    forEach(FuncType Func) const noexcept {
        uint64_t B = getPrimitive<false>();

        while (B != 0) {
            Square Sq = static_cast<Square>(countTrailingZero(B));
            B &= (B - 1);
            Func(Sq);
        }

        B = getPrimitive<true>();
        while (B != 0) {
            Square Sq = static_cast<Square>(63 + countTrailingZero(B));
            B &= (B - 1);
            Func(Sq);
        }
    }

    template <typename FuncType>
        requires std::is_invocable_r_v<bool, FuncType, Square>
    inline __attribute__((always_inline)) bool
    any(FuncType Func) const noexcept {
        uint64_t B = getPrimitive<false>();

        while (B != 0) {
            Square Sq = static_cast<Square>(countTrailingZero(B));
            B &= (B - 1);
            const bool Result = Func(Sq);

            if (Result) {
                return true;
            }
        }

        B = getPrimitive<true>();
        while (B != 0) {
            Square Sq = static_cast<Square>(63 + countTrailingZero(B));
            B &= (B - 1);
            const bool Result = Func(Sq);

            if (Result) {
                return true;
            }
        }

        return false;
    }

    [[maybe_unused]] [[nodiscard]] uint64_t horizontalOr() const noexcept {
        return getPrimitive<false>() | getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] uint64_t horizontalAnd() const noexcept {
        return getPrimitive<false>() & getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] uint64_t horizontalXor() const noexcept {
        return getPrimitive<false>() ^ getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] uint8_t popCount() const noexcept {
#if defined(USE_AVX2)
        const auto Count1 = _mm_popcnt_u64(getPrimitive<false>());
        const auto Count2 = _mm_popcnt_u64(getPrimitive<true>());
#else
        const auto Count1 = std::popcount(getPrimitive<false>());
        const auto Count2 = std::popcount(getPrimitive<true>());
#endif

        return (uint8_t)(Count1 + Count2);
    }

#if defined(USE_SSE41)
    __m128i getRaw() const noexcept {
        return Bitboard_;
    }
#elif defined(USE_NEON)
    uint64x2_t getRaw() const noexcept {
        return Bitboard_;
    }
#endif

 private:
#if defined(USE_SSE41)
    __m128i Bitboard_;
#elif defined(USE_NEON)
    uint64x2_t Bitboard_;
#else
    uint64_t Primitive[2];
#endif
};

// Per-square ray masks for computing slider attacks with bit operations
// only. Each of the two 64-bit lanes of a ray is resolved independently
// and branchlessly (see getAscendingRayAttackBB() and
// getDescendingRayAttackBB()); when a ray crosses the lane border, the
// lane farther from the origin square is masked out unless the nearer
// lane is free of blockers.
struct alignas(64) BishopRay {
    Bitboard NorthWest; // ascending
    Bitboard SouthWest; // ascending
    Bitboard NorthEast; // descending
    Bitboard SouthEast; // descending
};

struct alignas(64) RookRay {
    Bitboard North; // ascending
    Bitboard West;  // ascending
    Bitboard South; // descending
    Bitboard East;  // descending
};

extern BishopRay BishopRayBB[NumSquares];
extern RookRay RookRayBB[NumSquares];

template <Color C, PieceTypeKind Type>
inline Bitboard getAttackBB(Square From) noexcept {
    static_assert(Type != PTK_Lance,
                  "getAttackBB() is not implemented for PieceType::PTK_Lance");
    static_assert(Type != PTK_Bishop,
                  "getAttackBB() is not implemented for PieceType::PTK_Bishop");
    static_assert(Type != PTK_Rook,
                  "getAttackBB() is not implemented for PieceType::PTK_Rook");

    if constexpr (Type == PTK_Pawn) {
        if constexpr (C == Black) {
            return SquareBB[From].getLeftShiftEpi64<1>();
        } else {
            return SquareBB[From].getRightShiftEpi64<1>();
        }
    }
    if constexpr (Type == PTK_Knight) {
        return StepPieceAttackBB[C][0][From];
    } else if constexpr (Type == PTK_Silver) {
        return StepPieceAttackBB[C][1][From];
    } else if constexpr (Type == PTK_Gold || Type == PTK_ProPawn ||
                         Type == PTK_ProLance || Type == PTK_ProKnight ||
                         Type == PTK_ProSilver) {
        return StepPieceAttackBB[C][2][From];
    } else if constexpr (Type == PTK_King || Type == PTK_ProBishop ||
                         Type == PTK_ProRook) {
        return KingAttackBB[From];
    } else {
        assert(false);
        return Bitboard::ZeroBB();
    }
}

template <PieceTypeKind Type>
inline Bitboard getAttackBB(Color C, Square From) noexcept {
    static_assert(Type != PTK_Lance,
                  "getAttackBB() is not implemented for PieceType::PTK_Lance");
    static_assert(Type != PTK_Bishop,
                  "getAttackBB() is not implemented for PieceType::PTK_Bishop");
    static_assert(Type != PTK_Rook,
                  "getAttackBB() is not implemented for PieceType::PTK_Rook");

    if constexpr (Type == PTK_Pawn) {
        if (C == Black) {
            return SquareBB[From].getLeftShiftEpi64<1>();
        } else {
            return SquareBB[From].getRightShiftEpi64<1>();
        }
    }
    if constexpr (Type == PTK_Knight) {
        return StepPieceAttackBB[C][0][From];
    } else if constexpr (Type == PTK_Silver) {
        return StepPieceAttackBB[C][1][From];
    } else if constexpr (Type == PTK_Gold || Type == PTK_ProPawn ||
                         Type == PTK_ProLance || Type == PTK_ProKnight ||
                         Type == PTK_ProSilver) {
        return StepPieceAttackBB[C][2][From];
    } else if constexpr (Type == PTK_King || Type == PTK_ProBishop ||
                         Type == PTK_ProRook) {
        return KingAttackBB[From];
    } else {
        assert(false);
        return Bitboard::ZeroBB();
    }
}

template <Color C>
inline Bitboard getStepAttackBB(PieceTypeKind Type, Square From) noexcept {
    switch (Type) {
    case PTK_Pawn:
        return getAttackBB<C, PTK_Pawn>(From);
    case PTK_Knight:
        return getAttackBB<C, PTK_Knight>(From);
    case PTK_Silver:
        return getAttackBB<C, PTK_Silver>(From);
    case PTK_Gold:
    case PTK_ProPawn:
    case PTK_ProLance:
    case PTK_ProKnight:
    case PTK_ProSilver:
        return getAttackBB<C, PTK_Gold>(From);
    case PTK_King:
    case PTK_ProBishop:
    case PTK_ProRook:
        return getAttackBB<C, PTK_King>(From);
    default:
        return Bitboard::ZeroBB();
    }
}

// Computes the attacked squares along a ray whose squares ascend in bit
// position as the ray leaves its origin square. Within each lane the same
// sweep as the black lance is used: subtracting one flips exactly the bits
// at or below the lowest set bit of (occupancy & ray), i.e. the blocker
// nearest to the origin (all bits when the lane is empty), so the xor
// spans the ray squares up to and including that blocker. The low lane
// holds the part of the ray closer to the origin, so when the ray crosses
// the lane border, the high lane contributes only when the low lane has
// no blocker.
template <bool CrossesLanes>
inline __attribute__((always_inline)) Bitboard getAscendingRayAttackBB(
    const Bitboard& OccupiedBB, const Bitboard& RayBB) noexcept {
    const Bitboard OccupiedOnRayBB = OccupiedBB & RayBB;
    const Bitboard AttackBB =
        (OccupiedOnRayBB ^ OccupiedOnRayBB.subtract(Bitboard(1, 1))) & RayBB;

    if constexpr (CrossesLanes) {
        return AttackBB & (OccupiedOnRayBB.getAllOnesIfLaneEmpty<false>() |
                           Bitboard(0, ~0ULL));
    }

    return AttackBB;
}

// The descending-ray counterpart of getAscendingRayAttackBB(): the blocker
// nearest to the origin square is now the highest set bit. Within one
// lane, consecutive ray squares are exactly `StepBits` bit positions
// apart, and one lane holds at most eight ray squares, so smearing the
// blockers downward along that grid by 1 + 2 + 4 steps reaches every ray
// square below the nearest blocker; shifting the smear one more step
// excludes the blocker itself, and whatever it does not cover is the
// attacked part of the ray. The high lane holds the part of the ray closer
// to the origin here.
template <int StepBits, bool CrossesLanes>
inline __attribute__((always_inline)) Bitboard getDescendingRayAttackBB(
    const Bitboard& OccupiedBB, const Bitboard& RayBB) noexcept {
    const Bitboard OccupiedOnRayBB = OccupiedBB & RayBB;

    // Smear the blockers downward so that every ray square below a
    // blocker gets filled.
    Bitboard SmearBB = OccupiedOnRayBB;
    SmearBB |= SmearBB.getRightShiftEpi64<StepBits>();
    SmearBB |= SmearBB.getRightShiftEpi64<2 * StepBits>();
    SmearBB |= SmearBB.getRightShiftEpi64<4 * StepBits>();
    const Bitboard AttackBB =
        SmearBB.getRightShiftEpi64<StepBits>().andNot(RayBB);

    if constexpr (CrossesLanes) {
        // The smear does not propagate across the lane border, so if the
        // high lane has a blocker, keep the high lane only.
        return AttackBB & (OccupiedOnRayBB.getAllOnesIfLaneEmpty<true>() |
                           Bitboard(~0ULL, 0));
    }

    return AttackBB;
}

template <Color C>
inline Bitboard getLanceAttackBB(Square Sq,
                                 const Bitboard& OccupiedBB) noexcept {
    if constexpr (C == Black) {
        if (Bitboard::FurthermostBB<C>().isSet(Sq)) {
            return bitboard::Bitboard::ZeroBB();
        }

        auto Temp = OccupiedBB ^ (OccupiedBB.subtract(SquareBB[Sq + North]));
        return Temp & ForwardBB[Sq];
    } else {
        // No guard for the furthermost rank: BackwardBB[Sq] is empty there
        // and the ray sweep returns the empty bitboard by itself.
        return getDescendingRayAttackBB<-South, false>(OccupiedBB,
                                                       BackwardBB[Sq]);
    }
}

inline Bitboard getLanceAttackBB(Color C, Square Sq,
                                 const Bitboard& OccupiedBB) noexcept {
    if (C == Black) {
        if (Bitboard::FurthermostBB(C).isSet(Sq)) {
            return Bitboard::ZeroBB();
        }

        auto Temp = OccupiedBB ^ (OccupiedBB.subtract(SquareBB[Sq + North]));
        return Temp & ForwardBB[Sq];
    } else {
        return getDescendingRayAttackBB<-South, false>(OccupiedBB,
                                                       BackwardBB[Sq]);
    }
}

#if defined(USE_AVX2)

// AVX2 variants that resolve two rays at once, one ray in each 128-bit
// half of a 256-bit register. The ray pairs are laid out adjacently in
// BishopRay/RookRay so that one aligned 256-bit load fetches both. The
// lane gate is applied to both halves unconditionally: for a ray that
// stays within one 64-bit lane the gated (far) side of the ray is empty,
// so the gate is a no-op there.

inline __attribute__((always_inline)) __m256i
loadRayPair(const Bitboard* RayPair) noexcept {
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(RayPair));
}

inline __attribute__((always_inline)) __m256i
getAscendingRayPairAttack(__m256i Occupied2, __m256i Ray2) noexcept {
    const __m256i OccupiedOnRay = _mm256_and_si256(Occupied2, Ray2);
    const __m256i Swept = _mm256_xor_si256(
        OccupiedOnRay, _mm256_sub_epi64(OccupiedOnRay, _mm256_set1_epi64x(1)));
    const __m256i Attack = _mm256_and_si256(Swept, Ray2);

    const __m256i LaneIsZero =
        _mm256_cmpeq_epi64(OccupiedOnRay, _mm256_setzero_si256());
    const __m256i Gate =
        _mm256_or_si256(_mm256_shuffle_epi32(LaneIsZero, 0x44),
                        _mm256_set_epi64x(0, -1, 0, -1));
    return _mm256_and_si256(Attack, Gate);
}

// `StepBits0` applies to the ray in the low 128-bit half, `StepBits1` to
// the one in the high half.
template <int StepBits0, int StepBits1>
inline __attribute__((always_inline)) __m256i
getDescendingRayPairAttack(__m256i Occupied2, __m256i Ray2) noexcept {
    const __m256i Step1 =
        _mm256_set_epi64x(StepBits1, StepBits1, StepBits0, StepBits0);
    const __m256i Step2 =
        _mm256_set_epi64x(2 * StepBits1, 2 * StepBits1, 2 * StepBits0,
                          2 * StepBits0);
    const __m256i Step4 =
        _mm256_set_epi64x(4 * StepBits1, 4 * StepBits1, 4 * StepBits0,
                          4 * StepBits0);

    const __m256i OccupiedOnRay = _mm256_and_si256(Occupied2, Ray2);
    __m256i Smear = OccupiedOnRay;
    Smear = _mm256_or_si256(Smear, _mm256_srlv_epi64(Smear, Step1));
    Smear = _mm256_or_si256(Smear, _mm256_srlv_epi64(Smear, Step2));
    Smear = _mm256_or_si256(Smear, _mm256_srlv_epi64(Smear, Step4));
    const __m256i Attack =
        _mm256_andnot_si256(_mm256_srlv_epi64(Smear, Step1), Ray2);

    const __m256i LaneIsZero =
        _mm256_cmpeq_epi64(OccupiedOnRay, _mm256_setzero_si256());
    const __m256i Gate =
        _mm256_or_si256(_mm256_shuffle_epi32(LaneIsZero, 0xEE),
                        _mm256_set_epi64x(-1, 0, -1, 0));
    return _mm256_and_si256(Attack, Gate);
}

inline __attribute__((always_inline)) Bitboard
mergeRayPair(__m256i Attack2) noexcept {
    return Bitboard(_mm_or_si128(_mm256_castsi256_si128(Attack2),
                                 _mm256_extracti128_si256(Attack2, 1)));
}

#endif

template <PieceTypeKind Type>
inline Bitboard getBishopAttackBB(Square Sq,
                                  const Bitboard& OccupiedBB) noexcept {
    static_assert(
        Type == PTK_Bishop || Type == PTK_ProBishop,
        "the template parameter `Type` must be PTK_Bishop or PTK_ProBishop.");

    const BishopRay& Rays = BishopRayBB[Sq];

#if defined(USE_AVX2)
    const __m256i Occupied2 = _mm256_broadcastsi128_si256(OccupiedBB.getRaw());
    const Bitboard AttackBB = mergeRayPair(_mm256_or_si256(
        getAscendingRayPairAttack(Occupied2, loadRayPair(&Rays.NorthWest)),
        getDescendingRayPairAttack<-NorthEast, -SouthEast>(
            Occupied2, loadRayPair(&Rays.NorthEast))));
#else
    // The step of each descending ray is the bit-position distance of its
    // consecutive squares within one lane, i.e. the negated direction.
    const Bitboard AttackBB =
        (getAscendingRayAttackBB<true>(OccupiedBB, Rays.NorthWest) |
         getAscendingRayAttackBB<true>(OccupiedBB, Rays.SouthWest)) |
        (getDescendingRayAttackBB<-NorthEast, true>(OccupiedBB,
                                                    Rays.NorthEast) |
         getDescendingRayAttackBB<-SouthEast, true>(OccupiedBB,
                                                    Rays.SouthEast));
#endif

    if constexpr (Type == PTK_ProBishop) {
        return AttackBB | KingAttackBB[Sq];
    }

    return AttackBB;
}

template <PieceTypeKind Type>
inline Bitboard getRookAttackBB(Square Sq,
                                const Bitboard& OccupiedBB) noexcept {
    static_assert(
        Type == PTK_Rook || Type == PTK_ProRook,
        "the template parameter `Type` must be PTK_Rook or PTK_ProRook.");

    const RookRay& Rays = RookRayBB[Sq];

#if defined(USE_AVX2)
    const __m256i Occupied2 = _mm256_broadcastsi128_si256(OccupiedBB.getRaw());
    const Bitboard AttackBB = mergeRayPair(_mm256_or_si256(
        getAscendingRayPairAttack(Occupied2, loadRayPair(&Rays.North)),
        getDescendingRayPairAttack<-South, -East>(Occupied2,
                                                  loadRayPair(&Rays.South))));
#else
    // A file never crosses the lane border, so the north and south rays
    // skip the lane gate.
    const Bitboard AttackBB =
        (getAscendingRayAttackBB<false>(OccupiedBB, Rays.North) |
         getAscendingRayAttackBB<true>(OccupiedBB, Rays.West)) |
        (getDescendingRayAttackBB<-South, false>(OccupiedBB, Rays.South) |
         getDescendingRayAttackBB<-East, true>(OccupiedBB, Rays.East));
#endif

    if constexpr (Type == PTK_ProRook) {
        return AttackBB | KingAttackBB[Sq];
    }

    return AttackBB;
}

template <Color C, PieceTypeKind Type>
inline Bitboard getSliderAttackBB(Square Sq,
                                  const Bitboard& OccupiedBB) noexcept {
    static_assert(Type == PTK_Lance || Type == PTK_Bishop ||
                  Type == PTK_ProBishop || Type == PTK_Rook ||
                  Type == PTK_ProRook);

    if constexpr (Type == PTK_Lance) {
        return getLanceAttackBB<C>(Sq, OccupiedBB);
    } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
        return getBishopAttackBB<Type>(Sq, OccupiedBB);
    } else {
        return getRookAttackBB<Type>(Sq, OccupiedBB);
    }
}

template <Color C>
inline Bitboard getSliderAttackBB(PieceTypeKind Type, Square Sq,
                                  const Bitboard& OccupiedBB) noexcept {
    switch (Type) {
    case PTK_Lance:
        return getSliderAttackBB<C, PTK_Lance>(Sq, OccupiedBB);
    case PTK_Bishop:
    case PTK_ProBishop:
        return getSliderAttackBB<C, PTK_Bishop>(Sq, OccupiedBB);
    case PTK_Rook:
    case PTK_ProRook:
        return getSliderAttackBB<C, PTK_Rook>(Sq, OccupiedBB);
    default:
        return Bitboard::ZeroBB();
    }
}

inline Bitboard getBetweenBB(Square Sq1, Square Sq2) noexcept {
    return BetweenBB[Sq1][Sq2];
}

inline Bitboard getDiagBB(Square Sq) noexcept {
    return DiagBB[Sq];
}

inline Bitboard getCrossBB(Square Sq) noexcept {
    return CrossBB[Sq];
}

template <Color C>
inline Bitboard getForwardBB(Square Sq) noexcept {
    if constexpr (C == Black) {
        return ForwardBB[Sq];
    } else {
        return BackwardBB[Sq];
    }
}

void initializeBitboards();

} // namespace bitboard
} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_BITBOARD_H
