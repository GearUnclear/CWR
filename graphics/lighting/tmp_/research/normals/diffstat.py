"""Mean / p99 absolute difference of each variant against Remastered, and a x4 diff heatmap."""
import sys
from pathlib import Path
import numpy as np
from PIL import Image
run = Path(sys.argv[1]); shots = run / "shots"
views = sorted({p.name.split("__")[0] for p in shots.glob("*.png")})
for v in views:
    base = shots / f"{v}__remastered.png"
    if not base.exists(): continue
    b = np.asarray(Image.open(base).convert("RGB")).astype(np.float32)
    for p in sorted(shots.glob(f"{v}__*.png")):
        lab = p.stem.split("__")[1]
        if lab in ("remastered",): continue
        a = np.asarray(Image.open(p).convert("RGB")).astype(np.float32)
        d = np.abs(a - b).mean(axis=2)
        print(f"{v:22s} {lab:12s} mean {d.mean():6.2f}  p99 {np.percentile(d,99):6.1f}")
        if len(sys.argv) > 2 and lab in sys.argv[2].split(","):
            Image.fromarray(np.clip(d * 4, 0, 255).astype(np.uint8)).save(run / f"{v}__diff_{lab}.png")
