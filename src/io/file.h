//
// Copyright (c) 2025 @nyashiki
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

namespace nshogi {
namespace io {
namespace file {

namespace az_teacher {

ml::AZTeacher load(std::ifstream&);
void save(std::ofstream&, const ml::AZTeacher&);

} // namespace az_teacher

namespace simple_teacher {

ml::SimpleTeacher load(std::ifstream&);
void save(std::ofstream&, const ml::SimpleTeacher&);

} // namespace simple_teacher

template <typename T>
T load(std::ifstream&);

template <typename T>
void save(std::ofstream&, const T&);

} // namespace file
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_FILE_H
