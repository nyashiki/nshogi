#ifndef NSHOGI_CORE_HASH_H
#define NSHOGI_CORE_HASH_H

#include "types.h"
#include "position.h"
#include <cinttypes>


namespace nshogi {
namespace core {

template<typename HashValueType>
struct Hash {
 public:
    Hash();
    Hash(const Hash<HashValueType>&);

    Hash& operator=(const Hash& H) {
        Value = H.Value;
        return *this;
    }

    void clear();
    void refresh(const Position& Pos);

    template<Color C>
    void update(PieceTypeKind Type, Square Sq) {
        Value ^= OnBoardHash[C][Type][Sq];
    }

    inline void updateColor() {
        Value ^= ColorHash;
    }

    inline HashValueType getValue() const {
        return Value;
    }

 private:
    HashValueType Value;

    static bool IsTableGenerated;
    static HashValueType OnBoardHash[NumColors][NumPieceType][NumSquares];
    static HashValueType ColorHash;
};

template<typename HashValueType>
bool Hash<HashValueType>::IsTableGenerated = false;
template<typename HashValueType>
HashValueType Hash<HashValueType>::OnBoardHash[NumColors][NumPieceType][NumSquares];
template<typename HashValueType>
HashValueType Hash<HashValueType>::ColorHash;

} // namespace core
} // namespace nshogi


#endif // #ifndef NSHOGI_CORE_HASH_H
