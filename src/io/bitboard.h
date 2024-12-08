#ifndef NSHOGI_IO_BITBOARD_H
#define NSHOGI_IO_BITBOARD_H

#include "../core/internal/bitboard.h"

namespace nshogi {
namespace io {
namespace bitboard {

void print(const core::internal::bitboard::Bitboard& BB);

} // namespace bitboard
} // namespace io
} // namespace nshogi

#endif // #ifndef NSHOGI_IO_BITBOARD_H
