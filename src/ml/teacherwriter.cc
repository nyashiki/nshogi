//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "teacherwriter.h"
#include "../io/file.h"
#include "azteacher.h"

#include <cstddef>
#include <fstream>
#include <numeric>
#include <random>
#include <vector>

namespace nshogi {
namespace ml {

template <typename TeacherType>
ThreadsafeTeacherWriter<TeacherType>::ThreadsafeTeacherWriter(
    const std::string& TeacherPath)
    : Path(TeacherPath) {
}

template <typename TeacherType>
void ThreadsafeTeacherWriter<TeacherType>::write(const TeacherType& Teacher) {
    std::lock_guard<std::mutex> lk(Mutex);

    std::ofstream Ofs(Path, std::ios::out | std::ios::app);
    io::file::save(Ofs, Teacher);

    Ofs.flush();
}

template <typename TeacherType>
void ThreadsafeTeacherWriter<TeacherType>::shuffle(
    const TeacherLoaderForFixedSizeTeacher<TeacherType>& Loader,
    const std::string& OutputPath, uint64_t Seed) {
    std::ofstream Ofs(OutputPath, std::ios::out | std::ios::app);

    std::vector<std::size_t> Indices(Loader.size());
    std::iota(Indices.begin(), Indices.end(), 0);

    std::mt19937_64 mt(Seed);

    for (std::size_t I = Indices.size() - 1; I > 0; --I) {
        std::size_t J = mt() % I;

        std::swap(Indices[I], Indices[J]);
    }

    for (const std::size_t Index : Indices) {
        const auto T = Loader[Index];
        io::file::save(Ofs, T);
    }
}

template class ThreadsafeTeacherWriter<AZTeacher>;

template class ThreadsafeTeacherWriter<SimpleTeacher>;

} // namespace ml
} // namespace nshogi
