"""B6 review round: the numbers and crops for the three review items and the
off-equals-before check, from the review run (b6r, b6reden) against the r6 run
(the change the reviewer saw), the base build's pinned run and the r6 pinned
prefix.

    python review_b6r.py <review run dir>

Writes <review run dir>/review_analysis.txt and crops into
E:/gr-lanes/gr-B6/evidence/final_images_rev1/.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage

EV = Path(r"E:\gr-lanes\gr-B6\evidence")
R6 = EV / "20261018-163649r6-new" / "tri" / "b6v3"
BASE_PIN = EV / "20261018-162922pin-base" / "tri"
OUT = EV / "final_images_rev1"


def find(root, label):
    hits = sorted(root.rglob(f"*_{label}.png"))
    return hits[0] if hits else None


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


def diff_stats(a, b):
    d = np.abs(a - b).max(axis=2)
    return int(d.max()), int((d > 2).sum()), int((d > 8).sum())


def fog_check(off, on, box):
    """Pixels of the off frame at the fog colour (the commonest colour of the
    far crop, within 3 levels), and how many of them the reflections change."""
    x0, y0, x1, y1 = box
    crop = off[y0:y1, x0:x1].reshape(-1, 3)
    cols, counts = np.unique(crop, axis=0, return_counts=True)
    fog = cols[counts.argmax()]
    near = np.abs(off - fog).max(axis=2) <= 3
    near[:y0, :] = False
    near[y1:, :] = False
    near[:, :x0] = False
    near[:, x1:] = False
    d = np.abs(on.astype(np.int32) - off.astype(np.int32)).max(axis=2)
    changed = near & (d > 3)
    delta = (on.astype(np.float32) - off.astype(np.float32)).mean(axis=2)
    return fog, int(near.sum()), int(changed.sum()), float(delta[near].mean()) if near.any() else 0.0


def dots(img, box):
    """Dark dots: pixels darker than their 9 x 9 median by more than 12 levels,
    counted as connected blobs in the box."""
    lum = img.mean(axis=2)
    med = ndimage.median_filter(lum, 9)
    d = med - lum
    x0, y0, x1, y1 = box
    mask = np.zeros_like(lum, dtype=bool)
    mask[y0:y1, x0:x1] = d[y0:y1, x0:x1] > 12
    lab, n = ndimage.label(mask)
    contrast = float(d[mask].mean()) if n else 0.0
    return n, contrast


def zoom(img, box, k):
    x0, y0, x1, y1 = box
    return Image.fromarray(img[y0:y1, x0:x1].astype(np.uint8)).resize(((x1 - x0) * k, (y1 - y0) * k), Image.NEAREST)


def labelled(tiles, labels, cols):
    w, h = tiles[0].size
    rows = (len(tiles) + cols - 1) // cols
    sheet = Image.new("RGB", (w * cols, (h + 22) * rows), (20, 20, 20))
    dr = ImageDraw.Draw(sheet)
    for i, (t, lab) in enumerate(zip(tiles, labels)):
        x, y = (i % cols) * w, (i // cols) * (h + 22)
        dr.text((x + 6, y + 4), lab, fill=(230, 230, 230))
        sheet.paste(t, (x, y + 22))
    return sheet


def main():
    run = Path(sys.argv[1])
    tri = run / "tri"
    OUT.mkdir(exist_ok=True)
    lines = []

    def P(s=""):
        print(s)
        lines.append(s)

    # 1. Fog: pixels at the fog colour stay with the reflections on.
    P("== 1. fog fade: pixels at the fog colour (off), changed by more than 3 levels with ssr on")
    # The fog colour: the commonest colour of a fully fogged crop (the far hills
    # of the field; the haze at the far end of the street). Then every pixel of
    # the frame within 3 levels of it in the off shot.
    fog_crops = {"ssr_rain_fields": (1500, 350, 1920, 600), "ssr_town_rain_day": (790, 335, 935, 425),
                 "ssr_town_rain": (790, 335, 935, 425)}
    for view, crop in fog_crops.items():
        offs = {}
        for tag, root in (("r6 (reviewed)", R6), ("review run", tri)):
            off_p, on_p = find(root, f"{view}__off"), find(root, f"{view}__on")
            if not off_p or not on_p:
                P(f" {view} {tag}: missing")
                continue
            off, on = load(off_p), load(on_p)
            offs[tag] = off
            fog, _, _, _ = fog_check(off, on, crop)
            near = np.abs(off - fog).max(axis=2) <= 3
            d = np.abs(on.astype(np.int32) - off.astype(np.int32)).max(axis=2)
            n, ch = int(near.sum()), int((near & (d > 3)).sum())
            mean = float((on.astype(np.float32) - off.astype(np.float32)).mean(axis=2)[near].mean()) if n else 0.0
            P(f" {view:20s} {tag:14s} fog {tuple(int(c) for c in fog)}: {ch} of {n} changed, mean on-off {mean:+.2f}")
        if len(offs) == 2:
            # The noise floor: the same view's off shots of the two runs (rain drawn anew every shot).
            a, b = offs["r6 (reviewed)"], offs["review run"]
            fog, _, _, _ = fog_check(b, b, crop)
            near = np.abs(b - fog).max(axis=2) <= 3
            d = np.abs(a.astype(np.int32) - b.astype(np.int32)).max(axis=2)
            P(f" {view:20s} noise: off of the two runs at those pixels: {int((near & (d > 3)).sum())} of {int(near.sum())} "
              f"differ by more than 3 levels")
    # The reviewer's pixel: the hill next to a fogged tree.
    for tag, root in (("r6 (reviewed)", R6), ("review run", tri)):
        off_p, on_p = find(root, "ssr_rain_fields__off"), find(root, "ssr_rain_fields__on")
        if off_p and on_p:
            off, on = load(off_p), load(on_p)
            crop_off, crop_on = off[350:600, 1500:1920], on[350:600, 1500:1920]
            P(f" rain field far crop {tag}: mean off {crop_off.reshape(-1, 3).mean(0).round(1)}, "
              f"on {crop_on.reshape(-1, 3).mean(0).round(1)}")
            tiles = [zoom(off, (1500, 350, 1920, 600), 2), zoom(on, (1500, 350, 1920, 600), 2)]
            labelled(tiles, [f"{tag} off", f"{tag} on"], 2).save(OUT / f"B6r_fog_rain_fields_{'r6' if 'r6' in tag else 'rev'}.png")

    # 2. The jeep's reflection: no pale box. Also against the jeep's own streak above the box.
    for tag, root in (("r6", R6), ("rev", tri)):
        for kind in ("off", "on", "refl"):
            p = find(root, f"ssr_town_rain_day__{kind}")
            if p:
                img = load(p).astype(np.float32)
                P(f" jeep {tag:3s} {kind:4s}: box x 955-1035, y 690-725 {img[690:725, 955:1035].reshape(-1, 3).mean(0).round(1)}, "
                  f"the streak above it (y 640-680) {img[640:680, 955:1035].reshape(-1, 3).mean(0).round(1)}")
    P("")
    P("== 2. the jeep's reflection (town_rain_day, box x 955-1035, y 690-725)")
    box = (955, 690, 1035, 725)
    ring = (900, 650, 1100, 780)
    tiles, labs = [], []
    for tag, root in (("r6", R6), ("rev", tri)):
        for kind in ("off", "on", "refl", "tags"):
            p = find(root, f"ssr_town_rain_day__{kind}")
            if not p:
                continue
            img = load(p)
            x0, y0, x1, y1 = box
            inner = img[y0:y1, x0:x1].reshape(-1, 3).mean(0)
            # The streak beside the box: the jeep's reflection at the same rows, left and right of it.
            side = np.concatenate([img[y0:y1, 920:950].reshape(-1, 3), img[y0:y1, 1040:1070].reshape(-1, 3)]).mean(0)
            if kind in ("on", "refl"):
                P(f" {tag:3s} {kind:4s}: box mean {inner.round(1)}, beside it {side.round(1)}, box minus beside "
                  f"{(inner - side).mean():+.1f}")
            tiles.append(zoom(img, ring, 3))
            labs.append(f"{tag} {kind}")
    labelled(tiles, labs, 4).save(OUT / "B6r_jeep_reflection_r6_vs_rev.png")

    # 3. The dot lattice.
    P("")
    P("== 3. dark dots (blobs 12+ levels under their 9x9 median), road left of the jeep x 100-900, y 500-1000")
    dbox = (100, 500, 900, 1000)
    road = (300, 500, 900, 660)  # the open road between the puddle and the jeep (no mirrored house)
    for tag, root in (("r6", R6), ("rev", tri)):
        for kind in ("off", "on", "refl"):
            p = find(root, f"ssr_town_rain_day__{kind}")
            if p:
                img = load(p)
                n, c = dots(img, dbox)
                n2, _ = dots(img, road)
                P(f" {tag:3s} {kind:4s}: {n} dots, mean depth {c:.1f}; open road x 300-900, y 500-660: {n2}")
    tiles, labs = [], []
    for tag, root in (("r6", R6), ("rev", tri)):
        for kind in ("on", "refl"):
            p = find(root, f"ssr_town_rain_day__{kind}")
            if p:
                tiles.append(zoom(load(p), (760, 520, 960, 620), 4))
                labs.append(f"{tag} {kind} x4")
    labelled(tiles, labs, 2).save(OUT / "B6r_dots_r6_vs_rev.png")
    # Motion: per frame, the dots on the road, and the frame-to-frame change.
    P(" motion (town_rain_day, 16 frames, sim clock 1/30 s a frame, TAA on):")
    for mode in ("off", "on"):
        frames = [find(tri, f"ssr_motion_town__{mode}__f{k:03d}") for k in range(16)]
        frames = [load(p) for p in frames if p]
        if not frames:
            continue
        counts = [dots(f, road)[0] for f in frames]
        steps = [np.abs(frames[k + 1] - frames[k]).max(axis=2)[500:1000, 100:900] for k in range(len(frames) - 1)]
        med = [float(np.median(s)) for s in steps]
        p99 = [float(np.percentile(s, 99)) for s in steps]
        P(f"  {mode}: dots per frame on the open road {counts}; frame-to-frame change x 100-900, y 500-1000 "
          f"median {np.mean(med):.2f}, "
          f"99th percentile {np.mean(p99):.2f}")
        if mode == "on":
            tiles = [zoom(frames[k], (760, 520, 960, 620), 3) for k in (0, 5, 10, 15)]
            labelled(tiles, [f"on f{k:03d} x3" for k in (0, 5, 10, 15)], 4).save(OUT / "B6r_dots_motion_on.png")

    # 4. The water views: the hero and the sunset sea against r6.
    P("")
    P("== 4. water views against r6 (on vs on, refl vs refl; max / >2 / >8 levels)")
    for view in ("ssr_bay_offshore", "ssr_coastline_sunset"):
        for kind in ("off", "on", "refl"):
            a, b = find(R6, f"{view}__{kind}"), find(tri, f"{view}__{kind}")
            if a and b:
                P(f" {view} {kind}: {diff_stats(load(a), load(b))}")
        on_p, off_p = find(tri, f"{view}__on"), find(tri, f"{view}__off")
        if on_p and off_p:
            on, off = load(on_p).astype(np.float32), load(off_p).astype(np.float32)
            sea = slice(560, 1080)
            P(f" {view} sea rows 560+: mean off {off[sea].reshape(-1, 3).mean(0).round(1)}, on {on[sea].reshape(-1, 3).mean(0).round(1)}")

    # 5. Off equals before: the pinned prefix.
    P("")
    P("== 5. off equals before (pinned prefix; max / >2 / >8 levels of 2073600)")
    for v in ("bay", "sunset", "lowsun", "noon"):
        new_off, new_off2 = find(tri, f"pin_{v}__off"), find(tri, f"pin_{v}__off2")
        r6_off, base_off, base_off2 = find(R6, f"pin_{v}__off"), find(BASE_PIN, f"pin_{v}__off"), find(BASE_PIN, f"pin_{v}__off2")
        if new_off and r6_off:
            P(f" {v:6s} review off vs r6 off (same sequence place): {diff_stats(load(new_off), load(r6_off))}")
        if new_off and base_off:
            P(f" {v:6s} review off vs base off               : {diff_stats(load(new_off), load(base_off))}")
        if new_off and base_off2:
            P(f" {v:6s} review off vs base off2              : {diff_stats(load(new_off), load(base_off2))}")
        if base_off and base_off2:
            P(f" {v:6s} base off vs base off2 (repeat)       : {diff_stats(load(base_off), load(base_off2))}")
        if new_off and new_off2:
            P(f" {v:6s} review off vs review off2 (repeat)   : {diff_stats(load(new_off), load(new_off2))}")

    # 6. Eden: the clutter bush.
    P("")
    P("== 6. Eden clutter bush (overcast rain)")
    eden = {k: find(tri, f"ssr_eden_bush__{k}") for k in ("off", "on", "tags", "refl")}
    if all(eden.values()):
        imgs = {k: load(p) for k, p in eden.items()}
        crop = (500, 380, 760, 540)
        labelled([zoom(imgs[k], crop, 2) for k in ("off", "on", "tags", "refl")], ["off", "on", "tags", "refl"],
                 2).save(OUT / "B6r_eden_bush.png")
        tags = imgs["tags"]
        magenta = (tags[..., 0] > 200) & (tags[..., 1] < 80) & (tags[..., 2] > 180)
        P(f" covered (magenta) pixels in the tag view: {int(magenta.sum())}")
        ys, xs = np.nonzero(magenta)
        if len(xs):
            P(f" covered bounding box x {xs.min()}-{xs.max()}, y {ys.min()}-{ys.max()}")
    else:
        P(" missing: " + ", ".join(k for k, p in eden.items() if not p))

    (run / "review_analysis.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
