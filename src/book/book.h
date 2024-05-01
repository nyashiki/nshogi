#ifndef NSHOGI_BOOK_BOOK_H
#define NSHOGI_BOOK_BOOK_H

#include "entry.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace nshogi {
namespace book {

class Book {
 public:
    Book();
    Book(const Book&) = delete;
    Book(Book&&) noexcept;
    Book& operator=(const Book&) = delete;
    Book& operator=(Book&&) = delete;

    static Book loadYaneuraOuFormat(const char* Path, double PonanzaConstant = 600, const char* Description = nullptr);
    static Book makeBookFromSfens(const char* Path, uint16_t PlyLimit, uint16_t PlyCutoff, const char* Description = nullptr);
    void patchFromSfens(const char* Path, uint16_t PlyLimit, uint16_t PlyCutoff, const char* Description = nullptr);

    Entry* addEntry(const char* Sfen);
    Entry* findEntry(const char* Sfen);

    std::size_t size() const;

    void dump(const char* Path) const;
    static Book load(const char* Path);

 private:
    void makeBookFromSfensInternal(const char* Path, uint16_t PlyLimit, uint16_t PlyCutOff, const char* Description = nullptr);
    void removePositionsInSfens(const char* Path);

    std::unordered_map<std::string, std::unique_ptr<Entry>> Entries;
};

} // namespace book
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_BOOK_BOOK_H
