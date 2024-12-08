#ifndef NSHOGI_CORE_INTERNAL_STATEIMPL_H
#define NSHOGI_CORE_INTERNAL_STATEIMPL_H

#include "bitboard.h"
#include "hash.h"
#include "statehelper.h"
#include "../position.h"

namespace nshogi {
namespace core {
namespace internal {

class StateImpl {
 public:
    StateImpl() = delete;
    StateImpl(const StateImpl&) = delete;
    StateImpl(StateImpl&&) noexcept;
    StateImpl(const Position& P);
    StateImpl(const Position& P, uint16_t Ply);
    StateImpl(const Position& CurrentP, const Position& InitP);

    StateImpl& operator=(const StateImpl&) = delete;
    StateImpl& operator=(StateImpl&&) = delete;

    ~StateImpl() = default;

    StateImpl clone() const;

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

    uint16_t getPly(bool IncludeOffset = true) const {
        return Helper.getPly(IncludeOffset);
    }

    Move32 getHistoryMove(uint16_t Ply) const {
        return Helper.getStepHelper(Ply).Move;
    }

    Move32 getLastMove() const {
        return getHistoryMove(getPly(false) - 1);
    }

    // Manipulations.
    template <Color C>
    void doMove(Move32 Move);
    void doMove(Move32 Move);

    template <Color C>
    Move32 undoMove();
    Move32 undoMove();

    // Re-compute "second" variables, where "second" means
    // any variables that can be computed by "first" variables
    // that are neccesary to stand for the game representation.
    void refresh();

    // Helper functions.

    template <bool Strict = false>
    inline RepetitionStatus getRepetitionStatus() const {
        const Color SideToMove = getPosition().sideToMove();
        const StepHelper& CurrentStepHelper = Helper.getCurrentStepHelper();

        const Stands MyStand = getPosition().getStand(SideToMove);
        const Stands OpStand = getPosition().getStand(~SideToMove);

        uint16_t RepetitionCount = 0;
        for (int Ply = Helper.Ply - 4; Ply >= 0; Ply -= 2) {
            const StepHelper& SHelper = Helper.getStepHelper((uint16_t)Ply);
            const Stands MyStepStand = SHelper.EachStand[SideToMove];
            const Stands OpStepStand = SHelper.EachStand[~SideToMove];

            if (HashValue.getValue() == SHelper.BoardHash) {
                if (MyStand == MyStepStand && OpStand == OpStepStand) {
                    if constexpr (Strict) {
                        if (CurrentStepHelper.ContinuousCheckCounts[~SideToMove] >= 6 &&
                                CurrentStepHelper.ContinuousCheckCounts[~SideToMove] * 2 >= Helper.Ply - Ply) {
                            return RepetitionStatus::WinRepetition;
                        }
                    } else {
                        if (CurrentStepHelper.ContinuousCheckCounts[~SideToMove] * 2 >= Helper.Ply - Ply) {
                            return RepetitionStatus::WinRepetition;
                        }
                    }

                    if constexpr (Strict) {
                        if (CurrentStepHelper.ContinuousCheckCounts[SideToMove] >= 6 &&
                                CurrentStepHelper.ContinuousCheckCounts[SideToMove] * 2 >= Helper.Ply - Ply) {
                            return RepetitionStatus::LossRepetition;
                        }
                    } else {
                        if (CurrentStepHelper.ContinuousCheckCounts[SideToMove] * 2 >= Helper.Ply - Ply) {
                            return RepetitionStatus::LossRepetition;
                        }
                    }

                    if constexpr (Strict) {
                        if (RepetitionCount == 2) {
                            return RepetitionStatus::Repetition;
                        }
                        ++RepetitionCount;
                        Ply -= 2;
                    } else {
                        return RepetitionStatus::Repetition;
                    }
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

    template <Color C>
    inline constexpr Square getKingSquare() const {
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

    template <Color C>
    uint8_t computePieceScore(uint8_t SliderScoreUnit, uint8_t StepScoreUnit, bool OnlyInPromotableZone) const {
        uint8_t SliderScore = 0;
        uint8_t StepScore = 0;

        bitboard::Bitboard SlideresBB =
            (getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>() | getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())
            & getBitboard<C>();

        bitboard::Bitboard StepsBB = getBitboard<C>() ^ getBitboard<C, PTK_King>() ^ SlideresBB;

        if (OnlyInPromotableZone) {
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

    template <Color C>
    inline constexpr uint8_t computeDeclarationScore(uint8_t SliderScoreUnit = 5, uint8_t StepScoreUnit = 1) const {
        return computePieceScore<C>(SliderScoreUnit, StepScoreUnit, true);
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
            NorthAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(NorthAttackCandidateBB);
        }

        StepAttackBB |= bitboard::RankBB[RankA].andNot(NorthAttackCandidateBB).getLeftShiftEpi64<1>();

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
            NorthEastAndNorthWestAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(NorthEastAndNorthWestAttackCandidateBB);
        }

        StepAttackBB |= (bitboard::FileBB[File1].andNot(bitboard::RankBB[RankA].andNot(NorthEastAndNorthWestAttackCandidateBB))).getRightShiftSi128<8>();
        StepAttackBB |= (bitboard::FileBB[File9].andNot(bitboard::RankBB[RankA].andNot(NorthEastAndNorthWestAttackCandidateBB))).getLeftShiftSi128<10>();

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
            EastAndWestAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(EastAndWestAttackCandidateBB);
        }

        StepAttackBB |= (bitboard::FileBB[File1].andNot(EastAndWestAttackCandidateBB)).getRightShiftSi128<9>();
        StepAttackBB |= (bitboard::FileBB[File9].andNot(EastAndWestAttackCandidateBB)).getLeftShiftSi128<9>();

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
            SouthEastAndSouthWestAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(SouthEastAndSouthWestAttackCandidateBB);
        }

        StepAttackBB |= (bitboard::FileBB[File1].andNot(bitboard::RankBB[RankI].andNot(SouthEastAndSouthWestAttackCandidateBB))).getRightShiftSi128<10>();
        StepAttackBB |= (bitboard::FileBB[File9].andNot(bitboard::RankBB[RankI].andNot(SouthEastAndSouthWestAttackCandidateBB))).getLeftShiftSi128<8>();

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
            SouthAttackCandidateBB = bitboard::SquareBB[ExcludeSq].andNot(SouthAttackCandidateBB);
        }

        StepAttackBB |= (bitboard::RankBB[RankI].andNot(SouthAttackCandidateBB)).getRightShiftEpi64<1>();

        // Knight attacks.
        bitboard::Bitboard KnightBB = getBitboard<C, PTK_Knight>();

        if (ExcludeSq != SqInvalid) {
            KnightBB = bitboard::SquareBB[ExcludeSq].andNot(KnightBB);
        }

        if constexpr (C == Black) {
            StepAttackBB |= (bitboard::FileBB[File1].andNot(bitboard::FirstAndSecondFurthestBB[C].andNot(KnightBB))).getRightShiftSi128<7>();
            StepAttackBB |= (bitboard::FileBB[File9].andNot(bitboard::FirstAndSecondFurthestBB[C].andNot(KnightBB))).getLeftShiftSi128<11>();
        } else {
            StepAttackBB |= (bitboard::FileBB[File1].andNot(bitboard::FirstAndSecondFurthestBB[White].andNot(KnightBB))).getRightShiftSi128<11>();
            StepAttackBB |= (bitboard::FileBB[File9].andNot(bitboard::FirstAndSecondFurthestBB[White].andNot(KnightBB))).getLeftShiftSi128<7>();
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
                                     : (bitboard::SquareBB[ExcludeSq].andNot(getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>())));

        const bitboard::Bitboard NotRankAAndEmptyBB = bitboard::RankBB[RankA].andNot(EmptyBB);
        const bitboard::Bitboard NotRankIAndEmptyBB = bitboard::RankBB[RankI].andNot(EmptyBB);

        bitboard::Bitboard GeneratorBB;
        bitboard::Bitboard TempBB;

        if (!BishopBB.isZero()) {
            // clang-format off
            GeneratorBB = BishopBB;
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankA].andNot(GeneratorBB).getRightShiftSi128<8>();
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
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankI].andNot(GeneratorBB).getRightShiftSi128<10>();
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
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankI].andNot(GeneratorBB).getLeftShiftSi128<8>();
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
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankA].andNot(GeneratorBB).getLeftShiftSi128<10>();
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
                                    : (bitboard::SquareBB[ExcludeSq].andNot(getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>())));

        const bitboard::Bitboard LanceBB = getBitboard<C>() &
            ((ExcludeSq == SqInvalid)? getBitboard<PTK_Lance>()
                                    : (bitboard::SquareBB[ExcludeSq].andNot(getBitboard<PTK_Lance>())));

        const bitboard::Bitboard ForwardBB  = (C == Black)? (RookBB | LanceBB) : RookBB;
        const bitboard::Bitboard BackwardBB = (C == White)? (RookBB | LanceBB) : RookBB;

        if (!ForwardBB.isZero()) {
            GeneratorBB = ForwardBB;

            // clang-format off
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankA].andNot(GeneratorBB).getLeftShiftEpi64<1>();
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
            SliderAttackBB |= GeneratorBB = bitboard::RankBB[RankI].andNot(GeneratorBB).getRightShiftEpi64<1>();
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

    template <Color C>
    inline bool isAttackedByOneStep(Square Sq) const {
        const bitboard::Bitboard UnderAttackedBB =
            (bitboard::getAttackBB<C, PTK_Pawn>(Sq) & getBitboard<PTK_Pawn>()) |
            (bitboard::getAttackBB<C, PTK_Knight>(Sq) & getBitboard<PTK_Knight>()) |
            (bitboard::getAttackBB<C, PTK_Silver>(Sq) & getBitboard<PTK_Silver>()) |
            (bitboard::getAttackBB<C, PTK_King>(Sq) & getBitboard<PTK_King>()) |
            (bitboard::getAttackBB<C, PTK_Gold>(Sq) & (
               getBitboard<PTK_Gold>() | getBitboard<PTK_ProPawn>() | getBitboard<PTK_ProLance>() |
               getBitboard<PTK_ProKnight>() | getBitboard<PTK_ProSilver>()));

        return !(UnderAttackedBB & getBitboard<~C>()).isZero();
    }

    template <Color C, PieceTypeKind Type>
    inline bool isAttackedBySlider(Square Sq, const bitboard::Bitboard& OccupiedBB, Square VirtualSq = SqInvalid) const {
        static_assert(Type == PTK_Lance || Type == PTK_Bishop ||
                          Type == PTK_ProBishop || Type == PTK_Rook ||
                          Type == PTK_ProRook,
                      "possibly caused by calling isAttackedBySlider() with a "
                      "template parameter "
                      "that must not be passed in.");

        if constexpr (Type == PTK_Lance) {
            return (getBitboard<~C, PTK_Lance>() & bitboard::getForwardBB<C>(Sq)).any([&](Square LanceSq) {
                if (VirtualSq != SqInvalid && LanceSq == VirtualSq) {
                    return false;
                }
                const bitboard::Bitboard BetweenOccupiedBB =
                    bitboard::getBetweenBB(Sq, LanceSq) & OccupiedBB;

                return BetweenOccupiedBB.isZero();
            });
        } else if constexpr (Type == PTK_Bishop || Type == PTK_ProBishop) {
            if (((getBitboard<PTK_Bishop>() | getBitboard<PTK_ProBishop>()) & getBitboard<~C>() & bitboard::DiagBB[Sq]).any([&](Square BishopSq) {
                if (VirtualSq != SqInvalid && BishopSq == VirtualSq) {
                    return false;
                }
                const bitboard::Bitboard BetweenOccupiedBB =
                    bitboard::getBetweenBB(Sq, BishopSq) & OccupiedBB;

                return BetweenOccupiedBB.isZero();
            })) {
                return true;
            }

            return bitboard::isAttacked(
                    bitboard::KingAttackBB[Sq], getBitboard<~C, PTK_ProBishop>());
        } else if constexpr (Type == PTK_Rook || Type == PTK_ProRook) {
            if (((getBitboard<PTK_Rook>() | getBitboard<PTK_ProRook>()) & getBitboard<~C>() & bitboard::CrossBB[Sq]).any([&](Square RookSq) {
                if (VirtualSq != SqInvalid && RookSq == VirtualSq) {
                    return false;
                }
                const bitboard::Bitboard BetweenOccupiedBB =
                    bitboard::getBetweenBB(Sq, RookSq) & OccupiedBB;

                return BetweenOccupiedBB.isZero();
            })) {
                return true;
            }

            return bitboard::isAttacked(
                    bitboard::KingAttackBB[Sq], getBitboard<~C, PTK_ProRook>());
        }
    }

    template <Color C>
    inline bool isAttackedBySlider(Square Sq,
                                   const bitboard::Bitboard& OccupiedBB, Square ExcludeSq = SqInvalid, Square VirtualSq = SqInvalid) const {
        bitboard::Bitboard OccupiedBB2 = OccupiedBB;
        if (ExcludeSq != SqInvalid) {
            assert(OccupiedBB.isSet(ExcludeSq));
            OccupiedBB2.toggleBit(ExcludeSq);
        }
        if (VirtualSq != SqInvalid) {
            OccupiedBB2 |= bitboard::SquareBB[VirtualSq];
        }
        return isAttackedBySlider<C, PTK_Bishop>(Sq, OccupiedBB2, VirtualSq) ||
            isAttackedBySlider<C, PTK_Rook>(Sq, OccupiedBB2, VirtualSq) ||
            isAttackedBySlider<C, PTK_Lance>(Sq, OccupiedBB2, VirtualSq);
    }

    template <Color C>
    inline bool isAttacked(Square Sq, Square ExcludeSq = SqInvalid, Square VirtualSq = SqInvalid) const {
        if (isAttackedByOneStep<C>(Sq)) {
            return true;
        }

        const bitboard::Bitboard OccupiedBB = getBitboard<Black>() | getBitboard<White>();
        return isAttackedBySlider<C>(Sq, OccupiedBB, ExcludeSq, VirtualSq);
    }

 private:
    Position Pos;
    Hash<uint64_t> HashValue;
    StateHelper Helper;

    template <Color C, bool UpdateCheckerBySliders>
    void setDefendingOpponentSliderBB(StepHelper* SHelper,
                                      const bitboard::Bitboard& OccupiedBB);

    template <Color C>
    void setCheckerBB(StepHelper* SHelper);

};

} // namespace internal
} // namespace core
} // namespace nshogi

#endif // #ifndef NSHOGI_CORE_INTERNAL_STATEIMPL_H
