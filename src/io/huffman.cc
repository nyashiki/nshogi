#include "huffman.h"
#include <cstdio>

namespace nshogi {
namespace io {
namespace huffman {

void print(const core::HuffmanCode& HuffmanCode) {
    uint64_t Codes[4];
    std::memcpy(static_cast<void*>(Codes), HuffmanCode.data(), sizeof(HuffmanCode));
    std::printf("(0x%lx, 0x%lx, 0x%lx, 0x%lx)\n", Codes[0], Codes[1], Codes[2], Codes[3]);
}

} // namespace huffman
} // namespace io
} // namespace nshogi
