//
// Copyright (c) 2025-2026 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "azteacher.h"
#include "../core/movegenerator.h"
#include "../io/sfen.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <random>
#include <string>

namespace nshogi {
namespace ml {

AZTeacher::AZTeacher()
    : V(0.0f)
    , Q(0.0f)
    , GamePly(0) {
}

AZTeacher::AZTeacher(const AZTeacher& T) {
    std::memcpy(Sfen, T.Sfen, SfenCStrLength * sizeof(char));

    SideToMove = T.SideToMove;
    Winner = T.Winner;
    NumMoves = T.NumMoves;

    std::copy(T.Moves.begin(), T.Moves.end(), Moves.data());

    std::memcpy(reinterpret_cast<char*>(Visits.data()),
                reinterpret_cast<const char*>(T.Visits.data()),
                NumSavedPlayouts * sizeof(uint16_t));

    EndingRule = T.EndingRule;
    MaxPly = T.MaxPly;
    BlackDrawValue = T.BlackDrawValue;
    WhiteDrawValue = T.WhiteDrawValue;

    Declared = T.Declared;

    V = T.V;
    Q = T.Q;
    GamePly = T.GamePly;
}

AZTeacher& AZTeacher::operator=(const AZTeacher& T) {
    if (this != &T) {
        std::memcpy(Sfen, T.Sfen, SfenCStrLength * sizeof(char));

        SideToMove = T.SideToMove;
        Winner = T.Winner;
        NumMoves = T.NumMoves;

        std::copy(T.Moves.begin(), T.Moves.end(), Moves.data());

        std::memcpy(reinterpret_cast<char*>(Visits.data()),
                    reinterpret_cast<const char*>(T.Visits.data()),
                    NumSavedPlayouts * sizeof(uint16_t));

        EndingRule = T.EndingRule;
        MaxPly = T.MaxPly;
        BlackDrawValue = T.BlackDrawValue;
        WhiteDrawValue = T.WhiteDrawValue;

        Declared = T.Declared;

        V = T.V;
        Q = T.Q;
        GamePly = T.GamePly;
    }
    return *this;
}

void AZTeacher::corruptMyself() {
    static std::mt19937_64 Mt(20230711);

    for (std::size_t I = 0; I < SfenCStrLength; ++I) {
        Sfen[I] = (char)Mt();
    }

    SideToMove = (core::Color)Mt();
    Winner = (core::Color)Mt();
    NumMoves = (uint8_t)Mt();

    char Buffer[1024];
    for (std::size_t I = 0; I < 1024; ++I) {
        Buffer[I] = (char)Mt();
    }

    for (std::size_t I = 0; I < NumSavedPlayouts; ++I) {
        std::memcpy(Moves[I].data(), &Buffer[I], 6 * sizeof(char));
    }

    std::memcpy(reinterpret_cast<char*>(Visits.data()), Buffer,
                NumSavedPlayouts * sizeof(uint16_t));

    EndingRule = (core::EndingRule)Mt();
    MaxPly = (uint16_t)Mt();

    std::memcpy(reinterpret_cast<char*>(&BlackDrawValue), &Buffer[512],
                sizeof(float));
    std::memcpy(reinterpret_cast<char*>(&WhiteDrawValue), &Buffer[544],
                sizeof(float));

    Declared = (bool)Mt();

    std::memcpy(reinterpret_cast<char*>(&V), &Buffer[576], sizeof(float));
    std::memcpy(reinterpret_cast<char*>(&Q), &Buffer[608], sizeof(float));
    GamePly = (uint16_t)Mt();
}

bool AZTeacher::equals(const AZTeacher& T) const {
    if (std::memcmp(Sfen, T.Sfen, SfenCStrLength) != 0) {
        return false;
    }

    if (SideToMove != T.SideToMove) {
        return false;
    }

    if (Winner != T.Winner) {
        return false;
    }

    if (NumMoves != T.NumMoves) {
        return false;
    }

    if (!std::equal(Moves.begin(), Moves.end(), T.Moves.begin())) {
        return false;
    }

    if (!std::equal(Visits.begin(), Visits.end(), T.Visits.begin())) {
        return false;
    }

    if (EndingRule != T.EndingRule) {
        return false;
    }

    if (MaxPly != T.MaxPly) {
        return false;
    }

    if (std::memcmp(reinterpret_cast<const char*>(&BlackDrawValue),
                    reinterpret_cast<const char*>(&T.BlackDrawValue),
                    sizeof(float)) != 0) {
        return false;
    }

    if (std::memcmp(reinterpret_cast<const char*>(&WhiteDrawValue),
                    reinterpret_cast<const char*>(&T.WhiteDrawValue),
                    sizeof(float)) != 0) {
        return false;
    }

    if (Declared != T.Declared) {
        return false;
    }

    if (std::memcmp(reinterpret_cast<const char*>(&V),
                    reinterpret_cast<const char*>(&T.V), sizeof(float)) != 0) {
        return false;
    }

    if (std::memcmp(reinterpret_cast<const char*>(&Q),
                    reinterpret_cast<const char*>(&T.Q), sizeof(float)) != 0) {
        return false;
    }

    if (GamePly != T.GamePly) {
        return false;
    }

    return true;
}

bool AZTeacher::checkSanity(int Level) const {
    if (Level >= 0) {
        // Check constraiants.
        if ((SideToMove != core::Black) && (SideToMove != core::White)) {
            return false;
        }

        if ((Winner != core::Black) && (Winner != core::White) &&
            (Winner != core::NoColor)) {
            return false;
        }

        if (NumMoves > NumSavedPlayouts) {
            return false;
        }

        if (std::memchr(Sfen, '\0', SfenCStrLength) == nullptr) {
            return false;
        }

        if (BlackDrawValue < 0 || BlackDrawValue > 1) {
            return false;
        }

        if (WhiteDrawValue < 0 || WhiteDrawValue > 1) {
            return false;
        }

        if (V < 0 || V > 1) {
            return false;
        }

        if (Q < 0 || Q > 1) {
            return false;
        }
    }

    if (Level >= 1) {
        // Check if all SumVisits is greater than 0.
        if (NumMoves == 0) {
            return false;
        }

        uint16_t SumVisits = 0;
        for (uint8_t I = 0; I < NumMoves; ++I) {
            SumVisits += Visits[I];
            if (SumVisits > 0) {
                break;
            }
        }

        if (SumVisits == 0) {
            return false;
        }
    }

    if (Level >= 2) {
        try {
            const auto State =
                nshogi::io::sfen::StateBuilder::newState(std::string(Sfen));
            // Check if SideToMove is identical to that of Sfen.
            if (State.getSideToMove() != SideToMove) {
                return false;
            }

            // Check if all moves is legal.
            const auto LegalMoves =
                nshogi::core::MoveGenerator::generateLegalMoves(State);

            for (uint8_t I = 0; I < NumMoves; ++I) {
                if (std::memchr(Moves[I].data(), '\0', Moves[I].size()) ==
                    nullptr) {
                    return false;
                }

                const auto Move = nshogi::io::sfen::sfenToMove32(
                    State.getPosition(), std::string(Moves[I].data()));

                if (LegalMoves.find(Move) == LegalMoves.end()) {
                    return false;
                }
            }
        } catch (const std::exception&) {
            // An unparsable Sfen or move string is insane data,
            // not an error of this function.
            return false;
        }
    }

    return true;
}

} // namespace ml
} // namespace nshogi
