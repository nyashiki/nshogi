//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "feature.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../core/state.h"
#include "../core/stateconfig.h"
#include "../ml/common.h"
#include "../ml/featurestack.h"
#include "../ml/ka.h"
#include "../ml/p.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

namespace {

class PyFeatureStack {
 public:
    PyFeatureStack(const std::vector<nshogi::ml::FeatureType>& Types,
                   const nshogi::core::State& S,
                   const nshogi::core::StateConfig& Config)
        : FeatureStack(Types, S, Config) {
    }

    pybind11::array_t<float> to_numpy(bool ChannelsFirst) const {
        auto NpArray = pybind11::array_t<float>(
            (pybind11::ssize_t)nshogi::core::NumSquares *
            (pybind11::ssize_t)FeatureStack.size());
        auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

        if (ChannelsFirst) {
            FeatureStack.extract<nshogi::core::IterateOrder::Fastest, true>(
                Data);
            NpArray.resize({(pybind11::ssize_t)FeatureStack.size(),
                            (pybind11::ssize_t)9, (pybind11::ssize_t)9});
        } else {
            FeatureStack.extract<nshogi::core::IterateOrder::Fastest, false>(
                Data);
            NpArray.resize({(pybind11::ssize_t)9, (pybind11::ssize_t)9,
                            (pybind11::ssize_t)FeatureStack.size()});
        }

        return NpArray;
    }

 private:
    nshogi::ml::FeatureStackRuntime FeatureStack;
};

template <typename ExtractorType>
pybind11::tuple extractorIdsToNumpy(const ExtractorType& Extractor,
                                    const nshogi::core::State& State) {
    const auto& [MyIds, OpIds] = Extractor.ids(State);

    auto MyArray = pybind11::array_t<int32_t>((pybind11::ssize_t)MyIds.size());
    std::memcpy(MyArray.request().ptr, MyIds.data(),
                MyIds.size() * sizeof(int32_t));

    auto OpArray = pybind11::array_t<int32_t>((pybind11::ssize_t)OpIds.size());
    std::memcpy(OpArray.request().ptr, OpIds.data(),
                OpIds.size() * sizeof(int32_t));

    return pybind11::make_tuple(MyArray, OpArray);
}

} // namespace

namespace nshogi {
namespace python {
namespace feature {

void bind(pybind11::module_& Module) {
    pybind11::enum_<nshogi::ml::FeatureType>(Module, "FeatureType")
        .value("BLACK", nshogi::ml::FeatureType::FT_Black)
        .value("WHITE", nshogi::ml::FeatureType::FT_White)
        .value("MY_PAWN", nshogi::ml::FeatureType::FT_MyPawn)
        .value("MY_LANCE", nshogi::ml::FeatureType::FT_MyLance)
        .value("MY_KNIGHT", nshogi::ml::FeatureType::FT_MyKnight)
        .value("MY_SILVER", nshogi::ml::FeatureType::FT_MySilver)
        .value("MY_GOLD", nshogi::ml::FeatureType::FT_MyGold)
        .value("MY_KING", nshogi::ml::FeatureType::FT_MyKing)
        .value("MY_BISHOP", nshogi::ml::FeatureType::FT_MyBishop)
        .value("MY_ROOK", nshogi::ml::FeatureType::FT_MyRook)
        .value("MY_PROPAWN", nshogi::ml::FeatureType::FT_MyProPawn)
        .value("MY_PROLANCE", nshogi::ml::FeatureType::FT_MyProLance)
        .value("MY_PROKNIGHT", nshogi::ml::FeatureType::FT_MyProKnight)
        .value("MY_PROSILVER", nshogi::ml::FeatureType::FT_MyProSilver)
        .value("MY_PROBISHOP", nshogi::ml::FeatureType::FT_MyProBishop)
        .value("MY_PROROOK", nshogi::ml::FeatureType::FT_MyProRook)
        .value("MY_BISHOP_AND_PROBISHOP",
               nshogi::ml::FeatureType::FT_MyBishopAndProBishop)
        .value("MY_ROOK_AND_PROROOK",
               nshogi::ml::FeatureType::FT_MyRookAndProRook)
        .value("OP_PAWN", nshogi::ml::FeatureType::FT_OpPawn)
        .value("OP_LANCE", nshogi::ml::FeatureType::FT_OpLance)
        .value("OP_KNIGHT", nshogi::ml::FeatureType::FT_OpKnight)
        .value("OP_SILVER", nshogi::ml::FeatureType::FT_OpSilver)
        .value("OP_GOLD", nshogi::ml::FeatureType::FT_OpGold)
        .value("OP_KING", nshogi::ml::FeatureType::FT_OpKing)
        .value("OP_BISHOP", nshogi::ml::FeatureType::FT_OpBishop)
        .value("OP_ROOK", nshogi::ml::FeatureType::FT_OpRook)
        .value("OP_PROPAWN", nshogi::ml::FeatureType::FT_OpProPawn)
        .value("OP_PROLANCE", nshogi::ml::FeatureType::FT_OpProLance)
        .value("OP_PROKNIGHT", nshogi::ml::FeatureType::FT_OpProKnight)
        .value("OP_PROSILVER", nshogi::ml::FeatureType::FT_OpProSilver)
        .value("OP_PROBISHOP", nshogi::ml::FeatureType::FT_OpProBishop)
        .value("OP_PROROOK", nshogi::ml::FeatureType::FT_OpProRook)
        .value("OP_BISHOP_AND_PROBISHOP",
               nshogi::ml::FeatureType::FT_OpBishopAndProBishop)
        .value("OP_ROOK_AND_PROROOK",
               nshogi::ml::FeatureType::FT_OpRookAndProRook)
        .value("MY_STAND_PAWN_1", nshogi::ml::FeatureType::FT_MyStandPawn1)
        .value("MY_STAND_PAWN_2", nshogi::ml::FeatureType::FT_MyStandPawn2)
        .value("MY_STAND_PAWN_3", nshogi::ml::FeatureType::FT_MyStandPawn3)
        .value("MY_STAND_PAWN_4", nshogi::ml::FeatureType::FT_MyStandPawn4)
        .value("MY_STAND_PAWN_5", nshogi::ml::FeatureType::FT_MyStandPawn5)
        .value("MY_STAND_PAWN_6", nshogi::ml::FeatureType::FT_MyStandPawn6)
        .value("MY_STAND_PAWN_7", nshogi::ml::FeatureType::FT_MyStandPawn7)
        .value("MY_STAND_PAWN_8", nshogi::ml::FeatureType::FT_MyStandPawn8)
        .value("MY_STAND_PAWN_9", nshogi::ml::FeatureType::FT_MyStandPawn9)
        .value("MY_STAND_LANCE_1", nshogi::ml::FeatureType::FT_MyStandLance1)
        .value("MY_STAND_LANCE_2", nshogi::ml::FeatureType::FT_MyStandLance2)
        .value("MY_STAND_LANCE_3", nshogi::ml::FeatureType::FT_MyStandLance3)
        .value("MY_STAND_LANCE_4", nshogi::ml::FeatureType::FT_MyStandLance4)
        .value("MY_STAND_KNIGHT_1", nshogi::ml::FeatureType::FT_MyStandKnight1)
        .value("MY_STAND_KNIGHT_2", nshogi::ml::FeatureType::FT_MyStandKnight2)
        .value("MY_STAND_KNIGHT_3", nshogi::ml::FeatureType::FT_MyStandKnight3)
        .value("MY_STAND_KNIGHT_4", nshogi::ml::FeatureType::FT_MyStandKnight4)
        .value("MY_STAND_SILVER_1", nshogi::ml::FeatureType::FT_MyStandSilver1)
        .value("MY_STAND_SILVER_2", nshogi::ml::FeatureType::FT_MyStandSilver2)
        .value("MY_STAND_SILVER_3", nshogi::ml::FeatureType::FT_MyStandSilver3)
        .value("MY_STAND_SILVER_4", nshogi::ml::FeatureType::FT_MyStandSilver4)
        .value("MY_STAND_GOLD_1", nshogi::ml::FeatureType::FT_MyStandGold1)
        .value("MY_STAND_GOLD_2", nshogi::ml::FeatureType::FT_MyStandGold2)
        .value("MY_STAND_GOLD_3", nshogi::ml::FeatureType::FT_MyStandGold3)
        .value("MY_STAND_GOLD_4", nshogi::ml::FeatureType::FT_MyStandGold4)
        .value("MY_STAND_BISHOP_1", nshogi::ml::FeatureType::FT_MyStandBishop1)
        .value("MY_STAND_BISHOP_2", nshogi::ml::FeatureType::FT_MyStandBishop2)
        .value("MY_STAND_ROOK_1", nshogi::ml::FeatureType::FT_MyStandRook1)
        .value("MY_STAND_ROOK_2", nshogi::ml::FeatureType::FT_MyStandRook2)
        .value("OP_STAND_PAWN_1", nshogi::ml::FeatureType::FT_OpStandPawn1)
        .value("OP_STAND_PAWN_2", nshogi::ml::FeatureType::FT_OpStandPawn2)
        .value("OP_STAND_PAWN_3", nshogi::ml::FeatureType::FT_OpStandPawn3)
        .value("OP_STAND_PAWN_4", nshogi::ml::FeatureType::FT_OpStandPawn4)
        .value("OP_STAND_PAWN_5", nshogi::ml::FeatureType::FT_OpStandPawn5)
        .value("OP_STAND_PAWN_6", nshogi::ml::FeatureType::FT_OpStandPawn6)
        .value("OP_STAND_PAWN_7", nshogi::ml::FeatureType::FT_OpStandPawn7)
        .value("OP_STAND_PAWN_8", nshogi::ml::FeatureType::FT_OpStandPawn8)
        .value("OP_STAND_PAWN_9", nshogi::ml::FeatureType::FT_OpStandPawn9)
        .value("OP_STAND_LANCE_1", nshogi::ml::FeatureType::FT_OpStandLance1)
        .value("OP_STAND_LANCE_2", nshogi::ml::FeatureType::FT_OpStandLance2)
        .value("OP_STAND_LANCE_3", nshogi::ml::FeatureType::FT_OpStandLance3)
        .value("OP_STAND_LANCE_4", nshogi::ml::FeatureType::FT_OpStandLance4)
        .value("OP_STAND_KNIGHT_1", nshogi::ml::FeatureType::FT_OpStandKnight1)
        .value("OP_STAND_KNIGHT_2", nshogi::ml::FeatureType::FT_OpStandKnight2)
        .value("OP_STAND_KNIGHT_3", nshogi::ml::FeatureType::FT_OpStandKnight3)
        .value("OP_STAND_KNIGHT_4", nshogi::ml::FeatureType::FT_OpStandKnight4)
        .value("OP_STAND_SILVER_1", nshogi::ml::FeatureType::FT_OpStandSilver1)
        .value("OP_STAND_SILVER_2", nshogi::ml::FeatureType::FT_OpStandSilver2)
        .value("OP_STAND_SILVER_3", nshogi::ml::FeatureType::FT_OpStandSilver3)
        .value("OP_STAND_SILVER_4", nshogi::ml::FeatureType::FT_OpStandSilver4)
        .value("OP_STAND_GOLD_1", nshogi::ml::FeatureType::FT_OpStandGold1)
        .value("OP_STAND_GOLD_2", nshogi::ml::FeatureType::FT_OpStandGold2)
        .value("OP_STAND_GOLD_3", nshogi::ml::FeatureType::FT_OpStandGold3)
        .value("OP_STAND_GOLD_4", nshogi::ml::FeatureType::FT_OpStandGold4)
        .value("OP_STAND_BISHOP_1", nshogi::ml::FeatureType::FT_OpStandBishop1)
        .value("OP_STAND_BISHOP_2", nshogi::ml::FeatureType::FT_OpStandBishop2)
        .value("OP_STAND_ROOK_1", nshogi::ml::FeatureType::FT_OpStandRook1)
        .value("OP_STAND_ROOK_2", nshogi::ml::FeatureType::FT_OpStandRook2)
        .value("CHECK", nshogi::ml::FeatureType::FT_Check)
        .value("NO_MY_PAWN_FILE", nshogi::ml::FeatureType::FT_NoMyPawnFile)
        .value("NO_OP_PAWN_FILE", nshogi::ml::FeatureType::FT_NoOpPawnFile)
        .value("PROGRESS", nshogi::ml::FeatureType::FT_Progress)
        .value("PROGRESS_UNIT", nshogi::ml::FeatureType::FT_ProgressUnit)
        .value("MY_DRAW_VALUE", nshogi::ml::FeatureType::FT_MyDrawValue)
        .value("OP_DRAW_VALUE", nshogi::ml::FeatureType::FT_OpDrawValue)
        .value("MY_DECLARATION_SCORE",
               nshogi::ml::FeatureType::FT_MyDeclarationScore)
        .value("OP_DECLARATION_SCORE",
               nshogi::ml::FeatureType::FT_OpDeclarationScore)
        .value("MY_PIECE_SCORE", nshogi::ml::FeatureType::FT_MyPieceScore)
        .value("OP_PIECE_SCORE", nshogi::ml::FeatureType::FT_OpPieceScore)
        .value("MY_ATTACK", nshogi::ml::FeatureType::FT_MyAttack)
        .value("OP_ATTACK", nshogi::ml::FeatureType::FT_OpAttack)
        .value("MY_DECLARATION_SCORE_REMAINING",
               nshogi::ml::FeatureType::FT_MyDeclarationRemaining)
        .value("OP_DECLARATION_SCORE_REMAINING",
               nshogi::ml::FeatureType::FT_OpDeclarationRemaining);

    Module.def(
        "move_to_index",
        [](nshogi::core::Color Color, const nshogi::core::Move32 Move,
           bool ChannlesFirst) -> std::size_t {
            if (ChannlesFirst) {
                return nshogi::ml::getMoveIndex<true>(Color, Move);
            } else {
                return nshogi::ml::getMoveIndex<false>(Color, Move);
            }
        },
        pybind11::arg("color"), pybind11::arg("move"),
        pybind11::arg("channels_first"));

    pybind11::class_<PyFeatureStack>(Module, "FeatureStack")
        .def(pybind11::init<const std::vector<nshogi::ml::FeatureType>&,
                            const nshogi::core::State&,
                            const nshogi::core::StateConfig&>())
        .def("to_numpy", &PyFeatureStack::to_numpy,
             pybind11::arg("channels_first") = true);

    pybind11::class_<nshogi::ml::IFeatureExtractor,
                     std::shared_ptr<nshogi::ml::IFeatureExtractor>>(
        Module, "IFeatureExtractor");

    pybind11::class_<nshogi::ml::KAFeatureExtractor,
                     nshogi::ml::IFeatureExtractor,
                     std::shared_ptr<nshogi::ml::KAFeatureExtractor>>(
        Module, "KAFeatureExtractor")
        .def(pybind11::init<>())
        .def("ids", &extractorIdsToNumpy<nshogi::ml::KAFeatureExtractor>);

    pybind11::class_<nshogi::ml::PFeatureExtractor,
                     nshogi::ml::IFeatureExtractor,
                     std::shared_ptr<nshogi::ml::PFeatureExtractor>>(
        Module, "PFeatureExtractor")
        .def(pybind11::init<>())
        .def("ids", &extractorIdsToNumpy<nshogi::ml::PFeatureExtractor>);
}

} // namespace feature
} // namespace python
} // namespace nshogi
