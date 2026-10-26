"""Build the comparison sheets of one combat research run (after collect.py).

    python make_sheets.py <run_dir> [--tag vN]

Three-way (Classic | Remastered | Remastered + combat lights) for every shot
label found in all three modes, plus per-mode sequence strips and the 4-way
(with the no-in-scatter variant y) where it exists.
"""
import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import collect as C  # noqa: E402

LABEL = re.compile(r"^([a-z]+)_([a-z])_(.+)$")
EXTRA = {"y": "combat lights, no in-scatter", "z": "variant z", "o": "combat lights as run 5",
         "w": "new flash + fire, no shadow", "s": "round 6 (smoke 1)"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--tag", default="")
    ap.add_argument("--only", default="", help="comma list of view prefixes")
    ap.add_argument("--xname", default="", help="panel title of mode x")
    a = ap.parse_args()
    run = Path(a.run)
    t = C.load_timings(run)
    shots = run / "shots"
    have = {}
    for p in shots.glob("*.png"):
        m = LABEL.match(p.stem)
        if m:
            have.setdefault((m.group(1), m.group(3)), {})[m.group(2)] = p
    only = set(a.only.split(",")) if a.only else None
    tag = f" ({a.tag})" if a.tag else ""
    names = dict(C.MODE_NAMES)
    names.update(EXTRA)
    if a.xname:
        names["x"] = a.xname
    for (view, suf), modes in sorted(have.items()):
        if only and view not in only:
            continue
        if all(m in modes for m in "crx"):
            panels = [C.panel(modes[m], names[m], C.gpu_text(t.get(f"{view}_{m}_{suf}")), 0.5) for m in "crx"]
            C.sheet(panels, f"{view} {suf}{tag}", 3, run / f"{view}_{suf}_3way.png")
        extra = [m for m in "oswyz" if m in modes]
        if extra and all(m in modes for m in "crx"):
            ms = list("cr") + extra + ["x"]
            panels = [C.panel(modes[m], names[m], C.gpu_text(t.get(f"{view}_{m}_{suf}")), 0.5) for m in ms]
            C.sheet(panels, f"{view} {suf}{tag}", 2, run / f"{view}_{suf}_{len(ms)}way.png")
    # per-mode strips
    views = sorted({v for (v, _s) in have})
    for view in views:
        if only and view not in only:
            continue
        sufs = sorted(s for (v, s) in have if v == view)
        for m in "crowsxyz":
            labels = [f"{view}_{m}_{s}" for s in sufs if (shots / f"{view}_{m}_{s}.png").exists()]
            if len(labels) < 2:
                continue
            panels = [C.panel(shots / f"{l}.png", l, C.gpu_text(t.get(l)), 0.33) for l in labels]
            C.sheet(panels, f"{view} {names.get(m, m)}{tag}: every shot", 4, run / f"{view}_{m}_seq.png")


if __name__ == "__main__":
    main()
