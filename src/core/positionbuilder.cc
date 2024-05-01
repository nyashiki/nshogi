#include "positionbuilder.h"
#include "squareiterator.h"
#include "types.h"

namespace nshogi {
namespace core {

Position PositionBuilder::getInitialPosition() {
    PositionBuilder Builder;

    Builder.setColor(Black);
    Builder.setPlyOffset(0);

    // clang-format off
    Builder.setPieces<IterateOrder::NWSE>({
        PK_WhiteLance, PK_WhiteKnight, PK_WhiteSilver, PK_WhiteGold, PK_WhiteKing, PK_WhiteGold, PK_WhiteSilver, PK_WhiteKnight, PK_WhiteLance,
        PK_Empty,      PK_WhiteRook,   PK_Empty,       PK_Empty,     PK_Empty,     PK_Empty,     PK_Empty,       PK_WhiteBishop, PK_Empty,
        PK_WhitePawn,  PK_WhitePawn,   PK_WhitePawn,   PK_WhitePawn, PK_WhitePawn, PK_WhitePawn, PK_WhitePawn,   PK_WhitePawn,   PK_WhitePawn,
        PK_Empty,      PK_Empty,       PK_Empty,       PK_Empty,     PK_Empty,     PK_Empty,     PK_Empty,       PK_Empty,       PK_Empty,
        PK_Empty,      PK_Empty,       PK_Empty,       PK_Empty,     PK_Empty,     PK_Empty,     PK_Empty,       PK_Empty,       PK_Empty,
        PK_Empty,      PK_Empty,       PK_Empty,       PK_Empty,     PK_Empty,     PK_Empty,     PK_Empty,       PK_Empty,       PK_Empty,
        PK_BlackPawn,  PK_BlackPawn,   PK_BlackPawn,   PK_BlackPawn, PK_BlackPawn, PK_BlackPawn, PK_BlackPawn,   PK_BlackPawn,   PK_BlackPawn,
        PK_Empty,      PK_BlackBishop, PK_Empty,       PK_Empty,     PK_Empty,     PK_Empty,     PK_Empty,       PK_BlackRook,   PK_Empty,
        PK_BlackLance, PK_BlackKnight, PK_BlackSilver, PK_BlackGold, PK_BlackKing, PK_BlackGold, PK_BlackSilver, PK_BlackKnight, PK_BlackLance,
    });
    // clang-format on

    for (PieceTypeKind Type : StandPieceTypes) {
        Builder.setStand(Black, Type, 0);
        Builder.setStand(White, Type, 0);
    }

    return Builder.build();
}

} // namespace core
} // namespace nshogi
