"""Builds a shader override dir from the dumped programs plus text patches.

  python mkovr.py <dump dir> <out dir> <patch.py>

patch.py defines PATCHES = {"psRemasterCommon": [(old, new), ...], ...}; every
`old` must occur exactly once. Output is LF.
"""
import runpy
import sys
from pathlib import Path


def main():
    dump, out, patch = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
    patches = runpy.run_path(patch)["PATCHES"]
    out.mkdir(parents=True, exist_ok=True)
    for name, reps in patches.items():
        src = (dump / f"{name}.glsl").read_text(encoding="utf-8").replace("\r\n", "\n")
        for old, new in reps:
            n = src.count(old)
            if n != 1:
                raise SystemExit(f"{name}: pattern found {n} times:\n{old[:200]}")
            src = src.replace(old, new)
        (out / f"{name}.glsl").write_text(src, encoding="utf-8", newline="\n")
        print("wrote", out / f"{name}.glsl")


if __name__ == "__main__":
    main()
