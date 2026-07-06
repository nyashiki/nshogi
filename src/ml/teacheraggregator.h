//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_TEACHERAGGREGATOR_H
#define NSHOGI_ML_TEACHERAGGREGATOR_H

#include "azteacher.h"
#include "simpleteacher.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nshogi {
namespace ml {

///
/// @class TeacherAggregator
/// @brief Aggregates SimpleTeacher records that share the same position
///        (the ply is not distinguished) into one AZTeacher per unique
///        position. All the aggregation states are kept in memory.
///
/// Aggregation rules:
///  - Moves/Visits: the frequency distribution of the next moves
///    (the most frequent `AZTeacher::NumSavedPlayouts` moves are kept).
///    When a count exceeds the 16-bit Visits, all the counts are
///    scaled down proportionally.
///  - V and Q: the averages over the aggregated records.
///  - Winner: the color that won more games. NoColor when the draws
///    are the most frequent outcome or when the wins are tied.
///  - GamePly and MaxPly: the maxima over the aggregated records.
///  - BlackDrawValue and WhiteDrawValue: the averages.
///  - Declared: the majority vote over the aggregated records
///    (false on a tie).
///
class TeacherAggregator {
 public:
    TeacherAggregator();
    ~TeacherAggregator();

    /// Accumulate one SimpleTeacher record.
    void add(const SimpleTeacher&);

    /// The number of records accumulated so far.
    std::size_t numAddedTeachers() const;

    /// The number of unique positions seen so far.
    std::size_t numUniquePositions() const;

    /// Build one AZTeacher per unique position. The output is sorted
    /// by the huffman code of the position so that it is deterministic.
    /// @param Rule The ending rule to be set on every output teacher.
    std::vector<AZTeacher> aggregate(core::EndingRule Rule) const;

 private:
    struct Impl;
    std::unique_ptr<Impl> PImpl;
};

///
/// @class ExternalTeacherAggregator
/// @brief The external-sort variant of TeacherAggregator for inputs
///        that do not fit in memory.
///
/// The input files are split into sorted runs of at most
/// `MaxMemoryBytes`, the runs are merged into one stream ordered by
/// the huffman code, and the records of each position (which arrive
/// contiguously) are aggregated on the fly with the same rules as
/// TeacherAggregator. The output is therefore identical to that of
/// TeacherAggregator applied to the same records.
///
class ExternalTeacherAggregator {
 public:
    /// @param TempDirectory The directory for the temporary sorted runs.
    /// @param MaxMemoryBytes The memory budget of the run generation.
    /// @param FileVersion The version of the input SimpleTeacher files.
    ExternalTeacherAggregator(const std::string& TempDirectory,
                              std::size_t MaxMemoryBytes,
                              int32_t FileVersion = 2);

    /// Register one input SimpleTeacher file.
    void addTeacherFile(const std::string& Path);

    /// Aggregate all the registered files and save one AZTeacher per
    /// unique position to `OutputPath`, sorted by the huffman code.
    /// @return The number of unique positions written.
    uint64_t aggregate(const std::string& OutputPath,
                       core::EndingRule Rule) const;

 private:
    const std::string TempDirectory;
    const std::size_t MaxMemoryBytes;
    const int32_t Version;

    std::vector<std::string> InputPaths;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_TEACHERAGGREGATOR_H
