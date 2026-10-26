"""Yaw proxy: x centroid of skin pixels (R-G>22, R-B>35, R>80) in each head box, per frame.
Snap = |step| >= 2 px; flips = sign changes between consecutive snaps. Also the skin pixel count."""
import glob, json, os, re, sys
import numpy as np
from PIL import Image
src, boxes_path = sys.argv[1], sys.argv[2]
boxes = json.load(open(boxes_path))
files = {}
for f in glob.glob(os.path.join(src, "**", "*__f[0-9][0-9][0-9][0-9].png"), recursive=True):
    files.setdefault(int(re.search(r"__f(\d{4})", f).group(1)), f)
ks = sorted(files)
cx = {n: [] for n in boxes}; cnt = {n: [] for n in boxes}
for k in ks:
    a = np.asarray(Image.open(files[k]).convert("RGB"), dtype=np.int32)
    for n, b in boxes.items():
        c = a[b[1]:b[3], b[0]:b[2]]
        m = (c[..., 0] - c[..., 1] > 22) & (c[..., 0] - c[..., 2] > 35) & (c[..., 0] > 80)
        ys, xs = np.nonzero(m)
        cnt[n].append(len(xs)); cx[n].append(xs.mean() if len(xs) > 5 else np.nan)
out = []
for n in boxes:
    x = np.array(cx[n]); s = np.diff(x)
    s = np.where(np.isnan(s), 0, s)
    sn = np.nonzero(np.abs(s) >= 2)[0]
    sg = np.sign(s[sn]); fl = int((sg[1:] != sg[:-1]).sum()) if len(sg) > 1 else 0
    sn60 = sn[sn < 59]; sg60 = np.sign(s[sn60]); fl60 = int((sg60[1:] != sg60[:-1]).sum()) if len(sg60) > 1 else 0
    line = f"{n:11s} skinpx {int(np.median(cnt[n])):4d} xrange {np.nanmin(x):5.1f}-{np.nanmax(x):5.1f} snaps {len(sn):3d} flips {fl:3d} | first60 snaps {len(sn60):2d} flips {fl60:2d}"
    out.append(line); print(line)
if len(sys.argv) > 3:
    open(sys.argv[3], "w").write("\n".join(out) + "\n")
