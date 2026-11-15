r"""Builds the before and after proof pictures from a texres_charts run.

  python make_proof.py <tri output dir> <proof dir>

For each camera of the BEFORE (pb_*) and AFTER (pa_*) bodies it writes
  proof_<camera>.png          the two frames side by side with a caption strip
  proof_<camera>_crop.png     the same pair, a 640 x 400 cut at full size around a spot that
                              matters (CROPS below), so texels can be judged
and for the two "shifted" pairs (the camera moved sideways by under one pixel) the mean
change between the two frames inside the creature's silhouette: a still-frame stand-in for
shimmer. It is not a motion test.
"""
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

# camera -> (centre x, centre y) of the full size cut, as fractions of the frame
CROPS = {
    "02_scale_props": (0.42, 0.50),
    "03_foot_and_tank": (0.42, 0.52),
    "04_toes_and_soldier": (0.45, 0.50),
    "05_feet_up": (0.50, 0.55),
    "06_shin_from_feet": (0.50, 0.50),
    "07_under_pelvis": (0.50, 0.50),
    "01_60m": (0.50, 0.62),
    "08_150m": (0.50, 0.50),
    "09_300m": (0.50, 0.50),
    "12_300m_lens4": (0.50, 0.60),
}


def font(px):
    try:
        return ImageFont.truetype("arial.ttf", px)
    except OSError:
        return ImageFont.load_default()


def pair(before, after, caption, out, crop=None):
    a, b = Image.open(before).convert("RGB"), Image.open(after).convert("RGB")
    if crop:
        w, h = a.size
        cw, ch = 640, 400
        x = int(min(max(0, crop[0] * w - cw / 2), w - cw))
        y = int(min(max(0, crop[1] * h - ch / 2), h - ch))
        a, b = a.crop((x, y, x + cw, y + ch)), b.crop((x, y, x + cw, y + ch))
        scale = 2
        a = a.resize((cw * scale, ch * scale), Image.NEAREST)
        b = b.resize((cw * scale, ch * scale), Image.NEAREST)
    else:
        a, b = a.resize((a.size[0] // 2, a.size[1] // 2), Image.LANCZOS), b.resize((b.size[0] // 2, b.size[1] // 2), Image.LANCZOS)
    w, h = a.size
    sheet = Image.new("RGB", (2 * w + 12, h + 40), (20, 20, 20))
    sheet.paste(a, (0, 40))
    sheet.paste(b, (w + 12, 40))
    d = ImageDraw.Draw(sheet)
    f = font(20)
    d.text((8, 8), "BEFORE: " + caption[0], fill=(255, 190, 190), font=f)
    d.text((w + 20, 8), "AFTER: " + caption[1], fill=(190, 255, 190), font=f)
    sheet.save(out)


def shimmer(p1, p2):
    """Mean absolute change between two frames where the creature is (not sky, not ground)."""
    a = np.asarray(Image.open(p1).convert("L"), dtype=np.float32)
    b = np.asarray(Image.open(p2).convert("L"), dtype=np.float32)
    h, w = a.shape
    # the creature stands in the middle column of these frames, above the horizon
    box = (slice(int(h * 0.15), int(h * 0.62)), slice(int(w * 0.44), int(w * 0.56)))
    return float(np.abs(a[box] - b[box]).mean()), float(np.abs(a[box] - b[box]).max())


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    run_dir, out_dir = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    cap = (sys.argv[3], sys.argv[4]) if len(sys.argv) >= 5 else ("one 1024 sheet", "4096 sheets")
    os.makedirs(out_dir, exist_ok=True)
    frames = {}
    for root, _d, files in os.walk(run_dir):
        for f in files:
            m = re.match(r"\d+_(p[ab]_.+)\.png$", f)
            if m:
                frames[m.group(1)] = os.path.join(root, f)
    for name in sorted(frames):
        if not name.startswith("pb_"):
            continue
        cam = name[3:]
        other = "pa_" + cam
        if other not in frames or cam.startswith("00"):
            continue
        pair(frames[name], frames[other], cap, os.path.join(out_dir, "proof_%s.png" % cam))
        if cam in CROPS:
            pair(frames[name], frames[other], cap, os.path.join(out_dir, "proof_%s_crop.png" % cam), CROPS[cam])
    lines = []
    for tag, label in (("pb", "BEFORE"), ("pa", "AFTER")):
        for base, shifted in (("10_600m", "10s_600m_shifted"), ("11_1km", "11s_1km_shifted"), ("12_300m_lens4", "12s_300m_lens4_shifted")):
            k1, k2 = "%s_%s" % (tag, base), "%s_%s" % (tag, shifted)
            if k1 in frames and k2 in frames:
                mean, peak = shimmer(frames[k1], frames[k2])
                lines.append("%-6s %-14s mean change %.2f of 255, largest %.0f" % (label, base, mean, peak))
    with open(os.path.join(out_dir, "shimmer.txt"), "w", encoding="utf-8") as fh:
        fh.write("Change between two frames whose camera moved sideways by less than a pixel,\n"
                 "inside the middle of the picture where the creature stands. Stills only: not a motion test.\n\n")
        fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
