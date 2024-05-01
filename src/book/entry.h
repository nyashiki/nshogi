#ifndef NSHOGI_BOOK_ENTRY_H
#define NSHOGI_BOOK_ENTRY_H

#include "bookmove.h"

#include <fstream>
#include <memory>
#include <vector>

namespace nshogi {
namespace book {


struct Entry {
 public:
    Entry(const char* Sfen);

    const char* getSfen() const;
    uint8_t getNumBookMoves() const;

    bool isRegistered(const core::Move32& Move) const;
    BookMove* getBookMove(std::size_t Index);
    BookMove* addBookMove(const core::Move32& Move);

    void clear();

    void dump(std::ofstream& Ofs) const;
    static Entry load(std::ifstream& Ifs);

 private:
    Entry();

    static constexpr uint8_t MaxNumBookMoves = 5;

    char Sfen[140];
    uint8_t NumBookMoves;
    std::vector<std::unique_ptr<BookMove>> BookMoves;
};

} // namespace book
} // namespace nshogi

#endif // #ifndef NSHOGI_BOOK_ENTRY_H
