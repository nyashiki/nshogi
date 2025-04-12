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

#include "inttypes.h"

// Types.

typedef uint32_t nshogi_move_t;

typedef enum {
    NSHOGI_BLACK,
    NSHOGI_WHITE,
} nshogi_color_t;

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
} nshogi_feature_type_t;

typedef struct nshogi_state nshogi_state_t;
typedef struct nshogi_state_config nshogi_state_config_t;

// Api.

typedef struct nshogi_state_api {
    void (*destroyState)(nshogi_state_t* state);

    // Getter.
    nshogi_color_t (*getSideToMove)(nshogi_state_t* state);
    int (*getPly)(nshogi_state_t* state);
    nshogi_repetition_t (*getRepetition)(nshogi_state_t* state);
    int (*canDeclare)(nshogi_state_t* state);

    // Operator.
    int (*generateMoves)(nshogi_state_t* state, int wily_promote, nshogi_move_t* dest);
    void (*doMove)(nshogi_state_t* state, nshogi_move_t move);
    void (*undoMove)(nshogi_state_t* state);

    // StateConfig.
    nshogi_state_config_t* (*createStateConfig)(void);
    void (*destroyStateConfig)(nshogi_state_config_t* state_config);

    // Getter.
    uint16_t (*getMaxPly)(nshogi_state_config_t* state_config);
    float (*getBlackDrawValue)(nshogi_state_config_t* state_config);
    float (*getWhiteDrawValue)(nshogi_state_config_t* state_config);

    // Setter.
    void (*setMaxPly)(nshogi_state_config_t* state_config, uint16_t max_ply);
    void (*setBlackDrawValue)(nshogi_state_config_t* state_config, float value);
    void (*setWhiteDrawValue)(nshogi_state_config_t* state_config, float value);
} nshogi_state_api_t;

typedef struct nshogi_ml_api {
    void (*makeFeatureVector)(float* dest, nshogi_state_t*, nshogi_state_config_t*, nshogi_feature_type_t*, int num_features);
} nshogi_ml_api_t;

typedef struct nshogi_io_api {
    nshogi_state_t* (*createStateFromSfen)(char* sfen);
    char* (*moveToSfen)(nshogi_move_t move);
} nshogi_io_api_t;

typedef struct nshogi_api {
    nshogi_state_api_t* (*stateApi)(void);
    nshogi_ml_api_t* (*mlApi)(void);
    nshogi_io_api_t* (*ioApi)(void);

    // Utilities.
    int (*isDroppingPawn)(nshogi_move_t move);
    int (*moveToIndex)(nshogi_state_t* state, nshogi_move_t move);
    nshogi_move_t (*winDeclarationMove)(void);
} nshogi_api_t;

nshogi_api_t* nshogiApi(void);

#ifdef __cplusplus

} // extern "C"

#endif


#endif // #ifndef NSHOGI_C_API_H
