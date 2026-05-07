#include "p.h"

namespace nshogi {
namespace ml {

namespace {

// clang-format off
constexpr std::size_t OnBoardFeatureSize =
    (std::size_t)core::NumSquares // My piece's position.
    * (std::size_t)core::NumColors // My piece's color.
    * ((std::size_t)core::NumPieceType - 1); // My piece's type (excluding empty).

constexpr std::size_t OtherFeatureSize =
    18 * 2 // Pawns (max 18 for each color).
    + 4 * 4 * 2 // Lances, knights, silvers, and golds (max 4 for each color).
    + 2 * 2 * 2; // Bishops and rooks (max 2 for each color).
// clang-format on

template <core::Color C>
std::vector<std::size_t> ids_(const core::State& S) {
    std::vector<std::size_t> Ids;

    const auto Position = S.getPosition();

    for (auto Sq : core::Squares) {
        const auto Piece = Position.pieceOn(Sq);
        if (Piece != core::PK_Empty) {
            const auto PieceType = core::getPieceType(Piece);
            const auto PieceColor = (C == core::Black) ? core::getColor(Piece) : ~core::getColor(Piece);
            const auto IndexSq = (C == core::Black) ? Sq : (80 - Sq);
            const std::size_t Index =
                (std::size_t)IndexSq * ((std::size_t)core::NumColors * ((std::size_t)core::NumPieceType - 1)) +
                (std::size_t)PieceColor * ((std::size_t)core::NumPieceType - 1) +
                ((std::size_t)PieceType - 1);

            Ids.push_back(Index);
        }
    }

    std::size_t AuxiliaryBase = OnBoardFeatureSize;

    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Pawn>(); ++I) {
        Ids.push_back(AuxiliaryBase + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Pawn>(); ++I) {
        Ids.push_back(AuxiliaryBase + 18 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Lance>(); ++I) {
        Ids.push_back(AuxiliaryBase + 36 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Lance>(); ++I) {
        Ids.push_back(AuxiliaryBase + 40 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Knight>(); ++I) {
        Ids.push_back(AuxiliaryBase + 44 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Knight>(); ++I) {
        Ids.push_back(AuxiliaryBase + 48 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Silver>(); ++I) {
        Ids.push_back(AuxiliaryBase + 52 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Silver>(); ++I) {
        Ids.push_back(AuxiliaryBase + 56 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Gold>(); ++I) {
        Ids.push_back(AuxiliaryBase + 60 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Gold>(); ++I) {
        Ids.push_back(AuxiliaryBase + 64 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Bishop>(); ++I) {
        Ids.push_back(AuxiliaryBase + 68 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Bishop>(); ++I) {
        Ids.push_back(AuxiliaryBase + 70 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<C, core::PTK_Rook>(); ++I) {
        Ids.push_back(AuxiliaryBase + 72 + I);
    }
    for (std::size_t I = 0; I < Position.getStandCount<~C, core::PTK_Rook>(); ++I) {
        Ids.push_back(AuxiliaryBase + 74 + I);
    }

    return Ids;
}

template <core::Color C>
void extract_(int8_t* Data, const core::State& S) {
    const auto Ids = ids_<C>(S);
    for (const auto Id : Ids) {
        Data[Id] = 1;
    }
}

} // namespace

PFeatureExtractor::PFeatureExtractor() {
}

std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
PFeatureExtractor::ids(const core::State& S) const {
    if (S.getSideToMove() == core::Black) {
        return {
            ids_<core::Black>(S),
            ids_<core::White>(S)
        };
    } else {
        return {
            ids_<core::White>(S),
            ids_<core::Black>(S)
        };
    }
}

std::vector<int8_t> PFeatureExtractor::extract(const core::State& S) const {
    std::vector<int8_t> Data(2 * (OnBoardFeatureSize + OtherFeatureSize), 0);

    if (S.getSideToMove() == core::Black) {
        extract_<core::Black>(Data.data(), S);
        extract_<core::White>(Data.data() + OnBoardFeatureSize + OtherFeatureSize, S);
    } else {
        extract_<core::White>(Data.data(), S);
        extract_<core::Black>(Data.data() + OnBoardFeatureSize + OtherFeatureSize, S);
    }

    return Data;
}

} // namespace ml
} // namespace nshogi
