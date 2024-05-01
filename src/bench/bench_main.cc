#include "../core/initializer.h"
#include "../core/state.h"
#include "../io/sfen.h"
#include "../io/csa.h"
#include "../ml/featurestack.h"
#include "../solver/mate1ply.h"
#include "common.hpp"

#include <fstream>

void benchMoveGeneration(const nshogi::core::State&);
void benchMate1ply(const std::vector<nshogi::core::State>&);
void benchPerft(int Ply);

int main() {
    using namespace nshogi::bench;

    nshogi::core::initializer::initializeAll();

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

    return 0;
}
