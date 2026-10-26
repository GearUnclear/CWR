"""Motion check of the texsharp walk (b11walk: 0.25 m per frame, S mode, off pass
then on pass, TAA on in both).

    python motion.py <run folder>

Per band (far 36-48%, mid 48-62%, near 62-100% of the height, as the research's
shimmer.py) and pass:
  temporal  mean |f(i+1) - f(i)|                    (coherent motion and flicker)
  flicker   mean |f(i+1) - 2 f(i) + f(i-1)|         (what does not move smoothly)
  spatial   mean |Laplacian of f|                   (detail on screen)
and the ratios temporal / spatial (the research's shimmer index) and
flicker / spatial, on against off. Added detail that moves with the ground
raises temporal, flicker and spatial together; sparkle raises flicker faster.
Also writes motion_diff.png (the frame-to-frame difference x4, off over on, at
the middle of the walk) and motion.md.
"""
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image

BANDS = {"far": (0.36, 0.48), "mid": (0.48, 0.62), "near": (0.62, 1.0)}
FRAME_RE = re.compile(r"^(\d{3,})_(b11walk)__(off|on)__f(\d{4})\.png$")


def frames(run):
    found = {"off": {}, "on": {}}
    for p in Path(run, "tri").rglob("*.png"):
        m = FRAME_RE.match(p.name)
        if m:
            found[m.group(3)][int(m.group(4))] = p
    return found


def gray(p):
    a = np.asarray(Image.open(p).convert("RGB"), dtype=np.float32)
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def main():
    run = Path(sys.argv[1])
    f = frames(run)
    n = min(len(f["off"]), len(f["on"]))
    keys = sorted(set(f["off"]) & set(f["on"]))
    out = [f"# texsharp walk motion check ({run.name}), {n} frames per pass\n",
           "| band | pass | temporal | flicker | spatial | temporal/spatial | flicker/spatial |",
           "|---|---|---|---|---|---|---|"]
    stats = {}
    seqs = {p: [gray(f[p][k]) for k in keys] for p in ("off", "on")}
    h = seqs["off"][0].shape[0]
    for b, (y0, y1) in BANDS.items():
        a, z = int(y0 * h), int(y1 * h)
        for p in ("off", "on"):
            ims = [im[a:z] for im in seqs[p]]
            t = np.mean([np.abs(ims[i + 1] - ims[i]).mean() for i in range(len(ims) - 1)])
            fl = np.mean([np.abs(ims[i + 1] - 2 * ims[i] + ims[i - 1]).mean() for i in range(1, len(ims) - 1)])
            s = np.mean([np.abs(4 * im[1:-1, 1:-1] - im[:-2, 1:-1] - im[2:, 1:-1] - im[1:-1, :-2] - im[1:-1, 2:]).mean()
                         for im in ims])
            stats[(b, p)] = (t, fl, s)
            out.append(f"| {b} | {p} | {t:.2f} | {fl:.2f} | {s:.2f} | {t / s:.3f} | {fl / s:.3f} |")
    out.append("\n| band | temporal/spatial on vs off | flicker/spatial on vs off |")
    out.append("|---|---|---|")
    for b in BANDS:
        to, fo, so = stats[(b, "off")]
        tn, fn, sn = stats[(b, "on")]
        out.append(f"| {b} | {(tn / sn) / (to / so):.2f}x | {(fn / sn) / (fo / so):.2f}x |")
    mid = len(keys) // 2
    rows = []
    for p in ("off", "on"):
        d = np.abs(seqs[p][mid + 1] - seqs[p][mid]) * 4.0
        rows.append(np.clip(d, 0, 255).astype(np.uint8))
    img = Image.fromarray(np.vstack([rows[0], np.full((8, rows[0].shape[1]), 255, np.uint8), rows[1]]))
    img.save(run / "motion_diff.png")
    text = "\n".join(out) + "\n"
    (run / "motion.md").write_text(text, encoding="utf-8")
    print(text)


main()
