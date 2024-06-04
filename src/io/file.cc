#include "file.h"

namespace nshogi {
namespace io {
namespace file {

namespace simple_teacher {

namespace {

class SimpleTeacherIO : public ml::SimpleTeacher {
 public:
    SimpleTeacherIO() = default;

    SimpleTeacherIO(const ml::SimpleTeacher& ST)
        : ml::SimpleTeacher(ST) {
    }

    ml::SimpleTeacher load(std::ifstream& Ifs) {
        uint64_t Codes[4];
        uint16_t Move16;

        Ifs.read(reinterpret_cast<char*>(Codes), 4 * sizeof(uint64_t));
        Ifs.read(reinterpret_cast<char*>(&Ply), sizeof(Ply));
        Ifs.read(reinterpret_cast<char*>(&MaxPly), sizeof(MaxPly));
        Ifs.read(reinterpret_cast<char*>(DrawValues), sizeof(DrawValues));
        Ifs.read(reinterpret_cast<char*>(&Move16), sizeof(uint16_t));
        Ifs.read(reinterpret_cast<char*>(&Winner), sizeof(Winner));

        HuffmanCode = core::HuffmanCode(Codes[3], Codes[2], Codes[1], Codes[0]);
        NextMove = core::Move16::fromValue(Move16);

        return *this;
    }

    void save(std::ofstream& Ofs) {
        uint16_t Move16 = NextMove.value();

        Ofs.write(HuffmanCode.data(), 4 * sizeof(uint64_t));
        Ofs.write(reinterpret_cast<char*>(&Ply), sizeof(Ply));
        Ofs.write(reinterpret_cast<char*>(&MaxPly), sizeof(MaxPly));
        Ofs.write(reinterpret_cast<char*>(DrawValues), sizeof(DrawValues));
        Ofs.write(reinterpret_cast<char*>(&Move16), sizeof(uint16_t));
        Ofs.write(reinterpret_cast<char*>(&Winner), sizeof(Winner));
    }
};

} // namespace

ml::SimpleTeacher load(std::ifstream& Ifs) {
    SimpleTeacherIO IO;
    return IO.load(Ifs);
}

void save(std::ofstream& Ofs, const ml::SimpleTeacher& SimpleTeacher) {
    SimpleTeacherIO IO(SimpleTeacher);
    IO.save(Ofs);
}

} // namespace simple_teacher

} // namespace file
} // namespace io
} // namespace nshogi
