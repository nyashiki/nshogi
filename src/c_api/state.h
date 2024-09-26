#ifndef NSHOGI_CAPI_STATE_H
#define NSHOGI_CAPI_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum : uint8_t {
    NO_REPETITION,
    REPETITION,
    WIN_REPETITION,
    LOSS_REPETITION,
    SUPERIOR_REPETITION,
    INFERIOR_REPETITION,
} RepetitionStatus;

Color nshogi_state_get_side_to_move(const State* state);
uint16_t nshogi_state_get_ply(const State* state);
PieceKind nshogi_state_get_piece_on_square(const State* state, int square);
uint8_t nshogi_state_get_stand_count(const State* state, Color color, PieceTypeKind kind);
RepetitionStatus nshogi_state_get_repetition_status(const State* state);
int8_t nshogi_state_can_declare(const State* state);

void nshogi_state_do_move(State* state, Move32 move);
void nshogi_state_undo_move(State* state, Move32* last_move);

void nshogi_free_state(State** state);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef NSHOGI_CAPI_STATE_H
