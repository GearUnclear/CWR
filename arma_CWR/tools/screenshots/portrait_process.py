#!/usr/bin/env python3
"""Turn a portrait shoot's raw captures into the .paa catalogue the dossier reads.

    python portrait_process.py <shoot-output-dir> <lane> [--keep-bmp]

`lane` is "vanilla" or "lobo"; it only decides which key set the lighting check
reports against and which lane the catalogue records.  Everything else is driven
by the files actually present in the shoot directory.

Per capture, in this order:

  1. read the PNG and take W,H FROM THE FILE.  The capture is
     SDL_GetWindowSizeInPixels, not the size the toml asked for
     (EngineGL33_VertexBuffer.cpp:545-547), so a crop rectangle derived from the
     toml would silently drift under display scaling.
  2. measure a fixed background patch in the UNTOUCHED image.  This is the
     lighting proof: every subject in a lane stands on one anchor under one sky
     with one heading, so those pixels are the same image in every frame, and a
     spread wider than the tolerances below means the sun, weather or anchor
     moved and the run is invalid.
  3. crop the frozen square, tone it, resize to 236, and paste it on a 256x256
     notepad-paper card with a one-pixel inner rule.
  4. encode to guerrilla-mode/core/portraits/<key>.paa via PoseidonTools, DXT1.

Then it writes catalogue.json and deletes the BMP twins triScreenshot leaves
behind (an extensionless path makes ScreenshotWriter emit both formats).

Requires Pillow, like contact_sheet.py next door.
"""

import glob
import json
import os
import re
import subprocess
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import portrait_roster  # noqa: E402

REPO_ROOT = portrait_roster.REPO_ROOT
OUT_DIR = os.path.join(REPO_ROOT, "guerrilla-mode", "core", "portraits")

# ---------------------------------------------------------------------------
# FROZEN CONSTANTS.  Measured off the calibration frames; see
# guerrilla-mode/core/portraits/README.md.  Changing one of these means every
# .paa in the catalogue has to be regenerated, because a portrait's job is to
# sit beside the other portraits and look like it was taken the same afternoon.
# ---------------------------------------------------------------------------

# Crop square, as fractions of the capture HEIGHT.  CROP_TOP was raised from a
# first guess of 0.39 because at that value the helmet of every helmeted class
# was clipped by the top edge.
CROP_SIDE = 0.40
CROP_DX = 0.005
CROP_TOP = 0.345

# Background patch: two rectangles in the top corners of the untouched frame,
# as fractions of (W, H).  Both are open sky in every frame of both lanes and
# neither can ever touch the silhouette, which lives in the middle third.
PATCHES = [(0.00, 0.00, 0.10, 0.10), (0.90, 0.00, 1.00, 0.10)]

# Tone.
SATURATION = 0.40  # blend toward luminance, keeping this much colour
WARM = (1.06, 1.00, 0.92)  # per-channel multiply, clipped
FLOOR, CEIL = 18, 245  # map 0..255 onto this, so nothing is pure black or white
CONTRAST = 1.08

# Card.
CARD = 256
INNER = 236
MARGIN = (CARD - INNER) // 2
# The notepad's own paper measures (209, 210, 207) on a clean stretch of a real
# 800x600 dossier capture: a neutral light grey, not the cream a first guess of
# (222, 214, 196) assumed.  The card is set slightly warmer and lighter than
# that on purpose.  Matching the paper exactly makes the border vanish and the
# photograph reads as a hole cut in the page; a shade of warmth makes it read as
# a print mounted on the page, which is what the page wants.
PAPER = (218, 214, 201)
RULE = (194, 190, 177)  # one-pixel inner rule, a shade darker than the card

# Lighting tolerances, per channel, in 0..255 units.
TOL_WITHIN_CLASS = 2.0
TOL_WITHIN_LANE = 6.0

# Below this mean pixel difference the four faces of a class are, to the eye,
# the same photograph. Reported, never failed.
FACE_VARIATION_FLOOR = 4.0


def tools_exe():
    for suffix in ("x64-win-rwdi", "x64-win-rel", "x64-win-dbg"):
        for name in ("PoseidonTools.exe", "PoseidonTools"):
            p = os.path.join(REPO_ROOT, "dist", suffix, name)
            if os.path.exists(p):
                return p
    for p in glob.glob(os.path.join(REPO_ROOT, "dist", "*", "PoseidonTools*")):
        return p
    return None


def patch_mean(img):
    """Mean RGB of the fixed background patches of an untouched capture."""
    W, H = img.size
    total = [0.0, 0.0, 0.0]
    n = 0
    for x0f, y0f, x1f, y1f in PATCHES:
        box = (int(x0f * W), int(y0f * H), int(x1f * W), int(y1f * H))
        region = img.crop(box)
        px = region.load()
        w, h = region.size
        for y in range(0, h, 4):
            for x in range(0, w, 4):
                r, g, b = px[x, y][:3]
                total[0] += r
                total[1] += g
                total[2] += b
                n += 1
    return [round(c / max(n, 1), 3) for c in total]


def tone(img):
    """Desaturate, warm, flatten, then a touch of contrast.  Order matters: the
    flatten has to see the warmed values or the warm tint gets re-normalised
    away, and the contrast has to come last or it re-crushes the flattened
    ends."""
    px = img.load()
    w, h = img.size
    lo, span = FLOOR, (CEIL - FLOOR) / 255.0
    for y in range(h):
        for x in range(w):
            r, g, b = px[x, y][:3]
            lum = 0.299 * r + 0.587 * g + 0.114 * b
            r = lum + (r - lum) * SATURATION
            g = lum + (g - lum) * SATURATION
            b = lum + (b - lum) * SATURATION
            r *= WARM[0]
            g *= WARM[1]
            b *= WARM[2]
            out = []
            for c in (r, g, b):
                c = min(max(c, 0.0), 255.0)
                c = lo + c * span
                c = 128.0 + (c - 128.0) * CONTRAST
                out.append(int(min(max(c, 0.0), 255.0)))
            px[x, y] = (out[0], out[1], out[2])
    return img


def card(square):
    """236 px portrait on a 256 px paper card with a one-pixel inner rule."""
    thumb = square.resize((INNER, INNER), Image.LANCZOS)
    sheet = Image.new("RGB", (CARD, CARD), PAPER)
    sheet.paste(thumb, (MARGIN, MARGIN))
    d = sheet.load()
    x0, y0 = MARGIN - 1, MARGIN - 1
    x1, y1 = MARGIN + INNER, MARGIN + INNER
    for x in range(x0, x1 + 1):
        d[x, y0] = RULE
        d[x, y1] = RULE
    for y in range(y0, y1 + 1):
        d[x0, y] = RULE
        d[x1, y] = RULE
    return sheet


def key_of(path):
    """`NNN_<key>.png` -> key.  No ordering assumption: the number is stripped,
    not relied on."""
    stem = os.path.splitext(os.path.basename(path))[0]
    return re.sub(r"^\d+_", "", stem).lower()


def class_of(key):
    return key.rsplit("__", 1)[0] if "__" in key else key


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    keep_bmp = "--keep-bmp" in sys.argv
    if len(args) < 2:
        print(__doc__)
        return 2
    src, lane = args[0], args[1]
    if lane not in portrait_roster.LANES:
        print("lane must be one of %s" % ", ".join(portrait_roster.LANES))
        return 2

    # Only real portrait keys.  The shoot's first capture is a deliberate
    # throwaway (the first triScreenshot of a run lands pre-gamma) and is
    # labelled without a "__" so it drops out here.
    files = [p for p in sorted(glob.glob(os.path.join(src, "*.png"))) if "__" in key_of(p)]
    if not files:
        print("no portrait PNGs in %s" % src)
        return 1
    exe = tools_exe()
    if not exe:
        print("PoseidonTools not found under dist/ - build it first")
        return 1
    os.makedirs(OUT_DIR, exist_ok=True)
    work = os.path.join(src, "_processed")
    os.makedirs(work, exist_ok=True)

    records = []
    digests = {}
    for path in files:
        key = key_of(path)
        img = Image.open(path).convert("RGB")
        W, H = img.size
        mean = patch_mean(img)

        S = round(H * CROP_SIDE)
        cx = W // 2 + round(H * CROP_DX)
        top = round(H * CROP_TOP)
        square = img.crop((cx - S // 2, top, cx - S // 2 + S, top + S))
        digests[key] = square.resize((32, 32), Image.LANCZOS).tobytes()

        png = os.path.join(work, key + ".png")
        card(tone(square)).save(png)

        paa = os.path.join(OUT_DIR, key + ".paa")
        r = subprocess.run([exe, "image", "convert", png, paa, "-f", "DXT1"],
                           capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists(paa):
            print("FAILED to encode %s: %s%s" % (key, r.stdout, r.stderr))
            return 1
        records.append({
            "key": key,
            "class": class_of(key),
            "face": key.rsplit("__", 1)[1] if "__" in key else "",
            "lane": lane,
            "source": os.path.basename(path),
            "sourceSize": [W, H],
            "backgroundPatchMean": mean,
            "paa": os.path.basename(paa),
            "paaSize": [CARD, CARD],
            "paaFormat": "DXT1",
        })
        print("%-44s %s  patch=%s" % (key, os.path.basename(paa), mean))

    # ---- the lighting proof ------------------------------------------------
    ok = True
    by_class = {}
    for rec in records:
        by_class.setdefault(rec["class"], []).append(rec)
    for cls, group in sorted(by_class.items()):
        for ch in range(3):
            vals = [g["backgroundPatchMean"][ch] for g in group]
            spread = max(vals) - min(vals)
            if spread > TOL_WITHIN_CLASS:
                ok = False
                print("LIGHTING: %s channel %d spread %.2f > %.1f across %d faces"
                      % (cls, ch, spread, TOL_WITHIN_CLASS, len(group)))
    for ch in range(3):
        vals = [r["backgroundPatchMean"][ch] for r in records]
        spread = max(vals) - min(vals)
        print("lane channel %d patch spread %.2f (tolerance %.1f)" % (ch, spread, TOL_WITHIN_LANE))
        if spread > TOL_WITHIN_LANE:
            ok = False
            print("LIGHTING: lane channel %d spread %.2f > %.1f - the sun, the weather or the"
                  " anchor moved during this run.  The frames are not usable; reshoot."
                  % (ch, spread, TOL_WITHIN_LANE))

    # ---- how much the four faces of a class actually differ ----------------
    # Not a pass/fail: it is the evidence behind the README's list of classes
    # whose model hides the face (a ghillie hood, a keffiyeh wrap, a balaclava).
    # Those still need all four keys - the registry rolls a face regardless and
    # the dossier must resolve something - but the reader should be told the
    # four photographs are near enough the same picture.
    baked = []
    for cls, group in sorted(by_class.items()):
        keys = [g["key"] for g in group]
        worst = 0.0
        for i in range(len(keys)):
            for j in range(i + 1, len(keys)):
                a, b = digests[keys[i]], digests[keys[j]]
                worst = max(worst, sum(abs(a[k] - b[k]) for k in range(len(a))) / float(len(a)))
        for g in group:
            g["faceVariation"] = round(worst, 3)
        if len(group) > 1 and worst < FACE_VARIATION_FLOOR:
            baked.append(cls)
    if baked:
        print("FACE BARELY VARIES (mean pixel difference under %.1f/255 - a baked head, or a"
              " model that covers the face): %s" % (FACE_VARIATION_FLOOR, ", ".join(baked)))

    # ---- catalogue ---------------------------------------------------------
    cat_path = os.path.join(OUT_DIR, "catalogue.json")
    catalogue = {}
    if os.path.exists(cat_path):
        with open(cat_path, "r", encoding="utf-8") as fh:
            catalogue = json.load(fh)
    entries = {e["key"]: e for e in catalogue.get("entries", []) if e.get("lane") != lane}
    for rec in records:
        entries[rec["key"]] = rec
    catalogue = {
        "note": "Generated by tools/screenshots/portrait_process.py.  One record per"
                " dossier portrait key; the key is lower(bodyClass) + '__' + lower(face),"
                " which is what GuerrillaJournalPages.cpp PortraitKeyOf builds.",
        "crop": {"side": CROP_SIDE, "dx": CROP_DX, "top": CROP_TOP},
        "tone": {"saturation": SATURATION, "warm": list(WARM),
                 "levels": [FLOOR, CEIL], "contrast": CONTRAST},
        "card": {"size": CARD, "inner": INNER, "paper": list(PAPER), "rule": list(RULE)},
        "faceBarelyVaries": sorted(set(baked) | {c for c in catalogue.get("faceBarelyVaries", [])
                                                 if c.lower().startswith("lobo") != (lane == "lobo")}),
        "entries": [entries[k] for k in sorted(entries)],
    }
    with open(cat_path, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(catalogue, fh, indent=2)
        fh.write("\n")
    print("wrote %s (%d entries)" % (cat_path, len(catalogue["entries"])))

    if not keep_bmp:
        n = 0
        for bmp in glob.glob(os.path.join(src, "*.bmp")):
            os.remove(bmp)
            n += 1
        print("removed %d BMP twins" % n)

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
