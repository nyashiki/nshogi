//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_IO_FILE_H
#define NSHOGI_IO_FILE_H

#include "../ml/azteacher.h"
#include "../ml/simpleteacher.h"

#include <fstream>
#include <cstddef>

namespace nshogi {
namespace io {
namespace file {

namespace az_teacher {

ml::AZTeacher load(std::ifstream&);
ml::AZTeacher load(const char* Source);
void save(std::ofstream&, const ml::AZTeacher&);

} // namespace az_teacher

namespace simple_teacher {

void loadAt(ml::SimpleTeacher* Dest, std::ifstream&);
std::size_t loadAt(ml::SimpleTeacher* Dest, const char* Source);
ml::SimpleTeacher load(std::ifstream&);
ml::SimpleTeacher load(const char* Source);

void save(std::ofstream&, const ml::SimpleTeacher&);

} // namespace simple_teacher

template <typename T>
T load(std::ifstream&);

template <typename T>
T load(const char* Source, std::size_t* BytesRead = nullptr);

template <typename T>
void save(std::ofstream&, const T&);

} // namespace file
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_FILE_H
