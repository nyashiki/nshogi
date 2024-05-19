#ifndef NSHOGI_CORE_STATE_H
#define NSHOGI_CORE_STATE_H

#include <memory>

#include <cassert>
#include <cstdint>

#include "bitboard.h"
#include "position.h"
#include "stateconfig.h"
#include "statehelper.h"
#include "hash.h"
#include "types.h"
#include "utils.h"

namespace nshogi {
namespace core {

enum struct RepetitionStatus : uint8_t {
    NoRepetition,
    Repetition,
    WinRepetition,
    LossRepetition,
    SuperiorRepetition,
    InferiorRepetition,
};

class State {
 public:
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = delete;

    State(State&&) noexcept;

    ~State() = default;

    State clone() const;

    // Getters.
    inline constexpr const Position& getPosition() const {
        return Pos;
    }

    template <Color C>
    inline constexpr const bitboard::Bitboard getBitboard() const {
        return Helper.ColorBB[C];
    }

    template <PieceTypeKind Type>
    inline constexpr const bitboard::Bitboard getBitboard() const {
        return Helper.TypeBB[Type];
    }

    template <Color C, PieceTypeKind Type>
    inline constexpr bitboard::Bitboard getBitboard() const {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    inline const bitboard::Bitboard getBitboard(Color C) const {
        return Helper.ColorBB[C];
    }

    inline bitboard::Bitboard getBitboard(Color C, PieceTypeKind Type) const {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    template <PieceTypeKind Type>
    inline bitboard::Bitboard getBitboard(Color C) const {
        return Helper.ColorBB[C] & Helper.TypeBB[Type];
    }

    inline const bitboard::Bitboard getCheckerBB() const {
        return Helper.getCurrentStepHelper().CheckerBB;
    }

    inline uint64_t getHash() const {
        return HashValue.getValue() ^
                ((uint64_t)(getPosition().getStand<Black>()) << 33) ^
                ((uint64_t)(getPosition().getStand<White>()) << 1);
    }

    const Position& getInitialPosition() const {
        return Helper.getInitialPosition();
    }

    uint16_t getPly() const {
        return getPosition().getPlyOffset() + Helper.getPly();
    }

    const Move32& getHistoryMove(uint16_t Ply) const {
        return Helper.getStepHelper(Ply).Move;
    }

    const Move32& getLastMove() const {
        return getHistoryMove(Helper.getPly() - 1);
    }

    // Manipulations.
    template <Color C> void doMove(const Move32& Move);
    void doMove(const Move32& Move);

    template <Color C> Move32 undoMove();
    Move32 undoMove();

    // Re-compute "second" variables, where "second" means
    // any variables that can be computed by "first" variables
    // that are neccesary to stand for the game representation.
    void refresh();

    // Helper functions.

    template <Color C> inline bool isSuicideMove(const Move32& Move) const {
        assert(Move.drop() ||
               getPieceType(Pos.pieceOn(Move.from())) == Move.pieceType());

        if (Move.drop()) {
            return false;
        }

        return isSuicideMoveImpl<C>(Move);
    }

    inline bool isSuicideMove(const Move32& Move) const {
        if (getPosition().sideToMove() == Black) {
            return isSuicideMove<Black>(Move);
        } else {
            return isSuicideMove<White>(Move);
        }
    }

    inline RepetitionStatus getRepetitionStatus() const {
        const Color SideToMove = getPosition().sideToMove();
        const StepHelper& CurrentStepHelper = Helper.getCurrentStepHelper();

        const Stands MyStand = getPosition().getStand(SideToMove);
        const Stands OpStand = getPosition().getStand(~SideToMove);

        for (int Ply = Helper.Ply - 4; Ply >= 0; Ply -= 2) {
            const StepHelper& SHelper = Helper.getStepHelper((uint16_t)Ply);
            const Stands MyStepStand = SHelper.EachStand[SideToMove];
            const Stands OpStepStand = SHelper.EachStand[~SideToMove];

            if (HashValue.getValue() == SHelper.BoardHash) {
                if (MyStand == MyStepStand && OpStand == OpStepStand) {
                    if (CurrentStepHelper.ContinuousCheckCounts[~SideToMove] * 2 >= Helper.Ply - Ply) {
                        return RepetitionStatus::WinRepetition;
                    }

                    if (CurrentStepHelper.ContinuousCheckCounts[SideToMove] * 2 >= Helper.Ply - Ply) {
                        return RepetitionStatus::LossRepetition;
                    }

                    return RepetitionStatus::Repetition;
                } else {
                    if (isSuperiorOrEqual(MyStand, MyStepStand)) {
                        return RepetitionStatus::SuperiorRepetition;
                    }

                    if (isSuperiorOrEqual(MyStepStand, MyStand)) {
                        return RepetitionStatus::InferiorRepetition;
                    }
                }
            }
        }

        return RepetitionStatus::NoRepetition;
    }

    inline constexpr Color getSideToMove() const {
        return getPosition().sideToMove();
    }

    template <Color C, PieceTypeKind Type>
    uint8_t getStandCount() const {
        return getPosition().getStandCount<C, Type>();
    }

    uint8_t getStandCount(Color C, PieceTypeKind Type) const {
        return getPosition().getStandCount(C, Type);
    }

    template <Color C> inline constexpr Square getKingSquare() const {
        return Helper.KingSquare[C];
    }

    inline constexpr Square getKingSquare(Color C) const {
        return Helper.KingSquare[C];
    }

    Move32 getMove32FromMove16(Move16 M16) const {
        if (M16.drop()) {
            const PieceTypeKind Type = (PieceTypeKind)(M16.from() - NumSquares + 1);
            return Move32::droppingMove(M16.to(), Type);
        }

        const Square From = M16.from();
        const Square To = M16.to();
        const PieceTypeKind Type = getPieceType(getPosition().pieceOn(From));
        const PieceTypeKind CaptureType = getPieceType(getPosition().pieceOn(To));

        if (M16.promote()) {
            return Move32::boardPromotingMove(From, To, Type, CaptureType);
        }

        return Move32::boardMove(From, To, Type, CaptureType);
    }

    template <Color C, uint8_t SliderScoreUnit, uint8_t StepScoreUnit, bool OnlyInPromotableZone>
    inline constexpr uint8_t computePieceScore() const {
        uint8_t SliderScore = 0;
        uint8_t StepScore = 0;

        bitboard::Bitboard SlideresBB =
            (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>() | getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())
            & getBitboard<C>();

        bitboard::Bitboard StepsBB = getBitboard<C>() ^ getBitboard<C, PTK_King>() ^ SlideresBB;

        if constexpr (OnlyInPromotableZone) {
            SlideresBB &= bitboard::PromotableBB[C];
            StepsBB &= bitboard::PromotableBB[C];
        }

        SliderScore = SlideresBB.popCount();
        StepScore = StepsBB.popCount();

        const Stands St = getPosition().getStand<C>();

        StepScore += (uint8_t)(core::getStandCount<PTK_Pawn>(St) + core::getStandCount<PTK_Lance>(St) +
                        core::getStandCount<PTK_Knight>(St) + core::getStandCount<PTK_Silver>(St) + core::getStandCount<PTK_Gold>(St));

        SliderScore += (uint8_t)(core::getStandCount<PTK_Bishop>(St) + core::getStandCount<PTK_Rook>(St));
        return (uint8_t)(SliderScore * SliderScoreUnit + StepScore * StepScoreUnit);
    }

    template <Color C, uint8_t SliderScoreUnit = 5, uint8_t StepScoreUnit = 1>
    inline constexpr uint8_t computeDeclarationScore() const {
        return computePieceScore<C, SliderScoreUnit, StepScoreUnit, true>();
    }

    bool canDeclare() const;

    template <Color C>
    inline constexpr const bitboard::Bitboard getDefendingOpponentSliderBB() const {
        return Helper.getCurrentStepHelper().DefendingOpponentSliderBB[C];
    }

    template <Color C>
    inline bitboard::Bitboard getStepAttackBB(Square ExcludeSq = SqInvalid) const {
        bitboard::Bitboard StepAttackBB = bitboard::Bitboard::ZeroBB();

        // North attacks.
        bitboard::Bitboard NorthAttackCandidateBB = getBitboard<C>() &
            ((C == Black)? (getBitboard<PTK_Pawn>()      |
                            getBitboard<PTK_Silver>()    |
                            getBitboard<PTK_Gold>()      |
                            getBitboard<PTK_King>()      |
                            getBitboard<PTK_ProPawn>()   |
                            getBitboard<PTK_ProLance>()  |
                            getBitboard<PTK_ProKnight>() |
                            getBitboard<PTK_ProSilver>() |
                            getBitboard<PTK_ProBishop>())

                        : (getBitboard<PTK_Gold>()      |
                           getBitboard<PTK_King>()      |
                           getBitboard<PTK_ProPawn>()   |
                           getBitboard<PTK_ProLance>()  |
                           getBitboard<PTK_ProKnight>() |
                           getBitboard<PTK_ProSilver>() |
                           getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            NorthAttackCandidateBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        StepAttackBB |= (NorthAttackCandidateBB & ~bitboard::RankBB[RankA]).getLeftShiftEpi64<1>();

        // NorthEast attacks and NorthWest attacks.
        bitboard::Bitboard NorthEastAndNorthWestAttackCandidateBB = getBitboard<C>() &
            ((C == Black) ? (getBitboard<PTK_Silver>()    |
                             getBitboard<PTK_Gold>()      |
                             getBitboard<PTK_King>()      |
                             getBitboard<PTK_ProPawn>()   |
                             getBitboard<PTK_ProLance>()  |
                             getBitboard<PTK_ProKnight>() |
                             getBitboard<PTK_ProSilver>() |
                             getBitboard<PTK_ProRook>())

                         : (getBitboard<PTK_King>()  |
                            getBitboard<PTK_Silver>() |
                            getBitboard<PTK_ProRook>()));

        if (ExcludeSq != SqInvalid) {
            NorthEastAndNorthWestAttackCandidateBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        StepAttackBB |= (NorthEastAndNorthWestAttackCandidateBB & ~bitboard::RankBB[RankA] & ~bitboard::FileBB[File1]).getRightShiftSi128<8>();
        StepAttackBB |= (NorthEastAndNorthWestAttackCandidateBB & ~bitboard::RankBB[RankA] & ~bitboard::FileBB[File9]).getLeftShiftSi128<10>();

        // East attacks and West attacks.
        bitboard::Bitboard EastAndWestAttackCandidateBB = getBitboard<C>() &
            ((C == Black) ? (getBitboard<PTK_Gold>()      |
                             getBitboard<PTK_King>()      |
                             getBitboard<PTK_ProPawn>()   |
                             getBitboard<PTK_ProLance>()  |
                             getBitboard<PTK_ProKnight>() |
                             getBitboard<PTK_ProSilver>() |
                             getBitboard<PTK_ProBishop>())

                         : (getBitboard<PTK_Gold>()      |
                            getBitboard<PTK_King>()      |
                            getBitboard<PTK_ProPawn>()   |
                            getBitboard<PTK_ProLance>()  |
                            getBitboard<PTK_ProKnight>() |
                            getBitboard<PTK_ProSilver>() |
                            getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            EastAndWestAttackCandidateBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        StepAttackBB |= (EastAndWestAttackCandidateBB & ~bitboard::FileBB[File1]).getRightShiftSi128<9>();
        StepAttackBB |= (EastAndWestAttackCandidateBB & ~bitboard::FileBB[File9]).getLeftShiftSi128<9>();

        // SouthEast attacks and SouthWest attacks.
        bitboard::Bitboard SouthEastAndSouthWestAttackCandidateBB = getBitboard<C>() &
            ((C == Black)? (getBitboard<PTK_King>()   |
                            getBitboard<PTK_Silver>() |
                            getBitboard<PTK_ProRook>())

                        : (getBitboard<PTK_Silver>()    |
                           getBitboard<PTK_Gold>()      |
                           getBitboard<PTK_King>()      |
                           getBitboard<PTK_ProPawn>()   |
                           getBitboard<PTK_ProLance>()  |
                           getBitboard<PTK_ProKnight>() |
                           getBitboard<PTK_ProSilver>() |
                           getBitboard<PTK_ProRook>()));

        if (ExcludeSq != SqInvalid) {
            SouthEastAndSouthWestAttackCandidateBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        StepAttackBB |= (SouthEastAndSouthWestAttackCandidateBB & ~bitboard::RankBB[RankI] & ~bitboard::FileBB[File1]).getRightShiftSi128<10>();
        StepAttackBB |= (SouthEastAndSouthWestAttackCandidateBB & ~bitboard::RankBB[RankI] & ~bitboard::FileBB[File9]).getLeftShiftSi128<8>();

        // South attacks.
        bitboard::Bitboard SouthAttackCandidateBB = getBitboard<C>() &
            ((C == Black)? (getBitboard<PTK_Gold>()      |
                            getBitboard<PTK_King>()      |
                            getBitboard<PTK_ProPawn>()   |
                            getBitboard<PTK_ProLance>()  |
                            getBitboard<PTK_ProKnight>() |
                            getBitboard<PTK_ProSilver>() |
                            getBitboard<PTK_ProBishop>())

                        : (getBitboard<PTK_Pawn>()      |
                           getBitboard<PTK_Silver>()    |
                           getBitboard<PTK_Gold>()      |
                           getBitboard<PTK_King>()      |
                           getBitboard<PTK_ProPawn>()   |
                           getBitboard<PTK_ProLance>()  |
                           getBitboard<PTK_ProKnight>() |
                           getBitboard<PTK_ProSilver>() |
                           getBitboard<PTK_ProBishop>()));

        if (ExcludeSq != SqInvalid) {
            SouthAttackCandidateBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        StepAttackBB |= (SouthAttackCandidateBB & ~bitboard::RankBB[RankI]).getRightShiftEpi64<1>();

        // Knight attacks.
        bitboard::Bitboard KnightBB = getBitboard<C, PTK_Knight>();

        if (ExcludeSq != SqInvalid) {
            KnightBB &= ~bitboard::SquareBB[ExcludeSq];
        }

        if constexpr (C == Black) {
            StepAttackBB |= (KnightBB & ~bitboard::FirstAndSecondFurthestBB[C] & ~bitboard::FileBB[File1]).getRightShiftSi128<7>();
            StepAttackBB |= (KnightBB & ~bitboard::FirstAndSecondFurthestBB[C] & ~bitboard::FileBB[File9]).getLeftShiftSi128<11>();
        } else {
            StepAttackBB |= (KnightBB & ~bitboard::FirstAndSecondFurthestBB[White] & ~bitboard::FileBB[File1]).getRightShiftSi128<11>();
            StepAttackBB |= (KnightBB & ~bitboard::FirstAndSecondFurthestBB[White] & ~bitboard::FileBB[File9]).getLeftShiftSi128<7>();
        }

        return StepAttackBB;
    }

    /// Compute Sliders' attacks by Dumb7fill algorithm.
    template <Color C>
    bitboard::Bitboard getSliderAttackBB(Square ExcludeSq = SqInvalid, Square VirtualSq = SqInvalid) const {
        const bitboard::Bitboard OccupiedBB =
            (VirtualSq == SqInvalid)? (getBitboard<Black>() | getBitboard<White>())
                                    : (getBitboard<Black>() | getBitboard<White>() | bitboard::SquareBB[VirtualSq]);

        const bitboard::Bitboard EmptyBB = (ExcludeSq == SqInvalid)? ~OccupiedBB : (~OccupiedBB | bitboard::SquareBB[ExcludeSq]);

        bitboard::Bitboard SliderAttackBB = bitboard::Bitboard::ZeroBB();

        // Bishops.
        const bitboard::Bitboard BishopBB = getBitboard<C>() &
            ((ExcludeSq == SqInvalid)? (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>())
                                     : ((getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>()) & ~bitboard::SquareBB[ExcludeSq]));

        const bitboard::Bitboard NotRankABB = ~bitboard::RankBB[RankA];
        const bitboard::Bitboard NotRankIBB = ~bitboard::RankBB[RankI];
        const bitboard::Bitboard NotRankAAndEmptyBB = NotRankABB & EmptyBB;
        const bitboard::Bitboard NotRankIAndEmptyBB = NotRankIBB & EmptyBB;

        bitboard::Bitboard GeneratorBB;
        bitboard::Bitboard TempBB;

        if (!BishopBB.isZero()) {
            // clang-format off
            GeneratorBB = BishopBB;
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankABB).getRightShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();

            TempBB = GeneratorBB & NotRankAAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getRightShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getRightShiftSi128<8>();
            }

            GeneratorBB = BishopBB;
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIBB).getRightShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
            TempBB = GeneratorBB & NotRankIAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getRightShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftSi128<10>();
            }

            GeneratorBB = BishopBB;
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIBB).getLeftShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
            TempBB = GeneratorBB & NotRankIAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getLeftShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getLeftShiftSi128<8>();
            }

            GeneratorBB = BishopBB;
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankABB).getLeftShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
            TempBB = GeneratorBB & NotRankAAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getLeftShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftSi128<10>();
            }
            // clang-format on
        }

        const bitboard::Bitboard RookBB = getBitboard<C>() &
            ((ExcludeSq == SqInvalid)? (getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())
                                    : ((getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()) & ~bitboard::SquareBB[ExcludeSq]));

        const bitboard::Bitboard LanceBB = getBitboard<C>() &
            ((ExcludeSq == SqInvalid)? getBitboard<PTK_Lance>()
                                    : (getBitboard<PTK_Lance>() & ~bitboard::SquareBB[ExcludeSq]));

        const bitboard::Bitboard ForwardBB  = (C == Black)? (RookBB | LanceBB) : RookBB;
        const bitboard::Bitboard BackwardBB = (C == White)? (RookBB | LanceBB) : RookBB;

        if (!ForwardBB.isZero()) {
            GeneratorBB = ForwardBB;

            // clang-format off
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankABB).getLeftShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
            TempBB = GeneratorBB & NotRankAAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getLeftShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankAAndEmptyBB).getLeftShiftEpi64<1>();
            }
            // clang-format on
        }

        if (!BackwardBB.isZero()) {
            GeneratorBB = BackwardBB;

            // clang-format off
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIBB).getRightShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
            TempBB = GeneratorBB & NotRankIAndEmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getRightShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & NotRankIAndEmptyBB).getRightShiftEpi64<1>();
            }
            // clang-format on
        }

        if (!RookBB.isZero()) {
            // clang-format off
            GeneratorBB = RookBB;
            SliderAttackBB |= GeneratorBB = GeneratorBB.getRightShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
            TempBB = GeneratorBB & EmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getRightShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getRightShiftSi128<9>();
            }
            // clang-format on

            // clang-format off
            GeneratorBB = RookBB;
            SliderAttackBB |= GeneratorBB = GeneratorBB.getLeftShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
            SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
            TempBB = GeneratorBB & EmptyBB;
            if (!TempBB.isZero()) {
                SliderAttackBB |= GeneratorBB = TempBB.getLeftShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
                SliderAttackBB |= GeneratorBB = (GeneratorBB & EmptyBB).getLeftShiftSi128<9>();
            }
            // clang-format on
        }

        return SliderAttackBB;
    }

    template <Color C>
    inline bitboard::Bitboard getAttackBB(Square ExcludeSq = SqInvalid) const {
        return getStepAttackBB<C>(ExcludeSq) | getSliderAttackBB<C>(ExcludeSq);
    }

    template <Color C, PieceTypeKind Type>
    inline bool isAttackedByOneStep(Square Sq) const {
        return bitboard::isAttacked(bitboard::getAttackBB<C, Type>(Sq),
                                    getBitboard<~C, Type>());
    }

    template <Color C, PieceTypeKind Type>
    inline bool isAttackedBySlider(Square Sq,
                                   const bitboard::Bitboard& OccupiedBB) const {
        static_assert(Type == PTK_Lance || Type == PTK_Bishop ||
                          Type == PTK_ProBishop || Type == PTK_Rook ||
                          Type == PTK_ProRook,
                      "possibly caused by calling isAttackedBySlider() with a "
                      "template parameter "
                      "that must not be passed in.");

        if constexpr (Type == PTK_Lance) {
            return bitboard::isAttacked(
                bitboard::getLanceAttackBB<C>(Sq, OccupiedBB),
                getBitboard<~C, PTK_Lance>());
        } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
            return bitboard::isAttacked(
                bitboard::getBishopAttackBB<Type>(Sq, OccupiedBB),
                getBitboard<~C, Type>());
        } else if constexpr (Type == PTK_Rook || Type == PTK_ProRook) {
            return bitboard::isAttacked(
                bitboard::getRookAttackBB<Type>(Sq, OccupiedBB),
                getBitboard<~C, Type>());
        }
    }

    template <Color C>
    inline bool isAttacked(Square Sq, Square ExcludeSq = SqInvalid) const {
        if (isAttackedByOneStep<C, PTK_Pawn>(Sq) ||
            isAttackedByOneStep<C, PTK_Knight>(Sq) ||
            isAttackedByOneStep<C, PTK_Silver>(Sq) ||
            isAttackedByOneStep<C, PTK_Gold>(Sq) ||
            isAttackedByOneStep<C, PTK_King>(Sq) ||
            isAttackedByOneStep<C, PTK_ProPawn>(Sq) ||
            isAttackedByOneStep<C, PTK_ProLance>(Sq) ||
            isAttackedByOneStep<C, PTK_ProKnight>(Sq) ||
            isAttackedByOneStep<C, PTK_ProSilver>(Sq)) {

            return true;
        }

        bitboard::Bitboard OccupiedBB =
            getBitboard<Black>() | getBitboard<White>();
        if (ExcludeSq != SqInvalid) {
            OccupiedBB.toggleBit(ExcludeSq);
        }

        if (isAttackedBySlider<C, PTK_Lance>(Sq, OccupiedBB) ||
            isAttackedBySlider<C, PTK_Bishop>(Sq, OccupiedBB) ||
            isAttackedBySlider<C, PTK_Rook>(Sq, OccupiedBB) ||
            isAttackedBySlider<C, PTK_ProBishop>(Sq, OccupiedBB) ||
            isAttackedBySlider<C, PTK_ProRook>(Sq, OccupiedBB)) {

            return true;
        }

        return false;
    }

 private:
    State() = delete;
    State(const Position& P);
    State(const Position& CurrentP, const Position& InitP);

    Position Pos;
    Hash<uint64_t> HashValue;
    StateHelper Helper;

    template <Color C, bool UpdateCheckerBySliders>
    void setDefendingOpponentSliderBB(StepHelper* SHelper,
                                      const bitboard::Bitboard& OccupiedBB);

    template <Color C>
    void setCheckerBB(StepHelper* SHelper);

    template <Color C> inline bool isSuicideMoveImpl(const Move32& Move) const {
        if (Move.pieceType() == PTK_King) {
            // When moving the king, the square moving onto
            // must be not attacked by opponent pieces.
            return isAttacked<C>(Move.to(), Move.from());
        }

        // Here, we check the other pieces (i.e., excluding king) moves.
        // If the piece is defending an opponent slider attack,
        // it must move onto a square that is on the same line.
        // Note: it cannot be true that one piece is defending
        // more than one opponent slider attacks by the rule.
        const Square From = Move.from();
        if (!getDefendingOpponentSliderBB<C>().isSet(From)) {
            // This piece is irrelevant defending an opponent slider attack.
            return false;
        }

        // Now, this piece is defending opponent slider attacks.
        // Therefore this piece must go onto a square on the same line.
        return !utils::isSameLine(From, Move.to(), getKingSquare<C>());
    }

    friend class StateBuilder;
};

} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_STATE_H
