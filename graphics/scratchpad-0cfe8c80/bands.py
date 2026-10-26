"""Mean colour per band for <view>__<a>.png vs <view>__<b>.png in a folder.

python bands.py <dir> <a> <b>
Bands: sky (top 35%), mid (35-55%), ground (55-100%). Prints mean RGB,
luma, R/B and G/R for each band and the luma ratio b/a.
"""
import sys
from pathlib import Path
from PIL import Image
import numpy as np

d, a, b = Path(sys.argv[1]), sys.argv[2], sys.argv[3]
BANDS = {"sky": (0.0, 0.35), "mid": (0.35, 0.55), "ground": (0.55, 1.0)}


def stats(p):
    im = np.asarray(Image.open(p).convert("RGB")).astype(np.float64)
    h = im.shape[0]
    out = {}
    for k, (t, u) in BANDS.items():
        px = im[int(h * t):int(h * u)].reshape(-1, 3)
        m = px.mean(0)
        luma = 0.2126 * m[0] + 0.7152 * m[1] + 0.0722 * m[2]
        out[k] = (m, luma)
    return out


print(f"{'view':34} band    {a:>18} {b:>18}  luma {a}/{b}   R/B {a}/{b}   G/R {a}/{b}")
for pa in sorted(d.glob(f"*__{a}.png")):
    view = pa.name[: -len(f"__{a}.png")]
    pb = d / f"{view}__{b}.png"
    if not pb.exists():
        continue
    sa, sb = stats(pa), stats(pb)
    for k in BANDS:
        (ma, la), (mb, lb) = sa[k], sb[k]
        fa = ",".join(f"{v:.0f}" for v in ma)
        fb = ",".join(f"{v:.0f}" for v in mb)
        print(f"{view:34} {k:7} {fa:>18} {fb:>18}  {la:6.1f}/{lb:6.1f} ({lb / max(la, 1e-6):.2f})"
              f"  {ma[0] / max(ma[2], 1e-6):.2f}/{mb[0] / max(mb[2], 1e-6):.2f}"
              f"  {ma[1] / max(ma[0], 1e-6):.2f}/{mb[1] / max(mb[0], 1e-6):.2f}")
