#include "bitboard.h"
#include "types.h"
#include <cstring>
#include <iostream>
#include <random>
#include <tuple>

namespace nshogi {
namespace core {
namespace bitboard {

using BB = Bitboard;

// clang-format off
constexpr Bitboard SquareBB[NumSquares] = {
    BB(0x00000ULL, 0x00000000000001ULL), BB(0x00000ULL, 0x00000000000002ULL), BB(0x000000ULL, 0x000000000000004ULL), BB(0x000000ULL, 0x000000000000008ULL), BB(0x000000ULL, 0x000000000000010ULL), BB(0x000000ULL, 0x000000000000020ULL), BB(0x0000000ULL, 0x0000000000000040ULL), BB(0x0000000ULL, 0x0000000000000080ULL), BB(0x0000000ULL, 0x0000000000000100ULL),
    BB(0x00000ULL, 0x00000000000200ULL), BB(0x00000ULL, 0x00000000000400ULL), BB(0x000000ULL, 0x000000000000800ULL), BB(0x000000ULL, 0x000000000001000ULL), BB(0x000000ULL, 0x000000000002000ULL), BB(0x000000ULL, 0x000000000004000ULL), BB(0x0000000ULL, 0x0000000000008000ULL), BB(0x0000000ULL, 0x0000000000010000ULL), BB(0x0000000ULL, 0x0000000000020000ULL),
    BB(0x00000ULL, 0x00000000040000ULL), BB(0x00000ULL, 0x00000000080000ULL), BB(0x000000ULL, 0x000000000100000ULL), BB(0x000000ULL, 0x000000000200000ULL), BB(0x000000ULL, 0x000000000400000ULL), BB(0x000000ULL, 0x000000000800000ULL), BB(0x0000000ULL, 0x0000000001000000ULL), BB(0x0000000ULL, 0x0000000002000000ULL), BB(0x0000000ULL, 0x0000000004000000ULL),
    BB(0x00000ULL, 0x00000008000000ULL), BB(0x00000ULL, 0x00000010000000ULL), BB(0x000000ULL, 0x000000020000000ULL), BB(0x000000ULL, 0x000000040000000ULL), BB(0x000000ULL, 0x000000080000000ULL), BB(0x000000ULL, 0x000000100000000ULL), BB(0x0000000ULL, 0x0000000200000000ULL), BB(0x0000000ULL, 0x0000000400000000ULL), BB(0x0000000ULL, 0x0000000800000000ULL),
    BB(0x00000ULL, 0x00001000000000ULL), BB(0x00000ULL, 0x00002000000000ULL), BB(0x000000ULL, 0x000004000000000ULL), BB(0x000000ULL, 0x000008000000000ULL), BB(0x000000ULL, 0x000010000000000ULL), BB(0x000000ULL, 0x000020000000000ULL), BB(0x0000000ULL, 0x0000040000000000ULL), BB(0x0000000ULL, 0x0000080000000000ULL), BB(0x0000000ULL, 0x0000100000000000ULL),
    BB(0x00000ULL, 0x00200000000000ULL), BB(0x00000ULL, 0x00400000000000ULL), BB(0x000000ULL, 0x000800000000000ULL), BB(0x000000ULL, 0x001000000000000ULL), BB(0x000000ULL, 0x002000000000000ULL), BB(0x000000ULL, 0x004000000000000ULL), BB(0x0000000ULL, 0x0008000000000000ULL), BB(0x0000000ULL, 0x0010000000000000ULL), BB(0x0000000ULL, 0x0020000000000000ULL),
    BB(0x00000ULL, 0x40000000000000ULL), BB(0x00000ULL, 0x80000000000000ULL), BB(0x000000ULL, 0x100000000000000ULL), BB(0x000000ULL, 0x200000000000000ULL), BB(0x000000ULL, 0x400000000000000ULL), BB(0x000000ULL, 0x800000000000000ULL), BB(0x0000000ULL, 0x1000000000000000ULL), BB(0x0000000ULL, 0x2000000000000000ULL), BB(0x0000000ULL, 0x4000000000000000ULL),
    BB(0x00001ULL, 0x00000000000000ULL), BB(0x00002ULL, 0x00000000000000ULL), BB(0x000004ULL, 0x000000000000000ULL), BB(0x000008ULL, 0x000000000000000ULL), BB(0x000010ULL, 0x000000000000000ULL), BB(0x000020ULL, 0x000000000000000ULL), BB(0x0000040ULL, 0x0000000000000000ULL), BB(0x0000080ULL, 0x0000000000000000ULL), BB(0x0000100ULL, 0x0000000000000000ULL),
    BB(0x00200ULL, 0x00000000000000ULL), BB(0x00400ULL, 0x00000000000000ULL), BB(0x000800ULL, 0x000000000000000ULL), BB(0x001000ULL, 0x000000000000000ULL), BB(0x002000ULL, 0x000000000000000ULL), BB(0x004000ULL, 0x000000000000000ULL), BB(0x0008000ULL, 0x0000000000000000ULL), BB(0x0010000ULL, 0x0000000000000000ULL), BB(0x0020000ULL, 0x0000000000000000ULL),
};

constexpr Bitboard FileBB[NumFiles] = {
    SquareBB[Sq1A] | SquareBB[Sq1B] | SquareBB[Sq1C] | SquareBB[Sq1D] | SquareBB[Sq1E] | SquareBB[Sq1F] | SquareBB[Sq1G] | SquareBB[Sq1H] | SquareBB[Sq1I],
    SquareBB[Sq2A] | SquareBB[Sq2B] | SquareBB[Sq2C] | SquareBB[Sq2D] | SquareBB[Sq2E] | SquareBB[Sq2F] | SquareBB[Sq2G] | SquareBB[Sq2H] | SquareBB[Sq2I],
    SquareBB[Sq3A] | SquareBB[Sq3B] | SquareBB[Sq3C] | SquareBB[Sq3D] | SquareBB[Sq3E] | SquareBB[Sq3F] | SquareBB[Sq3G] | SquareBB[Sq3H] | SquareBB[Sq3I],
    SquareBB[Sq4A] | SquareBB[Sq4B] | SquareBB[Sq4C] | SquareBB[Sq4D] | SquareBB[Sq4E] | SquareBB[Sq4F] | SquareBB[Sq4G] | SquareBB[Sq4H] | SquareBB[Sq4I],
    SquareBB[Sq5A] | SquareBB[Sq5B] | SquareBB[Sq5C] | SquareBB[Sq5D] | SquareBB[Sq5E] | SquareBB[Sq5F] | SquareBB[Sq5G] | SquareBB[Sq5H] | SquareBB[Sq5I],
    SquareBB[Sq6A] | SquareBB[Sq6B] | SquareBB[Sq6C] | SquareBB[Sq6D] | SquareBB[Sq6E] | SquareBB[Sq6F] | SquareBB[Sq6G] | SquareBB[Sq6H] | SquareBB[Sq6I],
    SquareBB[Sq7A] | SquareBB[Sq7B] | SquareBB[Sq7C] | SquareBB[Sq7D] | SquareBB[Sq7E] | SquareBB[Sq7F] | SquareBB[Sq7G] | SquareBB[Sq7H] | SquareBB[Sq7I],
    SquareBB[Sq8A] | SquareBB[Sq8B] | SquareBB[Sq8C] | SquareBB[Sq8D] | SquareBB[Sq8E] | SquareBB[Sq8F] | SquareBB[Sq8G] | SquareBB[Sq8H] | SquareBB[Sq8I],
    SquareBB[Sq9A] | SquareBB[Sq9B] | SquareBB[Sq9C] | SquareBB[Sq9D] | SquareBB[Sq9E] | SquareBB[Sq9F] | SquareBB[Sq9G] | SquareBB[Sq9H] | SquareBB[Sq9I],
};

constexpr Bitboard RankBB[NumRanks] = {
    SquareBB[Sq9I] | SquareBB[Sq8I] | SquareBB[Sq7I] | SquareBB[Sq6I] | SquareBB[Sq5I] | SquareBB[Sq4I] | SquareBB[Sq3I] | SquareBB[Sq2I] | SquareBB[Sq1I],
    SquareBB[Sq9H] | SquareBB[Sq8H] | SquareBB[Sq7H] | SquareBB[Sq6H] | SquareBB[Sq5H] | SquareBB[Sq4H] | SquareBB[Sq3H] | SquareBB[Sq2H] | SquareBB[Sq1H],
    SquareBB[Sq9G] | SquareBB[Sq8G] | SquareBB[Sq7G] | SquareBB[Sq6G] | SquareBB[Sq5G] | SquareBB[Sq4G] | SquareBB[Sq3G] | SquareBB[Sq2G] | SquareBB[Sq1G],
    SquareBB[Sq9F] | SquareBB[Sq8F] | SquareBB[Sq7F] | SquareBB[Sq6F] | SquareBB[Sq5F] | SquareBB[Sq4F] | SquareBB[Sq3F] | SquareBB[Sq2F] | SquareBB[Sq1F],
    SquareBB[Sq9E] | SquareBB[Sq8E] | SquareBB[Sq7E] | SquareBB[Sq6E] | SquareBB[Sq5E] | SquareBB[Sq4E] | SquareBB[Sq3E] | SquareBB[Sq2E] | SquareBB[Sq1E],
    SquareBB[Sq9D] | SquareBB[Sq8D] | SquareBB[Sq7D] | SquareBB[Sq6D] | SquareBB[Sq5D] | SquareBB[Sq4D] | SquareBB[Sq3D] | SquareBB[Sq2D] | SquareBB[Sq1D],
    SquareBB[Sq9C] | SquareBB[Sq8C] | SquareBB[Sq7C] | SquareBB[Sq6C] | SquareBB[Sq5C] | SquareBB[Sq4C] | SquareBB[Sq3C] | SquareBB[Sq2C] | SquareBB[Sq1C],
    SquareBB[Sq9B] | SquareBB[Sq8B] | SquareBB[Sq7B] | SquareBB[Sq6B] | SquareBB[Sq5B] | SquareBB[Sq4B] | SquareBB[Sq3B] | SquareBB[Sq2B] | SquareBB[Sq1B],
    SquareBB[Sq9A] | SquareBB[Sq8A] | SquareBB[Sq7A] | SquareBB[Sq6A] | SquareBB[Sq5A] | SquareBB[Sq4A] | SquareBB[Sq3A] | SquareBB[Sq2A] | SquareBB[Sq1A],
};

// constexpr Bitboard AllBB =
//     SquareBB[Sq9A] | SquareBB[Sq8A] | SquareBB[Sq7A] | SquareBB[Sq6A] | SquareBB[Sq5A] | SquareBB[Sq4A] | SquareBB[Sq3A] | SquareBB[Sq2A] | SquareBB[Sq1A] |
//     SquareBB[Sq9B] | SquareBB[Sq8B] | SquareBB[Sq7B] | SquareBB[Sq6B] | SquareBB[Sq5B] | SquareBB[Sq4B] | SquareBB[Sq3B] | SquareBB[Sq2B] | SquareBB[Sq1B] |
//     SquareBB[Sq9C] | SquareBB[Sq8C] | SquareBB[Sq7C] | SquareBB[Sq6C] | SquareBB[Sq5C] | SquareBB[Sq4C] | SquareBB[Sq3C] | SquareBB[Sq2C] | SquareBB[Sq1C] |
//     SquareBB[Sq9D] | SquareBB[Sq8D] | SquareBB[Sq7D] | SquareBB[Sq6D] | SquareBB[Sq5D] | SquareBB[Sq4D] | SquareBB[Sq3D] | SquareBB[Sq2D] | SquareBB[Sq1D] |
//     SquareBB[Sq9E] | SquareBB[Sq8E] | SquareBB[Sq7E] | SquareBB[Sq6E] | SquareBB[Sq5E] | SquareBB[Sq4E] | SquareBB[Sq3E] | SquareBB[Sq2E] | SquareBB[Sq1E] |
//     SquareBB[Sq9F] | SquareBB[Sq8F] | SquareBB[Sq7F] | SquareBB[Sq6F] | SquareBB[Sq5F] | SquareBB[Sq4F] | SquareBB[Sq3F] | SquareBB[Sq2F] | SquareBB[Sq1F] |
//     SquareBB[Sq9G] | SquareBB[Sq8G] | SquareBB[Sq7G] | SquareBB[Sq6G] | SquareBB[Sq5G] | SquareBB[Sq4G] | SquareBB[Sq3G] | SquareBB[Sq2G] | SquareBB[Sq1G] |
//     SquareBB[Sq9H] | SquareBB[Sq8H] | SquareBB[Sq7H] | SquareBB[Sq6H] | SquareBB[Sq5H] | SquareBB[Sq4H] | SquareBB[Sq3H] | SquareBB[Sq2H] | SquareBB[Sq1H] |
//     SquareBB[Sq9I] | SquareBB[Sq8I] | SquareBB[Sq7I] | SquareBB[Sq6I] | SquareBB[Sq5I] | SquareBB[Sq4I] | SquareBB[Sq3I] | SquareBB[Sq2I] | SquareBB[Sq1I];

Bitboard KnightAttackBB[NumColors][NumSquares];
Bitboard SilverAttackBB[NumColors][NumSquares];
Bitboard GoldAttackBB[NumColors][NumSquares];
Bitboard KingAttackBB[NumSquares];
Bitboard DiagStepAttackBB[NumSquares];
Bitboard CrossStepAttackBB[NumSquares];

Bitboard ForwardAttackBB[NumSquares][1 << 7];
Bitboard BackwardAttackBB[NumSquares][1 << 7];

MagicBitboard<BishopMagicBits> BishopMagicBB[NumSquares];
MagicBitboard<RookMagicBits> RookMagicBB[NumSquares];

constexpr Bitboard FirstAndSecondFurthestBB[NumColors] = {
    RankBB[RankA] | RankBB[RankB],
    RankBB[RankH] | RankBB[RankI],
};

constexpr Bitboard FurthermostBB[NumColors] = {
    RankBB[RankA] | Bitboard(0, 0),  // small hack for compiletime initialization of array.
    RankBB[RankI] | Bitboard(0, 0),
};

constexpr Bitboard SecondFurthestBB[NumColors] = {
    RankBB[RankB] | Bitboard(0, 0),  // small hack for compiletime initialization of array.
    RankBB[RankH] | Bitboard(0, 0),
};

constexpr Bitboard PromotableBB[NumColors] = {
    RankBB[RankA] | RankBB[RankB] | RankBB[RankC],
    RankBB[RankG] | RankBB[RankH] | RankBB[RankI],
};

// clang-format on

Bitboard LineBB[NumSquares][NumSquares];
Bitboard DirectionBB[11 + NorthNorthWest + 1][NumSquares];
Bitboard BetweenBB[NumSquares][NumSquares];
Bitboard DiagBB[NumSquares];
Bitboard CrossBB[NumSquares];
Bitboard ForwardBB[NumSquares];
Bitboard BackwardBB[NumSquares];

namespace {

Bitboard getBlackKnightAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if (!FileBB[File9].isSet(Sq) &&
        !FirstAndSecondFurthestBB[Black].isSet(Sq)) {
        BB.toggleBit(Sq + NorthNorthWest);
    }

    if (!FileBB[File1].isSet(Sq) &&
        !FirstAndSecondFurthestBB[Black].isSet(Sq)) {
        BB.toggleBit(Sq + NorthNorthEast);
    }

    return BB;
}

Bitboard getWhiteKnightAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if (!FileBB[File9].isSet(Sq) &&
        !FirstAndSecondFurthestBB[White].isSet(Sq)) {
        BB.toggleBit(Sq + SouthSouthWest);
    }

    if (!FileBB[File1].isSet(Sq) &&
        !FirstAndSecondFurthestBB[White].isSet(Sq)) {
        BB.toggleBit(Sq + SouthSouthEast);
    }

    return BB;
}

Bitboard getBlackSilverAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + North);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthEast);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthWest);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthEast);
    }

    return BB;
}

Bitboard getWhiteSilverAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + South);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthEast);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthWest);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthEast);
    }

    return BB;
}

Bitboard getBlackGoldAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + North);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthEast);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero()) {
        BB.toggleBit(Sq + West);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero()) {
        BB.toggleBit(Sq + East);
    }

    if ((SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + South);
    }

    return BB;
}

Bitboard getWhiteGoldAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + South);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthEast);
    }

    if ((SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + North);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero()) {
        BB.toggleBit(Sq + West);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero()) {
        BB.toggleBit(Sq + East);
    }

    return BB;
}

Bitboard getKingAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + North);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthEast);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero()) {
        BB.toggleBit(Sq + West);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero()) {
        BB.toggleBit(Sq + East);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthWest);
    }

    if ((SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + South);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthEast);
    }

    return BB;
}

Bitboard getDiagStepAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthWest);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + NorthEast);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthWest);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero() &&
        (SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + SouthEast);
    }

    return BB;
}

Bitboard getCrossStepAttack(Square Sq) {
    Bitboard BB = Bitboard::ZeroBB();

    if ((SquareBB[Sq] & RankBB[RankA]).isZero()) {
        BB.toggleBit(Sq + North);
    }

    if ((SquareBB[Sq] & FileBB[File9]).isZero()) {
        BB.toggleBit(Sq + West);
    }

    if ((SquareBB[Sq] & FileBB[File1]).isZero()) {
        BB.toggleBit(Sq + East);
    }

    if ((SquareBB[Sq] & RankBB[RankI]).isZero()) {
        BB.toggleBit(Sq + South);
    }

    return BB;
}

void initializeKnightAttacks() {
    for (Square sq : Squares) {
        KnightAttackBB[Black][sq] = getBlackKnightAttack(sq);
        KnightAttackBB[White][sq] = getWhiteKnightAttack(sq);
    }
}

void initializeSilverAttacks() {
    for (Square sq : Squares) {
        SilverAttackBB[Black][sq] = getBlackSilverAttack(sq);
        SilverAttackBB[White][sq] = getWhiteSilverAttack(sq);
    }
}

void initializeGoldAttacks() {
    for (Square sq : Squares) {
        GoldAttackBB[Black][sq] = getBlackGoldAttack(sq);
        GoldAttackBB[White][sq] = getWhiteGoldAttack(sq);
    }
}

void initializeKingAttacks() {
    for (Square sq : Squares) {
        KingAttackBB[sq] = getKingAttack(sq);
    }
}

void initializeDiagStepAttacks() {
    for (Square sq : Squares) {
        DiagStepAttackBB[sq] = getDiagStepAttack(sq);
    }
}

void initializeCrossStepAttacks() {
    for (Square sq : Squares) {
        CrossStepAttackBB[sq] = getCrossStepAttack(sq);
    }
}

/// Forward means the RankI --> RankA direction.
void computeForwardAttacks() {
    std::memset(static_cast<void*>(ForwardAttackBB), 0,
                NumSquares * (1 << 7) * sizeof(Bitboard));

    for (Square Sq : Squares) {
        if (squareToRank(Sq) == RankA) {
            continue;
        }

        const File F = squareToFile(Sq);

        // clang-format off
        // This bit pattern represents for (RankH, RankG, ..., RankB) occupations.
        //                   (RankH) lower <--                    --> higher
        //                   (RankB)
        // clang-format on
        for (uint8_t Pattern = 0; Pattern < (1 << 7); ++Pattern) {
            // Going north.
            for (Rank R = (Rank)(squareToRank(Sq) + 1);; R = (Rank)(R + 1)) {
                ForwardAttackBB[Sq][Pattern] |=
                    SquareBB[makeSquare(R, F)];

                if (R == RankA) {
                    break;
                }

                if ((Pattern & (1 << (R - 1))) != 0) {
                    break;
                }

                if (R == RankB) {
                    ForwardAttackBB[Sq][Pattern] |=
                        SquareBB[makeSquare(RankA, F)];
                }
            }
        }
    }
}

/// Backward means the RankA --> RankI direction.
void computeBackwardAttacks() {
    std::memset(static_cast<void*>(BackwardAttackBB), 0,
                NumSquares * (1 << 7) * sizeof(Bitboard));

    for (Square Sq : Squares) {
        if (squareToRank(Sq) == RankI) {
            continue;
        }

        const File F = squareToFile(Sq);

        // This bit pattern represents for (RankH, RankG, ..., RankB)
        // occupations.
        for (uint8_t Pattern = 0; Pattern < (1 << 7); ++Pattern) {
            // Going north.
            for (Rank R = (Rank)(squareToRank(Sq) - 1);; R = (Rank)(R - 1)) {
                BackwardAttackBB[Sq][Pattern] |=
                    SquareBB[makeSquare(R, F)];

                if (R == RankI) {
                    break;
                }

                if ((Pattern & (1 << (R - 1))) != 0) {
                    break;
                }

                if (R == RankH) {
                    BackwardAttackBB[Sq][Pattern] |=
                        SquareBB[makeSquare(RankI, F)];
                }
            }
        }
    }
}

template <Direction Dir> constexpr bool isBorder(Square Sq) {
    const Rank R = squareToRank(Sq);
    const File F = squareToFile(Sq);
    return ((Dir == North || Dir == NorthEast || Dir == NorthWest) &&
            R == RankA) ||
           ((Dir == NorthEast || Dir == East || Dir == SouthEast) &&
            F == File1) ||
           ((Dir == South || Dir == SouthEast || Dir == SouthWest) &&
            R == RankI) ||
           ((Dir == West || Dir == NorthWest || Dir == SouthWest) &&
            F == File9);
}

template <Direction Dir, bool IsRoot>
std::tuple<Bitboard, Bitboard, uint8_t>
getPatternBB(Square Sq, uint16_t Pattern, bool Blocked, uint8_t BitIndex) {
    if (IsRoot && isBorder<Dir>(Sq)) {
        return {{0, 0}, {0, 0}, BitIndex};
    }

    const Square NextSq = Sq + Dir;

    Bitboard AttackBB(0, 0);
    if (!Blocked) {
        AttackBB |= SquareBB[NextSq];
    }

    if (isBorder<Dir>(NextSq)) {
        return {AttackBB, {0, 0}, BitIndex};
    }

    Bitboard ExistBB(0, 0);
    if (Pattern & (1 << BitIndex)) {
        ExistBB |= SquareBB[NextSq];
        Blocked = true;
    }

    const auto& [NAttackBB, NExistBB, NBitIndex] =
        getPatternBB<Dir, false>(NextSq, Pattern, Blocked, BitIndex + 1);
    return {AttackBB | NAttackBB, ExistBB | NExistBB, NBitIndex};
}

std::pair<Bitboard, Bitboard> diagPatternBB(Square Sq, const uint16_t Pattern) {
    const auto& [A1, E1, B1] =
        getPatternBB<NorthEast, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] =
        getPatternBB<SouthEast, true>(Sq, Pattern, false, B1);
    const auto& [A3, E3, B3] =
        getPatternBB<SouthWest, true>(Sq, Pattern, false, B2);
    const auto& [A4, E4, B4] =
        getPatternBB<NorthWest, true>(Sq, Pattern, false, B3);

    assert(B4 <= 12);

    const Bitboard AttackBB = (A1 | A2 | A3 | A4) & ~SquareBB[Sq];

    const Bitboard ExistBB = (E1 | E2 | E3 | E4) & ~FileBB[File9] &
                             ~FileBB[File1] & ~RankBB[RankI] & ~RankBB[RankA] &
                             ~SquareBB[Sq];

    return {AttackBB, ExistBB};
}

std::pair<Bitboard, Bitboard> crossPatternBB(Square Sq,
                                             const uint16_t Pattern) {
    const auto& [A1, E1, B1] = getPatternBB<North, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] = getPatternBB<East, true>(Sq, Pattern, false, B1);
    const auto& [A3, E3, B3] =
        getPatternBB<South, true>(Sq, Pattern, false, B2);
    const auto& [A4, E4, B4] = getPatternBB<West, true>(Sq, Pattern, false, B3);

    assert(B4 <= 14);

    const Bitboard AttackBB = (A1 | A2 | A3 | A4) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2 | E3 | E4) & ~SquareBB[Sq];

    return {AttackBB, ExistBB};
}

Bitboard diagPatternMask(Square Sq) {
    return diagPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard crossPatternMask(Square Sq) {
    return crossPatternBB(Sq, (1 << 15) - 1).second;
}

void setBishopMagicMasks() {
    for (Square Sq : Squares) {
        BishopMagicBB[Sq].Mask = diagPatternMask(Sq);
    }
}

std::pair<bool, std::vector<Bitboard>>
isBishopMagicNumberOK(Square Sq, uint64_t MagicNumber) {
    std::vector<Bitboard> AttackBBs(1 << BishopMagicBits, {0, 0});

    // clang-format off
    const uint16_t NumBits[NumSquares] = {
        7, 6, 6,  6,  6,  6, 6, 6, 7,
        6, 6, 6,  6,  6,  6, 6, 6, 6,
        6, 6, 8,  8,  8,  8, 8, 6, 6,
        6, 6, 8, 10, 10, 10, 8, 6, 6,
        6, 6, 8, 10, 12, 10, 8, 6, 6,
        6, 6, 8, 10, 10, 10, 8, 6, 6,
        6, 6, 8,  8,  8,  8, 8, 6, 6,
        6, 6, 6,  6,  6,  6, 6, 6, 6,
        7, 6, 6,  6,  6,  6, 6, 6, 7,
    };
    // clang-format on

    const uint64_t ShiftAmount = 64 - BishopMagicBits;
    const uint16_t NumBit = NumBits[Sq];

    for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
        const auto& [AttackBB, BB] = diagPatternBB(Sq, Pattern);

        const uint16_t Result =
            (uint16_t)((BB.horizontalOr() * MagicNumber) >> ShiftAmount);

        if (AttackBBs[Result].isZero()) {
            AttackBBs[Result].copyFrom(AttackBB);
        } else if (AttackBBs[Result] != AttackBB) {
            return std::make_pair(false, AttackBBs);
        }
    }

    return std::make_pair(true, AttackBBs);
}

uint64_t generateMagicNumberCandidate() {
    static std::mt19937_64 Mt((uint64_t)'N' | (uint64_t)'o' << 8 |
                              (uint64_t)'v' << 16 | (uint64_t)'i' << 24 |
                              (uint64_t)'c' << 32 | (uint64_t)'e' << 40);

    return Mt() & Mt() & Mt();
}

void setBishopMagicNumbers() {
    constexpr uint64_t BishopMagicNumbers[NumSquares] = {
        0x0008000701008c81ULL, 0x5002852104003202ULL, 0x0000884000800804ULL,
        0x0c00190800824000ULL, 0x0440880501880000ULL, 0x00000a01001c0001ULL,
        0x1000d30920082000ULL, 0x00740420010000c1ULL, 0x4011050811020060ULL,
        0x40440200020a0030ULL, 0x4204001280c8a024ULL, 0x0048014110280004ULL,
        0x00108241002000a0ULL, 0xa22841024119a060ULL, 0x90042000d0011a00ULL,
        0x962208020a68e0c2ULL, 0x020a0001811181c0ULL, 0x00480000302a0108ULL,
        0x0020a00504200010ULL, 0x0080080080310203ULL, 0x0740400900001828ULL,
        0xa004002818132042ULL, 0x0006060000419080ULL, 0x8201009c89880410ULL,
        0x00101286009480a0ULL, 0x401000406a400808ULL, 0x0290020500080820ULL,
        0x0132180260400801ULL, 0x4608000941000801ULL, 0x08d0c28200010008ULL,
        0x0000c20080620004ULL, 0x0880021000404204ULL, 0x0101640181480010ULL,
        0x2092088008081044ULL, 0x0104484008088008ULL, 0x0018002402804008ULL,
        0x8080090023099006ULL, 0x6040058600801a04ULL, 0x8011020040040442ULL,
        0x911c00820020a041ULL, 0x4480800820003004ULL, 0x8400401401800012ULL,
        0x01a2001000204204ULL, 0x0008226010201008ULL, 0x0201480800822001ULL,
        0x884402064082c808ULL, 0x0801014081450041ULL, 0x0090429082002040ULL,
        0x0020802081800970ULL, 0x000c001200080409ULL, 0x0418100800800402ULL,
        0x00c0900100040042ULL, 0x0040140040001826ULL, 0x808060050100805cULL,
        0x0240124002800400ULL, 0x1050904404011080ULL, 0x0009020400254020ULL,
        0x0202408040010220ULL, 0x050c0002008c8400ULL, 0x001003085000a212ULL,
        0x3010102a28200c10ULL, 0x1080c10940050081ULL, 0x0482100040000800ULL,
        0x0200240a00584003ULL, 0x2000851000801020ULL, 0x0400090200a0440aULL,
        0x4004000004114020ULL, 0x0800002000024014ULL, 0x28002204a8200201ULL,
        0x000c00c1c0004001ULL, 0x3088042002014808ULL, 0x400018440811004dULL,
        0x9a00c23000c00020ULL, 0x240050204400e021ULL, 0x40c8082030404412ULL,
        0x400918010001010cULL, 0x2080120100142093ULL, 0x03a4506000480494ULL,
        0x1001002800224001ULL, 0x0004228041800410ULL, 0x2000890024080108ULL,
    };

    for (Square Sq : Squares) {
        BishopMagicBB[Sq].MagicNumber = BishopMagicNumbers[Sq];
    }
};

void setRookMagicNumbers() {
    constexpr uint64_t RookMagicNumbers[NumSquares] = {
        0x02c0000c91064100ULL, 0x0021000400080801ULL, 0x8808000200424208ULL,
        0xa001010010000801ULL, 0x2e06020000120010ULL, 0x3840148001004010ULL,
        0x0008000801040408ULL, 0x0004000488501046ULL, 0x8040003888088282ULL,
        0x4000340080800140ULL, 0x84301900a2000020ULL, 0x4001000c14099002ULL,
        0x0400680048450018ULL, 0x0056088080800008ULL, 0x0854200090a00110ULL,
        0x4109a00004200081ULL, 0x3080560006081504ULL, 0x0011100028011041ULL,
        0x2105050200080481ULL, 0x308080120040a500ULL, 0x0520101880400042ULL,
        0x0040010c00100018ULL, 0x0200008100002401ULL, 0x00d0204300105090ULL,
        0x2000210800a0080aULL, 0x00080c2800104001ULL, 0x8340001200840082ULL,
        0x4514008010400a00ULL, 0x400c1600a4000422ULL, 0x00608001a4002008ULL,
        0x08890a0001002104ULL, 0x0043002000400820ULL, 0xc004882028002088ULL,
        0xc000904208000209ULL, 0x0081008a020008c1ULL, 0x0882801040401086ULL,
        0x0048001000288020ULL, 0x1208408000412080ULL, 0x102008220d208002ULL,
        0x2101000a12000410ULL, 0x0006040120028002ULL, 0x0402004100880001ULL,
        0x8804408000820482ULL, 0x02004400c0042004ULL, 0x0201020001100302ULL,
        0x05c0011120008140ULL, 0x1280080119408880ULL, 0x0002401080010074ULL,
        0x2149404080038140ULL, 0x00800210024a0020ULL, 0x0800842040001004ULL,
        0x0010800825044008ULL, 0x0005400104008404ULL, 0x0001002002008022ULL,
        0x00c0400802100444ULL, 0x0131440804002221ULL, 0x020c08101000800dULL,
        0x02200400a0009424ULL, 0x8040002020204201ULL, 0x0044200063c00009ULL,
        0x0200008004140001ULL, 0x211888000c100a03ULL, 0x6401001104430404ULL,
        0x0082000108107002ULL, 0x9008001080120060ULL, 0x100061184001c340ULL,
        0x2104000084400802ULL, 0x0060110000600020ULL, 0x0a080e0202204010ULL,
        0x4840024000039001ULL, 0x0130044850008003ULL, 0x0040022002221041ULL,
        0x2011200049004300ULL, 0x040c080010680040ULL, 0x0001120003000080ULL,
        0x040a040002100009ULL, 0x0002010028084408ULL, 0x0000400008800810ULL,
        0x0280200030500088ULL, 0x0050880050a20041ULL, 0x90082000040088aaULL,
    };

    for (Square Sq : Squares) {
        RookMagicBB[Sq].MagicNumber = RookMagicNumbers[Sq];
    }
}

void computeBishopMagicBitboard() {
    bool IsPrecomputed = BishopMagicBB[0].MagicNumber != 0;

    for (Square Sq : Squares) {
        if (!IsPrecomputed) {
            printf("Searching for a bishop magic number for square %2d ... ",
                   Sq);
            std::cout << std::flush;
        }

        for (uint64_t Trial = 0;; ++Trial) {
            const uint64_t MagicNumberCandidate =
                (BishopMagicBB[Sq].MagicNumber != 0)
                    ? BishopMagicBB[Sq].MagicNumber
                    : generateMagicNumberCandidate();

            const auto& Result =
                isBishopMagicNumberOK(Sq, MagicNumberCandidate);
            if (Result.first) {
                BishopMagicBB[Sq].MagicNumber = MagicNumberCandidate;
                std::memcpy(static_cast<void*>(BishopMagicBB[Sq].AttackBB),
                            static_cast<const void*>(Result.second.data()),
                            (1 << BishopMagicBits) * sizeof(Bitboard));

                if (!IsPrecomputed) {
                    printf("OK (found 0x%016lx with %7lu trials).",
                           MagicNumberCandidate, Trial);
                    std::cout << std::endl;
                }
                break;
            }
        }
    }

    if (!IsPrecomputed) {
        printf("constexpr uint64_t BishopMagicNumbers[NumSquares] = {\n");
        for (Square Sq : Squares) {
            printf("0x%016lxULL, ", BishopMagicBB[Sq].MagicNumber);

            if (squareToRank(Sq) == RankA) {
                printf("\n");
            }
        }
        printf("};\n");
    }
}

void setRookMagicMasks() {
    for (Square Sq : Squares) {
        RookMagicBB[Sq].Mask = crossPatternMask(Sq);
    }
}

std::pair<bool, std::vector<Bitboard>>
isRookMagicNumberOK(Square Sq, uint64_t MagicNumber) {
    std::vector<Bitboard> AttackBBs(1 << RookMagicBits, {0, 0});

    // clang-format off
    const uint16_t NumBits[NumSquares] = {
        14, 13, 13, 13, 13, 13, 13, 13, 14,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 12, 12, 12, 12, 12, 12, 12, 13,
        14, 13, 13, 13, 13, 13, 13, 13, 14,
    };
    // clang-format on

    const uint64_t ShiftAmount = 64 - RookMagicBits;
    const uint16_t NumBit = NumBits[Sq];

    for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
        const auto& [AttackBB, BB] = crossPatternBB(Sq, Pattern);
        const uint16_t Result =
            (uint16_t)((BB.horizontalOr() * MagicNumber) >> ShiftAmount);

        if (AttackBBs[Result].isZero()) {
            AttackBBs[Result].copyFrom(AttackBB);
        } else if (AttackBBs[Result] != AttackBB) {
            return std::make_pair(false, AttackBBs);
        }
    }

    return std::make_pair(true, AttackBBs);
}

void computeRookMagicBitboard() {
    bool IsPrecomputed = RookMagicBB[0].MagicNumber != 0;

    for (Square Sq : Squares) {
        if (!IsPrecomputed) {
            printf("Searching for a rook magic number for square %2d ... ", Sq);
            std::cout << std::flush;
        }

        for (uint64_t Trial = 0;; ++Trial) {
            const uint64_t MagicNumberCandidate =
                (IsPrecomputed) ? RookMagicBB[Sq].MagicNumber
                                : generateMagicNumberCandidate();

            const auto& Result = isRookMagicNumberOK(Sq, MagicNumberCandidate);
            if (Result.first) {
                RookMagicBB[Sq].MagicNumber = MagicNumberCandidate;
                std::memcpy(static_cast<void*>(RookMagicBB[Sq].AttackBB),
                            static_cast<const void*>(Result.second.data()),
                            (1 << RookMagicBits) * sizeof(Bitboard));

                if (!IsPrecomputed) {
                    printf("OK (found 0x%016lx with %7lu trials).",
                           MagicNumberCandidate, Trial);
                    std::cout << std::endl;
                }
                break;
            }
        }
    }

    if (!IsPrecomputed) {
        printf("constexpr uint64_t RookMagicNumbers[NumSquares] = {\n");
        for (Square Sq : Squares) {
            printf("0x%016lxULL, ", RookMagicBB[Sq].MagicNumber);

            if (squareToRank(Sq) == RankA) {
                printf("\n");
            }
        }
        printf("};\n");
    }
}

void initializeLineBB() {
    std::memset(static_cast<void*>(LineBB), 0, sizeof(LineBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            if ((std::abs(squareToFile(Sq1) - squareToFile(Sq2))) ==
                std::abs(squareToRank(Sq1) - squareToRank(Sq2))) {
                LineBB[Sq1][Sq2].copyFrom(
                    (SquareBB[Sq1] |
                     getBishopAttackBB<PTK_Bishop>(Sq1, Bitboard::ZeroBB())) &
                    (SquareBB[Sq2] |
                     getBishopAttackBB<PTK_Bishop>(Sq2, Bitboard::ZeroBB())));
            } else if (squareToFile(Sq1) == squareToFile(Sq2)) {
                LineBB[Sq1][Sq2].copyFrom(FileBB[squareToFile(Sq1)]);
            } else if (squareToRank(Sq1) == squareToRank(Sq2)) {
                LineBB[Sq1][Sq2].copyFrom(RankBB[squareToRank(Sq1)]);
            }
        }
    }
}

void initializeDirectionBB() {
    std::memset(static_cast<void*>(DirectionBB), 0, sizeof(DirectionBB));

    for (Square Sq : Squares) {
        DirectionBB[11 + North][Sq] = std::get<1>(
            getPatternBB<North, true>(Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + NorthEast][Sq] =
            std::get<1>(getPatternBB<NorthEast, true>(
                Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + East][Sq] = std::get<1>(
            getPatternBB<East, true>(Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + SouthEast][Sq] =
            std::get<1>(getPatternBB<SouthEast, true>(
                Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + South][Sq] = std::get<1>(
            getPatternBB<South, true>(Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + SouthWest][Sq] =
            std::get<1>(getPatternBB<SouthWest, true>(
                Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + West][Sq] = std::get<1>(
            getPatternBB<West, true>(Sq, (1 << RookMagicBits) - 1, false, 0));
        DirectionBB[11 + NorthWest][Sq] =
            std::get<1>(getPatternBB<NorthWest, true>(
                Sq, (1 << RookMagicBits) - 1, false, 0));
    }
}

void initializeBetweenBB() {
    std::memset(static_cast<void*>(BetweenBB), 0, sizeof(BetweenBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            BetweenBB[Sq1][Sq2] |=
                DirectionBB[11 + North][Sq1] & DirectionBB[11 + South][Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + NorthEast][Sq1] &
                                   DirectionBB[11 + SouthWest][Sq2];
            BetweenBB[Sq1][Sq2] |=
                DirectionBB[11 + East][Sq1] & DirectionBB[11 + West][Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + SouthEast][Sq1] &
                                   DirectionBB[11 + NorthWest][Sq2];
            BetweenBB[Sq1][Sq2] |=
                DirectionBB[11 + South][Sq1] & DirectionBB[11 + North][Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + SouthWest][Sq1] &
                                   DirectionBB[11 + NorthEast][Sq2];
            BetweenBB[Sq1][Sq2] |=
                DirectionBB[11 + West][Sq1] & DirectionBB[11 + East][Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + NorthWest][Sq1] &
                                   DirectionBB[11 + SouthEast][Sq2];
        }
    }
}

void initializeDiagBB() {
    std::memset(static_cast<void*>(DiagBB), 0, sizeof(DiagBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            const auto Direction = SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == NorthWest || Direction == NorthEast
                || Direction == SouthWest || Direction == SouthEast) {
                DiagBB[Sq1].toggleBit(Sq2);
            }
        }
    }
}

void initializeCrossBB() {
    std::memset(static_cast<void*>(CrossBB), 0, sizeof(CrossBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            const auto Direction = SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == North || Direction == East
                || Direction == South || Direction == West) {
                CrossBB[Sq1].toggleBit(Sq2);
            }
        }
    }
}

void initializeForwardBB() {
    std::memset(static_cast<void*>(ForwardBB), 0, sizeof(ForwardBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            const auto Direction = SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == North) {
                ForwardBB[Sq1].toggleBit(Sq2);
            }
        }
    }
}

void initializeBackwardBB() {
    std::memset(static_cast<void*>(BackwardBB), 0, sizeof(BackwardBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            const auto Direction = SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == South) {
                BackwardBB[Sq1].toggleBit(Sq2);
            }
        }
    }
}


} // namespace

void initializeBitboards() {
    initializeKnightAttacks();
    initializeSilverAttacks();
    initializeGoldAttacks();
    initializeKingAttacks();
    initializeDiagStepAttacks();
    initializeCrossStepAttacks();

    std::memset(static_cast<void*>(BishopMagicBB), 0, sizeof(BishopMagicBB));
    setBishopMagicMasks();
    setBishopMagicNumbers();
    computeBishopMagicBitboard();

    std::memset(static_cast<void*>(RookMagicBB), 0, sizeof(RookMagicBB));
    setRookMagicMasks();
    setRookMagicNumbers();
    computeRookMagicBitboard();

    computeForwardAttacks();
    computeBackwardAttacks();

    initializeLineBB();
    initializeDirectionBB();
    initializeBetweenBB();
    initializeDiagBB();
    initializeCrossBB();
    initializeForwardBB();
    initializeBackwardBB();
}

} // namespace bitboard
} // namespace core
} // namespace nshogi
