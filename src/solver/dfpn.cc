//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "dfpn.h"
#include "internal/mate1ply.h"
#include "../core/internal/stateadapter.h"
#include "../core/internal/movegenerator.h"

#include <algorithm>
#include <numeric>

#include <iostream>
#include "../io/sfen.h"

namespace nshogi {
namespace solver {
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


class Solver::SolverImpl {
 public:
    SolverImpl();
    ~SolverImpl();

    core::Move32 solve(core::State* S);

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

    std::size_t TTSize;
    std::unique_ptr<DfPnTTBundle[]> TT;

    uint8_t Generation;
};

Solver::SolverImpl::SolverImpl() {
    static_assert(sizeof(DfPnTTEntry) == 32);
    static_assert(sizeof(DfPnTTBundle) == 256);

    TTSize = 2048ULL * 1024ULL * 1024ULL / sizeof(DfPnTTBundle);
    TT = std::make_unique<DfPnTTBundle[]>(TTSize);

    for (std::size_t I = 0; I < TTSize; ++I) {
        for (std::size_t J = 0; J < DfPnTTBundle::BundleSize; ++J) {
            TT[I].Entries[J].setGeneration(0);
        }
    }

    Generation = 0;
}

Solver::SolverImpl::~SolverImpl() {
}

template <core::Color C>
void Solver::SolverImpl::storeToTT(
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
DfPnValue Solver::SolverImpl::loadFromTT(core::internal::StateImpl* S, uint64_t Depth) const {
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
core::Move32 Solver::SolverImpl::search(core::internal::StateImpl* S, uint64_t Depth, DfPnValue* ThisValue, DfPnValue* Threshold) {
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

    // Step 2: search.
    if constexpr (Attacking) { // OR node.
        while (true) {
            uint32_t MinProof = DfPnValue::Infinity;
            uint32_t MinProof2 = DfPnValue::Infinity;
            uint32_t SumDisproof = 0;
            DfPnValue BestChildValue;
            core::Move32 BestMove;
            for (const core::Move32 Move : Moves) {
                S->doMove(Move);
                DfPnValue ChildValue = loadFromTT<~C>(S, Depth + 1);
                S->undoMove();

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

            S->doMove(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue, &ChildThreshold);
            S->undoMove();

            if (BestChildValue.ProofNumber == 0) {
                ThisValue->ProofNumber = 0;
                ThisValue->DisproofNumber = DfPnValue::Infinity;
                return BestMove;
            }
       }
    } else { // AND node.
        while (true) {
            uint32_t SumProof = 0;
            uint32_t MinDisproof = DfPnValue::Infinity;
            uint32_t MinDisproof2 = DfPnValue::Infinity;
            DfPnValue BestChildValue;
            core::Move32 BestMove;

            for (const core::Move32 Move : Moves) {
                S->doMove(Move);
                DfPnValue ChildValue = loadFromTT<~C>(S, Depth + 1);
                S->undoMove();

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

            S->doMove(BestMove);
            search<~C, !Attacking>(S, Depth + 1, &BestChildValue, &ChildThreshold);
            S->undoMove();

            if (BestChildValue.DisproofNumber == 0) {
                ThisValue->ProofNumber = DfPnValue::Infinity;
                ThisValue->DisproofNumber = 0;
                return BestMove;
            }
        }
    }
}

core::Move32 Solver::SolverImpl::solve(core::State* S) {
    ++Generation;

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
    }
}

Solver::Solver()
    : Impl(std::make_unique<SolverImpl>()) {
}

Solver::~Solver() {
}

core::Move32 Solver::solve(core::State* S) {
    return Impl->solve(S);
}

} // namespace dfpn
} // namespace solver
} // namespace nshogi
