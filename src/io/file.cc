//
// Copyright (c) 2025-2026 @nyashiki
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

    static void loadAt_0_1_0(ml::AZTeacher* Dest, std::ifstream& Ifs) {
        Ifs.read(Dest->Sfen, SfenCStrLength * sizeof(char));

        Ifs.read(reinterpret_cast<char*>(&Dest->SideToMove), sizeof(core::Color));
        Ifs.read(reinterpret_cast<char*>(&Dest->Winner), sizeof(core::Color));
        Ifs.read(reinterpret_cast<char*>(&Dest->NumMoves), sizeof(uint8_t));
        for (std::size_t I = 0; I < NumSavedPlayouts; ++I) {
            Ifs.read(Dest->Moves[I].data(), 6 * sizeof(char));
        }
        Ifs.read(reinterpret_cast<char*>(Dest->Visits.data()),
                 NumSavedPlayouts * sizeof(uint16_t));

        Ifs.read(reinterpret_cast<char*>(&Dest->EndingRule),
                 sizeof(core::EndingRule));
        Ifs.read(reinterpret_cast<char*>(&Dest->MaxPly), sizeof(uint16_t));
        Ifs.read(reinterpret_cast<char*>(&Dest->BlackDrawValue), sizeof(float));
        Ifs.read(reinterpret_cast<char*>(&Dest->WhiteDrawValue), sizeof(float));

        Ifs.read(reinterpret_cast<char*>(&Dest->Declared), sizeof(bool));

        int16_t Dummy = 0;
        Ifs.read(reinterpret_cast<char*>(&Dummy), sizeof(int16_t));

        if (Dummy != 0xabc) {
            throw std::runtime_error("Parity incoincidence.");
        }
    }

    static std::size_t loadAt_0_1_0(ml::AZTeacher* Dest, const char* Source) {
        const char* Cur = Source;

        std::memcpy(Dest->Sfen, Cur, SfenCStrLength * sizeof(char));
        Cur += SfenCStrLength * sizeof(char);

        std::memcpy(&Dest->SideToMove, Cur, sizeof(core::Color));
        Cur += sizeof(core::Color);
        std::memcpy(&Dest->Winner, Cur, sizeof(core::Color));
        Cur += sizeof(core::Color);
        std::memcpy(&Dest->NumMoves, Cur, sizeof(uint8_t));
        Cur += sizeof(uint8_t);
        for (std::size_t I = 0; I < NumSavedPlayouts; ++I) {
            std::memcpy(Dest->Moves[I].data(), Cur, 6 * sizeof(char));
            Cur += 6 * sizeof(char);
        }
        std::memcpy(Dest->Visits.data(), Cur,
                    NumSavedPlayouts * sizeof(uint16_t));
        Cur += NumSavedPlayouts * sizeof(uint16_t);

        std::memcpy(&Dest->EndingRule, Cur, sizeof(core::EndingRule));
        Cur += sizeof(core::EndingRule);
        std::memcpy(&Dest->MaxPly, Cur, sizeof(uint16_t));
        Cur += sizeof(uint16_t);
        std::memcpy(&Dest->BlackDrawValue, Cur, sizeof(float));
        Cur += sizeof(float);
        std::memcpy(&Dest->WhiteDrawValue, Cur, sizeof(float));
        Cur += sizeof(float);

        std::memcpy(&Dest->Declared, Cur, sizeof(bool));
        Cur += sizeof(bool);

        int16_t Dummy;
        std::memcpy(&Dummy, Cur, sizeof(int16_t));
        Cur += sizeof(int16_t);

        if (Dummy != 0xabc) {
            throw std::runtime_error("Parity incoincidence.");
        }

        return static_cast<std::size_t>(Cur - Source);
    }

    ml::AZTeacher load_0_1_0(std::ifstream& Ifs) {
        loadAt_0_1_0(this, Ifs);
        return *this;
    }

    ml::AZTeacher load_0_1_0(const char* Source, std::size_t* BytesRead) {
        const std::size_t Read = loadAt_0_1_0(this, Source);

        if (BytesRead != nullptr) {
            *BytesRead = Read;
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

    ml::AZTeacher load(const char* Source, std::size_t* BytesRead) {
        char Version[8];
        std::memcpy(Version, Source, 8 * sizeof(char));

        if (std::strcmp(Version, "0.1.0") == 0) {
            return load_0_1_0(Source, BytesRead);
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

ml::AZTeacher load(const char* Source, std::size_t* BytesRead) {
    AZTeacherIO IO;
    return IO.load(Source, BytesRead);
}

void save(std::ofstream& Ofs, const ml::AZTeacher& AT) {
    AZTeacherIO IO(AT);
    IO.save(Ofs);
}

} // namespace az_teacher

namespace simple_teacher {

class SimpleTeacherIO : public ml::SimpleTeacher {
 public:
    SimpleTeacherIO() = default;

    SimpleTeacherIO(const ml::SimpleTeacher& ST)
        : ml::SimpleTeacher(ST) {
    }

    static void loadAt(ml::SimpleTeacher* Dest, std::ifstream& Ifs, int32_t Version) {
        uint64_t Codes[4];
        uint16_t Move16;

        Ifs.read(reinterpret_cast<char*>(Codes), 4 * sizeof(uint64_t));
        Ifs.read(reinterpret_cast<char*>(&Dest->Ply), sizeof(Ply));
        Ifs.read(reinterpret_cast<char*>(&Dest->MaxPly), sizeof(MaxPly));
        Ifs.read(reinterpret_cast<char*>(Dest->DrawValues), sizeof(DrawValues));
        Ifs.read(reinterpret_cast<char*>(&Move16), sizeof(uint16_t));
        Ifs.read(reinterpret_cast<char*>(&Dest->Winner), sizeof(Winner));

        Dest->HuffmanCode = core::HuffmanCode(Codes[3], Codes[2], Codes[1], Codes[0]);
        Dest->NextMove = core::Move16::fromValue(Move16);

        Dest->Q = 0.0f;
        Dest->GamePly = 0;

        if (Version >= 2) {
            Ifs.read(reinterpret_cast<char*>(&Dest->Q), sizeof(float));
            Ifs.read(reinterpret_cast<char*>(&Dest->GamePly), sizeof(uint16_t));
        }
    }

    static std::size_t loadAt(ml::SimpleTeacher* Dest, const char* Source, int32_t Version) {
        const char* Cur = Source;

        uint64_t Codes[4];
        uint16_t Move16;

        std::memcpy(Codes, Cur, 4 * sizeof(uint64_t));
        Cur += 4 * sizeof(uint64_t);
        std::memcpy(&Dest->Ply, Cur, sizeof(Ply));
        Cur += sizeof(Ply);
        std::memcpy(&Dest->MaxPly, Cur, sizeof(MaxPly));
        Cur += sizeof(MaxPly);
        std::memcpy(Dest->DrawValues, Cur, sizeof(DrawValues));
        Cur += sizeof(DrawValues);
        std::memcpy(&Move16, Cur, sizeof(uint16_t));
        Cur += sizeof(uint16_t);
        std::memcpy(&Dest->Winner, Cur, sizeof(Winner));
        Cur += sizeof(Winner);

        Dest->HuffmanCode = core::HuffmanCode(Codes[3], Codes[2], Codes[1], Codes[0]);
        Dest->NextMove = core::Move16::fromValue(Move16);

        Dest->Q = 0.0f;
        Dest->GamePly = 0;

        if (Version >= 2) {
            std::memcpy(&Dest->Q, Cur, sizeof(float));
            Cur += sizeof(float);
            std::memcpy(&Dest->GamePly, Cur, sizeof(uint16_t));
            Cur += sizeof(uint16_t);
        }

        return static_cast<std::size_t>(Cur - Source);
    }

    ml::SimpleTeacher load(std::ifstream& Ifs, int32_t Version) {
        loadAt(this, Ifs, Version);
        return *this;
    }

    ml::SimpleTeacher load(const char* Source, int32_t Version, std::size_t* BytesRead = nullptr) {
        const std::size_t Read = loadAt(this, Source, Version);

        if (BytesRead != nullptr) {
            *BytesRead = Read;
        }

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
        Ofs.write(reinterpret_cast<char*>(&Q), sizeof(float));
        Ofs.write(reinterpret_cast<char*>(&GamePly), sizeof(uint16_t));
    }
};

ml::SimpleTeacher load(std::ifstream& Ifs, int32_t Version) {
    SimpleTeacherIO IO;
    return IO.load(Ifs, Version);
}

ml::SimpleTeacher load(const char* Source, int32_t Version, std::size_t* BytesRead) {
    SimpleTeacherIO IO;
    return IO.load(Source, Version, BytesRead);
}

void save(std::ofstream& Ofs, const ml::SimpleTeacher& SimpleTeacher) {
    SimpleTeacherIO IO(SimpleTeacher);
    IO.save(Ofs);
}

void loadAt(ml::SimpleTeacher* Dest, std::ifstream& Ifs, int32_t Version) {
    SimpleTeacherIO::loadAt(Dest, Ifs, Version);
}

std::size_t loadAt(ml::SimpleTeacher* Dest, const char* Source, int32_t Version) {
    return SimpleTeacherIO::loadAt(Dest, Source, Version);
}

} // namespace simple_teacher

template <>
ml::AZTeacher load<ml::AZTeacher>(std::ifstream& Ifs, int32_t /* Version */) {
    return az_teacher::load(Ifs);
}

template <>
ml::SimpleTeacher load<ml::SimpleTeacher>(std::ifstream& Ifs, int32_t Version) {
    return simple_teacher::load(Ifs, Version);
}

template <>
ml::AZTeacher load<ml::AZTeacher>(const char* Source, int32_t /* Version */, std::size_t* BytesRead) {
    return az_teacher::load(Source, BytesRead);
}

template <>
ml::SimpleTeacher load<ml::SimpleTeacher>(const char* Source, int32_t Version, std::size_t* BytesRead) {
    return simple_teacher::load(Source, Version, BytesRead);
}

template <>
void save<ml::AZTeacher>(std::ofstream& Ofs, const ml::AZTeacher& AT) {
    az_teacher::save(Ofs, AT);
}

template <>
void save<ml::SimpleTeacher>(std::ofstream& Ofs, const ml::SimpleTeacher& ST) {
    simple_teacher::save(Ofs, ST);
}

} // namespace file
} // namespace io
} // namespace nshogi
