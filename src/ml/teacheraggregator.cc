//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "teacheraggregator.h"
#include "../core/statebuilder.h"
#include "../io/file.h"
#include "../io/sfen.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <stdexcept>

namespace nshogi {
namespace ml {

namespace internal {

struct TeacherAggregationEntry {
    std::vector<std::pair<uint16_t, uint64_t>> MoveCounts;
    uint64_t Count = 0;
    double VSum = 0.0;
    double BlackDrawValueSum = 0.0;
    double WhiteDrawValueSum = 0.0;
    uint64_t NumBlackWins = 0;
    uint64_t NumWhiteWins = 0;
    uint64_t NumDeclared = 0;
    uint16_t GamePlyMax = 0;
    uint16_t MaxPlyMax = 0;

    void accumulate(const SimpleTeacher&);
    void clear();
};

void TeacherAggregationEntry::accumulate(const SimpleTeacher& Teacher) {
    const core::StateConfig Config = Teacher.getConfig();

    const uint16_t MoveValue = Teacher.getNextMove().value();
    bool MoveFound = false;
    for (auto& [Value, MoveCount] : MoveCounts) {
        if (Value == MoveValue) {
            ++MoveCount;
            MoveFound = true;
            break;
        }
    }
    if (!MoveFound) {
        MoveCounts.emplace_back(MoveValue, 1);
    }

    ++Count;
    VSum += Teacher.v();
    BlackDrawValueSum += Config.BlackDrawValue;
    WhiteDrawValueSum += Config.WhiteDrawValue;
    GamePlyMax = std::max(GamePlyMax, Teacher.gamePly());
    MaxPlyMax = std::max(MaxPlyMax, Config.MaxPly);

    if (Teacher.getWinner() == core::Black) {
        ++NumBlackWins;
    } else if (Teacher.getWinner() == core::White) {
        ++NumWhiteWins;
    }

    if (Teacher.declared()) {
        ++NumDeclared;
    }
}

void TeacherAggregationEntry::clear() {
    MoveCounts.clear();
    Count = 0;
    VSum = 0.0;
    BlackDrawValueSum = 0.0;
    WhiteDrawValueSum = 0.0;
    NumBlackWins = 0;
    NumWhiteWins = 0;
    NumDeclared = 0;
    GamePlyMax = 0;
    MaxPlyMax = 0;
}

///
/// @brief Build one AZTeacher from the aggregation state of a position.
///
AZTeacher aggregateEntry(const core::HuffmanCode& HC,
                         const TeacherAggregationEntry& Entry,
                         core::EndingRule Rule) {
    assert(Entry.Count > 0);

    const core::Position Pos = core::HuffmanCode::decode(HC);
    const core::State State = core::StateBuilder::newState(Pos);

    AZTeacher T;

    std::memset(T.Sfen, 0, sizeof(T.Sfen));
    const std::string Sfen = io::sfen::positionToSfen(Pos);
    std::snprintf(T.Sfen, sizeof(T.Sfen), "%s", Sfen.c_str());

    T.SideToMove = Pos.sideToMove();

    const uint64_t NumDraws =
        Entry.Count - Entry.NumBlackWins - Entry.NumWhiteWins;
    if (NumDraws > Entry.NumBlackWins && NumDraws > Entry.NumWhiteWins) {
        T.Winner = core::NoColor;
    } else if (Entry.NumBlackWins > Entry.NumWhiteWins) {
        T.Winner = core::Black;
    } else if (Entry.NumWhiteWins > Entry.NumBlackWins) {
        T.Winner = core::White;
    } else {
        T.Winner = core::NoColor;
    }

    // The most frequent moves come first. Ties are broken by the
    // move value to keep the output deterministic.
    std::vector<std::pair<uint16_t, uint64_t>> MoveCounts = Entry.MoveCounts;
    std::sort(MoveCounts.begin(), MoveCounts.end(),
              [](const auto& Elem1, const auto& Elem2) {
                  if (Elem1.second != Elem2.second) {
                      return Elem1.second > Elem2.second;
                  }
                  return Elem1.first < Elem2.first;
              });

    const std::size_t NumMoves =
        std::min<std::size_t>(MoveCounts.size(), T.Moves.size());
    T.NumMoves = (uint8_t)NumMoves;

    // Visits is 16 bits wide: when a count exceeds its maximum,
    // scale all the counts down proportionally.
    const uint64_t MaxCount = MoveCounts.empty() ? 0 : MoveCounts[0].second;
    const double Scale = (MaxCount > std::numeric_limits<uint16_t>::max())
                             ? (double)std::numeric_limits<uint16_t>::max() /
                                   (double)MaxCount
                             : 1.0;

    for (std::size_t I = 0; I < T.Moves.size(); ++I) {
        T.Moves[I].fill('\0');
        T.Visits[I] = 0;

        if (I >= NumMoves) {
            continue;
        }

        const core::Move32 Move = State.getMove32FromMove16(
            core::Move16::fromValue(MoveCounts[I].first));
        const std::string MoveSfen = io::sfen::move32ToSfen(Move);
        std::snprintf(T.Moves[I].data(), T.Moves[I].size(), "%s",
                      MoveSfen.c_str());

        const uint64_t Scaled = std::max<uint64_t>(
            1, (uint64_t)std::llround((double)MoveCounts[I].second * Scale));
        T.Visits[I] = (uint16_t)std::min<uint64_t>(
            Scaled, std::numeric_limits<uint16_t>::max());
    }

    T.EndingRule = Rule;
    T.MaxPly = Entry.MaxPlyMax;
    T.BlackDrawValue = (float)(Entry.BlackDrawValueSum / (double)Entry.Count);
    T.WhiteDrawValue = (float)(Entry.WhiteDrawValueSum / (double)Entry.Count);

    // The majority vote (false on a tie).
    T.Declared = Entry.NumDeclared * 2 > Entry.Count;

    T.V = (float)(Entry.VSum / (double)Entry.Count);

    // The win rate of the player to move computed from the winners of
    // the aggregated records, counting a draw as a half win.
    const uint64_t NumStmWins = (T.SideToMove == core::Black)
                                    ? Entry.NumBlackWins
                                    : Entry.NumWhiteWins;
    T.Q = (float)(((double)NumStmWins + 0.5 * (double)NumDraws) /
                  (double)Entry.Count);

    T.GamePly = Entry.GamePlyMax;

    return T;
}

} // namespace internal

struct TeacherAggregator::Impl {
    using PositionKey = std::array<uint64_t, 4>;

    struct PositionKeyHash {
        std::size_t operator()(const PositionKey& Key) const {
            uint64_t Hash = 0x9e3779b97f4a7c15ULL;

            for (const uint64_t K : Key) {
                Hash ^= K;
                Hash *= 0xbf58476d1ce4e5b9ULL;
                Hash ^= Hash >> 31;
            }

            return static_cast<std::size_t>(Hash);
        }
    };

    std::unordered_map<PositionKey, internal::TeacherAggregationEntry,
                       PositionKeyHash>
        Entries;
    std::size_t NumTeachers = 0;
};

TeacherAggregator::TeacherAggregator()
    : PImpl(std::make_unique<Impl>()) {
}

TeacherAggregator::~TeacherAggregator() = default;

void TeacherAggregator::add(const SimpleTeacher& Teacher) {
    assert(core::HuffmanCode::size() == sizeof(Impl::PositionKey));

    Impl::PositionKey Key;
    std::memcpy(Key.data(), Teacher.getHuffmanCode().data(),
                sizeof(Impl::PositionKey));

    PImpl->Entries[Key].accumulate(Teacher);
    ++PImpl->NumTeachers;
}

std::size_t TeacherAggregator::numAddedTeachers() const {
    return PImpl->NumTeachers;
}

std::size_t TeacherAggregator::numUniquePositions() const {
    return PImpl->Entries.size();
}

std::vector<AZTeacher> TeacherAggregator::aggregate(
    core::EndingRule Rule) const {
    // Sort the keys so that the output does not depend on the
    // iteration order of the hash map.
    std::vector<const std::pair<const Impl::PositionKey,
                                internal::TeacherAggregationEntry>*>
        Sorted;
    Sorted.reserve(PImpl->Entries.size());
    for (const auto& KeyAndEntry : PImpl->Entries) {
        Sorted.emplace_back(&KeyAndEntry);
    }
    std::sort(Sorted.begin(), Sorted.end(),
              [](const auto* Elem1, const auto* Elem2) {
                  return Elem1->first < Elem2->first;
              });

    std::vector<AZTeacher> Teachers;
    Teachers.reserve(Sorted.size());

    for (const auto* KeyAndEntry : Sorted) {
        const core::HuffmanCode HC(
            reinterpret_cast<const char*>(KeyAndEntry->first.data()));
        Teachers.emplace_back(
            internal::aggregateEntry(HC, KeyAndEntry->second, Rule));
    }

    return Teachers;
}

namespace {

// Compares two 32-byte huffman codes in the same order as
// std::array<uint64_t, 4>'s operator< so that ExternalTeacherAggregator
// emits the records in exactly the same order as TeacherAggregator.
int compareHuffmanCodeBytes(const char* A, const char* B) {
    for (std::size_t I = 0; I < 4; ++I) {
        uint64_t WA, WB;
        std::memcpy(&WA, A + I * sizeof(uint64_t), sizeof(uint64_t));
        std::memcpy(&WB, B + I * sizeof(uint64_t), sizeof(uint64_t));

        if (WA != WB) {
            return WA < WB ? -1 : 1;
        }
    }
    return 0;
}

// A buffered reader of one sorted run file.
class RunReader {
 public:
    RunReader(const std::string& Path, std::size_t RecordSize)
        : Ifs(Path, std::ios::binary)
        , RecordSize_(RecordSize)
        , Buffer(BufferRecordCount * RecordSize)
        , Pos(0)
        , Filled(0) {
        if (!Ifs) {
            throw std::runtime_error("Failed to open a run file: " + Path);
        }
        refill();
    }

    bool exhausted() const {
        return Pos >= Filled;
    }

    const char* current() const {
        return Buffer.data() + Pos * RecordSize_;
    }

    void advance() {
        ++Pos;
        if (Pos >= Filled) {
            refill();
        }
    }

 private:
    static constexpr std::size_t BufferRecordCount = 65536;

    void refill() {
        Ifs.read(Buffer.data(),
                 (std::streamsize)(BufferRecordCount * RecordSize_));
        const std::size_t BytesRead = (std::size_t)Ifs.gcount();
        Filled = BytesRead / RecordSize_;
        Pos = 0;
    }

    std::ifstream Ifs;
    const std::size_t RecordSize_;
    std::vector<char> Buffer;
    std::size_t Pos;
    std::size_t Filled;
};

} // namespace

ExternalTeacherAggregator::ExternalTeacherAggregator(
    const std::string& TempDir, std::size_t MemoryBytes, int32_t FileVersion)
    : TempDirectory(TempDir)
    , MaxMemoryBytes(MemoryBytes)
    , Version(FileVersion) {
}

void ExternalTeacherAggregator::addTeacherFile(const std::string& Path) {
    InputPaths.emplace_back(Path);
}

uint64_t ExternalTeacherAggregator::aggregate(const std::string& OutputPath,
                                              core::EndingRule Rule) const {
    static constexpr std::size_t MaxNumRuns = 256;
    static constexpr std::size_t HuffmanCodeSize = 32;

    std::ofstream Ofs(OutputPath, std::ios::binary);
    if (!Ofs) {
        throw std::runtime_error("Failed to open the output file: " +
                                 OutputPath);
    }

    // Determine the record size by parsing one record.
    std::size_t RecordSize = 0;
    uint64_t TotalRecords = 0;
    for (const std::string& Path : InputPaths) {
        std::ifstream Ifs(Path, std::ios::binary);
        if (!Ifs) {
            throw std::runtime_error("Failed to open an input file: " + Path);
        }
        Ifs.seekg(0, std::ios::end);
        const uint64_t FileSize = (uint64_t)Ifs.tellg();
        if (FileSize == 0) {
            continue;
        }

        if (RecordSize == 0) {
            char Probe[1024] = {};
            Ifs.seekg(0);
            Ifs.read(Probe, (std::streamsize)std::min<uint64_t>(
                                FileSize, sizeof(Probe)));
            SimpleTeacher T;
            RecordSize = io::file::simple_teacher::loadAt(&T, Probe, Version);
        }

        if (FileSize % RecordSize != 0) {
            throw std::runtime_error(
                "The file size is not a multiple of the record size: " + Path);
        }
        TotalRecords += FileSize / RecordSize;
    }

    if (TotalRecords == 0) {
        return 0;
    }

    // ----- Phase 1: sorted run generation. -----
    const std::size_t ChunkRecords = std::min<std::size_t>(
        std::max<std::size_t>(1,
                              MaxMemoryBytes /
                                  (RecordSize + sizeof(uint32_t))),
        std::numeric_limits<uint32_t>::max());

    if ((TotalRecords + ChunkRecords - 1) / ChunkRecords > MaxNumRuns) {
        throw std::runtime_error(
            "Too many sorted runs: increase the memory budget.");
    }

    std::vector<char> Chunk(ChunkRecords * RecordSize);
    std::vector<uint32_t> Order;
    Order.reserve(ChunkRecords);

    std::vector<std::string> RunPaths;

    // Sorts the first `NumRecords` records buffered in `Chunk` by the
    // huffman code and writes the whole records out as one sorted run
    // file (the run file has the same record format as the input).
    const auto FlushRun = [&](std::size_t NumRecords) {
        Order.resize(NumRecords);
        for (std::size_t I = 0; I < NumRecords; ++I) {
            Order[I] = (uint32_t)I;
        }
        // Ties are broken by the input order so that the records of one
        // position are accumulated in exactly the same order as
        // TeacherAggregator::add() would (this makes the floating-point
        // sums, and thus the output, bit-identical).
        std::sort(Order.begin(), Order.end(),
                  [&](const uint32_t Lhs, const uint32_t Rhs) {
                      const int Cmp = compareHuffmanCodeBytes(
                          Chunk.data() + (std::size_t)Lhs * RecordSize,
                          Chunk.data() + (std::size_t)Rhs * RecordSize);
                      if (Cmp != 0) {
                          return Cmp < 0;
                      }
                      return Lhs < Rhs;
                  });

        const std::string RunPath =
            TempDirectory + "/nshogi_teacher_aggregator_run_" +
            std::to_string(RunPaths.size()) + ".tmp";
        std::ofstream RunOfs(RunPath, std::ios::binary);
        if (!RunOfs) {
            throw std::runtime_error("Failed to open a run file: " + RunPath);
        }
        for (std::size_t I = 0; I < NumRecords; ++I) {
            RunOfs.write(Chunk.data() + (std::size_t)Order[I] * RecordSize,
                         (std::streamsize)RecordSize);
        }
        RunPaths.emplace_back(RunPath);
    };

    std::size_t Buffered = 0;
    for (const std::string& Path : InputPaths) {
        std::ifstream Ifs(Path, std::ios::binary);

        while (true) {
            Ifs.read(Chunk.data() + Buffered * RecordSize,
                     (std::streamsize)((ChunkRecords - Buffered) *
                                       RecordSize));
            Buffered += (std::size_t)Ifs.gcount() / RecordSize;

            if (Buffered == ChunkRecords) {
                FlushRun(Buffered);
                Buffered = 0;
            }

            if (!Ifs || Ifs.eof()) {
                break;
            }
        }
    }
    if (Buffered > 0) {
        FlushRun(Buffered);
    }

    Chunk.clear();
    Chunk.shrink_to_fit();
    Order.clear();
    Order.shrink_to_fit();

    // ----- Phase 2 and 3: merge the runs and aggregate on the fly. -----
    std::vector<RunReader> Readers;
    Readers.reserve(RunPaths.size());
    for (const std::string& RunPath : RunPaths) {
        Readers.emplace_back(RunPath, RecordSize);
    }

    const auto HeapGreater = [&](const std::size_t Lhs, const std::size_t Rhs) {
        const int Cmp = compareHuffmanCodeBytes(Readers[Lhs].current(),
                                                Readers[Rhs].current());
        if (Cmp != 0) {
            return Cmp > 0;
        }
        return Lhs > Rhs;
    };

    // A min-heap of the run indices ordered by each run's current
    // record, used to pick which run holds the smallest huffman code
    // next (i.e., the k-way merge of the sorted runs).
    std::priority_queue<std::size_t, std::vector<std::size_t>,
                        decltype(HeapGreater)>
        Heap(HeapGreater);
    for (std::size_t I = 0; I < Readers.size(); ++I) {
        if (!Readers[I].exhausted()) {
            Heap.push(I);
        }
    }

    internal::TeacherAggregationEntry Entry;
    SimpleTeacher Teacher;
    char CurrentKey[HuffmanCodeSize];
    bool HasCurrent = false;
    uint64_t NumUnique = 0;

    const auto Finalize = [&]() {
        const core::HuffmanCode HC(CurrentKey);
        const AZTeacher T = internal::aggregateEntry(HC, Entry, Rule);
        io::file::az_teacher::save(Ofs, T);
        Entry.clear();
        ++NumUnique;
    };

    while (!Heap.empty()) {
        const std::size_t RunIndex = Heap.top();
        Heap.pop();

        const char* Record = Readers[RunIndex].current();

        if (!HasCurrent ||
            compareHuffmanCodeBytes(Record, CurrentKey) != 0) {
            // A new position has arrived: the previous position (if
            // any) is complete, so save it and start a new entry.
            if (HasCurrent) {
                Finalize();
            }
            std::memcpy(CurrentKey, Record, HuffmanCodeSize);
            HasCurrent = true;
        }

        io::file::simple_teacher::loadAt(&Teacher, Record, Version);
        Entry.accumulate(Teacher);

        Readers[RunIndex].advance();
        if (!Readers[RunIndex].exhausted()) {
            Heap.push(RunIndex);
        }
    }

    if (HasCurrent) {
        Finalize();
    }

    Readers.clear();
    for (const std::string& RunPath : RunPaths) {
        std::remove(RunPath.c_str());
    }

    return NumUnique;
}

} // namespace ml
} // namespace nshogi
