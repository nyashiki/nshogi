# nshogi: a shogi library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![build](https://github.com/Nyashiki/nshogi/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/Nyashiki/nshogi/actions/workflows/build.yml)
[![test](https://github.com/nyashiki/nshogi/actions/workflows/test.yml/badge.svg?branch=master)](https://github.com/nyashiki/nshogi/actions/workflows/test.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/c82fbf71ad27453395499d1a677326fe)](https://app.codacy.com/gh/nyashiki/nshogi/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

nshogi is a comprehensive shogi library that allows you to manipulate game states, determine the next state from a given move and state, and generate all legal moves for a given state, and more.
Additionally, nshogi provides essential functionalities for machine learning, such as creating feature vectors for AlphaZero's neural network from a given state.

## Installation

### Requirements

- GNU Make
- g++ or clang++
- (optional) for Python module
  - python3
  - pybind11

### Build from source

#### C++ Library

Type `make [CXX=<CXX>] [<SSE INSTRUCTION>=1] [PREFIX=<PREFIX>] install` in your terminal, where:

- `<CXX>` is a C++ compiler.
- `<SSE INSTRUCTION>` is one of the {SSE, SSE41, SSE42, AVX, AVX2}, depends on the instructions your CPU supports.
- `<PREFIX>` is in which directory the library will be installed. The default value is `/usr/local`.

#### Python module

Type `make [CXX=<CXX>] [<SSE INSTRUCTION>=1] install-python` in your terminal, where:

- `<CXX>` is a C++ compiler.
- `<SSE INSTRUCTION>` is one of the {SSE, SSE41, SSE42, AVX, AVX2}, depends on the instructions your CPU supports.

### Run tests for the library

Type `make [CXX=<CXX>] [<SSE INSTRUCTION>=1] [PREFIX=<PREFIX>] runtest-{static|shared}` in your terminal, where:

- `<CXX>` is a C++ compiler.
- `<SSE INSTRUCTION>` is one of the {SSE, SSE41, SSE42, AVX, AVX2}, depends on the instructions your CPU supports.
- `<PREFIX>` is in which directory the library will be installed. The default value is `/usr/local`.
- runtest-static target will test the static library, and runtest-shared target will test the shared library.

### Examples with code

See [wiki](https://github.com/nyashiki/nshogi/wiki).

## License

This repository is released under the MIT License.

For details about licenses of third-party dependencies, please see [LICENSE-THIRD-PARTY.md](./LICENSE-THIRD-PARTY.md).

### Important note on included data

This repository includes some resources in the `res/` directory that are sourced from [やねうら王公式からクリスマスプレゼントに詰将棋500万問を謹呈](https://yaneuraou.yaneu.com/2020/12/25/christmas-present/).
These data files are proveded under their own terms and are **not** subject to the MIT License.
For more details, please refer to `res/README.md`.
