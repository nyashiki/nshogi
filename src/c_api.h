//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_C_API_H
#define NSHOGI_C_API_H

#ifdef __cplusplus

extern "C" {

#endif

#include <inttypes.h>

// Types.

typedef uint32_t nshogi_move_t;

typedef enum {
    NSHOGI_BLACK,
    NSHOGI_WHITE,
} nshogi_color_t;

typedef enum {
    NSHOGI_PTK_EMPTY = 0,
    NSHOGI_PTK_PAWN = 1,
    NSHOGI_PTK_LANCE = 2,
    NSHOGI_PTK_KNIGHT = 3,
    NSHOGI_PTK_SILVER = 4,
    NSHOGI_PTK_BISHOP = 5,
    NSHOGI_PTK_ROOK = 6,
    NSHOGI_PTK_GOLD = 7,
    NSHOGI_PTK_KING = 8,
    NSHOGI_PTK_PROPAWN = 9,
    NSHOGI_PTK_PROLANCE = 10,
    NSHOGI_PTK_PROKNIGHT = 11,
    NSHOGI_PTK_PROSILVER = 12,
    NSHOGI_PTK_PROBISHOP = 13,
    NSHOGI_PTK_PROROOK = 14,
} nshogi_piece_type_t;

typedef enum {
    NSHOGI_PK_EMPTY = 0,
    NSHOGI_PK_BLACKPAWN = 1,
    NSHOGI_PK_BLACKLANCE = 2,
    NSHOGI_PK_BLACKKNIGHT = 3,
    NSHOGI_PK_BLACKSILVER = 4,
    NSHOGI_PK_BLACKBISHOP = 5,
    NSHOGI_PK_BLACKROOK = 6,
    NSHOGI_PK_BLACKGOLD = 7,
    NSHOGI_PK_BLACKKING = 8,
    NSHOGI_PK_BLACKPROPAWN = 9,
    NSHOGI_PK_BLACKPROLANCE = 10,
    NSHOGI_PK_BLACKPROKNIGHT = 11,
    NSHOGI_PK_BLACKPROSILVER = 12,
    NSHOGI_PK_BLACKPROBISHOP = 13,
    NSHOGI_PK_BLACKPROROOK = 14,
    NSHOGI_PK_WHITEPAWN = 17,
    NSHOGI_PK_WHITELANCE = 18,
    NSHOGI_PK_WHITEKNIGHT = 19,
    NSHOGI_PK_WHITESILVER = 20,
    NSHOGI_PK_WHITEBISHOP = 21,
    NSHOGI_PK_WHITEROOK = 22,
    NSHOGI_PK_WHITEGOLD = 23,
    NSHOGI_PK_WHITEKING = 24,
    NSHOGI_PK_WHITEPROPAWN = 25,
    NSHOGI_PK_WHITEPROLANCE = 26,
    NSHOGI_PK_WHITEPROKNIGHT = 27,
    NSHOGI_PK_WHITEPROSILVER = 28,
    NSHOGI_PK_WHITEPROBISHOP = 29,
    NSHOGI_PK_WHITEPROROOK = 30,
} nshogi_piece_t;

typedef enum {
    NSHOGI_SQUARE_9A = 80,
    NSHOGI_SQUARE_8A = 71,
    NSHOGI_SQUARE_7A = 62,
    NSHOGI_SQUARE_6A = 53,
    NSHOGI_SQUARE_5A = 44,
    NSHOGI_SQUARE_4A = 35,
    NSHOGI_SQUARE_3A = 26,
    NSHOGI_SQUARE_2A = 17,
    NSHOGI_SQUARE_1A = 8,
    NSHOGI_SQUARE_9B = 79,
    NSHOGI_SQUARE_8B = 70,
    NSHOGI_SQUARE_7B = 61,
    NSHOGI_SQUARE_6B = 52,
    NSHOGI_SQUARE_5B = 43,
    NSHOGI_SQUARE_4B = 34,
    NSHOGI_SQUARE_3B = 25,
    NSHOGI_SQUARE_2B = 16,
    NSHOGI_SQUARE_1B = 7,
    NSHOGI_SQUARE_9C = 78,
    NSHOGI_SQUARE_8C = 69,
    NSHOGI_SQUARE_7C = 60,
    NSHOGI_SQUARE_6C = 51,
    NSHOGI_SQUARE_5C = 42,
    NSHOGI_SQUARE_4C = 33,
    NSHOGI_SQUARE_3C = 24,
    NSHOGI_SQUARE_2C = 15,
    NSHOGI_SQUARE_1C = 6,
    NSHOGI_SQUARE_9D = 77,
    NSHOGI_SQUARE_8D = 68,
    NSHOGI_SQUARE_7D = 59,
    NSHOGI_SQUARE_6D = 50,
    NSHOGI_SQUARE_5D = 41,
    NSHOGI_SQUARE_4D = 32,
    NSHOGI_SQUARE_3D = 23,
    NSHOGI_SQUARE_2D = 14,
    NSHOGI_SQUARE_1D = 5,
    NSHOGI_SQUARE_9E = 76,
    NSHOGI_SQUARE_8E = 67,
    NSHOGI_SQUARE_7E = 58,
    NSHOGI_SQUARE_6E = 49,
    NSHOGI_SQUARE_5E = 40,
    NSHOGI_SQUARE_4E = 31,
    NSHOGI_SQUARE_3E = 22,
    NSHOGI_SQUARE_2E = 13,
    NSHOGI_SQUARE_1E = 4,
    NSHOGI_SQUARE_9F = 75,
    NSHOGI_SQUARE_8F = 66,
    NSHOGI_SQUARE_7F = 57,
    NSHOGI_SQUARE_6F = 48,
    NSHOGI_SQUARE_5F = 39,
    NSHOGI_SQUARE_4F = 30,
    NSHOGI_SQUARE_3F = 21,
    NSHOGI_SQUARE_2F = 12,
    NSHOGI_SQUARE_1F = 3,
    NSHOGI_SQUARE_9G = 74,
    NSHOGI_SQUARE_8G = 65,
    NSHOGI_SQUARE_7G = 56,
    NSHOGI_SQUARE_6G = 47,
    NSHOGI_SQUARE_5G = 38,
    NSHOGI_SQUARE_4G = 29,
    NSHOGI_SQUARE_3G = 20,
    NSHOGI_SQUARE_2G = 11,
    NSHOGI_SQUARE_1G = 2,
    NSHOGI_SQUARE_9H = 73,
    NSHOGI_SQUARE_8H = 64,
    NSHOGI_SQUARE_7H = 55,
    NSHOGI_SQUARE_6H = 46,
    NSHOGI_SQUARE_5H = 37,
    NSHOGI_SQUARE_4H = 28,
    NSHOGI_SQUARE_3H = 19,
    NSHOGI_SQUARE_2H = 10,
    NSHOGI_SQUARE_1H = 1,
    NSHOGI_SQUARE_9I = 72,
    NSHOGI_SQUARE_8I = 63,
    NSHOGI_SQUARE_7I = 54,
    NSHOGI_SQUARE_6I = 45,
    NSHOGI_SQUARE_5I = 36,
    NSHOGI_SQUARE_4I = 27,
    NSHOGI_SQUARE_3I = 18,
    NSHOGI_SQUARE_2I = 9,
    NSHOGI_SQUARE_1I = 0,
} nshogi_square_t;

typedef enum {
    NSHOGI_NO_REPETITION,
    NSHOGI_REPETITION,
    NSHOGI_WIN_REPETITION,
    NSHOGI_LOSS_REPETITION,
    NSHOGI_SUPERIOR_REPETITION,
    NSHOGI_INFERIOR_REPETITION,
} nshogi_repetition_t;

typedef enum {
    NSHOGI_FT_BLACK,
    NSHOGI_FT_WHITE,
    NSHOGI_FT_MYPAWN,
    NSHOGI_FT_MYLANCE,
    NSHOGI_FT_MYKNIGHT,
    NSHOGI_FT_MYSILVER,
    NSHOGI_FT_MYGOLD,
    NSHOGI_FT_MYKING,
    NSHOGI_FT_MYBISHOP,
    NSHOGI_FT_MYROOK,
    NSHOGI_FT_MYPROPAWN,
    NSHOGI_FT_MYPROLANCE,
    NSHOGI_FT_MYPROKNIGHT,
    NSHOGI_FT_MYPROSILVER,
    NSHOGI_FT_MYPROBISHOP,
    NSHOGI_FT_MYPROROOK,
    NSHOGI_FT_MYBISHOP_AND_PROBISHOP,
    NSHOGI_FT_MYROOK_AND_PROROOK,
    NSHOGI_FT_OPPAWN,
    NSHOGI_FT_OPLANCE,
    NSHOGI_FT_OPKNIGHT,
    NSHOGI_FT_OPSILVER,
    NSHOGI_FT_OPGOLD,
    NSHOGI_FT_OPKING,
    NSHOGI_FT_OPBISHOP,
    NSHOGI_FT_OPROOK,
    NSHOGI_FT_OPPROPAWN,
    NSHOGI_FT_OPPROLANCE,
    NSHOGI_FT_OPPROKNIGHT,
    NSHOGI_FT_OPPROSILVER,
    NSHOGI_FT_OPPROBISHOP,
    NSHOGI_FT_OPPROROOK,
    NSHOGI_FT_OPBISHOP_AND_PROBISHOP,
    NSHOGI_FT_OPROOK_AND_PROROOK,
    NSHOGI_FT_MYSTANDPAWN1,
    NSHOGI_FT_MYSTANDPAWN2,
    NSHOGI_FT_MYSTANDPAWN3,
    NSHOGI_FT_MYSTANDPAWN4,
    NSHOGI_FT_MYSTANDPAWN5,
    NSHOGI_FT_MYSTANDPAWN6,
    NSHOGI_FT_MYSTANDPAWN7,
    NSHOGI_FT_MYSTANDPAWN8,
    NSHOGI_FT_MYSTANDPAWN9,
    NSHOGI_FT_MYSTANDLANCE1,
    NSHOGI_FT_MYSTANDLANCE2,
    NSHOGI_FT_MYSTANDLANCE3,
    NSHOGI_FT_MYSTANDLANCE4,
    NSHOGI_FT_MYSTANDKNIGHT1,
    NSHOGI_FT_MYSTANDKNIGHT2,
    NSHOGI_FT_MYSTANDKNIGHT3,
    NSHOGI_FT_MYSTANDKNIGHT4,
    NSHOGI_FT_MYSTANDSILVER1,
    NSHOGI_FT_MYSTANDSILVER2,
    NSHOGI_FT_MYSTANDSILVER3,
    NSHOGI_FT_MYSTANDSILVER4,
    NSHOGI_FT_MYSTANDGOLD1,
    NSHOGI_FT_MYSTANDGOLD2,
    NSHOGI_FT_MYSTANDGOLD3,
    NSHOGI_FT_MYSTANDGOLD4,
    NSHOGI_FT_MYSTANDBISHOP1,
    NSHOGI_FT_MYSTANDBISHOP2,
    NSHOGI_FT_MYSTANDROOK1,
    NSHOGI_FT_MYSTANDROOK2,
    NSHOGI_FT_OPSTANDPAWN1,
    NSHOGI_FT_OPSTANDPAWN2,
    NSHOGI_FT_OPSTANDPAWN3,
    NSHOGI_FT_OPSTANDPAWN4,
    NSHOGI_FT_OPSTANDPAWN5,
    NSHOGI_FT_OPSTANDPAWN6,
    NSHOGI_FT_OPSTANDPAWN7,
    NSHOGI_FT_OPSTANDPAWN8,
    NSHOGI_FT_OPSTANDPAWN9,
    NSHOGI_FT_OPSTANDLANCE1,
    NSHOGI_FT_OPSTANDLANCE2,
    NSHOGI_FT_OPSTANDLANCE3,
    NSHOGI_FT_OPSTANDLANCE4,
    NSHOGI_FT_OPSTANDKNIGHT1,
    NSHOGI_FT_OPSTANDKNIGHT2,
    NSHOGI_FT_OPSTANDKNIGHT3,
    NSHOGI_FT_OPSTANDKNIGHT4,
    NSHOGI_FT_OPSTANDSILVER1,
    NSHOGI_FT_OPSTANDSILVER2,
    NSHOGI_FT_OPSTANDSILVER3,
    NSHOGI_FT_OPSTANDSILVER4,
    NSHOGI_FT_OPSTANDGOLD1,
    NSHOGI_FT_OPSTANDGOLD2,
    NSHOGI_FT_OPSTANDGOLD3,
    NSHOGI_FT_OPSTANDGOLD4,
    NSHOGI_FT_OPSTANDBISHOP1,
    NSHOGI_FT_OPSTANDBISHOP2,
    NSHOGI_FT_OPSTANDROOK1,
    NSHOGI_FT_OPSTANDROOK2,
    NSHOGI_FT_CHECK,
    NSHOGI_FT_NOMYPAWNFILE,
    NSHOGI_FT_NOOPPAWNFILE,
    NSHOGI_FT_PROGRESS,
    NSHOGI_FT_PROGRESSUNIT,
    NSHOGI_FT_RULEDECLARE27,
    NSHOGI_FT_RULEDRAW24,
    NSHOGI_FT_RULETRYING,
    NSHOGI_FT_MYDRAWVALUE,
    NSHOGI_FT_OPDRAWVALUE,
    NSHOGI_FT_MYDECLARATIONSCORE,
    NSHOGI_FT_OPDECLARATIONSCORE,
    NSHOGI_FT_MYPIECESCORE,
    NSHOGI_FT_OPPIECESCORE,
    NSHOGI_FT_MYATTACK,
    NSHOGI_FT_OPATTACK,
} nshogi_feature_type_t;

typedef struct nshogi_position nshogi_position_t;
typedef struct nshogi_state nshogi_state_t;
typedef struct nshogi_state_config nshogi_state_config_t;

// Api.

typedef struct nshogi_move_api {
    int8_t (*to)(nshogi_move_t);
    int8_t (*from)(nshogi_move_t);
    int (*drop)(nshogi_move_t);
    int (*promote)(nshogi_move_t);
    uint8_t (*pieceType)(nshogi_move_t);
    uint8_t (*capturePieceType)(nshogi_move_t);
    int (*isNone)(nshogi_move_t move);
    int (*isWin)(nshogi_move_t move);

    // Constants.
    nshogi_move_t (*noneMove)(void);
    nshogi_move_t (*winMove)(void);
} nshogi_move_api_t;

typedef struct nshogi_position_api {
    nshogi_color_t (*getSideToMove)(const nshogi_position_t* position);
    nshogi_piece_t (*pieceOn)(const nshogi_position_t* position,
                              nshogi_square_t square);
    uint8_t (*getStandCount)(const nshogi_position_t* position,
                             nshogi_color_t color,
                             nshogi_piece_type_t piece_type);
} nshogi_position_api_t;

typedef struct nshogi_state_api {
    void (*destroyState)(nshogi_state_t* state);

    nshogi_state_t* (*clone)(const nshogi_state_t* state);

    // Getter.
    nshogi_color_t (*getSideToMove)(const nshogi_state_t* state);
    int (*getPly)(const nshogi_state_t* state);
    const nshogi_position_t* (*getPosition)(const nshogi_state_t* state);
    const nshogi_position_t* (*getInitialPosition)(const nshogi_state_t* state);
    nshogi_repetition_t (*getRepetition)(const nshogi_state_t* state);
    int (*canDeclare)(const nshogi_state_t* state);
    nshogi_move_t (*getHistoryMove)(const nshogi_state_t* state, uint16_t ply);
    nshogi_move_t (*getLastMove)(const nshogi_state_t* state);
    uint64_t (*getHash)(const nshogi_state_t* state);
    int (*isInCheck)(const nshogi_state_t* state);

    // Operator.
    int (*generateMoves)(const nshogi_state_t* state, int wily_promote,
                         nshogi_move_t* dest);
    void (*doMove)(nshogi_state_t* state, nshogi_move_t move);
    void (*undoMove)(nshogi_state_t* state);

    // StateConfig.
    nshogi_state_config_t* (*createStateConfig)(void);
    void (*destroyStateConfig)(nshogi_state_config_t* state_config);

    // Getter.
    uint16_t (*getMaxPly)(const nshogi_state_config_t* state_config);
    float (*getBlackDrawValue)(const nshogi_state_config_t* state_config);
    float (*getWhiteDrawValue)(const nshogi_state_config_t* state_config);

    // Setter.
    void (*setMaxPly)(nshogi_state_config_t* state_config, uint16_t max_ply);
    void (*setBlackDrawValue)(nshogi_state_config_t* state_config, float value);
    void (*setWhiteDrawValue)(nshogi_state_config_t* state_config, float value);
} nshogi_state_api_t;

typedef struct nshogi_solver_dfpn nshogi_solver_dfpn_t;

typedef struct nshogi_solver_api {
    nshogi_move_t (*dfs)(nshogi_state_t*, int depth);

    nshogi_solver_dfpn_t* (*createDfPnSolver)(uint64_t memory_mb);
    void (*destroyDfPnSolver)(nshogi_solver_dfpn_t*);
    nshogi_move_t (*solveByDfPn)(nshogi_state_t*, nshogi_solver_dfpn_t*,
                                 uint64_t max_node_count, int max_depth);
} nshogi_solver_api_t;

typedef struct nshogi_ml_api {
    void (*makeFeatureVector)(float* dest, const nshogi_state_t*,
                              const nshogi_state_config_t*,
                              const nshogi_feature_type_t*, int num_features);
    void (*makeFeatureVectorWithOption)(float* dest, const nshogi_state_t*,
                              const nshogi_state_config_t*,
                              const nshogi_feature_type_t*,
                              int num_features,
                              int channels_first);
    int (*moveToIndex)(const nshogi_state_t* state, nshogi_move_t move);
    int (*moveToIndexWithOption)(const nshogi_state_t* state, nshogi_move_t move,
                                 int channels_first);
} nshogi_ml_api_t;

typedef struct nshogi_io_api {
    // Sfen.
    int (*canCreateStateFromSfen)(const char* sfen);
    int (*canCreateMoveFromSfen)(const nshogi_state_t* state, const char* sfen);
    nshogi_state_t* (*createStateFromSfen)(const char* sfen);
    nshogi_move_t (*createMoveFromSfen)(const nshogi_state_t* state,
                                        const char* sfen);
    int32_t (*moveToSfen)(char* dest, nshogi_move_t move);
    int32_t (*stateToSfen)(char* dest, int32_t max_length,
                           const nshogi_state_t* state);
    int32_t (*positionToSfen)(char* dest, int32_t max_length,
                              const nshogi_position_t* position);

    // CSA.
    int (*canCreateStateFromCSA)(const char* csa);
    int (*canCreateMoveFromCSA)(const nshogi_state_t* state, const char* csa);
    nshogi_state_t* (*createStateFromCSA)(const char* csa);
    nshogi_move_t (*createMoveFromCSA)(const nshogi_state_t* state,
                                       const char* csa);
    int32_t (*moveToCSA)(char* dest, const nshogi_state_t* state,
                         nshogi_move_t move);
    int32_t (*stateToCSA)(char* dest, int32_t max_length,
                          const nshogi_state_t* state);
    int32_t (*positionToCSA)(char* dest, int32_t max_length,
                             const nshogi_position_t* state);
} nshogi_io_api_t;

typedef struct nshogi_api {
    nshogi_move_api_t* (*moveApi)(void);
    nshogi_position_api_t* (*positionApi)(void);
    nshogi_state_api_t* (*stateApi)(void);
    nshogi_solver_api_t* (*solverApi)(void);
    nshogi_ml_api_t* (*mlApi)(void);
    nshogi_io_api_t* (*ioApi)(void);
} nshogi_api_t;

nshogi_api_t* nshogiApi(void);

#ifdef __cplusplus

} // extern "C"

#endif

#endif // #ifndef NSHOGI_C_API_H
