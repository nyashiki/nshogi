//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_TEST_COMMON_H
#define NSHOGI_TEST_COMMON_H

#include <chrono>
#include <concepts>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <cstdint>

namespace nshogi {
namespace test {
namespace common {

struct TestStatistics {
    TestStatistics(uint64_t Success = 0, uint64_t Fail = 0)
        : NumSuccess(Success)
        , NumFail(Fail) {
    }

    uint64_t NumSuccess;
    uint64_t NumFail;

    TestStatistics operator+(const TestStatistics& T) const {
        return TestStatistics(NumSuccess + T.NumSuccess, NumFail + T.NumFail);
    }

    TestStatistics& operator+=(const TestStatistics& T) {
        *this = *this + T;
        return *this;
    }
};

class TestCase {
 public:
    TestCase(std::function<void()> Func, const char* Suite, const char* Case)
        : TestFunc(Func)
        , SuiteName(Suite)
        , CaseName(Case) {
    }
    virtual ~TestCase() = default;

    TestStatistics run() const {
        const auto StartTime = std::chrono::steady_clock::now();
        std::cout << "[ RUN      ] " << SuiteName << "." << CaseName << std::endl;
        try {
            TestFunc();
            const auto EndTime = std::chrono::steady_clock::now();
            const auto Duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime).count();
            std::cout << "[       OK ] " << SuiteName << "." << CaseName
                << " (" << Duration << " milliseconds)" << std::endl;
            return TestStatistics(1, 0);
        } catch (const std::exception& Exception) {
            std::cout << Exception.what() << std::endl;
            std::cout << "[   FAILED ] " << SuiteName << "." << CaseName << std::endl;
            return TestStatistics(0, 1);
        } catch (...) {
            std::cout << "Unknown error." << std::endl;
            return TestStatistics(0, 1);
        }
    }

 private:
    std::function<void()> TestFunc;
    const char* SuiteName;
    const char* CaseName;
};

class TestSuite {
 public:
    explicit TestSuite(const char* Name) : SuiteName(Name) {}

    TestStatistics run() const {
        TestStatistics Statistics;
        for (const auto& testCase : TestCases) {
            Statistics += testCase.run();
        }
        return Statistics;
    }

    const char* name() const {
        return SuiteName.c_str();
    }

    void addTest(const TestCase& Case) {
        TestCases.push_back(Case);
    }

 private:
    std::string SuiteName;
    std::vector<TestCase> TestCases;
};

class TestBody {
 public:
    static TestBody& getInstance() {
        static TestBody Instance;
        return Instance;
    }

    TestStatistics run() const {
        TestStatistics Statistics;
        for (const auto& Suite : TestSuites) {
            Statistics += Suite.run();
        }
        return Statistics;
    }

    void addTest(const char* SuiteName, const char* CaseName, std::function<void()> TestFunc) {
        for (auto& Suite : TestSuites) {
            if (Suite.name() == std::string(SuiteName)) {
                Suite.addTest(TestCase(TestFunc, SuiteName, CaseName));
                return;
            }
        }
        TestSuite Suite(SuiteName);
        Suite.addTest(TestCase(TestFunc, SuiteName, CaseName));
        TestSuites.push_back(Suite);
    }

 private:
    std::vector<TestSuite> TestSuites;

    TestBody() = default;
    ~TestBody() = default;
    TestBody(const TestBody&) = delete;
    TestBody& operator=(const TestBody&) = delete;
};

} // namespace common
} // namespace test
} // namespace nshogi

#define TEST_ASSERT_EQ(Actual, Expected)                              \
    do {                                                              \
        auto&& _Actual = (Actual);                                    \
        auto&& _Expected = (Expected);                                \
        if (_Actual != _Expected) {                                   \
            std::ostringstream Oss;                                   \
            Oss << "Test failed: " << #Actual << " == " << #Expected; \
            throw std::runtime_error(Oss.str());                      \
        }                                                             \
    } while (false)

#define TEST_ASSERT_NEQ(Actual, Expected)                             \
    do {                                                              \
        auto&& _Actual = (Actual);                                    \
        auto&& _Expected = (Expected);                                \
        if (_Actual == _Expected) {                                   \
            std::ostringstream Oss;                                   \
            Oss << "Test failed: " << #Actual << " != " << #Expected; \
            throw std::runtime_error(Oss.str());                      \
        }                                                             \
    } while (false)

#define TEST_ASSERT_STREQ(Actual, Expected) \
    if (std::string(Actual) != std::string(Expected)) { \
        std::ostringstream Oss; \
        Oss << "Test failed: " << #Actual << " == " << #Expected << "\n" \
            << "    Expected: " << (Expected) << "\n" \
            << "    Actual: " << (Actual); \
        throw std::runtime_error(Oss.str()); \
    }

#define TEST_ASSERT_TRUE(Condition) \
    if (!(Condition)) { \
        std::ostringstream Oss; \
        Oss << "Test failed:\n    Expected " << #Condition << " is true."; \
        throw std::runtime_error(Oss.str()); \
    }

#define TEST_ASSERT_FALSE(Condition) \
    if (Condition) { \
        std::ostringstream Oss; \
        Oss << "Test failed:\n    Expected " << #Condition << " is false."; \
        throw std::runtime_error(Oss.str()); \
    }

#define TEST_ASSERT_FLOAT_EQ(Actual, Expected, Tolerance) \
    if (std::fabs((Actual) - (Expected)) > (Tolerance)) { \
        std::ostringstream Oss; \
        Oss << "Test failed: " << "\n" \
            << "    Expected: " << (Expected) << "\n" \
            << "    Actual: " << (Actual) << "\n" \
            << "    Tolerance: " << (Tolerance); \
        throw std::runtime_error(Oss.str()); \
    }

#define TEST(SuiteName, CaseName) \
    static void SuiteName##_##CaseName(); \
    namespace { \
        struct SuiteName##_##CaseName##_Adder { \
            SuiteName##_##CaseName##_Adder() { \
                nshogi::test::common::TestBody::getInstance().addTest(#SuiteName, #CaseName, SuiteName##_##CaseName); \
            } \
        } SuiteName##_##CaseName##_Adder_instance; \
    } \
    static void SuiteName##_##CaseName()

#endif // #ifndef NSHOGI_TEST_COMMON_H
