import sys
import numpy as np
from PIL import Image
run = sys.argv[1]
box = (300, 150, 700, 420)
for lab in ["H-classic", "H-rem", "H-cov2", "H-cov3", "H-lz", "H-lzrcas05", "H-lzrcas10", "H-blrcas05", "H-compcas10", "H-best2", "H-best3"]:
    im = np.asarray(Image.open(f"{run}/town_night__{lab}.png").convert("RGB").crop(box)).astype(np.float32)
    print(f"town_night {lab:12s} house-region mean luma {(im @ np.array([0.2126, 0.7152, 0.0722])).mean():6.2f}")
