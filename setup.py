#
# Copyright (c) 2025-2026 @nyashiki
#
# This software is licensed under the MIT license.
# For details, see the LICENSE file in the root of this repository.
#
# SPDX-License-Identifier: MIT
#

from __future__ import annotations

import os
import pathlib

from pybind11.setup_helpers import ParallelCompile, Pybind11Extension, build_ext
from setuptools import setup

ROOT = pathlib.Path(__file__).parent.resolve()

SOURCES = [
    "src/python/bind.cc",
    "src/python/core.cc",
    "src/python/solver.cc",
    "src/python/io.cc",
    "src/python/feature.cc",
    "src/python/teacher.cc",
    "src/buildinfo/capability.cc",
    "src/core/initializer.cc",
    "src/core/position.cc",
    "src/core/positionbuilder.cc",
    "src/core/state.cc",
    "src/core/extendedstate.cc",
    "src/core/statebuilder.cc",
    "src/core/movegenerator.cc",
    "src/core/huffman.cc",
    "src/core/internal/bitboard.cc",
    "src/core/internal/huffmanimpl.cc",
    "src/core/internal/hash.cc",
    "src/core/internal/movegenerator.cc",
    "src/core/internal/statehelper.cc",
    "src/core/internal/stateimpl.cc",
    "src/solver/mate1ply.cc",
    "src/solver/internal/mate1ply.cc",
    "src/solver/dfs.cc",
    "src/solver/dfpn.cc",
    "src/solver/internal/dfpn.cc",
    "src/ml/azteacher.cc",
    "src/ml/featurebitboard.cc",
    "src/ml/featurestack.cc",
    "src/ml/ka.cc",
    "src/ml/p.cc",
    "src/ml/simpleteacher.cc",
    "src/ml/teacheraggregator.cc",
    "src/ml/teacherloader.cc",
    "src/ml/teacherwriter.cc",
    "src/ml/batchedteacherloader.cc",
    "src/ml/utils.cc",
    "src/io/huffman.cc",
    "src/io/sfen.cc",
    "src/io/csa.cc",
    "src/io/file.cc",
    "src/c_api/api.cc",
    "src/c_api/move.cc",
    "src/c_api/position.cc",
    "src/c_api/state.cc",
    "src/c_api/solver.cc",
    "src/c_api/ml.cc",
    "src/c_api/io.cc",
]


def read_version() -> str:
    return (ROOT / "NSHOGI_VERSION").read_text(encoding="utf-8").strip()


def env_enabled(name: str) -> bool:
    return os.environ.get(name, "0") == "1"


def arch_options() -> tuple[list[str], list[tuple[str, str | None]]]:
    flags: list[str] = []
    defines: list[tuple[str, str | None]] = []

    if env_enabled("GENERIC"):
        flags += ["-march=x86-64", "-mtune=generic"]
    elif env_enabled("TUNENATIVE"):
        flags += ["-march=native", "-mtune=native"]
    else:
        if env_enabled("SSE41"):
            flags += ["-msse2", "-msse4.1"]
            defines += [("USE_SSE41", None)]
        if env_enabled("SSE42"):
            flags += ["-msse2", "-msse4.1", "-msse4.2"]
            defines += [("USE_SSE41", None), ("USE_SSE42", None)]
        if env_enabled("AVX"):
            flags += ["-msse2", "-msse4.1", "-msse4.2", "-mbmi", "-mbmi2", "-mavx"]
            defines += [("USE_SSE41", None), ("USE_SSE42", None), ("USE_AVX", None)]
        if env_enabled("AVX2"):
            flags += [
                "-msse2",
                "-msse4.1",
                "-msse4.2",
                "-mbmi",
                "-mbmi2",
                "-mavx",
                "-mavx2",
                "-mlzcnt",
            ]
            defines += [
                ("USE_SSE41", None),
                ("USE_SSE42", None),
                ("USE_BMI1", None),
                ("USE_BMI2", None),
                ("USE_AVX", None),
                ("USE_AVX2", None),
                ("USE_LZCNT", None),
            ]
        if env_enabled("NEON"):
            flags += ["-march=armv8"]
            defines += [("USE_NEON", None)]

    return flags, defines


def build_options() -> tuple[list[str], list[str], list[tuple[str, str | None]]]:
    arch_flags, defines = arch_options()

    warning_flags = ["-Wall", "-Wextra", "-Wconversion", "-Wpedantic", "-Wshadow"]

    if os.environ.get("BUILD", "release") == "debug":
        compile_flags = warning_flags + ["-g3", "-fno-omit-frame-pointer", "-pipe"]
    else:
        compile_flags = warning_flags + [
            "-O3",
            "-ffast-math",
            "-fomit-frame-pointer",
            "-fno-stack-protector",
            "-flto",
            "-pipe",
        ]
        defines += [("NDEBUG", None)]

    extra_compile_args = arch_flags + compile_flags
    extra_link_args = arch_flags + [
        flag for flag in compile_flags if flag not in warning_flags
    ]

    return extra_compile_args, extra_link_args, defines


extra_compile_args, extra_link_args, defines = build_options()

ParallelCompile("NSHOGI_NUM_BUILD_JOBS", default=0).install()

setup(
    name="nshogi",
    version=read_version(),
    description="nshogi Python library",
    author="nyashiki",
    license="MIT",
    license_files=("LICENSE", "LICENSE-THIRD-PARTY.md"),
    ext_modules=[
        Pybind11Extension(
            "nshogi",
            sources=SOURCES,
            cxx_std=20,
            define_macros=[("VERSION", f'"{read_version()}"')] + defines,
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
        ),
    ],
    cmdclass={
        "build_ext": build_ext,
    },
)
