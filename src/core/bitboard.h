#ifndef NSHOGI_CORE_BITBOARD_H
#define NSHOGI_CORE_BITBOARD_H

#include "types.h"
#include <functional>
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstdint>

#if defined(USE_SSE2)

#include <emmintrin.h>

#endif

#if defined(USE_SSE41)

#include <smmintrin.h>

#endif

#if defined(USE_BMI1) || defined(USE_BMI2) || defined(USE_AVX2)

#include <immintrin.h>

#endif

#include <type_traits>

namespace nshogi {
namespace core {
namespace bitboard {

struct Bitboard;

extern const Bitboard SquareBB[NumSquares];
extern const Bitboard FileBB[NumFiles];
extern const Bitboard RankBB[NumRanks];
// extern const Bitboard AllBB;

extern Bitboard KnightAttackBB[NumColors][NumSquares];
extern Bitboard SilverAttackBB[NumColors][NumSquares];
extern Bitboard GoldAttackBB[NumColors][NumSquares];
extern Bitboard KingAttackBB[NumSquares];
extern Bitboard DiagStepAttackBB[NumSquares];
extern Bitboard CrossStepAttackBB[NumSquares];

extern Bitboard ForwardAttackBB[NumSquares][1 << 7];
extern Bitboard BackwardAttackBB[NumSquares][1 << 7];

//  Useful bitboards.
extern const Bitboard FirstAndSecondFurthestBB[NumColors];
extern const Bitboard FurthermostBB[NumColors];
extern const Bitboard SecondFurthestBB[NumColors];
extern const Bitboard PromotableBB[NumColors];

extern Bitboard LineBB[NumSquares][NumSquares];
extern Bitboard DirectionBB[11 + NorthNorthWest + 1][NumSquares];
extern Bitboard BetweenBB[NumSquares][NumSquares];
extern Bitboard DiagBB[NumSquares];
extern Bitboard CrossBB[NumSquares];
extern Bitboard ForwardBB[NumSquares];
extern Bitboard BackwardBB[NumSquares];

struct alignas(16) Bitboard {
    constexpr Bitboard() {
    }

    static constexpr Bitboard ZeroBB() {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_setzero_si128();
        }
#endif
        return Bitboard(0, 0);
    }

    static constexpr Bitboard AllBB() {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_set_epi64x(0x3ffff, 0x7fffffffffffffff);
        }
#endif
        return Bitboard(0x3ffff, 0x7fffffffffffffff);
    }

    constexpr Bitboard(uint64_t High, uint64_t Low) : Primitive{Low, High} {
    }

#if defined(USE_SSE2)
    constexpr Bitboard(const __m128i& BB) : Bitboard_(BB) {
    }
#endif

#if defined(USE_SSE2)
    Bitboard(const Bitboard& BB) : Bitboard_(BB.Bitboard_) {
    }
#else
    constexpr Bitboard(const Bitboard& BB) : Primitive { BB.getPrimitive<false>(), BB.getPrimitive<true>() } {
    }
#endif

    void copyFrom(const Bitboard& BB) {
#if defined(USE_SSE2)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.getPrimitive<false>();
        Primitive[1] = BB.getPrimitive<true>();
#endif
    }

    constexpr Bitboard& operator=(Bitboard&& BB) noexcept {
#if defined(USE_SSE2)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.Primitive[0];
        Primitive[1] = BB.Primitive[1];
#endif
        return *this;
    }

    constexpr Bitboard& operator=(const Bitboard& BB) noexcept {
#if defined(USE_SSE2)
        Bitboard_ = BB.Bitboard_;
#else
        Primitive[0] = BB.Primitive[0];
        Primitive[1] = BB.Primitive[1];
#endif
        return *this;
    }

    // Getters.
    constexpr bool isZero() const {
#if defined(USE_SSE41)
        if (!std::is_constant_evaluated()) {
            return _mm_testz_si128(Bitboard_, Bitboard_);
        }
#endif
        return Primitive[0] == 0 && Primitive[1] == 0;
    }

    constexpr bool isSet(Square Sq) const {
#if defined(USE_SSE41)
        if (!std::is_constant_evaluated()) {
            return !_mm_testz_si128(Bitboard_, SquareBB[Sq].Bitboard_);
        }
#endif
        return !(*this & SquareBB[Sq]).isZero();
    }

    // Setters.
    constexpr void clear() {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            Bitboard_ = _mm_setzero_si128();
        }
#else
        Primitive[0] = 0;
        Primitive[1] = 0;
#endif
    }

    constexpr void toggleBit(Square Sq) {
        (*this) ^= SquareBB[Sq];
    }

    // Bit operations.
    constexpr Bitboard& operator|=(const Bitboard& BB) {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            Bitboard_ = _mm_or_si128(Bitboard_, BB.Bitboard_);
            return *this;
        }
#endif
        Primitive[1] |= BB.Primitive[1];
        Primitive[0] |= BB.Primitive[0];
        return *this;
    }

    constexpr Bitboard operator|(const Bitboard& BB) const {
        if (std::is_constant_evaluated()) {
            Bitboard Res(Primitive[1], Primitive[0]);
            Res |= BB;
            return { Res.Primitive[1], Res.Primitive[0] };
        } else {
            Bitboard Res = *this;
            Res |= BB;
            return Res;
        }
    }

    constexpr Bitboard& operator&=(const Bitboard& BB) {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            Bitboard_ = _mm_and_si128(Bitboard_, BB.Bitboard_);
            return *this;
        }
#endif
        Primitive[1] &= BB.Primitive[1];
        Primitive[0] &= BB.Primitive[0];
        return *this;
    }

    constexpr Bitboard operator&(const Bitboard& BB) const {
        if (std::is_constant_evaluated()) {
            Bitboard Res(Primitive[1], Primitive[0]);
            Res &= BB;
            return { Res.Primitive[1], Res.Primitive[0] };
        } else {
            Bitboard Res = *this;
            Res &= BB;
            return Res;
        }
    }

    constexpr Bitboard& operator^=(const Bitboard& BB) {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            Bitboard_ = _mm_xor_si128(Bitboard_, BB.Bitboard_);
            return *this;
        }
#endif
        Primitive[1] ^= BB.Primitive[1];
        Primitive[0] ^= BB.Primitive[0];
        return *this;
    }

    constexpr Bitboard operator^(const Bitboard& BB) const {
        if (std::is_constant_evaluated()) {
            Bitboard Res(Primitive[1], Primitive[0]);
            Res ^= BB;
            return { Res.Primitive[1], Res.Primitive[0] };
        } else {
            Bitboard Res = *this;
            Res ^= BB;
            return Res;
        }
    }

    constexpr Bitboard andNot(const Bitboard& BB) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_andnot_si128(Bitboard_, BB.Bitboard_);
        }
#endif
        return { ~Primitive[1] & BB.Primitive[1], ~Primitive[0] & BB.Primitive[0] };
    }

    constexpr Bitboard operator~() const {
        return andNot(AllBB());
    }

    template <int Shift>
    constexpr Bitboard getRightShiftEpi64() const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_srli_epi64(Bitboard_, Shift);
        }
#endif
        return Bitboard(Primitive[1] >> Shift, Primitive[0] >> Shift);
    }

    constexpr Bitboard getRightShiftEpi64(int Shift) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_srli_epi64(Bitboard_, Shift);
        }
#endif
        return Bitboard(Primitive[1] >> Shift, Primitive[0] >> Shift);
    }

    template <int Shift>
    constexpr Bitboard getRightShiftSi128() const {
        // We can't use _mm_slli_si128 here when Shift % 8 == 0 holds because
        // the bitboard is composed of two 64-bit variables.
        // One of them uses 63 bits to represent the first 7 files (7 files * 9 squares = 63),
        // and the other one uses 18 bits for the remaining 2 files (2 files * 9 squares = 18).
        // Therefore, we must pay attention to the 1 bit of the former one.
        // Using _mm_slli_si128 collapses this configuration.
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            const uint64_t Carry = Primitive[1] << (63 - Shift);
            Bitboard BB = getRightShiftEpi64(Shift);
            BB.Primitive[0] |= Carry;
            return BB;
        }
#endif
        return Bitboard(Primitive[1] >> Shift,
                        (Primitive[0] >> Shift) | (Primitive[1] << (63 - Shift)));
    }

    template <int Shift>
    constexpr Bitboard getLeftShiftEpi64() const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_slli_epi64(Bitboard_, Shift);
        }
#endif
        return Bitboard(Primitive[1] << Shift, Primitive[0] << Shift);
    }

    constexpr Bitboard getLeftShiftEpi64(int Shift) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_slli_epi64(Bitboard_, Shift);
        }
#endif
        return Bitboard(Primitive[1] << Shift, Primitive[0] << Shift);
    }

    template <int Shift>
    constexpr Bitboard getLeftShiftSi128() const {
        // See the comment in getRightShiftSi128().
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            const uint64_t Carry = Primitive[0] >> (63 - Shift);
            Bitboard BB = getLeftShiftEpi64(Shift);
            BB.Primitive[1] |= Carry;
            return BB;
        }
#endif
        return Bitboard((Primitive[1] << Shift) | (Primitive[0] >> (63 - Shift)),
                        Primitive[0] << Shift);
    }

    constexpr bool operator==(const Bitboard& BB) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            __m128i Result = _mm_cmpeq_epi8(Bitboard_, BB.Bitboard_); // Compare
            return _mm_movemask_epi8(Result) == 0xFFFF;
        }
#endif
        return Primitive[0] == BB.Primitive[0] && Primitive[1] == BB.Primitive[1];
    }

    constexpr bool operator!=(const Bitboard& BB) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            __m128i Result = _mm_cmpeq_epi8(Bitboard_, BB.Bitboard_); // Compare
            return _mm_movemask_epi8(Result) != 0xFFFF;
        }
#endif
        return (Primitive[0] != BB.Primitive[0]) || (Primitive[1] != BB.Primitive[1]);
    }

    [[maybe_unused]] [[nodiscard]] constexpr Bitboard subtract(const Bitboard& BB) const {
#if defined(USE_SSE2)
        if (!std::is_constant_evaluated()) {
            return _mm_sub_epi64(Bitboard_, BB.Bitboard_);
        }
#endif
        return Bitboard(Primitive[1] - BB.Primitive[1], Primitive[0] - BB.Primitive[0]);
    }

    template <bool High>
    [[nodiscard]] constexpr uint64_t getPrimitive() const {
#if defined(USE_SSE41)
        if (!std::is_constant_evaluated()) {
            // _mm_cvtsi128_si64 is faster by one latency than _mm_extract_epi64
            // so use the function to extract the lower bits.
            return High ? (uint64_t)_mm_extract_epi64(Bitboard_, 1)
                        : (uint64_t)_mm_cvtsi128_si64(Bitboard_);
        }
#endif
        return High? Primitive[1] : Primitive[0];
    }

    [[maybe_unused]] [[nodiscard]]
    Square getOne() const {
        uint64_t Low = getPrimitive<false>();
        if (Low > 0) {
#if defined(USE_BMI1)
            return static_cast<Square>(_tzcnt_u64(Low));
#else
            return static_cast<Square>(__builtin_ctzll(Low));
#endif
        }

        uint64_t High = getPrimitive<true>();
        assert(High > 0);

#if defined(USE_BMI1)
        return static_cast<Square>(63 + _tzcnt_u64(High));
#else
        return static_cast<Square>(63 + __builtin_ctzll(High));
#endif
    }

    Square popOne() {
        const uint64_t Low = getPrimitive<false>();
        // 63 bits in `Low` is used while only 18 bits in `High` is used
        // so Low > 0 is more likely to happen.
        if (__builtin_expect((Low > 0), 1)) {
            Square Sq = static_cast<Square>(__builtin_ctzll(Low));

#if defined(USE_SSE2)
            Bitboard_ = _mm_and_si128(
                Bitboard_, _mm_sub_epi64(Bitboard_, _mm_set_epi64x(0, 1)));
#else
            Primitive[0] &= (Primitive[0] - 1);
#endif

            return Sq;
        }

        const uint64_t High = getPrimitive<true>();

        Square Sq = static_cast<Square>(63 + __builtin_ctzll(High));
#if defined(USE_SSE2)
        Bitboard_ = _mm_and_si128(
            Bitboard_, _mm_sub_epi64(Bitboard_, _mm_set_epi64x(1, 0)));
#else
        Primitive[1] &= (Primitive[1] - 1);
#endif

        return Sq;
    }

    // Use template of a function since using
    // std::function<void(Square)> can have overhead.
    template <typename FuncType>
    requires std::is_invocable_v<FuncType, Square>
    void forEach(FuncType Func) const {
        uint64_t B = getPrimitive<false>();

        while (B != 0) {
#if defined(USE_BMI1)
            Square Sq = static_cast<Square>(_tzcnt_u64(B));
#else
            Square Sq = static_cast<Square>(__builtin_ctzll(B));
#endif
            B &= (B - 1);
            Func(Sq);
        }

        B = getPrimitive<true>();
        while (B != 0) {
#if defined(USE_BMI1)
            Square Sq = static_cast<Square>(63 + _tzcnt_u64(B));
#else
            Square Sq = static_cast<Square>(63 + __builtin_ctzll(B));
#endif
            B &= (B - 1);
            Func(Sq);
        }
    }

    template <typename FuncType>
    requires std::is_invocable_r_v<bool, FuncType, Square>
    bool any(FuncType Func) const {
        uint64_t B = getPrimitive<false>();

        while (B != 0) {
#if defined(USE_BMI1)
            Square Sq = static_cast<Square>(_tzcnt_u64(B));
#else
            Square Sq = static_cast<Square>(__builtin_ctzll(B));
#endif
            B &= (B - 1);
            const bool Result = Func(Sq);

            if (Result) {
                return true;
            }
        }

        B = getPrimitive<true>();
        while (B != 0) {
#if defined(USE_BMI1)
            Square Sq = static_cast<Square>(63 + _tzcnt_u64(B));
#else
            Square Sq = static_cast<Square>(63 + __builtin_ctzll(B));
#endif
            B &= (B - 1);
            const bool Result = Func(Sq);

            if (Result) {
                return true;
            }
        }

        return false;
    }

    [[maybe_unused]] [[nodiscard]] constexpr uint64_t horizontalOr() const {
        return getPrimitive<false>() | getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] constexpr uint64_t horizontalAnd() const {
        return getPrimitive<false>() & getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] constexpr uint64_t horizontalXor() const {
        return getPrimitive<false>() ^ getPrimitive<true>();
    }

    [[maybe_unused]] [[nodiscard]] uint8_t popCount() const {
#if defined(USE_AVX2)
        const auto Count1 = _mm_popcnt_u64(getPrimitive<false>());
        const auto Count2 = _mm_popcnt_u64(getPrimitive<true>());
#else
        const auto Count1 = __builtin_popcountll(getPrimitive<false>());
        const auto Count2 = __builtin_popcountll(getPrimitive<true>());
#endif

        return (uint8_t)(Count1 + Count2);
    }

#if defined(USE_SSE2)
    constexpr __m128i getRaw() const {
        return Bitboard_;
    }
#endif

 private:
    // To implement constexpr constructors and for compatibility,
    // using union here with unsigned 64-bit integers rather than
    // just declaring `Bitboard_`.
    union {
#if defined(USE_SSE2)
        __m128i Bitboard_;
#endif
        uint64_t Primitive[2];
    };
};

// Magic bitboard.
template <uint64_t NumBits> struct MagicBitboard {
    MagicBitboard(){};

    uint64_t MagicNumber;
    Bitboard Masks[2];
    Bitboard AttackBB1[1 << NumBits];
    Bitboard AttackBB2[1 << NumBits];
};

constexpr uint64_t DiagMagicBits = 7;
extern MagicBitboard<DiagMagicBits> BishopMagicBB[NumSquares];

constexpr uint64_t CrossMagicBits = 7;
extern MagicBitboard<CrossMagicBits> RookMagicBB[NumSquares];

template <Color C, PieceTypeKind Type>
inline constexpr Bitboard getAttackBB(Square From) {
    static_assert(Type != PTK_Lance,
                  "getAttackBB() is not implemented for PieceType::PTK_Lance");
    static_assert(Type != PTK_Bishop,
                  "getAttackBB() is not implemented for PieceType::PTK_Bishop");
    static_assert(Type != PTK_Rook,
                  "getAttackBB() is not implemented for PieceType::PTK_Rook");
    static_assert(
        Type != PTK_ProBishop,
        "getAttackBB() is not implemented for PieceType::PTK_ProBishop");
    static_assert(
        Type != PTK_ProRook,
        "getAttackBB() is not implemented for PieceType::PTK_ProRook");

    if constexpr (Type == PTK_Pawn) {
        if constexpr (C == Black) {
            return SquareBB[From].getLeftShiftEpi64<1>();
        } else {
            return SquareBB[From].getRightShiftEpi64<1>();
        }
    }
    if constexpr (Type == PTK_Knight) {
        return KnightAttackBB[C][From];
    } else if constexpr (Type == PTK_Silver) {
        return SilverAttackBB[C][From];
    } else if constexpr (Type == PTK_Gold || Type == PTK_ProPawn ||
                         Type == PTK_ProLance || Type == PTK_ProKnight ||
                         Type == PTK_ProSilver) {
        return GoldAttackBB[C][From];
    }

    return KingAttackBB[From];
}

inline constexpr uint8_t pickUpFileBitPattern(Square Sq, const Bitboard& OccupiedBB) {
    const int ShiftTable[NumFiles] = {1, 10, 19, 28, 37, 46, 55, 1, 10};

    const File F = squareToFile(Sq);
    const Bitboard ShiftedBB = (OccupiedBB & FileBB[F]).getRightShiftEpi64(ShiftTable[F]);

    return ShiftedBB.horizontalOr() & 0x7f; // Pick up seven bits.
}

template <Color C>
inline Bitboard getLanceAttackBB(Square Sq, const Bitboard& OccupiedBB) {
    const uint8_t Pattern = pickUpFileBitPattern(Sq, OccupiedBB);

    if constexpr (C == Black) {
        return ForwardAttackBB[Sq][Pattern];
    } else {
        return BackwardAttackBB[Sq][Pattern];
    }
}

template <PieceTypeKind Type>
inline Bitboard getBishopAttackBB(Square Sq, const Bitboard& OccupiedBB) {
    static_assert(
        Type == PTK_Bishop || Type == PTK_ProBishop,
        "the template parameter `Type` must be PTK_Bishop or PTK_ProBishop.");

    const auto& Magic = BishopMagicBB[Sq];

    const uint16_t Pattern1 =
        (uint16_t)(((OccupiedBB & Magic.Masks[0]).horizontalOr() *
                    Magic.MagicNumber) >> (64 - DiagMagicBits));
    const uint16_t Pattern2 =
        (uint16_t)(((OccupiedBB & Magic.Masks[1]).horizontalOr() *
                    Magic.MagicNumber) >> (64 - DiagMagicBits));

    const auto AttackBB = Magic.AttackBB1[Pattern1] | Magic.AttackBB2[Pattern2];
    if constexpr (Type == PTK_ProBishop) {
        return AttackBB | KingAttackBB[Sq];
    }

    return AttackBB;
}

template <PieceTypeKind Type>
inline Bitboard getRookAttackBB(Square Sq, const Bitboard& OccupiedBB) {
    static_assert(
        Type == PTK_Rook || Type == PTK_ProRook,
        "the template parameter `Type` must be PTK_Rook or PTK_ProRook.");

    const auto& Magic = RookMagicBB[Sq];
    const uint16_t Pattern1 =
        (uint16_t)(((OccupiedBB & Magic.Masks[0]).horizontalOr() *
                    Magic.MagicNumber) >> (64 - CrossMagicBits));
    const uint16_t Pattern2 =
        (uint16_t)(((OccupiedBB & Magic.Masks[1]).horizontalOr() *
                    Magic.MagicNumber) >> (64 - CrossMagicBits));

    const auto AttackBB = Magic.AttackBB1[Pattern1] | Magic.AttackBB2[Pattern2];
    if constexpr (Type == PTK_ProRook) {
        return AttackBB | KingAttackBB[Sq];
    }

    return AttackBB;
}

inline bool isAttacked(const Bitboard& AttackBB, const Bitboard& ExistBB) {
    return !(AttackBB & ExistBB).isZero();
}

inline Bitboard getBetweenBB(Square Sq1, Square Sq2) {
    return BetweenBB[Sq1][Sq2];
}

inline Bitboard getDiagBB(Square Sq) {
    return DiagBB[Sq];
}

inline Bitboard getCrossBB(Square Sq) {
    return CrossBB[Sq];
}

template <Color C>
inline Bitboard getForwardBB(Square Sq) {
    if constexpr (C == Black) {
        return ForwardBB[Sq];
    } else {
        return BackwardBB[Sq];
    }
}

void initializeBitboards();

} // namespace bitboard
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_BITBOARD_H
