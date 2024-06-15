#include "featurestack.h"
#include "featuretype.h"

#include <cassert>

namespace nshogi {
namespace ml {

FeatureStackRuntime::FeatureStackRuntime(const std::vector<FeatureType>& Types, const core::State& State, const core::StateConfig& Config) {
    if (State.getPosition().sideToMove() == core::Black) {
        process<core::Black>(Types, State, Config);
    } else {
        process<core::White>(Types, State, Config);
    }
}

const FeatureBitboard& FeatureStackRuntime::get(std::size_t Index) const {
    return Features[Index];
}

template<core::Color C>
void FeatureStackRuntime::process(const std::vector<FeatureType>& Types, const core::State& State, const core::StateConfig& Config) {
    Features.resize(Types.size());

    for (size_t I = 0; I < Types.size(); ++I) {
        const auto& Type = Types[I];

        if (Type == FeatureType::FT_Black) {
            Features[I] = processIsBlackTurn<C>();
        } else if (Type == FeatureType::FT_White) {
            Features[I] = processIsWhiteTurn<C>();
        } else if (Type == FeatureType::FT_Check) {
            Features[I] = processCheck<C>(State);
        } else if (Type == FeatureType::FT_NoMyPawnFile) {
            Features[I] = processNoPawnFile<C, C>(State);
        } else if (Type == FeatureType::FT_NoOpPawnFile) {
            Features[I] = processNoPawnFile<C, ~C>(State);
        } else if (Type == FeatureType::FT_Progress) {
            Features[I] = processProgress(State, Config);
        } else if (Type == FeatureType::FT_ProgressUnit) {
            Features[I] = processProgressUnit(Config);
        } else if (Type == FeatureType::FT_MyPawn) {
            Features[I] = processPiece<C, C, core::PTK_Pawn>(State);
        } else if (Type == FeatureType::FT_MyLance) {
            Features[I] = processPiece<C, C, core::PTK_Lance>(State);
        } else if (Type == FeatureType::FT_MyKnight) {
            Features[I] = processPiece<C, C, core::PTK_Knight>(State);
        } else if (Type == FeatureType::FT_MySilver) {
            Features[I] = processPiece<C, C, core::PTK_Silver>(State);
        } else if (Type == FeatureType::FT_MyGold) {
            Features[I] = processPiece<C, C, core::PTK_Gold>(State);
        } else if (Type == FeatureType::FT_MyKing) {
            Features[I] = processPiece<C, C, core::PTK_King>(State);
        } else if (Type == FeatureType::FT_MyBishop) {
            Features[I] = processPiece<C, C, core::PTK_Bishop>(State);
        } else if (Type == FeatureType::FT_MyRook) {
            Features[I] = processPiece<C, C, core::PTK_Rook>(State);
        } else if (Type == FeatureType::FT_MyProPawn) {
            Features[I] = processPiece<C, C, core::PTK_ProPawn>(State);
        } else if (Type == FeatureType::FT_MyProLance) {
            Features[I] = processPiece<C, C, core::PTK_ProLance>(State);
        } else if (Type == FeatureType::FT_MyProKnight) {
            Features[I] = processPiece<C, C, core::PTK_ProKnight>(State);
        } else if (Type == FeatureType::FT_MyProSilver) {
            Features[I] = processPiece<C, C, core::PTK_ProSilver>(State);
        } else if (Type == FeatureType::FT_MyProBishop) {
            Features[I] = processPiece<C, C, core::PTK_ProBishop>(State);
        } else if (Type == FeatureType::FT_MyProRook) {
            Features[I] = processPiece<C, C, core::PTK_ProRook>(State);
        } else if (Type == FeatureType::FT_OpPawn) {
            Features[I] = processPiece<C, ~C, core::PTK_Pawn>(State);
        } else if (Type == FeatureType::FT_OpLance) {
            Features[I] = processPiece<C, ~C, core::PTK_Lance>(State);
        } else if (Type == FeatureType::FT_OpKnight) {
            Features[I] = processPiece<C, ~C, core::PTK_Knight>(State);
        } else if (Type == FeatureType::FT_OpSilver) {
            Features[I] = processPiece<C, ~C, core::PTK_Silver>(State);
        } else if (Type == FeatureType::FT_OpGold) {
            Features[I] = processPiece<C, ~C, core::PTK_Gold>(State);
        } else if (Type == FeatureType::FT_OpKing) {
            Features[I] = processPiece<C, ~C, core::PTK_King>(State);
        } else if (Type == FeatureType::FT_OpBishop) {
            Features[I] = processPiece<C, ~C, core::PTK_Bishop>(State);
        } else if (Type == FeatureType::FT_OpRook) {
            Features[I] = processPiece<C, ~C, core::PTK_Rook>(State);
        } else if (Type == FeatureType::FT_OpProPawn) {
            Features[I] = processPiece<C, ~C, core::PTK_ProPawn>(State);
        } else if (Type == FeatureType::FT_OpProLance) {
            Features[I] = processPiece<C, ~C, core::PTK_ProLance>(State);
        } else if (Type == FeatureType::FT_OpProKnight) {
            Features[I] = processPiece<C, ~C, core::PTK_ProKnight>(State);
        } else if (Type == FeatureType::FT_OpProSilver) {
            Features[I] = processPiece<C, ~C, core::PTK_ProSilver>(State);
        } else if (Type == FeatureType::FT_OpProBishop) {
            Features[I] = processPiece<C, ~C, core::PTK_ProBishop>(State);
        } else if (Type == FeatureType::FT_OpProRook) {
            Features[I] = processPiece<C, ~C, core::PTK_ProRook>(State);
        } else if (Type == FeatureType::FT_MyStandPawn1) {
            Features[I] = processStand<C, core::PTK_Pawn, 1>(State);
        } else if (Type == FeatureType::FT_MyStandPawn2) {
            Features[I] = processStand<C, core::PTK_Pawn, 2>(State);
        } else if (Type == FeatureType::FT_MyStandPawn3) {
            Features[I] = processStand<C, core::PTK_Pawn, 3>(State);
        } else if (Type == FeatureType::FT_MyStandPawn4) {
            Features[I] = processStand<C, core::PTK_Pawn, 4>(State);
        } else if (Type == FeatureType::FT_MyStandPawn5) {
            Features[I] = processStand<C, core::PTK_Pawn, 5>(State);
        } else if (Type == FeatureType::FT_MyStandPawn6) {
            Features[I] = processStand<C, core::PTK_Pawn, 6>(State);
        } else if (Type == FeatureType::FT_MyStandPawn7) {
            Features[I] = processStand<C, core::PTK_Pawn, 7>(State);
        } else if (Type == FeatureType::FT_MyStandPawn8) {
            Features[I] = processStand<C, core::PTK_Pawn, 8>(State);
        } else if (Type == FeatureType::FT_MyStandPawn9) {
            Features[I] = processStand<C, core::PTK_Pawn, 9>(State);
        } else if (Type == FeatureType::FT_MyStandLance1) {
            Features[I] = processStand<C, core::PTK_Lance, 1>(State);
        } else if (Type == FeatureType::FT_MyStandLance2) {
            Features[I] = processStand<C, core::PTK_Lance, 2>(State);
        } else if (Type == FeatureType::FT_MyStandLance3) {
            Features[I] = processStand<C, core::PTK_Lance, 3>(State);
        } else if (Type == FeatureType::FT_MyStandLance4) {
            Features[I] = processStand<C, core::PTK_Lance, 4>(State);
        } else if (Type == FeatureType::FT_MyStandKnight1) {
            Features[I] = processStand<C, core::PTK_Knight, 1>(State);
        } else if (Type == FeatureType::FT_MyStandKnight2) {
            Features[I] = processStand<C, core::PTK_Knight, 2>(State);
        } else if (Type == FeatureType::FT_MyStandKnight3) {
            Features[I] = processStand<C, core::PTK_Knight, 3>(State);
        } else if (Type == FeatureType::FT_MyStandKnight4) {
            Features[I] = processStand<C, core::PTK_Knight, 4>(State);
        } else if (Type == FeatureType::FT_MyStandSilver1) {
            Features[I] = processStand<C, core::PTK_Silver, 1>(State);
        } else if (Type == FeatureType::FT_MyStandSilver2) {
            Features[I] = processStand<C, core::PTK_Silver, 2>(State);
        } else if (Type == FeatureType::FT_MyStandSilver3) {
            Features[I] = processStand<C, core::PTK_Silver, 3>(State);
        } else if (Type == FeatureType::FT_MyStandSilver4) {
            Features[I] = processStand<C, core::PTK_Silver, 4>(State);
        } else if (Type == FeatureType::FT_MyStandGold1) {
            Features[I] = processStand<C, core::PTK_Gold, 1>(State);
        } else if (Type == FeatureType::FT_MyStandGold2) {
            Features[I] = processStand<C, core::PTK_Gold, 2>(State);
        } else if (Type == FeatureType::FT_MyStandGold3) {
            Features[I] = processStand<C, core::PTK_Gold, 3>(State);
        } else if (Type == FeatureType::FT_MyStandGold4) {
            Features[I] = processStand<C, core::PTK_Gold, 4>(State);
        } else if (Type == FeatureType::FT_MyStandBishop1) {
            Features[I] = processStand<C, core::PTK_Bishop, 1>(State);
        } else if (Type == FeatureType::FT_MyStandBishop2) {
            Features[I] = processStand<C, core::PTK_Bishop, 2>(State);
        } else if (Type == FeatureType::FT_MyStandRook1) {
            Features[I] = processStand<C, core::PTK_Rook, 1>(State);
        } else if (Type == FeatureType::FT_MyStandRook2) {
            Features[I] = processStand<C, core::PTK_Rook, 2>(State);
        } else if (Type == FeatureType::FT_OpStandPawn1) {
            Features[I] = processStand<~C, core::PTK_Pawn, 1>(State);
        } else if (Type == FeatureType::FT_OpStandPawn2) {
            Features[I] = processStand<~C, core::PTK_Pawn, 2>(State);
        } else if (Type == FeatureType::FT_OpStandPawn3) {
            Features[I] = processStand<~C, core::PTK_Pawn, 3>(State);
        } else if (Type == FeatureType::FT_OpStandPawn4) {
            Features[I] = processStand<~C, core::PTK_Pawn, 4>(State);
        } else if (Type == FeatureType::FT_OpStandPawn5) {
            Features[I] = processStand<~C, core::PTK_Pawn, 5>(State);
        } else if (Type == FeatureType::FT_OpStandPawn6) {
            Features[I] = processStand<~C, core::PTK_Pawn, 6>(State);
        } else if (Type == FeatureType::FT_OpStandPawn7) {
            Features[I] = processStand<~C, core::PTK_Pawn, 7>(State);
        } else if (Type == FeatureType::FT_OpStandPawn8) {
            Features[I] = processStand<~C, core::PTK_Pawn, 8>(State);
        } else if (Type == FeatureType::FT_OpStandPawn9) {
            Features[I] = processStand<~C, core::PTK_Pawn, 9>(State);
        } else if (Type == FeatureType::FT_OpStandLance1) {
            Features[I] = processStand<~C, core::PTK_Lance, 1>(State);
        } else if (Type == FeatureType::FT_OpStandLance2) {
            Features[I] = processStand<~C, core::PTK_Lance, 2>(State);
        } else if (Type == FeatureType::FT_OpStandLance3) {
            Features[I] = processStand<~C, core::PTK_Lance, 3>(State);
        } else if (Type == FeatureType::FT_OpStandLance4) {
            Features[I] = processStand<~C, core::PTK_Lance, 4>(State);
        } else if (Type == FeatureType::FT_OpStandKnight1) {
            Features[I] = processStand<~C, core::PTK_Knight, 1>(State);
        } else if (Type == FeatureType::FT_OpStandKnight2) {
            Features[I] = processStand<~C, core::PTK_Knight, 2>(State);
        } else if (Type == FeatureType::FT_OpStandKnight3) {
            Features[I] = processStand<~C, core::PTK_Knight, 3>(State);
        } else if (Type == FeatureType::FT_OpStandKnight4) {
            Features[I] = processStand<~C, core::PTK_Knight, 4>(State);
        } else if (Type == FeatureType::FT_OpStandSilver1) {
            Features[I] = processStand<~C, core::PTK_Silver, 1>(State);
        } else if (Type == FeatureType::FT_OpStandSilver2) {
            Features[I] = processStand<~C, core::PTK_Silver, 2>(State);
        } else if (Type == FeatureType::FT_OpStandSilver3) {
            Features[I] = processStand<~C, core::PTK_Silver, 3>(State);
        } else if (Type == FeatureType::FT_OpStandSilver4) {
            Features[I] = processStand<~C, core::PTK_Silver, 4>(State);
        } else if (Type == FeatureType::FT_OpStandGold1) {
            Features[I] = processStand<~C, core::PTK_Gold, 1>(State);
        } else if (Type == FeatureType::FT_OpStandGold2) {
            Features[I] = processStand<~C, core::PTK_Gold, 2>(State);
        } else if (Type == FeatureType::FT_OpStandGold3) {
            Features[I] = processStand<~C, core::PTK_Gold, 3>(State);
        } else if (Type == FeatureType::FT_OpStandGold4) {
            Features[I] = processStand<~C, core::PTK_Gold, 4>(State);
        } else if (Type == FeatureType::FT_OpStandBishop1) {
            Features[I] = processStand<~C, core::PTK_Bishop, 1>(State);
        } else if (Type == FeatureType::FT_OpStandBishop2) {
            Features[I] = processStand<~C, core::PTK_Bishop, 2>(State);
        } else if (Type == FeatureType::FT_OpStandRook1) {
            Features[I] = processStand<~C, core::PTK_Rook, 1>(State);
        } else if (Type == FeatureType::FT_OpStandRook2) {
            Features[I] = processStand<~C, core::PTK_Rook, 2>(State);
        } else if (Type == FeatureType::FT_RuleDeclare27) {
            Features[I] = processRule<core::EndingRule::ER_Declare27>(Config);
        } else if (Type == FeatureType::FT_RuleDraw24) {
            Features[I] = processRule<core::EndingRule::ER_Draw24>(Config);
        } else if (Type == FeatureType::FT_RuleTrying) {
            Features[I] = processRule<core::EndingRule::ER_Trying>(Config);
        } else if (Type == FeatureType::FT_MyDrawValue) {
            Features[I] = processDrawValue<C>(Config);
        } else if (Type == FeatureType::FT_OpDrawValue) {
            Features[I] = processDrawValue<~C>(Config);
        } else if (Type == FeatureType::FT_MyDeclarationScore) {
            Features[I] = processDeclarationScore<C>(State);
        } else if (Type == FeatureType::FT_OpDeclarationScore) {
            Features[I] = processDeclarationScore<~C>(State);
        } else if (Type == FeatureType::FT_MyPieceScore) {
            Features[I] = processPieceScore<C>(State);
        } else if (Type == FeatureType::FT_OpPieceScore) {
            Features[I] = processPieceScore<~C>(State);
        } else {
            assert(false);
        }
    }
}


} // namespace ml
} // namespace nshogi
