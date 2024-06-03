# nshogi: a shogi library.

[![build](https://github.com/Nyashiki/nshogi/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/Nyashiki/nshogi/actions/workflows/build.yml)
[![test](https://github.com/nyashiki/nshogi/actions/workflows/test.yml/badge.svg?branch=master)](https://github.com/nyashiki/nshogi/actions/workflows/test.yml)

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
- `<SSE INSTRUCTION>` is one of the {SSE, SSE41, SSE42, AVX, AVX2}, depends on the instructions your CPU have.

### Run tests for the library
Type `make [CXX=<CXX>] [<SSE INSTRUCTION>=1] [PREFIX=<PREFIX>] runtest-{static|shared}` in your terminal, where:
- `<CXX>` is a C++ compiler.
- `<SSE INSTRUCTION>` is one of the {SSE, SSE41, SSE42, AVX, AVX2}, depends on the instructions your CPU have.
- `<PREFIX>` is in which directory the library will be installed. The default value is `/usr/local`.
- runtest-static target will test the static library, and runtest-shared target will test the shared library.

### Examples with code
Under construction.
