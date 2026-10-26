"""Collect the look-ref reference shots into GR/tmp/look/ref/{vanilla,landed}/<view>.png,
write index.json and contact.png.

Raw tri trees: GR/tmp/look/ref/raw/<run>/tri/**/NNN_<label>.png
Vanilla shots get the vanilla engine's gamma pass offline (triScreenshot in
vanilla reads the frame before ApplyGammaPass; the pass is pow(c, 1/gamma)).
"""
import argparse
import json
import re
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

GR = Path("D:/Arma_CWA_worktrees/graphics-remaster")
REF = GR / "tmp/look/ref"
FINAL = GR / "tmp/look/director/final"

V960 = {"lowsun_fp": "Abel", "sunrise_ridge": "Abel", "noon_fields": "Abel", "forest_edge_golden": "Abel",
        "coastline_sunset": "Abel", "town_night": "Abel", "firefight_1": "Abel", "firefight_2": "Abel",
        "firefight_3": "Abel", "tank_gunner_interior": "Abel", "tank_gunner_optics": "Abel",
        "overcast_broken": "Eden", "overcast_heavy": "Eden", "sinai_lowsun_fp": "Sinai",
        "sinai_desert_noon": "Sinai"}
SRC960 = {"Abel": "abel_lighting.test.sqf", "Eden": "eden_lighting.test.sqf", "Sinai": "sinai_lobo_lighting.test.sqf"}
# The fork's look_director_optics shots are taken at the next present (triCaptureAtPresent) from inside
# a forEach block, so each lands one frame after its statement: its "hud" shot is the first GUNNER
# frame after the switch, "f1" the second. Vanilla's triScreenshot reads the last rendered frame at
# once, so its "hud" is the on-foot view and "f1"/"f2" are the first/second GUNNER frames. Each side
# maps the shot that shows the same frame as the final set's file.
OPTICS = {"landed": {"optics__hud_0": "optics_mist_hud", "optics__gun_0_f30": "optics_mist_sight",
                     "optics__gun_0_f1": "optics_mist_sight_f1"},
          "vanilla": {"optics__gun_0_f1": "optics_mist_hud", "optics__gun_0_f30": "optics_mist_sight",
                      "optics__gun_0_f2": "optics_mist_sight_f1"}}
EXTRA = {"vanilla": {"optics__hud_0": "vanilla_optics_mist_hud_onfoot"}}
SHOT = re.compile(r"^(\d{3})_(.+)\.png$")


def find_shots(raw_dirs, side_label):
    """label -> (path, test) for every PNG in the raw tri trees (latest run wins)."""
    out = {}
    for d in raw_dirs:
        for p in sorted(d.rglob("*.png")):
            m = SHOT.match(p.name)
            if not m:
                continue
            test = next((part for part in p.parts if part.startswith("ref_")), "?")
            out[(m.group(2), test)] = p
    return out


def view_names(label, test, mode, side):
    """Final-set view names a shot stands for."""
    if test.endswith("_optics"):
        return [OPTICS[side][label]] if label in OPTICS[side] else []
    if not label.endswith(f"__{mode}"):
        return []
    v = label[: -len(mode) - 2]
    if "_960_" in test:
        return [v]
    names = [f"{v}__1080"]
    if v not in V960:
        names.append(v)
    return names


def world_of(test):
    for w in ("abel", "eden", "sinai"):
        if f"_{w}_" in test:
            return w.capitalize()
    return "Eden"  # optics


def source_test(test, view):
    if test.endswith("_optics"):
        return "look_director_optics.test.sqf"
    w = world_of(test)
    if "_960_" in test:
        return SRC960[w]
    return f"look_final_{w.lower()}.test.sqf"


def gamma(img, g):
    a = np.asarray(img.convert("RGB")).astype(np.float32) / 255.0
    return Image.fromarray(np.clip(np.round(255.0 * np.power(a, 1.0 / g)), 0, 255).astype(np.uint8))


def make_sheet(rows, keys, title, W=300):
    font = ImageFont.load_default()
    cells = []
    for r in rows:
        ims = []
        for key in keys:
            pth = r.get(key)
            if pth and Path(pth).exists():
                im = Image.open(pth).convert("RGB")
                ims.append(im.resize((W, int(im.size[1] * W / im.size[0])), Image.BILINEAR))
            else:
                ims.append(None)
        h = max((i.size[1] for i in ims if i), default=169)
        cells.append((r["view"], ims, h))
    pad, lab = 4, 14
    total_h = sum(h + lab + pad for _, _, h in cells) + 20
    sheet = Image.new("RGB", (len(keys) * W + (len(keys) + 1) * pad, total_h), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((pad, 4), title, fill=(230, 230, 230), font=font)
    y = 20
    for name, ims, h in cells:
        d.text((pad, y), name, fill=(255, 220, 120), font=font)
        y += lab
        for k, im in enumerate(ims):
            x = pad + k * (W + pad)
            if im:
                sheet.paste(im, (x, y))
            else:
                d.rectangle([x, y, x + W - 1, y + h - 1], outline=(200, 60, 60))
                d.text((x + 6, y + 6), "missing", fill=(255, 90, 90), font=font)
        y += h + pad
    return sheet


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gamma", type=float, default=1.2)
    a = ap.parse_args()
    raw = REF / "raw"
    sides = {"vanilla": ("vanilla", sorted(p for p in raw.glob("vanilla*") if p.is_dir())),
             "landed": ("landed", [raw / "landed"])}
    index = {}
    for side, (mode, dirs) in sides.items():
        dst = REF / side
        dst.mkdir(parents=True, exist_ok=True)
        shots = find_shots([d / "tri" for d in dirs if (d / "tri").exists()], side)
        for (label, test), p in shots.items():
            m = "remastered" if (side == "landed" and "_960_" in test) else mode
            if label in EXTRA.get(side, {}) and test.endswith("_optics"):
                (REF / "extra").mkdir(exist_ok=True)
                im = Image.open(p).convert("RGB")
                (gamma(im, a.gamma) if side == "vanilla" else im).save(REF / "extra" / f"{EXTRA[side][label]}.png")
            for name in view_names(label, test, m, side):
                img = Image.open(p).convert("RGB")
                if side == "vanilla":
                    img = gamma(img, a.gamma)
                out = dst / f"{name}.png"
                img.save(out)
                e = index.setdefault(name, {"view": name})
                e["island"] = world_of(test)
                e["test"] = source_test(test, name)
                e[f"{side}_test"] = test + ".test.sqf"
                e["resolution"] = f"{img.size[0]}x{img.size[1]}"
                e[f"{side}_raw"] = str(p).replace("\\", "/")
                e[side] = str(out).replace("\\", "/")
    finals = sorted({f.name.split("__")[0] + ("__1080" if "__1080__" in f.name else "")
                     for f in FINAL.glob("*__overhaul.png")})
    for n in finals:
        e = index.setdefault(n, {"view": n})
        e["overhaul_before"] = str(FINAL / f"{n}__overhaul.png").replace("\\", "/")
        for side in ("vanilla", "landed"):
            e.setdefault(side, None)
    rows = [index[k] for k in sorted(index)]
    meta = {
        "vanilla_engine": "Bohemia upstream/main 3.05 (ffc6183), D:/Arma_CWA_worktrees/vanilla-upstream, build vanilla-upstream-rwdi",
        "landed_engine": "graphics-remaster fec1075, dist/graphics-remaster-rwdi, triSetLighting \"remastered\"",
        "vanilla_gamma_offline": a.gamma,
        "data": "D:/Arma_CWA/tmp/remaster-shadow-67 (vanilla Sinai: its @LoBo without Bin)",
        "notes": "GR/tmp/look/ref/NOTES.md; views shot only at 1080p appear as <view> and <view>__1080 (the same shot), as in director/final",
    }
    (REF / "index.json").write_text(json.dumps({"meta": meta, "views": rows}, indent=1), encoding="utf-8")
    missing = [(r["view"], s) for r in rows for s in ("vanilla", "landed") if not r.get(s)]
    extra = [r["view"] for r in rows if r["view"] not in finals]
    print(f"views {len(rows)}; final names {len(finals)}; missing {missing}; not in final {extra}")
    for fname, keys, title in (
            ("contact.png", ("vanilla", "landed"), "vanilla upstream 3.05 | landed fec1075 remastered"),
            ("contact_framing.png", ("vanilla", "landed", "overhaul_before"),
             "vanilla upstream 3.05 | landed fec1075 remastered | look-director overhaul (framing check)")):
        sheet = make_sheet(rows, keys, title)
        sheet.save(REF / fname)
        print(fname, sheet.size)


if __name__ == "__main__":
    main()
