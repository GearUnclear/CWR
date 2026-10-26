"""Agent 2 wobble metric. wobble.py <frames dir> <label glob> <out prefix> x0,y0,x1,y1 [...]
Per box (a head crop): frame-to-frame mean abs diff (MAD, 0-255 luma) and the
integer-pixel shift that best aligns the crop to the previous frame (search
+-12 px), plus a filmstrip of the crops for looking."""
import sys, glob, re
import numpy as np
from PIL import Image
d, pat, outp = sys.argv[1], sys.argv[2], sys.argv[3]
boxes = [tuple(int(v) for v in b.split(",")) for b in sys.argv[4:]]
files = sorted(glob.glob(f"{d}/**/*{pat}*.png", recursive=True), key=lambda f: re.search(r"__f(\d+)", f).group(1))
print(len(files), "frames")
R = 12
def luma(f):
    return np.asarray(Image.open(f).convert("L"), dtype=np.float32)
prev = None
stats = [[] for _ in boxes]
strips = [[] for _ in boxes]
for i, f in enumerate(files):
    im = luma(f)
    rgb = Image.open(f).convert("RGB")
    for bi, (x0, y0, x1, y1) in enumerate(boxes):
        if i < 60:
            strips[bi].append(rgb.crop((x0, y0, x1, y1)))
        if prev is not None:
            cur = im[y0:y1, x0:x1]
            best = (1e9, 0, 0)
            for dy in range(-R, R + 1, 1):
                for dx in range(-R, R + 1, 1):
                    p = prev[y0 + dy:y1 + dy, x0 + dx:x1 + dx]
                    e = float(np.mean(np.abs(cur - p)))
                    if e < best[0]:
                        best = (e, dx, dy)
            mad = float(np.mean(np.abs(cur - prev[y0:y1, x0:x1])))
            stats[bi].append((mad, best[1], best[2], best[0]))
    prev = im
with open(outp + "_metric.txt", "w") as fo:
    for bi, s in enumerate(stats):
        a = np.array(s)
        sh = np.hypot(a[:, 1], a[:, 2])
        line = (f"box{bi} {boxes[bi]}: MAD mean {a[:,0].mean():.2f} p95 {np.percentile(a[:,0],95):.2f} max {a[:,0].max():.2f} | "
                f"shift mean {sh.mean():.2f}px max {sh.max():.1f}px frames>=1px {int((sh>=1).sum())}/{len(sh)} | residual {a[:,3].mean():.2f}")
        print(line); fo.write(line + "\n")
        fo.write("  per-frame MAD: " + " ".join(f"{v:.1f}" for v in a[:, 0]) + "\n")
        fo.write("  per-frame shift: " + " ".join(f"{int(x)},{int(y)}" for x, y in a[:, 1:3]) + "\n")
# filmstrip: one row per box, first 60 frames (4x upscale is too big; 2x)
for bi, st in enumerate(strips):
    w, h = st[0].size
    cols = 20
    rows = (len(st) + cols - 1) // cols
    sheet = Image.new("RGB", (w * 2 * cols, h * 2 * rows))
    for j, c in enumerate(st):
        sheet.paste(c.resize((w * 2, h * 2), Image.NEAREST), ((j % cols) * w * 2, (j // cols) * h * 2))
    sheet.save(f"{outp}_strip{bi}.png")
