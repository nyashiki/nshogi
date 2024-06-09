#include "statehelper.h"
#include "bitboard.h"
#include "hash.h"
#include "state.h"
#include "types.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace nshogi {
namespace core {

StateHelper::StateHelper(const Position& Pos)
    : InitialPosition(Pos)
    , Ply(0) {
    SHelper.reserve(DefaultReserveSize);
    SHelper.emplace_back();
    assert(SHelper.size() == 1);
}

StateHelper::StateHelper(const Position& Pos, uint16_t PlyOffset)
    : InitialPosition(Pos, PlyOffset)
    , Ply(0) {
    SHelper.reserve(DefaultReserveSize);
    SHelper.emplace_back();
    assert(SHelper.size() == 1);
}

StateHelper::StateHelper(StateHelper&& Helper) noexcept
    : InitialPosition(Helper.InitialPosition), Ply(Helper.Ply),
      SHelper(Helper.SHelper) {

    std::memcpy(static_cast<void*>(ColorBB),
                static_cast<const void*>(Helper.ColorBB),
                NumColors * sizeof(bitboard::Bitboard));

    std::memcpy(static_cast<void*>(TypeBB),
                static_cast<const void*>(Helper.TypeBB),
                NumPieceType * sizeof(bitboard::Bitboard));

    std::memcpy(static_cast<void*>(KingSquare),
                static_cast<const void*>(Helper.KingSquare),
                NumColors * sizeof(Square));
}

StateHelper::~StateHelper() {
}

void StateHelper::proceedOneStep(const Move32& Move, uint64_t BoardHash, Stands BlackStand, Stands WhiteStand) {
    SHelper.emplace_back();

    assert(Ply < SHelper.size());

    // Record the move and increment Ply.
    SHelper[Ply].Move = Move;
    SHelper[Ply].BoardHash = BoardHash;
    SHelper[Ply].EachStand[Black] = BlackStand;
    SHelper[Ply].EachStand[White] = WhiteStand;

    Ply++;
}

Move32 StateHelper::goBackOneStep() {
    assert(Ply > 0);

    SHelper.pop_back();

    --Ply;
    const Move32 PrevMove = SHelper[Ply].Move;

    return PrevMove;
}

} // namespace core
} // namespace nshogi
