#ifndef NSHOGI_ML_AZTEACHER_H
#define NSHOGI_ML_AZTEACHER_H

#include "../core/types.h"
#include "../core/state.h"
#include "../core/stateconfig.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <array>
#include <vector>

namespace nshogi {
namespace ml {

struct AZTeacher {
 private:
    static constexpr std::size_t SfenCStrLength = 132;
    static constexpr uint8_t NumSavedPlayouts = 16;

 public:
    AZTeacher();

    AZTeacher(const AZTeacher& T);

    void dump(std::ofstream& Ofs) const {
        dump_0_1_0(Ofs);
    }

    void dump_0_1_0(std::ofstream& Ofs) const;

    bool checkSanity(int Level) const;

    [[nodiscard]]
    static AZTeacher load(std::ifstream& Ifs);

    // State.
    char Sfen[SfenCStrLength];

    // Targets.
    core::Color SideToMove;
    core::Color Winner;
    uint8_t NumMoves;
    std::array<std::array<char, 6>, NumSavedPlayouts> Moves;
    std::array<uint16_t, NumSavedPlayouts> Visits;

    // State configs.
    core::EndingRule EndingRule;
    uint16_t MaxPly;
    float BlackDrawValue;
    float WhiteDrawValue;

    // The other helpers.
    bool Declared;

    /// Generate completely random noisy data.
    /// This function is for test.
    [[maybe_unused]]
    void corruptMyself();

    [[maybe_unused]]
    bool equals(const AZTeacher& T) const;

 private:
    [[nodiscard]]
    static AZTeacher load_0_1_0(std::ifstream& Ifs);
};

} // namespace ml
} // namespace nshogi


#endif // #ifndef NSHOGI_ML_AZTEACHER_H
