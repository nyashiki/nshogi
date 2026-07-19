//
// Copyright (c) 2025-2026 @nyashiki
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

namespace {

constexpr uint32_t CapturingEvasionProofCost = 4;

uint32_t initialDisproofNumber(core::Move32 Evasion) {
    // Prefer defenses that are more likely to refute the attack. This value is
    // only used until the edge is expanded and obtains its derived PN/DN.
    if (Evasion.capturePieceType() != core::PTK_Empty) {
        return 1;
    }
    if (Evasion.pieceType() == core::PTK_King) {
        return 2;
    }
    return 3;
}

} // namespace

SolverImpl::SolverImpl(std::size_t MemoryMB) {
    static_assert(sizeof(DfPnNodeTTEntry) == 16);
    static_assert(sizeof(DfPnNodeTTBundle) == 256);
    static_assert(sizeof(DfPnEdgeTTEntry) == 16);
    static_assert(sizeof(DfPnEdgeTTBundle) == 256);

    const std::size_t TotalMemoryBytes = MemoryMB * 1024ULL * 1024ULL;
    const std::size_t NodeMemoryBytes = TotalMemoryBytes / 8;
    const std::size_t EdgeMemoryBytes = TotalMemoryBytes - NodeMemoryBytes;

    NodeTTSize =
        std::max<std::size_t>(1, NodeMemoryBytes / sizeof(DfPnNodeTTBundle));
    NodeTT = std::make_unique<DfPnNodeTTBundle[]>(NodeTTSize);

    EdgeTTSize =
        std::max<std::size_t>(1, EdgeMemoryBytes / sizeof(DfPnEdgeTTBundle));
    EdgeTT = std::make_unique<DfPnEdgeTTBundle[]>(EdgeTTSize);

    clearTT();

    Generation = 0;
    SearchedNodeCount = 0;
}

SolverImpl::~SolverImpl() {
}

void SolverImpl::clearTT() {
    for (std::size_t I = 0; I < NodeTTSize; ++I) {
        for (std::size_t J = 0; J < DfPnNodeTTBundle::BundleSize; ++J) {
            NodeTT[I].Entries[J].reset();
        }
    }

    for (std::size_t I = 0; I < EdgeTTSize; ++I) {
        for (std::size_t J = 0; J < DfPnEdgeTTBundle::BundleSize; ++J) {
            EdgeTT[I].Entries[J].setGeneration(0);
        }
    }
}

template <core::Color C, bool Attacking>
void SolverImpl::storeNodeToTT(core::internal::StateImpl* S, uint64_t Depth,
                               const DfPnValue& Value) {
    // Finite progress remains authoritative on edges. Persist only solved
    // derived summaries, which are stable and support stand superiority.
    if (!Value.NodeCacheable ||
        (Value.ProofNumber != 0 && Value.DisproofNumber != 0)) {
        return;
    }

    const core::Stands StandsAttacking = (Attacking)
                                             ? S->getPosition().getStand<C>()
                                             : S->getPosition().getStand<~C>();
    const uint64_t PositionHash = S->getBoardHash();
    const std::size_t Index =
        (((PositionHash + 2 * Depth) << 1) | static_cast<uint64_t>(Attacking)) %
        NodeTTSize;

    const std::size_t DeleteIndex =
        (PositionHash >> 32) % DfPnNodeTTBundle::BundleSize;

    for (std::size_t I = 0; I < DfPnNodeTTBundle::BundleSize; ++I) {
        DfPnNodeTTEntry* Entry = &NodeTT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            Entry->setPositionHash(PositionHash);
            Entry->setStandsAttacking(StandsAttacking);
            Entry->setProofNumber(static_cast<uint16_t>(Value.ProofNumber));
            Entry->setDisproofNumber(
                static_cast<uint16_t>(Value.DisproofNumber));
            Entry->setGeneration(Generation);
            return;
        }

        if (!Entry->isSamePosition(PositionHash)) {
            continue;
        }

        if (Entry->standsAttacking() == StandsAttacking) {
            Entry->setProofNumber(static_cast<uint16_t>(Value.ProofNumber));
            Entry->setDisproofNumber(
                static_cast<uint16_t>(Value.DisproofNumber));
            return;
        }

        if (Value.ProofNumber == 0 &&
            core::isSuperiorOrEqual(Entry->standsAttacking(),
                                    StandsAttacking)) {
            Entry->setStandsAttacking(StandsAttacking);
            Entry->setProofNumber(static_cast<uint16_t>(Value.ProofNumber));
            Entry->setDisproofNumber(
                static_cast<uint16_t>(Value.DisproofNumber));
            return;
        }

        if (Value.DisproofNumber == 0 &&
            core::isSuperiorOrEqual(StandsAttacking,
                                    Entry->standsAttacking())) {
            Entry->setStandsAttacking(StandsAttacking);
            Entry->setProofNumber(static_cast<uint16_t>(Value.ProofNumber));
            Entry->setDisproofNumber(
                static_cast<uint16_t>(Value.DisproofNumber));
            return;
        }
    }

    DfPnNodeTTEntry* Entry = &NodeTT[Index].Entries[DeleteIndex];
    Entry->setPositionHash(PositionHash);
    Entry->setStandsAttacking(StandsAttacking);
    Entry->setProofNumber(static_cast<uint16_t>(Value.ProofNumber));
    Entry->setDisproofNumber(static_cast<uint16_t>(Value.DisproofNumber));
    Entry->setGeneration(Generation);
}

template <core::Color C, bool Attacking>
DfPnValue SolverImpl::loadNodeFromTT(core::internal::StateImpl* S,
                                     uint64_t Depth, bool* IsFound) const {
    const core::Stands StandsAttacking = (Attacking)
                                             ? S->getPosition().getStand<C>()
                                             : S->getPosition().getStand<~C>();
    const uint64_t PositionHash = S->getBoardHash();
    const std::size_t Index =
        (((PositionHash + 2 * Depth) << 1) | static_cast<uint64_t>(Attacking)) %
        NodeTTSize;

    for (std::size_t I = 0; I < DfPnNodeTTBundle::BundleSize; ++I) {
        const DfPnNodeTTEntry* Entry = &NodeTT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            break;
        }

        if (!Entry->isSamePosition(PositionHash)) {
            continue;
        }

        if (Entry->standsAttacking() == StandsAttacking ||
            (Entry->proofNumber() == 0 &&
             core::isSuperiorOrEqual(StandsAttacking,
                                     Entry->standsAttacking())) ||
            (Entry->disproofNumber() == 0 &&
             core::isSuperiorOrEqual(Entry->standsAttacking(),
                                     StandsAttacking))) {
            *IsFound = true;
            return DfPnValue(static_cast<uint32_t>(Entry->proofNumber()),
                             static_cast<uint32_t>(Entry->disproofNumber()));
        }
    }

    *IsFound = false;
    return DfPnValue(1, 1);
}

template <bool Attacking>
void SolverImpl::storeEdgeToTT(core::internal::StateImpl* S, core::Move32 Move,
                               uint64_t Depth, const DfPnValue& Value) {
    const uint64_t SourceHash = S->getHash();
    const uint64_t EdgeHash =
        SourceHash ^ static_cast<uint64_t>(core::Move16(Move).value());
    const std::size_t Index =
        (((EdgeHash + 2 * Depth) << 1) | static_cast<uint64_t>(Attacking)) %
        EdgeTTSize;

    std::size_t DeleteIndex = 0;
    uint16_t PnPlusDnMax = 0;
    uint16_t PnMaxDnMax = 0;

    for (std::size_t I = 0; I < DfPnEdgeTTBundle::BundleSize; ++I) {
        DfPnEdgeTTEntry* Entry = &EdgeTT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            Entry->setSourceHash(SourceHash);
            Entry->setMove(Move);
            Entry->setProofNumberAndNodeCacheable((uint16_t)Value.ProofNumber,
                                                  Value.NodeCacheable);
            Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
            Entry->setGeneration(Generation);
            return;
        }

        const uint16_t PnPlusDn =
            Entry->proofNumber() + Entry->disproofNumber();

        const uint16_t PnMaxDn =
            std::max(Entry->proofNumber(), Entry->disproofNumber());
        if (PnPlusDn < DfPnValue::Infinity &&
            (PnPlusDn > PnPlusDnMax ||
             (PnPlusDn == PnPlusDnMax && PnMaxDn > PnMaxDnMax))) {
            PnPlusDnMax = PnPlusDn;
            PnMaxDnMax = PnMaxDn;
            DeleteIndex = I;
        }

        if (Entry->isSameEdge(SourceHash, Move)) {
            Entry->setProofNumberAndNodeCacheable((uint16_t)Value.ProofNumber,
                                                  Value.NodeCacheable);
            Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
            return;
        }
    }

    DfPnEdgeTTEntry* Entry = &EdgeTT[Index].Entries[DeleteIndex];
    Entry->setSourceHash(SourceHash);
    Entry->setMove(Move);
    Entry->setProofNumberAndNodeCacheable((uint16_t)Value.ProofNumber,
                                          Value.NodeCacheable);
    Entry->setDisproofNumber((uint16_t)Value.DisproofNumber);
    Entry->setGeneration(Generation);
}

template <bool Attacking>
DfPnValue SolverImpl::loadEdgeFromTT(core::internal::StateImpl* S,
                                     core::Move32 Move, uint64_t Depth,
                                     bool* IsFound) const {
    const uint64_t SourceHash = S->getHash();
    const uint64_t EdgeHash =
        SourceHash ^ static_cast<uint64_t>(core::Move16(Move).value());
    const std::size_t Index =
        (((EdgeHash + 2 * Depth) << 1) | static_cast<uint64_t>(Attacking)) %
        EdgeTTSize;
    for (std::size_t I = 0; I < DfPnEdgeTTBundle::BundleSize; ++I) {
        const DfPnEdgeTTEntry* Entry = &EdgeTT[Index].Entries[I];

        if (Entry->generation() != Generation) {
            break;
        }

        if (Entry->isSameEdge(SourceHash, Move)) {
            *IsFound = true;
            return DfPnValue((uint32_t)Entry->proofNumber(),
                             (uint32_t)Entry->disproofNumber(),
                             Entry->isNodeCacheable());
        }
    }

    // No entries found.
    *IsFound = false;
    return DfPnValue(1, 1);
}

template <core::Color C, bool Attacking, bool WilyPromote>
core::Move32 SolverImpl::search(core::internal::StateImpl* S, uint64_t Depth,
                                DfPnValue* IncomingEdgeValue,
                                DfPnValue* Threshold, uint64_t MaxNodeCount,
                                uint64_t MaxDepth) {
    ++SearchedNodeCount;

    // Step 1: check terminal.
    const auto RepetitionStatus = S->getRepetitionStatus();
    if (RepetitionStatus != core::RepetitionStatus::NoRepetition) {
        // Repetition depends on the path to this node. Keep it as edge state,
        // but prevent it from being promoted into the shared node cache.
        *IncomingEdgeValue = DfPnValue(DfPnValue::Infinity, 0, false);
        return core::Move32::MoveNone();
    }

    // A node value is only a derived cache of its outgoing edge values. Keep
    // finite search progress on the incoming edge authoritative: a node cache
    // can be older than an edge after another transposition updated it.
    DfPnValue NodeValue = *IncomingEdgeValue;
    if (NodeValue.ProofNumber >= Threshold->ProofNumber ||
        NodeValue.DisproofNumber >= Threshold->DisproofNumber) {
        Threshold->ProofNumber = NodeValue.ProofNumber;
        Threshold->DisproofNumber = NodeValue.DisproofNumber;
        return core::Move32::MoveNone();
    }

    bool IsNodeFound;
    const DfPnValue CachedNodeValue =
        loadNodeFromTT<C, Attacking>(S, Depth, &IsNodeFound);
    if (Depth > 0 && IsNodeFound &&
        (CachedNodeValue.ProofNumber == 0 ||
         CachedNodeValue.DisproofNumber == 0)) {
        NodeValue = CachedNodeValue;
        *IncomingEdgeValue = CachedNodeValue;
        Threshold->ProofNumber = CachedNodeValue.ProofNumber;
        Threshold->DisproofNumber = CachedNodeValue.DisproofNumber;
        return core::Move32::MoveNone();
    }

    const auto Moves =
        Attacking
            ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<
                  C, WilyPromote>(*S)
            : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<
                  C, WilyPromote>(*S);

    if constexpr (Attacking) {
        if (Moves.size() == 0) {
            NodeValue = DfPnValue(DfPnValue::Infinity, 0);
            storeNodeToTT<C, Attacking>(S, Depth, NodeValue);
            *IncomingEdgeValue = NodeValue;
            return core::Move32::MoveNone();
        }
    }

    if (MaxDepth > 0 && Depth >= MaxDepth) {
        NodeValue = DfPnValue(DfPnValue::Infinity, 0);
        storeNodeToTT<C, Attacking>(S, Depth, NodeValue);
        *IncomingEdgeValue = NodeValue;
        return core::Move32::MoveNone();
    }

    // Step 2: search.
    if constexpr (Attacking) { // OR node.
        while (MaxNodeCount == 0 || SearchedNodeCount < MaxNodeCount) {
            uint32_t MinProof = DfPnValue::Infinity;
            uint32_t MinProof2 = DfPnValue::Infinity;
            uint32_t SumDisproof = 0;
            DfPnValue BestEdgeValue;
            bool AllEdgesCacheable = true;

            // TODO(nyashiki): [[indeterminate]] could be used in C++26.
            core::Move32 BestMove = core::Move32::MoveNone();

            for (const core::Move32 Move : Moves) {
                bool IsFound;
                DfPnValue EdgeValue =
                    loadEdgeFromTT<Attacking>(S, Move, Depth, &IsFound);

                if (!IsFound) {
                    S->doMove<C>(Move);
                    const auto ChildRepetitionStatus = S->getRepetitionStatus();
                    if (ChildRepetitionStatus !=
                        core::RepetitionStatus::NoRepetition) {
                        EdgeValue = DfPnValue(DfPnValue::Infinity, 0, false);
                    } else {
                        const auto NextMoves =
                            core::internal::MoveGeneratorInternal::
                                generateLegalEvasionMoves<~C, WilyPromote>(*S);
                        if (NextMoves.size() == 0) {
                            const core::Move32 LastMove = S->getLastMove();
                            if (LastMove.drop() &&
                                LastMove.pieceType() ==
                                    core::PieceTypeKind::PTK_Pawn) {
                                EdgeValue = DfPnValue(DfPnValue::Infinity, 0);
                            } else {
                                EdgeValue = DfPnValue(0, DfPnValue::Infinity);
                                storeNodeToTT<~C, !Attacking>(S, Depth + 1,
                                                              EdgeValue);
                            }
                        } else {
                            uint32_t InitialProofNumber = 0;
                            for (const core::Move32 Evasion : NextMoves) {
                                // A check that can be answered by a capture is
                                // usually harder to prove than other checks.
                                InitialProofNumber +=
                                    (Evasion.capturePieceType() !=
                                     core::PTK_Empty)
                                        ? CapturingEvasionProofCost
                                        : 1;
                            }
                            EdgeValue.ProofNumber = std::min(
                                DfPnValue::Infinity, InitialProofNumber);
                        }
                    }
                    S->undoMove<~C>();
                    storeEdgeToTT<Attacking>(S, Move, Depth, EdgeValue);
                }

                AllEdgesCacheable &= EdgeValue.NodeCacheable;
                if (EdgeValue.ProofNumber < MinProof) {
                    MinProof2 = MinProof;
                    MinProof = EdgeValue.ProofNumber;
                    BestEdgeValue = EdgeValue;
                    BestMove = Move;
                } else if (EdgeValue.ProofNumber < MinProof2) {
                    MinProof2 = EdgeValue.ProofNumber;
                }
                SumDisproof += EdgeValue.DisproofNumber;
            }
            SumDisproof = std::min(DfPnValue::Infinity, SumDisproof);
            const bool NodeCacheable = (MinProof == 0)
                                           ? BestEdgeValue.NodeCacheable
                                           : AllEdgesCacheable;
            NodeValue = DfPnValue(MinProof, SumDisproof, NodeCacheable);
            storeNodeToTT<C, Attacking>(S, Depth, NodeValue);

            if (NodeValue.ProofNumber >= Threshold->ProofNumber ||
                NodeValue.DisproofNumber >= Threshold->DisproofNumber) {
                *IncomingEdgeValue = NodeValue;
                return BestMove;
            }

            assert(Threshold->DisproofNumber + BestEdgeValue.DisproofNumber >
                   SumDisproof);
            DfPnValue EdgeThreshold(
                std::min({Threshold->ProofNumber, MinProof2 + 1,
                          DfPnValue::Infinity}),
                (NodeValue.DisproofNumber >= DfPnValue::Infinity ||
                 Threshold->DisproofNumber >= DfPnValue::Infinity)
                    ? DfPnValue::Infinity
                    : std::min(Threshold->DisproofNumber +
                                   BestEdgeValue.DisproofNumber - SumDisproof,
                               DfPnValue::Infinity));

            S->doMove<C>(BestMove);
            search<~C, !Attacking, WilyPromote>(S, Depth + 1, &BestEdgeValue,
                                                &EdgeThreshold, MaxNodeCount,
                                                MaxDepth);
            S->undoMove<~C>();
            storeEdgeToTT<Attacking>(S, BestMove, Depth, BestEdgeValue);

            if (BestEdgeValue.ProofNumber == 0) {
                NodeValue = DfPnValue(0, DfPnValue::Infinity,
                                      BestEdgeValue.NodeCacheable);
                storeNodeToTT<C, Attacking>(S, Depth, NodeValue);
                *IncomingEdgeValue = NodeValue;
                return BestMove;
            }
        }
        *IncomingEdgeValue = NodeValue;
        return core::Move32::MoveNone();
    } else { // AND node.
        while (MaxNodeCount == 0 || SearchedNodeCount < MaxNodeCount) {
            uint32_t SumProof = 0;
            uint32_t MinDisproof = DfPnValue::Infinity;
            uint32_t MinDisproof2 = DfPnValue::Infinity;
            DfPnValue BestEdgeValue;
            bool AllEdgesCacheable = true;

            // TODO(nyashiki): [[indeterminate]] could be used in C++26.
            core::Move32 BestMove = core::Move32::MoveNone();

            for (const core::Move32 Move : Moves) {
                bool IsFound;
                DfPnValue EdgeValue =
                    loadEdgeFromTT<Attacking>(S, Move, Depth, &IsFound);

                if (!IsFound) {
                    EdgeValue.DisproofNumber = initialDisproofNumber(Move);
                }

                AllEdgesCacheable &= EdgeValue.NodeCacheable;
                SumProof += EdgeValue.ProofNumber;

                if (EdgeValue.DisproofNumber < MinDisproof) {
                    MinDisproof2 = MinDisproof;
                    MinDisproof = EdgeValue.DisproofNumber;
                    BestEdgeValue = EdgeValue;
                    BestMove = Move;
                } else if (EdgeValue.DisproofNumber < MinDisproof2) {
                    MinDisproof2 = EdgeValue.DisproofNumber;
                }
            }
            SumProof = std::min(DfPnValue::Infinity, SumProof);
            const bool NodeCacheable = (MinDisproof == 0)
                                           ? BestEdgeValue.NodeCacheable
                                           : AllEdgesCacheable;
            NodeValue = DfPnValue(SumProof, MinDisproof, NodeCacheable);
            storeNodeToTT<C, Attacking>(S, Depth, NodeValue);

            if (NodeValue.ProofNumber >= Threshold->ProofNumber ||
                NodeValue.DisproofNumber >= Threshold->DisproofNumber) {
                *IncomingEdgeValue = NodeValue;
                return core::Move32::MoveNone();
            }

            assert(Threshold->ProofNumber + BestEdgeValue.ProofNumber >
                   SumProof);
            DfPnValue EdgeThreshold(
                (NodeValue.ProofNumber >= DfPnValue::Infinity ||
                 Threshold->ProofNumber >= DfPnValue::Infinity)
                    ? DfPnValue::Infinity
                    : std::min(Threshold->ProofNumber +
                                   BestEdgeValue.ProofNumber - SumProof,
                               DfPnValue::Infinity),
                std::min({Threshold->DisproofNumber, MinDisproof2 + 1,
                          DfPnValue::Infinity}));

            S->doMove<C>(BestMove);
            search<~C, !Attacking, WilyPromote>(S, Depth + 1, &BestEdgeValue,
                                                &EdgeThreshold, MaxNodeCount,
                                                MaxDepth);
            S->undoMove<~C>();
            storeEdgeToTT<Attacking>(S, BestMove, Depth, BestEdgeValue);

            if (BestEdgeValue.DisproofNumber == 0) {
                NodeValue = DfPnValue(DfPnValue::Infinity, 0,
                                      BestEdgeValue.NodeCacheable);
                storeNodeToTT<C, Attacking>(S, Depth, NodeValue);
                *IncomingEdgeValue = NodeValue;
                return BestMove;
            }
        }
        *IncomingEdgeValue = NodeValue;
        return core::Move32::MoveNone();
    }
}

template <core::Color C, bool Attacking, bool WilyPromote>
std::vector<core::Move32> SolverImpl::findPV(core::internal::StateImpl* S,
                                             uint64_t Depth) const {
    const auto Moves =
        Attacking
            ? core::internal::MoveGeneratorInternal::generateLegalCheckMoves<
                  C, WilyPromote>(*S)
            : core::internal::MoveGeneratorInternal::generateLegalEvasionMoves<
                  C, WilyPromote>(*S);

    if (Moves.size() == 0) {
        return {};
    }

    std::vector<core::Move32> BestPV;
    for (const core::Move32 Move : Moves) {
        bool IsEdgeFound;
        DfPnValue EdgeValue =
            loadEdgeFromTT<Attacking>(S, Move, Depth, &IsEdgeFound);

        S->doMove<C>(Move);
        if (!IsEdgeFound) {
            bool IsNodeFound;
            EdgeValue =
                loadNodeFromTT<~C, !Attacking>(S, Depth + 1, &IsNodeFound);
            if (!IsNodeFound) {
                S->undoMove<~C>();
                continue;
            }
        }

        if (EdgeValue.ProofNumber != 0) {
            S->undoMove<~C>();
            continue;
        }

        auto SubPV = findPV<~C, !Attacking, WilyPromote>(S, Depth + 1);
        S->undoMove<~C>();

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

std::vector<core::Move32> SolverImpl::findPV(core::internal::StateImpl* S,
                                             bool Strict) const {
    if (Strict) {
        return (S->getSideToMove() == core::Black)
                   ? findPV<core::Black, true, false>(S, 0)
                   : findPV<core::White, true, false>(S, 0);
    } else {
        return (S->getSideToMove() == core::Black)
                   ? findPV<core::Black, true, true>(S, 0)
                   : findPV<core::White, true, true>(S, 0);
    }
}

core::Move32 SolverImpl::solve(core::State* S, uint64_t MaxNodeCount,
                               uint64_t MaxDepth, bool Strict) {
    const auto SideToMove = S->getSideToMove();
    core::internal::MutableStateAdapter Adapter(*S);

    if (Strict) {
        if (SideToMove == core::Black) {
            return solve<core::Black, false>(Adapter.get(), MaxNodeCount,
                                             MaxDepth);
        } else {
            return solve<core::White, false>(Adapter.get(), MaxNodeCount,
                                             MaxDepth);
        }
    } else {
        if (SideToMove == core::Black) {
            return solve<core::Black, true>(Adapter.get(), MaxNodeCount,
                                            MaxDepth);
        } else {
            return solve<core::White, true>(Adapter.get(), MaxNodeCount,
                                            MaxDepth);
        }
    }
}

template <core::Color C, bool WilyPromote>
core::Move32 SolverImpl::solve(core::internal::StateImpl* S,
                               uint64_t MaxNodeCount, uint64_t MaxDepth) {
    ++Generation;
    if (Generation == 0) {
        // Entries from a different root must not reappear when the 16-bit
        // generation wraps around.
        clearTT();
        Generation = 1;
    }
    SearchedNodeCount = 0;

    // The root has no real incoming edge, so keep one synthetic edge value on
    // the stack. All non-root values are persisted in the edge TT.
    DfPnValue RootEdgeValue(1, 1);
    DfPnValue Threshold(DfPnValue::Infinity, DfPnValue::Infinity);

    while (true) {
        const core::Move32 BestMove = search<C, true, WilyPromote>(
            S, 0, &RootEdgeValue, &Threshold, MaxNodeCount, MaxDepth);

        if (RootEdgeValue.ProofNumber == 0) {
            // Proven.
            return BestMove;
        }

        if (RootEdgeValue.DisproofNumber == 0) {
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
                                                  uint64_t MaxDepth,
                                                  bool Strict) {
    // First, just solve the problem.
    core::Move32 SolvedMove = solve(S, MaxNodeCount, MaxDepth, Strict);

    if (SolvedMove.isNone()) {
        return {};
    }

    // Then, traverse the tree through a terminal node with
    // a shortest-win longest-loss move sequence.
    core::internal::MutableStateAdapter Adapter(*S);
    return findPV(Adapter.get(), Strict);
}

uint64_t SolverImpl::searchedNodeCount() const {
    return SearchedNodeCount;
}

} // namespace dfpn
} // namespace internal
} // namespace solver
} // namespace nshogi
