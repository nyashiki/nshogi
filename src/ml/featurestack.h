//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_FEATURESTACK_H
#define NSHOGI_ML_FEATURESTACK_H

#include "../core/state.h"
#include "../core/stateconfig.h"
#include "featurebitboard.h"
#include "featuretype.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

namespace nshogi {
namespace ml {

template <core::Color C>
FeatureBitboard processIsBlackTurn();

template <core::Color C>
FeatureBitboard processIsWhiteTurn();

FeatureBitboard processCheck(const core::State& State);

template <core::Color MyColor, core::Color TargetColor>
FeatureBitboard processNoPawnFile(const core::State& State);

FeatureBitboard processProgress(const core::State& State,
                                const core::StateConfig& Config);

FeatureBitboard processProgressUnit(const core::StateConfig& Config);

template <core::Color MyColor, core::Color PieceColor>
FeatureBitboard processPiece(const core::State& State,
                             core::PieceTypeKind Type);

template <core::Color MyColor, core::Color PieceColor>
FeatureBitboard processPiece(const core::State& State,
                             core::PieceTypeKind Type1,
                             core::PieceTypeKind Type2);

template <core::Color MyColor>
FeatureBitboard processStand(const core::State& State, core::PieceTypeKind Type,
                             uint8_t Count);

template <core::EndingRule Rule>
FeatureBitboard processRule(const core::StateConfig& Config);

template <core::Color MyColor>
FeatureBitboard processDrawValue(const core::StateConfig& Config);

template <core::Color C>
FeatureBitboard processDeclarationScore(const core::State& State);

template <core::Color C>
FeatureBitboard processPieceScore(const core::State& State);

template <core::Color MyColor, core::Color TargetColor>
FeatureBitboard processAttack(const core::State& State);

template <core::Color MyColor>
FeatureBitboard processDeclarationRemaining(const core::State& State);

// Computes a single feature plane for a feature type known at compile
// time. The side to move `C` must match `State`.
template <core::Color C, FeatureType FT>
FeatureBitboard processFeature(const core::State& State,
                               const core::StateConfig& Config) {
    if constexpr (FT == FeatureType::FT_Black) {
        return processIsBlackTurn<C>();
    } else if constexpr (FT == FeatureType::FT_White) {
        return processIsWhiteTurn<C>();
    } else if constexpr (FT == FeatureType::FT_Check) {
        return processCheck(State);
    } else if constexpr (FT == FeatureType::FT_NoMyPawnFile) {
        return processNoPawnFile<C, C>(State);
    } else if constexpr (FT == FeatureType::FT_NoOpPawnFile) {
        return processNoPawnFile<C, ~C>(State);
    } else if constexpr (FT == FeatureType::FT_Progress) {
        return processProgress(State, Config);
    } else if constexpr (FT == FeatureType::FT_ProgressUnit) {
        return processProgressUnit(Config);
    } else if constexpr (FT == FeatureType::FT_MyPawn) {
        return processPiece<C, C>(State, core::PTK_Pawn);
    } else if constexpr (FT == FeatureType::FT_MyLance) {
        return processPiece<C, C>(State, core::PTK_Lance);
    } else if constexpr (FT == FeatureType::FT_MyKnight) {
        return processPiece<C, C>(State, core::PTK_Knight);
    } else if constexpr (FT == FeatureType::FT_MySilver) {
        return processPiece<C, C>(State, core::PTK_Silver);
    } else if constexpr (FT == FeatureType::FT_MyGold) {
        return processPiece<C, C>(State, core::PTK_Gold);
    } else if constexpr (FT == FeatureType::FT_MyKing) {
        return processPiece<C, C>(State, core::PTK_King);
    } else if constexpr (FT == FeatureType::FT_MyBishop) {
        return processPiece<C, C>(State, core::PTK_Bishop);
    } else if constexpr (FT == FeatureType::FT_MyRook) {
        return processPiece<C, C>(State, core::PTK_Rook);
    } else if constexpr (FT == FeatureType::FT_MyProPawn) {
        return processPiece<C, C>(State, core::PTK_ProPawn);
    } else if constexpr (FT == FeatureType::FT_MyProLance) {
        return processPiece<C, C>(State, core::PTK_ProLance);
    } else if constexpr (FT == FeatureType::FT_MyProKnight) {
        return processPiece<C, C>(State, core::PTK_ProKnight);
    } else if constexpr (FT == FeatureType::FT_MyProSilver) {
        return processPiece<C, C>(State, core::PTK_ProSilver);
    } else if constexpr (FT == FeatureType::FT_MyProBishop) {
        return processPiece<C, C>(State, core::PTK_ProBishop);
    } else if constexpr (FT == FeatureType::FT_MyProRook) {
        return processPiece<C, C>(State, core::PTK_ProRook);
    } else if constexpr (FT == FeatureType::FT_MyBishopAndProBishop) {
        return processPiece<C, C>(State, core::PTK_Bishop,
                                  core::PTK_ProBishop);
    } else if constexpr (FT == FeatureType::FT_MyRookAndProRook) {
        return processPiece<C, C>(State, core::PTK_Rook, core::PTK_ProRook);
    } else if constexpr (FT == FeatureType::FT_OpPawn) {
        return processPiece<C, ~C>(State, core::PTK_Pawn);
    } else if constexpr (FT == FeatureType::FT_OpLance) {
        return processPiece<C, ~C>(State, core::PTK_Lance);
    } else if constexpr (FT == FeatureType::FT_OpKnight) {
        return processPiece<C, ~C>(State, core::PTK_Knight);
    } else if constexpr (FT == FeatureType::FT_OpSilver) {
        return processPiece<C, ~C>(State, core::PTK_Silver);
    } else if constexpr (FT == FeatureType::FT_OpGold) {
        return processPiece<C, ~C>(State, core::PTK_Gold);
    } else if constexpr (FT == FeatureType::FT_OpKing) {
        return processPiece<C, ~C>(State, core::PTK_King);
    } else if constexpr (FT == FeatureType::FT_OpBishop) {
        return processPiece<C, ~C>(State, core::PTK_Bishop);
    } else if constexpr (FT == FeatureType::FT_OpRook) {
        return processPiece<C, ~C>(State, core::PTK_Rook);
    } else if constexpr (FT == FeatureType::FT_OpProPawn) {
        return processPiece<C, ~C>(State, core::PTK_ProPawn);
    } else if constexpr (FT == FeatureType::FT_OpProLance) {
        return processPiece<C, ~C>(State, core::PTK_ProLance);
    } else if constexpr (FT == FeatureType::FT_OpProKnight) {
        return processPiece<C, ~C>(State, core::PTK_ProKnight);
    } else if constexpr (FT == FeatureType::FT_OpProSilver) {
        return processPiece<C, ~C>(State, core::PTK_ProSilver);
    } else if constexpr (FT == FeatureType::FT_OpProBishop) {
        return processPiece<C, ~C>(State, core::PTK_ProBishop);
    } else if constexpr (FT == FeatureType::FT_OpProRook) {
        return processPiece<C, ~C>(State, core::PTK_ProRook);
    } else if constexpr (FT == FeatureType::FT_OpBishopAndProBishop) {
        return processPiece<C, ~C>(State, core::PTK_Bishop,
                                   core::PTK_ProBishop);
    } else if constexpr (FT == FeatureType::FT_OpRookAndProRook) {
        return processPiece<C, ~C>(State, core::PTK_Rook, core::PTK_ProRook);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn1) {
        return processStand<C>(State, core::PTK_Pawn, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn2) {
        return processStand<C>(State, core::PTK_Pawn, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn3) {
        return processStand<C>(State, core::PTK_Pawn, 3);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn4) {
        return processStand<C>(State, core::PTK_Pawn, 4);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn5) {
        return processStand<C>(State, core::PTK_Pawn, 5);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn6) {
        return processStand<C>(State, core::PTK_Pawn, 6);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn7) {
        return processStand<C>(State, core::PTK_Pawn, 7);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn8) {
        return processStand<C>(State, core::PTK_Pawn, 8);
    } else if constexpr (FT == FeatureType::FT_MyStandPawn9) {
        return processStand<C>(State, core::PTK_Pawn, 9);
    } else if constexpr (FT == FeatureType::FT_MyStandLance1) {
        return processStand<C>(State, core::PTK_Lance, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandLance2) {
        return processStand<C>(State, core::PTK_Lance, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandLance3) {
        return processStand<C>(State, core::PTK_Lance, 3);
    } else if constexpr (FT == FeatureType::FT_MyStandLance4) {
        return processStand<C>(State, core::PTK_Lance, 4);
    } else if constexpr (FT == FeatureType::FT_MyStandKnight1) {
        return processStand<C>(State, core::PTK_Knight, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandKnight2) {
        return processStand<C>(State, core::PTK_Knight, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandKnight3) {
        return processStand<C>(State, core::PTK_Knight, 3);
    } else if constexpr (FT == FeatureType::FT_MyStandKnight4) {
        return processStand<C>(State, core::PTK_Knight, 4);
    } else if constexpr (FT == FeatureType::FT_MyStandSilver1) {
        return processStand<C>(State, core::PTK_Silver, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandSilver2) {
        return processStand<C>(State, core::PTK_Silver, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandSilver3) {
        return processStand<C>(State, core::PTK_Silver, 3);
    } else if constexpr (FT == FeatureType::FT_MyStandSilver4) {
        return processStand<C>(State, core::PTK_Silver, 4);
    } else if constexpr (FT == FeatureType::FT_MyStandGold1) {
        return processStand<C>(State, core::PTK_Gold, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandGold2) {
        return processStand<C>(State, core::PTK_Gold, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandGold3) {
        return processStand<C>(State, core::PTK_Gold, 3);
    } else if constexpr (FT == FeatureType::FT_MyStandGold4) {
        return processStand<C>(State, core::PTK_Gold, 4);
    } else if constexpr (FT == FeatureType::FT_MyStandBishop1) {
        return processStand<C>(State, core::PTK_Bishop, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandBishop2) {
        return processStand<C>(State, core::PTK_Bishop, 2);
    } else if constexpr (FT == FeatureType::FT_MyStandRook1) {
        return processStand<C>(State, core::PTK_Rook, 1);
    } else if constexpr (FT == FeatureType::FT_MyStandRook2) {
        return processStand<C>(State, core::PTK_Rook, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn1) {
        return processStand<~C>(State, core::PTK_Pawn, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn2) {
        return processStand<~C>(State, core::PTK_Pawn, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn3) {
        return processStand<~C>(State, core::PTK_Pawn, 3);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn4) {
        return processStand<~C>(State, core::PTK_Pawn, 4);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn5) {
        return processStand<~C>(State, core::PTK_Pawn, 5);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn6) {
        return processStand<~C>(State, core::PTK_Pawn, 6);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn7) {
        return processStand<~C>(State, core::PTK_Pawn, 7);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn8) {
        return processStand<~C>(State, core::PTK_Pawn, 8);
    } else if constexpr (FT == FeatureType::FT_OpStandPawn9) {
        return processStand<~C>(State, core::PTK_Pawn, 9);
    } else if constexpr (FT == FeatureType::FT_OpStandLance1) {
        return processStand<~C>(State, core::PTK_Lance, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandLance2) {
        return processStand<~C>(State, core::PTK_Lance, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandLance3) {
        return processStand<~C>(State, core::PTK_Lance, 3);
    } else if constexpr (FT == FeatureType::FT_OpStandLance4) {
        return processStand<~C>(State, core::PTK_Lance, 4);
    } else if constexpr (FT == FeatureType::FT_OpStandKnight1) {
        return processStand<~C>(State, core::PTK_Knight, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandKnight2) {
        return processStand<~C>(State, core::PTK_Knight, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandKnight3) {
        return processStand<~C>(State, core::PTK_Knight, 3);
    } else if constexpr (FT == FeatureType::FT_OpStandKnight4) {
        return processStand<~C>(State, core::PTK_Knight, 4);
    } else if constexpr (FT == FeatureType::FT_OpStandSilver1) {
        return processStand<~C>(State, core::PTK_Silver, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandSilver2) {
        return processStand<~C>(State, core::PTK_Silver, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandSilver3) {
        return processStand<~C>(State, core::PTK_Silver, 3);
    } else if constexpr (FT == FeatureType::FT_OpStandSilver4) {
        return processStand<~C>(State, core::PTK_Silver, 4);
    } else if constexpr (FT == FeatureType::FT_OpStandGold1) {
        return processStand<~C>(State, core::PTK_Gold, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandGold2) {
        return processStand<~C>(State, core::PTK_Gold, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandGold3) {
        return processStand<~C>(State, core::PTK_Gold, 3);
    } else if constexpr (FT == FeatureType::FT_OpStandGold4) {
        return processStand<~C>(State, core::PTK_Gold, 4);
    } else if constexpr (FT == FeatureType::FT_OpStandBishop1) {
        return processStand<~C>(State, core::PTK_Bishop, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandBishop2) {
        return processStand<~C>(State, core::PTK_Bishop, 2);
    } else if constexpr (FT == FeatureType::FT_OpStandRook1) {
        return processStand<~C>(State, core::PTK_Rook, 1);
    } else if constexpr (FT == FeatureType::FT_OpStandRook2) {
        return processStand<~C>(State, core::PTK_Rook, 2);
    } else if constexpr (FT == FeatureType::FT_RuleDeclare27) {
        return processRule<core::EndingRule::ER_Declare27>(Config);
    } else if constexpr (FT == FeatureType::FT_RuleDraw24) {
        return processRule<core::EndingRule::ER_Draw24>(Config);
    } else if constexpr (FT == FeatureType::FT_RuleTrying) {
        return processRule<core::EndingRule::ER_Trying>(Config);
    } else if constexpr (FT == FeatureType::FT_MyDrawValue) {
        return processDrawValue<C>(Config);
    } else if constexpr (FT == FeatureType::FT_OpDrawValue) {
        return processDrawValue<~C>(Config);
    } else if constexpr (FT == FeatureType::FT_MyDeclarationScore) {
        return processDeclarationScore<C>(State);
    } else if constexpr (FT == FeatureType::FT_OpDeclarationScore) {
        return processDeclarationScore<~C>(State);
    } else if constexpr (FT == FeatureType::FT_MyPieceScore) {
        return processPieceScore<C>(State);
    } else if constexpr (FT == FeatureType::FT_OpPieceScore) {
        return processPieceScore<~C>(State);
    } else if constexpr (FT == FeatureType::FT_MyAttack) {
        return processAttack<C, C>(State);
    } else if constexpr (FT == FeatureType::FT_OpAttack) {
        return processAttack<C, ~C>(State);
    } else if constexpr (FT == FeatureType::FT_MyDeclarationRemaining) {
        return processDeclarationRemaining<C>(State);
    } else if constexpr (FT == FeatureType::FT_OpDeclarationRemaining) {
        return processDeclarationRemaining<~C>(State);
    } else {
        // A trick for check if `FeatureType` is handled in
        // constexpr-if blocks above.
        // https://stackoverflow.com/a/64354296
        []<bool Flag = false>() {
            static_assert(Flag, "Unhandled FeatureType");
        }();
    }
}

struct FeatureStack {
    virtual ~FeatureStack() {
    }

    virtual const FeatureBitboard& get(std::size_t Index) const = 0;
};

template <FeatureType... FeatureTypes>
struct FeatureStackComptime : FeatureStack {
    FeatureStackComptime(const core::State& State,
                         const core::StateConfig& Config)
        : Features(sizeof...(FeatureTypes)) {
        if (State.getPosition().sideToMove() == core::Black) {
            process<core::Black, FeatureTypes...>(State, Config,
                                                  Features.data());
        } else {
            process<core::White, FeatureTypes...>(State, Config,
                                                  Features.data());
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
        return *this;
    }

    static void constructAt(FeatureBitboard* FB, const core::State& State,
                            const core::StateConfig& Config) {
        if (State.getPosition().sideToMove() == core::Black) {
            process<core::Black, FeatureTypes...>(State, Config, FB);
        } else {
            process<core::White, FeatureTypes...>(State, Config, FB);
        }
    }

    template <core::IterateOrder Order = core::IterateOrder::NWSE,
              bool ChannelsFirst = true>
    std::vector<float> extract() const {
        std::vector<float> Dest(sizeof...(FeatureTypes) * core::NumSquares);

        extract<Order, ChannelsFirst>(Dest.data());

        return Dest;
    }

    template <core::IterateOrder Order = core::IterateOrder::NWSE,
              bool ChannelsFirst = true>
    void extract(float* Dest) const {
        if constexpr (ChannelsFirst) {
            for (std::size_t Channel = 0; Channel < sizeof...(FeatureTypes);
                 ++Channel) {
                Features[Channel].template extract<Order>(
                    Dest + Channel * core::NumSquares);
            }
        } else {
            core::SquareIterator<Order> SquareIt;
            for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
                for (std::size_t Channel = 0; Channel < sizeof...(FeatureTypes);
                     ++Channel) {
                    const uint64_t* BB = reinterpret_cast<const uint64_t*>(
                        Features[Channel].data());

                    const bool IsRotated = Features[Channel].isRotated();

                    if (!IsRotated) {
                        if (*It < 63) {
                            const bool IsSet = (BB[0] & (1ULL << *It)) != 0;
                            Dest[(std::size_t)*It * Features.size() + Channel] =
                                IsSet ? Features[Channel].getValue() : 0.0f;
                        } else {
                            const bool IsSet =
                                (BB[1] & (1ULL << (*It - 63))) != 0;
                            Dest[(std::size_t)*It * Features.size() + Channel] =
                                IsSet ? Features[Channel].getValue() : 0.0f;
                        }
                    } else {
                        if (*It < 63) {
                            const bool IsSet = (BB[0] & (1ULL << *It)) != 0;
                            Dest[(std::size_t)(core::NumSquares - 1 - *It) *
                                     Features.size() +
                                 Channel] =
                                IsSet ? Features[Channel].getValue() : 0.0f;
                        } else {
                            const bool IsSet =
                                (BB[1] & (1ULL << (*It - 63))) != 0;
                            Dest[(std::size_t)(core::NumSquares - 1 - *It) *
                                     Features.size() +
                                 Channel] =
                                IsSet ? Features[Channel].getValue() : 0.0f;
                        }
                    }
                }
            }
        }
    }

    const FeatureBitboard& get(std::size_t Index) const {
        assert(Index < sizeof...(FeatureTypes));

        return Features[Index];
    }

    const FeatureBitboard* data() const {
        return Features.data();
    }

    static constexpr std::size_t size() {
        return sizeof...(FeatureTypes);
    }

 private:
    template <core::Color C>
    static void process(const core::State&, const core::StateConfig&,
                        FeatureBitboard*) {
    }

    template <core::Color C, FeatureType HeadType, FeatureType... TailType>
    static void process(const core::State& State,
                        const core::StateConfig& Config, FeatureBitboard* FB) {
        *FB = processFeature<C, HeadType>(State, Config);
        process<C, TailType...>(State, Config, FB + 1);
    }

    std::vector<FeatureBitboard> Features;
};

struct FeatureStackRuntime : FeatureStack {
 public:
    FeatureStackRuntime(const std::vector<FeatureType>& Types,
                        const core::State&, const core::StateConfig&);
    ~FeatureStackRuntime() = default;

    const FeatureBitboard& get(std::size_t Index) const;

    template <core::IterateOrder Order = core::IterateOrder::NWSE,
              bool ChannelsFirst = true>
    std::vector<float> extract() const {
        std::vector<float> Dest(Features.size() * core::NumSquares);

        extract<Order, ChannelsFirst>(Dest.data());

        return Dest;
    }

    template <core::IterateOrder Order = core::IterateOrder::NWSE,
              bool ChannelsFirst = true>
        requires(ChannelsFirst)
    void extract(float* Dest) const {
        for (std::size_t Channel = 0; Channel < Features.size(); ++Channel) {
            Features[Channel].extract<Order>(Dest + Channel * core::NumSquares);
        }
    }

    template <core::IterateOrder Order = core::IterateOrder::NWSE,
              bool ChannelsFirst = true>
        requires(!ChannelsFirst)
    void extract(float* Dest) const;

    std::size_t size() const {
        return Features.size();
    }

 private:
    template <core::Color C>
    void process(const std::vector<FeatureType>& Types,
                 const core::State& State, const core::StateConfig& Config);

    std::vector<FeatureBitboard> Features;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_FEATURESTACK_H
