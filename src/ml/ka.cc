//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "ka.h"
#include "../core/internal/bitboard.h"
#include "../core/internal/stateadapter.h"

#include <stdexcept>

namespace nshogi {
namespace ml {

namespace {

// clang-format off
constexpr std::size_t BoardFeatureSize =
    (std::size_t)core::NumSquares // The other piece's position.
    * 27;

constexpr std::size_t StandFeatureSize =
    18 * 2 // Pawns (max 18 for each color).
    + 4 * 4 * 2 // Lances, knights, silvers, and golds (max 4 for each color).
    + 2 * 2 * 2; // Bishops and rooks (max 2 for each color).

constexpr std::size_t FeatureSizePerKingSquare =
    BoardFeatureSize + StandFeatureSize;

[[maybe_unused]]
constexpr std::size_t FeatureSize =
    (std::size_t)core::NumSquares // My king's position.
    * FeatureSizePerKingSquare;

constexpr static int32_t PieceToIndex[core::NumPieceKind] = {
    -1, // My Empty
     0, // My Pawn
     1, // My Lance
     2, // My Knight
     3, // My Silver
     4, // My Bishop
     5, // My Rook
     6, // My Gold
    -1, // My King
     7, // My ProPawn
     8, // My ProLance
     9, // My ProKnight
    10, // My ProSilver
    11, // My ProBishop
    12, // My ProRook
    -1, // Not used
    -1, // Not used
    13, // Op Pawn
    14, // Op Lance
    15, // Op Knight
    16, // Op Silver
    17, // Op Bishop
    18, // Op Rook
    19, // Op Gold
    20, // Op King
    21, // Op ProPawn
    22, // Op ProLance
    23, // Op ProKnight
    24, // Op ProSilver
    25, // Op ProBishop
    26, // Op ProRook
};

// clang-format on

std::size_t countPieces(const core::Position& Position,
                        const core::internal::bitboard::Bitboard& OccupiedBB) {
    std::size_t NumPieces = OccupiedBB.popCount();
    for (const auto Type :
         {core::PTK_Pawn, core::PTK_Lance, core::PTK_Knight, core::PTK_Silver,
          core::PTK_Gold, core::PTK_Bishop, core::PTK_Rook}) {
        NumPieces += (std::size_t)(Position.getStandCount(core::Black, Type) +
                                   Position.getStandCount(core::White, Type));
    }
    return NumPieces;
}

template <core::Color C>
void idsAt_(int32_t* DestMyIds, int32_t* DestOpIds, int32_t* DestMyIdsCount,
            int32_t* DestOpIdsCount, const core::State& S) {
    *DestMyIdsCount = 0;
    *DestOpIdsCount = 0;

    const auto& Position = S.getPosition();
    core::internal::ImmutableStateAdapter Adapter(S);

    const core::internal::bitboard::Bitboard OccupiedBB =
        Adapter->getBitboard<core::Black>() |
        Adapter->getBitboard<core::White>();

    // The destination buffers hold exactly 39 ids: every piece except the
    // perspective owner's king emits one id per perspective. Any other piece
    // count (or a missing/duplicated king) would overflow the buffers or
    // silently leave id 0 in the unused slots.
    if (countPieces(Position, OccupiedBB) != 40 ||
        Adapter->getBitboard<core::Black, core::PTK_King>().popCount() != 1 ||
        Adapter->getBitboard<core::White, core::PTK_King>().popCount() != 1) {
        throw std::runtime_error(
            "KAFeatureExtractor requires a position with exactly 40 pieces "
            "including one king for each color.");
    }

    const auto MyKingSquare =
        (C == core::Black) ? S.getKingSquare(core::Black)
                           : core::getInversed(S.getKingSquare(core::White));

    const auto OpKingSquare =
        (C == core::Black) ? S.getKingSquare(core::White)
                           : core::getInversed(S.getKingSquare(core::Black));

    const auto MyKingPiece =
        (C == core::Black) ? core::PK_BlackKing : core::PK_WhiteKing;
    const auto OpKingPiece =
        (C == core::Black) ? core::PK_WhiteKing : core::PK_BlackKing;

    const std::size_t MyBase =
        (std::size_t)MyKingSquare * FeatureSizePerKingSquare;
    const std::size_t OpBase =
        (std::size_t)core::getInversed(OpKingSquare) * FeatureSizePerKingSquare;

    OccupiedBB.forEach([&](core::Square Sq) {
        const auto RawPiece = Position.pieceOn(Sq);
        if (RawPiece != core::PK_Empty) {
            if (RawPiece != MyKingPiece) { // From my perspective.
                const auto Piece =
                    (C == core::Black) ? RawPiece : core::getInversed(RawPiece);

                const int32_t PieceIndex = PieceToIndex[(std::size_t)Piece];
                assert(PieceIndex >= 0 && PieceIndex < 27);

                const auto IndexSq =
                    (C == core::Black) ? Sq : core::getInversed(Sq);
                const std::size_t Index =
                    (std::size_t)IndexSq * 27 + (std::size_t)PieceIndex;
                DestMyIds[(*DestMyIdsCount)++] = (int32_t)(MyBase + Index);
            }

            if (RawPiece != OpKingPiece) { // From opponent's perspective.
                const auto Piece =
                    (C == core::Black) ? core::getInversed(RawPiece) : RawPiece;

                const int32_t PieceIndex = PieceToIndex[(std::size_t)Piece];
                assert(PieceIndex >= 0 && PieceIndex < 27);

                const auto IndexSq =
                    (C == core::Black) ? core::getInversed(Sq) : Sq;
                const std::size_t Index =
                    (std::size_t)IndexSq * 27 + (std::size_t)PieceIndex;
                DestOpIds[(*DestOpIdsCount)++] = (int32_t)(OpBase + Index);
            }
        }
    });

    const std::size_t MyAuxiliaryBase = MyBase + BoardFeatureSize;
    const std::size_t OpAuxiliaryBase = OpBase + BoardFeatureSize;

    auto appendStandPair = [&](std::size_t Offset, std::size_t MaxCount,
                               core::PieceTypeKind Type) {
        const uint8_t MyStandCount = Position.getStandCount(C, Type);
        const uint8_t OpStandCount = Position.getStandCount(~C, Type);
        for (uint8_t I = 0; I < MyStandCount; ++I) {
            DestMyIds[(*DestMyIdsCount)++] =
                (int32_t)(MyAuxiliaryBase + Offset + I);
            DestOpIds[(*DestOpIdsCount)++] =
                (int32_t)(OpAuxiliaryBase + Offset + MaxCount + I);
        }
        for (uint8_t I = 0; I < OpStandCount; ++I) {
            DestMyIds[(*DestMyIdsCount)++] =
                (int32_t)(MyAuxiliaryBase + Offset + MaxCount + I);
            DestOpIds[(*DestOpIdsCount)++] =
                (int32_t)(OpAuxiliaryBase + Offset + I);
        }
    };

    // clang-format off
    appendStandPair( 0, 18, core::PTK_Pawn  );
    appendStandPair(36,  4, core::PTK_Lance );
    appendStandPair(44,  4, core::PTK_Knight);
    appendStandPair(52,  4, core::PTK_Silver);
    appendStandPair(60,  2, core::PTK_Bishop);
    appendStandPair(64,  2, core::PTK_Rook  );
    appendStandPair(68,  4, core::PTK_Gold  );
    // clang-format on

#ifndef NDEBUG
    for (int32_t I = 0; I < *DestMyIdsCount; ++I) {
        assert(DestMyIds[I] >= 0);
        assert(DestMyIds[I] < FeatureSize);
    }
    for (int32_t I = 0; I < *DestOpIdsCount; ++I) {
        assert(DestOpIds[I] >= 0);
        assert(DestOpIds[I] < FeatureSize);
    }
#endif
}

template <core::Color C>
std::pair<std::vector<int32_t>, std::vector<int32_t>>
ids_(const core::State& S) {
    // There are 39 pieces on the board and stands except for my king.
    std::vector<int32_t> MyIds(39);
    std::vector<int32_t> OpIds(39);

    int32_t MyIdsCount;
    int32_t OpIdsCount;

    idsAt_<C>(MyIds.data(), OpIds.data(), &MyIdsCount, &OpIdsCount, S);

    return {std::move(MyIds), std::move(OpIds)};
}

} // namespace

KAFeatureExtractor::KAFeatureExtractor() {
}

void KAFeatureExtractor::idsAt(int32_t* DestMyIds, int32_t* DestOpIds,
                               int32_t* DestMyIdsCount, int32_t* DestOpIdsCount,
                               const core::State& S) const {
    if (S.getSideToMove() == core::Black) {
        idsAt_<core::Black>(DestMyIds, DestOpIds, DestMyIdsCount,
                            DestOpIdsCount, S);
        assert(*DestMyIdsCount == idSize());
        assert(*DestOpIdsCount == idSize());
    } else {
        idsAt_<core::White>(DestMyIds, DestOpIds, DestMyIdsCount,
                            DestOpIdsCount, S);
        assert(*DestMyIdsCount == idSize());
        assert(*DestOpIdsCount == idSize());
    }
}

std::pair<std::vector<int32_t>, std::vector<int32_t>>
KAFeatureExtractor::ids(const core::State& S) const {
    if (S.getSideToMove() == core::Black) {
        return ids_<core::Black>(S);
    } else {
        return ids_<core::White>(S);
    }
}

std::size_t KAFeatureExtractor::idSize() const noexcept {
    return 39;
}

} // namespace ml
} // namespace nshogi
