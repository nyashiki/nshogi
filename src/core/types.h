#ifndef NSHOGI_CORE_TYPES_H
#define NSHOGI_CORE_TYPES_H

#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <array>

#if defined(USE_BMI1)

#include <immintrin.h>

#endif

// clang-format off

namespace nshogi {
namespace core {

enum Color : uint8_t {
    Black = 0, White = 1,
    NoColor = 2,
    NumColors = 2,
};

constexpr Color Colors[NumColors] = {Black, White};

inline constexpr Color operator~(Color C) {
    return (Color)(C ^ White);
}

enum EndingRule : uint8_t {
    NoRule_ER    = 0b00000000,
    Declare27_ER = 0b00000001,
    Draw24_ER    = 0b00000010,
    Trying_ER    = 0b00000100,
};

enum PieceTypeKind : uint8_t {
    PTK_Empty   = 0,
    PTK_Pawn    = 1,     PTK_Lance = 2,     PTK_Knight = 3,    PTK_Silver =  4,     PTK_Bishop = 5,     PTK_Rook = 6, PTK_Gold = 7, PTK_King = 8,
    PTK_ProPawn = 9, PTK_ProLance = 10, PTK_ProKnight = 11, PTK_ProSilver = 12, PTK_ProBishop = 13, PTK_ProRook = 14,

    NumPieceType = 15,
};

static constexpr PieceTypeKind PieceTypes[] = {
    PTK_Pawn, PTK_Lance, PTK_Knight, PTK_Silver, PTK_Bishop, PTK_Rook, PTK_Gold, PTK_King,
    PTK_ProPawn, PTK_ProLance, PTK_ProKnight, PTK_ProSilver, PTK_ProBishop, PTK_ProRook,
};

inline constexpr PieceTypeKind promotePieceType(PieceTypeKind Pt) {
    return (PieceTypeKind)(Pt ^ 0b1000);
}

inline constexpr PieceTypeKind demotePieceType(PieceTypeKind Pt) {
    return (PieceTypeKind)(Pt & 0b111);
}

enum PieceKind : uint16_t {
    PK_Empty = 0,
    PK_BlackPawn    =  1, PK_BlackLance    =  2, PK_BlackKnight    =  3, PK_BlackSilver    =  4, PK_BlackBishop    =  5, PK_BlackRook    =  6, PK_BlackGold =  7, PK_BlackKing =  8,
    PK_BlackProPawn =  9, PK_BlackProLance = 10, PK_BlackProKnight = 11, PK_BlackProSilver = 12, PK_BlackProBishop = 13, PK_BlackProRook = 14,
    PK_WhitePawn    = 17, PK_WhiteLance    = 18, PK_WhiteKnight    = 19, PK_WhiteSilver    = 20, PK_WhiteBishop    = 21, PK_WhiteRook    = 22, PK_WhiteGold = 23, PK_WhiteKing = 24,
    PK_WhiteProPawn = 25, PK_WhiteProLance = 26, PK_WhiteProKnight = 27, PK_WhiteProSilver = 28, PK_WhiteProBishop = 29, PK_WhiteProRook = 30,
};

inline constexpr PieceTypeKind getPieceType(PieceKind P) {
    return (PieceTypeKind)((uint8_t)P & 0b1111);
}

inline constexpr Color getColor(PieceKind P) {
    return (Color)((0b10000 & P) >> 4);
}

inline constexpr PieceKind promotePiece(PieceKind P) {
    return (PieceKind)(0b1000 | P);
}

inline constexpr bool isPromoted(PieceTypeKind Type) {
    return (Type != PTK_King) && ((Type & 0b1000) != 0);
};

template<Color C>
inline constexpr PieceKind makePiece(PieceTypeKind Pt);

template<> inline constexpr PieceKind makePiece<Black>(PieceTypeKind Pt) {
    return (PieceKind)(Pt);
}

template<> inline constexpr PieceKind makePiece<White>(PieceTypeKind Pt) {
    return (PieceKind)(0b10000 | Pt);
}

inline PieceKind makePiece(Color C, PieceTypeKind Pt) {
    if (C == Color::Black) {
        return makePiece<Color::Black>(Pt);
    }

    return makePiece<Color::White>(Pt);
}

enum Square : int8_t {  // Use signed integer to handle arithmetic operations.
    Sq9A = 80, Sq8A = 71, Sq7A = 62, Sq6A = 53, Sq5A = 44, Sq4A = 35, Sq3A = 26, Sq2A = 17, Sq1A = 8,
    Sq9B = 79, Sq8B = 70, Sq7B = 61, Sq6B = 52, Sq5B = 43, Sq4B = 34, Sq3B = 25, Sq2B = 16, Sq1B = 7,
    Sq9C = 78, Sq8C = 69, Sq7C = 60, Sq6C = 51, Sq5C = 42, Sq4C = 33, Sq3C = 24, Sq2C = 15, Sq1C = 6,
    Sq9D = 77, Sq8D = 68, Sq7D = 59, Sq6D = 50, Sq5D = 41, Sq4D = 32, Sq3D = 23, Sq2D = 14, Sq1D = 5,
    Sq9E = 76, Sq8E = 67, Sq7E = 58, Sq6E = 49, Sq5E = 40, Sq4E = 31, Sq3E = 22, Sq2E = 13, Sq1E = 4,
    Sq9F = 75, Sq8F = 66, Sq7F = 57, Sq6F = 48, Sq5F = 39, Sq4F = 30, Sq3F = 21, Sq2F = 12, Sq1F = 3,
    Sq9G = 74, Sq8G = 65, Sq7G = 56, Sq6G = 47, Sq5G = 38, Sq4G = 29, Sq3G = 20, Sq2G = 11, Sq1G = 2,
    Sq9H = 73, Sq8H = 64, Sq7H = 55, Sq6H = 46, Sq5H = 37, Sq4H = 28, Sq3H = 19, Sq2H = 10, Sq1H = 1,
    Sq9I = 72, Sq8I = 63, Sq7I = 54, Sq6I = 45, Sq5I = 36, Sq4I = 27, Sq3I = 18, Sq2I =  9, Sq1I = 0,

    NumSquares = 81,
    SqInvalid = 82
};

static constexpr Square Squares[81] = {
    Sq1I, Sq1H, Sq1G, Sq1F, Sq1E, Sq1D, Sq1C, Sq1B, Sq1A,
    Sq2I, Sq2H, Sq2G, Sq2F, Sq2E, Sq2D, Sq2C, Sq2B, Sq2A,
    Sq3I, Sq3H, Sq3G, Sq3F, Sq3E, Sq3D, Sq3C, Sq3B, Sq3A,
    Sq4I, Sq4H, Sq4G, Sq4F, Sq4E, Sq4D, Sq4C, Sq4B, Sq4A,
    Sq5I, Sq5H, Sq5G, Sq5F, Sq5E, Sq5D, Sq5C, Sq5B, Sq5A,
    Sq6I, Sq6H, Sq6G, Sq6F, Sq6E, Sq6D, Sq6C, Sq6B, Sq6A,
    Sq7I, Sq7H, Sq7G, Sq7F, Sq7E, Sq7D, Sq7C, Sq7B, Sq7A,
    Sq8I, Sq8H, Sq8G, Sq8F, Sq8E, Sq8D, Sq8C, Sq8B, Sq8A,
    Sq9I, Sq9H, Sq9G, Sq9F, Sq9E, Sq9D, Sq9C, Sq9B, Sq9A,
};

inline Square& operator++(Square& Sq) {
    return Sq = (Square)((int)Sq + 1);
}

inline constexpr Square getInversed(Square Sq) {
    return (Square)((int16_t)NumSquares - 1 - (int16_t)Sq);
}

inline constexpr bool checkRange(Square Sq) {
    return (Sq >= Square::Sq1I) && (Sq <= Square::Sq9A);
}

enum Direction : int16_t {
    NorthNorthWest = 11,             NorthNorthEast =  -7,
    NorthWest =      10, North =  1, NorthEast =       -8,
    West =            9,             East =            -9,
    SouthWest =       8, South = -1, SouthEast =      -10,
    SouthSouthWest =  7,             SouthSouthEast = -11,
};

static constexpr Direction Directions[] = {
    SouthSouthEast, SouthEast, East, NorthEast, NorthNorthEast,
    South, North,
    SouthSouthWest, SouthWest, West, NorthWest, NorthNorthWest
};

inline constexpr Square operator+(Square Sq, Direction Dir) {
    return (Square)((int16_t)Sq + (int16_t)Dir);
}

enum File : uint8_t {
    File1, File2, File3, File4, File5, File6, File7, File8, File9,

    NumFiles = 9,
};

enum Rank : uint8_t {
    RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,

    NumRanks = 9,
};

inline constexpr bool checkRange(File F) {
    return F >= File1 && F <= File9;
}

inline constexpr bool checkRange(Rank R) {
    return R >= RankI && R <= RankA;
}

static constexpr File Files[NumFiles] = {
    File1, File2, File3, File4, File5, File6, File7, File8, File9,
};

static constexpr Rank Ranks[NumRanks] = {
    RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
};

inline constexpr Square makeSquare(Rank R, File F) {
    assert(checkRange(R) && checkRange(F));

    return (Square)((uint8_t)R + 9 * (uint8_t)F);
}

inline constexpr File squareToFile(Square Sq) {
    constexpr File Table[NumSquares] = {
        File1, File1, File1, File1, File1, File1, File1, File1, File1,
        File2, File2, File2, File2, File2, File2, File2, File2, File2,
        File3, File3, File3, File3, File3, File3, File3, File3, File3,
        File4, File4, File4, File4, File4, File4, File4, File4, File4,
        File5, File5, File5, File5, File5, File5, File5, File5, File5,
        File6, File6, File6, File6, File6, File6, File6, File6, File6,
        File7, File7, File7, File7, File7, File7, File7, File7, File7,
        File8, File8, File8, File8, File8, File8, File8, File8, File8,
        File9, File9, File9, File9, File9, File9, File9, File9, File9,
    };

    return Table[Sq];
};

inline constexpr Rank squareToRank(Square Sq) {
    constexpr Rank Table[NumSquares] = {
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
        RankI, RankH, RankG, RankF, RankE, RankD, RankC, RankB, RankA,
    };

    return Table[Sq];
};

namespace {

/// Relative direction from `Sq1` to `Sq2`.
static constexpr auto DirectionDataInternal = []() ->
    std::pair<std::array<std::array<Direction, NumSquares>, NumSquares>,
              std::array<std::array<uint8_t, NumSquares>, NumSquares>> {
    std::array<std::array<Direction, NumSquares>, NumSquares> Directions = { };
    std::array<std::array<uint8_t, NumSquares>, NumSquares> SerializedDirections  = { };

    for (Square Sq1 : Squares) {
        for (Square Sq2 : Squares) {
            if (Sq1 == Sq2) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = (Direction)0;
            }

            const Rank R1 = squareToRank(Sq1);
            const Rank R2 = squareToRank(Sq2);
            const File F1 = squareToFile(Sq1);
            const File F2 = squareToFile(Sq2);
            const bool Comp = (int)Sq1 > (int)Sq2;

            if (R1 == R2) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? East : West;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? 2 : 6;
            } else if (F1 == F2) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? South : North;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? 4 : 0;
            } else if (Sq2 == Sq1 + NorthNorthEast && F1 != File1) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = NorthNorthEast;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = 8;
            } else if (Sq2 == Sq1 + NorthNorthWest && F1 != File9) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = NorthNorthWest;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = 9;
            } else if (Sq2 == Sq1 + SouthSouthEast && F1 != File1) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = SouthSouthEast;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = 10;
            } else if (Sq2 == Sq1 + SouthSouthWest && F1 != File9) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = SouthSouthWest;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = 11;
            } else if ((int)F1 - (int)F2 == (int)R1 - (int)R2) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? SouthEast : NorthWest;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? 3 : 7;
            } else if ((int)F1 - (int)F2 == (int)R2 - (int)R1) {
                Directions[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? NorthEast : SouthWest;
                SerializedDirections[(std::size_t)Sq1][(std::size_t)Sq2] = Comp? 1 : 5;
            }
        }
    }

    return {Directions, SerializedDirections};
}();

} // namespace

static constexpr std::array<std::array<Direction, NumSquares>, NumSquares> SquareDirection = DirectionDataInternal.first;
static constexpr std::array<std::array<uint8_t, NumSquares>, NumSquares> SquareSerializedDirection = DirectionDataInternal.second;

enum Stands : uint32_t {
    NumStandKinds =  8
};

static constexpr PieceTypeKind StandPieceTypes[7] = {
    PTK_Rook,
    PTK_Bishop,
    PTK_Gold,
    PTK_Silver,
    PTK_Knight,
    PTK_Lance,
    PTK_Pawn,
};

static constexpr uint32_t StandShifts[NumStandKinds] = {
    0,
    0, 6, 10, 14, 22, 25, 18,
};

static constexpr uint32_t StandBits[NumStandKinds] = {
    0U,
    1U <<   StandShifts[PTK_Pawn],
    1U <<  StandShifts[PTK_Lance],
    1U << StandShifts[PTK_Knight],
    1U << StandShifts[PTK_Silver],
    1U << StandShifts[PTK_Bishop],
    1U <<   StandShifts[PTK_Rook],
    1U <<   StandShifts[PTK_Gold],
};

static constexpr uint32_t StandMasks[NumStandKinds] = {
                                     0U,
    0b11111U <<   StandShifts[PTK_Pawn],
    0b00111U <<  StandShifts[PTK_Lance],
    0b00111U << StandShifts[PTK_Knight],
    0b00111U << StandShifts[PTK_Silver],
    0b00011U << StandShifts[PTK_Bishop],
    0b00011U <<   StandShifts[PTK_Rook],
    0b00111U <<   StandShifts[PTK_Gold],
};

template<PieceTypeKind Pt>
inline constexpr uint8_t getStandCount(Stands St) {
    return (uint8_t)((St & StandMasks[Pt]) >> StandShifts[Pt]);
}

inline constexpr uint8_t getStandCount(Stands St, PieceTypeKind Pt) {
    return (uint8_t)((St & StandMasks[Pt]) >> StandShifts[Pt]);
}

/// Increment the number of a `Pt` piece on `St`.
inline constexpr Stands incrementStand(Stands St, PieceTypeKind Pt) {
    return (Stands)(St + StandBits[Pt]);
}

/// Decrement the number of a `Pt` piece on `St`.
inline constexpr Stands decrementStand(Stands St, PieceTypeKind Pt) {
    assert(getStandCount(St, Pt) > 0);
    return (Stands)(St - StandBits[Pt]);
}

[[maybe_unused]]
inline constexpr Stands constructStand(uint8_t PawnCount, uint8_t LanceCount, uint8_t KnightCount, uint8_t SilverCount, uint8_t GoldCount, uint8_t BishopCount, uint8_t RookCount) {
    return (Stands)((uint32_t)PawnCount   * StandBits[PTK_Pawn]   +
                    (uint32_t)LanceCount  * StandBits[PTK_Lance]  +
                    (uint32_t)KnightCount * StandBits[PTK_Knight] +
                    (uint32_t)SilverCount * StandBits[PTK_Silver] +
                    (uint32_t)BishopCount * StandBits[PTK_Bishop] +
                    (uint32_t)RookCount   * StandBits[PTK_Rook]   +
                    (uint32_t)GoldCount   * StandBits[PTK_Gold]);
}

/// Checks if `St1` is superior or equal to `St2`.
///
/// Specifically, this function returns `true` if, for every type of standing piece
/// (Pawn, Lance, Knight, Silver, Gold, Bishop, and Rook), the number of each
/// piece type represented by `St1` is greater than or equal to the number of that
/// piece type represented by `St2`.
///
/// The function uses a mask to extract the relevant bits from the standing piece
/// representations and compare them.
///
/// @param St1 The first standing pieces representation.
/// @param St2 The second standing pieces representation.
/// @return `true` if `St1` is superior or equal to `St2`.
inline constexpr bool isSuperiorOrEqual(Stands St1, Stands St2) {
    constexpr uint32_t Mask = 1U << (StandShifts[PTK_Pawn]   + 5) |
                              1U << (StandShifts[PTK_Lance]  + 3) |
                              1U << (StandShifts[PTK_Knight] + 3) |
                              1U << (StandShifts[PTK_Silver] + 3) |
                              1U << (StandShifts[PTK_Bishop] + 2) |
                              1U << (StandShifts[PTK_Rook]   + 2) |
                              1U << (StandShifts[PTK_Gold]   + 3);

    return ((St1 - St2) & Mask) == 0;
}

struct Move16;

struct Move32 {
 public:
    constexpr Move32(const Move32& M): C_(M.C_) {
    }

    constexpr Move32() {
    }

    constexpr Move32& operator=(Move32 M) {
        C_ = M.C_;
        return *this;
    }

    constexpr Move32(Move16 M16);

    constexpr bool operator<(Move32 M) const {
        return C_ < M.C_;
    }

    constexpr bool operator==(Move32 M) const {
        return C_ == M.C_;
    }

    constexpr bool operator!=(Move32 M) const {
        return C_ != M.C_;
    }

    static constexpr Move32 MoveNone() {
        return Move32(0);
    }

    static constexpr Move32 MoveInvalid() {
        return Move32((81 << FromShift) | (81 << ToShift));
    }

    constexpr bool isNone() const {
        return C_ == 0;
    }

    constexpr bool isWin() const {
        return C_ == MoveWin().value();
    }

    constexpr bool isInvalid() const {
        return C_ == MoveInvalid().value();
    }

    constexpr uint32_t value() const {
        return C_;
    }

    static constexpr Move32 fromValue(uint32_t Value) {
        return Move32(Value);
    }

    /// This move is used for the declaration win.
    /// Note: since the from square and the to square is the same,
    /// this constant is not used as a standard move so this move
    /// will never cause conflicts.
    static constexpr Move32 MoveWin() {
        return Move32((1 << FromShift) | (1 << ToShift));
    }

    static Move32 boardMove(Square From, Square To, PieceTypeKind Pt) {
        return Move32(((uint32_t)Pt   << PieceTypeShift) |
                      ((uint32_t)From << FromShift)      |
                      ((uint32_t)To));
    }

    static Move32 boardMove(Square From, Square To, PieceTypeKind Pt, PieceTypeKind Capture) {
        return Move32(((uint32_t)Capture << CaptureTypeShift) |
                      ((uint32_t)Pt      << PieceTypeShift)   |
                      ((uint32_t)From    << FromShift)        |
                      ((uint32_t)To));
    }


    static Move32 boardPromotingMove(Square From, Square To, PieceTypeKind Pt) {
        return Move32(((uint32_t)1    << PromoteShift)   |
                      ((uint32_t)Pt   << PieceTypeShift) |
                      ((uint32_t)From << FromShift)      |
                      ((uint32_t)To));
    }

    static Move32 boardPromotingMove(Square From, Square To, PieceTypeKind Pt, PieceTypeKind Capture) {
        return Move32(((uint32_t)Capture << CaptureTypeShift) |
                      ((uint32_t)1       << PromoteShift)     |
                      ((uint32_t)Pt      << PieceTypeShift)   |
                      ((uint32_t)From    << FromShift)        |
                      ((uint32_t)To));
    }

    static Move32 droppingMove(Square To, PieceTypeKind Pt) {
        return Move32((uint32_t)Pt                             << PieceTypeShift |
                     ((uint32_t)Pt + (uint32_t)NumSquares - 1) << FromShift      |
                     ((uint32_t)To));
    }

    constexpr Square to() const {
        return (Square)(C_ & ToBits);
    }

    constexpr Square from() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (Square)(_bextr_u32(C_, FromShift, 7));
        }
#endif
        return (Square)((C_ & FromBits) >> FromShift);
    }

    constexpr bool promote() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (bool)(_bextr_u32(C_, PromoteShift, 1));
        }
#endif
        return (bool)((C_ & PromoteBit) >> PromoteShift);
    }

    constexpr PieceTypeKind pieceType() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (PieceTypeKind)(_bextr_u32(C_, PieceTypeShift, 4));
        }
#endif
        return (PieceTypeKind)((C_ & PieceTypeBits) >> PieceTypeShift);
    }

    constexpr PieceTypeKind capturePieceType() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (PieceTypeKind)(_bextr_u32(C_, CaptureTypeShift, 4));
        }
#endif
        return (PieceTypeKind)((C_ & CaptureTypeBits) >> CaptureTypeShift);
    }

    constexpr bool drop() const {
        return from() >= NumSquares;
    }

 private:
    constexpr Move32(uint32_t Val): C_(Val) {
    }

    static const uint32_t ToBits          = 0b00000000000000001111111;
    static const uint32_t FromBits        = 0b00000000011111110000000;
    static const uint32_t PromoteBit      = 0b00000000100000000000000;
    static const uint32_t PieceTypeBits   = 0b00001111000000000000000;
    static const uint32_t CaptureTypeBits = 0b11110000000000000000000;

    static const uint32_t ToShift          =  0;
    static const uint32_t FromShift        =  7;
    static const uint32_t PromoteShift     = 14;
    static const uint32_t PieceTypeShift   = 15;
    static const uint32_t CaptureTypeShift = 19;

    uint32_t C_;

 friend struct Move16;
 friend struct MoveList;
};

struct Move16 {
 public:
    constexpr Move16(Move32 M): C_(M.C_ & 0xffff) {
    }

    constexpr Move16(const Move16& M): C_(M.C_) {
    }

    constexpr Move16() {
    }

    constexpr Move16& operator=(Move16 M) {
        C_ = M.C_;
        return *this;
    }

    constexpr bool operator<(Move16 M) const {
        return C_ < M.C_;
    }

    constexpr bool operator==(Move16 M) const {
        return C_ == M.C_;
    }

    constexpr bool operator!=(Move16 M) const {
        return C_ != M.C_;
    }

    static constexpr Move16 MoveNone() {
        return Move16(0);
    }

    static constexpr Move16 MoveInvalid() {
        return Move16(Move32::MoveInvalid());
    }

    constexpr bool isNone() const {
        return C_ == 0;
    }

    constexpr bool isInvalid() const {
        return C_ == MoveInvalid().value();
    }

    static constexpr Move16 MoveWin() {
        return Move16(Move32::MoveWin());
    }

    constexpr Square from() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (Square)(_bextr_u32(C_, (uint16_t)Move32::FromShift, 7));
        }
#endif
        return (Square)((C_ & (uint16_t)Move32::FromBits) >> (uint16_t)Move32::FromShift);
    }

    constexpr Square to() const {
        return (Square)(C_ & (uint16_t)Move32::ToBits);
    }

    constexpr bool drop() const {
        return from() >= NumSquares;
    }

    constexpr bool promote() const {
#if defined(USE_BMI1)
        if (!std::is_constant_evaluated()) {
            return (bool)(_bextr_u32(C_, (uint16_t)Move32::PromoteShift, 1));
        }
#endif
        return (bool)((C_ & (uint16_t)Move32::PromoteBit) >> (uint16_t)Move32::PromoteShift);
    }

    constexpr uint16_t value() const {
        return C_;
    }

    static constexpr Move16 fromValue(uint16_t Value) {
        return Move16(Value);
    }

 private:
    constexpr Move16(uint16_t C): C_(C) {
    }

    uint16_t C_;

 friend struct Move32;
};

inline constexpr Move32::Move32(Move16 M16): C_(M16.C_) {
}

}  // namespace core
}  // namespace nshogi

// clang-format on

#endif // #ifndef NSHOGI_CORE_TYPES_H
