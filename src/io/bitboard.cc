#include "bitboard.h"
#include "../core/squareiterator.h"

#include <iostream>

namespace nshogi {
namespace io {
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
} // namespace io
} // namespace nshogi
