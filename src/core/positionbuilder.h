#ifndef NSHOGI_CORE_POSITIONBUILDER_H
#define NSHOGI_CORE_POSITIONBUILDER_H

#include "position.h"
#include "squareiterator.h"
#include <cstdint>
#include <utility>

namespace nshogi {
namespace core {

class PositionBuilder {
 public:
    static Position getInitialPosition();

    Position build() {
        return std::move(Instance);
    }

 protected:
    PositionBuilder() = default;

    void setColor(Color C) {
        Instance.SideToMove = C;
    }

    void setPlyOffset(uint16_t Offset) {
        Instance.PlyOffset = Offset;
    }

    template <IterateOrder Order>
    void setPieces(std::initializer_list<PieceKind> Ps) {
        auto SquareIt = SquareIterator<Order>().begin();

        for (auto It = Ps.begin(); It != Ps.end(); ++It) {
            Square Sq = *SquareIt;
            Instance.OnBoard[Sq] = *It;

            ++SquareIt;
        }
    }

    void setPiece(Square Sq, PieceKind Piece) {
        Instance.OnBoard[Sq] = Piece;
    }

    void setStand(Color C, PieceTypeKind Pt, uint32_t Count) {
        // The stand representation of `Stands` is kind of
        // tricky. To set the number of a standing piece `Pt`,
        // increment/decrement it one by one since this is
        // not a speed-intended part.
        while (getStandCount(Instance.getStand(C), Pt) < Count) {
            Instance.incrementStand(C, Pt);
        }

        while (getStandCount(Instance.getStand(C), Pt) > Count) {
            Instance.decrementStand(C, Pt);
        }
    }

    void clearStands() {
        Instance.EachStands[Black] = (Stands)0;
        Instance.EachStands[White] = (Stands)0;
    }

    void incrementStand(Color C, PieceTypeKind Pt) {
        Instance.incrementStand(C, Pt);
    }

 private:
    Position Instance;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_POSITIONBUILDER_H
