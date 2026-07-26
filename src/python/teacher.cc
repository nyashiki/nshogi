//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "teacher.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../core/internal/stateadapter.h"
#include "../core/movegenerator.h"
#include "../core/state.h"
#include "../core/stateconfig.h"
#include "../io/file.h"
#include "../io/sfen.h"
#include "../ml/azteacher.h"
#include "../ml/batchedteacherloader.h"
#include "../ml/common.h"
#include "../ml/featurestack.h"
#include "../ml/simpleteacher.h"
#include "../ml/teacherloader.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

template <typename TeacherType>
TeacherType
getTeacherAt(nshogi::ml::TeacherLoaderForFixedSizeTeacher<TeacherType>& Loader,
             std::size_t Index) {
    if (Index >= Loader.size()) {
        throw pybind11::index_error("teacher index out of range");
    }
    return Loader[Index];
}

// AZTeacher records come from external files, so a corrupted record may
// hold char arrays without a null terminator; reading them as C strings
// would run out of bounds.
std::string azTeacherSfen(const nshogi::ml::AZTeacher& T) {
    if (std::memchr(T.Sfen, '\0', sizeof(T.Sfen)) == nullptr) {
        throw std::runtime_error(
            "AZTeacher: Sfen is not null-terminated (corrupted record?).");
    }
    return std::string(T.Sfen);
}

std::string azTeacherMove(const nshogi::ml::AZTeacher& T, uint8_t Index) {
    if (std::memchr(T.Moves[Index].data(), '\0', T.Moves[Index].size()) ==
        nullptr) {
        throw std::runtime_error("AZTeacher: a move string is not "
                                 "null-terminated (corrupted record?).");
    }
    return std::string(T.Moves[Index].data());
}

template <typename T>
pybind11::array_t<T> makeArrayFromUniquePtr2d(std::unique_ptr<T[]> Data,
                                              std::size_t Dim0,
                                              std::size_t Dim1) {

    T* RawPtr = Data.release();

    pybind11::capsule Base(RawPtr,
                           [](void* Ptr) { delete[] static_cast<T*>(Ptr); });

    return pybind11::array_t<T>(
        {Dim0, Dim1},
        {static_cast<pybind11::ssize_t>(Dim1 * sizeof(T)),
         static_cast<pybind11::ssize_t>(sizeof(T))},
        RawPtr, Base);
}

} // namespace

namespace nshogi {
namespace python {
namespace teacher {

void bind(pybind11::module_& Module) {
    pybind11::class_<nshogi::ml::AZTeacher>(Module, "AZTeacher")
        .def("state",
             [](const nshogi::ml::AZTeacher& T) {
                 auto State =
                     nshogi::io::sfen::StateBuilder::newState(azTeacherSfen(T));

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
        .def("sfen",
             [](const nshogi::ml::AZTeacher& T) { return azTeacherSfen(T); })
        .def(
            "policy",
            [](const nshogi::ml::AZTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(
                    (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                std::memset(reinterpret_cast<char*>(Data), 0,
                            nshogi::ml::MoveIndexMax * sizeof(float));

                if (T.NumMoves > T.Visits.size()) {
                    throw std::runtime_error(
                        "AZTeacher.policy(): NumMoves exceeds the number of "
                        "saved playouts (corrupted record?).");
                }

                uint32_t SumVisits = 0;

                for (uint8_t I = 0; I < T.NumMoves; ++I) {
                    SumVisits += (uint32_t)T.Visits[I];
                }

                if (SumVisits == 0) {
                    throw std::runtime_error(
                        "AZTeacher.policy(): sum of visits is zero.");
                }

                const auto State =
                    nshogi::io::sfen::StateBuilder::newState(azTeacherSfen(T));

                if (State.getSideToMove() != T.SideToMove) {
                    throw std::runtime_error(
                        "AZTeacher.policy(): SideToMove is inconsistent with "
                        "Sfen (corrupted record?).");
                }

                for (uint8_t I = 0; I < T.NumMoves; ++I) {
                    const auto Move = nshogi::io::sfen::sfenToMove32(
                        State.getPosition(), azTeacherMove(T, I));
                    const std::size_t Index =
                        ChannelsFirst
                            ? nshogi::ml::getMoveIndex<true>(T.SideToMove, Move)
                            : nshogi::ml::getMoveIndex<false>(T.SideToMove,
                                                              Move);
                    Data[Index] = (float)T.Visits[I] / (float)SumVisits;
                }

                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def(
            "legal_moves",
            [](const nshogi::ml::AZTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(
                    (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                std::memset(reinterpret_cast<char*>(Data), 0,
                            nshogi::ml::MoveIndexMax * sizeof(float));

                const auto State =
                    nshogi::io::sfen::StateBuilder::newState(azTeacherSfen(T));
                const auto LegalMoves =
                    nshogi::core::MoveGenerator::generateLegalMoves(State);

                if (State.getPosition().sideToMove() != T.SideToMove) {
                    throw std::runtime_error(
                        "AZTeacher.legal_moves(): SideToMove is inconsistent "
                        "with Sfen (corrupted record?).");
                }

                for (const nshogi::core::Move32 Move : LegalMoves) {
                    const std::size_t Index =
                        ChannelsFirst
                            ? nshogi::ml::getMoveIndex<true>(T.SideToMove, Move)
                            : nshogi::ml::getMoveIndex<false>(T.SideToMove,
                                                              Move);
                    Data[Index] = 1;
                }

                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def(
            "attacks",
            [](const nshogi::ml::AZTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(2 * 81);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);

                std::memset(reinterpret_cast<char*>(Data), 0,
                            2 * 81 * sizeof(float));

                const auto State =
                    nshogi::io::sfen::StateBuilder::newState(azTeacherSfen(T));

                nshogi::ml::FeatureStackRuntime FSR(
                    {nshogi::ml::FeatureType::FT_MyAttack,
                     nshogi::ml::FeatureType::FT_OpAttack},
                    State, nshogi::core::StateConfig());

                if (ChannelsFirst) {
                    FSR.extract<nshogi::core::IterateOrder::Fastest, true>(
                        Data);
                    NpArray.resize({2, 9, 9});
                } else {
                    FSR.extract<nshogi::core::IterateOrder::Fastest, false>(
                        Data);
                    NpArray.resize({9, 9, 2});
                }

                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def("value",
             [](const nshogi::ml::AZTeacher& T) {
                 if (T.Winner == nshogi::core::NoColor) {
                     return 0.5f;
                 }
                 return (T.SideToMove == T.Winner) ? 1.0f : 0.0f;
             })
        .def("draw", [](const nshogi::ml::AZTeacher& T) {
            return (T.Winner == nshogi::core::NoColor) ? 1.0f : 0.0f;
        });

    pybind11::class_<nshogi::ml::SimpleTeacher>(Module, "SimpleTeacher")
        .def("state", &nshogi::ml::SimpleTeacher::getState)
        .def("config", &nshogi::ml::SimpleTeacher::getConfig)
        .def("sfen",
             [](const nshogi::ml::SimpleTeacher& T) {
                 return nshogi::io::sfen::stateToSfen(T.getState());
             })
        .def("move",
             [](const nshogi::ml::SimpleTeacher& T) {
                 const auto State = T.getState();
                 return State.getMove32FromMove16(T.getNextMove());
             })
        .def(
            "policy",
            [](const nshogi::ml::SimpleTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(
                    (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                std::memset(reinterpret_cast<char*>(Data), 0,
                            nshogi::ml::MoveIndexMax * sizeof(float));

                const std::size_t Index =
                    ChannelsFirst
                        ? nshogi::ml::getMoveIndex<true>(
                              T.getState().getSideToMove(), T.getNextMove())
                        : nshogi::ml::getMoveIndex<false>(
                              T.getState().getSideToMove(), T.getNextMove());
                Data[Index] = 1.0f;
                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def(
            "policy_index",
            [](const nshogi::ml::SimpleTeacher& T, bool ChannelsFirst) {
                const std::size_t Index =
                    ChannelsFirst
                        ? nshogi::ml::getMoveIndex<true>(
                              T.getState().getSideToMove(), T.getNextMove())
                        : nshogi::ml::getMoveIndex<false>(
                              T.getState().getSideToMove(), T.getNextMove());
                return Index;
            },
            pybind11::arg("channels_first"))
        .def(
            "legal_moves",
            [](const nshogi::ml::SimpleTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(
                    (pybind11::ssize_t)nshogi::ml::MoveIndexMax);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                std::memset(reinterpret_cast<char*>(Data), 0,
                            nshogi::ml::MoveIndexMax * sizeof(float));

                const auto State = T.getState();
                const auto LegalMoves =
                    nshogi::core::MoveGenerator::generateLegalMoves(State);

                for (const nshogi::core::Move32 Move : LegalMoves) {
                    const std::size_t Index =
                        ChannelsFirst ? nshogi::ml::getMoveIndex<true>(
                                            State.getSideToMove(), Move)
                                      : nshogi::ml::getMoveIndex<false>(
                                            State.getSideToMove(), Move);
                    Data[Index] = 1.0f;
                }
                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def(
            "attacks",
            [](const nshogi::ml::SimpleTeacher& T, bool ChannelsFirst) {
                auto NpArray = pybind11::array_t<float>(2 * 81);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                std::memset(reinterpret_cast<char*>(Data), 0,
                            2 * 81 * sizeof(float));

                const auto State = T.getState();

                nshogi::ml::FeatureStackRuntime FSR(
                    {nshogi::ml::FeatureType::FT_MyAttack,
                     nshogi::ml::FeatureType::FT_OpAttack},
                    State, nshogi::core::StateConfig());

                if (ChannelsFirst) {
                    FSR.extract<nshogi::core::IterateOrder::Fastest, true>(
                        Data);
                    NpArray.resize({2, 9, 9});
                } else {
                    FSR.extract<nshogi::core::IterateOrder::Fastest, false>(
                        Data);
                    NpArray.resize({9, 9, 2});
                }

                return NpArray;
            },
            pybind11::arg("channels_first"))
        .def("value",
             [](const nshogi::ml::SimpleTeacher& T) {
                 if (T.getWinner() == nshogi::core::NoColor) {
                     return 0.5f;
                 }
                 const auto SideToMove = T.getState().getSideToMove();
                 return (T.getWinner() == SideToMove) ? 1.0f : 0.0f;
             })
        .def("draw",
             [](const nshogi::ml::SimpleTeacher& T) {
                 return (T.getWinner() == nshogi::core::NoColor) ? 1.0f : 0.0f;
             })
        .def("q", &nshogi::ml::SimpleTeacher::q)
        .def("game_ply", &nshogi::ml::SimpleTeacher::gamePly)
        .def("declaration_score", [](const nshogi::ml::SimpleTeacher& T) {
            const auto State = T.getState();
            nshogi::core::internal::ImmutableStateAdapter Adapter(State);

            if (State.getSideToMove() == nshogi::core::Black) {
                const float MyScore =
                    (float)Adapter
                        ->computeDeclarationScore<nshogi::core::Black>() /
                    28.0f;
                const float OpScore =
                    (float)Adapter
                        ->computeDeclarationScore<nshogi::core::White>() /
                    27.0f;
                auto NpArray = pybind11::array_t<float>(2);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                Data[0] = MyScore;
                Data[1] = OpScore;
                return NpArray;
            } else {
                const float MyScore =
                    (float)Adapter
                        ->computeDeclarationScore<nshogi::core::White>() /
                    27.0f;
                const float OpScore =
                    (float)Adapter
                        ->computeDeclarationScore<nshogi::core::Black>() /
                    28.0f;
                auto NpArray = pybind11::array_t<float>(2);
                auto Data = reinterpret_cast<float*>(NpArray.request().ptr);
                Data[0] = MyScore;
                Data[1] = OpScore;
                return NpArray;
            }
        });

    pybind11::class_<
        nshogi::ml::TeacherLoaderForFixedSizeTeacher<nshogi::ml::AZTeacher>>(
        Module, "AZTeacherLoader")
        .def(pybind11::init<const std::string&, bool, int32_t>(),
             pybind11::arg("path"), pybind11::arg("shuffle"),
             pybind11::arg("version"))
        .def("filter",
             [](nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                    nshogi::ml::AZTeacher>& Loader,
                const std::string& OutputPath) {
                 std::ofstream Ofs(OutputPath, std::ios::out | std::ios::app |
                                                   std::ios::binary);

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
        .def("__getitem__", &getTeacherAt<nshogi::ml::AZTeacher>);

    pybind11::class_<nshogi::ml::TeacherLoaderForFixedSizeTeacher<
        nshogi::ml::SimpleTeacher>>(Module, "SimpleTeacherLoader")
        .def(pybind11::init<const std::string&, bool, int32_t>(),
             pybind11::arg("path"), pybind11::arg("shuffle"),
             pybind11::arg("version"))
        .def("__len__", &nshogi::ml::TeacherLoaderForFixedSizeTeacher<
                            nshogi::ml::SimpleTeacher>::size)
        .def("__getitem__", &getTeacherAt<nshogi::ml::SimpleTeacher>);

    pybind11::class_<nshogi::ml::BatchedTeacherLoader>(Module,
                                                       "BatchedTeacherLoader")
        .def(
            pybind11::init<const std::string&,
                           std::shared_ptr<nshogi::ml::IFeatureExtractor>,
                           std::size_t, bool, bool, std::size_t, std::size_t>(),
            pybind11::arg("path"), pybind11::arg("feature_extractor"),
            pybind11::arg("batch_size"), pybind11::arg("shuffle"),
            pybind11::arg("batch_shuffle"), pybind11::arg("num_workers"),
            pybind11::arg("prefetch"))
        .def("__len__", &nshogi::ml::BatchedTeacherLoader::size)
        .def("next",
             [](nshogi::ml::BatchedTeacherLoader& Loader) -> pybind11::object {
                 std::optional<nshogi::ml::BatchedTeacher> Batch;

                 {
                     pybind11::gil_scoped_release Release;
                     Batch = Loader.next();
                 }

                 if (!Batch.has_value()) {
                     return pybind11::none();
                 }

                 auto& B = Batch.value();

                 const std::size_t BatchSize = Loader.batchSize();

                 pybind11::array_t<int32_t> MyIds =
                     makeArrayFromUniquePtr2d<int32_t>(
                         std::move(B.MyIds), BatchSize, Loader.idSize());

                 pybind11::array_t<int32_t> OpIds =
                     makeArrayFromUniquePtr2d<int32_t>(
                         std::move(B.OpIds), BatchSize, Loader.idSize());

                 pybind11::array_t<int8_t> Results =
                     makeArrayFromUniquePtr2d<int8_t>(std::move(B.Results),
                                                      BatchSize, 1);

                 pybind11::array_t<float> Qs = makeArrayFromUniquePtr2d<float>(
                     std::move(B.Qs), BatchSize, 1);

                 pybind11::array_t<int8_t> IsStables =
                     makeArrayFromUniquePtr2d<int8_t>(std::move(B.IsStables),
                                                      BatchSize, 1);

                 return pybind11::make_tuple(MyIds, OpIds, Results, Qs,
                                             IsStables);
             });
}

} // namespace teacher
} // namespace python
} // namespace nshogi
