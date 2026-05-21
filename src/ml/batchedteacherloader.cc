//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "batchedteacherloader.h"
#include "../ml//p.h"
#include "../core/movegenerator.h"

#include <random>

namespace nshogi {
namespace ml {

BatchedTeacherLoader::BatchedTeacherLoader(
    const std::string& Path,
    std::shared_ptr<IFeatureExtractor> Ext,
    std::size_t BatchSize,
    bool Shuffle,
    bool BatchShuffle,
    std::size_t NumWorkerThreads,
    std::size_t Prefetch
) : TeacherPath(Path)
  , Extractor(std::move(Ext))
  , MyBatchSize(BatchSize)
  , ShuffleEnabled(Shuffle)
  , BatchShuffleEnabled(BatchShuffle)
  , PrefetchFactor(Prefetch) {
    if (NumWorkerThreads == 0) {
        throw std::invalid_argument("NumWorkerThreads must be greater than 0.");
    }
    if (PrefetchFactor == 0) {
        throw std::invalid_argument("Prefetch must be greater than 0.");
    }

    {
        TeacherLoaderForFixedSizeTeacher<SimpleTeacher> TeacherLoader(Path, false, /* Version = */ 2);
        NumBatches = TeacherLoader.size() / MyBatchSize;
        if (ShuffleEnabled) {
            std::random_device RD;

            if (BatchShuffleEnabled) {
                PG = std::make_unique<utils::PermutationGenerator>(RD(), NumBatches);
            } else {
                PG = std::make_unique<utils::PermutationGenerator>(RD(), TeacherLoader.size());
            }
        }
    }

    Finished = false;

    NumRunningWorkers = NumWorkerThreads;
    CurrentBatchIndex = 0;
    for (std::size_t I = 0; I < NumWorkerThreads; ++I) {
        Workers.emplace_back(&BatchedTeacherLoader::doTask, this);
    }
}

BatchedTeacherLoader::~BatchedTeacherLoader() {
    Finished.store(true);
    ProducerCV.notify_all();
    ConsumerCV.notify_all();

    for (auto& Worker : Workers) {
        if (Worker.joinable()) {
            Worker.join();
        }
    }
}

std::size_t BatchedTeacherLoader::size() const noexcept {
    return NumBatches;
}

std::optional<BatchedTeacher> BatchedTeacherLoader::next() {
    std::unique_lock<std::mutex> Lock(BatchesMutex);
    ConsumerCV.wait(Lock, [this] {
        return !Batches.empty() || NumRunningWorkers.load() == 0;
    });

    if (!Batches.empty()) {
        BatchedTeacher Result = std::move(Batches.front());
        Batches.pop();
        Lock.unlock();

        ProducerCV.notify_one();
        return Result;
    } else {
        // No more batches will be produced.
        return std::nullopt;
    }
}

void BatchedTeacherLoader::doTask() {
    TeacherLoaderForFixedSizeTeacher<SimpleTeacher> TeacherLoader(TeacherPath, false, /* Version = */ 2);

    SimpleTeacher Teacher;

    while (true) {
        if (Finished.load()) {
            break;
        }

        const std::size_t MyIndex = CurrentBatchIndex.fetch_add(1);
        if (MyIndex >= NumBatches) {
            // All batches are consumed.
            break;
        }

        // Prepare the buffer.
        std::unique_ptr<int32_t[]> MyIds = std::make_unique<int32_t[]>(MyBatchSize * Extractor->idSize());
        std::unique_ptr<int32_t[]> OpIds = std::make_unique<int32_t[]>(MyBatchSize * Extractor->idSize());
        std::unique_ptr<int8_t[]> Results = std::make_unique<int8_t[]>(MyBatchSize);
        std::unique_ptr<float[]> Qs = std::make_unique<float[]>(MyBatchSize);
        std::unique_ptr<int8_t[]> IsStables = std::make_unique<int8_t[]>(MyBatchSize);

        for (std::size_t I = 0; I < MyBatchSize; ++I) {
            // Load the teacher.
            const std::size_t TargetIndex = [&]() {
                if (ShuffleEnabled) {
                    if (BatchShuffleEnabled) {
                        return (*PG)(MyIndex) * MyBatchSize + I;
                    } else {
                        return (*PG)(MyIndex * MyBatchSize + I);
                    }
                } else {
                    return MyIndex * MyBatchSize + I;
                }
            }();

            TeacherLoader.loadAt(&Teacher, TargetIndex);

            // Extract information can be done without lock.
            const auto State = Teacher.getState();
            int32_t MyIdsCount;
            int32_t OpIdsCount;
            Extractor->idsAt(
                MyIds.get() + I * Extractor->idSize(),
                OpIds.get() + I * Extractor->idSize(),
                &MyIdsCount,
                &OpIdsCount,
                State
            );
            assert(MyIdsCount == Extractor->idSize());
            assert(OpIdsCount == Extractor->idSize());

            if (Teacher.getWinner() == core::NoColor) {
                Results[I] = 0;
            } else {
                Results[I] =
                    (Teacher.getWinner() == State.getSideToMove()) ? 1 : -1;
            }

            Qs[I] = Teacher.q();

            IsStables[I] = 1;

            if (State.isInCheck()) {
                IsStables[I] = 0;
            } else {
                const auto NextMove = Teacher.getNextMove();
                if (State.getPosition().pieceOn(NextMove.to()) != core::PK_Empty) {
                    IsStables[I] = 0;
                }
            }
        }

        {
            std::unique_lock<std::mutex> Lock(BatchesMutex);
            ProducerCV.wait(Lock, [this] {
                return Batches.size() < PrefetchFactor * Workers.size() || Finished.load();
            });

            // Push the batch to the queue.
            Batches.push(BatchedTeacher{
                    .MyIds = std::move(MyIds),
                    .OpIds = std::move(OpIds),
                    .Results = std::move(Results),
                    .Qs = std::move(Qs),
                    .IsStables = std::move(IsStables)
                    });
        }
        ConsumerCV.notify_one();
    }

    if (NumRunningWorkers.fetch_sub(1) == 1) {
        ConsumerCV.notify_all();
    }
}

} // namespace ml
} // namespace nshogi
