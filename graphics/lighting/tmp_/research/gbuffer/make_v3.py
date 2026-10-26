"""Sheets, diff stats and timing rounds for run07_v3 (GI radius 15 m, bounce-only).

    python make_v3.py <run07_dir> <run06_dir>
"""
import statistics
import sys
from pathlib import Path

import numpy as np
from PIL import Image

import collect_gb as cg
import make_v2_sheets as m2

cg.PRETTY.update({
    "v3r15": "+ GI v2, r 15 m (x1)",
    "v3r15_x2_noao": "+ GI r 15 m x2, bounce only",
    "v3r15_x3s_noao": "+ GI r 15 m x3 whole-frame, bounce only",
    "v3dbg_bounce": "GI r 15 m bounce irradiance",
})


def main():
    run7, run6 = Path(sys.argv[1]), Path(sys.argv[2])
    keys = cg.collect(run7 / "tri", run7)
    rows7, other = cg.read_rows(run7 / "tri")
    rows6, _ = cg.read_rows(run6 / "tri")
    rows = {(run7.name, v, m): r for (v, m), r in rows7.items()}
    rows.update({(run6.name, v, m): r for (v, m), r in rows6.items()})
    views = sorted({v for v, l in keys})
    r7, r6 = str(run7), str(run6)
    for v in views:
        m2.sheet(v, [(r6, "classic"), (r7, "rm_vbo"), (r7, "v3r15_x2_noao"), (r7, "v3r15_x3s_noao")],
                 run7 / f"cmp_{v}__v3.png", rows, f"{v}: Classic | Remastered + late VBO | GI r 15 m bounce only x2 | x3 whole-frame")
        m2.sheet(v, [(r7, "rm_vbo"), (r7, "v2gi"), (r7, "v3r15"), (r7, "v3dbg_bounce")],
                 run7 / f"radius_{v}.png", rows, f"{v}: GI radius 6 m vs 15 m")
        ref = np.asarray(Image.open(run7 / f"{v}__rm_vbo.png").convert("RGB")).astype(np.float32)
        for l in ("v2gi", "v3r15", "v3r15_x2_noao", "v3r15_x3s_noao"):
            p = run7 / f"{v}__{l}.png"
            if not p.exists():
                continue
            a = np.asarray(Image.open(p).convert("RGB")).astype(np.float32)
            d = (a - ref).mean(axis=2)
            print(f"{v:18s} {l:16s} mean {d.mean():+.2f}  p99 {np.percentile(d, 99):+.1f}  p1 {np.percentile(d, 1):+.1f}  "
                  f"|d|>4 {(np.abs(d) > 4).mean() * 100:.1f}%  |d|>10 {(np.abs(d) > 10).mean() * 100:.1f}%")
            cg.diff(v, l, run7, ref="rm_vbo", gain=4)
    tr = {}
    for (v, m), r in rows7.items():
        if m.startswith("t") and m[1:2].isdigit() and "_" in m:
            tr.setdefault((v, m.split("_", 1)[1]), []).append(r)
    md = ["| view | mode | n | frame median | frame min | world median | world min | post median | ao/gi median |",
          "|---|---|---|---|---|---|---|---|---|"]
    for (v, mode), rs in sorted(tr.items()):
        fr = [r["frame"] for r in rs if r.get("frame") is not None]
        wo = [r["world"] for r in rs if r.get("world") is not None]
        po = [r["post"] for r in rs if r.get("post") is not None]
        ao = [r["ao"] for r in rs if r.get("ao") is not None]
        med = lambda a: f"{statistics.median(a):.2f}" if a else ""
        mn = lambda a: f"{min(a):.2f}" if a else ""
        md.append(f"| {v} | {mode} | {len(fr)} | {med(fr)} | {mn(fr)} | {med(wo)} | {mn(wo)} | {med(po)} | {med(ao)} |")
    (run7 / "timing_rounds.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    md2 = ["| view | shot | GPU frame | world | post | ao/gi |", "|---|---|---|---|---|---|"]
    for (v, m), r in sorted(rows7.items()):
        if m.startswith("t") and m[1:2].isdigit():
            continue
        f = lambda k: "" if r.get(k) is None else f"{r[k]:.2f}"
        md2.append(f"| {v} | {m} | {f('frame')} | {f('world')} | {f('post')} | {f('ao')} |")
    (run7 / "timings_gb.md").write_text("\n".join(md2) + "\n", encoding="utf-8")
    print("\n".join(md))
    print("\n".join(md2))


if __name__ == "__main__":
    main()
