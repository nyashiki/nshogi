#include "../core/types.h"

extern "C" {

#include "types.h"

PieceTypeKind nshogi_get_piece_type_from_piece_kind(PieceKind PK) {
    return (PieceTypeKind)nshogi::core::getPieceType((nshogi::core::PieceKind)PK);
}

Color nshogi_get_color_from_piece_kind(PieceKind P) {
    return (Color)nshogi::core::getColor((nshogi::core::PieceKind)P);
}

void nshogi_extract_move(
        Move32 M,
        int* To,
        int* From,
        int* IsPromote,
        PieceTypeKind* Kind,
        PieceTypeKind* Capture) {
    nshogi::core::Move32 Move = nshogi::core::Move32::fromValue(M);

    *To = Move.to();
    *From = Move.from();
    *IsPromote = Move.promote();
    *Kind = (PieceTypeKind)Move.pieceType();
    *Capture = (PieceTypeKind)Move.capturePieceType();
}

} // extern "C"
