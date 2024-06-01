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

MagicBitboard2<DiagMagicBits> BishopMagicBB[NumSquares];
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

std::pair<Bitboard, Bitboard> diagSWNEPatternBB(Square Sq, const uint16_t Pattern) {
    const auto& [A1, E1, B1] =
        getPatternBB<NorthEast, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] =
        getPatternBB<SouthWest, true>(Sq, Pattern, false, B1);

    assert(B2 <= 12);
    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~FileBB[File9] &
                             ~FileBB[File1] & ~RankBB[RankI] & ~RankBB[RankA] &
                             ~SquareBB[Sq];

    return {AttackBB, ExistBB};
}

std::pair<Bitboard, Bitboard> diagNWSEPatternBB(Square Sq, const uint16_t Pattern) {
    const auto& [A1, E1, B1] =
        getPatternBB<SouthEast, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] =
        getPatternBB<NorthWest, true>(Sq, Pattern, false, B1);

    assert(B2 <= 12);
    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~FileBB[File9] &
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

Bitboard diagSWNEPatternMask(Square Sq) {
    return diagSWNEPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard diagNWSEPatternMask(Square Sq) {
    return diagNWSEPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard crossPatternMask(Square Sq) {
    return crossPatternBB(Sq, (1 << 15) - 1).second;
}

void setBishopMagicMasks() {
    for (Square Sq : Squares) {
        BishopMagicBB[Sq].Mask1 = diagSWNEPatternMask(Sq);
        BishopMagicBB[Sq].Mask2 = diagNWSEPatternMask(Sq);
    }
}

std::tuple<bool, std::vector<Bitboard>, std::vector<Bitboard>>
isBishopMagicNumberOK(Square Sq, uint64_t MagicNumber) {
    std::vector<Bitboard> AttackBBs1(1 << DiagMagicBits, {0, 0});
    std::vector<Bitboard> AttackBBs2(1 << DiagMagicBits, {0, 0});

    // clang-format off
    static const uint16_t NumBitsSWNE[NumSquares] = {
        0, 0, 1, 2, 3, 4, 5, 6, 7,
        0, 0, 1, 2, 3, 4, 5, 6, 6,
        1, 1, 2, 3, 4, 5, 6, 5, 5,
        2, 2, 3, 4, 5, 6, 5, 4, 4,
        3, 3, 4, 5, 6, 5, 4, 3, 3,
        4, 4, 5, 6, 5, 4, 3, 2, 2,
        5, 5, 6, 5, 4, 3, 2, 2, 1,
        6, 6, 5, 4, 3, 2, 1, 0, 0,
        7, 6, 5, 4, 3, 2, 1, 0, 0,
    };
    static const uint16_t NumBitsNWSE[NumSquares] = {
        7, 6, 5, 4, 3, 2, 1, 0, 0,
        6, 6, 5, 4, 3, 2, 1, 0, 0,
        5, 5, 6, 5, 4, 3, 2, 2, 1,
        4, 4, 5, 6, 5, 4, 3, 2, 2,
        3, 3, 4, 5, 6, 5, 4, 3, 3,
        2, 2, 3, 4, 5, 6, 5, 4, 4,
        1, 1, 2, 3, 4, 5, 6, 5, 5,
        0, 0, 1, 2, 3, 4, 5, 6, 6,
        0, 0, 1, 2, 3, 4, 5, 6, 7,
    };
    // clang-format on

    const uint64_t ShiftAmount = 64 - DiagMagicBits;
    {
        const uint16_t NumBit = NumBitsSWNE[Sq];
        for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
            const auto& [AttackBB, BB] = diagSWNEPatternBB(Sq, Pattern);

            const uint16_t Result =
                (uint16_t)((BB.horizontalOr() * MagicNumber) >> ShiftAmount);

            if (AttackBBs1[Result].isZero()) {
                AttackBBs1[Result].copyFrom(AttackBB);
            } else if (AttackBBs1[Result] != AttackBB) {
                return std::make_tuple(false, AttackBBs1, AttackBBs2);
            }
        }
    }
    {
        const uint16_t NumBit = NumBitsNWSE[Sq];
        for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
            const auto& [AttackBB, BB] = diagNWSEPatternBB(Sq, Pattern);

            const uint16_t Result =
                (uint16_t)((BB.horizontalOr() * MagicNumber) >> ShiftAmount);

            if (AttackBBs2[Result].isZero()) {
                AttackBBs2[Result].copyFrom(AttackBB);
            } else if (AttackBBs2[Result] != AttackBB) {
                return std::make_tuple(false, AttackBBs1, AttackBBs2);
            }
        }
    }

    return std::make_tuple(true, AttackBBs1, AttackBBs2);
}

uint64_t generateMagicNumberCandidate() {
    // clang-format off
    static std::mt19937_64 Mt(
              (uint64_t)'n'
            | (uint64_t)'s' << 8
            | (uint64_t)'h' << 16
            | (uint64_t)'o' << 24
            | (uint64_t)'g' << 32
            | (uint64_t)'i' << 40);
    // clang-format on

    return Mt() & Mt() & Mt();
}

void setBishopMagicNumbers() {
    // clang-format off
    constexpr uint64_t BishopMagicNumbers[NumSquares] = {
        0x0048000200020004ULL, 0x001000200880a002ULL, 0x0010905093002882ULL, 0x00010040412a0010ULL, 0x1008210090200002ULL, 0x404080010500a000ULL, 0x0c0442100208040cULL, 0x0800002040010008ULL, 0xc095001003810640ULL,
        0x1840880900004038ULL, 0x00080130c00c4081ULL, 0x00c014006000a002ULL, 0x0808020644001011ULL, 0x0208280432020108ULL, 0x2020014000900200ULL, 0x2070050220030004ULL, 0x20c8090090002080ULL, 0x022280190048204cULL,
        0x0201300089080020ULL, 0x1080e2518080080bULL, 0x000984a400420008ULL, 0x000a200010400081ULL, 0xb020400089620020ULL, 0x2064002000841d80ULL, 0x8100400005005020ULL, 0x029000a00a000820ULL, 0xc022202000100201ULL,
        0x468122431040c081ULL, 0x200d1001428040a0ULL, 0x8014054201090102ULL, 0x0282104002081082ULL, 0x0500094000012002ULL, 0x8009424402040080ULL, 0x0484028022001015ULL, 0x0005082044043001ULL, 0x41005202800a2204ULL,
        0x8044803220110004ULL, 0x280002400088140cULL, 0x00a00201100a4020ULL, 0x0003008410400821ULL, 0x4a00408000421002ULL, 0x0040802100a00141ULL, 0x000a008294510842ULL, 0x3420200110004008ULL, 0x0118010018000b82ULL,
        0x000c20200304c842ULL, 0x0100005100080282ULL, 0x1102230410800442ULL, 0x1000800280400021ULL, 0x0100049010008042ULL, 0x0010e52000400a08ULL, 0x47131410600208c1ULL, 0x00014208010080c1ULL, 0x24409000a0022034ULL,
        0x010000c062011880ULL, 0x0020220b00c01008ULL, 0x0040168828040040ULL, 0x0100240060070008ULL, 0x0401880060040080ULL, 0x2480408008010048ULL, 0x4010000240100349ULL, 0x001a44002000a142ULL, 0x0094000080040114ULL,
        0x4809884100c00414ULL, 0x0440002320820306ULL, 0x00148440f0200040ULL, 0x104e000900200410ULL, 0x0020c40042820041ULL, 0x0300010802020018ULL, 0x9004082005204202ULL, 0x0010041041068023ULL, 0x108005191c30360eULL,
        0x0400048002002010ULL, 0x8a00044009500018ULL, 0x044000002430d420ULL, 0x0080040000012124ULL, 0x0040001000884007ULL, 0x0080052022104102ULL, 0xc052480080800010ULL, 0xc110706518204004ULL, 0x0098004804c07008ULL,
    };
    // clang-format on

    for (Square Sq : Squares) {
        // BishopMagicBB[Sq].MagicNumber = BishopMagicNumbers[Sq];
    }
};

void setRookMagicNumbers() {
    // clang-format off
    constexpr uint64_t RookMagicNumbers[NumSquares] = {
        0x810002200a0210c1ULL, 0x0081000800008480ULL, 0x88004008c002408cULL, 0x0108004100020208ULL, 0x0a10008202011404ULL, 0x00100010a0084086ULL, 0x0020009020380184ULL, 0x0802020002040002ULL, 0x44400008c10b2202ULL,
        0x8012a00000848105ULL, 0x0090080000818108ULL, 0xa200020020801040ULL, 0x1500080080202014ULL, 0x30c80500001ac508ULL, 0xe000420088804008ULL, 0x4202840200040014ULL, 0x1040600880d00012ULL, 0x0001200025020082ULL,
        0x08c0800300820080ULL, 0x04842010004a00a1ULL, 0x681010020000e08aULL, 0x8901800200120040ULL, 0x208002200018a020ULL, 0x1200001080210010ULL, 0x0000804800200e52ULL, 0x8001028040010201ULL, 0x0408008800010841ULL,
        0x0030028102000040ULL, 0x0100100060080040ULL, 0x0a00020006002082ULL, 0x0908001000a00090ULL, 0x0040002000802c10ULL, 0x1c41200404800810ULL, 0x4002001081000108ULL, 0x0008304604000421ULL, 0x020040420c000181ULL,
        0x0403000101000840ULL, 0x100010002010c880ULL, 0x07422c0001201010ULL, 0x4880048004009008ULL, 0x0040510001414020ULL, 0x2000203040040010ULL, 0x0801000800809011ULL, 0x1140008002020402ULL, 0x0400208811010002ULL,
        0x8480104010040090ULL, 0x008302000414c300ULL, 0x5020048002810088ULL, 0x0002400040000840ULL, 0x2026001800082020ULL, 0x010a80a100800810ULL, 0x0444204a04010008ULL, 0x0644c08010048102ULL, 0x0a4000820a094042ULL,
        0x1540000808880042ULL, 0x000238101c004142ULL, 0x4010040000264201ULL, 0x002110c004004084ULL, 0x824280a000808012ULL, 0x30062100002019d2ULL, 0x0403000060040302ULL, 0x0000112082016408ULL, 0x4009011808908084ULL,
        0x0508008020118020ULL, 0x0614020808004210ULL, 0x0218028101009011ULL, 0x0080812842010040ULL, 0x441000c000081020ULL, 0x1208208208540002ULL, 0x0828000980200004ULL, 0x0040040424080821ULL, 0x1004000c90004041ULL,
        0x0010200000839100ULL, 0x00040400800280c0ULL, 0x0008800042020091ULL, 0xb100ca0000408010ULL, 0x0005040022020102ULL, 0x0180200080100448ULL, 0x480010008818004cULL, 0x0824600050008104ULL, 0x20802000622202c2ULL,
    };
    // clang-format on

    for (Square Sq : Squares) {
        RookMagicBB[Sq].MagicNumber = RookMagicNumbers[Sq];
    }
}

void computeBishopMagicBitboard() {
    bool IsPrecomputed = BishopMagicBB[0].MagicNumber != 0;

    for (Square Sq : Squares) {
        if (!IsPrecomputed) {
            std::printf("Searching for a bishop magic number for square %2d ... ",
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
            if (std::get<0>(Result)) {
                BishopMagicBB[Sq].MagicNumber = MagicNumberCandidate;
                std::memcpy(static_cast<void*>(BishopMagicBB[Sq].AttackBB1),
                            static_cast<const void*>(std::get<1>(Result).data()),
                            (1 << DiagMagicBits) * sizeof(Bitboard));
                std::memcpy(static_cast<void*>(BishopMagicBB[Sq].AttackBB2),
                            static_cast<const void*>(std::get<2>(Result).data()),
                            (1 << DiagMagicBits) * sizeof(Bitboard));

                if (!IsPrecomputed) {
                    std::printf("OK (found 0x%016" PRIx64 " with %7" PRIu64 " trials).",
                           MagicNumberCandidate, Trial);
                    std::cout << std::endl;
                }
                break;
            }
        }
    }

    if (!IsPrecomputed) {
        std::printf("constexpr uint64_t BishopMagicNumbers[NumSquares] = {\n");
        for (Square Sq : Squares) {
            std::printf("0x%016" PRIx64 "ULL, ", BishopMagicBB[Sq].MagicNumber);

            if (squareToRank(Sq) == RankA) {
                std::printf("\n");
            }
        }
        std::printf("};\n");
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
            std::printf("Searching for a rook magic number for square %2d ... ", Sq);
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
                    std::printf("OK (found 0x%016" PRIx64 " with %7" PRIu64 " trials).",
                           MagicNumberCandidate, Trial);
                    std::cout << std::endl;
                }
                break;
            }
        }
    }

    if (!IsPrecomputed) {
        std::printf("constexpr uint64_t RookMagicNumbers[NumSquares] = {\n");
        for (Square Sq : Squares) {
            std::printf("0x%016" PRIx64 "ULL, ", RookMagicBB[Sq].MagicNumber);

            if (squareToRank(Sq) == RankA) {
                std::printf("\n");
            }
        }
        std::printf("};\n");
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
