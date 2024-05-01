#ifndef NSHOGI_BENCH_COMMON_HPP
#define NSHOGI_BENCH_COMMON_HPP

#include <chrono>
#include <cinttypes>
#include <functional>
#include <iostream>

namespace nshogi {
namespace bench {

struct BenchResult {
    std::string Name;
    double MilliSeconds;
    uint64_t Called;
};

template <typename Function, bool TimeBased, typename... Arguments>
inline BenchResult run_(const std::string& Name, Function&& F, uint32_t Limit,
                        Arguments&&... Args) {
    using namespace std::chrono;

    auto start = high_resolution_clock::now();

    uint64_t Count = 0;

    if constexpr (TimeBased) {
        auto end = start + seconds(Limit);
        while (high_resolution_clock::now() < end) {
            F(std::forward<Arguments>(Args)...);
            ++Count;
        }
    } else {
        while (Count < Limit) {
            F(std::forward<Arguments>(Args)...);
            ++Count;
        }
    }

    const double Elapsed =
        (double)duration_cast<milliseconds>(high_resolution_clock::now() - start)
            .count();

    return {Name, Elapsed, Count};
}

template <typename Function, typename... Arguments>
inline void runDurationBench(const std::string& Name, Function&& F,
                             uint32_t Duration, Arguments&&... Args) {
    std::cout << "Name: " << Name << " (running)" << std::flush;

    auto Result =
        run_<Function, true, Arguments...>(Name, F, Duration, std::forward<Arguments>(Args)...);

    print(Result);
}

template <typename Function, typename... Arguments>
inline void runCountBench(const std::string& Name, Function&& F, uint32_t Count,
                          Arguments&&... Args) {
    std::cout << "Name: " << Name << " (running)" << std::flush;

    auto Result = run_<Function, false, Arguments...>(Name, F, Count, std::forward<Arguments>(Args)...);

    print(Result);
}

inline void print(const BenchResult& Result) {
    const double CalledPerSecond = (double)Result.Called * 1000 / Result.MilliSeconds;

    std::cout << "\rName: " << Result.Name << " (done)    " << std::endl;
    std::cout << "    +---- Called: " << Result.Called << std::endl;
    std::cout << "    +---- Elapsed: " << Result.MilliSeconds << " (ms)"
              << std::endl;
    std::cout << "    +---- Called per second: " << CalledPerSecond
              << std::endl;
}

} // namespace bench
} // namespace nshogi

#endif // #ifndef NSHOGI_BENCH_COMMON_HPP
