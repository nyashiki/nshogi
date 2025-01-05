//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"
#include "../core/positionbuilder.h"
#include "../core/statebuilder.h"
#include "../io/sfen.h"
#include <stdexcept>
#include <vector>

TEST(Sfen, Files) {
    using namespace nshogi::core;
    using namespace nshogi::io::sfen;

    TEST_ASSERT_EQ(fileToChar(File1), '1');
    TEST_ASSERT_EQ(fileToChar(File2), '2');
    TEST_ASSERT_EQ(fileToChar(File3), '3');
    TEST_ASSERT_EQ(fileToChar(File4), '4');
    TEST_ASSERT_EQ(fileToChar(File5), '5');
    TEST_ASSERT_EQ(fileToChar(File6), '6');
    TEST_ASSERT_EQ(fileToChar(File7), '7');
    TEST_ASSERT_EQ(fileToChar(File8), '8');
    TEST_ASSERT_EQ(fileToChar(File9), '9');

    TEST_ASSERT_EQ(charToFile('1'), File1);
    TEST_ASSERT_EQ(charToFile('2'), File2);
    TEST_ASSERT_EQ(charToFile('3'), File3);
    TEST_ASSERT_EQ(charToFile('4'), File4);
    TEST_ASSERT_EQ(charToFile('5'), File5);
    TEST_ASSERT_EQ(charToFile('6'), File6);
    TEST_ASSERT_EQ(charToFile('7'), File7);
    TEST_ASSERT_EQ(charToFile('8'), File8);
    TEST_ASSERT_EQ(charToFile('9'), File9);
}

TEST(Sfen, Ranks) {
    using namespace nshogi::core;
    using namespace nshogi::io::sfen;

    TEST_ASSERT_EQ(rankToChar(RankA), 'a');
    TEST_ASSERT_EQ(rankToChar(RankB), 'b');
    TEST_ASSERT_EQ(rankToChar(RankC), 'c');
    TEST_ASSERT_EQ(rankToChar(RankD), 'd');
    TEST_ASSERT_EQ(rankToChar(RankE), 'e');
    TEST_ASSERT_EQ(rankToChar(RankF), 'f');
    TEST_ASSERT_EQ(rankToChar(RankG), 'g');
    TEST_ASSERT_EQ(rankToChar(RankH), 'h');
    TEST_ASSERT_EQ(rankToChar(RankI), 'i');

    TEST_ASSERT_EQ(charToRank('a'), RankA);
    TEST_ASSERT_EQ(charToRank('b'), RankB);
    TEST_ASSERT_EQ(charToRank('c'), RankC);
    TEST_ASSERT_EQ(charToRank('d'), RankD);
    TEST_ASSERT_EQ(charToRank('e'), RankE);
    TEST_ASSERT_EQ(charToRank('f'), RankF);
    TEST_ASSERT_EQ(charToRank('g'), RankG);
    TEST_ASSERT_EQ(charToRank('h'), RankH);
    TEST_ASSERT_EQ(charToRank('i'), RankI);
}

TEST(Sfen, PieceToSfen) {
    using namespace nshogi::core;
    using namespace nshogi::io::sfen;

    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackPawn).c_str(), "P");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackLance).c_str(), "L");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackKnight).c_str(), "N");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackSilver).c_str(), "S");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackGold).c_str(), "G");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackKing).c_str(), "K");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackBishop).c_str(), "B");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackRook).c_str(), "R");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackProLance).c_str(), "+L");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackProKnight).c_str(), "+N");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackProSilver).c_str(), "+S");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackProBishop).c_str(), "+B");
    TEST_ASSERT_STREQ(pieceToSfen(PK_BlackProRook).c_str(), "+R");

    TEST_ASSERT_STREQ(pieceToSfen(PK_WhitePawn).c_str(), "p");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteLance).c_str(), "l");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteKnight).c_str(), "n");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteSilver).c_str(), "s");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteGold).c_str(), "g");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteKing).c_str(), "k");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteBishop).c_str(), "b");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteRook).c_str(), "r");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteProLance).c_str(), "+l");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteProKnight).c_str(), "+n");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteProSilver).c_str(), "+s");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteProBishop).c_str(), "+b");
    TEST_ASSERT_STREQ(pieceToSfen(PK_WhiteProRook).c_str(), "+r");
}

TEST(Sfen, InitialPosition) {
    nshogi::core::Position Position =
        nshogi::core::PositionBuilder::getInitialPosition();
    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(Position).c_str(),
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}

TEST(Sfen, SetGetSfenPosition) {
    // clang-format off
    const std::vector<std::string> Sfens({
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1",
        "lnsgk1snl/1r4gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BG4R1/LNS1KGSNL b - 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/P8/1PPPPPP1P/1BG3SR1/LNS1KG1NL w - 1",
        "ln1gk1snl/1rs3gb1/p1pppppp1/8p/7P1/P5P2/1PPPPP2P/1BG1K1SR1/LNS2G1NL w p 1",
        "ln1g2snl/1rs1k1gb1/p1pppp1p1/6p1p/9/P1P3PR1/1P1PPP2P/1BG1K1S2/LNS2G1NL w Pp 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/8P/PPPPPPP2/1BG3SR1/LNS1KG1NL w - 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BGK2SR1/LNS2G1NL w - 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/9/1p5P1/9/PPPPPPP1P/1BG1K1SR1/LNS2G1NL w - 1",
        "ln1gk1snl/1rs3gb1/p1ppppppp/7P1/1p7/9/PPPPPPP1P/1BG3SR1/LNS1KG1NL w - 1",
        "ln1gk1snl/2s3gb1/p1pppp2p/6pR1/9/1rP3P2/P2PPP2P/1BG1K4/LNS2GSNL w 2P2p 1",
        "ln1g2snl/2s1k1g2/2pppp2p/p5pp1/5N3/PrP3P2/B2PPP2P/1SGK2S+b1/LN3G2L b Pr2p 1",
        "ln4knl/1r2g1g2/p1s1ppsp1/b2P2p1p/1p6P/2S1SP3/PP2P1P2/2GK3R1/LN3G1NL b B3Pp 1",
        "ln1gk1snl/1rs3gb1/p1p1pppp1/3R4p/9/P5P2/1PPPPP2P/1BG1K1S2/LNS2G1NL w 2Pp 1",
        "ln1gk1s+Rl/2s3g2/p1pppp3/6p1p/1r7/PPP3P2/3PPP2P/1+b2K1S2/LNS2G1NL w BNPg2p 1",
        "ln1gk1snl/2s3gb1/p1pppp1pp/6p2/7P1/1rP1P4/P2P1PP1P/1BG2S1R1/LNSK1G1NL w Pp 1",
        "ln1g3n+B/1r1s2k2/p2ppp1pp/2p6/1p3s3/2P3P2/PPSPPP2P/3K2S2/LN1G1G2+b b GL2Prnl 1",
        "lnsgk1snl/6gb1/p1pppp2p/6R2/9/1rP6/P2PPPP1P/1BG6/LNS1KGSNL w 3P2p 1",
        "ln1gk1snl/1rs3gb1/p1p1pp1pp/1p1p2p2/7P1/2P6/PPSPPPP1P/1B5R1/LN1GKGSNL b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2P6/PPSPPPPPP/1B5R1/LN1GKGSNL w - 1",
        "ln1gk2nl/1r1s1sgb1/p1pppp1pp/1p4p2/9/2P4P1/PPSPPPP1P/1BG4R1/LN2KGSNL b - 1",
        "ln1gkg1nl/1r1s3b1/p1ppppspp/1p4p2/7P1/2P6/PPSPPPP1P/1BG4R1/LN2KGSNL w - 1",
        "lnsgk2nl/1r4gb1/p1ppppspp/1p4p2/7P1/2P6/PPSPPPP1P/1B5R1/LN1GKGSNL b - 1",
        "lnsgk2nl/1r4gs1/p1pppp1pp/6p2/1p7/2P4P1/PPSPPPP1P/2G4R1/LN2KGSNL b Bb 1",
        "lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p7/2P3PP1/PPBPPP2P/1S5R1/LN1GKGSNL w - 1",
        "lr5nl/3g1kg2/2n1p1sp1/p1pp2p1p/1p2sS1P1/P1PP2P1P/1PS1P1N2/2GK1G3/LN5RL b BPbp 1",
        "lr5nl/2g2kg2/2n1ppsp1/p1pps1p1p/1p7/P1PPSPPPP/1PS1P1N2/2G2G3/LNK4RL w Bb 1",
        "l2r3nl/2g2kg2/2n1ppsp1/p1p3p1p/1p1s3P1/P1P1SPP1P/1PS1P1N2/2G2G3/LNKR4L w BPbp 1",
        "lB5nl/2g2kg2/2n1ppsp1/p1p3p1p/1p1r3P1/P4PP1P/1PP1P1N2/2GS1G3/LNKR4L w SPbsp 1",
        "l2gk2nl/1r1s2g2/2npppsp1/p1p3p1p/1p5P1/P1P2PP1P/1PSPP1N2/2G3SR1/LN2KG2L w Bb 1",
        "l2gk2nl/1r1s2g2/2npppspp/p1p3p2/1p7/P1P2PPPP/1PSPP4/2GK2SR1/LN3G1NL w Bb 1",
        "ln5nl/4gkg2/p2pp1spp/5pp2/7P1/1rpP1PP2/P3PSN1P/2G1K1G2/LN5RL b BS2Pbsp 1",
        "ln1g3nl/1r3kg2/2psppsp1/p2p4p/1p3NpP1/P1P2P2P/1PSPP4/3K2SR1/LN1G1G2L w Bbp 1",
        "ln1g3n1/1r3kg2/2pspp1s1/p2p5/1p3NpRl/P1P2P3/1PSPP4/3K2S2/LN1G1G3 w B2Pbl3p 1",
        "ln1gk2nl/1r1s2g2/p2pppsp1/2p3p1p/1p5P1/P1P3P1P/1PSPPP3/5S1R1/LN1GKG1NL b Bb 1",
        "ln1g4l/1r2k1g2/6np1/p1pp1p2p/1p5b1/P1P2PP1P/1PSPPS3/2G2S3/LN2KG1PL w BS2Prn 1",
        "lnsgk3l/1r4gs1/p1ppppnp1/6p1p/1p5P1/2P3P1P/PPSPPP3/7R1/LN1GKGSNL b Bb 1",
        "l6nl/3g2kg1/4ppb2/p1PPs1ppp/1r3P3/P1p1S1P1P/1S2P4/1PG2G3/LNK4RL b BN3Psn 1",
        "l1S4nl/1r1g1kgs1/2n1p2p1/p2p2p1p/2p1Sp1P1/PPn2PP1P/3PP4/2GK1G1R1/LN6L b BPbsp 1",
        "lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p5P1/2PP5/PPB1PPP1P/7R1/LNSGKGSNL w - 1",
        "ln4knl/1r2gbg2/p1sp2spp/2p1ppp2/1p5PP/2PPPPP2/PP1S2N2/2GBGS1R1/LN2K3L w - 1",
        "ln2k2nl/1r1sg1gb1/p1pp1s1pp/4ppp2/1p7/2P1SPP2/PPBPP1NPP/2GS3R1/LN1K1G2L w - 1",
        "ln5nl/1r2gkg2/p1pps2pp/4ppp2/1p7/2P6/PPSPP1PPP/2G2R3/LNK2G1NL w BSPbs 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/6p2/1p7/2PP5/PPB1PPPPP/1R7/LNSGKGSNL w - 1",
        "lnsgkg1nl/7r1/1p1pppspp/2p3p2/p6P1/2P6/PP1PPPP1P/1SK1GS1R1/LN1G3NL b Bb 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/2P6/9/PP1PPPPPP/1BR6/LNSGKGSNL w - 1",
        "lnsgkg1nl/5srb1/ppppppppp/9/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2PP5/PP2PPPPP/1BR6/LNSGKGSNL w - 1",
        "lnsgkgsnl/6rb1/pppppp1pp/6p2/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/1r5b1/p1pppp1pp/1p4p2/9/2PP5/PP2PPPPP/1B1R5/LNSGKGSNL w - 1",
        "lnsgkgsnl/5r1b1/pppppp1pp/6p2/9/2P4P1/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "lnsgkgsnl/5r3/ppppp1bpp/5pp2/7P1/2P6/PP1PPPP1P/1B3S1R1/LNSGKG1NL b - 1",
        "lnsgkgsnl/4r2b1/pppp1p1pp/4p1p2/7P1/2P6/PP1PPPP1P/1B5R1/LNSGKGSNL b - 1",
        "ln1gkgsnl/1r1s3b1/p1pppp1pp/1p4p2/4P4/2P6/PP1P1PPPP/1B2R4/LNSGKGSNL w - 1",
        "lnsg1g1nl/3k1srb1/pppppp1pp/9/6p2/2PP5/PPBSPPPPP/1R7/LN1GKGSNL w - 1",
        "ln1g3nl/1r1kP2g1/3p1ps1p/p3p1p2/8P/P4P3/1P1P+b1PR1/2G1S4/LN2KG1NL w BS3Ps2p 1",
        "4k4/9/9/9/9/9/9/9/4K4 b 2R2B4G4S4N4L18P 1",
        "4k4/9/9/9/9/9/9/9/4K4 w 2r2b4g4s4n4l18p 1"
    });
    // clang-format on

    for (const auto& Sfen : Sfens) {
        nshogi::core::Position Position =
            nshogi::io::sfen::PositionBuilder::newPosition(Sfen);
        TEST_ASSERT_STREQ(
            nshogi::io::sfen::positionToSfen(Position).c_str(), Sfen.c_str());

        nshogi::core::State State =
            nshogi::io::sfen::StateBuilder::newState(Sfen);
        TEST_ASSERT_STREQ(nshogi::io::sfen::positionToSfen(
                                   State.getInitialPosition())
                                   .c_str(),
                               Sfen.c_str());
    }
}

TEST(State, InitialState) {
    nshogi::core::State State = nshogi::core::StateBuilder::getInitialState();
    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}

TEST(State, InvalidSfens) {
    const std::string SfenMoves[] = {"aaaa", "abcdef", "a", "2i2j", "B*3a+"};

    for (const auto& SfenMove : SfenMoves) {
        try {
            nshogi::core::State State =
                nshogi::core::StateBuilder::getInitialState();
            nshogi::io::sfen::sfenToMove32(State.getPosition(), SfenMove);
            TEST_ASSERT_FALSE(true);
        } catch ([[maybe_unused]] std::runtime_error& e) {
            TEST_ASSERT_TRUE(true);
        }
    }
}

TEST(State, MoveToSfen) {
    using namespace nshogi::core;
    using namespace nshogi::io::sfen;

    TEST_ASSERT_STREQ(
        "7g7f", move32ToSfen(Move32::boardMove(Sq7G, Sq7F, PTK_Empty)).c_str());
    TEST_ASSERT_STREQ(
        "2g2f", move32ToSfen(Move32::boardMove(Sq2G, Sq2F, PTK_Empty)).c_str());
    TEST_ASSERT_STREQ(
        "G*5e", move32ToSfen(Move32::droppingMove(Sq5E, PTK_Gold)).c_str());
    TEST_ASSERT_STREQ(
        "2c2b+", move32ToSfen(Move32::boardPromotingMove(Sq2C, Sq2B, PTK_Empty))
                     .c_str());
    TEST_ASSERT_STREQ("resign", move32ToSfen(Move32::MoveNone()).c_str());
}

TEST(State, SfenWithMoves) {
    const std::string Sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/"
                             "LNSGKGSNL b - 1 moves 7g7f 3c3d 8h2b+ 3a2b";
    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkg1nl/1r5s1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL b Bb 1");

    TEST_ASSERT_STREQ(nshogi::io::sfen::stateToSfen(State).c_str(),
                           Sfen.c_str());
}

TEST(State, SfenStartpos) {
    const std::string Sfen = "startpos";

    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}

TEST(State, SfenStartposWithMoves) {
    const std::string Sfen = "startpos moves 2g2f 8c8d 7g7f 8d8e 8h7g";

    nshogi::core::State State = nshogi::io::sfen::StateBuilder::newState(Sfen);

    TEST_ASSERT_STREQ(
        nshogi::io::sfen::positionToSfen(State.getPosition()).c_str(),
        "lnsgkgsnl/1r5b1/p1ppppppp/9/1p7/2P4P1/PPBPPPP1P/7R1/LNSGKGSNL w - 1");
}
