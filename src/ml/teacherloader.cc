//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "teacherloader.h"
#include "../io/file.h"
#include "azteacher.h"
#include "simpleteacher.h"

#include <cstdint>
#include <iostream>
#include <random>

#ifdef __linux__

// Use mmap() in Linux instead of ifstream to avoid overhead.
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#endif

namespace nshogi {
namespace ml {

template <typename TeacherType>
TeacherLoaderForFixedSizeTeacher<TeacherType>::TeacherLoaderForFixedSizeTeacher(
    const std::string& TeacherPath, bool Shuffle)
    : Path(TeacherPath)
    , ShuffleEnabled(Shuffle)
    , FileSize(0)
    , MappedFile(nullptr) {

    ensureOpen();

    if (ShuffleEnabled) {
        std::random_device RD;
        PG = std::make_unique<utils::PermutationGenerator>(RD(), NumTeachers);
    }
}

template <typename TeacherType>
TeacherLoaderForFixedSizeTeacher<TeacherType>::~TeacherLoaderForFixedSizeTeacher() {
#ifdef __linux__
    if (MappedFile != nullptr) {
        ::munmap(MappedFile, FileSize);
        MappedFile = nullptr;
    }
#endif
}

template <typename TeacherType>
std::size_t TeacherLoaderForFixedSizeTeacher<TeacherType>::size() const {
    return NumTeachers;
}

template <typename TeacherType>
void TeacherLoaderForFixedSizeTeacher<TeacherType>::ensureOpen() {
#ifdef __linux__
    if (MappedFile != nullptr) {
        return;
    }

    const int FileDescriptor = ::open(Path.c_str(), O_RDONLY);
    if (FileDescriptor < 0) {
        throw std::runtime_error("File not found.");
    }

    struct stat Stat{};
    if (::fstat(FileDescriptor, &Stat) < 0) {
        ::close(FileDescriptor);
        throw std::runtime_error("Failed to get file size.");
    }

    FileSize = static_cast<std::size_t>(Stat.st_size);
    if (FileSize == 0) {
        ::close(FileDescriptor);
        throw std::runtime_error("File is empty.");
    }

    MappedFile = ::mmap(nullptr, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0);
    ::close(FileDescriptor);

    if (MappedFile == MAP_FAILED) {
        throw std::runtime_error("Failed to map file.");
    }

    io::file::load<TeacherType>(static_cast<const char*>(MappedFile), &TeacherSizeUnit);
#else
    const pid_t CurrentPid = getpid();

    if (Ifs.is_open() && CurrentPid == OpenPid) {
        return;
    }

    if (Ifs.is_open()) {
        Ifs.close();
    }

    Ifs.clear();
    Ifs.open(Path, std::ios::in | std::ios::binary);
    if (!Ifs) {
        throw std::runtime_error("File not found.");
    }

    OpenPid = CurrentPid;

    // Load one teacher entry so that we can determine
    // the size of one teacher binary.
    [[maybe_unused]] TeacherType T = io::file::load<TeacherType>(Ifs);

    TeacherSizeUnit = (std::size_t)Ifs.tellg();

    // Now we got the size of the teacher unit.
    // Hence now we can compute the number of
    // teacher entries that the file has.
    Ifs.seekg(0, std::ios_base::end);
    FileSize = (std::size_t)Ifs.tellg();
#endif

    if (FileSize % TeacherSizeUnit != 0) {
        std::cout << "FileSize: " << FileSize << std::endl;
        std::cout << "TeacherSizeUnit: " << TeacherSizeUnit << std::endl;

        throw std::runtime_error("FileSize must be divided by the unit.\n"
                                 "Unfortunately, the file is possibly broken.");
    }

    NumTeachers = FileSize / TeacherSizeUnit;
}

template <typename TeacherType>
TeacherType TeacherLoaderForFixedSizeTeacher<TeacherType>::operator[](
    std::size_t Index) {
    if (ShuffleEnabled) {
        Index = (*PG)(Index);
    }
    assert(Index < NumTeachers);

#ifdef __linux__
    // Since MappedFile can be read after fork(),
    // we can skip ensureOpen() in Linux.
#else
    ensureOpen();
#endif


#ifdef __linux__
    TeacherType T = io::file::load<TeacherType>(
        static_cast<const char*>(MappedFile) + Index * TeacherSizeUnit
    );
#else
    Ifs.clear();
    Ifs.seekg((long)(Index * TeacherSizeUnit), std::ios_base::beg);
    TeacherType T = io::file::load<TeacherType>(Ifs);
#endif

    return T;
}

template <>
void TeacherLoaderForFixedSizeTeacher<SimpleTeacher>::loadAt(
    SimpleTeacher* Dest,
    std::size_t Index
) {
    if (ShuffleEnabled) {
        Index = (*PG)(Index);
    }
    assert(Index < NumTeachers);

#ifdef __linux__
    // Same in operator[], we can skip ensureOpen() in Linux.
#else
    ensureOpen();
#endif

#ifdef __linux__
    io::file::simple_teacher::loadAt(
        Dest,
        static_cast<const char*>(MappedFile) + Index * TeacherSizeUnit
    );
#else
    Ifs.clear();
    Ifs.seekg((long)(Index * TeacherSizeUnit), std::ios_base::beg);
    io::file::simple_teacher::loadAt(Dest, Ifs);
#endif
};

template class TeacherLoaderForFixedSizeTeacher<AZTeacher>;

template class TeacherLoaderForFixedSizeTeacher<SimpleTeacher>;

} // namespace ml
} // namespace nshogi
