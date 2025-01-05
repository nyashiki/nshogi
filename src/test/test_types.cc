//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"

#include "../core/types.h"
#include <iostream>

namespace {

void testStand(uint32_t CountMax, nshogi::core::PieceTypeKind Type) {
    nshogi::core::Stands Stands = (nshogi::core::Stands)0;
    for (int I = 1; I <= (int)CountMax; I++) {
        Stands = nshogi::core::incrementStand(Stands, Type);
        TEST_ASSERT_EQ(nshogi::core::getStandCount(Stands, Type), (uint32_t)I);
    }
    for (int I = (int)CountMax - 1; I >= 0; I--) {
        Stands = nshogi::core::decrementStand(Stands, Type);
        TEST_ASSERT_EQ(nshogi::core::getStandCount(Stands, Type), (uint32_t)I);
    }
}

} // namespace

TEST(Types, FlipColors) {
    TEST_ASSERT_EQ(nshogi::core::Black, ~nshogi::core::White);
    TEST_ASSERT_EQ(nshogi::core::White, ~nshogi::core::Black);
}

TEST(Types, PieceToPieceType) {
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackPawn),
                    nshogi::core::PTK_Pawn);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackLance),
                    nshogi::core::PTK_Lance);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackKnight),
                    nshogi::core::PTK_Knight);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackSilver),
                    nshogi::core::PTK_Silver);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackGold),
                    nshogi::core::PTK_Gold);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackKing),
                    nshogi::core::PTK_King);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackBishop),
                    nshogi::core::PTK_Bishop);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackRook),
                    nshogi::core::PTK_Rook);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProPawn),
                    nshogi::core::PTK_ProPawn);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProLance),
                    nshogi::core::PTK_ProLance);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProKnight),
                    nshogi::core::PTK_ProKnight);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProSilver),
                    nshogi::core::PTK_ProSilver);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProBishop),
                    nshogi::core::PTK_ProBishop);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_BlackProRook),
                    nshogi::core::PTK_ProRook);

    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhitePawn),
                    nshogi::core::PTK_Pawn);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteLance),
                    nshogi::core::PTK_Lance);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteKnight),
                    nshogi::core::PTK_Knight);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteSilver),
                    nshogi::core::PTK_Silver);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteGold),
                    nshogi::core::PTK_Gold);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteKing),
                    nshogi::core::PTK_King);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteBishop),
                    nshogi::core::PTK_Bishop);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteRook),
                    nshogi::core::PTK_Rook);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProPawn),
                    nshogi::core::PTK_ProPawn);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProLance),
                    nshogi::core::PTK_ProLance);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProKnight),
                    nshogi::core::PTK_ProKnight);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProSilver),
                    nshogi::core::PTK_ProSilver);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProBishop),
                    nshogi::core::PTK_ProBishop);
    TEST_ASSERT_EQ(getPieceType(nshogi::core::PK_WhiteProRook),
                    nshogi::core::PTK_ProRook);
}

TEST(Types, PieceTypePromote) {
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Pawn),
                    nshogi::core::PTK_ProPawn);
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Lance),
                    nshogi::core::PTK_ProLance);
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Knight),
                    nshogi::core::PTK_ProKnight);
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Silver),
                    nshogi::core::PTK_ProSilver);
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Bishop),
                    nshogi::core::PTK_ProBishop);
    TEST_ASSERT_EQ(promotePieceType(nshogi::core::PTK_Rook),
                    nshogi::core::PTK_ProRook);
}

TEST(Types, PieceGetColor) {
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackPawn), nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackLance), nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackKnight),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackSilver),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackGold), nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackKing), nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackBishop),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackRook), nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProPawn),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProLance),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProKnight),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProSilver),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProBishop),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_BlackProRook),
                    nshogi::core::Black);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhitePawn), nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteLance), nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteKnight),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteSilver),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteGold), nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteKing), nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteBishop),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteRook), nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProPawn),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProLance),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProKnight),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProSilver),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProBishop),
                    nshogi::core::White);
    TEST_ASSERT_EQ(getColor(nshogi::core::PK_WhiteProRook),
                    nshogi::core::White);
}

TEST(Types, PiecePromote) {
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackPawn),
                    nshogi::core::PieceKind::PK_BlackProPawn);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackLance),
                    nshogi::core::PieceKind::PK_BlackProLance);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackKnight),
                    nshogi::core::PieceKind::PK_BlackProKnight);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackSilver),
                    nshogi::core::PieceKind::PK_BlackProSilver);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackBishop),
                    nshogi::core::PieceKind::PK_BlackProBishop);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_BlackRook),
                    nshogi::core::PieceKind::PK_BlackProRook);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhitePawn),
                    nshogi::core::PieceKind::PK_WhiteProPawn);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhiteLance),
                    nshogi::core::PieceKind::PK_WhiteProLance);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhiteKnight),
                    nshogi::core::PieceKind::PK_WhiteProKnight);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhiteSilver),
                    nshogi::core::PieceKind::PK_WhiteProSilver);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhiteBishop),
                    nshogi::core::PieceKind::PK_WhiteProBishop);
    TEST_ASSERT_EQ(promotePiece(nshogi::core::PK_WhiteRook),
                    nshogi::core::PieceKind::PK_WhiteProRook);
}

TEST(Types, IsPromoted) {
    using namespace nshogi::core;

    TEST_ASSERT_EQ(isPromoted(PTK_Pawn), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Lance), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Knight), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Silver), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Bishop), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Rook), false);
    TEST_ASSERT_EQ(isPromoted(PTK_Gold), false);
    TEST_ASSERT_EQ(isPromoted(PTK_King), false);

    TEST_ASSERT_EQ(isPromoted(PTK_ProPawn), true);
    TEST_ASSERT_EQ(isPromoted(PTK_ProLance), true);
    TEST_ASSERT_EQ(isPromoted(PTK_ProKnight), true);
    TEST_ASSERT_EQ(isPromoted(PTK_ProSilver), true);
    TEST_ASSERT_EQ(isPromoted(PTK_ProBishop), true);
}

TEST(Types, PieceTypeDemote) {
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Pawn),
                    nshogi::core::PTK_Pawn);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Lance),
                    nshogi::core::PTK_Lance);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Knight),
                    nshogi::core::PTK_Knight);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Silver),
                    nshogi::core::PTK_Silver);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Bishop),
                    nshogi::core::PTK_Bishop);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Rook),
                    nshogi::core::PTK_Rook);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_Gold),
                    nshogi::core::PTK_Gold);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProPawn),
                    nshogi::core::PTK_Pawn);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProLance),
                    nshogi::core::PTK_Lance);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProKnight),
                    nshogi::core::PTK_Knight);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProSilver),
                    nshogi::core::PTK_Silver);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProBishop),
                    nshogi::core::PTK_Bishop);
    TEST_ASSERT_EQ(demotePieceType(nshogi::core::PTK_ProRook),
                    nshogi::core::PTK_Rook);
}

TEST(Types, MakePiece) {
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Pawn),
                    nshogi::core::PK_BlackPawn);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Lance),
                    nshogi::core::PK_BlackLance);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Knight),
                    nshogi::core::PK_BlackKnight);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Silver),
                    nshogi::core::PK_BlackSilver);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Gold),
                    nshogi::core::PK_BlackGold);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_King),
                    nshogi::core::PK_BlackKing);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Bishop),
                    nshogi::core::PK_BlackBishop);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_Rook),
                    nshogi::core::PK_BlackRook);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProPawn),
                    nshogi::core::PK_BlackProPawn);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProLance),
                    nshogi::core::PK_BlackProLance);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProKnight),
                    nshogi::core::PK_BlackProKnight);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProSilver),
                    nshogi::core::PK_BlackProSilver);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProBishop),
                    nshogi::core::PK_BlackProBishop);
    TEST_ASSERT_EQ(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProRook),
                    nshogi::core::PK_BlackProRook);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Pawn),
                    nshogi::core::PK_WhitePawn);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Lance),
                    nshogi::core::PK_WhiteLance);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Knight),
                    nshogi::core::PK_WhiteKnight);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Silver),
                    nshogi::core::PK_WhiteSilver);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Gold),
                    nshogi::core::PK_WhiteGold);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_King),
                    nshogi::core::PK_WhiteKing);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Bishop),
                    nshogi::core::PK_WhiteBishop);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_Rook),
                    nshogi::core::PK_WhiteRook);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProPawn),
                    nshogi::core::PK_WhiteProPawn);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProLance),
                    nshogi::core::PK_WhiteProLance);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProKnight),
                    nshogi::core::PK_WhiteProKnight);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProSilver),
                    nshogi::core::PK_WhiteProSilver);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProBishop),
                    nshogi::core::PK_WhiteProBishop);
    TEST_ASSERT_EQ(makePiece<nshogi::core::White>(nshogi::core::PTK_ProRook),
                    nshogi::core::PK_WhiteProRook);
}

TEST(Types, FileRange) {
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File1));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File2));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File3));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File4));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File5));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File6));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File7));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File8));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::File9));
}

TEST(Types, RankRange) {
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankA));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankB));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankC));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankD));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankE));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankF));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankG));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankH));
    TEST_ASSERT_TRUE(checkRange(nshogi::core::RankI));
}

TEST(Types, SquareRange) {
    for (nshogi::core::Square sq : nshogi::core::Squares) {
        TEST_ASSERT_TRUE(checkRange(sq));
    }
}

TEST(Types, StandPawn) {
    testStand(18, nshogi::core::PTK_Pawn);
}

TEST(Types, StandGold) {
    testStand(4, nshogi::core::PTK_Gold);
}

TEST(Types, StandSilver) {
    testStand(4, nshogi::core::PTK_Silver);
}

TEST(Types, StandKnight) {
    testStand(4, nshogi::core::PTK_Knight);
}

TEST(Types, StandLance) {
    testStand(4, nshogi::core::PTK_Lance);
}

TEST(Types, StandBishop) {
    testStand(2, nshogi::core::PTK_Bishop);
}

TEST(Types, StandRook) {
    testStand(2, nshogi::core::PTK_Rook);
}

TEST(Types, StandSuperiorOrEqual) {
    using namespace nshogi::core;

    // clang-format off
    for (uint8_t St1PawnCount = 0; St1PawnCount <= 18; ++St1PawnCount) {
        for (uint8_t St1LanceCount = 0; St1LanceCount <= 4; ++St1LanceCount) {
            for (uint8_t St1KnightCount = 0; St1KnightCount <= 4; ++St1KnightCount) {
                for (uint8_t St1SilverCount = 0; St1SilverCount <= 4; ++St1SilverCount) {
                    for (uint8_t St1GoldCount = 0; St1GoldCount <= 4; ++St1GoldCount) {
                        for (uint8_t St1BishopCount = 0; St1BishopCount <= 2; ++St1BishopCount) {
                            for (uint8_t St1RookCount = 0; St1RookCount <= 2; ++St1RookCount) {
                                for (uint8_t St2PawnCount = 0; St2PawnCount <= 18; ++St2PawnCount) {
                                    for (uint8_t St2LanceCount = 1; St2LanceCount <= 4; St2LanceCount += 2) {
                                        for (uint8_t St2KnightCount = 1; St2KnightCount <= 4; St2KnightCount += 2) {
                                            for (uint8_t St2SilverCount = 1; St2SilverCount <= 4; St2SilverCount += 2) {
                                                for (uint8_t St2GoldCount = 1; St2GoldCount <= 4; St2GoldCount += 2) {
                                                    for (uint8_t St2BishopCount = 1; St2BishopCount <= 2; ++St2BishopCount) {
                                                        for (uint8_t St2RookCount = 1; St2RookCount <= 2; ++St2RookCount) {
                                                            Stands St1 = constructStand(St1PawnCount, St1LanceCount, St1KnightCount, St1SilverCount, St1GoldCount, St1BishopCount, St1RookCount);
                                                            Stands St2 = constructStand(St2PawnCount, St2LanceCount, St2KnightCount, St2SilverCount, St2GoldCount, St2BishopCount, St2RookCount);

                                                            if (St1PawnCount >= St2PawnCount && St1LanceCount >= St2LanceCount && St1KnightCount >= St2KnightCount &&
                                                                    St1SilverCount >= St2SilverCount && St1GoldCount >= St2GoldCount && St1BishopCount >= St2BishopCount && St1RookCount >= St2RookCount) {
                                                                TEST_ASSERT_TRUE(isSuperiorOrEqual(St1, St2));
                                                            } else {
                                                                TEST_ASSERT_FALSE(isSuperiorOrEqual(St1, St2));
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // clang-format on
}

TEST(Types, SquareDirection) {
    using namespace nshogi::core;

    TEST_ASSERT_EQ(SquareDirection[Sq1I][Sq1A], North);
    TEST_ASSERT_EQ(SquareDirection[Sq4F][Sq4E], North);
    TEST_ASSERT_EQ(SquareDirection[Sq1I][Sq9I], West);
    TEST_ASSERT_EQ(SquareDirection[Sq6D][Sq7D], West);
    TEST_ASSERT_EQ(SquareDirection[Sq4A][Sq3A], East);
    TEST_ASSERT_EQ(SquareDirection[Sq8B][Sq7B], East);
    TEST_ASSERT_EQ(SquareDirection[Sq5E][Sq5H], South);
    TEST_ASSERT_EQ(SquareDirection[Sq9H][Sq9I], South);
    TEST_ASSERT_EQ(SquareDirection[Sq7G][Sq6E], NorthNorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq5H][Sq6F], NorthNorthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq3B][Sq2D], SouthSouthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq7A][Sq8C], SouthSouthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq9I][Sq8H], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq6I][Sq5H], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq6I][Sq4G], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq6I][Sq3F], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq6I][Sq2E], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq6I][Sq1D], NorthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq9H][Sq8I], SouthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq9G][Sq8H], SouthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq9G][Sq7I], SouthEast);
    TEST_ASSERT_EQ(SquareDirection[Sq7A][Sq8B], SouthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq7A][Sq9C], SouthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq3D][Sq4C], NorthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq3D][Sq5B], NorthWest);
    TEST_ASSERT_EQ(SquareDirection[Sq3D][Sq6A], NorthWest);

    TEST_ASSERT_EQ(SquareSerializedDirection[Sq1I][Sq1A], 0);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq4F][Sq4E], 0);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq1I][Sq9I], 6);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6D][Sq7D], 6);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq4A][Sq3A], 2);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq8B][Sq7B], 2);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq5E][Sq5H], 4);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq9H][Sq9I], 4);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq7G][Sq6E], 8);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq5H][Sq6F], 9);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq3B][Sq2D], 10);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq7A][Sq8C], 11);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq9I][Sq8H], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6I][Sq5H], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6I][Sq4G], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6I][Sq3F], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6I][Sq2E], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq6I][Sq1D], 1);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq9H][Sq8I], 3);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq9G][Sq8H], 3);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq9G][Sq7I], 3);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq7A][Sq8B], 5);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq7A][Sq9C], 5);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq3D][Sq4C], 7);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq3D][Sq5B], 7);
    TEST_ASSERT_EQ(SquareSerializedDirection[Sq3D][Sq6A], 7);
}

TEST(Types, BoardMove) {
    using namespace nshogi::core;

    for (Square FromSq : Squares) {
        for (Square ToSq : Squares) {
            for (PieceTypeKind Type : PieceTypes) {
                Move32 Move = Move32::boardMove(FromSq, ToSq, Type);
                TEST_ASSERT_EQ(Move.from(), FromSq);
                TEST_ASSERT_EQ(Move.to(), ToSq);
                TEST_ASSERT_EQ(Move.pieceType(), Type);
                TEST_ASSERT_FALSE(Move.promote());
                TEST_ASSERT_FALSE(Move.drop());
                TEST_ASSERT_EQ(Move16(Move).from(), FromSq);
                TEST_ASSERT_EQ(Move16(Move).to(), ToSq);
                TEST_ASSERT_FALSE(Move16(Move).promote());
                TEST_ASSERT_FALSE(Move16(Move).drop());

                Move32 MoveP = Move32::boardPromotingMove(FromSq, ToSq, Type);
                TEST_ASSERT_EQ(MoveP.from(), FromSq);
                TEST_ASSERT_EQ(MoveP.to(), ToSq);
                TEST_ASSERT_EQ(MoveP.pieceType(), Type);
                TEST_ASSERT_TRUE(MoveP.promote());
                TEST_ASSERT_FALSE(MoveP.drop());
                TEST_ASSERT_EQ(Move16(MoveP).from(), FromSq);
                TEST_ASSERT_EQ(Move16(MoveP).to(), ToSq);
                TEST_ASSERT_TRUE(Move16(MoveP).promote());
                TEST_ASSERT_FALSE(Move16(MoveP).drop());

                for (PieceTypeKind Type2 : PieceTypes) {
                    Move32 MoveC = Move32::boardMove(FromSq, ToSq, Type, Type2);
                    TEST_ASSERT_EQ(MoveC.from(), FromSq);
                    TEST_ASSERT_EQ(MoveC.to(), ToSq);
                    TEST_ASSERT_EQ(MoveC.pieceType(), Type);
                    TEST_ASSERT_EQ(MoveC.capturePieceType(), Type2);
                    TEST_ASSERT_FALSE(MoveC.promote());
                    TEST_ASSERT_FALSE(MoveC.drop());
                    TEST_ASSERT_EQ(Move16(MoveC).from(), FromSq);
                    TEST_ASSERT_EQ(Move16(MoveC).to(), ToSq);
                    TEST_ASSERT_FALSE(Move16(MoveC).promote());
                    TEST_ASSERT_FALSE(Move16(MoveC).drop());

                    Move32 MovePC = Move32::boardPromotingMove(FromSq, ToSq, Type, Type2);
                    TEST_ASSERT_EQ(MovePC.from(), FromSq);
                    TEST_ASSERT_EQ(MovePC.to(), ToSq);
                    TEST_ASSERT_EQ(MovePC.pieceType(), Type);
                    TEST_ASSERT_EQ(MovePC.capturePieceType(), Type2);
                    TEST_ASSERT_TRUE(MovePC.promote());
                    TEST_ASSERT_FALSE(Move16(MovePC).drop());
                    TEST_ASSERT_EQ(Move16(MovePC).from(), FromSq);
                    TEST_ASSERT_EQ(Move16(MovePC).to(), ToSq);
                    TEST_ASSERT_TRUE(Move16(MovePC).promote());
                    TEST_ASSERT_FALSE(Move16(MovePC).drop());
                }
            }
        }
    }
}

TEST(Types, DroppingMove) {
    using namespace nshogi::core;

    for (Square ToSq : Squares) {
        for (PieceTypeKind Type : StandPieceTypes) {
            Move32 Move = Move32::droppingMove(ToSq, Type);
            TEST_ASSERT_EQ(Move.to(), ToSq);
            TEST_ASSERT_EQ(Move.pieceType(), Type);
            TEST_ASSERT_FALSE(Move.promote());
            TEST_ASSERT_TRUE(Move.drop());
            TEST_ASSERT_EQ(Move16(Move).to(), ToSq);
            TEST_ASSERT_FALSE(Move16(Move).promote());
            TEST_ASSERT_TRUE(Move16(Move).drop());
        }
    }
}

TEST(Types, SpecialMove) {
    using namespace nshogi::core;

    TEST_ASSERT_TRUE(Move32::MoveNone().isNone());
    TEST_ASSERT_FALSE(Move32::MoveNone().isInvalid());
    TEST_ASSERT_TRUE(Move32::MoveInvalid().isInvalid());
    TEST_ASSERT_FALSE(Move32::MoveInvalid().isNone());
    TEST_ASSERT_TRUE(Move16::MoveNone().isNone());
    TEST_ASSERT_FALSE(Move16::MoveNone().isInvalid());
    TEST_ASSERT_TRUE(Move16::MoveInvalid().isInvalid());
    TEST_ASSERT_FALSE(Move16::MoveInvalid().isNone());
}
