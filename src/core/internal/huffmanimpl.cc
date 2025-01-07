//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "huffmanimpl.h"
#include "../positionbuilder.h"

namespace nshogi {
namespace core {
namespace internal {

namespace {

struct HuffmanCodeElem {
 public:
    constexpr HuffmanCodeElem(uint8_t PT, uint8_t S)
        : Pattern(PT)
        , Size(S) {
    }

    uint8_t Pattern;
    uint8_t Size;
};

// clang-format off

static constexpr HuffmanCodeElem HuffmanTable[31] = {
    {        0b1, 1 }, // Empty
    {     0b0000, 4 }, // BlackPawn
    {   0b000010, 6 }, // BlackLance
    {   0b001010, 6 }, // BlackKnight
    {   0b000110, 6 }, // BlackSilver
    { 0b00001110, 8 }, // BlackBishop
    { 0b00101110, 8 }, // BlackRook
    {   0b011110, 6 }, // BlackGold
    { 0,          0 }, // BlackKing
    {     0b0100, 4 }, // BlackProPawn
    {   0b010010, 6 }, // BlackProLance
    {   0b011010, 6 }, // BlackProKnight
    {   0b010110, 6 }, // BlackProSilver
    { 0b01001110, 8 }, // BlackProBishop
    { 0b01101110, 8 }, // BlackProRook
    { 0,          0 }, // (NOT USED)
    { 0,          0 }, // (NOT USED)
    {     0b1000, 4 }, // WhitePawn
    {   0b100010, 6 }, // WhiteLance
    {   0b101010, 6 }, // WhiteKnight
    {   0b100110, 6 }, // WhiteSilver
    { 0b10001110, 8 }, // WhiteBishop
    { 0b10101110, 8 }, // WhiteRook
    {   0b111110, 6 }, // WhiteGold
    { 0         , 0 }, // WhiteKing
    {     0b1100, 4 }, // WhiteProPawn
    {   0b110010, 6 }, // WhiteProLance
    {   0b111010, 6 }, // WhiteProKnight
    {   0b110110, 6 }, // WhiteProSilver
    { 0b11001110, 8 }, // WhiteProBishop
    { 0b11101110, 8 }, // WhiteProRook
};

static constexpr HuffmanCodeElem HuffmanStandTable[8] = {
    {        0, 0 }, // Empty
    {     0b00, 2 }, // Pawn
    {   0b0001, 4 }, // Lance
    {   0b0101, 4 }, // Knight
    {   0b0011, 4 }, // Silver
    { 0b001111, 6 }, // Bishop
    { 0b011111, 6 }, // Rook
    {   0b0111, 4 }, // Gold
};

// clang-format on

static constexpr auto HuffmanLUT =
    []() -> std::array<std::array<uint16_t, 1 << 9>, 9> {
    std::array<std::array<uint16_t, 1 << 9>, 9> LUT;

    for (std::size_t I = 0; I < 9; ++I) {
        for (std::size_t Pattern = 0; Pattern < (1 << 9); ++Pattern) {
            LUT[I][Pattern] = 0;
        }
    }

    for (std::size_t I = 0; I < 31; ++I) {
        std::size_t Size = (std::size_t)(HuffmanTable[I].Size);

        if (Size == 0) {
            continue;
        }

        if (Size == 1) {
            LUT[Size][HuffmanTable[I].Pattern] = 1;
        } else {
            LUT[Size][HuffmanTable[I].Pattern] = (uint16_t)(I + 1);
        }
    }

    return LUT;
}();

static constexpr auto HuffmanStandLUT =
    []() -> std::array<std::array<uint16_t, 1 << 8>, 8> {
    std::array<std::array<uint16_t, 1 << 8>, 8> LUT;

    for (std::size_t I = 0; I < 8; ++I) {
        for (std::size_t Pattern = 0; Pattern < (1 << 8); ++Pattern) {
            LUT[I][Pattern] = 0;
        }
    }

    for (std::size_t I = 0; I < 8; ++I) {
        std::size_t Size = (std::size_t)(HuffmanStandTable[I].Size);

        if (Size == 0) {
            continue;
        }

        LUT[Size][HuffmanStandTable[I].Pattern] = (uint16_t)(I + 1);
    }

    return LUT;
}();

class HuffmanCodeStream : public HuffmanCodeImpl {
 public:
    HuffmanCodeStream()
        : HuffmanCodeImpl()
        , Cursor(0) {
    }

    HuffmanCodeStream(const HuffmanCodeImpl& HC)
        : HuffmanCodeImpl(HC)
        , Cursor(0) {
    }

    template <typename T>
    void write(T Val, uint8_t Size) {
        for (uint8_t I = 0; I < Size; ++I) {
            const uint8_t Index = Cursor >> 6;
            const uint8_t Shift = Cursor & 0x3f;
            Data[Index] |= (uint64_t)(Val & 0b1) << Shift;
            Val = (T)(Val >> 1);

            assert((uint16_t)Cursor < 256);
            ++Cursor;
        }
    }

    uint8_t read() {
        const uint8_t Index = Cursor >> 6;
        const uint8_t Shift = Cursor & 0x3f;
        uint8_t Bit = (Data[Index] >> Shift) & 0b1;

        assert((uint16_t)Cursor < 256);
        ++Cursor;

        return Bit;
    }

 private:
    uint8_t Cursor;
};

void writeStand(HuffmanCodeStream* HCS, Color C, PieceTypeKind Kind,
                uint8_t Count) {
    while (Count > 0) {
        const auto& Elem = HuffmanStandTable[(std::size_t)Kind];
        HCS->write(Elem.Pattern, Elem.Size);
        HCS->write(C, 1);
        --Count;
    }
}

class HuffmanPositionBuilder : public PositionBuilder {
 public:
    HuffmanPositionBuilder(const HuffmanCodeImpl& HC) {
        HuffmanCodeStream HCS(HC);

        int LoadBits = 0;
        uint8_t Pattern = 0;
        uint8_t Size = 0;
        Square Sq = (Square)0;

        Square BlackKingSquare, WhiteKingSquare;
        for (int I = 0; I < 7; ++I) {
            Pattern |= (uint8_t)(HCS.read() << I);
            ++LoadBits;
        }
        BlackKingSquare = (Square)Pattern;

        Pattern = 0;
        for (int I = 0; I < 7; ++I) {
            Pattern |= (uint8_t)(HCS.read() << I);
            ++LoadBits;
        }
        WhiteKingSquare = (Square)Pattern;

        Pattern = 0;
        while (Sq <= (Square)80) {
            if (Sq == BlackKingSquare || Sq == WhiteKingSquare) {
                Sq = (Square)((int)Sq + 1);
                continue;
            }

            Pattern |= (uint8_t)(HCS.read() << Size);
            ++Size;
            ++LoadBits;

            if (HuffmanLUT[Size][Pattern] != 0) {
                setPiece(Sq, (PieceKind)(HuffmanLUT[Size][Pattern] - 1));
                Sq = (Square)((int)Sq + 1);
                Pattern = 0;
                Size = 0;
            }
        }

        setPiece((Square)BlackKingSquare, PK_BlackKing);
        setPiece((Square)WhiteKingSquare, PK_WhiteKing);

        Color SideToMove = (Color)HCS.read();

        Pattern = 0;
        Size = 0;

        clearStands();

        while (LoadBits < 256) {
            Pattern |= (uint8_t)(HCS.read() << Size);
            ++Size;
            ++LoadBits;

            if (HuffmanStandLUT[Size][Pattern] != 0) {
                Color C = (Color)HCS.read();
                ++LoadBits;

                incrementStand(
                    C, (PieceTypeKind)(HuffmanStandLUT[Size][Pattern] - 1));

                Pattern = 0;
                Size = 0;
            }
        }

        setColor(SideToMove);
    }
};

} // namespace

HuffmanCodeImpl HuffmanCodeImpl::encode(const Position& Pos) {
    Square BlackKingSquare = (Square)0;
    Square WhiteKingSquare = (Square)0;

    for (Square Sq = (Square)0; Sq < NumSquares; ++Sq) {
        if (Pos.pieceOn(Sq) == PK_BlackKing) {
            BlackKingSquare = Sq;
        } else if (Pos.pieceOn(Sq) == PK_WhiteKing) {
            WhiteKingSquare = Sq;
        }
    }

    return encode(Pos, BlackKingSquare, WhiteKingSquare);
}

HuffmanCodeImpl HuffmanCodeImpl::encode(const Position& Pos,
                                        Square BlackKingSquare,
                                        Square WhiteKingSquare) {
    HuffmanCodeStream HCS;

    HCS.write(BlackKingSquare, 7);
    HCS.write(WhiteKingSquare, 7);

    for (Square Sq = (Square)0; Sq < NumSquares; ++Sq) {
        const PieceKind PK = Pos.pieceOn(Sq);

        if (Sq == BlackKingSquare || Sq == WhiteKingSquare) {
            continue;
        }

        const auto& Elem = HuffmanTable[PK];
        assert(Elem.Size > 0);
        HCS.write(Elem.Pattern, Elem.Size);
    }

    HCS.write(Pos.sideToMove(), 1);

    writeStand(&HCS, Black, PTK_Pawn, Pos.getStandCount<Black, PTK_Pawn>());
    writeStand(&HCS, Black, PTK_Lance, Pos.getStandCount<Black, PTK_Lance>());
    writeStand(&HCS, Black, PTK_Knight, Pos.getStandCount<Black, PTK_Knight>());
    writeStand(&HCS, Black, PTK_Silver, Pos.getStandCount<Black, PTK_Silver>());
    writeStand(&HCS, Black, PTK_Bishop, Pos.getStandCount<Black, PTK_Bishop>());
    writeStand(&HCS, Black, PTK_Rook, Pos.getStandCount<Black, PTK_Rook>());
    writeStand(&HCS, Black, PTK_Gold, Pos.getStandCount<Black, PTK_Gold>());

    writeStand(&HCS, White, PTK_Pawn, Pos.getStandCount<White, PTK_Pawn>());
    writeStand(&HCS, White, PTK_Lance, Pos.getStandCount<White, PTK_Lance>());
    writeStand(&HCS, White, PTK_Knight, Pos.getStandCount<White, PTK_Knight>());
    writeStand(&HCS, White, PTK_Silver, Pos.getStandCount<White, PTK_Silver>());
    writeStand(&HCS, White, PTK_Bishop, Pos.getStandCount<White, PTK_Bishop>());
    writeStand(&HCS, White, PTK_Rook, Pos.getStandCount<White, PTK_Rook>());
    writeStand(&HCS, White, PTK_Gold, Pos.getStandCount<White, PTK_Gold>());

    return HCS;
}

Position HuffmanCodeImpl::decode(const HuffmanCodeImpl& HC) {
    HuffmanPositionBuilder HPB(HC);
    return HPB.build();
}

} // namespace internal
} // namespace core
} // namespace nshogi
