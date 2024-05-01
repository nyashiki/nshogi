#ifndef NSHOGI_CORE_BITITERATOR_H
#define NSHOGI_CORE_BITITERATOR_H

#include "bitboard.h"
#include "types.h"
#include <cassert>
#include <utility>

namespace nshogi {
namespace core {

class BitIterator {
 public:
    BitIterator(const bitboard::Bitboard& Bitboard)
        : BB(Bitboard), PopCount(Bitboard.popCount()), Counter(0) {
    }

    BitIterator(const BitIterator&) = delete;
    BitIterator& operator=(const BitIterator&) = delete;

    BitIterator(const BitIterator&&) = delete;
    BitIterator& operator=(const BitIterator&&) = delete;

    Square next() {
        // The implementation code might be slightly faster than
        // ```
        //     ++Counter;
        //     return BB.popOne();
        // ```
        // because of CPU pipeline efficiency.
        Square Sq = BB.popOne();
        ++Counter;
        return Sq;
    }

    bool end() const {
        // assert(Counter != PopCount || BB.isZero());
        return Counter == PopCount;
    }

 private:
    bitboard::Bitboard BB;
    const int PopCount;
    int Counter;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_BITITERATOR_H
