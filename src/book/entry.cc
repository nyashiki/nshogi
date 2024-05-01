#include "entry.h"

#include <cassert>
#include <cstring>

namespace nshogi {
namespace book {

Entry::Entry()
    : NumBookMoves(0)
    , BookMoves(MaxNumBookMoves) {
}

Entry::Entry(const char* Sfen_)
    : NumBookMoves(0)
    , BookMoves(MaxNumBookMoves) {

    std::strcpy(Sfen, Sfen_);
}

const char* Entry::getSfen() const {
    return Sfen;
}

uint8_t Entry::getNumBookMoves() const {
    return NumBookMoves;
}

bool Entry::isRegistered(const core::Move32& Move) const {
    for (uint8_t I = 0; I < NumBookMoves; ++I) {
        if (BookMoves[(std::size_t)I]->getMove() == Move) {
            return true;
        }
    }

    return false;
}

BookMove* Entry::getBookMove(std::size_t Index) {
    assert(Index < NumBookMoves);
    return BookMoves[Index].get();
}

BookMove* Entry::addBookMove(const core::Move32& Move) {
    for (std::size_t I = 0; I < NumBookMoves; ++I) {
        if (BookMoves[I]->getMove() == Move) {
            // Reorder.
            if (I != 0) {
                std::swap(BookMoves[0], BookMoves[I]);
            }

            return BookMoves[0].get();
        }
    }

    if (NumBookMoves >= MaxNumBookMoves) {
        return nullptr;
    }

    BookMoves[(std::size_t)NumBookMoves] = std::make_unique<BookMove>(BookMove(Move));
    if (NumBookMoves != 0) {
        std::swap(BookMoves[0], BookMoves[(std::size_t)NumBookMoves]);
    }

    ++NumBookMoves;
    return BookMoves[0].get();
}

void Entry::clear() {
    NumBookMoves = 0;
}

void Entry::dump(std::ofstream& Ofs) const {
    Ofs.write(Sfen, 140 * sizeof(char));
    Ofs.write(reinterpret_cast<const char*>(&NumBookMoves), sizeof(NumBookMoves));

    for (std::size_t I = 0; I < MaxNumBookMoves; ++I) {
        if (I < NumBookMoves) {
            BookMoves[I]->dump(Ofs);
        } else {
            BookMove().dump(Ofs);
        }
    }
}

Entry Entry::load(std::ifstream& Ifs) {
    Entry E;

    Ifs.read(E.Sfen, 140 * sizeof(char));
    Ifs.read(reinterpret_cast<char*>(&E.NumBookMoves), sizeof(NumBookMoves));

    for (std::size_t I = 0; I < MaxNumBookMoves; ++I) {
        if (I < E.NumBookMoves) {
            E.BookMoves[I] = std::make_unique<BookMove>(BookMove::load(Ifs));
        } else {
            BookMove::load(Ifs);
        }
    }

    return E;
}

} // namespace book
} // namespace nshogi
