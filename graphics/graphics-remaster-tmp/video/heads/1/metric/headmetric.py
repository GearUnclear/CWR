"""Head wobble metric for agent 1 probes.

usage: headmetric.py <frames dir (searched recursively for *__<side>__f*.png)> <boxes json> <out prefix>
boxes json: {"name": [x0, y0, x1, y1], ...} in full-frame pixels.
Per box: frame-to-frame mean absolute difference of the grey crop (0-255),
mean, p95, max, and the number of frames over 3x the box median ("jumps").
Writes <out prefix>_strip_<name>.png (every 4th frame side by side, 2x) and
<out prefix>_metric.txt.
"""
import glob, json, os, re, sys
import numpy as np
from PIL import Image

src, boxes_path, outp = sys.argv[1], sys.argv[2], sys.argv[3]
boxes = json.load(open(boxes_path))
files = sorted(glob.glob(os.path.join(src, "**", "*__f[0-9][0-9][0-9][0-9].png"), recursive=True),
               key=lambda p: int(re.search(r"__f(\d{4})", p).group(1)))
# de-dup by frame number (tri tree vs collected copies)
seen, fl = set(), []
for f in files:
    k = int(re.search(r"__f(\d{4})", f).group(1))
    if k not in seen:
        seen.add(k); fl.append(f)
files = fl
print("frames", len(files))
crops = {n: [] for n in boxes}
for f in files:
    im = Image.open(f).convert("RGB")
    for n, b in boxes.items():
        crops[n].append(np.asarray(im.crop(tuple(b)), dtype=np.float32))
lines = [f"src {src}", f"frames {len(files)}"]
for n, cs in crops.items():
    g = [c.mean(axis=2) for c in cs]
    d = np.array([np.abs(g[i] - g[i - 1]).mean() for i in range(1, len(g))])
    med = float(np.median(d)) if len(d) else 0
    jumps = int((d > max(3 * med, 1.0)).sum())
    lines.append(f"{n:10s} mean {d.mean():6.2f} p95 {np.percentile(d,95):6.2f} max {d.max():6.2f} median {med:5.2f} jumps {jumps}")
    strip = [Image.fromarray(cs[i].astype(np.uint8)) for i in range(0, len(cs), 4)]
    w, h = strip[0].size
    sheet = Image.new("RGB", (w * len(strip), h))
    for i, s in enumerate(strip):
        sheet.paste(s, (i * w, 0))
    sheet = sheet.resize((sheet.width * 2, sheet.height * 2), Image.NEAREST)
    sheet.save(f"{outp}_strip_{n}.png")
    np.savetxt(f"{outp}_series_{n}.txt", d, fmt="%.3f")
open(f"{outp}_metric.txt", "w").write("\n".join(lines) + "\n")
print("\n".join(lines))
