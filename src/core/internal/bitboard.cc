//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "bitboard.h"
#include <cstring>

namespace nshogi {
namespace core {
namespace internal {
namespace bitboard {

using BB = Bitboard;

// clang-format off

const Bitboard SquareBB[NumSquares] = {
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

const Bitboard FileBB[NumFiles] = {
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

const Bitboard RankBB[NumRanks] = {
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

const Bitboard FirstAndSecondFurthestBB[NumColors] = {
    RankBB[RankA] | RankBB[RankB],
    RankBB[RankH] | RankBB[RankI],
};

const Bitboard PromotableBB[NumColors] = {
    RankBB[RankA] | RankBB[RankB] | RankBB[RankC],
    RankBB[RankG] | RankBB[RankH] | RankBB[RankI],
};

Bitboard KingAttackBB[NumSquares];
Bitboard StepPieceAttackBB[NumColors][3][NumSquares];

Bitboard DiagStepAttackBB[NumSquares];
Bitboard CrossStepAttackBB[NumSquares];

Bitboard LineBB[NumSquares][NumSquares];
Bitboard DirectionBB[11 + NorthNorthWest + 1][NumSquares];
Bitboard BetweenBB[NumSquares][NumSquares];
Bitboard DiagBB[NumSquares];
Bitboard CrossBB[NumSquares];
Bitboard ForwardBB[NumSquares];
Bitboard BackwardBB[NumSquares];

BishopRay BishopRayBB[NumSquares];
RookRay RookRayBB[NumSquares];

// clang-format on

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
    for (Square Sq : Squares) {
        StepPieceAttackBB[Black][0][Sq] = getBlackKnightAttack(Sq);
        StepPieceAttackBB[White][0][Sq] = getWhiteKnightAttack(Sq);
    }
}

void initializeSilverAttacks() {
    for (Square Sq : Squares) {
        StepPieceAttackBB[Black][1][Sq] = getBlackSilverAttack(Sq);
        StepPieceAttackBB[White][1][Sq] = getWhiteSilverAttack(Sq);
    }
}

void initializeGoldAttacks() {
    for (Square Sq : Squares) {
        StepPieceAttackBB[Black][2][Sq] = getBlackGoldAttack(Sq);
        StepPieceAttackBB[White][2][Sq] = getWhiteGoldAttack(Sq);
    }
}

void initializeKingAttacks() {
    for (Square Sq : Squares) {
        KingAttackBB[Sq] = getKingAttack(Sq);
    }
}

void initializeDiagStepAttacks() {
    for (Square Sq : Squares) {
        DiagStepAttackBB[Sq] = getDiagStepAttack(Sq);
    }
}

void initializeCrossStepAttacks() {
    for (Square Sq : Squares) {
        CrossStepAttackBB[Sq] = getCrossStepAttack(Sq);
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
                LineBB[Sq1][Sq2] = Bitboard::ZeroBB();
                LineBB[Sq1][Sq2] |=
                    (SquareBB[Sq1] | DirectionBB[11 + NorthEast][Sq1] |
                     DirectionBB[11 + SouthWest][Sq1]) &
                    (SquareBB[Sq2] | DirectionBB[11 + NorthEast][Sq2] |
                     DirectionBB[11 + SouthWest][Sq2]);
                LineBB[Sq1][Sq2] |=
                    (SquareBB[Sq1] | DirectionBB[11 + SouthEast][Sq1] |
                     DirectionBB[11 + NorthWest][Sq1]) &
                    (SquareBB[Sq2] | DirectionBB[11 + SouthEast][Sq2] |
                     DirectionBB[11 + NorthWest][Sq2]);
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

    for (Square From : Squares) {
        for (Square To : Squares) {
            Direction D = SquareDirection[(std::size_t)From][(std::size_t)To];
            if (D != Direction(0)) {
                DirectionBB[11 + D][From] |= SquareBB[To];
            }
        }
    }
}

void initializeBetweenBB() {
    std::memset(static_cast<void*>(BetweenBB), 0, sizeof(BetweenBB));

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                continue;
            }

            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + North][Sq1] &
                                   DirectionBB[11 + South][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + NorthEast][Sq1] &
                                   DirectionBB[11 + SouthWest][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + East][Sq1] &
                                   DirectionBB[11 + West][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + SouthEast][Sq1] &
                                   DirectionBB[11 + NorthWest][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + South][Sq1] &
                                   DirectionBB[11 + North][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + SouthWest][Sq1] &
                                   DirectionBB[11 + NorthEast][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + West][Sq1] &
                                   DirectionBB[11 + East][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
            BetweenBB[Sq1][Sq2] |= DirectionBB[11 + NorthWest][Sq1] &
                                   DirectionBB[11 + SouthEast][Sq2] &
                                   ~SquareBB[Sq1] & ~SquareBB[Sq2];
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

            const auto Direction =
                SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == NorthWest || Direction == NorthEast ||
                Direction == SouthWest || Direction == SouthEast) {
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

            const auto Direction =
                SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == North || Direction == East || Direction == South ||
                Direction == West) {
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

            const auto Direction =
                SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

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

            const auto Direction =
                SquareDirection[(std::size_t)Sq1][(std::size_t)Sq2];

            if (Direction == South) {
                BackwardBB[Sq1].toggleBit(Sq2);
            }
        }
    }
}

// Requires DirectionBB to be initialized beforehand.
void initializeSliderRayBB() {
    for (Square Sq : Squares) {
        BishopRayBB[Sq].NorthWest = DirectionBB[11 + NorthWest][Sq];
        BishopRayBB[Sq].SouthWest = DirectionBB[11 + SouthWest][Sq];
        BishopRayBB[Sq].NorthEast = DirectionBB[11 + NorthEast][Sq];
        BishopRayBB[Sq].SouthEast = DirectionBB[11 + SouthEast][Sq];

        RookRayBB[Sq].North = DirectionBB[11 + North][Sq];
        RookRayBB[Sq].West = DirectionBB[11 + West][Sq];
        RookRayBB[Sq].South = DirectionBB[11 + South][Sq];
        RookRayBB[Sq].East = DirectionBB[11 + East][Sq];
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

    initializeDirectionBB();
    initializeLineBB();
    initializeBetweenBB();
    initializeDiagBB();
    initializeCrossBB();
    initializeForwardBB();
    initializeBackwardBB();

    initializeSliderRayBB();
}

} // namespace bitboard
} // namespace internal
} // namespace core
} // namespace nshogi
