//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "sfen.h"
#include "../core/squareiterator.h"
#include "../core/types.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace nshogi {
namespace io {
namespace sfen {

char fileToChar(core::File File) {
    return (char)('1' + File);
}

char rankToChar(core::Rank Rank) {
    return (char)('i' - Rank);
}

core::Rank charToRank(char C) {
    if (C != 'a' && C != 'b' && C != 'c' && C != 'd' && C != 'e' && C != 'f' &&
        C != 'g' && C != 'h' && C != 'i') {
        throw std::runtime_error(std::string("invalid sfen rank ") + C + ".");
    }

    return (core::Rank)('i' - C);
}

core::File charToFile(char C) {
    if (C < '1' || '9' < C) {
        throw std::runtime_error(std::string("invalid sfen file.") + C + ".");
    }

    return (core::File)(C - '1');
}

std::string pieceToSfen(core::PieceKind Piece) {
    // "$" represents error, i.e., `Piece` is not given correctly.
    static const char* Table[31] = {
        "$",  "P",  "L",  "N",  "S",  "B",  "R",  "G",  "K",  "+P", "+L",
        "+N", "+S", "+B", "+R", "$",  "$",  "p",  "l",  "n",  "s",  "b",
        "r",  "g",  "k",  "+p", "+l", "+n", "+s", "+b", "+r",
    };

    return std::string(Table[Piece]);
}

std::string move16ToSfen(core::Move16 Move) {
    if (Move == core::Move16::MoveNone()) {
        return "resign";
    }

    if (Move == core::Move16::MoveWin()) {
        return "win";
    }

    std::stringstream SStream;

    const core::Square To = Move.to();

    if (Move.drop()) {
        const core::PieceTypeKind Type =
            (core::PieceTypeKind)(Move.from() - core::NumSquares + 1);
        ;

        SStream << pieceToSfen(core::makePiece(core::Black, Type)) << '*';
        SStream << squareToSfen(To);
    } else {
        const core::Square From = Move.from();

        SStream << squareToSfen(From) << squareToSfen(To);
        if (Move.promote()) {
            SStream << '+';
        }
    }

    return SStream.str();
}

std::string move32ToSfen(core::Move32 Move) {
    return move16ToSfen(core::Move16(Move));
}

core::Move32 sfenToMove32(const core::Position& Pos, const std::string& Sfen) {
    if (Sfen == "resign") {
        return core::Move32::MoveNone();
    }

    if (Sfen == "win") {
        return core::Move32::MoveWin();
    }

    if (Sfen.size() < 4) {
        throw std::runtime_error("invalid sfen string (length check error).\n" +
                                 ("    Position:" + positionToSfen(Pos) +
                                  "\n"
                                  "    Sfen: " +
                                  Sfen));
    }

    core::File ToFile = charToFile(Sfen[2]);
    core::Rank ToRank = charToRank(Sfen[3]);
    core::Square ToSq = core::makeSquare(ToRank, ToFile);

    if (Sfen[1] == '*') {
        if (Sfen.size() != 4) {
            throw std::runtime_error(
                "invalid sfen string (dropping check error).\n" +
                ("    Position:" + positionToSfen(Pos) +
                 "\n"
                 "    Sfen: " +
                 Sfen));
        }

        const core::PieceKind Piece = charToPiece(Sfen[0], false);
        const core::PieceTypeKind Type = core::getPieceType(Piece);

        if (pieceToSfen(Piece)[0] != Sfen[0]) {
            throw std::runtime_error(
                "invalid sfen string (piece check error).\n" +
                ("    Position:" + positionToSfen(Pos) +
                 "\n"
                 "    Sfen: " +
                 Sfen));
        }

        return core::Move32::droppingMove(ToSq, Type);
    }

    if (Sfen.size() != 4 && Sfen.size() != 5) {
        throw std::runtime_error("invalid sfen string (size check error).\n" +
                                 ("    Position:" + positionToSfen(Pos) +
                                  "\n"
                                  "    Sfen: " +
                                  Sfen));
    }

    if (Sfen.size() == 5 && Sfen[4] != '+') {
        throw std::runtime_error(
            "invalid sfen string (promotion check error).\n" +
            ("    Position:" + positionToSfen(Pos) +
             "\n"
             "    Sfen: " +
             Sfen));
    }

    core::File FromFile = charToFile(Sfen[0]);
    core::Rank FromRank = charToRank(Sfen[1]);
    core::Square FromSq = makeSquare(FromRank, FromFile);

    core::PieceTypeKind Type = core::getPieceType(Pos.pieceOn(FromSq));
    core::PieceTypeKind CaptureType = core::getPieceType(Pos.pieceOn(ToSq));

    if (Type == core::PTK_Empty) {
        throw std::runtime_error("invalid sfen string (type check error).\n" +
                                 ("    Position:" + positionToSfen(Pos) +
                                  "\n"
                                  "    Sfen: " +
                                  Sfen));
    }

    bool IsPromoting = Sfen.size() == 5 && Sfen[4] == '+';

    if (IsPromoting) {
        return core::Move32::boardPromotingMove(FromSq, ToSq, Type,
                                                CaptureType);
    } else {
        return core::Move32::boardMove(FromSq, ToSq, Type, CaptureType);
    }
}

core::PieceKind charToPiece(char C, bool Promote) noexcept {
    using namespace core;

    static const std::map<char, PieceKind> Table({
        {'P', PK_BlackPawn},
        {'p', PK_WhitePawn},
        {'L', PK_BlackLance},
        {'l', PK_WhiteLance},
        {'N', PK_BlackKnight},
        {'n', PK_WhiteKnight},
        {'S', PK_BlackSilver},
        {'s', PK_WhiteSilver},
        {'B', PK_BlackBishop},
        {'b', PK_WhiteBishop},
        {'R', PK_BlackRook},
        {'r', PK_WhiteRook},
        {'G', PK_BlackGold},
        {'g', PK_WhiteGold},
        {'K', PK_BlackKing},
        {'k', PK_WhiteKing},
    });

    const auto& It = Table.find(C);

    if (It == Table.end()) {
        return PK_Empty;
    }

    if (Promote) {
        PieceTypeKind Type = getPieceType(It->second);

        if (Type == PTK_Gold || Type == PTK_King) {
            // These peaces cannot be promoted.
            return PK_Empty;
        }

        return promotePiece(It->second);
    }

    return It->second;
}

std::string squareToSfen(core::Square Sq) {
    const char SfenSq[3] = {
        fileToChar(core::squareToFile(Sq)),
        rankToChar(core::squareToRank(Sq)),
    };

    return std::string(SfenSq);
}

std::string positionToSfen(const core::Position& Pos, uint16_t Ply) {
    using namespace core;

    std::stringstream SStream;

    SquareIterator<IterateOrder::NWSE> SquareIt;

    int SequentialEmptyCount = 0;
    for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
        const Square Sq = *It;

        if (Pos.pieceOn(Sq) == PK_Empty) {
            SequentialEmptyCount++;
        } else {
            if (SequentialEmptyCount > 0) {
                SStream << SequentialEmptyCount;
                SequentialEmptyCount = 0;
            }

            SStream << pieceToSfen(Pos.pieceOn(Sq));
        }

        if (squareToFile(Sq) == File1) {
            if (SequentialEmptyCount > 0) {
                SStream << SequentialEmptyCount;
                SequentialEmptyCount = 0;
            }

            if (squareToRank(Sq) != RankI) {
                SStream << '/';
            }
        }
    }

    SStream << ((Pos.sideToMove() == Black) ? " b " : " w ");

    bool StandExists = false;

    for (Color C : Colors) {
        for (PieceTypeKind Pt : StandPieceTypes) {
            const uint8_t StandCount = getStandCount(Pos.getStand(C), Pt);

            if (StandCount > 0) {
                StandExists = true;
                if (StandCount > 1) {
                    SStream << (int)StandCount;
                }
                SStream << pieceToSfen(makePiece(C, Pt));
            }
        }
    }

    if (!StandExists) {
        SStream << "-";
    }

    SStream << " " << Pos.getPlyOffset() + Ply + 1;

    return SStream.str();
}

std::string stateToSfen(const core::State& State) {
    std::stringstream SStream;

    SStream << positionToSfen(State.getInitialPosition());

    if (State.getPly() > State.getPosition().getPlyOffset()) {
        SStream << " moves";

        for (uint16_t Ply = State.getPosition().getPlyOffset();
             Ply < State.getPly(); ++Ply) {
            SStream << " " << move32ToSfen(State.getHistoryMove(Ply));
        }
    }

    return SStream.str();
}

core::Position PositionBuilder::newPosition(const std::string& Sfen) {
    using namespace core;

    if (Sfen.substr(0, 8) == "startpos") {
        return core::PositionBuilder::getInitialPosition();
    }

    PositionBuilder Builder;
    SquareIterator<IterateOrder::NWSE> SquareIt;
    bool Promote = false;
    std::size_t Cursor = 0;

    for (; Sfen[Cursor] != ' '; ++Cursor) {
        if (Cursor >= Sfen.size()) {
            throw std::runtime_error("invalid sfen string (cursor error).\n" +
                                     ("    Sfen: " + Sfen));
        }

        if (Sfen[Cursor] == '/') {
            continue;
        }

        if (Sfen[Cursor] == '+') {
            Promote = true;
            continue;
        }

        if (Sfen[Cursor] >= '1' && Sfen[Cursor] <= '9') {
            if (Promote) {
                throw std::runtime_error(
                    "invalid sfen string (empty count error).\n" +
                    ("    Sfen: " + Sfen));
            }

            SquareIt += Sfen[Cursor] - '0';
            continue;
        }

        PieceKind Piece = charToPiece(Sfen[Cursor], Promote);

        if (Piece == PK_Empty) {
            throw std::runtime_error(
                "invalid sfen string (piece empty error).\n" +
                ("    Sfen: " + Sfen));
        }

        Builder.setPiece(*SquareIt, Piece);
        ++SquareIt;
        Promote = false;
    }

    if (*SquareIt != NumSquares) {
        throw std::runtime_error("invalid sfen string (square error).\n" +
                                 ("    Sfen: " + Sfen));
    }

    ++Cursor;

    if (Sfen[Cursor] == 'b') {
        Builder.setColor(Black);
    } else if (Sfen[Cursor] == 'w') {
        Builder.setColor(White);
    } else {
        throw std::runtime_error("invalid sfen string (color error).\n" +
                                 ("    Sfen: " + Sfen));
    }

    ++Cursor;

    if (Sfen[Cursor] != ' ') {
        throw std::runtime_error(
            "invalid sfen string (empty charactor check error).\n" +
            ("    Sfen: " + Sfen));
    }

    ++Cursor;

    uint8_t StandCount = 0;
    for (; Sfen[Cursor] != ' '; ++Cursor) {
        if (Cursor >= Sfen.size()) {
            throw std::runtime_error(
                "invalid sfen string (stand cursor error).\n" +
                ("    Sfen: " + Sfen));
        }

        if (Sfen[Cursor] == '-') {
            continue;
        }

        if (Sfen[Cursor] >= '0' && Sfen[Cursor] <= '9') {
            StandCount = (uint8_t)(10 * StandCount + (Sfen[Cursor] - '0'));
            continue;
        }

        PieceKind Piece = charToPiece(Sfen[Cursor], false);
        PieceTypeKind Type = getPieceType(Piece);

        if (Type == PTK_Empty || Type == PTK_King) {
            throw std::runtime_error(
                "invalid sfen string (empty or king check error).\n" +
                ("    Sfen: " + Sfen));
        }

        if (StandCount == 0) {
            StandCount = 1;
        }

        if (getColor(Piece) == Black) {
            Builder.setStand(Black, Type, StandCount);
        } else {
            Builder.setStand(White, Type, StandCount);
        }

        StandCount = 0;
    }

    if (StandCount > 0) {
        throw std::runtime_error("invalid sfen string (stand count error).\n" +
                                 ("    Sfen: " + Sfen));
    }

    ++Cursor;

    if (Cursor >= Sfen.size()) {
        throw std::runtime_error("invalid sfen string (ply cursor error).\n" +
                                 ("    Sfen: " + Sfen));
    }

    uint16_t Ply = 0;

    for (; Sfen[Cursor] != ' '; ++Cursor) {
        if (Cursor == Sfen.size()) {
            break;
        }

        if (Sfen[Cursor] < '0' || Sfen[Cursor] > '9') {
            throw std::runtime_error(
                "invalid sfen string (ply not a number error).\n" +
                ("    Sfen: " + Sfen));
        }

        Ply = (uint16_t)(Ply * 10) + (uint16_t)(Sfen[Cursor] - '0');
    }

    if (Ply == 0) {
        throw std::runtime_error("invalid sfen string (ply zero error).\n" +
                                 ("    Sfen: " + Sfen));
    }

    Builder.setPlyOffset(Ply - 1);

    return Builder.build();
}

StateBuilder::StateBuilder(const std::string& Sfen)
    : core::StateBuilder(PositionBuilder::newPosition(Sfen)) {

    std::size_t Cursor = 0;

    if (Sfen.substr(0, 8) == "startpos") {
        Cursor = 9;
    } else {
        int WhiteSpaceCount = 0;
        while (WhiteSpaceCount < 4) {
            if (Cursor == Sfen.size()) {
                return;
            }

            if (Sfen[Cursor++] == ' ') {
                ++WhiteSpaceCount;
            }
        }
    }

    if (Cursor >= Sfen.size()) {
        return;
    }

    if (Sfen.size() < Cursor + 5) {
        const auto ExpectMovesStr = Sfen.substr(Cursor, +5);

        if (ExpectMovesStr != "moves") {
            throw std::runtime_error("invalid sfen string (moves error)\n" +
                                     ("    Sfen: " + Sfen));
        }
    }

    Cursor += 6;

    if (Cursor >= Sfen.size()) {
        return;
    }

    std::string MoveSfen = "";
    bool MoveWinDetected = false;
    while (Cursor < Sfen.size()) {
        char Scan = Sfen[Cursor];

        if (Scan != ' ') {
            MoveSfen += Scan;
        }

        ++Cursor;
        if (Scan == ' ' || Cursor == Sfen.size()) {
            if (MoveWinDetected) {
                throw std::runtime_error("move after declaration");
            }

            core::Move32 Move = sfenToMove32(Instance.getPosition(), MoveSfen);

            if (!Move.isWin()) {
                Instance.doMove(Move);
            } else {
                MoveWinDetected = true;
            }

            MoveSfen = "";
        }
    }
}

core::State StateBuilder::newState(const std::string& Sfen) {
    StateBuilder Builder(Sfen);
    return Builder.build();
}

} // namespace sfen
} // namespace io
} // namespace nshogi
