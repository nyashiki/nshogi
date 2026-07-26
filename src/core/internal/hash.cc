//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "hash.h"
#include <cstdint>
#include <random>

namespace nshogi {
namespace core {
namespace internal {

template <>
Hash<uint64_t>::Hash() {
    static std::mt19937_64 Rng(20260711);

    if (!IsTableGenerated) {
        IsTableGenerated = true;

        for (Color C : Colors) {
            for (PieceTypeKind Type : PieceTypes) {
                for (Square Sq : Squares) {
                    // Shift by one so that bit 0 stays reserved for
                    // ColorHash (the side to move).
                    OnBoardHash[C][Type][Sq] = Rng() << 1;
                }
            }
        }

        ColorHash = 1;
    }
}

template <>
Hash<uint64_t>::Hash(const Hash<uint64_t>& H)
    : Value(H.Value) {
}

template <>
void Hash<uint64_t>::clear() {
    Value = 0;
}

template <>
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
