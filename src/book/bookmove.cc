#include "bookmove.h"

namespace nshogi {
namespace book {

BookMove::BookMove()
    : Move(core::Move32::MoveNone()) {
}

BookMove::BookMove(core::Move32 Move_)
    : Move(Move_) {
}

BookMove::BookMove(const BookMove& BM)
    : Move(BM.Move)
    , Meta(BM.Meta) {
}

BookMove& BookMove::operator=(const BookMove& BM) {
    Move = BM.getMove();
    Meta = BM.getMeta();

    return *this;
}

core::Move32 BookMove::getMove() const {
    return Move;
}

const BookMoveMeta& BookMove::getMeta() const {
    return Meta;
}

BookMoveMeta& BookMove::getMeta() {
    return Meta;
}

void BookMove::dump(std::ofstream& Ofs) const {
    Ofs.write(reinterpret_cast<const char*>(&Move), sizeof(Move));
    Meta.dump(Ofs);
}

BookMove BookMove::load(std::ifstream& Ifs) {
    BookMove BM;

    Ifs.read(reinterpret_cast<char*>(&BM.Move), sizeof(Move));
    BM.Meta = BookMoveMeta::load(Ifs);

    return BM;
}

} // namespace book
} // namespace nshogi
