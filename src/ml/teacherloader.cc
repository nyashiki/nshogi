#include "teacherloader.h"
#include "azteacher.h"
#include "simpleteacher.h"
#include "../io/file.h"

#include <cstdint>
#include <fstream>
#include <iostream>


namespace nshogi {
namespace ml {

template<typename TeacherType>
TeacherLoaderForFixedSizeTeacher<TeacherType>::TeacherLoaderForFixedSizeTeacher(const std::string& TeacherPath): Path(TeacherPath) {
    std::ifstream Ifs(Path, std::ios::in | std::ios::binary);

    if (!Ifs) {
        throw std::runtime_error("File not found.");
    }

    // Load one teacher entry so that we can determine
    // the size of one teacher binary.
    [[maybe_unused]] TeacherType T = io::file::load<TeacherType>(Ifs);

    TeacherSizeUnit = (std::size_t)Ifs.tellg();

    // Now we got the size of the teacher unit.
    // Hence now we can compute the number of
    // teacher entries that the file has.

    Ifs.seekg(0, std::ios_base::end);

    const std::size_t FileSize = (std::size_t)Ifs.tellg();

    if (FileSize % TeacherSizeUnit != 0) {
        std::cout << "FileSize: " << FileSize << std::endl;
        std::cout << "TeacherSizeUnit: " << TeacherSizeUnit << std::endl;

        throw std::runtime_error(
            "FileSize must be divided by the unit.\n"
            "Unfortunately, the file is possibly broken.");
    }

    NumTeachers = FileSize / TeacherSizeUnit;
}

template<typename TeacherType>
std::size_t TeacherLoaderForFixedSizeTeacher<TeacherType>::size() const {
    return NumTeachers;
}

template<typename TeacherType>
TeacherType TeacherLoaderForFixedSizeTeacher<TeacherType>::operator[](std::size_t Index) const {
    assert(Index < NumTeachers);

    std::ifstream Ifs(Path, std::ios::in | std::ios::binary);

    Ifs.seekg((long)(Index * TeacherSizeUnit), std::ios_base::beg);

    TeacherType T = io::file::load<TeacherType>(Ifs);

    return T;
}

template
class TeacherLoaderForFixedSizeTeacher<AZTeacher>;

template
class TeacherLoaderForFixedSizeTeacher<SimpleTeacher>;

} // namespace ml
} // namespace nshogi
