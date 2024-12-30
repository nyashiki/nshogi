#include "hash.h"
#include <cstdint>
#include <random>


namespace nshogi {
namespace core {
namespace internal {

template<>
Hash<uint64_t>::Hash() {
    static std::ranlux48 Rng(20230622);

    if (!IsTableGenerated) {
        IsTableGenerated = true;

        for (Color C : Colors) {
            for (PieceTypeKind Type : PieceTypes) {
                for (Square Sq : Squares) {
                    OnBoardHash[C][Type][Sq] = Rng() << 1;
                }
            }
        }

        ColorHash = 1;
    }
}

template<>
Hash<uint64_t>::Hash(const Hash<uint64_t>& H): Value(H.Value) {
}


template<>
void Hash<uint64_t>::clear() {
    Value = 0;
}

template<>
void Hash<uint64_t>::refresh(const Position& Pos) {
    clear();

    for (Square Sq : Squares) {
        const PieceKind Piece = Pos.pieceOn(Sq);

        if (Piece != PK_Empty) {
            const Color C = getColor(Piece);
            const PieceTypeKind Type = getPieceType(Piece);

            if (C == Black) {
                update<Black>(Type, Sq);
            } else {
                update<White>(Type, Sq);
            }
        }
    }

    if (Pos.sideToMove() == White) {
        updateColor();
    }
}

template struct Hash<uint64_t>;

} // namespace internal
} // namespace core
} // namespace nshogi
