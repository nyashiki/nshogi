#include "featurebitboard.h"

#include "../core/internal/bitboard.h"

namespace nshogi {
namespace ml {

template<core::IterateOrder Order>
void FeatureBitboard::extract(float* Dest) const {
    const core::internal::bitboard::Bitboard Bitboard =
        *reinterpret_cast<const core::internal::bitboard::Bitboard*>(C);

    if (C[11] == 0) {  // no rotation.
        core::SquareIterator<Order> SquareIt;

        int Counter = 0;
        for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
            Dest[Counter] = (Bitboard.isSet(*It))? F[3] : 0;
            ++Counter;
        }
    } else {
        core::SquareIterator<core::reverseOrder<Order>()> SquareIt;

        int Counter = 0;
        for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
            Dest[Counter] = (Bitboard.isSet(*It))? F[3] : 0;
            ++Counter;
        }
    }
}

template
void FeatureBitboard::extract<core::IterateOrder::ESWN>(float* Dest) const;
template
void FeatureBitboard::extract<core::IterateOrder::NWSE>(float* Dest) const;
template
void FeatureBitboard::extract<core::IterateOrder::SENW>(float* Dest) const;
template
void FeatureBitboard::extract<core::IterateOrder::WNES>(float* Dest) const;

} // namespace ml
} // namespace nshogi
