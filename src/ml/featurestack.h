#ifndef NSHOGI_ML_FEATURESTACK_H
#define NSHOGI_ML_FEATURESTACK_H

#include "featurebitboard.h"
#include "featuretype.h"
#include "../core/state.h"
#include "../core/stateconfig.h"
#include "../core/internal/stateadapter.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

namespace nshogi {
namespace ml {

namespace {

template<core::Color C, FeatureType Type>
struct FeatureProcessor {
    static FeatureBitboard process(const core::State& State, const core::StateConfig& Config);
};

template<core::Color C>
[[maybe_unused]] FeatureBitboard processIsBlackTurn();

template<>
[[maybe_unused]] FeatureBitboard processIsBlackTurn<core::Black>() {
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), 1, false);
}

template<>
[[maybe_unused]] FeatureBitboard processIsBlackTurn<core::White>() {
    return FeatureBitboard(core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template<core::Color C>
[[maybe_unused]] FeatureBitboard processIsWhiteTurn();

template<>
[[maybe_unused]] FeatureBitboard processIsWhiteTurn<core::Black>() {
    return FeatureBitboard(core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template<>
[[maybe_unused]] FeatureBitboard processIsWhiteTurn<core::White>() {
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), 1, false);
}

template<core::Color C>
[[maybe_unused]] FeatureBitboard processCheck(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    if (!Adapter->getCheckerBB().isZero()) {
        return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), 1, false);
    }

    return FeatureBitboard(core::internal::bitboard::Bitboard::ZeroBB(), 0, false);
}

template<core::Color MyColor, core::Color TargetColor>
[[maybe_unused]] FeatureBitboard processNoPawnFile(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    const core::internal::bitboard::Bitboard PawnBB = Adapter->getBitboard<TargetColor, core::PTK_Pawn>();
    core::internal::bitboard::Bitboard PawnExistFilesBB = core::internal::bitboard::Bitboard::ZeroBB();

    if (!(PawnBB & core::internal::bitboard::FileBB[core::File1]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File1];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File2]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File2];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File3]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File3];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File4]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File4];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File5]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File5];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File6]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File6];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File7]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File7];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File8]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File8];
    }
    if (!(PawnBB & core::internal::bitboard::FileBB[core::File9]).isZero()) {
        PawnExistFilesBB |= core::internal::bitboard::FileBB[core::File9];
    }

    return FeatureBitboard(~PawnExistFilesBB, 1, MyColor == core::White);
}

[[maybe_unused]] FeatureBitboard processProgress(const core::State& State, const core::StateConfig& Config) {
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (float)State.getPly() / Config.MaxPly, false);
}

[[maybe_unused]] FeatureBitboard processProgressUnit(const core::StateConfig& Config) {
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), 1.0f / Config.MaxPly, false);
}

template<core::Color MyColor, core::Color PieceColor, core::PieceTypeKind Type>
[[maybe_unused]] FeatureBitboard processPiece(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return FeatureBitboard(Adapter->getBitboard<PieceColor, Type>(), 1, MyColor == core::White);
}

template<core::Color MyColor, core::PieceTypeKind Type, uint8_t Count>
[[maybe_unused]] FeatureBitboard processStand(const core::State& State) {
    uint8_t TypeStandCount = State.getPosition().getStandCount<MyColor, Type>();
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (TypeStandCount >= Count), false);
}

template<core::EndingRule Rule>
[[maybe_unused]] FeatureBitboard processRule(const core::StateConfig& Config) {
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (Config.Rule == Rule), false);
}

template<core::Color MyColor>
[[maybe_unused]] FeatureBitboard processDrawValue(const core::StateConfig& Config) {
    if constexpr (MyColor == core::Black) {
        return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), Config.BlackDrawValue, false);
    } else {
        return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), Config.WhiteDrawValue, false);
    }
}

template <core::Color C>
FeatureBitboard processDeclarationScore(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    if constexpr (C == core::Black) {
        return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (float)Adapter->computeDeclarationScore<C>() / 28.0f, false);
    } else {
        return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (float)Adapter->computeDeclarationScore<C>() / 27.0f, false);
    }
}

template <core::Color C>
FeatureBitboard processPieceScore(const core::State& State) {
    core::internal::ImmutableStateAdapter Adapter(State);
    return FeatureBitboard(core::internal::bitboard::Bitboard::AllBB(), (float)Adapter->computePieceScore<C>(5, 1, false) / 54.0f, false);
}

} // namespace

struct FeatureStack {
    virtual ~FeatureStack() {
    }

    virtual const FeatureBitboard& get(std::size_t Index) const = 0;
};

template<FeatureType... FeatureTypes>
struct FeatureStackComptime : FeatureStack {
    FeatureStackComptime(const core::State& State, const core::StateConfig& Config)
        : Features(sizeof...(FeatureTypes)) {
        if (State.getPosition().sideToMove() == core::Black) {
            process<core::Black, FeatureTypes...>(State, Config, Features.data());
        } else {
            process<core::White, FeatureTypes...>(State, Config, Features.data());
        }
    }

    FeatureStackComptime(const FeatureStackComptime&) = delete;
    FeatureStackComptime(FeatureStackComptime&& FSC) noexcept
        : Features(std::move(FSC.Features)) {
    }

    FeatureStackComptime& operator=(FeatureStackComptime&& FSC) noexcept {
        if (this != &FSC) {
            Features = std::move(FSC.Features);
        }
    }

    static void constructAt(FeatureBitboard* FB, const core::State& State, const core::StateConfig& Config) {
        if (State.getPosition().sideToMove() == core::Black) {
            process<core::Black, FeatureTypes...>(State, Config, FB);
        } else {
            process<core::White, FeatureTypes...>(State, Config, FB);
        }
    }

    template<core::IterateOrder Order = core::IterateOrder::NWSE>
    std::vector<float> extract() const {
        std::vector<float> Dest(sizeof...(FeatureTypes) * core::NumSquares);

        extract<Order>(Dest.data());

        return Dest;
    }

    template<core::IterateOrder Order = core::IterateOrder::NWSE>
    void extract(float* Dest) const {
        for (std::size_t Channel = 0; Channel < sizeof...(FeatureTypes); ++Channel) {
            Features[Channel].template extract<Order>(Dest + Channel * core::NumSquares);
        }
    }

    const FeatureBitboard& get(std::size_t Index) const {
        assert(Index >= 0 && Index < sizeof...(FeatureTypes));

        return Features[Index];
    }

    const FeatureBitboard* data() const {
        return Features.data();
    }

    static constexpr std::size_t size() {
        return sizeof...(FeatureTypes);
    }

 private:
    template<core::Color C>
    static void process([[maybe_unused]] const core::State& State, [[maybe_unused]] const core::StateConfig& Config, [[maybe_unused]] FeatureBitboard* FB) {
    }

    template<core::Color C, FeatureType HeadType, FeatureType... TailType>
    static void process(const core::State& State, const core::StateConfig& Config, FeatureBitboard* FB) {
        if constexpr (HeadType == FeatureType::FT_Black) {
            *FB = processIsBlackTurn<C>();
        } else if constexpr (HeadType == FeatureType::FT_White) {
            *FB = processIsWhiteTurn<C>();
        } else if constexpr (HeadType == FeatureType::FT_Check) {
            *FB = processCheck<C>(State);
        } else if constexpr (HeadType == FeatureType::FT_NoMyPawnFile) {
            *FB = processNoPawnFile<C, C>(State);
        } else if constexpr (HeadType == FeatureType::FT_NoOpPawnFile) {
            *FB = processNoPawnFile<C, ~C>(State);
        } else if constexpr (HeadType == FeatureType::FT_Progress) {
            *FB = processProgress(State, Config);
        } else if constexpr (HeadType == FeatureType::FT_ProgressUnit) {
            *FB = processProgressUnit(Config);
        } else if constexpr (HeadType == FeatureType::FT_MyPawn) {
            *FB = processPiece<C, C, core::PTK_Pawn>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyLance) {
            *FB = processPiece<C, C, core::PTK_Lance>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyKnight) {
            *FB = processPiece<C, C, core::PTK_Knight>(State);
        } else if constexpr (HeadType == FeatureType::FT_MySilver) {
            *FB = processPiece<C, C, core::PTK_Silver>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyGold) {
            *FB = processPiece<C, C, core::PTK_Gold>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyKing) {
            *FB = processPiece<C, C, core::PTK_King>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyBishop) {
            *FB = processPiece<C, C, core::PTK_Bishop>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyRook) {
            *FB = processPiece<C, C, core::PTK_Rook>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProPawn) {
            *FB = processPiece<C, C, core::PTK_ProPawn>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProLance) {
            *FB = processPiece<C, C, core::PTK_ProLance>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProKnight) {
            *FB = processPiece<C, C, core::PTK_ProKnight>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProSilver) {
            *FB = processPiece<C, C, core::PTK_ProSilver>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProBishop) {
            *FB = processPiece<C, C, core::PTK_ProBishop>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyProRook) {
            *FB = processPiece<C, C, core::PTK_ProRook>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpPawn) {
            *FB = processPiece<C, ~C, core::PTK_Pawn>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpLance) {
            *FB = processPiece<C, ~C, core::PTK_Lance>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpKnight) {
            *FB = processPiece<C, ~C, core::PTK_Knight>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpSilver) {
            *FB = processPiece<C, ~C, core::PTK_Silver>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpGold) {
            *FB = processPiece<C, ~C, core::PTK_Gold>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpKing) {
            *FB = processPiece<C, ~C, core::PTK_King>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpBishop) {
            *FB = processPiece<C, ~C, core::PTK_Bishop>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpRook) {
            *FB = processPiece<C, ~C, core::PTK_Rook>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProPawn) {
            *FB = processPiece<C, ~C, core::PTK_ProPawn>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProLance) {
            *FB = processPiece<C, ~C, core::PTK_ProLance>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProKnight) {
            *FB = processPiece<C, ~C, core::PTK_ProKnight>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProSilver) {
            *FB = processPiece<C, ~C, core::PTK_ProSilver>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProBishop) {
            *FB = processPiece<C, ~C, core::PTK_ProBishop>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpProRook) {
            *FB = processPiece<C, ~C, core::PTK_ProRook>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn1) {
            *FB = processStand<C, core::PTK_Pawn, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn2) {
            *FB = processStand<C, core::PTK_Pawn, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn3) {
            *FB = processStand<C, core::PTK_Pawn, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn4) {
            *FB = processStand<C, core::PTK_Pawn, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn5) {
            *FB = processStand<C, core::PTK_Pawn, 5>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn6) {
            *FB = processStand<C, core::PTK_Pawn, 6>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn7) {
            *FB = processStand<C, core::PTK_Pawn, 7>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn8) {
            *FB = processStand<C, core::PTK_Pawn, 8>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandPawn9) {
            *FB = processStand<C, core::PTK_Pawn, 9>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandLance1) {
            *FB = processStand<C, core::PTK_Lance, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandLance2) {
            *FB = processStand<C, core::PTK_Lance, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandLance3) {
            *FB = processStand<C, core::PTK_Lance, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandLance4) {
            *FB = processStand<C, core::PTK_Lance, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandKnight1) {
            *FB = processStand<C, core::PTK_Knight, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandKnight2) {
            *FB = processStand<C, core::PTK_Knight, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandKnight3) {
            *FB = processStand<C, core::PTK_Knight, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandKnight4) {
            *FB = processStand<C, core::PTK_Knight, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandSilver1) {
            *FB = processStand<C, core::PTK_Silver, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandSilver2) {
            *FB = processStand<C, core::PTK_Silver, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandSilver3) {
            *FB = processStand<C, core::PTK_Silver, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandSilver4) {
            *FB = processStand<C, core::PTK_Silver, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandGold1) {
            *FB = processStand<C, core::PTK_Gold, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandGold2) {
            *FB = processStand<C, core::PTK_Gold, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandGold3) {
            *FB = processStand<C, core::PTK_Gold, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandGold4) {
            *FB = processStand<C, core::PTK_Gold, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandBishop1) {
            *FB = processStand<C, core::PTK_Bishop, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandBishop2) {
            *FB = processStand<C, core::PTK_Bishop, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandRook1) {
            *FB = processStand<C, core::PTK_Rook, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyStandRook2) {
            *FB = processStand<C, core::PTK_Rook, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn1) {
            *FB = processStand<~C, core::PTK_Pawn, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn2) {
            *FB = processStand<~C, core::PTK_Pawn, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn3) {
            *FB = processStand<~C, core::PTK_Pawn, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn4) {
            *FB = processStand<~C, core::PTK_Pawn, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn5) {
            *FB = processStand<~C, core::PTK_Pawn, 5>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn6) {
            *FB = processStand<~C, core::PTK_Pawn, 6>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn7) {
            *FB = processStand<~C, core::PTK_Pawn, 7>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn8) {
            *FB = processStand<~C, core::PTK_Pawn, 8>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandPawn9) {
            *FB = processStand<~C, core::PTK_Pawn, 9>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandLance1) {
            *FB = processStand<~C, core::PTK_Lance, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandLance2) {
            *FB = processStand<~C, core::PTK_Lance, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandLance3) {
            *FB = processStand<~C, core::PTK_Lance, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandLance4) {
            *FB = processStand<~C, core::PTK_Lance, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandKnight1) {
            *FB = processStand<~C, core::PTK_Knight, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandKnight2) {
            *FB = processStand<~C, core::PTK_Knight, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandKnight3) {
            *FB = processStand<~C, core::PTK_Knight, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandKnight4) {
            *FB = processStand<~C, core::PTK_Knight, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandSilver1) {
            *FB = processStand<~C, core::PTK_Silver, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandSilver2) {
            *FB = processStand<~C, core::PTK_Silver, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandSilver3) {
            *FB = processStand<~C, core::PTK_Silver, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandSilver4) {
            *FB = processStand<~C, core::PTK_Silver, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandGold1) {
            *FB = processStand<~C, core::PTK_Gold, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandGold2) {
            *FB = processStand<~C, core::PTK_Gold, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandGold3) {
            *FB = processStand<~C, core::PTK_Gold, 3>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandGold4) {
            *FB = processStand<~C, core::PTK_Gold, 4>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandBishop1) {
            *FB = processStand<~C, core::PTK_Bishop, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandBishop2) {
            *FB = processStand<~C, core::PTK_Bishop, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandRook1) {
            *FB = processStand<~C, core::PTK_Rook, 1>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpStandRook2) {
            *FB = processStand<~C, core::PTK_Rook, 2>(State);
        } else if constexpr (HeadType == FeatureType::FT_RuleDeclare27) {
            *FB = processRule<core::EndingRule::ER_Declare27>(Config);
        } else if constexpr (HeadType == FeatureType::FT_RuleDraw24) {
            *FB = processRule<core::EndingRule::ER_Draw24>(Config);
        } else if constexpr (HeadType == FeatureType::FT_RuleTrying) {
            *FB = processRule<core::EndingRule::ER_Trying>(Config);
        } else if constexpr (HeadType == FeatureType::FT_MyDrawValue) {
            *FB = processDrawValue<C>(Config);
        } else if constexpr (HeadType == FeatureType::FT_OpDrawValue) {
            *FB = processDrawValue<~C>(Config);
        } else if constexpr (HeadType == FeatureType::FT_MyDeclarationScore) {
            *FB = processDeclarationScore<C>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpDeclarationScore) {
            *FB = processDeclarationScore<~C>(State);
        } else if constexpr (HeadType == FeatureType::FT_MyPieceScore) {
            *FB = processPieceScore<C>(State);
        } else if constexpr (HeadType == FeatureType::FT_OpPieceScore) {
            *FB = processPieceScore<~C>(State);
        } else {
            // A trick for check if `FeatureType` is handled in
            // constexpr-if blocks above.
            // https://stackoverflow.com/a/64354296
            []<bool Flag = false>()
                {static_assert(Flag, "Unhandled FeatureType");}();
        }

        process<C, TailType...>(State, Config, FB + 1);
    }

    std::vector<FeatureBitboard> Features;
};

struct FeatureStackRuntime : FeatureStack {
 public:
    FeatureStackRuntime(const std::vector<FeatureType>& Types, const core::State&, const core::StateConfig&);
    ~FeatureStackRuntime() = default;

    const FeatureBitboard& get(std::size_t Index) const;

    template<core::IterateOrder Order = core::IterateOrder::NWSE>
    std::vector<float> extract() const {
        std::vector<float> Dest(Features.size() * core::NumSquares);

        extract<Order>(Dest.data());

        return Dest;
    }

    template<core::IterateOrder Order = core::IterateOrder::NWSE>
    void extract(float* Dest) const {
        for (std::size_t Channel = 0; Channel < Features.size(); ++Channel) {
            Features[Channel].extract<Order>(Dest + Channel * core::NumSquares);
        }
    }

    std::size_t size() const {
        return Features.size();
    }

 private:
    template<core::Color C>
    void process(const std::vector<FeatureType>& Types, const core::State& State, const core::StateConfig& Config);

    std::vector<FeatureBitboard> Features;
};


} // namespace ml
} // namespace nshogi


#endif // #ifndef NSHOGI_ML_FEATURESTACK_H
