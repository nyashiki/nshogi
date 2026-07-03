//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_BATCHEDTEACHERLOADER_H
#define NSHOGI_ML_BATCHEDTEACHERLOADER_H

#include "featureextractor.h"
#include "simpleteacher.h"
#include "teacherloader.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace nshogi {
namespace ml {

struct BatchedTeacher {
 public:
    std::unique_ptr<int32_t[]> MyIds;
    std::unique_ptr<int32_t[]> OpIds;
    std::unique_ptr<int8_t[]> Results;
    std::unique_ptr<float[]> Qs;
    std::unique_ptr<int8_t[]> IsStables;
};

class BatchedTeacherLoader {
 public:
    BatchedTeacherLoader(const std::string& Path,
                         std::shared_ptr<IFeatureExtractor>,
                         std::size_t BatchSize, bool Shuffle, bool BatchShuffle,
                         std::size_t NumWorkerThreads, std::size_t Prefetch);

    ~BatchedTeacherLoader();

    BatchedTeacherLoader(const BatchedTeacherLoader&) = delete;
    BatchedTeacherLoader& operator=(const BatchedTeacherLoader&) = delete;
    BatchedTeacherLoader(BatchedTeacherLoader&&) = delete;
    BatchedTeacherLoader& operator=(BatchedTeacherLoader&&) = delete;

    std::size_t batchSize() const noexcept {
        return MyBatchSize;
    }

    std::size_t idSize() const noexcept {
        return Extractor->idSize();
    }

    std::size_t size() const noexcept;

    std::optional<BatchedTeacher> next();

    void doTask();

 private:
    std::unique_ptr<utils::PermutationGenerator> PG;

    const std::string TeacherPath;
    std::shared_ptr<IFeatureExtractor> Extractor;
    const std::size_t MyBatchSize;
    const bool ShuffleEnabled;
    const bool BatchShuffleEnabled;
    const std::size_t PrefetchFactor;
    std::size_t NumBatches;

    std::atomic<bool> Finished;
    std::mutex BatchesMutex;
    std::condition_variable ProducerCV;
    std::condition_variable ConsumerCV;

    std::vector<std::thread> Workers;
    std::atomic<std::size_t> NumRunningWorkers;
    std::atomic<std::size_t> CurrentBatchIndex;
    std::queue<BatchedTeacher> Batches;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_BATCHEDTEACHERLOADER_H
