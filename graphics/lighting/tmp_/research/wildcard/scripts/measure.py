"""Brightness and change statistics between captures of one run.

    python measure.py <run_dir> <ref_label> <label> [<label> ...]

For every view that has <view>__<ref_label>.png and the other labels, prints the
mean display luminance (0-255) of each image, the mean signed and absolute
change against the reference, the share of pixels that change by more than 4
and 12 levels, and the p5/p50/p95 luminance. Writes the table to
<run_dir>/measure_<ref_label>.md.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image


def lum(p):
    a = np.asarray(Image.open(p).convert("RGB"), dtype=np.float32)
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def main():
    run = Path(sys.argv[1])
    ref = sys.argv[2]
    labels = sys.argv[3:]
    views = sorted({p.name.split("__")[0] for p in run.glob(f"*__{ref}.png")})
    out = ["| view | image | mean | p5 | p50 | p95 | mean change | mean abs change | >4 levels | >12 levels |",
           "|---|---|---|---|---|---|---|---|---|---|"]
    for v in views:
        r = lum(run / f"{v}__{ref}.png")
        out.append("| %s | %s | %.1f | %.0f | %.0f | %.0f | | | | |" % (
            v, ref, r.mean(), *np.percentile(r, [5, 50, 95])))
        for lab in labels:
            p = run / f"{v}__{lab}.png"
            if not p.exists():
                continue
            x = lum(p)
            d = x - r
            out.append("| %s | %s | %.1f | %.0f | %.0f | %.0f | %+.2f | %.2f | %.1f%% | %.1f%% |" % (
                v, lab, x.mean(), *np.percentile(x, [5, 50, 95]), d.mean(), np.abs(d).mean(),
                100 * (np.abs(d) > 4).mean(), 100 * (np.abs(d) > 12).mean()))
    text = "\n".join(out) + "\n"
    (run / f"measure_{ref}.md").write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
