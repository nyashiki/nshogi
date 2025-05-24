//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfpn.h"
#include "mate1ply.h"
#include "../../core/internal/movegenerator.h"
#include "../../core/internal/stateadapter.h"

#include <algorithm>

namespace nshogi {
namespace solver {
namespace internal {
namespace dfpn {

SolverImpl::SolverImpl(std::size_t MemoryMB, uint64_t MaxDepth_, uint64_t MaxNodeCount_)
    : MaxDepth(MaxDepth_)
    , MaxNodeCount(MaxNodeCount_) {
    static_assert(sizeof(DfPnTTEntry) == 32);
    static_assert(sizeof(DfPnTTBundle) == 256);

    TTSize = MemoryMB * 1024ULL * 1024ULL / sizeof(DfPnTTBundle);
    TT = std::make_unique<DfPnTTBundle[]>(TTSize);

    for (std::size_t I = 0; I < TTSize; ++I) {
        for (std::size_t J = 0; J < DfPnTTBundle::BundleSize; ++J) {
            TT[I].Entries[J].setGeneration(0);
        }
    }

    Generation = 0;
    SearchedNodeCount = 0;
}

SolverImpl::~SolverImpl() {
}

template <core::Color C>
void SolverImpl::storeToTT(
        core::internal::StateImpl* S, uint64_t Depth, const DfPnValue& Value) {
    const core::Stands StandsSideToMove = S->getPosition().getStand<C>();
    const uint64_t Hash = S->getBoardHash();
    const std::size_t Index = (Hash + Depth) % TTSize;
    for (std::size_t I = 0; I < DfPnTTBundle::BundleSize; ++I) {
        DfPnTTEntry* Entry = &TT[Index].Entries[I];

        if (Entry->generation() != Generation || I == DfPnTTBundle::BundleSize - 1) {
            Entry->setHash(Hash);
            Entry->setStandsSideToMove(StandsSideToMove);
            Entry->setProofNumber(Value.ProofNumber);
            Entry->setDisproofNumber(Value.DisproofNumber);
            Entry->setGeneration(Generation);
            return;
        }

        if (Entry->hash() == Hash) {
            if (Value.ProofNumber == 0 &&
                    core::isSuperiorOrEqual(Entry->standsSideToMove(), StandsSideToMove)) {
                Entry->setStandsSideToMove(StandsSideToMove);
                Entry->setProofNumber(Value.ProofNumber);
                Entry->setDisproofNumber(Value.DisproofNumber);
                return;
            }

            if (Entry->standsSideToMove() == StandsSideToMove) {
                Entry->setProofNumber(Value.ProofNumber);
                Entry->setDisproofNumber(Value.DisproofNumber);
                return;
            }
        }
    }
}

template <core::Color C>
DfPnValue SolverImpl::loadFromTT(core::internal::StateImpl* S, uint64_t Depth) const {
    const core::Stands StandsSideToMove = S->getPosition().getStand<C>();
    const uint64_t Hash = S->getBoardHash();
    const std::size_t Index = (Hash + Depth) % TTSize;
    for (std::size_t I = 0; I < DfPnTTBundle::BundleSize; ++I) {
        DfPnTTEntry* Entry = &TT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            break;
        }

        if (Entry->hash() == Hash) {
            if (Entry->proofNumber() == 0) {
                if (core::isSuperiorOrEqual(StandsSideToMove, Entry->standsSideToMove())) {
                    return DfPnValue(Entry->proofNumber(), Entry->disproofNumber());
                }
            }

            if (Entry->standsSideToMove() == StandsSideToMove) {
                return DfPnValue(Entry->proofNumber(), Entry->disproofNumber());
            }
        }
    }

    // No entries found.
    return DfPnValue(1, 1);
}

template <core::Color C, bool Attacking>
core::Move32 SolverImpl::search(core::internal::StateImpl* S, uint64_t Depth, DfPnValue* ThisValue, DfPnValue* Threshold) {
    TTSaver<C> Saver(this, S, Depth, ThisValue);

    if (ThisValue->ProofNumber >= Threshold->ProofNumber ||
            ThisValue->DisproofNumber >= Threshold->DisproofNumber) {
        Threshold->ProofNumber = ThisValue->ProofNumber;
        Threshold->DisproofNumber = ThisValue->DisproofNumber;
        return core::Move32::MoveNone();
    }

    // Step 1: check terminal.
    // Repetition.
    const auto RepetitionStatus = S->getRepetitionStatus();
    if (RepetitionStatus == core::RepetitionStatus::WinRepetition ||
            RepetitionStatus == core::RepetitionStatus::SuperiorRepetition) {
        if constexpr (Attacking) {
            *ThisValue = DfPnValue(0, DfPnValue::Infinity);
            return core::Move32::MoveNone();
        } else {
            *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
            return core::Move32::MoveNone();
        }
    } else if (RepetitionStatus == core::RepetitionStatus::LossRepetition ||
            RepetitionStatus == core::RepetitionStatus::InferiorRepetition) {
        if constexpr (Attacking) {
            *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
            return core::Move32::MoveNone();
        } else {
            *ThisValue = DfPnValue(0, DfPnValue::Infinity);
            return core::Move32::MoveNone();
        }
    } else if (RepetitionStatus == core::RepetitionStatus::Repetition) {
        *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
        return core::Move32::MoveNone();
    }

    // Checkmate.
    const auto Moves = Attacking
        ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<C, true>(*S)
        : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<C, true>(*S);

    if (Moves.size() == 0) {
        if constexpr (Attacking) {
            *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
            return core::Move32::MoveNone();
        } else {
            const core::Move32 LastMove = S->getLastMove();
            if (LastMove.drop() && LastMove.pieceType() == core::PieceTypeKind::PTK_Pawn) {
                *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
            } else {
                *ThisValue = DfPnValue(0, DfPnValue::Infinity);
            }
            return core::Move32::MoveNone();
        }
    }

    // Mate 1 ply.
    if constexpr (Attacking) {
        const core::Move32 Checkmate1Ply = internal::mate1ply::solve<C>(*S);
        if (!Checkmate1Ply.isNone()) {
            *ThisValue = DfPnValue(0, DfPnValue::Infinity);
            return Checkmate1Ply;
        }
    }

    // Depth limit.
    if (MaxDepth > 0 && Depth >= MaxDepth) {
        *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
        return core::Move32::MoveNone();
    }

    // Step 2: search.
    if constexpr (Attacking) { // OR node.
        while (MaxNodeCount == 0 || SearchedNodeCount < MaxNodeCount) {
            uint32_t MinProof = DfPnValue::Infinity;
            uint32_t MinProof2 = DfPnValue::Infinity;
            uint32_t SumDisproof = 0;
            DfPnValue BestChildValue;
            core::Move32 BestMove;
            for (const core::Move32 Move : Moves) {
                S->doMove<C>(Move);
                DfPnValue ChildValue = loadFromTT<~C>(S, Depth + 1);
                S->undoMove<~C>();

                if (ChildValue.ProofNumber < MinProof) {
                    MinProof2 = MinProof;
                    MinProof = ChildValue.ProofNumber;
                    BestChildValue = ChildValue;
                    BestMove = Move;
                } else if (ChildValue.ProofNumber < MinProof2) {
                    MinProof2 = ChildValue.ProofNumber;
                }
                SumDisproof += ChildValue.DisproofNumber;
            }
            SumDisproof = std::min(DfPnValue::Infinity, SumDisproof);

            if (MinProof >= Threshold->ProofNumber || SumDisproof >= Threshold->DisproofNumber) {
                ThisValue->ProofNumber = MinProof;
                ThisValue->DisproofNumber = SumDisproof;
                return core::Move32::MoveNone();
            }

            DfPnValue ChildThreshold(
                std::min({Threshold->ProofNumber, MinProof2 + 1, DfPnValue::Infinity}),
                (ThisValue->DisproofNumber >= DfPnValue::Infinity || Threshold->DisproofNumber >= DfPnValue::Infinity)
                    ? DfPnValue::Infinity
                    : std::min(Threshold->DisproofNumber + BestChildValue.DisproofNumber - SumDisproof, DfPnValue::Infinity)
            );

            S->doMove<C>(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue, &ChildThreshold);
            S->undoMove<~C>();
            ++SearchedNodeCount;

            if (BestChildValue.ProofNumber == 0) {
                ThisValue->ProofNumber = 0;
                ThisValue->DisproofNumber = DfPnValue::Infinity;
                return BestMove;
            }
        }
        return core::Move32::MoveNone();
    } else { // AND node.
        while (MaxNodeCount == 0 || SearchedNodeCount < MaxNodeCount) {
            uint32_t SumProof = 0;
            uint32_t MinDisproof = DfPnValue::Infinity;
            uint32_t MinDisproof2 = DfPnValue::Infinity;
            DfPnValue BestChildValue;
            core::Move32 BestMove;

            for (const core::Move32 Move : Moves) {
                S->doMove<C>(Move);
                DfPnValue ChildValue = loadFromTT<~C>(S, Depth + 1);
                S->undoMove<~C>();

                SumProof += ChildValue.ProofNumber;

                if (ChildValue.DisproofNumber < MinDisproof) {
                    MinDisproof2 = MinDisproof;
                    MinDisproof = ChildValue.DisproofNumber;
                    BestChildValue = ChildValue;
                    BestMove = Move;
                } else if (ChildValue.DisproofNumber < MinDisproof2) {
                    MinDisproof2 = ChildValue.DisproofNumber;
                }
            }
            SumProof = std::min(DfPnValue::Infinity, SumProof);

            if (SumProof >= Threshold->ProofNumber || MinDisproof >= Threshold->DisproofNumber) {
                ThisValue->ProofNumber = SumProof;
                ThisValue->DisproofNumber = MinDisproof;
                return core::Move32::MoveNone();
            }

            DfPnValue ChildThreshold(
                (ThisValue->ProofNumber >= DfPnValue::Infinity || Threshold->ProofNumber >= DfPnValue::Infinity)
                ? DfPnValue::Infinity
                : std::min(Threshold->ProofNumber + BestChildValue.ProofNumber - SumProof, DfPnValue::Infinity),
                std::min({Threshold->DisproofNumber, MinDisproof2 + 1, DfPnValue::Infinity})
            );

            S->doMove<C>(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue, &ChildThreshold);
            S->undoMove<~C>();
            ++SearchedNodeCount;

            if (BestChildValue.DisproofNumber == 0) {
                ThisValue->ProofNumber = DfPnValue::Infinity;
                ThisValue->DisproofNumber = 0;
                return BestMove;
            }
        }
        return core::Move32::MoveNone();
    }
}

template <core::Color C, bool Attacking>
std::vector<core::Move32> SolverImpl::findPV(core::internal::StateImpl* S, uint64_t Depth) const {
    const auto Moves = Attacking
        ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<C, true>(*S)
        : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<C, true>(*S);

    if (Moves.size() == 0) {
        return { };
    }

    std::vector<core::Move32> BestPV;
    for (const core::Move32 Move : Moves) {
        S->doMove<C>(Move);

        const auto ChildValue = loadFromTT<~C>(S, Depth + 1);
        std::vector<core::Move32> SubPV { };
        if (ChildValue.ProofNumber == 0) {
            SubPV = findPV<~C, !Attacking>(S, Depth + 1);
        }

        S->undoMove<~C>();

        if (ChildValue.ProofNumber != 0) {
            continue;
        }

        if (SubPV.size() + 1 > BestPV.size()) {
            BestPV.clear();
            BestPV.reserve(SubPV.size() + 1);
            BestPV.push_back(Move);
            BestPV.insert(
                    BestPV.end(),
                    std::make_move_iterator(SubPV.begin()),
                    std::make_move_iterator(SubPV.end()));
        }
    }

    if (BestPV.size() == 0) {
        const core::Move32 Checkmate1Ply = solver::internal::mate1ply::solve<C>(*S);
        if (!Checkmate1Ply.isNone()) {
            return { Checkmate1Ply };
        }
    }

    return BestPV;
}

std::vector<core::Move32> SolverImpl::findPV(core::internal::StateImpl* S) const {
    return (S->getSideToMove() == core::Black)
        ? findPV<core::Black, true>(S, 0)
        : findPV<core::White, true>(S, 0);
}

core::Move32 SolverImpl::solve(core::State* S) {
    ++Generation;
    SearchedNodeCount = 0;

    core::internal::MutableStateAdapter Adapter(*S);

    const auto SideToMove = S->getSideToMove();

    DfPnValue RootValue(1, 1);
    DfPnValue Threshold(DfPnValue::Infinity, DfPnValue::Infinity);

    while (true) {
        const core::Move32 BestMove = (SideToMove == core::Black)
            ? search<core::Black, true>(Adapter.get(), 0, &RootValue, &Threshold)
            : search<core::White, true>(Adapter.get(), 0, &RootValue, &Threshold);

        if (RootValue.ProofNumber == 0) {
            // Proven.
            return BestMove;
        }

        if (RootValue.DisproofNumber == 0) {
            // Disproven.
            return core::Move32::MoveNone();
        }

        if (MaxNodeCount > 0 && searchedNodeCount() >= MaxNodeCount) {
            return core::Move32::MoveNone();
        }
    }
}

std::vector<core::Move32> SolverImpl::solveWithPV(core::State* S) {
    // First, just solve the problem.
    core::Move32 SolvedMove = solve(S);

    if (SolvedMove.isNone()) {
        return { };
    }

    // Then, traverse the tree through a terminal node with the longest move sequence.
    core::internal::MutableStateAdapter Adapter(*S);
    return findPV(Adapter.get());
}

uint64_t SolverImpl::searchedNodeCount() const {
    return SearchedNodeCount;
}

} // namespace dfpn
} // namespaec internal
} // namespace solver
} // namespace nshogi
