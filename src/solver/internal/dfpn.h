//
// Copyright (c) 2025 @nyashiki
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
        , DisproofNumber(1) {
    }

    explicit DfPnValue(uint32_t P, uint32_t D)
        : ProofNumber(P)
        , DisproofNumber(D) {
    }

    bool operator==(const DfPnValue& V) const {
        return ProofNumber == V.ProofNumber &&
               DisproofNumber == V.DisproofNumber;
    }

    uint32_t ProofNumber;
    uint32_t DisproofNumber;

    static constexpr uint32_t Infinity = 1 << 14;
};

// sizeof(DfPnTTEntry) == 16 byte.
struct DfPnTTEntry {
 public:
    uint64_t hash() const {
        return DataU64[1] & HASH_MASK;
    }

    core::Stands standsAttacking() const {
        return static_cast<core::Stands>(DataU32[0]);
    }

    uint16_t proofNumber() const {
        return DataU16[2];
    }

    uint16_t disproofNumber() const {
        return DataU16[3];
    }

    uint16_t generation() const {
        return (uint16_t)(DataU64[1] >> HASH_BITS);
    }

    bool isSameHash(uint64_t Hash) const {
        return (Hash >> (64 - HASH_BITS)) == hash();
    }

    void setHash(uint64_t Hash) {
        const uint64_t Generation = DataU64[1] & GENERATION_MASK;
        DataU64[1] = Generation | (Hash >> (64 - HASH_BITS));
    }

    void setStandsAttacking(core::Stands Stands) {
        DataU32[0] = static_cast<uint32_t>(Stands);
    }

    void setProofNumber(uint16_t Proof) {
        DataU16[2] = Proof;
    }

    void setDisproofNumber(uint16_t Disproof) {
        DataU16[3] = Disproof;
    }

    void setGeneration(uint16_t Generation) {
        const uint64_t Hash = DataU64[1] & HASH_MASK;
        DataU64[1] = ((uint64_t)Generation << HASH_BITS) | Hash;
    }

 private:
    static constexpr int HASH_BITS = 48;
    static constexpr uint64_t HASH_MASK = (1ULL << HASH_BITS) - 1ULL;
    static constexpr uint64_t GENERATION_MASK = ~HASH_MASK;

    union {
        // DataU64[1]: hash.
        uint64_t DataU64[2];

        // DataU32[0]: `Stands` for the side to move.
        // DataU32[1-]: unused.
        uint32_t DataU32[4];

        // DataU16[2]: proof number.
        // DataU16[3]: disproof number.
        // DataU16[4-6]: hash.
        // DataU16[7]: generation.
        uint16_t DataU16[8];
    };
};

// sizeof(DfPnTTBundle) == 256 byte.
struct alignas(256) DfPnTTBundle {
    static constexpr std::size_t BundleSize = 15;
    DfPnTTEntry Entries[BundleSize];

    core::RepetitionStatus repetitionStatus(std::size_t Index) const {
        return static_cast<core::RepetitionStatus>(Reserved[Index]);
    }

    void setRepetitionStatus(std::size_t Index, core::RepetitionStatus RS) {
        Reserved[Index] = static_cast<uint8_t>(RS);
    }

    uint8_t Reserved[16];
};

class SolverImpl {
 public:
    SolverImpl(std::size_t MemoryMB);
    ~SolverImpl();

    core::Move32 solve(core::State* S, uint64_t MaxDepth,
                       uint64_t MaxNodeCount);
    std::vector<core::Move32> solveWithPV(core::State* S, uint64_t MaxDepth,
                                          uint64_t MaxNodeCount);

    uint64_t searchedNodeCount() const;

 private:
    template <core::Color C, bool Attacking>
    class TTSaver {
     public:
        TTSaver(SolverImpl* SI, core::internal::StateImpl* S,
                core::RepetitionStatus RS, uint64_t Depth, DfPnValue* Value)
            : Impl(SI)
            , State(S)
            , RepetitionStatus(RS)
            , ThisDepth(Depth)
            , Target(Value) {
        }

        ~TTSaver() {
            Impl->storeToTT<C, Attacking>(State, RepetitionStatus, ThisDepth,
                                          *Target);
        }

     private:
        SolverImpl* Impl;
        core::internal::StateImpl* State;
        core::RepetitionStatus RepetitionStatus;
        uint64_t ThisDepth;
        DfPnValue* Target;
    };

    template <core::Color C, bool Attacking>
    void storeToTT(core::internal::StateImpl* S, core::RepetitionStatus RS,
                   uint64_t Depth, const DfPnValue& Value);

    template <core::Color C, bool Attacking>
    DfPnValue loadFromTT(core::internal::StateImpl* S, uint64_t Depth,
                         bool* IsFound) const;

    template <core::Color C, bool Attacking>
    core::Move32 search(core::internal::StateImpl* S, uint64_t Depth,
                        DfPnValue* ThisValue, DfPnValue* Threshold,
                        uint64_t MaxNodeCount, uint64_t MaxDepth);

    template <core::Color C, bool Attacking>
    std::vector<core::Move32> findPV(core::internal::StateImpl* S,
                                     uint64_t Depth) const;
    std::vector<core::Move32> findPV(core::internal::StateImpl* S) const;

    std::size_t TTSize;
    std::unique_ptr<DfPnTTBundle[]> TT;

    uint16_t Generation;
    uint64_t SearchedNodeCount;
};

} // namespace dfpn
} // namespace internal
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
