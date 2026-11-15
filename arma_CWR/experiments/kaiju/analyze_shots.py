r"""Collects a Dedushka photo run: measurements, a per-shot table and a contact sheet.

  python analyze_shots.py <tri output dir> <stills dir>

<tri output dir> is what run-tri.ps1 printed as OUT= (it holds
dedushka_photo\*.png and game_stdout.log). Into <stills dir> go:

  shots\<label>.png     the frames, without the runner's sequence prefix
  sheet.png             every frame on one contact sheet
  measurements.txt      the DEDPHOTO and DEDSHOT lines of the game log, then a
                        table per frame: is the body in the frame (orange
                        fullbright pixels near the frame centre), how many
                        pixels, and which LOD the head colour says it is

The proxy model paints the head of each visual LOD a different flat colour
(LOD0 bone white, LOD1 red, LOD2 green, LOD3 blue) and carries a fullbright
orange chest patch; an art candidate has neither, so for it only the frames
and the log lines mean anything (the table then says "no patch").
Requires Pillow.
"""
import glob
import os
import re
import shutil
import sys

from PIL import Image, ImageDraw


def classify(path):
    """(orange, red, green, blue) pixel counts in the central column of the frame."""
    im = Image.open(path).convert("RGB")
    w, h = im.size
    x0, x1 = int(w * 0.36), int(w * 0.64)
    box = im.crop((x0, 0, x1, h))
    counts = [0, 0, 0, 0]
    raw = box.tobytes()
    for i in range(0, len(raw), 3):
        r, g, b = raw[i], raw[i + 1], raw[i + 2]
        # Day (255, 115, 8) and night (218, 131, 65) readings of the patch.
        if r > 140 and b < 0.36 * r and 0.30 * r < g < 0.66 * r:
            counts[0] += 1
        elif r > 60 and g < 0.3 * r and b < 0.3 * r:
            counts[1] += 1
        elif g > 60 and r < 0.4 * g and b < 0.45 * g:
            counts[2] += 1
        elif b > 90 and r < 0.3 * b and g < 0.4 * b:
            counts[3] += 1
    return counts


def lod_guess(c):
    orange, red, green, blue = c
    best = max((red, 1), (green, 2), (blue, 3))
    if best[0] >= 3 and best[0] >= 0.02 * max(orange, 1):
        return "LOD%d" % best[1]
    if orange >= 3:
        return "LOD0 (or lit marker not seen)"
    return "-"


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    run, out = sys.argv[1], sys.argv[2]
    pngs = sorted(glob.glob(os.path.join(run, "**", "*.png"), recursive=True))
    logs = glob.glob(os.path.join(run, "**", "game_stdout.log"), recursive=True)
    shots = os.path.join(out, "shots")
    os.makedirs(shots, exist_ok=True)
    rows = []
    kept = []
    for p in pngs:
        label = re.sub(r"^\d+_", "", os.path.splitext(os.path.basename(p))[0])
        if "discard" in label:
            continue
        dst = os.path.join(shots, label + ".png")
        shutil.copyfile(p, dst)
        kept.append((label, dst))
        c = classify(dst)
        rows.append((label, c, lod_guess(c)))

    lines = []
    if logs:
        with open(logs[0], errors="ignore") as fh:
            for line in fh:
                m = re.search(r'"(DED(?:PHOTO|SHOT) .*)"\s*$', line)
                if m:
                    lines.append(m.group(1))
    with open(os.path.join(out, "measurements.txt"), "w", encoding="utf-8") as fh:
        fh.write("Game log (%s)\n\n" % (logs[0] if logs else "no game_stdout.log found"))
        for l in lines:
            fh.write(l + "\n")
        fh.write("\nFrames (pixel counts in the central 28 percent of the frame width)\n\n")
        fh.write("%-28s %8s %6s %6s %6s  %s\n" % ("shot", "orange", "red", "green", "blue", "reads as"))
        for label, c, guess in rows:
            body = guess if c[0] >= 3 or guess != "-" else "no patch seen (not drawn, hidden, or no patch on this model)"
            fh.write("%-28s %8d %6d %6d %6d  %s\n" % (label, c[0], c[1], c[2], c[3], body))

    if kept:
        tw, th, lh, cols = 400, 300, 16, 5
        nrows = (len(kept) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * tw, nrows * (th + lh)), (20, 20, 20))
        draw = ImageDraw.Draw(sheet)
        for i, (label, p) in enumerate(kept):
            im = Image.open(p).convert("RGB")
            im.thumbnail((tw, th))
            x, y = (i % cols) * tw, (i // cols) * (th + lh)
            sheet.paste(im, (x + (tw - im.width) // 2, y + (th - im.height) // 2))
            draw.text((x + 4, y + th + 2), label, fill=(230, 230, 230))
        sheet.save(os.path.join(out, "sheet.png"))
    print("[stills] %d frames -> %s" % (len(kept), shots))
    print("[stills] %d log lines -> %s" % (len(lines), os.path.join(out, "measurements.txt")))
    return 0 if kept else 1


if __name__ == "__main__":
    sys.exit(main())
