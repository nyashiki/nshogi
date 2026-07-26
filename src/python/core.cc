//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "core.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/stateconfig.h"
#include "../io/csa.h"
#include "../io/sfen.h"

#include <vector>

namespace nshogi {
namespace python {
namespace core {

void bind(pybind11::module_& Module) {
    pybind11::enum_<nshogi::core::Color>(Module, "Color")
        .value("BLACK", nshogi::core::Black)
        .value("WHITE", nshogi::core::White);

    // clang-format off
    pybind11::enum_<nshogi::core::Square>(Module, "Square")
        .value("SQ9A", nshogi::core::Sq9A) .value("SQ8A", nshogi::core::Sq8A) .value("SQ7A", nshogi::core::Sq7A)
        .value("SQ6A", nshogi::core::Sq6A) .value("SQ5A", nshogi::core::Sq5A) .value("SQ4A", nshogi::core::Sq4A)
        .value("SQ3A", nshogi::core::Sq3A) .value("SQ2A", nshogi::core::Sq2A) .value("SQ1A", nshogi::core::Sq1A)
        .value("SQ9B", nshogi::core::Sq9B) .value("SQ8B", nshogi::core::Sq8B) .value("SQ7B", nshogi::core::Sq7B)
        .value("SQ6B", nshogi::core::Sq6B) .value("SQ5B", nshogi::core::Sq5B) .value("SQ4B", nshogi::core::Sq4B)
        .value("SQ3B", nshogi::core::Sq3B) .value("SQ2B", nshogi::core::Sq2B) .value("SQ1B", nshogi::core::Sq1B)
        .value("SQ9C", nshogi::core::Sq9C) .value("SQ8C", nshogi::core::Sq8C) .value("SQ7C", nshogi::core::Sq7C)
        .value("SQ6C", nshogi::core::Sq6C) .value("SQ5C", nshogi::core::Sq5C) .value("SQ4C", nshogi::core::Sq4C)
        .value("SQ3C", nshogi::core::Sq3C) .value("SQ2C", nshogi::core::Sq2C) .value("SQ1C", nshogi::core::Sq1C)
        .value("SQ9D", nshogi::core::Sq9D) .value("SQ8D", nshogi::core::Sq8D) .value("SQ7D", nshogi::core::Sq7D)
        .value("SQ6D", nshogi::core::Sq6D) .value("SQ5D", nshogi::core::Sq5D) .value("SQ4D", nshogi::core::Sq4D)
        .value("SQ3D", nshogi::core::Sq3D) .value("SQ2D", nshogi::core::Sq2D) .value("SQ1D", nshogi::core::Sq1D)
        .value("SQ9E", nshogi::core::Sq9E) .value("SQ8E", nshogi::core::Sq8E) .value("SQ7E", nshogi::core::Sq7E)
        .value("SQ6E", nshogi::core::Sq6E) .value("SQ5E", nshogi::core::Sq5E) .value("SQ4E", nshogi::core::Sq4E)
        .value("SQ3E", nshogi::core::Sq3E) .value("SQ2E", nshogi::core::Sq2E) .value("SQ1E", nshogi::core::Sq1E)
        .value("SQ9F", nshogi::core::Sq9F) .value("SQ8F", nshogi::core::Sq8F) .value("SQ7F", nshogi::core::Sq7F)
        .value("SQ6F", nshogi::core::Sq6F) .value("SQ5F", nshogi::core::Sq5F) .value("SQ4F", nshogi::core::Sq4F)
        .value("SQ3F", nshogi::core::Sq3F) .value("SQ2F", nshogi::core::Sq2F) .value("SQ1F", nshogi::core::Sq1F)
        .value("SQ9G", nshogi::core::Sq9G) .value("SQ8G", nshogi::core::Sq8G) .value("SQ7G", nshogi::core::Sq7G)
        .value("SQ6G", nshogi::core::Sq6G) .value("SQ5G", nshogi::core::Sq5G) .value("SQ4G", nshogi::core::Sq4G)
        .value("SQ3G", nshogi::core::Sq3G) .value("SQ2G", nshogi::core::Sq2G) .value("SQ1G", nshogi::core::Sq1G)
        .value("SQ9H", nshogi::core::Sq9H) .value("SQ8H", nshogi::core::Sq8H) .value("SQ7H", nshogi::core::Sq7H)
        .value("SQ6H", nshogi::core::Sq6H) .value("SQ5H", nshogi::core::Sq5H) .value("SQ4H", nshogi::core::Sq4H)
        .value("SQ3H", nshogi::core::Sq3H) .value("SQ2H", nshogi::core::Sq2H) .value("SQ1H", nshogi::core::Sq1H)
        .value("SQ9I", nshogi::core::Sq9I) .value("SQ8I", nshogi::core::Sq8I) .value("SQ7I", nshogi::core::Sq7I)
        .value("SQ6I", nshogi::core::Sq6I) .value("SQ5I", nshogi::core::Sq5I) .value("SQ4I", nshogi::core::Sq4I)
        .value("SQ3I", nshogi::core::Sq3I) .value("SQ2I", nshogi::core::Sq2I) .value("SQ1I", nshogi::core::Sq1I);
    // clang-format on

    pybind11::class_<nshogi::core::Move32>(Module, "Move")
        .def_property_readonly("from_sq", &nshogi::core::Move32::from)
        .def_property_readonly("to_sq", &nshogi::core::Move32::to)
        .def_property_readonly("piece_type", &nshogi::core::Move32::pieceType)
        .def_property_readonly("capture_piece_type",
                               &nshogi::core::Move32::capturePieceType)
        .def_property_readonly("drop", &nshogi::core::Move32::drop)
        .def_property_readonly("promote", &nshogi::core::Move32::promote)
        .def("is_none", &nshogi::core::Move32::isNone);

    pybind11::enum_<nshogi::core::RepetitionStatus>(Module, "RepetitionStatus")
        .value("NO_REPETITION", nshogi::core::RepetitionStatus::NoRepetition)
        .value("REPETITION", nshogi::core::RepetitionStatus::Repetition)
        .value("WIN_REPETITION", nshogi::core::RepetitionStatus::WinRepetition)
        .value("LOSS_REPETITION",
               nshogi::core::RepetitionStatus::LossRepetition)
        .value("SUPERIOR_REPETITION",
               nshogi::core::RepetitionStatus::SuperiorRepetition)
        .value("INFERIOR_REPETITION",
               nshogi::core::RepetitionStatus::InferiorRepetition);

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
        .def("to_csa",
             [](const nshogi::core::State& S) {
                 return nshogi::io::csa::stateToCSA(S);
             })
        .def("get_stand_count",
             [](const nshogi::core::State& State, nshogi::core::Color C,
                nshogi::core::PieceTypeKind Type) {
                 return State.getPosition().getStandCount(C, Type);
             })
        .def("get_piece_on",
             [](const nshogi::core::State& S, int Square) {
                 return S.getPosition().pieceOn(nshogi::core::Square(Square));
             })
        .def("get_repetition",
             [](const nshogi::core::State& S, bool Strict) {
                 return S.getRepetitionStatus(Strict);
             })
        .def("can_declare", &nshogi::core::State::canDeclare)
        .def_property_readonly(
            "legal_moves",
            [](const nshogi::core::State& S) {
                auto Moves = nshogi::core::MoveGenerator::generateLegalMoves(S);

                return std::vector<nshogi::core::Move32>(Moves.begin(),
                                                         Moves.end());
            })
        .def_property_readonly("is_in_check", &nshogi::core::State::isInCheck)
        .def_property_readonly(
            "ply", [](const nshogi::core::State& S) { return S.getPly(true); })
        .def_property_readonly("last_move", &nshogi::core::State::getLastMove)
        .def_property_readonly("hash", &nshogi::core::State::getHash)
        .def_property_readonly("side_to_move",
                               &nshogi::core::State::getSideToMove)
        .def_property_readonly("initial_position",
                               &nshogi::core::State::getInitialPosition)
        .def_property_readonly("position", &nshogi::core::State::getPosition)
        .def_property_readonly("history",
                               [](const nshogi::core::State& S) {
                                   std::vector<nshogi::core::Move32> Moves;

                                   for (uint16_t Ply = 0; Ply < S.getPly(false);
                                        ++Ply) {
                                       Moves.push_back(S.getHistoryMove(Ply));
                                   }

                                   return Moves;
                               })
        .def_property_readonly("is_stable", [](const nshogi::core::State& S) {
            if (S.isInCheck()) {
                return false;
            }

            return nshogi::core::MoveGenerator::generateLegalCaptureMoves(S)
                       .size() == 0;
        });

    pybind11::class_<nshogi::core::StateConfig>(Module, "StateConfig")
        .def(pybind11::init<>())
        .def_readwrite("max_ply", &nshogi::core::StateConfig::MaxPly)
        .def_readwrite("black_draw_value",
                       &nshogi::core::StateConfig::BlackDrawValue)
        .def_readwrite("white_draw_value",
                       &nshogi::core::StateConfig::WhiteDrawValue);
}

} // namespace core
} // namespace python
} // namespace nshogi
