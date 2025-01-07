//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include <cstddef>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../core/initializer.h"
#include "../core/internal/bitboard.h"
#include "../core/internal/stateadapter.h"
#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/stateconfig.h"

#include "../io/csa.h"
#include "../io/file.h"
#include "../io/sfen.h"

#include "../ml/azteacher.h"
#include "../ml/common.h"
#include "../ml/featurestack.h"
#include "../ml/internal/featurebitboardutil.h"
#include "../ml/teacherloader.h"
#include "../ml/teacherwriter.h"

#include <string>
#include <vector>

namespace {

class PyFeatureStack {
 public:
    PyFeatureStack(const std::vector<nshogi::ml::FeatureType>& Types,
                   const nshogi::core::State& S,
                   const nshogi::core::StateConfig& Config)
        : FeatureStack(Types, S, Config) {
    }

    pybind11::array_t<float> to_numpy() const {
        auto NpArray = pybind11::array_t<float>(
            (pybind11::ssize_t)nshogi::core::NumSquares *
            (pybind11::ssize_t)FeatureStack.size());
        auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

        FeatureStack.extract<nshogi::core::IterateOrder::Fastest>(Data);

        return NpArray;
    }

 private:
    nshogi::ml::FeatureStackRuntime FeatureStack;
};

} // namespace

PYBIND11_MODULE(nshogi, Module) {
    Module.doc() = "";
    Module.attr("__version__") = VERSION;

    nshogi::core::initializer::initializeAll();

    pybind11::enum_<nshogi::core::Color>(Module, "Color")
        .value("BLACK", nshogi::core::Black)
        .value("WHITE", nshogi::core::White);

    pybind11::enum_<nshogi::core::Square>(Module, "Square");

    pybind11::class_<nshogi::core::Move32>(Module, "Move")
        .def_property_readonly("from", &nshogi::core::Move32::from)
        .def_property_readonly("to", &nshogi::core::Move32::to)
        .def_property_readonly("piece_type", &nshogi::core::Move32::pieceType)
        .def_property_readonly("capture_piece_type",
                               &nshogi::core::Move32::capturePieceType)
        .def_property_readonly("drop", &nshogi::core::Move32::drop)
        .def_property_readonly("promote", &nshogi::core::Move32::promote);

    pybind11::enum_<nshogi::core::PieceTypeKind>(Module, "PieceType")
        .value("EMPTY", nshogi::core::PTK_Empty)
        .value("PAWN", nshogi::core::PTK_Pawn)
        .value("LANCE", nshogi::core::PTK_Lance)
        .value("KNIGHT", nshogi::core::PTK_Knight)
        .value("SILVER", nshogi::core::PTK_Silver)
        .value("BISHOP", nshogi::core::PTK_Bishop)
        .value("ROOK", nshogi::core::PTK_Rook)
        .value("GOLD", nshogi::core::PTK_Gold)
        .value("KING", nshogi::core::PTK_King)
        .value("PRO_PAWN", nshogi::core::PTK_ProPawn)
        .value("PRO_LANCE", nshogi::core::PTK_ProLance)
        .value("PRO_KNIGHT", nshogi::core::PTK_ProKnight)
        .value("PRO_SILVER", nshogi::core::PTK_ProSilver)
        .value("PRO_BISHOP", nshogi::core::PTK_ProBishop)
        .value("PRO_ROOK", nshogi::core::PTK_ProRook);

    pybind11::enum_<nshogi::core::PieceKind>(Module, "Piece")
        .value("EMPTY", nshogi::core::PK_Empty)
        .value("BLACK_PAWN", nshogi::core::PK_BlackPawn)
        .value("BLACK_LANCE", nshogi::core::PK_BlackLance)
        .value("BLACK_KNIGHT", nshogi::core::PK_BlackKnight)
        .value("BLACK_SILVER", nshogi::core::PK_BlackSilver)
        .value("BLACK_BISHOP", nshogi::core::PK_BlackBishop)
        .value("BLACK_ROOK", nshogi::core::PK_BlackRook)
        .value("BLACK_GOLD", nshogi::core::PK_BlackGold)
        .value("BLACK_KING", nshogi::core::PK_BlackKing)
        .value("BLACK_PRO_PAWN", nshogi::core::PK_BlackProPawn)
        .value("BLACK_PRO_LANCE", nshogi::core::PK_BlackProLance)
        .value("BLACK_PRO_KNIGHT", nshogi::core::PK_BlackProKnight)
        .value("BLACK_PRO_SILVER", nshogi::core::PK_BlackProSilver)
        .value("BLACK_PRO_BISHOP", nshogi::core::PK_BlackProBishop)
        .value("BLACK_PRO_ROOK", nshogi::core::PK_BlackProRook)
        .value("WHITE_PAWN", nshogi::core::PK_WhitePawn)
        .value("WHITE_LANCE", nshogi::core::PK_WhiteLance)
        .value("WHITE_KNIGHT", nshogi::core::PK_WhiteKnight)
        .value("WHITE_SILVER", nshogi::core::PK_WhiteSilver)
        .value("WHITE_BISHOP", nshogi::core::PK_WhiteBishop)
        .value("WHITE_ROOK", nshogi::core::PK_WhiteRook)
        .value("WHITE_GOLD", nshogi::core::PK_WhiteGold)
        .value("WHITE_KING", nshogi::core::PK_WhiteKing)
        .value("WHITE_PRO_PAWN", nshogi::core::PK_WhiteProPawn)
        .value("WHITE_PRO_LANCE", nshogi::core::PK_WhiteProLance)
        .value("WHITE_PRO_KNIGHT", nshogi::core::PK_WhiteProKnight)
        .value("WHITE_PRO_SILVER", nshogi::core::PK_WhiteProSilver)
        .value("WHITE_PRO_BISHOP", nshogi::core::PK_WhiteProBishop)
        .value("WHITE_PRO_ROOK", nshogi::core::PK_WhiteProRook);

    pybind11::class_<nshogi::core::Position>(Module, "Position")
        .def("get_piece_on", &nshogi::core::Position::pieceOn)
        .def("get_stand_count",
             [](const nshogi::core::Position& Pos, nshogi::core::Color C,
                nshogi::core::PieceTypeKind Type) {
                 return Pos.getStandCount(C, Type);
             })
        .def("to_sfen",
             [](const nshogi::core::Position& Pos) {
                 return nshogi::io::sfen::positionToSfen(Pos);
             })
        .def("to_csa",
             [](const nshogi::core::Position& Pos) {
                 return nshogi::io::csa::positionToCSA(Pos);
             })
        .def_property_readonly("side_to_move",
                               &nshogi::core::Position::sideToMove);

    pybind11::class_<nshogi::core::State>(Module, "State")
        .def("clone", &nshogi::core::State::clone)
        .def("do_move",
             static_cast<void (nshogi::core::State::*)(nshogi::core::Move32)>(
                 &nshogi::core::State::doMove))
        .def("undo_move", static_cast<void (nshogi::core::State::*)()>(
                              &nshogi::core::State::undoMove))
        .def("to_sfen",
             [](const nshogi::core::State& S) {
                 return nshogi::io::sfen::stateToSfen(S);
             })
        .def("get_stand_count",
             [](const nshogi::core::State& State, nshogi::core::Color C,
                nshogi::core::PieceTypeKind Type) {
                 return State.getStandCount(C, Type);
             })
        .def("get_piece_on",
             [](const nshogi::core::State& S, int Square) {
                 return S.getPosition().pieceOn(nshogi::core::Square(Square));
             })
        .def("get_repetition",
             [](const nshogi::core::State& S, bool Strict) {
                 return S.getRepetitionStatus(Strict);
             })
        .def_property_readonly(
            "legal_moves",
            [](const nshogi::core::State& S) {
                auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(S);

                return std::vector<nshogi::core::Move32>(Moves.begin(),
                                                         Moves.end());
            })
        .def_property_readonly(
            "is_in_check",
            [](const nshogi::core::State& S) {
                nshogi::core::internal::ImmutableStateAdapter Adapter(S);
                return !Adapter->getCheckerBB().isZero();
            })
        .def_property_readonly(
            "ply", [](const nshogi::core::State& S) { return S.getPly(true); })
        .def_property_readonly("last_move", &nshogi::core::State::getLastMove)
        .def_property_readonly("hash", &nshogi::core::State::getHash)
        .def_property_readonly("side_to_move",
                               &nshogi::core::State::getSideToMove)
        .def_property_readonly("initial_position",
                               &nshogi::core::State::getInitialPosition)
        .def_property_readonly("position", &nshogi::core::State::getPosition)
        .def_property_readonly("history", [](const nshogi::core::State& S) {
            std::vector<nshogi::core::Move32> Moves;

            for (uint16_t Ply = 0; Ply < S.getPly(); ++Ply) {
                Moves.push_back(S.getHistoryMove(Ply));
            }

            return Moves;
        });

    pybind11::class_<nshogi::core::StateConfig>(Module, "StateConfig")
        .def(pybind11::init<>())
        .def_readwrite("max_ply", &nshogi::core::StateConfig::MaxPly)
        .def_readwrite("black_draw_value",
                       &nshogi::core::StateConfig::BlackDrawValue)
        .def_readwrite("white_draw_value",
                       &nshogi::core::StateConfig::WhiteDrawValue);

    auto IOModule = Module.def_submodule("io");
    auto SfenModule = IOModule.def_submodule("sfen");

    SfenModule.def("make_state_from_sfen", [](const std::string& Sfen) {
        return nshogi::io::sfen::StateBuilder::newState(Sfen);
    });

    SfenModule.def("stringify", [](nshogi::core::Move32 Move) {
        return nshogi::io::sfen::move32ToSfen(Move);
    });

    SfenModule.def("move_from_sfen", [](const nshogi::core::State& State,
                                        const std::string& Sfen) {
        return nshogi::io::sfen::sfenToMove32(State.getPosition(), Sfen);
    });

    auto CSAModule = IOModule.def_submodule("csa");

    CSAModule.def("make_state_from_csa", [](const std::string& CSA) {
        return nshogi::io::csa::StateBuilder::newState(CSA);
    });

    CSAModule.def("stringify", [](nshogi::core::Move32 Move,
                                  const nshogi::core::Color Color) {
        return nshogi::io::csa::move32ToCSA(Move, Color);
    });

    CSAModule.def("move_from_csa", [](const nshogi::core::State& State,
                                      const std::string& CSA) {
        return nshogi::io::csa::CSAToMove32(State.getPosition(), CSA);
    });

    auto MLModule = Module.def_submodule("ml");

    pybind11::enum_<nshogi::ml::FeatureType>(MLModule, "FeatureType")
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
        .value("OP_PIECE_SCORE", nshogi::ml::FeatureType::FT_OpPieceScore);

    MLModule.def("move_to_index", (std::size_t(*)(nshogi::core::Color,
                                                  const nshogi::core::Move32)) &
                                      nshogi::ml::getMoveIndex);

    pybind11::class_<PyFeatureStack>(MLModule, "FeatureStack")
        .def(pybind11::init<const std::vector<nshogi::ml::FeatureType>&,
                            const nshogi::core::State&,
                            const nshogi::core::StateConfig&>())
        .def("to_numpy", &PyFeatureStack::to_numpy);

    pybind11::class_<nshogi::ml::AZTeacher>(MLModule, "AZTeacher")
        .def("state",
             [](const nshogi::ml::AZTeacher& T) {
                 auto State = nshogi::io::sfen::StateBuilder::newState(
                     std::string(T.Sfen));

                 return State;
             })
        .def("config",
             [](const nshogi::ml::AZTeacher& T) {
                 nshogi::core::StateConfig Config;

                 Config.Rule = T.EndingRule;
                 Config.MaxPly = T.MaxPly;
                 Config.BlackDrawValue = T.BlackDrawValue;
                 Config.WhiteDrawValue = T.WhiteDrawValue;

                 return Config;
             })
        .def("sfen", [](const nshogi::ml::AZTeacher& T) { return T.Sfen; })
        .def("policy",
             [](const nshogi::ml::AZTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(
                     (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                 std::memset(reinterpret_cast<char*>(Data), 0,
                             nshogi::ml::MoveIndexMax * sizeof(float));

                 uint32_t SumVisits = 0;

                 for (uint8_t I = 0; I < T.NumMoves; ++I) {
                     SumVisits += (uint32_t)T.Visits[I];
                 }

                 assert(SumVisits > 0);

                 const auto State = nshogi::io::sfen::StateBuilder::newState(
                     std::string(T.Sfen));

                 for (uint8_t I = 0; I < T.NumMoves; ++I) {
                     const auto Move = nshogi::io::sfen::sfenToMove32(
                         State.getPosition(), std::string(T.Moves[I].data()));
                     const std::size_t Index =
                         nshogi::ml::getMoveIndex(T.SideToMove, Move);
                     Data[Index] = (float)T.Visits[I] / (float)SumVisits;
                 }

                 return NpArray;
             })
        .def("legal_moves",
             [](const nshogi::ml::AZTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(
                     (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                 std::memset(reinterpret_cast<char*>(Data), 0,
                             nshogi::ml::MoveIndexMax * sizeof(float));

                 const auto State = nshogi::io::sfen::StateBuilder::newState(
                     std::string(T.Sfen));
                 const auto LegalMoves =
                     nshogi::core::MoveGenerator::generateLegalMoves(State);

                 assert(State.getPosition().sideToMove() == T.SideToMove);

                 for (const auto& Move : LegalMoves) {
                     const std::size_t Index =
                         nshogi::ml::getMoveIndex(T.SideToMove, Move);
                     Data[Index] = 1;
                 }

                 return NpArray;
             })
        .def("attacks",
             [](const nshogi::ml::AZTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(2 * 81);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                 std::memset(reinterpret_cast<char*>(Data), 0,
                             2 * 81 * sizeof(float));

                 const auto State = nshogi::io::sfen::StateBuilder::newState(
                     std::string(T.Sfen));
                 nshogi::core::internal::ImmutableStateAdapter Adapter(State);

                 if (State.getSideToMove() == nshogi::core::Black) {
                     const nshogi::core::internal::bitboard::Bitboard
                         MyAttackBB =
                             Adapter->getAttackBB<nshogi::core::Black>();
                     const nshogi::core::internal::bitboard::Bitboard
                         OpAttackBB =
                             Adapter->getAttackBB<nshogi::core::White>();

                     const nshogi::ml::FeatureBitboard MyAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(MyAttackBB, 1.0, false);
                     const nshogi::ml::FeatureBitboard OpAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(OpAttackBB, 1.0, false);

                     MyAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data);
                     OpAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data +
                                                                          81);
                 } else {
                     const nshogi::core::internal::bitboard::Bitboard
                         MyAttackBB =
                             Adapter->getAttackBB<nshogi::core::White>();
                     const nshogi::core::internal::bitboard::Bitboard
                         OpAttackBB =
                             Adapter->getAttackBB<nshogi::core::Black>();

                     const nshogi::ml::FeatureBitboard MyAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(MyAttackBB, 1.0, true);
                     const nshogi::ml::FeatureBitboard OpAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(OpAttackBB, 1.0, true);

                     MyAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data);
                     OpAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data +
                                                                          81);
                 }

                 return NpArray;
             })
        .def("value",
             [](const nshogi::ml::AZTeacher& T) {
                 if (T.SideToMove == T.Winner) {
                     return 1.0;
                 } else {
                     return 0.0;
                 }
             })
        .def("draw", [](const nshogi::ml::AZTeacher& T) {
            if (T.Winner == nshogi::core::NoColor) {
                return 1.0;
            }

            return 0.0;
        });

    pybind11::class_<nshogi::ml::SimpleTeacher>(MLModule, "SimpleTeacher")
        .def("state", &nshogi::ml::SimpleTeacher::getState)
        .def("config", &nshogi::ml::SimpleTeacher::getConfig)
        .def("sfen",
             [](const nshogi::ml::SimpleTeacher& T) {
                 return nshogi::io::sfen::stateToSfen(T.getState());
             })
        .def("policy",
             [](const nshogi::ml::SimpleTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(
                     (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                 std::memset(reinterpret_cast<char*>(Data), 0,
                             nshogi::ml::MoveIndexMax * sizeof(float));

                 const std::size_t Index = nshogi::ml::getMoveIndex(
                     T.getState().getSideToMove(), T.getNextMove());
                 Data[Index] = 1.0f;
                 return NpArray;
             })
        .def("policy_index",
             [](const nshogi::ml::SimpleTeacher& T) {
                 const std::size_t Index = nshogi::ml::getMoveIndex(
                     T.getState().getSideToMove(), T.getNextMove());
                 return Index;
             })
        .def("legal_moves",
             [](const nshogi::ml::SimpleTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(
                     (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                 std::memset(reinterpret_cast<char*>(Data), 0,
                             nshogi::ml::MoveIndexMax * sizeof(float));

                 const auto State = T.getState();
                 const auto LegalMoves =
                     nshogi::core::MoveGenerator::generateLegalMoves(State);

                 for (const auto& Move : LegalMoves) {
                     const std::size_t Index =
                         nshogi::ml::getMoveIndex(State.getSideToMove(), Move);
                     Data[Index] = 1.0f;
                 }
                 return NpArray;
             })
        .def("attacks",
             [](const nshogi::ml::SimpleTeacher& T) {
                 auto NpArray = pybind11::array_t<float>(2 * 81);
                 auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                 std::memset(reinterpret_cast<char*>(Data), 0,
                             2 * 81 * sizeof(float));

                 const auto State = T.getState();
                 nshogi::core::internal::ImmutableStateAdapter Adapter(State);

                 if (State.getSideToMove() == nshogi::core::Black) {
                     const nshogi::core::internal::bitboard::Bitboard
                         MyAttackBB =
                             Adapter->getAttackBB<nshogi::core::Black>();
                     const nshogi::core::internal::bitboard::Bitboard
                         OpAttackBB =
                             Adapter->getAttackBB<nshogi::core::White>();
                     const nshogi::ml::FeatureBitboard MyAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(MyAttackBB, 1.0f, false);
                     const nshogi::ml::FeatureBitboard OpAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(OpAttackBB, 1.0f, false);
                     MyAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data);
                     OpAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data +
                                                                          81);
                 } else {
                     const nshogi::core::internal::bitboard::Bitboard
                         MyAttackBB =
                             Adapter->getAttackBB<nshogi::core::White>();
                     const nshogi::core::internal::bitboard::Bitboard
                         OpAttackBB =
                             Adapter->getAttackBB<nshogi::core::Black>();
                     const nshogi::ml::FeatureBitboard MyAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(MyAttackBB, 1.0f, true);
                     const nshogi::ml::FeatureBitboard OpAttackFB =
                         nshogi::ml::internal::FeatureBitboardUtil::
                             makeFeatureBitboard(OpAttackBB, 1.0f, true);
                     MyAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data);
                     OpAttackFB.extract<nshogi::core::IterateOrder::ESWN>(Data +
                                                                          81);
                 }
                 return NpArray;
             })
        .def("value",
             [](const nshogi::ml::SimpleTeacher& T) {
                 if (T.getWinner() == nshogi::core::NoColor) {
                     return 0.5f;
                 }
                 const auto SideToMove = T.getState().getSideToMove();
                 return (T.getWinner() == SideToMove) ? 1.0f : 0.0f;
             })
        .def("draw", [](const nshogi::ml::SimpleTeacher& T) {
            return (T.getWinner() == nshogi::core::NoColor) ? 1.0f : 0.0f;
        });

    pybind11::class_<
        nshogi::ml::TeacherLoaderForFixedSizeTeacher<nshogi::ml::AZTeacher>>(
        MLModule, "AZTeacherLoader")
        .def(pybind11::init<const std::string&>())
        .def("shuffle",
             [](const nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                    nshogi::ml::AZTeacher>& Loader,
                const std::string& OutputPath, uint64_t Seed) {
                 nshogi::ml::ThreadsafeTeacherWriter<
                     nshogi::ml::AZTeacher>::shuffle(Loader, OutputPath, Seed);
             })
        .def("filter",
             [](const nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                    nshogi::ml::AZTeacher>& Loader,
                const std::string& OutputPath) {
                 std::ofstream Ofs(OutputPath, std::ios::out | std::ios::app);

                 for (std::size_t I = 0; I < Loader.size(); ++I) {
                     const auto T = Loader[I];

                     if (T.Visits[0] == 1 && T.Visits[1] == 0) {
                         continue;
                     }

                     nshogi::io::file::save(Ofs, T);
                 }
             })
        .def("__len__", &nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                            nshogi::ml::AZTeacher>::size)
        .def("__getitem__", &nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                                nshogi::ml::AZTeacher>::operator[]);

    pybind11::class_<nshogi::ml::TeacherLoaderForFixedSizeTeacher<
        nshogi::ml::SimpleTeacher>>(MLModule, "SimpleTeacherLoader")
        .def(pybind11::init<const std::string&>())
        .def("shuffle",
             [](const nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                    nshogi::ml::SimpleTeacher>& Loader,
                const std::string& OutputPath, uint64_t Seed) {
                 nshogi::ml::ThreadsafeTeacherWriter<
                     nshogi::ml::SimpleTeacher>::shuffle(Loader, OutputPath,
                                                         Seed);
             })
        .def("__len__", &nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                            nshogi::ml::SimpleTeacher>::size)
        .def("__getitem__", &nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                                nshogi::ml::SimpleTeacher>::operator[]);
}
