//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfpn.h"
#include "../../core/internal/movegenerator.h"
#include "../../core/internal/stateadapter.h"

#include <algorithm>

namespace nshogi {
namespace solver {
namespace internal {
namespace dfpn {

SolverImpl::SolverImpl(std::size_t MemoryMB) {
    static_assert(sizeof(DfPnTTEntry) == 16);
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

template <core::Color C, bool Attacking>
void SolverImpl::storeToTT(core::internal::StateImpl* S,
                           core::RepetitionStatus RS, uint64_t Depth,
                           const DfPnValue& Value) {
    const core::Stands StandsAttacking = (Attacking)
                                             ? S->getPosition().getStand<C>()
                                             : S->getPosition().getStand<~C>();
    const uint64_t Hash = S->getBoardHash();
    const std::size_t Index =
        (((Hash + 2 * Depth) << 1) | (uint64_t)Attacking) % TTSize;

    std::size_t DeleteIndex = 0;
    uint16_t PnPlusDnMax = 0;
    uint16_t PnMaxDnMax = 0;

    for (std::size_t I = 0; I < DfPnTTBundle::BundleSize; ++I) {
        DfPnTTEntry* Entry = &TT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            Entry->setHash(Hash);
            Entry->setStandsAttacking(StandsAttacking);
            Entry->setProofNumber((uint16_t)Value.ProofNumber);
            Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
            Entry->setGeneration(Generation);
            TT[Index].setRepetitionStatus(I, RS);
            return;
        }

        const uint16_t PnPlusDn =
            Entry->proofNumber() + Entry->disproofNumber();

        if (PnPlusDn < DfPnValue::Infinity) {
            const uint16_t PnMaxDn =
                std::max(Entry->proofNumber(), Entry->disproofNumber());
            if (PnPlusDn > PnPlusDnMax) {
                PnPlusDnMax = PnPlusDn;
                PnMaxDnMax = PnMaxDn;
                DeleteIndex = I;
            } else if (PnMaxDn == PnPlusDnMax) {
                if (PnMaxDn > PnMaxDnMax) {
                    PnMaxDnMax = PnMaxDn;
                    DeleteIndex = I;
                }
            }
        }

        if (Entry->isSameHash(Hash)) {
            if (Entry->standsAttacking() == StandsAttacking) {
                Entry->setProofNumber((uint16_t)Value.ProofNumber);
                Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
                return;
            }

            if (RS == core::RepetitionStatus::NoRepetition) {
                if (Value.ProofNumber == 0) {
                    if (core::isSuperiorOrEqual(Entry->standsAttacking(),
                                                StandsAttacking)) {
                        Entry->setStandsAttacking(StandsAttacking);
                        Entry->setProofNumber((uint16_t)Value.ProofNumber);
                        Entry->setDisproofNumber(
                            (uint16_t)Value.DisproofNumber);
                        return;
                    }
                }
                if (Value.DisproofNumber == 0) {
                    if (core::isSuperiorOrEqual(StandsAttacking,
                                                Entry->standsAttacking())) {
                        Entry->setStandsAttacking(StandsAttacking);
                        Entry->setProofNumber((uint16_t)Value.ProofNumber);
                        Entry->setDisproofNumber(
                            (uint16_t)Value.DisproofNumber);
                        return;
                    }
                }
            }
        }
    }

    DfPnTTEntry* Entry = &TT[Index].Entries[DeleteIndex];
    Entry->setHash(Hash);
    Entry->setStandsAttacking(StandsAttacking);
    Entry->setProofNumber((uint16_t)Value.ProofNumber);
    Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
    Entry->setGeneration(Generation);
    TT[Index].setRepetitionStatus(DeleteIndex, RS);
}

template <core::Color C, bool Attacking>
DfPnValue SolverImpl::loadFromTT(core::internal::StateImpl* S, uint64_t Depth,
                                 bool* IsFound) const {
    const core::Stands StandsAttacking = (Attacking)
                                             ? S->getPosition().getStand<C>()
                                             : S->getPosition().getStand<~C>();
    const uint64_t Hash = S->getBoardHash();
    const std::size_t Index =
        (((Hash + 2 * Depth) << 1) | (uint64_t)Attacking) % TTSize;
    for (std::size_t I = 0; I < DfPnTTBundle::BundleSize; ++I) {
        const DfPnTTEntry* Entry = &TT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            break;
        }

        if (Entry->isSameHash(Hash)) {
            if (Entry->standsAttacking() == StandsAttacking) {
                *IsFound = true;
                return DfPnValue((uint32_t)Entry->proofNumber(),
                                 (uint32_t)Entry->disproofNumber());
            }

            if (TT[Index].repetitionStatus(I) ==
                core::RepetitionStatus::NoRepetition) {
                if (Entry->proofNumber() == 0) {
                    if (core::isSuperiorOrEqual(StandsAttacking,
                                                Entry->standsAttacking())) {
                        *IsFound = true;
                        return DfPnValue((uint32_t)Entry->proofNumber(),
                                         (uint32_t)Entry->disproofNumber());
                    }
                }
                if (Entry->disproofNumber() == 0) {
                    if (core::isSuperiorOrEqual(Entry->standsAttacking(),
                                                StandsAttacking)) {
                        *IsFound = true;
                        return DfPnValue((uint32_t)Entry->proofNumber(),
                                         (uint32_t)Entry->disproofNumber());
                    }
                }
            }
        }
    }

    // No entries found.
    *IsFound = false;
    return DfPnValue(1, 1);
}

template <core::Color C, bool Attacking>
core::Move32 SolverImpl::search(core::internal::StateImpl* S, uint64_t Depth,
                                DfPnValue* ThisValue, DfPnValue* Threshold,
                                uint64_t MaxNodeCount, uint64_t MaxDepth) {
    ++SearchedNodeCount;

    if (ThisValue->ProofNumber >= Threshold->ProofNumber ||
        ThisValue->DisproofNumber >= Threshold->DisproofNumber) {
        Threshold->ProofNumber = ThisValue->ProofNumber;
        Threshold->DisproofNumber = ThisValue->DisproofNumber;
        return core::Move32::MoveNone();
    }

    const auto RepetitionStatus = S->getRepetitionStatus();
    TTSaver<C, Attacking> Saver(this, S, RepetitionStatus, Depth, ThisValue);

    // Step 1: check terminal.
    // Repetition.
    if (RepetitionStatus != core::RepetitionStatus::NoRepetition) {
        *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
        return core::Move32::MoveNone();
    }

    // Checkmate.
    const auto Moves =
        Attacking
            ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<
                  C, true>(*S)
            : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<
                  C, true>(*S);

    if constexpr (Attacking) {
        if (Moves.size() == 0) {
            *ThisValue = DfPnValue(DfPnValue::Infinity, 0);
            return core::Move32::MoveNone();
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
                bool IsFound;
                DfPnValue ChildValue =
                    loadFromTT<~C, !Attacking>(S, Depth + 1, &IsFound);

                if (!IsFound) {
                    const auto ChildRepetitionStatus = S->getRepetitionStatus();
                    if (ChildRepetitionStatus !=
                        core::RepetitionStatus::NoRepetition) {
                        ChildValue = DfPnValue(DfPnValue::Infinity, 0);
                    } else {
                        const auto NextMoves =
                            core::internal::MoveGeneratorInternal::
                                generateLegalEvasionMoves<~C, true>(*S);
                        if (NextMoves.size() == 0) {
                            const core::Move32 LastMove = S->getLastMove();
                            if (LastMove.drop() &&
                                LastMove.pieceType() ==
                                    core::PieceTypeKind::PTK_Pawn) {
                                ChildValue = DfPnValue(DfPnValue::Infinity, 0);
                            } else {
                                ChildValue = DfPnValue(0, DfPnValue::Infinity);
                            }
                        } else {
                            ChildValue.ProofNumber = (uint32_t)NextMoves.size();
                        }
                    }
                    storeToTT<~C, !Attacking>(S, ChildRepetitionStatus,
                                              Depth + 1, ChildValue);
                }

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

            if (MinProof >= Threshold->ProofNumber ||
                SumDisproof >= Threshold->DisproofNumber) {
                ThisValue->ProofNumber = MinProof;
                ThisValue->DisproofNumber = SumDisproof;
                return BestMove;
            }

            assert(Threshold->DisproofNumber + BestChildValue.DisproofNumber >
                   SumDisproof);
            DfPnValue ChildThreshold(
                std::min({Threshold->ProofNumber, MinProof2 + 1,
                          DfPnValue::Infinity}),
                (ThisValue->DisproofNumber >= DfPnValue::Infinity ||
                 Threshold->DisproofNumber >= DfPnValue::Infinity)
                    ? DfPnValue::Infinity
                    : std::min(Threshold->DisproofNumber +
                                   BestChildValue.DisproofNumber - SumDisproof,
                               DfPnValue::Infinity));

            S->doMove<C>(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue,
                                   &ChildThreshold, MaxNodeCount, MaxDepth);
            S->undoMove<~C>();

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
                bool IsFound;
                DfPnValue ChildValue =
                    loadFromTT<~C, !Attacking>(S, Depth + 1, &IsFound);
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

            if (SumProof >= Threshold->ProofNumber ||
                MinDisproof >= Threshold->DisproofNumber) {
                ThisValue->ProofNumber = SumProof;
                ThisValue->DisproofNumber = MinDisproof;
                return core::Move32::MoveNone();
            }

            assert(Threshold->ProofNumber + BestChildValue.ProofNumber >
                   SumProof);
            DfPnValue ChildThreshold(
                (ThisValue->ProofNumber >= DfPnValue::Infinity ||
                 Threshold->ProofNumber >= DfPnValue::Infinity)
                    ? DfPnValue::Infinity
                    : std::min(Threshold->ProofNumber +
                                   BestChildValue.ProofNumber - SumProof,
                               DfPnValue::Infinity),
                std::min({Threshold->DisproofNumber, MinDisproof2 + 1,
                          DfPnValue::Infinity}));

            S->doMove<C>(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue,
                                   &ChildThreshold, MaxNodeCount, MaxDepth);
            S->undoMove<~C>();

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
std::vector<core::Move32> SolverImpl::findPV(core::internal::StateImpl* S,
                                             uint64_t Depth) const {
    const auto Moves =
        Attacking
            ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<
                  C, true>(*S)
            : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<
                  C, true>(*S);

    if (Moves.size() == 0) {
        return {};
    }

    std::vector<core::Move32> BestPV;
    for (const core::Move32 Move : Moves) {
        S->doMove<C>(Move);
        bool IsFound;
        const auto ChildValue =
            loadFromTT<~C, !Attacking>(S, Depth + 1, &IsFound);
        std::vector<core::Move32> SubPV{};
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
            BestPV.insert(BestPV.end(), std::make_move_iterator(SubPV.begin()),
                          std::make_move_iterator(SubPV.end()));
        }
    }

    return BestPV;
}

std::vector<core::Move32>
SolverImpl::findPV(core::internal::StateImpl* S) const {
    return (S->getSideToMove() == core::Black)
               ? findPV<core::Black, true>(S, 0)
               : findPV<core::White, true>(S, 0);
}

core::Move32 SolverImpl::solve(core::State* S, uint64_t MaxNodeCount,
                               uint64_t MaxDepth) {
    ++Generation;
    SearchedNodeCount = 0;

    core::internal::MutableStateAdapter Adapter(*S);

    const auto SideToMove = S->getSideToMove();

    DfPnValue RootValue(1, 1);
    DfPnValue Threshold(DfPnValue::Infinity, DfPnValue::Infinity);

    while (true) {
        const core::Move32 BestMove =
            (SideToMove == core::Black)
                ? search<core::Black, true>(Adapter.get(), 0, &RootValue,
                                            &Threshold, MaxNodeCount, MaxDepth)
                : search<core::White, true>(Adapter.get(), 0, &RootValue,
                                            &Threshold, MaxNodeCount, MaxDepth);

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

std::vector<core::Move32> SolverImpl::solveWithPV(core::State* S,
                                                  uint64_t MaxNodeCount,
                                                  uint64_t MaxDepth) {
    // First, just solve the problem.
    core::Move32 SolvedMove = solve(S, MaxNodeCount, MaxDepth);

    if (SolvedMove.isNone()) {
        return {};
    }

    // Then, traverse the tree through a terminal node with the longest move
    // sequence.
    core::internal::MutableStateAdapter Adapter(*S);
    return findPV(Adapter.get());
}

uint64_t SolverImpl::searchedNodeCount() const {
    return SearchedNodeCount;
}

} // namespace dfpn
} // namespace internal
} // namespace solver
} // namespace nshogi
