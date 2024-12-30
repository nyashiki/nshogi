#ifndef NSHOGI_CORE_MOVELIST_H
#define NSHOGI_CORE_MOVELIST_H

#include "types.h"
#include <cstddef>
#include <initializer_list>

namespace nshogi {
namespace core {

namespace internal {

class MoveGeneratorInternal;

} // namespace internal

struct MoveList {
 public:
    MoveList(MoveList&&) noexcept = default;

    MoveList(const MoveList&) = delete;
    MoveList& operator=(const MoveList&) = delete;
    MoveList& operator=(MoveList&&) = delete;

    MoveList(std::initializer_list<Move32> Ms) {
        Move32* Head = Moves;

        for(const auto& M : Ms) {
            *Head++ = M;
        }

        Tail = Head;
    }

    inline const Move32* begin() const {
        return Moves;
    }

    inline const Move32* end() const {
        return Tail;
    }

    inline Move32 operator[](std::size_t Index) const {
        return Moves[Index];
    }

    [[maybe_unused]] inline const Move32* find(Move32 Move) const {
        for (const auto& EachMove : *this) {
            if (EachMove == Move) {
                return &EachMove;
            }
        }

        return end();
    }

    inline std::size_t size() const {
        return (std::size_t)(Tail - Moves);
    }

 private:
    MoveList() : Tail(Moves) {
    }

    static constexpr std::size_t MoveCountMax = 600;

    Move32 Moves[MoveCountMax];
    Move32* Tail;

    friend class nshogi::core::internal::MoveGeneratorInternal;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_MOVELIST_H
