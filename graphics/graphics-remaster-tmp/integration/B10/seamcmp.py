"""The sun-up view (Eden 14:30, sun 54 deg, looking at the sun) under each
switch combination: lit clouds alone (on against nolit, noshadow against off)
in a run, and each combination against the same one of another run.

    python seamcmp.py <run dir> <reference run dir> [--md out.md]
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("run")
ap.add_argument("ref")
ap.add_argument("--md", default="")
a = ap.parse_args()


def shots(run):
    return {p.stem.split("__")[1]: p for p in (Path(run) / "tri" / "b10_seam").glob("*.png")}


def img(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


def cmp(pa, pb):
    d = np.abs(img(pa) - img(pb)).max(axis=2)
    return f"{int(d.max())} / {int((d > 2).sum()):,} / {int((d > 8).sum()):,}"


new, old = shots(a.run), shots(a.ref)
lines = ["max channel difference / pixels over 2 levels / pixels over 8 levels, whole frame", "",
         f"| comparison | {Path(a.run).name} | {Path(a.ref).name} |", "|---|---|---|"]
for x, y in (("on", "nolit"), ("noshadow", "off")):
    lines.append(f"| lit clouds alone: {x} against {y} | {cmp(new[x], new[y])} | {cmp(old[x], old[y])} |")
for k in ("on", "off", "nolit", "noshadow", "noatmo", "classic"):
    if k in old:
        lines.append(f"| {k}: {Path(a.run).name} against {Path(a.ref).name} | {cmp(new[k], old[k])} | |")
text = "\n".join(lines)
print(text)
if a.md:
    Path(a.md).write_text(text + "\n", encoding="utf-8")
