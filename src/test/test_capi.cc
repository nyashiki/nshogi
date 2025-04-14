//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "common.h"
#include "../c_api.h"
#include "../core/types.h"
#include "../ml/common.h"
#include "../io/sfen.h"

#include <random>
#include <cstring>

TEST(CAPI, InitialPosition) {
    const char* Sfen = "startpos";
    nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen(Sfen);
    const nshogi_position_t* Position = nshogiApi()->stateApi()->getPosition(State);

    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getSideToMove(Position),
            NSHOGI_BLACK);

   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1A),
           NSHOGI_PK_WHITELANCE);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_1I),
           NSHOGI_PK_BLACKLANCE);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2A),
           NSHOGI_PK_WHITEKNIGHT);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2B),
           NSHOGI_PK_WHITEBISHOP);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2H),
           NSHOGI_PK_BLACKROOK);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_2I),
           NSHOGI_PK_BLACKKNIGHT);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3A),
           NSHOGI_PK_WHITESILVER);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_3I),
           NSHOGI_PK_BLACKSILVER);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4A),
           NSHOGI_PK_WHITEGOLD);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_4I),
           NSHOGI_PK_BLACKGOLD);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5A),
           NSHOGI_PK_WHITEKING);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_5I),
           NSHOGI_PK_BLACKKING);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6A),
           NSHOGI_PK_WHITEGOLD);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_6I),
           NSHOGI_PK_BLACKGOLD);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7A),
           NSHOGI_PK_WHITESILVER);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_7I),
           NSHOGI_PK_BLACKSILVER);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8A),
           NSHOGI_PK_WHITEKNIGHT);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8B),
           NSHOGI_PK_WHITEROOK);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8H),
           NSHOGI_PK_BLACKBISHOP);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_8I),
           NSHOGI_PK_BLACKKNIGHT);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9A),
           NSHOGI_PK_WHITELANCE);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9B),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9C),
           NSHOGI_PK_WHITEPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9D),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9E),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9F),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9G),
           NSHOGI_PK_BLACKPAWN);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9H),
           NSHOGI_PK_EMPTY);
   TEST_ASSERT_EQ(
           nshogiApi()->positionApi()->pieceOn(Position, NSHOGI_SQUARE_9I),
           NSHOGI_PK_BLACKLANCE);

    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_PAWN),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_LANCE),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_KNIGHT),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_SILVER),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_GOLD),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_BISHOP),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_BLACK, NSHOGI_PTK_ROOK),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_PAWN),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_LANCE),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_KNIGHT),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_SILVER),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_GOLD),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_BISHOP),
            0);
    TEST_ASSERT_EQ(
            nshogiApi()->positionApi()->getStandCount(Position, NSHOGI_WHITE, NSHOGI_PTK_ROOK),
            0);

    nshogiApi()->stateApi()->destroyState(State);
}

TEST(CAPI, InitialState) {
    const char* Sfen = "startpos";
    nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen(Sfen);

    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getSideToMove(State), NSHOGI_BLACK);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getPly(State), 0);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->getRepetition(State), NSHOGI_NO_REPETITION);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->canDeclare(State), 0);
    TEST_ASSERT_EQ(nshogiApi()->stateApi()->isInCheck(State), 0);

    nshogi_move_t Moves[600];
    int MoveCount = nshogiApi()->stateApi()->generateMoves(State, 1, Moves);
    TEST_ASSERT_EQ(MoveCount, 30);

    int MoveCountNoWilyPromote = nshogiApi()->stateApi()->generateMoves(State, 0, Moves);
    TEST_ASSERT_EQ(MoveCountNoWilyPromote, 30);

    TEST_ASSERT_EQ(
            nshogiApi()->stateApi()->getHash(State),
            nshogi::io::sfen::StateBuilder::newState("startpos").getHash());
}

TEST(CAPI, DoAndUndo) {
    const int N = 10;

    std::mt19937_64 Mt(20250413);
    nshogi_move_t Moves[600];
    nshogi_move_t MoveHistory[1024];

    for (int I = 0; I < N; ++I) {
        nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen("startpos");
        char* InitialPositionSfen = nshogiApi()->ioApi()->positionToSfen(
                nshogiApi()->stateApi()->getPosition(State));

        for (int Ply = 0; Ply < 1024; ++Ply) {
            int MoveCount = nshogiApi()->stateApi()->generateMoves(State, 1, Moves);

            for (int J = 0; J < Ply; ++J) {
                TEST_ASSERT_EQ(nshogiApi()->stateApi()->getHistoryMove(State, (uint16_t)J), MoveHistory[J]);
            }

            if (MoveCount == 0) {
                break;
            }

            char* Sfen = nshogiApi()->ioApi()->stateToSfen(State);

            for (int J = 0; J < MoveCount; ++J) {
                nshogi_move_t M = Moves[(std::size_t)J];

                char* SfenMove = nshogiApi()->ioApi()->moveToSfen(M);
                nshogi_move_t M_ = nshogiApi()->ioApi()->createMoveFromSfen(State, SfenMove);
                TEST_ASSERT_EQ(M, M_);

                if (std::strncmp(SfenMove, "P*", 2) == 0) {
                    TEST_ASSERT_TRUE(nshogiApi()->isDroppingPawn(M));
                } else {
                    TEST_ASSERT_FALSE(nshogiApi()->isDroppingPawn(M));
                }

                free(SfenMove);

                nshogiApi()->stateApi()->doMove(State, M);

                TEST_ASSERT_EQ(nshogiApi()->stateApi()->getLastMove(State), M);

                nshogiApi()->stateApi()->undoMove(State);

                char* DoAndUndoSfen = nshogiApi()->ioApi()->stateToSfen(State);
                TEST_ASSERT_EQ(std::strcmp(Sfen, DoAndUndoSfen), 0);
                free(DoAndUndoSfen);

                char* InitialPositionSfenCurrent =
                    nshogiApi()->ioApi()->positionToSfen(nshogiApi()->stateApi()->getInitialPosition(State));

                TEST_ASSERT_EQ(std::strcmp(InitialPositionSfen, InitialPositionSfenCurrent), 0);

                free(InitialPositionSfenCurrent);

                TEST_ASSERT_EQ(
                    (std::size_t)nshogiApi()->mlApi()->moveToIndex(State, M),
                    nshogi::ml::getMoveIndex(
                        static_cast<nshogi::core::Color>(
                            nshogiApi()->stateApi()->getSideToMove(State)),
                        nshogi::core::Move32::fromValue(M)));
                TEST_ASSERT_EQ(
                    (std::size_t)nshogiApi()->mlApi()->moveToIndex(State, M),
                    nshogi::ml::getMoveIndex(
                        static_cast<nshogi::core::Color>(
                            nshogiApi()->stateApi()->getSideToMove(State)),
                        nshogi::core::Move16(
                            nshogi::core::Move32::fromValue(M))));
            }

            free(Sfen);

            nshogi_move_t RandomMove = Moves[(std::size_t)((std::size_t)Mt() % (std::size_t)MoveCount)];
            nshogiApi()->stateApi()->doMove(State, RandomMove);

            MoveHistory[(std::size_t)Ply] = RandomMove;
        }

        free(InitialPositionSfen);
        nshogiApi()->stateApi()->destroyState(State);
    }
}

TEST(CAPI, Clone) {
    const int N = 100;

    std::mt19937_64 Mt(20250413);
    nshogi_move_t Moves[600];

    for (int I = 0; I < N; ++I) {
        nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen("startpos");

        for (int Ply = 0; Ply < 1024; ++Ply) {
            int MoveCount = nshogiApi()->stateApi()->generateMoves(State, 1, Moves);

            if (MoveCount == 0) {
                break;
            }

            nshogi_move_t RandomMove = Moves[(std::size_t)((std::size_t)Mt() % (std::size_t)MoveCount)];
            nshogiApi()->stateApi()->doMove(State, RandomMove);

            nshogi_state_t* ClonedState = nshogiApi()->stateApi()->clone(State);

            char* Sfen = nshogiApi()->ioApi()->stateToSfen(State);
            char* ClonedSfen = nshogiApi()->ioApi()->stateToSfen(ClonedState);

            TEST_ASSERT_EQ(std::strcmp(Sfen, ClonedSfen), 0);

            free(Sfen);
            free(ClonedSfen);
            nshogiApi()->stateApi()->destroyState(ClonedState);
        }

        nshogiApi()->stateApi()->destroyState(State);
    }
}

TEST(CAPI, StateConfig) {
    nshogi_state_config_t* StateConfig = nshogiApi()->stateApi()->createStateConfig();

    for (uint16_t MaxPly = 1; MaxPly < 65535; ++MaxPly) {
        for (float BlackDrawValue = 0.0f; BlackDrawValue <= 1.0f; BlackDrawValue += 0.1f) {
            for (float WhiteDrawValue = 0.0f; WhiteDrawValue <= 1.0f; WhiteDrawValue += 0.1f) {
                nshogiApi()->stateApi()->setMaxPly(StateConfig, MaxPly);
                nshogiApi()->stateApi()->setBlackDrawValue(StateConfig, BlackDrawValue);
                nshogiApi()->stateApi()->setWhiteDrawValue(StateConfig, WhiteDrawValue);

                TEST_ASSERT_EQ(MaxPly, nshogiApi()->stateApi()->getMaxPly(StateConfig));
                TEST_ASSERT_FLOAT_EQ(
                        BlackDrawValue,
                        nshogiApi()->stateApi()->getBlackDrawValue(StateConfig),
                        1e-6);
                TEST_ASSERT_FLOAT_EQ(
                        WhiteDrawValue,
                        nshogiApi()->stateApi()->getWhiteDrawValue(StateConfig),
                        1e-6);
            }
        }
    }

    nshogiApi()->stateApi()->destroyStateConfig(StateConfig);
}

TEST(CAPI, WinDeclarationMove) {
    nshogi_move_t WinMove = nshogiApi()->winDeclarationMove();

    TEST_ASSERT_EQ(static_cast<uint32_t>(WinMove), nshogi::core::Move32::MoveWin().value());
}

TEST(CAPI, MLFeatureVector) {
    float* Dest = static_cast<float*>(malloc(4 * 9 * 9 * sizeof(float)));

    nshogi_feature_type_t Features[] = {
        NSHOGI_FT_BLACK,
        NSHOGI_FT_WHITE,
        NSHOGI_FT_MYPAWN,
        NSHOGI_FT_OPPAWN,
    };

    int NumFeatures = sizeof(Features) / sizeof(Features[0]);

    nshogi_state_t* State = nshogiApi()->ioApi()->createStateFromSfen("startpos");
    nshogi_state_config_t* StateConfig = nshogiApi()->stateApi()->createStateConfig();

    nshogiApi()->mlApi()->makeFeatureVector(Dest, State, StateConfig, Features, NumFeatures);

    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)Square], 1.0f, 1e-6);
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(1 * 81 + Square)], 0.0f, 1e-6);
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        nshogi_piece_t Piece = nshogiApi()->positionApi()->pieceOn(
                nshogiApi()->stateApi()->getPosition(State), Square);
        if (Piece == NSHOGI_PK_BLACKPAWN) {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(2 * 81 + Square)], 1.0f, 1e-6);
        } else {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(2 * 81 + Square)], 0.0f, 1e-6);
        }
    }
    for (nshogi_square_t Square = (nshogi_square_t)0;
         Square <= (nshogi_square_t)80;
         Square = (nshogi_square_t)((int)Square + 1)) {
        nshogi_piece_t Piece = nshogiApi()->positionApi()->pieceOn(
                nshogiApi()->stateApi()->getPosition(State), Square);
        if (Piece == NSHOGI_PK_WHITEPAWN) {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(3 * 81 + Square)], 1.0f, 1e-6);
        } else {
            TEST_ASSERT_FLOAT_EQ(Dest[(std::size_t)(3 * 81 + Square)], 0.0f, 1e-6);
        }
    }

    nshogiApi()->stateApi()->destroyStateConfig(StateConfig);
    nshogiApi()->stateApi()->destroyState(State);

    free(Dest);
}
