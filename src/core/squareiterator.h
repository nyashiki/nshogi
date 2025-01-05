//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_CORE_SQUAREITERATOR_H
#define NSHOGI_CORE_SQUAREITERATOR_H

#include "position.h"
#include "types.h"
#include <cstdint>
#include <string>

namespace nshogi {
namespace core {

// clang-format off
enum struct IterateOrder {
    ESWN,  // from eastsouth to westnorth (Sq1I, Sq1H, ..., Sq1A, Sq2I, Sq2H, ..., Sq9A).
    WNES,  // from westnorth to eastsouth (Sq9A, Sq9B, ..., Sq9I, Sq8A, Sq8B, ..., Sq1I).
    NWSE,  // from northwest to southeast (Sq9A, Sq8A, ..., Sq1A, Sq9B, Sq8B, ..., Sq1I).
    SENW,  // from southeast to northwest (Sq1I, Sq2I, ..., Sq9I, Sq1H, Sq2H, ..., Sq9A).
    Fastest = ESWN,
};
// clang-format on

template<IterateOrder Order>
constexpr IterateOrder reverseOrder() {
    if constexpr (Order == IterateOrder::ESWN) {
        return IterateOrder::WNES;
    }

    if constexpr (Order == IterateOrder::WNES) {
        return IterateOrder::ESWN;
    }

    if constexpr (Order == IterateOrder::NWSE) {
        return IterateOrder::SENW;
    }

    if constexpr (Order == IterateOrder::SENW) {
        return IterateOrder::NWSE;
    }
}

template <IterateOrder Order> struct SquareIterator {
 public:
    SquareIterator() {
        if constexpr (Order == IterateOrder::Fastest ||
                      Order == IterateOrder::ESWN) {
            Sq = (Square)0;
        } else if constexpr (Order == IterateOrder::NWSE) {
            Sq = Sq9A;
            R = RankA;
            F = File9;
        } else if constexpr (Order == IterateOrder::SENW) {
            Sq = Sq1I;
            R = RankI;
            F = File1;
        } else if constexpr (Order == IterateOrder::WNES) {
            Sq = Sq9A;
        }
    }

    Square operator*() const {
        return Sq;
    }

    bool operator==(const SquareIterator<Order>& It) const {
        return Sq == It.Sq;
    }

    bool operator!=(const SquareIterator<Order>& It) const {
        return !((*this) == It);
    }

    SquareIterator<Order> begin() const {
        return SquareIterator<Order>();
    }

    SquareIterator<Order> end() const {
        SquareIterator<Order> It;
        It.Sq = NumSquares;

        return It;
    }

    SquareIterator<Order>& operator++() {
        if constexpr (Order == IterateOrder::Fastest ||
                      Order == IterateOrder::ESWN) {
            Sq = (Square)(Sq + 1);
            return *this;
        } else if constexpr (Order == IterateOrder::WNES) {
            if (Sq == Sq1I) {
                Sq = NumSquares;
                return *this;
            }

            Sq = (Square)(Sq - 1);
            return *this;
        } else if constexpr (Order == IterateOrder::NWSE) {
            if (F == File1) {
                if (R == RankI) {
                    Sq = NumSquares;
                    return *this;
                }

                R = (Rank)(R - 1);
                F = File9;
            } else {
                F = (File)(F - 1);
            }
        } else if constexpr (Order == IterateOrder::SENW) {
            if (F == File9) {
                if (R == RankA) {
                    Sq = NumSquares;
                    return *this;
                }

                R = (Rank)(R + 1);
                F = File1;
            } else {
                F = (File)(F + 1);
            }
        }

        Sq = makeSquare(R, F);
        return *this;
    }

    SquareIterator<Order>& operator+=(int Inc) {
        for (int I = 0; I < Inc; ++I) {
            ++(*this);
        }

        return *this;
    }

 private:
    Square Sq;

    Rank R;
    File F;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_SQUAREITERATOR_H
