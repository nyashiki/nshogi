#include "huffman.h"

#include "internal/huffmanimpl.h"
#include <cassert>

namespace nshogi {
namespace core {

HuffmanCode::HuffmanCode(const HuffmanCode& HC)
    : Impl(new internal::HuffmanCodeImpl(*HC.Impl)) {
}

HuffmanCode::HuffmanCode(uint64_t Code3, uint64_t Code2, uint64_t Code1, uint64_t Code0)
    : Impl(new internal::HuffmanCodeImpl(Code3, Code2, Code1, Code0)) {
}

HuffmanCode::~HuffmanCode() {
    assert(Impl != nullptr);
    delete Impl;
}

HuffmanCode HuffmanCode::operator=(const HuffmanCode& HC) {
    delete Impl;
    Impl = new internal::HuffmanCodeImpl(*HC.Impl);
    return *this;
}

bool HuffmanCode::operator==(const HuffmanCode& HC) const {
    return *Impl == *HC.Impl;
}

bool HuffmanCode::operator!=(const HuffmanCode& HC) const {
    return *Impl != *HC.Impl;
}

std::size_t HuffmanCode::size() {
    return internal::HuffmanCodeImpl::size();
}

const char* HuffmanCode::data() const {
    return Impl->data();
}

HuffmanCode HuffmanCode::zero() {
    return HuffmanCode(new internal::HuffmanCodeImpl(internal::HuffmanCodeImpl::zero()));
}

HuffmanCode HuffmanCode::encode(const Position& Pos) {
    return HuffmanCode(new internal::HuffmanCodeImpl(internal::HuffmanCodeImpl::encode(Pos)));
}

HuffmanCode HuffmanCode::encode(const Position& Pos, Square BlackKingSquare, Square WhiteKingSquare) {
    return HuffmanCode(
            new internal::HuffmanCodeImpl(
                internal::HuffmanCodeImpl::encode(
                    Pos,
                    BlackKingSquare,
                    WhiteKingSquare)));
}

Position HuffmanCode::decode(const HuffmanCode& HC) {
    return internal::HuffmanCodeImpl::decode(*HC.Impl);
}

} // namespace core
} // namespace nshogi
