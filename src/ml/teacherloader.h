//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_TEACHERLOADER_H
#define NSHOGI_ML_TEACHERLOADER_H

#include "utils.h"

#include <memory>
#include <string>

namespace nshogi {
namespace ml {

template <typename TeacherType>
class TeacherLoaderForFixedSizeTeacher {
 public:
    TeacherLoaderForFixedSizeTeacher(const std::string& TeacherPath,
                                     bool Shuffle);

    TeacherType operator[](std::size_t Index) const;

    std::size_t size() const;

 private:
    const std::string Path;
    std::size_t TeacherSizeUnit;
    std::size_t NumTeachers;

    const bool ShuffleEnabled;
    std::unique_ptr<utils::PermutationGenerator> PG;
};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_TEACHERLOADER_H
