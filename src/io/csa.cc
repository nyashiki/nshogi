//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "csa.h"
#include "../core/internal/utils.h"
#include "../core/squareiterator.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace nshogi {
namespace io {
namespace csa {

char fileToChar(core::File File) {
    return '1' + (char)File;
}

char rankToChar(core::Rank Rank) {
    return '9' - (char)Rank;
}

core::Rank charToRank(char C) {
    if (C < '1' || C > '9') {
        throw std::runtime_error(std::string("invalid CSA rank ") + C + ".");
    }

    return (core::Rank)('9' - C);
}

core::File charToFile(char C) {
    if (C < '1' || C > '9') {
        throw std::runtime_error(std::string("invalid CSA file ") + C + ".");
    }

    return (core::File)(C - '1');
}

std::string pieceTypeToCSA(core::PieceTypeKind Type) {
    static const char* Table[15] = {
        " * ", "FU", "KY", "KE", "GI", "KA", "HI", "KI",
        "OU",  "TO", "NY", "NK", "NG", "UM", "RY",
    };

    if ((uint8_t)Type >= 15) {
        throw std::runtime_error(std::string("invalid piece type ") +
                                 std::to_string(Type) + ".");
    }

    return std::string(Table[Type]);
}

std::string pieceToCSA(core::PieceKind Piece) {
    if (Piece == core::PK_Empty) {
        return " * ";
    }

    if (core::getColor(Piece) == core::Black) {
        return "+" + pieceTypeToCSA(core::getPieceType(Piece));
    } else {
        return "-" + pieceTypeToCSA(core::getPieceType(Piece));
    }
}

core::PieceTypeKind CSAToPieceType(const std::string& CSA) noexcept {
    using namespace nshogi::core;

    static const std::map<std::string, PieceTypeKind> Table({
        {"FU", PTK_Pawn},
        {"KY", PTK_Lance},
        {"KE", PTK_Knight},
        {"GI", PTK_Silver},
        {"KA", PTK_Bishop},
        {"HI", PTK_Rook},
        {"KI", PTK_Gold},
        {"OU", PTK_King},
        {"TO", PTK_ProPawn},
        {"NY", PTK_ProLance},
        {"NK", PTK_ProKnight},
        {"NG", PTK_ProSilver},
        {"UM", PTK_ProBishop},
        {"RY", PTK_ProRook},
    });

    const auto& It = Table.find(CSA.substr(0, 2));

    if (It == Table.end()) {
        return PTK_Empty;
    }

    return It->second;
}

core::PieceKind CSAToPiece(const std::string& CSA) noexcept {
    core::PieceTypeKind Type = CSAToPieceType(CSA.substr(1));

    if (Type == core::PTK_Empty) {
        return core::PK_Empty;
    }

    if (CSA[0] == '+') {
        return core::makePiece(core::Black, Type);
    } else if (CSA[0] == '-') {
        return core::makePiece(core::White, Type);
    } else {
        return core::PK_Empty;
    }
}

std::string squareToCSA(core::Square Sq) {
    const char CSASq[3] = {
        fileToChar(core::squareToFile(Sq)),
        rankToChar(core::squareToRank(Sq)),
    };

    return std::string(CSASq);
}

std::string move32ToCSA(core::Move32 Move, core::Color Color) {
    if (Move.isNone()) {
        return "%TORYO";
    }

    if (Move == core::Move32::MoveWin()) {
        return "%KACHI";
    }

    std::stringstream SStream;

    SStream << ((Color == core::Black) ? "+" : "-");

    if (Move.drop()) {
        SStream << "00" << squareToCSA(Move.to());
        SStream << pieceTypeToCSA(Move.pieceType());
    } else {
        SStream << squareToCSA(Move.from());
        SStream << squareToCSA(Move.to());

        if (Move.promote()) {
            SStream << pieceTypeToCSA(core::promotePieceType(Move.pieceType()));
        } else {
            SStream << pieceTypeToCSA(Move.pieceType());
        }
    }

    return SStream.str();
}

core::Move32 CSAToMove32(const core::Position& Pos, const std::string& CSA) {
    if (CSA.size() < 7) {
        throw std::runtime_error("invalid CSA string (size check error).\n" +
                                 ("    CSA: " + CSA));
    }

    core::File ToFile = charToFile(CSA[3]);
    core::Rank ToRank = charToRank(CSA[4]);
    core::Square ToSq = core::makeSquare(ToRank, ToFile);

    const core::PieceTypeKind Type = CSAToPieceType(CSA.substr(5));

    if (pieceTypeToCSA(Type) != CSA.substr(5, 2)) {
        throw std::runtime_error("invalid CSA string (type check error).\n" +
                                 ("    CSA: " + CSA));
    }

    if (CSA[1] == '0' && CSA[2] == '0') {
        return core::Move32::droppingMove(ToSq, Type);
    }

    core::File FromFile = charToFile(CSA[1]);
    core::Rank FromRank = charToRank(CSA[2]);
    core::Square FromSq = core::makeSquare(FromRank, FromFile);
    core::PieceTypeKind FromType = core::getPieceType(Pos.pieceOn(FromSq));
    core::PieceTypeKind CaptureType = core::getPieceType(Pos.pieceOn(ToSq));

    if (FromType != Type) {
        return core::Move32::boardPromotingMove(FromSq, ToSq, FromType,
                                                CaptureType);
    } else {
        return core::Move32::boardMove(FromSq, ToSq, FromType, CaptureType);
    }
}

std::string positionToCSA(const core::Position& Pos) {
    using namespace core;

    std::stringstream SStream;

    SquareIterator<IterateOrder::NWSE> SquareIt;

    for (auto It = SquareIt.begin(); It != SquareIt.end(); ++It) {
        const Square Sq = *It;

        if (squareToFile(Sq) == File9) {
            SStream << "P" << (int)(9 - squareToRank(Sq));
        }

        SStream << pieceToCSA(Pos.pieceOn(Sq));

        if (squareToFile(Sq) == File1) {
            SStream << "\n";
        }
    }

    Stands BlackStands = Pos.getStand<Black>();

    if (BlackStands != 0) {
        SStream << "P+";
        for (PieceTypeKind Type : StandPieceTypes) {
            for (uint8_t I = 0; I < Pos.getStandCount<Black>(Type); ++I) {
                SStream << "00" << pieceTypeToCSA(Type);
            }
        }
        SStream << "\n";
    }

    Stands WhiteStands = Pos.getStand<White>();
    if (WhiteStands != 0) {
        SStream << "P-";
        for (PieceTypeKind Type : StandPieceTypes) {
            for (uint8_t I = 0; I < Pos.getStandCount<White>(Type); ++I) {
                SStream << "00" << pieceTypeToCSA(Type);
            }
        }
        SStream << "\n";
    }

    SStream << ((Pos.sideToMove() == Black) ? "+" : "-") << "\n";

    return SStream.str();
}

std::string stateToCSA(const core::State& State) {
    std::stringstream SStream;

    SStream << positionToCSA(State.getInitialPosition());
    core::Color Color = State.getInitialPosition().sideToMove();

    if (State.getPly() > 0) {
        for (uint16_t Ply = 0; Ply < State.getPly(); ++Ply) {
            SStream << move32ToCSA(State.getHistoryMove(Ply), Color) << "\n";
            Color = ~Color;
        }
    }

    return SStream.str();
}

PositionBuilder::PositionBuilder(const core::Position& Position)
    : core::PositionBuilder(Position) {
}

core::Position PositionBuilder::newPosition(const std::string& CSA) {
    using namespace core;

    PositionBuilder Builder;

    std::vector<std::string> CSASplitted = internal::utils::split(CSA, '\n');

    int InputRowCount = 0;

    for (const auto& CSALine : CSASplitted) {
        if (CSALine == "+") {
            Builder.setColor(core::Black);
            continue;
        } else if (CSALine == "-") {
            Builder.setColor(core::White);
            continue;
        }

        const std::string Token = CSALine.substr(0, 2);

        core::File F = core::File9;
        core::Rank R = core::RankA;

        if (Token == "PI") {
            Builder = PositionBuilder(core::PositionBuilder::getInitialPosition());
            for (std::size_t I = 2; I < CSALine.size(); I += 4) {
                File DropF = charToFile(CSALine[I]);
                Rank DropR = charToRank(CSALine[I + 1]);
                Builder.setPiece(makeSquare(DropR, DropF), PK_Empty);
            }
            InputRowCount = 9;
            continue;
        } else if (Token == "P1") {
            R = RankA;
            ++InputRowCount;
        } else if (Token == "P2") {
            R = RankB;
            ++InputRowCount;
        } else if (Token == "P3") {
            R = RankC;
            ++InputRowCount;
        } else if (Token == "P4") {
            R = RankD;
            ++InputRowCount;
        } else if (Token == "P5") {
            R = RankE;
            ++InputRowCount;
        } else if (Token == "P6") {
            R = RankF;
            ++InputRowCount;
        } else if (Token == "P7") {
            R = RankG;
            ++InputRowCount;
        } else if (Token == "P8") {
            R = RankH;
            ++InputRowCount;
        } else if (Token == "P9") {
            R = RankI;
            ++InputRowCount;
        } else if (Token == "P+") {
            std::size_t Cursor = 2;
            uint32_t StandCounts[16] = {};

            while (Cursor + 4 <= CSALine.size()) {
                core::PieceTypeKind Type =
                    CSAToPieceType(CSALine.substr(Cursor + 2, 2));
                if (CSALine.substr(Cursor, 2) == "00") {
                    StandCounts[Type]++;
                } else {
                    F = charToFile(CSALine[Cursor]);
                    R = charToRank(CSALine[Cursor + 1]);
                    Builder.setPiece(makeSquare(R, F), makePiece(Black, Type));
                }
                Cursor += 4;
            }

            for (const auto Type : StandPieceTypes) {
                Builder.setStand(Black, Type, StandCounts[Type]);
            }
        } else if (Token == "P-") {
            std::size_t Cursor = 2;
            uint32_t StandCounts[16] = {};

            while (Cursor + 4 <= CSALine.size()) {
                core::PieceTypeKind Type =
                    CSAToPieceType(CSALine.substr(Cursor + 2, 2));
                if (CSALine.substr(Cursor, 2) == "00") {
                    StandCounts[Type]++;
                } else {
                    F = charToFile(CSALine[Cursor]);
                    R = charToRank(CSALine[Cursor + 1]);
                    Builder.setPiece(makeSquare(R, F), makePiece(White, Type));
                }
                Cursor += 4;
            }

            for (const auto Type : StandPieceTypes) {
                Builder.setStand(White, Type, StandCounts[Type]);
            }
        } else {
            continue;
        }

        std::size_t Cursor = 2;

        while (Cursor + 3 <= CSALine.size()) {
            const std::string CSAPiece = CSALine.substr(Cursor, 3);
            core::Square Sq = makeSquare(R, F);
            core::PieceKind Piece = CSAToPiece(CSAPiece);

            Builder.setPiece(Sq, Piece);

            Cursor += 3;
            F = (File)(F - 1);
        }
    }

    if (InputRowCount != 9) {
        throw std::runtime_error("Invalid CSA string.");
    }

    return Builder.build();
}

StateBuilder::StateBuilder(const std::string& CSA)
    : core::StateBuilder(PositionBuilder::newPosition(CSA)) {
    using namespace core;

    std::vector<std::string> CSASplitted = internal::utils::split(CSA, '\n');
    bool MoveWinDetected = false;

    for (const auto& CSALine : CSASplitted) {
        auto CSAMove = internal::utils::split(CSALine, ',')[0];
        if (CSAMove.size() == 7) {
            if (CSALine[0] == '+' || CSALine[0] == '-') {
                if (MoveWinDetected) {
                    throw std::runtime_error("move after declaration");
                }

                core::Move32 Move =
                    CSAToMove32(Instance.getPosition(), CSALine);
                Instance.doMove(Move);
            }
        } else {
            if (CSALine == "%KACHI") {
                MoveWinDetected = true;
            }
        }
    }
}

core::State StateBuilder::newState(const std::string& CSA) {
    StateBuilder Builder(CSA);
    return Builder.build();
}

} // namespace csa
} // namespace io
} // namespace nshogi
