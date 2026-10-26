import sys
from PIL import Image
import numpy as np
def L(p, box):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float32)[box[1]:box[3], box[0]:box[2]]
    lin = (a / 255.0) ** 2.2
    return (0.2126 * lin[..., 0] + 0.7152 * lin[..., 1] + 0.0722 * lin[..., 2]).mean()
boxes = {"near": (0, 900, 1920, 1080), "mid": (0, 650, 1920, 800)}
for view in sys.argv[1].split(","):
    row = [f"{view:16s}"]
    for name, box in boxes.items():
        v4 = L(f"../b4runs/v4-combat/{view}__on.png", box)
        v5 = L(f"../b4runs/v5-combat/{view}__on.png", box)
        off = L(f"../b4runs/v5-combat/{view}__off.png", box)
        row.append(f"{name}: v4 {v4:.4f} v5 {v5:.4f} v5/v4 {v5 / v4:.2f} off {off:.4f}")
    print("  ".join(row))
