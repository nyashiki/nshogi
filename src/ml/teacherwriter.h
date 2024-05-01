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

    static void sanitize(const TeacherLoaderForFixedSizeTeacher<TeacherType>& Loader, const std::string& OutputPath);

    static void shuffle(const TeacherLoaderForFixedSizeTeacher<TeacherType>& Loader, const std::string& OutputPath, uint64_t Seed);

 private:
    std::mutex Mutex;
    std::string Path;

};

} // namespace ml
} // namespace nshogi

#endif // #ifndef NSHOGI_ML_TEACHERWRITER_H
