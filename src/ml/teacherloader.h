#ifndef NSHOGI_ML_TEACHERLOADER_H
#define NSHOGI_ML_TEACHERLOADER_H


#include <string>

namespace nshogi {
namespace ml {

template<typename TeacherType>
class TeacherLoaderForFixedSizeTeacher {
 public:
    TeacherLoaderForFixedSizeTeacher(const std::string& TeacherPath);

    TeacherType operator[](std::size_t Index) const;

    std::size_t size() const;

 private:
    const std::string Path;
    std::size_t TeacherSizeUnit;
    std::size_t NumTeachers;
};


} // namespace ml
} // namespace nshogi


#endif // #ifndef NSHOGI_ML_TEACHERLOADER_H
