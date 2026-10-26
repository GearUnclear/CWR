"""Cloud-shadow share per view: debug view 1 (sun visibility) with cloud
shadows on vs off (dbg1 vs dbg1cs0), same build, same run. A pixel counts as
cloud-shadowed when it is at least 0.1 darker (display, 0-1) with the clouds
on. Ground pixels = pixels that are grey in dbg1cs0 (the sun-visibility debug
writes grey levels; the sky keeps its colour). Also frame luma of remastered
vs cs0 where both exist."""
import json
import pathlib
import sys

import numpy as np
from PIL import Image


def load(p):
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.float64) / 255.0


def luma(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def main(root):
    root = pathlib.Path(root)
    res = {}
    views = sorted({p.name.split("__")[0] for p in root.glob("*__dbg1.png")})
    for v in views:
        d1 = root / f"{v}__dbg1.png"
        d0 = root / f"{v}__dbg1cs0.png"
        r = {}
        if d1.exists() and d0.exists():
            a1, a0 = load(d1), load(d0)
            grey = (np.abs(a0[..., 0] - a0[..., 1]) < 0.02) & (np.abs(a0[..., 1] - a0[..., 2]) < 0.02)
            dark = (luma(a0) - luma(a1)) > 0.1
            r["ground_px_share"] = float(grey.mean())
            r["cloud_shadow_share_of_frame"] = float(dark.mean())
            r["cloud_shadow_share_of_ground"] = float((dark & grey).sum() / max(grey.sum(), 1))
        elif d1.exists():
            a1 = load(d1)
            grey = (np.abs(a1[..., 0] - a1[..., 1]) < 0.02) & (np.abs(a1[..., 1] - a1[..., 2]) < 0.02)
            l = luma(a1)
            r["ground_px_share"] = float(grey.mean())
            r["dbg1_dark_share_of_ground"] = float(((l < 0.5) & grey).sum() / max(grey.sum(), 1))
        rem = root / f"{v}__remastered.png"
        cs0 = root / f"{v}__cs0.png"
        if rem.exists():
            r["remastered_luma"] = float(luma(load(rem)).mean())
        if cs0.exists():
            r["cs0_luma"] = float(luma(load(cs0)).mean())
        res[v] = r
    print(json.dumps(res, indent=1))
    return res


if __name__ == "__main__":
    out = main(sys.argv[1])
    if len(sys.argv) > 2:
        pathlib.Path(sys.argv[2]).write_text(json.dumps(out, indent=1))
