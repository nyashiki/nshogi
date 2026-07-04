//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_TEACHERLOADER_H
#define NSHOGI_ML_TEACHERLOADER_H

#include "utils.h"

#include <fstream>
#include <memory>
#include <string>

namespace nshogi {
namespace ml {

template <typename TeacherType>
class TeacherLoaderForFixedSizeTeacher {
 public:
    TeacherLoaderForFixedSizeTeacher(const std::string& TeacherPath,
                                     bool Shuffle, int32_t FileVersion);
    ~TeacherLoaderForFixedSizeTeacher();

    TeacherType operator[](std::size_t Index);
    void loadAt(TeacherType* Dest, std::size_t Index);

    std::size_t size() const;

    void ensureOpen();

 private:
    const std::string Path;
    const bool ShuffleEnabled;
    const int32_t Version;

    std::ifstream Ifs;
    pid_t OpenPid;
    std::size_t FileSize;

    // Used in Linux for mmap().
    void* MappedFile;

    std::size_t TeacherSizeUnit;
    std::size_t NumTeachers;

    std::unique_ptr<utils::PermutationGenerator> PG;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_TEACHERLOADER_H
