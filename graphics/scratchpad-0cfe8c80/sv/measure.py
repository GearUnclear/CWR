"""Box luma (0-255, display) for shaft-veil before/after captures."""
import sys, json
from pathlib import Path
from PIL import Image

BOXES = {
    "lowsun_fp": {
        "jeep_side_front": (45, 352, 100, 372),
        "jeep_side_rear": (150, 360, 195, 377),
        "m113_side": (690, 287, 790, 315),
        "m113_rear45": (607, 280, 640, 330),
        "tent": (300, 280, 490, 340),
        "soldier": (528, 345, 585, 420),
        "ground_lit": (625, 400, 825, 430),
        "ground_shadow": (240, 500, 310, 540),
        "far_hills": (820, 240, 950, 280),
        "sky_near_sun": (80, 60, 200, 140),
        "sky_top": (400, 10, 700, 60),
    },
    "sinai_lowsun_fp": {
        "hmmwv_side": (60, 292, 105, 322),
        "hmmwv_side2": (150, 292, 205, 322),
        "zelda_front": (603, 245, 632, 300),
        "zelda_side": (690, 245, 790, 290),
        "camnet": (330, 260, 480, 330),
        "ground_lit": (625, 385, 825, 410),
        "sky_top": (400, 10, 700, 60),
    },
}

def luma(c):
    return 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]

def mean(im, box):
    c = im.crop(box)
    px = list(c.getdata())
    n = len(px)
    return [sum(p[i] for p in px) / n for i in range(3)]

root = Path(sys.argv[1])
views = sys.argv[2:] or list(BOXES)
out = {}
for view in views:
    shots = {
        "before": root / "before" / f"{view}__remastered.png",
        "before_noshafts": root / "before" / f"{view}_noshafts__remastered.png",
        "after": root / "after" / f"{view}__remastered.png",
        "after_noshafts": root / "after" / f"{view}_noshafts__remastered.png",
        "classic_after_run": root / "after" / f"{view}__classic.png",
    }
    ims = {k: Image.open(p).convert("RGB") for k, p in shots.items() if p.exists()}
    print(f"\n## {view}\n")
    print("| box | " + " | ".join(ims) + " | veil before | veil after |")
    print("|---|" + "---|" * (len(ims) + 2))
    out[view] = {}
    for name, box in BOXES.get(view, {}).items():
        vals = {k: luma(mean(im, box)) for k, im in ims.items()}
        out[view][name] = vals
        vb = vals["before"] - vals["before_noshafts"]
        va = vals["after"] - vals["after_noshafts"]
        print(f"| {name} | " + " | ".join(f"{vals[k]:.1f}" for k in ims) + f" | {vb:+.1f} | {va:+.1f} |")
(root / "measure.json").write_text(json.dumps(out, indent=1))
