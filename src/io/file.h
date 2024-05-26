#ifndef NSHOGI_IO_FILE_H
#define NSHOGI_IO_FILE_H

#include "../ml/simpleteacher.h"

#include <fstream>

namespace nshogi {
namespace io {
namespace file {

namespace simple_teacher {

ml::SimpleTeacher load(std::ifstream&);

} // namespace simple_teacher

} // namespace file
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_FILE_H
