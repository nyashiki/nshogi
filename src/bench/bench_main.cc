#include "../core/initializer.h"
#include "../core/state.h"
#include "../io/sfen.h"
#include "../io/csa.h"
#include "../ml/featurestack.h"
#include "../solver/mate1ply.h"
#include "../core/movegenerator.h"
#include "common.hpp"
#include "../core/internal/stateadapter.h"
#include <fstream>
#include <random>

void benchMoveGeneration(const nshogi::core::State&);
// void benchMoveGenerationInternal(const nshogi::core::internal::StateImpl&);
void benchMate1ply(const std::vector<nshogi::core::State>&);
void benchPerft(int Ply);

int main() {
    using namespace nshogi;
    using namespace nshogi::bench;

    nshogi::core::initializer::initializeAll();

    // std::mt19937_64 Mt(12345);
    // for (int i = 0; i < 10000; ++i) {
    //     core::State State = core::StateBuilder::getInitialState();

    //     while (true) {
    //         auto Moves = core::MoveGenerator::generateLegalMoves(State);

    //         if (Moves.size() == 0) {
    //             break;
    //         }

    //         const auto OccupiedBB =
    //             State.getBitboard<core::Black>() |
    //             State.getBitboard<core::White>();

    //         if (State.getSideToMove() == core::Black) {
    //             using namespace nshogi::core::bitboard;

    //             State.getBitboard<core::Black, core::PTK_Rook>().forEach([&](core::Square Sq) {
    //                 core::bitboard::Bitboard RookAttackBB = core::bitboard::getRookAttackBB<nshogi::core::PTK_Rook>(Sq, OccupiedBB);

    //                 auto Bottom = RankBB[core::RankA].subtract(SquareBB[Sq]);
    //                 auto Top = SquareBB[Sq].subtract(OccupiedBB ^ SquareBB[Sq]);

    //                 auto BB = Top.subtract(Bottom) ^ SquareBB[Sq];

    //                 if ((RookAttackBB & FileBB[core::squareToFile(Sq)]) != BB) {
    //                     std::cout << "RookAttackBB =====" << std::endl;
    //                     io::bitboard::print(RookAttackBB);
    //                     std::cout << "Bottom =====" << std::endl;
    //                     io::bitboard::print(Bottom);
    //                     std::cout << "Top =====" << std::endl;
    //                     io::bitboard::print(Top);
    //                     std::cout << "BB =====" << std::endl;
    //                     io::bitboard::print(BB);
    //                     std::abort();
    //                 }
    //             });
    //         }

    //         auto Move = Moves[Mt() % Moves.size()];
    //         State.doMove(Move);
    //     }
    // }

    // return 0;

    const int N = 10000000;

    {
        nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::getInitialState();
        runCountBench("Movegeneration initial position", benchMoveGeneration, N, State);
    }

    {
        nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::newState("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1");
        runCountBench("Movegeneration matsuri position", benchMoveGeneration, N, State);
    }

    {
        nshogi::core::State State =
        nshogi::io::sfen::StateBuilder::newState("R8/2K1S1SSk/4B4/9/9/9/9/9/1L1L1L3 b RBGSNLP3g3n17p 1");
        runCountBench("Movegeneration max-moves# position", benchMoveGeneration, N, State);
    }

    // {
    //     nshogi::core::State State =
    //     nshogi::io::sfen::StateBuilder::getInitialState();
    //     runCountBench("Movegeneration initial position", benchMoveGenerationInternal, N, *core::internal::ImmutableStateAdapter(State).get());
    // }

    // {
    //     nshogi::core::State State =
    //     nshogi::io::sfen::StateBuilder::newState("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1");
    //     runCountBench("Movegeneration matsuri position", benchMoveGenerationInternal, N, *core::internal::ImmutableStateAdapter(State).get());
    // }

    // {
    //     nshogi::core::State State =
    //     nshogi::io::sfen::StateBuilder::newState("R8/2K1S1SSk/4B4/9/9/9/9/9/1L1L1L3 b RBGSNLP3g3n17p 1");
    //     runCountBench("Movegeneration max-moves# position", benchMoveGenerationInternal, N, *core::internal::ImmutableStateAdapter(State).get());
    // }

    {
        std::vector<nshogi::core::State> PositiveStates;
        std::ifstream Ifs("./res/bench/mate-1-ply.txt");

        std::string Line;
        while (std::getline(Ifs, Line)) {
            PositiveStates.push_back(nshogi::io::sfen::StateBuilder::newState(Line));
        }

        runCountBench("Positive Mate1Ply", benchMate1ply, 10000, PositiveStates);
    }

    {
        std::vector<nshogi::core::State> NegativeStates;
        std::ifstream Ifs("./res/bench/no-mate-1-ply.txt");


        std::string Line;
        while (std::getline(Ifs, Line)) {
            NegativeStates.push_back(nshogi::io::sfen::StateBuilder::newState(Line));
        }

        runCountBench("Negative Mate1Ply", benchMate1ply, 10000, NegativeStates);
    }

    runCountBench("perft 1", benchPerft, 1, 1);
    runCountBench("perft 2", benchPerft, 1, 2);
    runCountBench("perft 3", benchPerft, 1, 3);
    runCountBench("perft 4", benchPerft, 1, 4);
    runCountBench("perft 5", benchPerft, 1, 5);
    runCountBench("perft 6", benchPerft, 1, 6);

    return 0;
}
