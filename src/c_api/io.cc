#include "../io/sfen.h"

extern "C" {

#include "io.h"

void nshogi_io_malloc_state_from_sfen(State** St, const char* Sfen) {
    auto* S = new nshogi::core::State(nshogi::io::sfen::StateBuilder::newState(Sfen));
    *St = reinterpret_cast<State*>(S);
}

uint32_t nshogi_io_get_sfen_from_state(const State* State, char* Sfen) {
    const auto* S = reinterpret_cast<const nshogi::core::State*>(State);
    const auto SfenString = nshogi::io::sfen::stateToSfen(*S);
    std::strcpy(Sfen, SfenString.c_str());
    return (uint32_t)SfenString.size();
}

uint32_t nshogi_io_get_sfen_from_move(Move32 Move, char* Sfen) {
    const auto SfenString = nshogi::io::sfen::move32ToSfen(nshogi::core::Move32::fromValue(Move));
    std::strcpy(Sfen, SfenString.c_str());
    return (uint32_t)SfenString.size();
}

} // extern "C"
