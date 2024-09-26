#ifndef NSHOGI_CAPI_MOVEGENERATOR_H
#define NSHOGI_CAPI_MOVEGENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "state.h"

uint16_t nshogi_generate_legal_moves(const State* state, Move32* moves);
uint16_t nshogi_generate_legal_moves_pruned(const State* state, Move32* moves);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef NSHOGI_CAPI_MOVEGENERATOR_H
