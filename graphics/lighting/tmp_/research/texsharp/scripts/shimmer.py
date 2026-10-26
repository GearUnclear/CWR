"""Shimmer index for the motion sequences (run_dir/motion/<view>__<var><i>.png).

Temporal change (mean |frame i+1 - frame i|) divided by spatial detail (mean
|Laplacian|) per band. Added texture that moves coherently raises both;
aliasing/shimmer raises the temporal change faster than the detail, so a
ratio above the stock ratio flags instability.

    python shimmer.py <run_dir>
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image

run = Path(sys.argv[1]) / "motion"
bands = {"far": (0.36, 0.48), "mid": (0.48, 0.62), "near": (0.62, 1.0)}
views = sorted({p.stem.split("__")[0] for p in run.glob("*.png")})
out = ["| view | variant | band | temporal | spatial | ratio | ratio vs stock |", "|---|---|---|---|---|---|---|"]
for v in views:
    base = {}
    for var in ("mstock", "mrdtile", "mself", "mbest"):
        ims = [np.asarray(Image.open(run / f"{v}__{var}{i}.png").convert("L"), dtype=np.float32) for i in range(8)
               if (run / f"{v}__{var}{i}.png").exists()]
        if len(ims) < 8:
            continue
        h = ims[0].shape[0]
        for b, (y0, y1) in bands.items():
            a, z = int(y0 * h), int(y1 * h)
            t = np.mean([np.abs(ims[i + 1][a:z] - ims[i][a:z]).mean() for i in range(7)])
            s = np.mean([np.abs(4 * im[a + 1:z - 1, 1:-1] - im[a:z - 2, 1:-1] - im[a + 2:z, 1:-1] - im[a + 1:z - 1, :-2]
                                - im[a + 1:z - 1, 2:]).mean() for im in ims])
            r = t / s
            if var == "mstock":
                base[b] = r
            out.append(f"| {v} | {var} | {b} | {t:.2f} | {s:.2f} | {r:.3f} | {r / base[b]:.2f} |")
text = "\n".join(out) + "\n"
(Path(sys.argv[1]) / "shimmer.md").write_text(text, encoding="utf-8")
print(text)
