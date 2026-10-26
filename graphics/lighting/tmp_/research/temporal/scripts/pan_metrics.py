"""Temporal-stability proxy for the pans: consecutive captures (0.1 deg of yaw
apart) are aligned by the best integer shift; the mean absolute residual over
the centre region is what changed beyond the shift (shimmer, crawl, plus
sub-pixel misregistration, which both modes share). Sharpness = mean absolute
neighbour difference of the same region (blur lowers both numbers, so read
them together). Also the static flicker of two consecutive TAA captures with
a still camera."""

import json
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402
from make_compare import gray, register_shift, sharpness  # noqa: E402

PANS = {
    "forest_edge_golden": [("MSAA4 150%", "run10-abel-r2", "panmsaa"), ("TAA 150%", "run10-abel-r2", "panbest15"),
                           ("TAA+MSAA4 100%", "run10-abel-r2", "panbest10msaa4")],
    "town_night": [("MSAA4 150%", "run10-abel-r2", "panmsaa"), ("TAA 150%", "run10-abel-r2", "panbest15"),
                   ("TAA+MSAA4 100%", "run10-abel-r2", "panbest10msaa4")],
    "sinai_lowsun_fp": [("MSAA4 150%", "run09-sinai-r3", "panmsaa"), ("TAA 150%", "run07-sinai-r2", "panbest15"),
                        ("TAA+MSAA4 100%", "run09-sinai-r3", "panbest10msaa4")],
    "overcast_broken": [("MSAA4 150%", "run08-eden-r2", "panmsaa"), ("TAA 150%", "run08-eden-r2", "panbest15"),
                        ("TAA+MSAA4 100%", "run11-eden-r3", "panbest10msaa4")],
}
STATIC = {"forest_edge_golden": [("run10-abel-r2", "best15"), ("run10-abel-r2", "best10msaa4")],
          "sinai_lowsun_fp": [("run07-sinai-r2", "best15")]}

out = {}
for v, modes in PANS.items():
    for name, run, tag in modes:
        frs = [S.shot(run, f"{v}__{tag}_f{i:02d}") for i in range(6)]
        if any(f is None for f in frs):
            print("missing", v, tag)
            continue
        errs = []
        for a, b in zip(frs[:-1], frs[1:]):
            ga = gray(a)[120:520, 160:800]
            gb = gray(b)[120:520, 160:800]
            errs.append(register_shift(ga, gb)[2])
        sh = float(np.mean([sharpness(f.crop((160, 120, 800, 520))) for f in frs]))
        out.setdefault(v, {})[name] = {"pan_residual": round(float(np.mean(errs)), 5), "sharpness": round(sh, 5),
                                       "residual_per_sharpness": round(float(np.mean(errs)) / sh, 4)}
for v, items in STATIC.items():
    for run, lab in items:
        a, b = S.shot(run, f"{v}__{lab}"), S.shot(run, f"{v}__{lab}_b")
        if a is not None and b is not None:
            out.setdefault(v, {})[f"static flicker {lab}"] = round(float(np.abs(gray(a) - gray(b)).mean()), 5)
(S.ROOT / "final" / "pan_metrics.json").write_text(json.dumps(out, indent=1), encoding="utf-8")
print(json.dumps(out, indent=1))
