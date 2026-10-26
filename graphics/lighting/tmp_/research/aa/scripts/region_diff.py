import sys
import numpy as np
from PIL import Image
a_run, b_run = sys.argv[1], sys.argv[2]
boxes = {"jeep+ground": (100, 380, 420, 560), "near ground": (0, 560, 960, 640), "forest": (560, 250, 960, 400)}
for lab in ["H-classic", "H-rem", "H-lz", "D-classic", "D-rem"]:
    A = np.asarray(Image.open(f"{a_run}/overcast_broken__{lab}.png").convert("RGB")).astype(np.float32)
    B = np.asarray(Image.open(f"{b_run}/overcast_broken__{lab}.png").convert("RGB")).astype(np.float32)
    parts = []
    for name, (x0, y0, x1, y1) in boxes.items():
        d = np.abs(A[y0:y1, x0:x1] - B[y0:y1, x0:x1]).mean(axis=2)
        parts.append(f"{name}: mean {d.mean():.3f} max {d.max():.0f} >2: {100 * (d > 2).mean():.2f}%")
    print(f"{lab:10s} " + " | ".join(parts))
