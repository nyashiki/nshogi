//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#ifndef NSHOGI_TEST_UTILS_H
#define NSHOGI_TEST_UTILS_H

#include "common.h"
#include <iostream>
#include <string>
#include <unistd.h>

namespace nshogi {
namespace test {

template <typename Function, typename... Arguments>
inline void testStdOut(const std::string& Expected, Function&& F,
                       Arguments&&... Args) {
    int PipeFD[2];
    char Buffer[4096] = {};

    // Make a temporary pipe.
    if (pipe(PipeFD) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Redirect all writes from stdout to the new pipe.
    fflush(stdout);
    int StdOut = dup(STDOUT_FILENO);
    if (dup2(PipeFD[1], STDOUT_FILENO) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(PipeFD[1]);

    // Call function F().
    F(std::forward<Arguments>(Args)...);

    fflush(stdout);
    dup2(StdOut, STDOUT_FILENO);
    close(StdOut);

    // Read from the buffer.
    ssize_t ReadBytes = read(PipeFD[0], Buffer, sizeof(Buffer));
    close(PipeFD[0]);
    Buffer[ReadBytes] = '\0';

    TEST_ASSERT_STREQ(Buffer, Expected.c_str());
}

} // namespace test
} // namespace nshogi

#endif // #ifndef NSHOGI_TEST_UTILS_H
