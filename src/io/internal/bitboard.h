#ifndef NSHOGI_IO_INTERNAL_BITBOARD_H
#define NSHOGI_IO_INTERNAL_BITBOARD_H

#include "../../core/squareiterator.h"
#include "../../core/internal/bitboard.h"

namespace nshogi {
namespace io {
namespace internal {
namespace bitboard {

void print(const core::internal::bitboard::Bitboard& BB) {
    auto SquareIt = core::SquareIterator<core::IterateOrder::NWSE>();

    for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
        std::cout << (BB.isSet(*It) ? " 1 " : " 0 ");

        if (core::squareToFile(*It) == core::File1) {
            std::cout << "\n";
        }
    }
}

} // namespace bitboard
} // namespace internal
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_INTERNAL_BITBOARD_H
