#
# Copyright (c) 2025 @nyashiki
#
# This software is licensed under the MIT license.
# For details, see the LICENSE file in the root of this repository.
#
# SPDX-License-Identifier: MIT
#

CXX := clang++

PREFIX ?= /usr/local

BUILD ?= release
VERSION = $(shell cat NSHOGI_VERSION)

OBJDIR = build/$(BUILD)_$(CXX)
ifeq ($(shell uname), Darwin)
	SHARED_TARGET_NAME := libnshogi.$(VERSION).dylib
    SHARED_TARGET_NAME_NO_VERSION := libnshogi.dylib
else
	SHARED_TARGET_NAME := libnshogi.so.$(VERSION)
    SHARED_TARGET_NAME_NO_VERSION := libnshogi.so
endif
SHARED_TARGET := $(OBJDIR)/lib/$(SHARED_TARGET_NAME)
STATIC_TARGET := $(OBJDIR)/lib/libnshogi_static.a
PYTHON_TARGET := $(OBJDIR)/lib/nshogi$(shell python3-config --extension-suffix)
TEST_STATIC_TARGET := $(OBJDIR)/bin/libnshogi_test_static
TEST_SHARED_TARGET := $(OBJDIR)/bin/libnshogi_test_shared
BENCH_TARGET := $(OBJDIR)/bin/nshogi_bench

INCLUDES :=
LINKS :=

PYTHON_INCLUDES := $(shell python3-config --includes) $(shell python3 -m pybind11 --includes)
PYTHON_LINKS := $(shell python3-config --ldflags) -Wl,-undefined,dynamic_lookup

ifeq ($(BUILD), debug)
	# CXX_FLAGS = -std=c++2b -Wall -Wextra -Wconversion -Wpedantic -Wshadow -fno-omit-frame-pointer -fsanitize=address -pipe
	CXX_FLAGS = -std=c++2b -Wall -Wextra -Wconversion -Wpedantic -Wshadow -fno-omit-frame-pointer -pipe
	PYTHON_CXX_FLAGS = $(CXX_FLAGS)
	OPTIM = -g3
else
	CXX_FLAGS = -std=c++20 -Wall -Wextra -Wconversion -Wpedantic -Wshadow -fomit-frame-pointer -fno-stack-protector -fno-rtti -flto -DNDEBUG -pipe
	# CXX_FLAGS = -std=c++20 -Wall -Wextra -Wconversion -Wpedantic -Wshadow -fno-omit-frame-pointer -flto -pipe
	PYTHON_CXX_FLAGS = -std=c++20 -Wall -Wextra -Wconversion -Wpedantic -Wshadow -fomit-frame-pointer -fno-stack-protector -flto -DNDEBUG -pipe
	OPTIM = -O3 -ffast-math
endif

SOURCES :=                                 \
	src/buildinfo/capability.cc        \
	src/core/initializer.cc            \
	src/core/position.cc               \
	src/core/positionbuilder.cc        \
	src/core/state.cc                  \
	src/core/statebuilder.cc           \
	src/core/movegenerator.cc          \
	src/core/huffman.cc                \
	src/core/internal/bitboard.cc      \
	src/core/internal/huffmanimpl.cc   \
	src/core/internal/hash.cc          \
	src/core/internal/movegenerator.cc \
	src/core/internal/statehelper.cc   \
	src/core/internal/stateimpl.cc     \
	src/solver/mate1ply.cc             \
	src/solver/internal/mate1ply.cc    \
	src/solver/dfs.cc                  \
    src/solver/dfpn.cc                 \
    src/solver/internal/dfpn.cc        \
	src/ml/azteacher.cc                \
	src/ml/featurebitboard.cc          \
	src/ml/featurestack.cc             \
	src/ml/simpleteacher.cc            \
	src/ml/teacherloader.cc            \
	src/ml/teacherwriter.cc            \
    src/ml/utils.cc                    \
	src/io/huffman.cc                  \
	src/io/sfen.cc                     \
	src/io/csa.cc                      \
	src/io/file.cc                     \
    src/c_api/api.cc                   \
    src/c_api/move.cc                  \
    src/c_api/position.cc              \
    src/c_api/state.cc                 \
    src/c_api/solver.cc                \
    src/c_api/ml.cc                    \
    src/c_api/io.cc

TEST_SOURCES :=                         \
	src/test/test_main.cc           \
	src/test/test_types.cc 		\
	src/test/test_bitboard.cc       \
	src/test/test_position.cc       \
	src/test/test_state.cc          \
	src/test/test_movegeneration.cc \
	src/test/test_squareiterator.cc \
	src/test/test_utils.cc          \
	src/test/test_csa.cc		\
	src/test/test_sfen.cc           \
	src/test/test_huffman.cc     	\
	src/test/test_solver.cc		\
	src/test/test_ml.cc             \
    src/test/test_capi.cc

BENCH_SOURCES :=                          \
	src/bench/bench_main.cc           \
	src/bench/bench_movegeneration.cc \
	src/bench/bench_mate1ply.cc       \
	src/bench/bench_perft.cc

PYTHON_SOURCES :=          \
	src/python/bind.cc

OBJECTS = $(patsubst %.cc,$(OBJDIR)/%.o,$(SOURCES))
TEST_OBJECTS = $(patsubst %.cc,$(OBJDIR)/%.o,$(TEST_SOURCES))
BENCH_OBJECTS = $(patsubst %.cc,$(OBJDIR)/%.o,$(BENCH_SOURCES))
PYTHON_OBJECTS = $(patsubst %.cc,$(OBJDIR)/%.o,$(PYTHON_SOURCES))

DEPENDINGS = $(patsubst %.cc,$(OBJDIR)/%.d,$(SOURCES))

ARCH_FLAGS :=

ifeq ($(TUNENATIVE),1)
	ARCH_FLAGS += -march=native -mtune=native
else
	ifeq ($(SSE2), 1)
		ARCH_FLAGS += -msse2
		CXX_FLAGS += -DUSE_SSE2
	endif
	ifeq ($(SSE41),1)
		ARCH_FLAGS += -msse2 -msse4.1
		CXX_FLAGS += -DUSE_SSE2 -DUSE_SSE41
	endif
	ifeq ($(SSE42),1)
		ARCH_FLAGS += -msse2 -msse4.1 -msse4.2
		CXX_FLAGS += -DUSE_SSE2 -DUSE_SSE41 -DUSE_SSE42
	endif
	ifeq ($(AVX),1)
		ARCH_FLAGS += -msse2 -msse4.1 -msse4.2 -mbmi -mbmi2 -mavx
		CXX_FLAGS += -DUSE_SSE2 -DUSE_SSE41 -DUSE_SSE42 -DUSE_AVX
	endif
	ifeq ($(AVX2),1)
		ARCH_FLAGS += -msse2 -msse4.1 -msse4.2 -mbmi -mbmi2 -mavx -mavx2 -mlzcnt
		CXX_FLAGS += -DUSE_SSE2 -DUSE_SSE41 -DUSE_SSE42 -DUSE_BMI1 -DUSE_BMI2 -DUSE_AVX -DUSE_AVX2 -DUSE_LZCNT
	endif
	ifeq ($(NEON),1)
		ARCH_FLAGS += -march=armv8
		CXX_FLAGS += -DUSE_NEON
	endif
endif

PEXT ?= 1

ifeq ($(PEXT),1)
	CXX_FLAGS += -DUSE_PEXT
endif

ifneq ($(MAKECMDGOALS),clean)
    -include $(DEPENDINGS)
endif

$(OBJDIR)/%.d: %.cc
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) -MM -MT '$(patsubst %.d,%.o,$@)' $< -MF $@

$(OBJECTS): $(OBJDIR)/%.o: %.cc Makefile
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) -fPIC $(INCLUDES) $<

$(TEST_OBJECTS): $(OBJDIR)/%.o: %.cc Makefile
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(INCLUDES) $<

$(BENCH_OBJECTS): $(OBJDIR)/%.o: %.cc Makefile
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(INCLUDES) $<

$(PYTHON_OBJECTS): $(OBJDIR)/%.o: %.cc Makefile
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(OPTIM) $(ARCH_FLAGS) $(PYTHON_CXX_FLAGS) $(INCLUDES) $(PYTHON_INCLUDES) -fPIC $<

$(SHARED_TARGET): $(OBJECTS)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
ifeq ($(shell uname), Darwin)
	$(CXX) -dynamiclib -Wl,-install_name,@rpath/libnshogi.dylib -o $@ $(OBJECTS) $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) -fPIC $(LINKS)
	-rm -f $(OBJDIR)/lib/libnshogi.dylib
	cd $(OBJDIR)/lib && ln -s $(notdir $@) libnshogi.dylib
else
	$(CXX) -shared -Wl,-soname,libnshogi.so -o $@ $(OBJECTS) $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) -fPIC $(LINKS)
	-rm -f $(OBJDIR)/lib/libnshogi.so
	cd $(OBJDIR)/lib && ln -s $(notdir $@) libnshogi.so
endif

$(STATIC_TARGET): $(OBJECTS)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	ar rcs $@ $^

$(PYTHON_TARGET): $(PYTHON_OBJECTS) $(STATIC_TARGET)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
ifeq ($(shell uname), Darwin)
	$(CXX) -shared -o $@ $(PYTHON_OBJECTS) $(STATIC_TARGET) $(OPTIM) $(ARCH_FLAGS) $(PYTHON_CXX_FLAGS) -fPIC $(LINKS) $(PYTHON_LINKS)
else
	$(CXX) -shared -o $@ $(PYTHON_OBJECTS) -Wl,--whole-archive $(STATIC_TARGET) -Wl,--no-whole-archive $(OPTIM) $(ARCH_FLAGS) $(PYTHON_CXX_FLAGS) -fPIC $(LINKS) $(PYTHON_LINKS)
endif

$(TEST_STATIC_TARGET): $(TEST_OBJECTS) $(STATIC_TARGET)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
ifeq ($(shell uname), Darwin)
	$(CXX) -o $@ $(TEST_OBJECTS) $(STATIC_TARGET) $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(LINKS)
else
	$(CXX) -o $@ $(TEST_OBJECTS) -Wl,--whole-archive $(STATIC_TARGET) -Wl,--no-whole-archive $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(LINKS)
endif

$(TEST_SHARED_TARGET): $(TEST_OBJECTS) $(SHARED_TARGET)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -o $@ $(TEST_OBJECTS) -L$(PWD)/$(OBJDIR)/lib -Wl,-rpath,$(PWD)/$(OBJDIR)/lib $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(LINKS) -lnshogi

$(BENCH_TARGET): $(BENCH_OBJECTS) $(STATIC_TARGET)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) -o $@ $(BENCH_OBJECTS) $(STATIC_TARGET) $(OPTIM) $(ARCH_FLAGS) $(CXX_FLAGS) $(LINKS)

.PHONY: libnshogi
libnshogi: $(SHARED_TARGET) $(STATIC_TARGET)

.PHONY: install
install: $(SHARED_TARGET) $(STATIC_TARGET)
	-rm -r $(PREFIX)/include/nshogi
	-rm $(PREFIX)/lib/libnshogi.so
	install -d $(PREFIX)/lib
	install -m 644 $(SHARED_TARGET) $(PREFIX)/lib
	install -m 644 $(STATIC_TARGET) $(PREFIX)/lib
	ln -s $(PREFIX)/lib/$(SHARED_TARGET_NAME) $(PREFIX)/lib/$(SHARED_TARGET_NAME_NO_VERSION)
	install -d $(PREFIX)/include/nshogi/core
	install -d $(PREFIX)/include/nshogi/io
	install -d $(PREFIX)/include/nshogi/ml
	install -d $(PREFIX)/include/nshogi/solver
	install -m 644 src/core/*.h $(PREFIX)/include/nshogi/core
	install -m 644 src/io/*.h $(PREFIX)/include/nshogi/io
	install -m 644 src/ml/*.h $(PREFIX)/include/nshogi/ml
	install -m 644 src/solver/*.h $(PREFIX)/include/nshogi/solver
	install -m 644 src/c_api.h $(PREFIX)/include/nshogi/c_api.h

.PHONY: test-static
test-static: $(TEST_STATIC_TARGET)

.PHONY: test-shared
test-shared: $(TEST_SHARED_TARGET)

.PHONY: bench
bench: $(BENCH_TARGET)

.PHONY: runtest-static
runtest-static: test-static
	./$(TEST_STATIC_TARGET)

.PHONY: runtest-shared
runtest-shared: test-shared
	./$(TEST_SHARED_TARGET)

.PHONY: python
python: PYTHON_CXX_FLAGS += -DVERSION=\"$(VERSION)\"
python: $(PYTHON_TARGET)

.PHONY: install-python
install-python: python
	cp $(PYTHON_TARGET) $(shell python3 -c "import site; print(site.getsitepackages()[0])")

.PHONY: runtest-python
runtest-python: python
	PYTHONPATH=$(OBJDIR)/lib/ python3 src/test/python/test.py

.PHONY: runbench
runbench: bench
	perf record -a --call-graph dwarf -F 49 -- ./$(BENCH_TARGET)
	perf report > bench.txt

.PHONY: llvm-cov
llvm-cov: CXX = clang++
llvm-cov: CXX_FLAGS += -fprofile-instr-generate -fcoverage-mapping
llvm-cov: clean-cov-prof runtest-static
	llvm-profdata merge -sparse default.profraw -o default.profdata
	llvm-cov show -format=html -output-dir=cov_html -instr-profile default.profdata $(TEST_STATIC_TARGET)

.PHONY: fmt
fmt:
	find src/ \( -name "*.cc" -o -name "*.h" \) -exec clang-format -i {} \;

.PHONY: clean
clean: clean-perf clean-cov-prof
	-rm -rf build

.PHONY: clean-cov-prof
clean-cov-prof:
	-find build -type f -name "*.gcda" -exec rm -f {} +
	-rm -rf default.profraw
	-rm -rf default.profdata
	-rm -rf cov_html

.PHONY: clean-perf
clean-perf:
	-rm -f perf.data perf.data.old bench.txt
