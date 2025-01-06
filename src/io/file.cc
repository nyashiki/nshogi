//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "file.h"

#include <cstring>

namespace nshogi {
namespace io {
namespace file {

namespace az_teacher {

namespace {

class AZTeacherIO : public ml::AZTeacher {
 public:
    AZTeacherIO() = default;

    AZTeacherIO(const ml::AZTeacher& AT)
        : ml::AZTeacher(AT) {
    }

    ml::AZTeacher load_0_1_0(std::ifstream& Ifs) {
        Ifs.read(Sfen, SfenCStrLength * sizeof(char));

        Ifs.read(reinterpret_cast<char*>(&SideToMove), sizeof(core::Color));
        Ifs.read(reinterpret_cast<char*>(&Winner), sizeof(core::Color));
        Ifs.read(reinterpret_cast<char*>(&NumMoves), sizeof(uint8_t));
        for (std::size_t I = 0; I < NumSavedPlayouts; ++I) {
            Ifs.read(Moves[I].data(), 6 * sizeof(char));
        }
        Ifs.read(reinterpret_cast<char*>(Visits.data()),
                 NumSavedPlayouts * sizeof(uint16_t));

        Ifs.read(reinterpret_cast<char*>(&EndingRule),
                 sizeof(core::EndingRule));
        Ifs.read(reinterpret_cast<char*>(&MaxPly), sizeof(uint16_t));
        Ifs.read(reinterpret_cast<char*>(&BlackDrawValue), sizeof(float));
        Ifs.read(reinterpret_cast<char*>(&WhiteDrawValue), sizeof(float));

        Ifs.read(reinterpret_cast<char*>(&Declared), sizeof(bool));

        int16_t Dummy = 0;
        Ifs.read(reinterpret_cast<char*>(&Dummy), sizeof(int16_t));

        if (Dummy != 0xabc) {
            throw std::runtime_error("Parity incoincidence.");
        }

        return *this;
    }

    ml::AZTeacher load(std::ifstream& Ifs) {
        char Version[8];
        Ifs.read(Version, 8 * sizeof(char));

        if (std::strcmp(Version, "0.1.0") == 0) {
            return load_0_1_0(Ifs);
        }

        const std::string ErrorMessage =
            "Unknown version (" + std::string(Version) + ").";
        throw std::runtime_error(ErrorMessage.c_str());
    }

    void save_0_1_0(std::ofstream& Ofs) {
        const char Version[8] = "0.1.0";

        Ofs.write(Version, 8 * sizeof(char));
        Ofs.write(Sfen, SfenCStrLength * sizeof(char));

        Ofs.write(reinterpret_cast<const char*>(&SideToMove),
                  sizeof(core::Color));
        Ofs.write(reinterpret_cast<const char*>(&Winner), sizeof(core::Color));
        Ofs.write(reinterpret_cast<const char*>(&NumMoves), sizeof(uint8_t));
        for (std::size_t I = 0; I < NumSavedPlayouts; ++I) {
            Ofs.write(Moves[I].data(), 6 * sizeof(char));
        }
        Ofs.write(reinterpret_cast<const char*>(Visits.data()),
                  NumSavedPlayouts * sizeof(uint16_t));

        Ofs.write(reinterpret_cast<const char*>(&EndingRule),
                  sizeof(core::EndingRule));
        Ofs.write(reinterpret_cast<const char*>(&MaxPly), sizeof(uint16_t));
        Ofs.write(reinterpret_cast<const char*>(&BlackDrawValue),
                  sizeof(float));
        Ofs.write(reinterpret_cast<const char*>(&WhiteDrawValue),
                  sizeof(float));

        Ofs.write(reinterpret_cast<const char*>(&Declared), sizeof(bool));

        int16_t Dummy = 0xabc;
        Ofs.write(reinterpret_cast<const char*>(&Dummy), sizeof(int16_t));
    }

    void save(std::ofstream& Ofs) {
        save_0_1_0(Ofs);
    }

 private:
    static constexpr std::size_t SfenCStrLength = 132;
    static constexpr uint8_t NumSavedPlayouts = 16;
};

} // namespace

ml::AZTeacher load(std::ifstream& Ifs) {
    AZTeacherIO IO;
    return IO.load(Ifs);
}

void save(std::ofstream& Ofs, const ml::AZTeacher& AT) {
    AZTeacherIO IO(AT);
    IO.save(Ofs);
}

} // namespace az_teacher

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

template <>
ml::AZTeacher load<ml::AZTeacher>(std::ifstream& Ifs) {
    return az_teacher::load(Ifs);
}

template <>
void save<ml::AZTeacher>(std::ofstream& Ofs, const ml::AZTeacher& AT) {
    az_teacher::save(Ofs, AT);
}

template <>
ml::SimpleTeacher load<ml::SimpleTeacher>(std::ifstream& Ifs) {
    return simple_teacher::load(Ifs);
}

template <>
void save<ml::SimpleTeacher>(std::ofstream& Ofs, const ml::SimpleTeacher& ST) {
    simple_teacher::save(Ofs, ST);
}

} // namespace file
} // namespace io
} // namespace nshogi
