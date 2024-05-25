#include <CUnit/CUnit.h>

#include "../core/types.h"
#include <iostream>

namespace {

void testFlipColors() {
    CU_ASSERT_EQUAL(nshogi::core::Black, ~nshogi::core::White);
    CU_ASSERT_EQUAL(nshogi::core::White, ~nshogi::core::Black);
}

void testPieceToPieceType() {
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackPawn),
                    nshogi::core::PTK_Pawn);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackLance),
                    nshogi::core::PTK_Lance);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackKnight),
                    nshogi::core::PTK_Knight);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackSilver),
                    nshogi::core::PTK_Silver);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackGold),
                    nshogi::core::PTK_Gold);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackKing),
                    nshogi::core::PTK_King);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackBishop),
                    nshogi::core::PTK_Bishop);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackRook),
                    nshogi::core::PTK_Rook);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProPawn),
                    nshogi::core::PTK_ProPawn);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProLance),
                    nshogi::core::PTK_ProLance);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProKnight),
                    nshogi::core::PTK_ProKnight);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProSilver),
                    nshogi::core::PTK_ProSilver);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProBishop),
                    nshogi::core::PTK_ProBishop);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_BlackProRook),
                    nshogi::core::PTK_ProRook);

    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhitePawn),
                    nshogi::core::PTK_Pawn);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteLance),
                    nshogi::core::PTK_Lance);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteKnight),
                    nshogi::core::PTK_Knight);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteSilver),
                    nshogi::core::PTK_Silver);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteGold),
                    nshogi::core::PTK_Gold);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteKing),
                    nshogi::core::PTK_King);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteBishop),
                    nshogi::core::PTK_Bishop);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteRook),
                    nshogi::core::PTK_Rook);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProPawn),
                    nshogi::core::PTK_ProPawn);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProLance),
                    nshogi::core::PTK_ProLance);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProKnight),
                    nshogi::core::PTK_ProKnight);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProSilver),
                    nshogi::core::PTK_ProSilver);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProBishop),
                    nshogi::core::PTK_ProBishop);
    CU_ASSERT_EQUAL(getPieceType(nshogi::core::PK_WhiteProRook),
                    nshogi::core::PTK_ProRook);
}

void testPieceTypePromote() {
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Pawn),
                    nshogi::core::PTK_ProPawn);
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Lance),
                    nshogi::core::PTK_ProLance);
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Knight),
                    nshogi::core::PTK_ProKnight);
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Silver),
                    nshogi::core::PTK_ProSilver);
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Bishop),
                    nshogi::core::PTK_ProBishop);
    CU_ASSERT_EQUAL(promotePieceType(nshogi::core::PTK_Rook),
                    nshogi::core::PTK_ProRook);
}

void testPieceGetColor() {
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackPawn), nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackLance), nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackKnight),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackSilver),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackGold), nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackKing), nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackBishop),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackRook), nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProPawn),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProLance),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProKnight),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProSilver),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProBishop),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_BlackProRook),
                    nshogi::core::Black);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhitePawn), nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteLance), nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteKnight),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteSilver),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteGold), nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteKing), nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteBishop),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteRook), nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProPawn),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProLance),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProKnight),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProSilver),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProBishop),
                    nshogi::core::White);
    CU_ASSERT_EQUAL(getColor(nshogi::core::PK_WhiteProRook),
                    nshogi::core::White);
}

void testPiecePromote() {
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackPawn),
                    nshogi::core::PieceKind::PK_BlackProPawn);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackLance),
                    nshogi::core::PieceKind::PK_BlackProLance);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackKnight),
                    nshogi::core::PieceKind::PK_BlackProKnight);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackSilver),
                    nshogi::core::PieceKind::PK_BlackProSilver);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackBishop),
                    nshogi::core::PieceKind::PK_BlackProBishop);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_BlackRook),
                    nshogi::core::PieceKind::PK_BlackProRook);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhitePawn),
                    nshogi::core::PieceKind::PK_WhiteProPawn);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhiteLance),
                    nshogi::core::PieceKind::PK_WhiteProLance);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhiteKnight),
                    nshogi::core::PieceKind::PK_WhiteProKnight);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhiteSilver),
                    nshogi::core::PieceKind::PK_WhiteProSilver);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhiteBishop),
                    nshogi::core::PieceKind::PK_WhiteProBishop);
    CU_ASSERT_EQUAL(promotePiece(nshogi::core::PK_WhiteRook),
                    nshogi::core::PieceKind::PK_WhiteProRook);
}

void testIsPromoted() {
    using namespace nshogi::core;

    CU_ASSERT_EQUAL(isPromoted(PTK_Pawn), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Lance), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Knight), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Silver), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Bishop), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Rook), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_Gold), false);
    CU_ASSERT_EQUAL(isPromoted(PTK_King), false);

    CU_ASSERT_EQUAL(isPromoted(PTK_ProPawn), true);
    CU_ASSERT_EQUAL(isPromoted(PTK_ProLance), true);
    CU_ASSERT_EQUAL(isPromoted(PTK_ProKnight), true);
    CU_ASSERT_EQUAL(isPromoted(PTK_ProSilver), true);
    CU_ASSERT_EQUAL(isPromoted(PTK_ProBishop), true);
}

void testPieceTypeDemote() {
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Pawn),
                    nshogi::core::PTK_Pawn);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Lance),
                    nshogi::core::PTK_Lance);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Knight),
                    nshogi::core::PTK_Knight);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Silver),
                    nshogi::core::PTK_Silver);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Bishop),
                    nshogi::core::PTK_Bishop);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Rook),
                    nshogi::core::PTK_Rook);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_Gold),
                    nshogi::core::PTK_Gold);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProPawn),
                    nshogi::core::PTK_Pawn);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProLance),
                    nshogi::core::PTK_Lance);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProKnight),
                    nshogi::core::PTK_Knight);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProSilver),
                    nshogi::core::PTK_Silver);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProBishop),
                    nshogi::core::PTK_Bishop);
    CU_ASSERT_EQUAL(demotePieceType(nshogi::core::PTK_ProRook),
                    nshogi::core::PTK_Rook);
}

void testMakePiece() {
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Pawn),
                    nshogi::core::PK_BlackPawn);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Lance),
                    nshogi::core::PK_BlackLance);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Knight),
                    nshogi::core::PK_BlackKnight);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Silver),
                    nshogi::core::PK_BlackSilver);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Gold),
                    nshogi::core::PK_BlackGold);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_King),
                    nshogi::core::PK_BlackKing);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Bishop),
                    nshogi::core::PK_BlackBishop);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_Rook),
                    nshogi::core::PK_BlackRook);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProPawn),
                    nshogi::core::PK_BlackProPawn);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProLance),
                    nshogi::core::PK_BlackProLance);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProKnight),
                    nshogi::core::PK_BlackProKnight);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProSilver),
                    nshogi::core::PK_BlackProSilver);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProBishop),
                    nshogi::core::PK_BlackProBishop);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::Black>(nshogi::core::PTK_ProRook),
                    nshogi::core::PK_BlackProRook);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Pawn),
                    nshogi::core::PK_WhitePawn);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Lance),
                    nshogi::core::PK_WhiteLance);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Knight),
                    nshogi::core::PK_WhiteKnight);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Silver),
                    nshogi::core::PK_WhiteSilver);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Gold),
                    nshogi::core::PK_WhiteGold);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_King),
                    nshogi::core::PK_WhiteKing);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Bishop),
                    nshogi::core::PK_WhiteBishop);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_Rook),
                    nshogi::core::PK_WhiteRook);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProPawn),
                    nshogi::core::PK_WhiteProPawn);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProLance),
                    nshogi::core::PK_WhiteProLance);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProKnight),
                    nshogi::core::PK_WhiteProKnight);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProSilver),
                    nshogi::core::PK_WhiteProSilver);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProBishop),
                    nshogi::core::PK_WhiteProBishop);
    CU_ASSERT_EQUAL(makePiece<nshogi::core::White>(nshogi::core::PTK_ProRook),
                    nshogi::core::PK_WhiteProRook);
}

void testFileRange() {
    CU_ASSERT_TRUE(checkRange(nshogi::core::File1));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File2));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File3));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File4));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File5));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File6));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File7));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File8));
    CU_ASSERT_TRUE(checkRange(nshogi::core::File9));
}

void testRankRange() {
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankA));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankB));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankC));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankD));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankE));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankF));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankG));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankH));
    CU_ASSERT_TRUE(checkRange(nshogi::core::RankI));
}

void testSquareRange() {
    for (nshogi::core::Square sq : nshogi::core::Squares) {
        CU_ASSERT_TRUE(checkRange(sq));
    }
}

void testStand(uint32_t CountMax, nshogi::core::PieceTypeKind Type) {
    nshogi::core::Stands Stands = (nshogi::core::Stands)0;
    for (int I = 1; I <= (int)CountMax; I++) {
        Stands = nshogi::core::incrementStand(Stands, Type);
        CU_ASSERT_EQUAL(nshogi::core::getStandCount(Stands, Type), (uint32_t)I);
    }
    for (int I = (int)CountMax - 1; I >= 0; I--) {
        Stands = nshogi::core::decrementStand(Stands, Type);
        CU_ASSERT_EQUAL(nshogi::core::getStandCount(Stands, Type), (uint32_t)I);
    }
}

void testStandPawn() {
    testStand(18, nshogi::core::PTK_Pawn);
}

void testStandGold() {
    testStand(4, nshogi::core::PTK_Gold);
}

void testStandSilver() {
    testStand(4, nshogi::core::PTK_Silver);
}

void testStandKnight() {
    testStand(4, nshogi::core::PTK_Knight);
}

void testStandLance() {
    testStand(4, nshogi::core::PTK_Lance);
}

void testStandBishop() {
    testStand(2, nshogi::core::PTK_Bishop);
}

void testStandRook() {
    testStand(2, nshogi::core::PTK_Rook);
}

void testStandSuperiorOrEqual() {
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
                                                                CU_ASSERT_TRUE(isSuperiorOrEqual(St1, St2));
                                                            } else {
                                                                CU_ASSERT_FALSE(isSuperiorOrEqual(St1, St2));
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

void testSquareDirection() {
    using namespace nshogi::core;

    CU_ASSERT_EQUAL(SquareDirection[Sq1I][Sq1A], North);
    CU_ASSERT_EQUAL(SquareDirection[Sq4F][Sq4E], North);
    CU_ASSERT_EQUAL(SquareDirection[Sq1I][Sq9I], West);
    CU_ASSERT_EQUAL(SquareDirection[Sq6D][Sq7D], West);
    CU_ASSERT_EQUAL(SquareDirection[Sq4A][Sq3A], East);
    CU_ASSERT_EQUAL(SquareDirection[Sq8B][Sq7B], East);
    CU_ASSERT_EQUAL(SquareDirection[Sq5E][Sq5H], South);
    CU_ASSERT_EQUAL(SquareDirection[Sq9H][Sq9I], South);
    CU_ASSERT_EQUAL(SquareDirection[Sq7G][Sq6E], NorthNorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq5H][Sq6F], NorthNorthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq3B][Sq2D], SouthSouthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq7A][Sq8C], SouthSouthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq9I][Sq8H], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq6I][Sq5H], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq6I][Sq4G], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq6I][Sq3F], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq6I][Sq2E], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq6I][Sq1D], NorthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq9H][Sq8I], SouthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq9G][Sq8H], SouthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq9G][Sq7I], SouthEast);
    CU_ASSERT_EQUAL(SquareDirection[Sq7A][Sq8B], SouthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq7A][Sq9C], SouthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq3D][Sq4C], NorthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq3D][Sq5B], NorthWest);
    CU_ASSERT_EQUAL(SquareDirection[Sq3D][Sq6A], NorthWest);

    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq1I][Sq1A], 0);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq4F][Sq4E], 0);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq1I][Sq9I], 6);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6D][Sq7D], 6);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq4A][Sq3A], 2);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq8B][Sq7B], 2);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq5E][Sq5H], 4);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq9H][Sq9I], 4);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq7G][Sq6E], 8);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq5H][Sq6F], 9);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq3B][Sq2D], 10);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq7A][Sq8C], 11);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq9I][Sq8H], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6I][Sq5H], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6I][Sq4G], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6I][Sq3F], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6I][Sq2E], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq6I][Sq1D], 1);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq9H][Sq8I], 3);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq9G][Sq8H], 3);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq9G][Sq7I], 3);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq7A][Sq8B], 5);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq7A][Sq9C], 5);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq3D][Sq4C], 7);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq3D][Sq5B], 7);
    CU_ASSERT_EQUAL(SquareSerializedDirection[Sq3D][Sq6A], 7);
}

void testBoardMove() {
    using namespace nshogi::core;

    for (Square FromSq : Squares) {
        for (Square ToSq : Squares) {
            for (PieceTypeKind Type : PieceTypes) {
                Move32 Move = Move32::boardMove(FromSq, ToSq, Type);
                CU_ASSERT_EQUAL(Move.from(), FromSq);
                CU_ASSERT_EQUAL(Move.to(), ToSq);
                CU_ASSERT_EQUAL(Move.pieceType(), Type);
                CU_ASSERT_FALSE(Move.promote());
                CU_ASSERT_FALSE(Move.drop());
                CU_ASSERT_EQUAL(Move16(Move).from(), FromSq);
                CU_ASSERT_EQUAL(Move16(Move).to(), ToSq);
                CU_ASSERT_FALSE(Move16(Move).promote());
                CU_ASSERT_FALSE(Move16(Move).drop());

                Move32 MoveP = Move32::boardPromotingMove(FromSq, ToSq, Type);
                CU_ASSERT_EQUAL(MoveP.from(), FromSq);
                CU_ASSERT_EQUAL(MoveP.to(), ToSq);
                CU_ASSERT_EQUAL(MoveP.pieceType(), Type);
                CU_ASSERT_TRUE(MoveP.promote());
                CU_ASSERT_FALSE(MoveP.drop());
                CU_ASSERT_EQUAL(Move16(MoveP).from(), FromSq);
                CU_ASSERT_EQUAL(Move16(MoveP).to(), ToSq);
                CU_ASSERT_TRUE(Move16(MoveP).promote());
                CU_ASSERT_FALSE(Move16(MoveP).drop());

                for (PieceTypeKind Type2 : PieceTypes) {
                    Move32 MoveC = Move32::boardMove(FromSq, ToSq, Type, Type2);
                    CU_ASSERT_EQUAL(MoveC.from(), FromSq);
                    CU_ASSERT_EQUAL(MoveC.to(), ToSq);
                    CU_ASSERT_EQUAL(MoveC.pieceType(), Type);
                    CU_ASSERT_EQUAL(MoveC.capturePieceType(), Type2);
                    CU_ASSERT_FALSE(MoveC.promote());
                    CU_ASSERT_FALSE(MoveC.drop());
                    CU_ASSERT_EQUAL(Move16(MoveC).from(), FromSq);
                    CU_ASSERT_EQUAL(Move16(MoveC).to(), ToSq);
                    CU_ASSERT_FALSE(Move16(MoveC).promote());
                    CU_ASSERT_FALSE(Move16(MoveC).drop());

                    Move32 MovePC = Move32::boardPromotingMove(FromSq, ToSq, Type, Type2);
                    CU_ASSERT_EQUAL(MovePC.from(), FromSq);
                    CU_ASSERT_EQUAL(MovePC.to(), ToSq);
                    CU_ASSERT_EQUAL(MovePC.pieceType(), Type);
                    CU_ASSERT_EQUAL(MovePC.capturePieceType(), Type2);
                    CU_ASSERT_TRUE(MovePC.promote());
                    CU_ASSERT_FALSE(Move16(MovePC).drop());
                    CU_ASSERT_EQUAL(Move16(MovePC).from(), FromSq);
                    CU_ASSERT_EQUAL(Move16(MovePC).to(), ToSq);
                    CU_ASSERT_TRUE(Move16(MovePC).promote());
                    CU_ASSERT_FALSE(Move16(MovePC).drop());
                }
            }
        }
    }
}

void testDroppingMove() {
    using namespace nshogi::core;

    for (Square ToSq : Squares) {
        for (PieceTypeKind Type : StandPieceTypes) {
            Move32 Move = Move32::droppingMove(ToSq, Type);
            CU_ASSERT_EQUAL(Move.to(), ToSq);
            CU_ASSERT_EQUAL(Move.pieceType(), Type);
            CU_ASSERT_FALSE(Move.promote());
            CU_ASSERT_TRUE(Move.drop());
            CU_ASSERT_EQUAL(Move16(Move).to(), ToSq);
            CU_ASSERT_FALSE(Move16(Move).promote());
            CU_ASSERT_TRUE(Move16(Move).drop());
        }
    }
}

void testSpecialMove() {
    using namespace nshogi::core;

    CU_ASSERT_TRUE(Move32::MoveNone().isNone());
    CU_ASSERT_FALSE(Move32::MoveNone().isInvalid());
    CU_ASSERT_TRUE(Move32::MoveInvalid().isInvalid());
    CU_ASSERT_FALSE(Move32::MoveInvalid().isNone());
    CU_ASSERT_TRUE(Move16::MoveNone().isNone());
    CU_ASSERT_FALSE(Move16::MoveNone().isInvalid());
    CU_ASSERT_TRUE(Move16::MoveInvalid().isInvalid());
    CU_ASSERT_FALSE(Move16::MoveInvalid().isNone());
}


} // namespace

int setupTestTypes() {
    CU_pSuite suite = CU_add_suite("types.h test", 0, 0);

    CU_add_test(suite, "FlipColor", testFlipColors);
    CU_add_test(suite, "PieceToPieceType", testPieceToPieceType);
    CU_add_test(suite, "PromotePieceType", testPieceTypePromote);
    CU_add_test(suite, "DemotePieceType", testPieceTypeDemote);
    CU_add_test(suite, "GetColorPiece", testPieceGetColor);
    CU_add_test(suite, "PromotePiece", testPiecePromote);
    CU_add_test(suite, "IsPromoted", testIsPromoted);
    CU_add_test(suite, "MakePiece", testMakePiece);
    CU_add_test(suite, "FileRange", testFileRange);
    CU_add_test(suite, "RankRange", testRankRange);
    CU_add_test(suite, "SquareRange", testSquareRange);
    CU_add_test(suite, "StandPawn", testStandPawn);
    CU_add_test(suite, "StandGold", testStandGold);
    CU_add_test(suite, "StandSilver", testStandSilver);
    CU_add_test(suite, "StandKnight", testStandKnight);
    CU_add_test(suite, "StandLance", testStandLance);
    CU_add_test(suite, "StandBishop", testStandBishop);
    CU_add_test(suite, "StandRook", testStandRook);
    CU_add_test(suite, "SuperiorOrEqual", testStandSuperiorOrEqual);
    CU_add_test(suite, "SquareDirection", testSquareDirection);
    CU_add_test(suite, "BoardMove", testBoardMove);
    CU_add_test(suite, "DroppingMove", testDroppingMove);
    CU_add_test(suite, "SpecialMove", testSpecialMove);

    return CUE_SUCCESS;
}
