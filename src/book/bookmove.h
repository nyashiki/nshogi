#ifndef NSHOGI_BOOK_BOOKMOVE_H
#define NSHOGI_BOOK_BOOKMOVE_H

#include "bookmovemeta.h"
#include "../core/types.h"

#include <fstream>

namespace nshogi {
namespace book {


struct BookMove {
 public:
    BookMove();
    BookMove(core::Move32 Move_);
    BookMove(const BookMove&);
    BookMove& operator=(const BookMove&);

    core::Move32 getMove() const;
    const BookMoveMeta& getMeta() const;
    BookMoveMeta& getMeta();

    void dump(std::ofstream& Ofs) const;
    static BookMove load(std::ifstream& Ifs);

 private:
    core::Move32 Move;
    BookMoveMeta Meta;
};


} // namespace book
} // namespace nshogi

#endif // #ifndef NSHOGI_BOOK_BOOKMOVE_H
