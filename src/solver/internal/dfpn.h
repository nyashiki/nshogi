//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "../dfpn.h"
#include "../../core/internal/stateimpl.h"
#include "../../core/types.h"

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
        return ProofNumber == V.ProofNumber && DisproofNumber == V.DisproofNumber;
    }

    uint32_t ProofNumber;
    uint32_t DisproofNumber;

    static constexpr uint32_t Infinity = 1 << 20;
};

// sizeof(DfPnTTEntry) == 32 byte.
struct DfPnTTEntry {
 public:
    uint64_t hash() const {
        return DataU64[0];
    }

    core::Stands standsSideToMove() const {
        return static_cast<core::Stands>(DataU32[2]);
    }

    uint32_t proofNumber() const {
        return DataU32[3];
    }

    uint32_t disproofNumber() const {
        return DataU32[4];
    }

    uint8_t generation() const {
        return DataU8[20];
    }

    void setHash(uint64_t Hash) {
        DataU64[0] = Hash;
    }

    void setStandsSideToMove(core::Stands Stands) {
        DataU32[2] = static_cast<uint32_t>(Stands);
    }

    void setProofNumber(uint32_t Proof) {
        DataU32[3] = Proof;
    }

    void setDisproofNumber(uint32_t Disproof) {
        DataU32[4] = Disproof;
    }

    void setGeneration(uint8_t Generation) {
        DataU8[20] = Generation;
    }

 private:
    union {
        // DataU64[0]: hash.
        // DataU64[1-3]: unused.
        uint64_t DataU64[4];

        // DataU32[0-1]: unused.
        // DataU32[2]: `Stands` for the side to move.
        // DataU32[3]: proof number.
        // DataU32[4]: disproof number.
        // DataU32[5-7]: unused.
        uint32_t DataU32[8];

        // DataU8[0-19]: unused.
        // Datau8[20]: generation.
        uint8_t DataU8[32];
    };
};

// sizeof(DfPnTTBundle) == 256 byte.
struct alignas(256) DfPnTTBundle {
    static constexpr std::size_t BundleSize = 8;
    DfPnTTEntry Entries[BundleSize];
};

class SolverImpl {
 public:
    SolverImpl(std::size_t MemoryMB, uint64_t MaxDepth, uint64_t MaxNodeCount);
    ~SolverImpl();

    core::Move32 solve(core::State* S);
    std::vector<core::Move32> solveWithPV(core::State* S);

    uint64_t searchedNodeCount() const;

 private:
    template <core::Color C>
    class TTSaver {
     public:
        TTSaver(SolverImpl* SI, core::internal::StateImpl* S, uint64_t Depth, DfPnValue* Value)
            : Impl(SI)
            , State(S)
            , ThisDepth(Depth)
            , Target(Value) {
        }

        ~TTSaver() {
            Impl->storeToTT<C>(State, ThisDepth, *Target);
        }

     private:
        SolverImpl* Impl;
        core::internal::StateImpl* State;
        uint64_t ThisDepth;
        DfPnValue* Target;
    };

    template <core::Color C>
    void storeToTT(core::internal::StateImpl* S, uint64_t Depth, const DfPnValue& Value);

    template <core::Color C>
    DfPnValue loadFromTT(core::internal::StateImpl* S, uint64_t Depth) const;

    template <core::Color C, bool Attacking>
    core::Move32 search(core::internal::StateImpl* S, uint64_t Depth, DfPnValue* ThisValue, DfPnValue* Threshold);

    template <core::Color C, bool Attacking>
    std::vector<core::Move32> findPV(core::internal::StateImpl* S, uint64_t Depth) const;
    std::vector<core::Move32> findPV(core::internal::StateImpl* S) const;

    std::size_t TTSize;
    std::unique_ptr<DfPnTTBundle[]> TT;

    const uint64_t MaxDepth;
    const uint64_t MaxNodeCount;
    uint8_t Generation;
    uint64_t SearchedNodeCount;
};

} // namespace dfpn
} // namespaec internal
} // namespace solver
} // namespace nshogi

#endif // #ifndef NSHOGI_SOLVER_INTERNAL_DFPN_H
