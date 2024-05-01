#include "bookmovemeta.h"

#include <stdexcept>
#include <cstring>

namespace nshogi {
namespace book {

BookMoveMeta::BookMoveMeta()
    : Count(0)
    , WinRate(0)
    , DrawRate(0) {

    for (std::size_t I = 0; I < 32; ++I) {
        PV[I] = core::Move32::MoveNone();
    }

    std::memset(Description, 0, 32 * sizeof(char));
}

BookMoveMeta::BookMoveMeta(const BookMoveMeta& BMM)
    : Count(BMM.Count)
    , WinRate(BMM.WinRate)
    , DrawRate(BMM.DrawRate) {
    std::memcpy(Description, BMM.Description, 32 * sizeof(char));
}

BookMoveMeta& BookMoveMeta::operator=(const BookMoveMeta& BMM) {
    Count = BMM.Count;
    WinRate = BMM.WinRate;
    DrawRate = BMM.DrawRate;
    std::memcpy(Description, BMM.Description, 32 * sizeof(char));

    return *this;
}

void BookMoveMeta::setCount(uint64_t Count_) {
    Count = Count_;
}

void BookMoveMeta::setWinRate(double WinRate_) {
    WinRate = WinRate_;
}

void BookMoveMeta::setDrawRate(double DrawRate_) {
    DrawRate = DrawRate_;
}

void BookMoveMeta::setPV(const core::Move32* PVMoves, uint8_t Depth) {
    for (std::size_t I = 0; I < 32; ++I) {
        PV[I] = core::Move32::MoveNone();
    }

    for (std::size_t I = 0; I < std::min((std::size_t)Depth, 32UL); ++I) {
        PV[I] = PVMoves[I];
    }
}

void BookMoveMeta::setDescription(const char* Description_) {
    if (std::strlen(Description_) > 64) {
        throw std::runtime_error("Description must be in 64 bytes.");
    }

    std::strcpy(Description, Description_);
}

void BookMoveMeta::incrementCount() {
    ++Count;
}

uint64_t BookMoveMeta::getCount() const {
    return Count;
}

double BookMoveMeta::getWinRate() const {
    return WinRate;
}

double BookMoveMeta::getDrawRate() const {
    return DrawRate;
}

const char* BookMoveMeta::getDescription() const {
    return Description;
}

void BookMoveMeta::dump(std::ofstream& Ofs) const {
    Ofs.write(reinterpret_cast<const char*>(&Count), sizeof(Count));
    Ofs.write(reinterpret_cast<const char*>(&WinRate), sizeof(WinRate));
    Ofs.write(reinterpret_cast<const char*>(&DrawRate), sizeof(DrawRate));
    Ofs.write(reinterpret_cast<const char*>(PV), 32 * sizeof(core::Move32));
    Ofs.write(Description, 64 * sizeof(char));
}

BookMoveMeta BookMoveMeta::load(std::ifstream& Ifs) {
    BookMoveMeta BMM;

    Ifs.read(reinterpret_cast<char*>(&BMM.Count), sizeof(Count));
    Ifs.read(reinterpret_cast<char*>(&BMM.WinRate), sizeof(WinRate));
    Ifs.read(reinterpret_cast<char*>(&BMM.DrawRate), sizeof(DrawRate));
    Ifs.read(reinterpret_cast<char*>(BMM.PV), 32  * sizeof(core::Move32));
    Ifs.read(reinterpret_cast<char*>(BMM.Description), 64  * sizeof(char));

    return BMM;
}

} // namespace book
} // namespace nshogi
