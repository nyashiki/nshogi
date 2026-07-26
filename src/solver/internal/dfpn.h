//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../../core/internal/stateimpl.h"
#include "../../core/types.h"
#include "../dfpn.h"

#ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
#define NSHOGI_SOLVER_INTERNAL_DFPN_H

namespace nshogi {
namespace solver {
namespace internal {
namespace dfpn {

struct DfPnValue {
 public:
    explicit DfPnValue()
        : ProofNumber(1)
        , DisproofNumber(1)
        , NodeCacheable(true) {
    }

    explicit DfPnValue(uint32_t P, uint32_t D, bool CanCacheNode = true)
        : ProofNumber(P)
        , DisproofNumber(D)
        , NodeCacheable(CanCacheNode) {
    }

    bool operator==(const DfPnValue& V) const {
        return ProofNumber == V.ProofNumber &&
               DisproofNumber == V.DisproofNumber &&
               NodeCacheable == V.NodeCacheable;
    }

    uint32_t ProofNumber;
    uint32_t DisproofNumber;
    // Path-dependent values remain valid for their edge but must not enter the
    // shared node cache.
    bool NodeCacheable;

    static constexpr uint32_t Infinity = 1 << 14;
};

// A node entry is a derived cache of its outgoing edge statistics. Keeping
// attacking stands separate from the position hash allows solved values to be
// reused through stand superiority/inferiority.
// sizeof(DfPnNodeTTEntry) == 16 byte.
struct DfPnNodeTTEntry {
 public:
    uint64_t positionHash() const {
        return PositionData & HASH_MASK;
    }

    core::Stands standsAttacking() const {
        return static_cast<core::Stands>(StandsAttackingValue);
    }

    uint16_t proofNumber() const {
        return ProofNumberValue;
    }

    uint16_t disproofNumber() const {
        return DisproofNumberValue;
    }

    uint16_t generation() const {
        return static_cast<uint16_t>(PositionData >> HASH_BITS);
    }

    bool isSamePosition(uint64_t Hash) const {
        return (Hash >> (64 - HASH_BITS)) == positionHash();
    }

    void setPositionHash(uint64_t Hash) {
        const uint64_t CurrentGeneration = PositionData & GENERATION_MASK;
        PositionData = CurrentGeneration | (Hash >> (64 - HASH_BITS));
    }

    void setStandsAttacking(core::Stands Stands) {
        StandsAttackingValue = static_cast<uint32_t>(Stands);
    }

    void setProofNumber(uint16_t Proof) {
        ProofNumberValue = Proof;
    }

    void setDisproofNumber(uint16_t Disproof) {
        DisproofNumberValue = Disproof;
    }

    void setGeneration(uint16_t NewGeneration) {
        const uint64_t Hash = PositionData & HASH_MASK;
        PositionData =
            (static_cast<uint64_t>(NewGeneration) << HASH_BITS) | Hash;
    }

    void reset() {
        StandsAttackingValue = 0;
        ProofNumberValue = 0;
        DisproofNumberValue = 0;
        PositionData = 0;
    }

 private:
    static constexpr int HASH_BITS = 48;
    static constexpr uint64_t HASH_MASK = (1ULL << HASH_BITS) - 1ULL;
    static constexpr uint64_t GENERATION_MASK = ~HASH_MASK;

    uint32_t StandsAttackingValue;
    uint16_t ProofNumberValue;
    uint16_t DisproofNumberValue;
    uint64_t PositionData;
};

// sizeof(DfPnNodeTTBundle) == 256 byte.
struct alignas(256) DfPnNodeTTBundle {
    static constexpr std::size_t BundleSize = 16;
    DfPnNodeTTEntry Entries[BundleSize];
};

// An entry stores the statistics for one outgoing edge. The source position
// and Move16 uniquely identify the edge; proof/disproof numbers describe the
// subtree reached by taking that edge.
// sizeof(DfPnEdgeTTEntry) == 16 byte.
struct DfPnEdgeTTEntry {
 public:
    uint64_t sourceHash() const {
        return SourceHash;
    }

    core::Move16 move() const {
        return core::Move16::fromValue(Move);
    }

    uint16_t proofNumber() const {
        return ProofNumberAndFlags & PROOF_NUMBER_MASK;
    }

    bool isNodeCacheable() const {
        return (ProofNumberAndFlags & NOT_NODE_CACHEABLE_BIT) == 0;
    }

    uint16_t disproofNumber() const {
        return DisproofNumber;
    }

    uint16_t generation() const {
        return Generation;
    }

    bool isSameEdge(uint64_t Hash, core::Move32 EdgeMove) const {
        return Hash == sourceHash() && core::Move16(EdgeMove) == move();
    }

    void setSourceHash(uint64_t Hash) {
        SourceHash = Hash;
    }

    void setMove(core::Move32 EdgeMove) {
        Move = core::Move16(EdgeMove).value();
    }

    void setProofNumberAndNodeCacheable(uint16_t Proof, bool Cacheable) {
        ProofNumberAndFlags = Proof | (Cacheable ? 0 : NOT_NODE_CACHEABLE_BIT);
    }

    void setDisproofNumber(uint16_t Disproof) {
        DisproofNumber = Disproof;
    }

    void setGeneration(uint16_t NewGeneration) {
        Generation = NewGeneration;
    }

 private:
    static constexpr uint16_t NOT_NODE_CACHEABLE_BIT = 1U << 15;
    static constexpr uint16_t PROOF_NUMBER_MASK = 0x7fffU;

    uint64_t SourceHash;
    uint16_t Move;
    uint16_t ProofNumberAndFlags;
    uint16_t DisproofNumber;
    uint16_t Generation;
};

// sizeof(DfPnEdgeTTBundle) == 256 byte.
struct alignas(256) DfPnEdgeTTBundle {
    static constexpr std::size_t BundleSize = 16;
    DfPnEdgeTTEntry Entries[BundleSize];
};

class SolverImpl {
 public:
    SolverImpl(std::size_t MemoryMB);
    ~SolverImpl();

    core::Move32 solve(core::State* S, uint64_t MaxNodeCount, uint64_t MaxDepth,
                       bool Strict);
    std::vector<core::Move32> solveWithPV(core::State* S, uint64_t MaxNodeCount,
                                          uint64_t MaxDepth, bool Strict);

    uint64_t searchedNodeCount() const;

 private:
    void clearTT();

    template <core::Color C, bool Attacking>
    void storeNodeToTT(core::internal::StateImpl* S, uint64_t Depth,
                       const DfPnValue& Value);

    template <core::Color C, bool Attacking>
    DfPnValue loadNodeFromTT(core::internal::StateImpl* S, uint64_t Depth,
                             bool* IsFound) const;

    template <bool Attacking>
    void storeEdgeToTT(core::internal::StateImpl* S, core::Move32 Move,
                       uint64_t Depth, const DfPnValue& Value);

    template <bool Attacking>
    DfPnValue loadEdgeFromTT(core::internal::StateImpl* S, core::Move32 Move,
                             uint64_t Depth, bool* IsFound) const;

    template <core::Color C, bool WilyPromote>
    core::Move32 solve(core::internal::StateImpl* S, uint64_t MaxNodeCount,
                       uint64_t MaxDepth);

    template <core::Color C, bool Attacking, bool WilyPromote>
    core::Move32 search(core::internal::StateImpl* S, uint64_t Depth,
                        DfPnValue* IncomingEdgeValue, DfPnValue* Threshold,
                        uint64_t MaxNodeCount, uint64_t MaxDepth);

    template <core::Color C, bool Attacking, bool WilyPromote>
    std::vector<core::Move32> findPV(core::internal::StateImpl* S,
                                     uint64_t Depth) const;
    std::vector<core::Move32> findPV(core::internal::StateImpl* S,
                                     bool Strict) const;

    std::size_t NodeTTSize;
    std::unique_ptr<DfPnNodeTTBundle[]> NodeTT;

    std::size_t EdgeTTSize;
    std::unique_ptr<DfPnEdgeTTBundle[]> EdgeTT;

    uint16_t Generation;
    uint64_t SearchedNodeCount;
};

} // namespace dfpn
} // namespace internal
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
