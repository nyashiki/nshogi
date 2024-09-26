#ifndef NSHOGI_CAPI_TYPES_H
#define NSHOGI_CAPI_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef void State;
typedef uint32_t Move32;

typedef enum : uint8_t {
    BLACK = 0, WHITE = 1,
} Color;

typedef enum : uint8_t {
    PTK_EMPTY   = 0,
    PTK_PAWN    = 1,     PTK_LANCE = 2,     PTK_KNIGHT = 3,    PTK_SILVER =  4,     PTK_BISHOP = 5,     PTK_ROOK = 6, PTK_GOLD = 7, PTK_KING = 8,
    PTK_PROPAWN = 9, PTK_PROLANCE = 10, PTK_PROKNIGHT = 11, PTK_PROSILVER = 12, PTK_PROBISHOP = 13, PTK_PROROOK = 14,
} PieceTypeKind;

typedef enum : uint16_t {
    PK_EMPTY = 0,
    PK_BLACKPAWN    =  1, PK_BLACKLANCE    =  2, PK_BLACKKNIGHT    =  3, PK_BLACKSILVER    =  4, PK_BLACKBISHOP    =  5, PK_BLACKROOK    =  6, PK_BLACKGOLD =  7, PK_BLACKKING =  8,
    PK_BLACKPROPAWN =  9, PK_BLACKPROLANCE = 10, PK_BLACKPROKNIGHT = 11, PK_BLACKPROSILVER = 12, PK_BLACKPROBISHOP = 13, PK_BLACKPROROOK = 14,
    PK_WHITEPAWN    = 17, PK_WHITELANCE    = 18, PK_WHITEKNIGHT    = 19, PK_WHITESILVER    = 20, PK_WHITEBISHOP    = 21, PK_WHITEROOK    = 22, PK_WHITEGOLD = 23, PK_WHITEKING = 24,
    PK_WHITEPROPAWN = 25, PK_WHITEPROLANCE = 26, PK_WHITEPROKNIGHT = 27, PK_WHITEPROSILVER = 28, PK_WHITEPROBISHOP = 29, PK_WHITEPROROOK = 30,
} PieceKind;

PieceTypeKind nshogi_get_piece_type_from_piece_kind(PieceKind piece_kind);
Color nshogi_get_color_from_piece_kind(PieceKind p);
void nshogi_extract_move(Move32 move, int* to, int* from, int* is_promote, PieceTypeKind* kind, PieceTypeKind* capture);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef NSHOGI_CAPI_TYPES_H
