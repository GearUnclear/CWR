"""Check header invalidation after ccache hits with clang-cl and Ninja.

Run on Windows with cmake, ninja, clang-cl and ccache on PATH, or pass their
paths using --cmake, --ninja, --compiler and --ccache. The compile-only fixture
and private cache are retained under tmp/ for inspection; no engine build is
modified. This exercises the actual CMake dependency-tracking module.
"""

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option, program in (
        ("cmake", "cmake"),
        ("ninja", "ninja"),
        ("compiler", "clang-cl"),
        ("ccache", "ccache"),
    ):
        parser.add_argument(f"--{option}", default=program)
    args = parser.parse_args()
    programs = {}
    for option, value in vars(args).items():
        program = shutil.which(value)
        if not program:
            parser.error(f"{value} not found; supply --{option} with its path")
        programs[option] = program

    root = Path(__file__).resolve().parents[2]
    (root / "tmp").mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix="clang-cl-dependencies-", dir=root / "tmp"))
    print(f"Fixture: {work}", flush=True)
    env = {key: value for key, value in os.environ.items() if not key.startswith("CCACHE_")}
    (work / "ccache.conf").write_text("", encoding="utf-8")
    env.update(
        CCACHE_DIR=str(work / "cache"),
        CCACHE_TEMPDIR=str(work / "cache-tmp"),
        CCACHE_CONFIGPATH=str(work / "ccache.conf"),
        CCACHE_LOGFILE=str(work / "ccache.log"),
    )

    def run(*command):
        result = subprocess.run(command, cwd=work, env=env, capture_output=True, text=True)
        output = result.stdout + result.stderr
        with (work / "proof.log").open("a", encoding="utf-8") as log:
            log.write(f"\n{command!r}\n{output}")
        if result.returncode:
            raise RuntimeError(f"Command failed: {command!r}\n{output}")
        return output

    def stats():
        return {
            key: int(value)
            for key, value in (line.split() for line in run(programs["ccache"], "--print-stats").splitlines())
        }

    module = (root / "cmake/ClangClDependencyTracking.cmake").as_posix()
    (work / "CMakeLists.txt").write_text(
        f'''cmake_minimum_required(VERSION 3.25)
project(DependencyProbe LANGUAGES C CXX)
include("{module}")
add_library(probe OBJECT probe.c probe.cpp pch_probe.cpp)
target_precompile_headers(probe PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${{CMAKE_CURRENT_SOURCE_DIR}}/pch.hpp>")
set_source_files_properties(probe.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
''',
        encoding="utf-8",
    )
    header = work / "dep.hpp"
    header.write_text("#define DEP_VALUE 1\n", encoding="utf-8")
    (work / "pch.hpp").write_text("struct PchType { int value; };\n", encoding="utf-8")
    for source, function in (("probe.c", "cprobe"), ("probe.cpp", "probe"), ("pch_probe.cpp", "pch_probe")):
        (work / source).write_text(
            f'#include "dep.hpp"\nint {function}(void) {{ return DEP_VALUE; }}\n', encoding="utf-8"
        )

    run(
        programs["cmake"], "-S", ".", "-B", "build", "-G", "Ninja",
        f'-DCMAKE_MAKE_PROGRAM={programs["ninja"]}',
        f'-DCMAKE_C_COMPILER={programs["compiler"]}',
        f'-DCMAKE_CXX_COMPILER={programs["compiler"]}',
        f'-DCMAKE_C_COMPILER_LAUNCHER={programs["ccache"]}',
        f'-DCMAKE_CXX_COMPILER_LAUNCHER={programs["ccache"]}',
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY",
        "-DCMAKE_C_COMPILER_WORKS=TRUE", "-DCMAKE_CXX_COMPILER_WORKS=TRUE",
    )
    run(programs["ninja"], "-C", "build")
    objects = [work / "build/CMakeFiles/probe.dir" / f"{source}.obj" for source in (
        "probe.c", "probe.cpp", "pch_probe.cpp"
    )]
    hits_before = stats()["direct_cache_hit"]
    for obj in objects:
        obj.rename(obj.with_suffix(".before-cache"))
    run(programs["ninja"], "-C", "build")
    assert stats()["direct_cache_hit"] - hits_before >= 2, "C and skipped-PCH C++ did not hit the cache"
    for obj in objects:
        deps = run(programs["ninja"], "-C", "build", "-t", "deps", obj.relative_to(work / "build").as_posix())
        assert "dep.hpp" in deps, f"Header dependency lost on cache hit: {obj}\n{deps}"

    before = [hashlib.sha256(obj.read_bytes()).digest() for obj in objects]
    header.write_text("#define DEP_VALUE 2\n", encoding="utf-8")
    header_time = max(header.stat().st_mtime_ns, max(obj.stat().st_mtime_ns for obj in objects) + 10_000_000)
    os.utime(header, ns=(header_time, header_time))
    run(programs["ninja"], "-C", "build")
    for obj, previous in zip(objects, before):
        assert hashlib.sha256(obj.read_bytes()).digest() != previous, f"Header change left stale object: {obj}"
    assert "no work to do" in run(programs["ninja"], "-C", "build", "-n"), "Fixture did not settle after rebuild"
    print("PASS: cached C/C++ retain header dependencies; C, skipped-PCH C++ and PCH C++ rebuild after a header edit.")


if __name__ == "__main__":
    main()
