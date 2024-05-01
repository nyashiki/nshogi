#ifndef NSHOGI_CORE_POSITION_H
#define NSHOGI_CORE_POSITION_H

#include "types.h"
#include <cstdint>

namespace nshogi {
namespace core {

struct Position {
 public:
    Position();
    Position(const Position&);

    ~Position() = default;

    constexpr Color sideToMove() const {
        return SideToMove;
    }

    constexpr PieceKind pieceOn(Square Sq) const {
        return OnBoard[Sq];
    }

    template <Color C> constexpr Stands getStand() const {
        return EachStands[C];
    }

    constexpr Stands getStand(Color C) const {
        return EachStands[C];
    }

    constexpr uint16_t getPlyOffset() const {
        return PlyOffset;
    }

    template <Color C, PieceTypeKind Type>
    constexpr uint8_t getStandCount() const {
        return core::getStandCount<Type>(getStand<C>());
    }

    template <Color C>
    uint8_t getStandCount(PieceTypeKind Type) const {
        return core::getStandCount(getStand<C>(), Type);
    }

    uint8_t getStandCount(Color C, PieceTypeKind Type) const {
        return core::getStandCount(getStand(C), Type);
    }

    bool equals(const Position& Pos) const;

 private:
    void changeSideToMove() {
        SideToMove = ~SideToMove;
    }

    void putPiece(Square Sq, PieceKind Piece) {
        OnBoard[Sq] = Piece;
    }

    PieceKind removePiece(Square Sq) {
        assert(OnBoard[Sq] != PK_Empty);

        PieceKind Piece = OnBoard[Sq];
        OnBoard[Sq] = PK_Empty;
        return Piece;
    }

    void incrementStand(Color C, PieceTypeKind Type) {
        EachStands[C] = nshogi::core::incrementStand(EachStands[C], Type);
    }

    void decrementStand(Color C, PieceTypeKind Type) {
        EachStands[C] = nshogi::core::decrementStand(EachStands[C], Type);
    }

    Color SideToMove;
    uint16_t PlyOffset;
    PieceKind OnBoard[NumSquares];
    Stands EachStands[NumColors];

    friend class State;
    friend class PositionBuilder;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_POSITION_H
