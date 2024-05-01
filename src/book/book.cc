#include "book.h"

#include <memory>
#include "../core/statebuilder.h"
#include "../io/sfen.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <cmath>

namespace nshogi {
namespace book {

Book::Book() {
}

Book::Book(Book&& B) noexcept {
    Entries = std::move(B.Entries);
};

Book Book::loadYaneuraOuFormat(const char* Path, double PonanzaConstant, const char* Desc) {
    Book B;

    std::ifstream Ifs(Path);
    std::string Line;

    std::unique_ptr<core::State> State = nullptr;

    std::size_t Counter = 0;
    while (std::getline(Ifs, Line)) {
        std::cout << "\rCounter: " << Counter++ << std::flush;
        if (Line[0] == '#') {
            // A line beginning with # is comment.
            continue;
        }

        if (Line.size() > 4 && Line.starts_with("sfen")) {
            // Set a target position.
            State = std::make_unique<core::State>(io::sfen::StateBuilder::newState(Line.substr(5)));
            continue;
        }

        if (State == nullptr) {
            continue;
        }

        std::stringstream SS(Line);
        std::string SMove, SCounterMove;
        double Evaluation;
        int Depth;

        SS >> SMove >> SCounterMove >> Evaluation >> Depth;

        auto Move = io::sfen::sfenToMove32(State->getPosition(), SMove);

        Entry* E = B.addEntry(io::sfen::positionToSfen(State->getPosition()).c_str());
        BookMove* BM = E->addBookMove(Move);

        State->doMove(Move);
        auto CounterMove = (SCounterMove == "none")
            ? core::Move32::MoveNone()
            : io::sfen::sfenToMove32(State->getPosition(), SCounterMove);
        State->undoMove();

        const double WinRate = 1.0 / (1.0 + std::exp(-Evaluation / PonanzaConstant));

        BM->getMeta().setCount(1);
        BM->getMeta().setWinRate(WinRate);
        BM->getMeta().setDrawRate(0);
        BM->getMeta().setPV(&CounterMove, 1);
        if (Desc == nullptr) {
            BM->getMeta().setDescription("Imported YaneuraOu-Formatted Book.");
        } else {
            BM->getMeta().setDescription(Desc);
        }
    }

    return B;
}

Book Book::makeBookFromSfens(const char* Path, uint16_t PlyLimit, uint16_t PlyCutoff, const char* Desc) {
    Book B;

    B.makeBookFromSfensInternal(Path, PlyLimit, PlyCutoff, Desc);

    return B;
}

void Book::patchFromSfens(const char* Path, uint16_t PlyLimit, uint16_t PlyCutoff, const char* Desc) {
    removePositionsInSfens(Path);
    makeBookFromSfensInternal(Path, PlyLimit, PlyCutoff, Desc);
}

Entry* Book::addEntry(const char* Sfen) {
    Entry* ExistingEntry = findEntry(Sfen);

    if (ExistingEntry != nullptr) {
        return ExistingEntry;
    }

    std::unique_ptr<Entry> NewEntry = std::make_unique<Entry>(Sfen);
    Entry* NewEntryP = NewEntry.get();

    Entries.emplace(std::string(Sfen), std::move(NewEntry));
    return NewEntryP;
}

Entry* Book::findEntry(const char* Sfen) {
    auto It = Entries.find(Sfen);

    if (It == Entries.end()) {
        return nullptr;
    }

    return It->second.get();
}

std::size_t Book::size() const {
    return Entries.size();
}

void Book::dump(const char* Path) const {
    std::ofstream Ofs(Path, std::ios::out | std::ios::binary);

    for (const auto& E: Entries) {
        E.second->dump(Ofs);
    }
}

Book Book::load(const char* Path) {
    std::ifstream Ifs(Path, std::ios::in | std::ios::binary);

    Book B;

    std::size_t NumEntries = 0;
    {
        std::size_t EntrySize = 0;
        [[maybe_unused]] Entry Dummy = Entry::load(Ifs);

        EntrySize = (std::size_t)Ifs.tellg();

        Ifs.seekg(0, std::ios_base::end);
        std::size_t FileSize = (std::size_t)Ifs.tellg();

        if (FileSize % EntrySize != 0) {
            std::cout << "FileSize: " << FileSize << std::endl;
            std::cout << "EntrySize: " << EntrySize << std::endl;

            throw std::runtime_error("FileSize must be divided by the entry size.\n"
                    "Unfortunately, the file is possibly broken.");
        }

        Ifs.seekg(0, std::ios_base::beg);
        NumEntries = FileSize / EntrySize;
    }

    for (std::size_t I = 0; I < NumEntries; ++I) {
        std::unique_ptr<Entry> NewEntry = std::make_unique<Entry>(Entry::load(Ifs));
        std::string Key = std::string(NewEntry->getSfen());
        B.Entries.emplace(std::move(Key), std::move(NewEntry));
    }

    return B;
}

void Book::makeBookFromSfensInternal(const char* Path, uint16_t PlyLimit, uint16_t PlyCutoff, const char* Desc) {
    std::ifstream Ifs(Path);
    std::string Line;

    std::size_t Counter = 0;
    while (std::getline(Ifs, Line)) {
        if (Line.size() == 0 || Line[0] == '#') {
            continue;
        }

        const auto State = io::sfen::StateBuilder::newState(Line);
        auto Replay = core::StateBuilder::newState(State.getInitialPosition());

        const core::Color Winner = ~State.getSideToMove();

        for (uint16_t Ply = 0; Ply < State.getPly(); ++Ply) {
            if (PlyLimit > 0 && Ply >= PlyLimit) {
                break;
            }

            if (PlyCutoff > 0 && State.getPly() >= PlyCutoff && Ply >= State.getPly() - PlyCutoff) {
                break;
            }

            const auto& Move = State.getHistoryMove(Ply);

            if (Winner == Replay.getSideToMove()) {
                Entry* E = addEntry(io::sfen::positionToSfen(Replay.getPosition()).c_str());

                BookMove* BM = E->addBookMove(Move);

                if (BM != nullptr) {
                    BM->getMeta().incrementCount();
                    BM->getMeta().setWinRate(0.5);
                    BM->getMeta().setDrawRate(0);
                    if (Desc == nullptr) {
                        BM->getMeta().setDescription("Made from sfens.");
                    } else {
                        BM->getMeta().setDescription(Desc);
                    }
                    ++Counter;
                }
            }

            Replay.doMove(Move);
        }
        std::printf("\rProcessed: %ld", Counter);
    }

    std::printf("\rProcessed: %ld\n", Counter);
}

void Book::removePositionsInSfens(const char* Path) {
    std::ifstream Ifs(Path);
    std::string Line;

    while (std::getline(Ifs, Line)) {
        if (Line.size() == 0 || Line[0] == '#') {
            continue;
        }

        const auto State = io::sfen::StateBuilder::newState(Line);
        auto Replay = core::StateBuilder::newState(State.getInitialPosition());

        const core::Color Winner = ~State.getSideToMove();

        for (uint16_t Ply = 0; Ply < State.getPly(); ++Ply) {
            const auto& Move = State.getHistoryMove(Ply);

            if (Winner == Replay.getSideToMove()) {
                Entry* E = findEntry(io::sfen::positionToSfen(Replay.getPosition()).c_str());

                if (E != nullptr) {
                    E->clear();
                }
            }

            Replay.doMove(Move);
        }
    }
}

} // namespace book
} // namespace nshogi
