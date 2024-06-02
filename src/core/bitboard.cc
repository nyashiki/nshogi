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

MagicBitboard<DiagMagicBits> BishopMagicBB[NumSquares];
MagicBitboard<CrossMagicBits> RookMagicBB[NumSquares];

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
    const auto& [A1, E1, B1] = getPatternBB<NorthEast, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] = getPatternBB<SouthWest, true>(Sq, Pattern, false, B1);

    assert(B2 <= 7);
    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~FileBB[File9] &
                             ~FileBB[File1] & ~RankBB[RankI] & ~RankBB[RankA] &
                             ~SquareBB[Sq];
    return {AttackBB, ExistBB};
}

std::pair<Bitboard, Bitboard> diagNWSEPatternBB(Square Sq, const uint16_t Pattern) {
    const auto& [A1, E1, B1] = getPatternBB<SouthEast, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] = getPatternBB<NorthWest, true>(Sq, Pattern, false, B1);

    assert(B2 <= 7);
    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~FileBB[File9] &
                             ~FileBB[File1] & ~RankBB[RankI] & ~RankBB[RankA] &
                             ~SquareBB[Sq];
    return {AttackBB, ExistBB};
}

std::pair<Bitboard, Bitboard> verticalPatternBB(Square Sq,
                                             const uint16_t Pattern) {
    const auto& [A1, E1, B1] = getPatternBB<North, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] = getPatternBB<South, true>(Sq, Pattern, false, B1);

    assert(B2 <= 7);
    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~SquareBB[Sq] & ~RankBB[RankA] & ~RankBB[RankI];
    return {AttackBB, ExistBB};
}

std::pair<Bitboard, Bitboard> horizontalPatternBB(Square Sq,
                                             const uint16_t Pattern) {
    const auto& [A1, E1, B1] = getPatternBB<East, true>(Sq, Pattern, false, 0);
    const auto& [A2, E2, B2] = getPatternBB<West, true>(Sq, Pattern, false, B1);

    assert(B2 <= 7);

    const Bitboard AttackBB = (A1 | A2) & ~SquareBB[Sq];
    const Bitboard ExistBB = (E1 | E2) & ~SquareBB[Sq];
    return {AttackBB, ExistBB};
}

Bitboard diagSWNEPatternMask(Square Sq) {
    return diagSWNEPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard diagNWSEPatternMask(Square Sq) {
    return diagNWSEPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard verticalPatternMask(Square Sq) {
    return verticalPatternBB(Sq, (1 << 15) - 1).second;
}

Bitboard horizontalPatternBB(Square Sq) {
    return horizontalPatternBB(Sq, (1 << 15) - 1).second;
}

void setBishopMagicMasks() {
    for (Square Sq : Squares) {
        BishopMagicBB[Sq].Masks[0] = diagSWNEPatternMask(Sq);
        BishopMagicBB[Sq].Masks[1] = diagNWSEPatternMask(Sq);
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
        0x400c024100082012ULL, 0x0400502040880514ULL, 0x121a100820820300ULL, 0x0019010020038000ULL, 0x0000410801002410ULL, 0x044184014100020eULL, 0x4008c02380090000ULL, 0x008004d080082120ULL, 0x0800800804400220ULL,
        0x8011004088101010ULL, 0x002060802c042214ULL, 0x040a105100040201ULL, 0xc30391a080088908ULL, 0xc108082042208004ULL, 0x0900027020420000ULL, 0x00004014300984d2ULL, 0x4001000250102804ULL, 0x2020000408012010ULL,
        0x9024140100600a10ULL, 0x0852100042041002ULL, 0x3048844100280808ULL, 0x00101c0c04c2090aULL, 0x082620048cc80a20ULL, 0x5810a04ca4288011ULL, 0x40014042a4100288ULL, 0x0800c0540c024408ULL, 0x0a2880002020a104ULL,
        0x0210004012001006ULL, 0x801100a0001d8214ULL, 0x0024004083022002ULL, 0x010a0081100aa004ULL, 0x0800a08c1301c404ULL, 0x0808420340080480ULL, 0x0119000800230390ULL, 0x0801002080481220ULL, 0x2251042300020811ULL,
        0x008a09218a202040ULL, 0x00a090804002aa11ULL, 0x0042005031008810ULL, 0x0200841030040c08ULL, 0x580402c080052004ULL, 0x2213002920210442ULL, 0x13022010c4c80691ULL, 0x110200044140089aULL, 0x001218253000a881ULL,
        0x2144084104800824ULL, 0x0901011222840052ULL, 0x1248d00440a80141ULL, 0x4045841031060008ULL, 0x810004490c080450ULL, 0x01010a0062061001ULL, 0x0042e19040018012ULL, 0xca880180040c0428ULL, 0x404040c001049084ULL,
        0x0101284201001100ULL, 0x0c02800815004081ULL, 0x0230068088401000ULL, 0x1182100810222004ULL, 0x1490200041102000ULL, 0x4040501200498204ULL, 0x01a0002102208119ULL, 0x0100c04d40058404ULL, 0x10080850024a3102ULL,
        0x0800480c0b001020ULL, 0x0800088020848210ULL, 0x000020142001080aULL, 0x01118000140820b1ULL, 0x80c8021040808616ULL, 0x0480102810101028ULL, 0x0080023011008601ULL, 0x800102084c803404ULL, 0x005400400820004cULL,
        0x400404210802c010ULL, 0x0504800e14a00931ULL, 0x6422000590062003ULL, 0x0a03801421101048ULL, 0x0100000400400825ULL, 0x020010a404400402ULL, 0x8100044126022492ULL, 0x102002182004100cULL, 0x00a080c120800804ULL,
    };
    // clang-format on

    for (Square Sq : Squares) {
        BishopMagicBB[Sq].MagicNumber = BishopMagicNumbers[Sq];
    }
};

void setRookMagicNumbers() {
    // clang-format off
    constexpr uint64_t RookMagicNumbers[NumSquares] = {
        0x4c08021004220280ULL, 0x0810010029020428ULL, 0x4a04080a80100440ULL, 0x0188020420043010ULL, 0x8100270100880108ULL, 0xa810208020922410ULL, 0x0100488010421055ULL, 0x010c804c21280501ULL, 0x1020282200840072ULL,
        0x800a220808210014ULL, 0x0210204144008180ULL, 0x0211412301003811ULL, 0xac08804102198801ULL, 0x20604042840c0c20ULL, 0x1040800422882110ULL, 0x0088c04014081009ULL, 0x208870130a100103ULL, 0x0060803002008022ULL,
        0x04040c0808208210ULL, 0x400a0040c4102080ULL, 0x84008c90424048a2ULL, 0x0101214081001026ULL, 0x0310204020524002ULL, 0x04202440801022d8ULL, 0x0200440801428204ULL, 0x0010040805480024ULL, 0x0248102001004232ULL,
        0x7806040502001241ULL, 0xa004044010688030ULL, 0x0104040020250022ULL, 0x0284040304042004ULL, 0x00202300c0801102ULL, 0x4a04403020020090ULL, 0x481c188020828404ULL, 0x08101021811b0244ULL, 0x00220400a0210202ULL,
        0x8004080184108300ULL, 0x10845500c3050100ULL, 0xd0018a0010210006ULL, 0x0044008202500404ULL, 0xd300410040102905ULL, 0x00a0810012101c01ULL, 0x2100042003240238ULL, 0x0480800202202901ULL, 0x2028410800810081ULL,
        0x2020241001400808ULL, 0x0220070034412040ULL, 0x0210002121011020ULL, 0x0080808100405021ULL, 0x84408058a0301802ULL, 0x0021890050410238ULL, 0x0089400c20211408ULL, 0x8900045464005016ULL, 0x2004220081028802ULL,
        0x2122020a02608014ULL, 0x0104180801002104ULL, 0x0084210040114411ULL, 0x004208439080108aULL, 0x0082204860204004ULL, 0x0a01080810100222ULL, 0x0a010a082004004eULL, 0x404008a12001403cULL, 0x1180011009220124ULL,
        0x0101220810512029ULL, 0x048910402020208aULL, 0x0102068100081004ULL, 0x1410220810602040ULL, 0x010010101880200aULL, 0x08102100051f0082ULL, 0x2100040802600902ULL, 0x10c02001011020c2ULL, 0x5100430242120031ULL,
        0x2404080054040040ULL, 0x4028a08089004080ULL, 0x5000802480210020ULL, 0x100084a040400220ULL, 0x0802408080100503ULL, 0x0010802004202001ULL, 0x0280800801040488ULL, 0x0048200400a20802ULL, 0x000a040108020882ULL,
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
            std::printf("Searching for a bishop magic number for square %2d ... ", Sq);
            std::cout << std::flush;
        }

        for (uint64_t Trial = 0;; ++Trial) {
            const uint64_t MagicNumberCandidate =
                (IsPrecomputed) ? BishopMagicBB[Sq].MagicNumber
                                : generateMagicNumberCandidate();

            const auto& Result = isBishopMagicNumberOK(Sq, MagicNumberCandidate);
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
        RookMagicBB[Sq].Masks[0] = verticalPatternMask(Sq);
        RookMagicBB[Sq].Masks[1] = horizontalPatternBB(Sq);
    }
}

std::tuple<bool, std::vector<Bitboard>, std::vector<Bitboard>>
isRookMagicNumberOK(Square Sq, uint64_t MagicNumber) {
    std::vector<Bitboard> AttackBBs1(1 << CrossMagicBits, {0, 0});
    std::vector<Bitboard> AttackBBs2(1 << CrossMagicBits, {0, 0});

    // clang-format off
    const uint16_t NumBitsSN[NumSquares] = {
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
        7, 6, 6, 6, 6, 6, 6, 6, 7,
    };
    const uint16_t NumBitsWE[NumSquares] = {
        7, 7, 7, 7, 7, 7, 7, 7, 7,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7,
    };
    // clang-format on

    const uint64_t ShiftAmount = 64 - CrossMagicBits;
    {
        const uint16_t NumBit = NumBitsSN[Sq];
        for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
            const auto& [AttackBB, BB] = verticalPatternBB(Sq, Pattern);
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
        const uint16_t NumBit = NumBitsWE[Sq];
        for (uint16_t Pattern = 0; Pattern < (1 << NumBit); ++Pattern) {
            const auto& [AttackBB, BB] = horizontalPatternBB(Sq, Pattern);
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
            if (std::get<0>(Result)) {
                RookMagicBB[Sq].MagicNumber = MagicNumberCandidate;
                std::memcpy(static_cast<void*>(RookMagicBB[Sq].AttackBB1),
                            static_cast<const void*>(std::get<1>(Result).data()),
                            (1 << CrossMagicBits) * sizeof(Bitboard));
                std::memcpy(static_cast<void*>(RookMagicBB[Sq].AttackBB2),
                            static_cast<const void*>(std::get<2>(Result).data()),
                            (1 << CrossMagicBits) * sizeof(Bitboard));

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
            getPatternBB<North, true>(Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + NorthEast][Sq] =
            std::get<1>(getPatternBB<NorthEast, true>(
                Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + East][Sq] = std::get<1>(
            getPatternBB<East, true>(Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + SouthEast][Sq] =
            std::get<1>(getPatternBB<SouthEast, true>(
                Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + South][Sq] = std::get<1>(
            getPatternBB<South, true>(Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + SouthWest][Sq] =
            std::get<1>(getPatternBB<SouthWest, true>(
                Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + West][Sq] = std::get<1>(
            getPatternBB<West, true>(Sq, (1 << 15) - 1, false, 0));
        DirectionBB[11 + NorthWest][Sq] =
            std::get<1>(getPatternBB<NorthWest, true>(
                Sq, (1 << 15) - 1, false, 0));
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
