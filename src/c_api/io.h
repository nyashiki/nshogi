#ifndef NSHOGI_CAPI_IO_H
#define NSHOGI_CAPI_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void nshogi_io_malloc_state_from_sfen(State** state, const char* sfen);
uint32_t nshogi_io_get_sfen_from_state(const State* state, char* sfen);
uint32_t nshogi_io_get_sfen_from_move(Move32 move, char* sfen);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef NSHOGI_CAPI_IO_H
