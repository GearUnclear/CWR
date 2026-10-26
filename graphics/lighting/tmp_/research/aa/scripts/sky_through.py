import sys
import numpy as np
from PIL import Image
run = sys.argv[1]
regions = {"forest_edge_golden": (560, 20, 960, 230), "overcast_broken": (560, 250, 960, 330)}
for view, box in regions.items():
    for lab in ["H-classic", "H-rem", "H-cov2", "H-cov3", "D-classic", "D-rem", "D-cov1", "D-cov3"]:
        im = np.asarray(Image.open(f"{run}/{view}__{lab}.png").convert("RGB").crop(box)).astype(np.float32)
        r, g, b = im[..., 0], im[..., 1], im[..., 2]
        lum = 0.2126 * r + 0.7152 * g + 0.0722 * b
        sky = (b > g) & (lum > 110)
        print(f"{view:20s} {lab:10s} sky-through fraction {sky.mean():.4f}  mean luma {lum.mean():.1f}")
