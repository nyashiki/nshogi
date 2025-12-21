from __future__ import annotations

import os
import pathlib
import shutil
import subprocess
import sysconfig

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

ROOT = pathlib.Path(__file__).parent.resolve()

class MakefileBuildExt(build_ext):
    def run(self):
        env = os.environ.copy()

        python_exe = sys.executable
        env["PYTHON"] = python_exe

        subprocess.check_call(["make", "GENERIC=1", "python"], cwd=str(ROOT), env=env)

        ext_suffix = sysconfig.get_config_var("EXT_SUFFIX") or ".so"

        build_cfg = env.get("BUILD", "release")
        cxx = env.get("CXX", "clang++")

        objdir = ROOT / "build" / f"{build_cfg}_{cxx}"
        libdir = objdir / "lib"
        built_ext = libdir / f"nshogi{ext_suffix}"

        if not built_ext.exists():
            raise RuntimeError(f"built extension not found: {built_ext}")

        for ext in self.extensions:
            if ext.name != "nshogi":
                continue

            dest_path = pathlib.Path(self.get_ext_fullpath(ext.name))
            dest_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(built_ext, dest_path)

def read_version() -> str:
    return (ROOT / "NSHOGI_VERSION").read_text(encoding="utf-8").strip()

setup(
    name="nshogi_dev",
    version=read_version(),
    description="nshogi Python library",
    author="nyashiki",
    license="MIT",
    license_files=("LICENSE", "LICENSE-THIRD-PARTY.md"),
    data_files=[("", ["NSHOGI_VERSION"])],
    ext_modules=[
        Extension("nshogi", sources=[], language="c++"),
    ],
    cmdclass={
        "build_ext": MakefileBuildExt,
    },
)
