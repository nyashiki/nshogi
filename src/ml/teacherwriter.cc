#include "teacherwriter.h"
#include "azteacher.h"
#include "../io/file.h"

#include <cstddef>
#include <fstream>
#include <vector>
#include <numeric>
#include <random>

namespace nshogi {
namespace ml {

template<typename TeacherType>
ThreadsafeTeacherWriter<TeacherType>::ThreadsafeTeacherWriter(const std::string& TeacherPath): Path(TeacherPath) {

}

template<typename TeacherType>
void ThreadsafeTeacherWriter<TeacherType>::write(const TeacherType& Teacher) {
    std::lock_guard<std::mutex> lk(Mutex);

    std::ofstream Ofs(Path, std::ios::out | std::ios::app);
    io::file::save(Ofs, Teacher);

    Ofs.flush();
}

template<typename TeacherType>
void ThreadsafeTeacherWriter<TeacherType>::sanitize(const TeacherLoaderForFixedSizeTeacher<TeacherType> &Loader, const std::string& OutputPath) {
    std::ofstream Ofs(OutputPath, std::ios::out | std::ios::app);

    for (std::size_t I = 0; I < Loader.size(); ++I) {
        const auto T = Loader[I];

        if (T.checkSanity(2)) {
            T.dump(Ofs);
        }
    }
}

template<typename TeacherType>
void ThreadsafeTeacherWriter<TeacherType>::shuffle(const TeacherLoaderForFixedSizeTeacher<TeacherType> &Loader, const std::string& OutputPath, uint64_t Seed) {
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

template
class ThreadsafeTeacherWriter<AZTeacher>;

template
class ThreadsafeTeacherWriter<SimpleTeacher>;

} // namespace ml
} // namespace nshogi
