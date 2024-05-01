#ifndef NSHOGI_BOOK_BOOKMOVEMETA_H
#define NSHOGI_BOOK_BOOKMOVEMETA_H

#include "../core/types.h"

#include <fstream>
#include <cinttypes>

namespace nshogi {
namespace book {

struct BookMoveMeta {
 public:
    BookMoveMeta();
    BookMoveMeta(const BookMoveMeta&);
    BookMoveMeta& operator=(const BookMoveMeta&);

    void setCount(uint64_t Count_);
    void setWinRate(double WinRate_);
    void setDrawRate(double DrawRate_);
    void setPV(const core::Move32* PVMoves, uint8_t Depth);
    void setDescription(const char* Description_);
    void incrementCount();

    uint64_t getCount() const;
    double getWinRate() const;
    double getDrawRate() const;
    const char* getDescription() const;

    void dump(std::ofstream& Ofs) const;
    static BookMoveMeta load(std::ifstream& Ifs);

 private:
    uint64_t Count;
    double WinRate;
    double DrawRate;

    core::Move32 PV[32];
    char Description[64];
};

} // namespace book
} // namespace nshogi

#endif // #ifndef NSHOGI_BOOK_BOOKMOVEMETA_H
