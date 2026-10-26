"""Agent 4: 2x crops of the mist band (x1000-1480, y520-790) for two frames of each run, plus the frame diff x8."""
import sys
import numpy as np
from PIL import Image
runs = sys.argv[2:]
out = sys.argv[1]
x0, y0, x1, y1 = 1000, 520, 1480, 790
rows = []
for r in runs:
    name, d = r.split("=", 1)
    a = np.asarray(Image.open(d + "/valley_mist__ud__f0327.png").convert("RGB"), np.float32)[y0:y1, x0:x1]
    b = np.asarray(Image.open(d + "/valley_mist__ud__f0328.png").convert("RGB"), np.float32)[y0:y1, x0:x1]
    diff = np.clip(128 + 8 * (b - a).mean(axis=2, keepdims=True).repeat(3, axis=2), 0, 255)
    rows.append(np.concatenate([a, b, diff], axis=1))
    print(name, "mean diff f328-f327 in crop: %.2f levels, |diff| mean %.2f" % ((b - a).mean(), np.abs(b - a).mean()))
img = Image.fromarray(np.concatenate(rows, axis=0).astype(np.uint8))
img = img.resize((img.width * 2 // 2, img.height * 2 // 2))
img.save(out)
print(out, img.size)
