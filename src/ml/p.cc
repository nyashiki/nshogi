#include "p.h"

#include "../core/internal/bitboard.h"
#include "../core/internal/stateadapter.h"

#include <stdexcept>

namespace nshogi {
namespace ml {

namespace {

// clang-format off

constexpr std::size_t OnBoardFeatureSize =
    (std::size_t)core::NumSquares // My piece's position.
    * (std::size_t)core::NumColors // My piece's color.
    * ((std::size_t)core::NumPieceType - 1); // My piece's type (excluding empty).

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

    // The destination buffers hold exactly 40 ids and every piece emits one
    // id per perspective, so any other piece count would overflow the
    // buffers (or silently leave id 0 in the unused slots).
    if (countPieces(Position, OccupiedBB) != 40) {
        throw std::runtime_error(
            "PFeatureExtractor requires a position with exactly 40 pieces.");
    }

    OccupiedBB.forEach([&](core::Square Sq) {
        const auto Piece = Position.pieceOn(Sq);
        assert(Piece != core::PieceKind::PK_Empty);

        const auto PieceType = core::getPieceType(Piece);
        const auto PieceColor =
            (C == core::Black) ? core::getColor(Piece) : ~core::getColor(Piece);
        const auto IndexSq = (C == core::Black) ? Sq : core::getInversed(Sq);

        const std::size_t MyIndex =
            (std::size_t)IndexSq * ((std::size_t)core::NumColors *
                                    ((std::size_t)core::NumPieceType - 1)) +
            (std::size_t)PieceColor * ((std::size_t)core::NumPieceType - 1) +
            ((std::size_t)PieceType - 1);

        const std::size_t OpIndex =
            (std::size_t)core::getInversed(IndexSq) *
                ((std::size_t)core::NumColors *
                 ((std::size_t)core::NumPieceType - 1)) +
            (std::size_t)(~PieceColor) * ((std::size_t)core::NumPieceType - 1) +
            ((std::size_t)PieceType - 1);

        assert(MyIndex < OnBoardFeatureSize);
        assert(OpIndex < OnBoardFeatureSize);

        DestMyIds[(*DestMyIdsCount)++] = (int32_t)MyIndex;
        DestOpIds[(*DestOpIdsCount)++] = (int32_t)OpIndex;
    });

    const int32_t AuxiliaryBase = OnBoardFeatureSize;

    auto appendStandPair = [&](int32_t MyBase, int32_t OpBase,
                               core::PieceTypeKind Type) {
        const uint8_t MyStandCount = Position.getStandCount(C, Type);
        const uint8_t OpStandCount = Position.getStandCount(~C, Type);
        for (uint8_t I = 0; I < MyStandCount; ++I) {
            DestMyIds[(*DestMyIdsCount)++] = MyBase + I;
            DestOpIds[(*DestOpIdsCount)++] = OpBase + I;
        }
        for (uint8_t I = 0; I < OpStandCount; ++I) {
            DestMyIds[(*DestMyIdsCount)++] = OpBase + I;
            DestOpIds[(*DestOpIdsCount)++] = MyBase + I;
        }
    };

    appendStandPair(AuxiliaryBase, AuxiliaryBase + 18, core::PTK_Pawn);
    appendStandPair(AuxiliaryBase + 36, AuxiliaryBase + 40, core::PTK_Lance);
    appendStandPair(AuxiliaryBase + 44, AuxiliaryBase + 48, core::PTK_Knight);
    appendStandPair(AuxiliaryBase + 52, AuxiliaryBase + 56, core::PTK_Silver);
    appendStandPair(AuxiliaryBase + 60, AuxiliaryBase + 62, core::PTK_Bishop);
    appendStandPair(AuxiliaryBase + 64, AuxiliaryBase + 66, core::PTK_Rook);
    appendStandPair(AuxiliaryBase + 68, AuxiliaryBase + 72, core::PTK_Gold);
}

template <core::Color C>
std::pair<std::vector<int32_t>, std::vector<int32_t>>
ids_(const core::State& S) {
    // There are 40 pieces on the board and stands.
    std::vector<int32_t> MyIds(40);
    std::vector<int32_t> OpIds(40);

    int32_t MyIdsCount;
    int32_t OpIdsCount;

    idsAt_<C>(MyIds.data(), OpIds.data(), &MyIdsCount, &OpIdsCount, S);

    return {std::move(MyIds), std::move(OpIds)};
}

} // namespace

PFeatureExtractor::PFeatureExtractor() {
}

void PFeatureExtractor::idsAt(int32_t* DestMyIds, int32_t* DestOpIds,
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
PFeatureExtractor::ids(const core::State& S) const {
    if (S.getSideToMove() == core::Black) {
        return ids_<core::Black>(S);
    } else {
        return ids_<core::White>(S);
    }
}

std::size_t PFeatureExtractor::idSize() const noexcept {
    return 40;
}

} // namespace ml
} // namespace nshogi
