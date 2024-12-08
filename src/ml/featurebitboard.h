#ifndef NSHOGI_ML_FEATUREBITBOARD_H
#define NSHOGI_ML_FEATUREBITBOARD_H

#include "../core/internal/bitboard.h"
#include "../core/squareiterator.h"

#include <cstring>

namespace nshogi {
namespace ml {

struct FeatureBitboard {
 public:
    constexpr FeatureBitboard(): Bitboard(0, 0) {
    }

    FeatureBitboard(const FeatureBitboard& FB) {
        std::memcpy(reinterpret_cast<void*>(this), &FB, sizeof(*this));
    }

    FeatureBitboard& operator=(const FeatureBitboard& FB) {
        std::memcpy(reinterpret_cast<void*>(this), &FB, sizeof(*this));
        return *this;
    }

    FeatureBitboard(const core::internal::bitboard::Bitboard& BB, float Value, bool Rotation): Bitboard(BB) {
        F[3] = Value;
        C[11] = Rotation;
    }

    core::internal::bitboard::Bitboard getBitboard() const {
        return core::internal::bitboard::Bitboard(Bitboard & core::internal::bitboard::Bitboard::AllBB());
    }

    float getValue() const {
        return F[3];
    }

    bool isRotated() const {
        return C[11];
    }

    const uint64_t* data() const {
        return reinterpret_cast<const uint64_t*>(&Bitboard);
    }

    template<core::IterateOrder Order>
    [[maybe_unused]]
    void extract(float* Dest) const {
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

 private:
    union {
        core::internal::bitboard::Bitboard Bitboard;
        float F[4];     // use F[3] for the value of a neural network channel.
        uint8_t C[16];  // use C[11] for the 180 degree rotation of the board.
    };
};


} // namespace ml
} // namespace nshogi


#endif // #ifndef NSHOGI
       // _ML_FEATUREBITBOARD_H
