//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_ML_TEACHERWRITER_H
#define NSHOGI_ML_TEACHERWRITER_H

#include <string>
#include <mutex>

#include "teacherloader.h"

namespace nshogi {
namespace ml {

template<typename TeacherType>
class ThreadsafeTeacherWriter {
 public:
    ThreadsafeTeacherWriter(const std::string& TeacherPath);

    void write(const TeacherType& Teacher);
    static void shuffle(const TeacherLoaderForFixedSizeTeacher<TeacherType>& Loader, const std::string& OutputPath, uint64_t Seed);

 private:
    std::mutex Mutex;
    std::string Path;

};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_TEACHERWRITER_H
