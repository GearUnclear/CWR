"""Build the temporal-track comparison images from one or more capture runs.

    python make_compare.py <run_dir> [<run_dir> ...] --out <dir>

Each run_dir is a run-captures.ps1 output folder (tri/ inside). Captures are
collected into <run_dir>/shots/<label>.png, timings into <run_dir>/timings.json.
Images are written into --out:
    <view>__3way.png            Classic | Remastered (4x MSAA) | Remastered + TAA (no MSAA), full frame
    <view>__aa_crops.png        2x crops: MSAA4@1.5, TAA@1.5, TAA@1.0, noAA@1.0, TAA+MSAA4@1.5
    <view>__shadow_stoch.png    stochastic contact-hardening shadow: no TAA (noisy) vs TAA (converged)
    <view>__ao_stoch.png        stochastic 4-tap AO without blur: no TAA vs TAA
    <view>__pan_strip.png       consecutive pan frames, MSAA row over TAA row (2x crop)
    <view>__pan.gif             the same pan as an animation, MSAA left, TAA right
    metrics.json                sharpness / temporal-difference numbers
"""

import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).parent))
import taalib as T  # noqa: E402

# Crop boxes in 960x640 output pixels: (x0, y0, x1, y1), 240x180 each.
CROPS = {
    "forest_edge_golden": {"crown": (700, 20, 940, 200), "birch": (220, 150, 460, 330), "shadow": (500, 360, 740, 540)},
    "sinai_lowsun_fp": {"net": (330, 170, 570, 350), "hmmwv": (60, 230, 300, 410), "shadow": (440, 330, 680, 510)},
    "town_night": {"crown": (330, 20, 570, 200), "house": (150, 160, 390, 340), "street": (560, 260, 800, 440)},
    "overcast_broken": {"treeline": (600, 280, 840, 460), "jeep": (110, 360, 350, 540), "ridge": (60, 260, 300, 440)},
}

MODE_TITLES = {
    "classic": "Classic (4x MSAA, 150%)",
    "remastered": "Remastered (4x MSAA, 150%)",
    "taa15": "Remastered + TAA (no MSAA, 150%)",
    "taa10": "Remastered + TAA (no MSAA, 100%)",
    "noaa10": "Remastered, no AA (100%)",
    "taa15msaa4": "Remastered + TAA + 4x MSAA (150%)",
    "shadowStoch_noTAA": "4-tap stochastic PCSS, no TAA",
    "shadowStoch_TAA": "4-tap stochastic PCSS + TAA",
    "shadowPcss12_noTAA": "12-tap PCSS, no TAA",
    "aoStoch_noTAA": "4-tap AO r2.5m, no blur, no TAA",
    "aoStoch_TAA": "4-tap AO r2.5m, no blur + TAA",
}


def load(shots, label):
    p = shots / f"{label}.png"
    return Image.open(p).convert("RGB") if p.exists() else None


def gray(img):
    return np.asarray(img.convert("L"), dtype=np.float32) / 255.0


def sharpness(img):
    g = gray(img)
    gx = np.abs(np.diff(g, axis=1)).mean()
    gy = np.abs(np.diff(g, axis=0)).mean()
    return float(gx + gy)


def register_shift(a, b, search=12):
    """Integer (dx, dy) minimising mean |a - shift(b)| over the overlap, plus that error."""
    best = (0, 0, 1e9)
    h, w = a.shape
    for dy in range(-3, 4):
        for dx in range(-search, search + 1):
            ya0, ya1 = max(0, dy), min(h, h + dy)
            xa0, xa1 = max(0, dx), min(w, w + dx)
            e = np.abs(a[ya0:ya1, xa0:xa1] - b[ya0 - dy:ya1 - dy, xa0 - dx:xa1 - dx]).mean()
            if e < best[2]:
                best = (dx, dy, float(e))
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    shots_dirs = []
    timing = {}
    for r in args.runs:
        r = Path(r)
        shots = r / "shots"
        T.collect(r / "tri", shots)
        rows, udcap = T.timings(r / "tri")
        T.save_json(r / "timings.json", rows)
        (r / "udcap.log").write_text("\n".join(udcap), encoding="utf-8")
        timing.update(rows)
        shots_dirs.append(shots)

    def find(label):
        for s in shots_dirs:
            im = load(s, label)
            if im is not None:
                return im
        return None

    metrics = {}
    for view, crops in CROPS.items():
        vm = metrics.setdefault(view, {})

        def sub(label):
            return T.gpu_sub(timing.get(f"{view}__{label}"))

        # 1. three-way full frame
        trio = [("classic", find(f"{view}__classic")), ("remastered", find(f"{view}__remastered")),
                ("taa15", find(f"{view}__taa15"))]
        if all(im is not None for _, im in trio):
            panels = [T.label_panel(im, MODE_TITLES[m], sub(m)) for m, im in trio]
            T.sheet(panels, 3, f"{view}: Classic | Remastered (current, 4x MSAA) | Remastered + TAA prototype (MSAA off)").save(out / f"{view}__3way.png")

        # 2. AA crops
        aa_modes = ["remastered", "taa15", "taa10", "noaa10", "taa15msaa4"]
        ims = {m: find(f"{view}__{m}") for m in aa_modes}
        if all(ims[m] is not None for m in ["remastered", "taa15"]):
            rows = []
            for cname, box in crops.items():
                for m in aa_modes:
                    if ims[m] is None:
                        continue
                    t = timing.get(f"{view}__{m}") or {}
                    ms = f"{t['frame']:.2f} ms" if t.get("frame") is not None else "n/a"
                    rows.append(T.label_panel(T.crop2x(ims[m], box), f"{MODE_TITLES[m]}", f"{cname} 2x | GPU {ms}", 0.8))
                    vm.setdefault("sharpness", {}).setdefault(cname, {})[m] = round(sharpness(ims[m].crop(box)), 5)
            ncol = sum(1 for m in aa_modes if ims[m] is not None)
            T.sheet(rows, ncol, f"{view}: anti-aliasing detail, 2x nearest crops (rows: {', '.join(crops)})").save(out / f"{view}__aa_crops.png")

        # 3. stochastic shadow + 4. AO
        for tag, modes in (("shadow_stoch", ["remastered", "shadowStoch_noTAA", "shadowStoch_TAA", "shadowPcss12_noTAA"]),
                           ("ao_stoch", ["remastered", "aoStoch_noTAA", "aoStoch_TAA"])):
            ims2 = {m: find(f"{view}__{m}") for m in modes}
            if any(ims2[m] is None for m in modes[1:]):
                continue
            full = [T.label_panel(ims2[m], MODE_TITLES[m], sub(m)) for m in modes if ims2[m] is not None]
            T.sheet(full, len(full), f"{view}: {tag}").save(out / f"{view}__{tag}_full.png")
            rows = []
            for cname, box in crops.items():
                for m in modes:
                    if ims2[m] is not None:
                        rows.append(T.label_panel(T.crop2x(ims2[m], box), MODE_TITLES[m], f"{cname} 2x", 0.8))
            T.sheet(rows, len(full), f"{view}: {tag}, 2x crops").save(out / f"{view}__{tag}.png")

        # 5. pans
        for kind in ("pan", "strafe"):
            fr_m = [find(f"{view}__{kind}msaa_f{i:02d}") for i in range(6)]
            fr_t = [find(f"{view}__{kind}taa_f{i:02d}") for i in range(6)]
            if any(f is None for f in fr_m + fr_t):
                continue
            box = list(crops.values())[0]
            strip = [T.label_panel(T.crop2x(f, box), f"MSAA4 f{i}", "", 0.7) for i, f in enumerate(fr_m)]
            strip += [T.label_panel(T.crop2x(f, box), f"TAA f{i}", "", 0.7) for i, f in enumerate(fr_t)]
            T.sheet(strip, 6, f"{view}: {kind}, consecutive captures (2 moving frames apart), top 4x MSAA, bottom TAA").save(out / f"{view}__{kind}_strip.png")
            gif = []
            for a, b in zip(fr_m, fr_t):
                fa = T.label_panel(a.crop((0, 0, 960, 640)).resize((480, 320), Image.LANCZOS), "4x MSAA")
                fb = T.label_panel(b.crop((0, 0, 960, 640)).resize((480, 320), Image.LANCZOS), "TAA")
                gif.append(T.sheet([fa, fb], 2))
            gif[0].save(out / f"{view}__{kind}.gif", save_all=True, append_images=gif[1:], duration=250, loop=0)
            # crop GIF at 2x for flicker judgement
            gif2 = []
            for a, b in zip(fr_m, fr_t):
                gif2.append(T.sheet([T.label_panel(T.crop2x(a, box), "4x MSAA", "", 0.8), T.label_panel(T.crop2x(b, box), "TAA", "", 0.8)], 2))
            gif2[0].save(out / f"{view}__{kind}_crop.gif", save_all=True, append_images=gif2[1:], duration=200, loop=0)
            # temporal difference after integer registration (centre region)
            for name, frs in (("msaa", fr_m), ("taa", fr_t)):
                errs = []
                for a, b in zip(frs[:-1], frs[1:]):
                    ga = gray(a)[120:520, 160:800]
                    gb = gray(b)[120:520, 160:800]
                    dx, dy, e = register_shift(ga, gb)
                    errs.append(e)
                vm.setdefault(kind, {})[name] = {"mean_reg_residual": round(float(np.mean(errs)), 5),
                                                 "sharpness_centre": round(float(np.mean([sharpness(f.crop((160, 120, 800, 520))) for f in frs])), 5)}

        # static-camera TAA flicker (two consecutive captures, same camera)
        a = find(f"{view}__taa15")
        b = find(f"{view}__taa15_b")
        if a is not None and b is not None:
            vm["static_taa_flicker_mean_abs"] = round(float(np.abs(gray(a) - gray(b)).mean()), 5)

    T.save_json(out / "metrics.json", metrics)
    print(json.dumps(metrics, indent=1))


if __name__ == "__main__":
    main()
