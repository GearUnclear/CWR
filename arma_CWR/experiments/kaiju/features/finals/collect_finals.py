r"""Collects one finals run: frames without the runner's sequence prefix, a contact sheet of all of
them, the FIN* lines of the game log and a small pixel table per frame.

  python collect_finals.py <run dir (runs\<tag>)> <out dir (shots\<tag>)> [--cols 6]

The pixel table: mean brightness of the frame (0..255), the share of pixels that are "furnace"
(bright orange to yellow: r > 150, g between 0.3 r and 0.95 r, b under 0.55 r) and their mean
colour. At night the furnace share and colour say whether the fullbright sheet drew bright.
Requires Pillow and numpy.
"""
import glob
import os
import re
import shutil
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont


def font(size):
    try:
        return ImageFont.truetype(r"C:\Windows\Fonts\segoeui.ttf", size)
    except OSError:
        return ImageFont.load_default()


def stats(path):
    a = np.asarray(Image.open(path).convert("RGB"), dtype=np.float32)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    furn = (r > 150) & (g > 0.3 * r) & (g < 0.95 * r) & (b < 0.55 * r)
    n = int(furn.sum())
    col = tuple(int(a[furn][:, k].mean()) for k in range(3)) if n else (0, 0, 0)
    return float(a.mean()), n / furn.size, col


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    cols = 6
    if "--cols" in sys.argv:
        cols = int(sys.argv[sys.argv.index("--cols") + 1])
    run, out = args[0], args[1]
    pngs = sorted(glob.glob(os.path.join(run, "**", "*.png"), recursive=True))
    logs = glob.glob(os.path.join(run, "**", "game_stdout.log"), recursive=True)
    os.makedirs(out, exist_ok=True)
    kept = []
    for p in pngs:
        label = re.sub(r"^\d+_", "", os.path.splitext(os.path.basename(p))[0])
        if "discard" in label:
            continue
        dst = os.path.join(out, label + ".png")
        shutil.copyfile(p, dst)
        kept.append((label, dst))
    lines = []
    errors = []
    if logs:
        with open(logs[0], errors="ignore") as fh:
            for line in fh:
                m = re.search(r'"(FIN[A-Z]+ .*)"\s*$', line)
                if m:
                    lines.append(m.group(1))
                elif re.search(r"uddedushka|dedushka\.p3d|\.paa|rtm|skeleton|Extreme texture|Cannot load|Texture init|very complex shadow", line, re.I):
                    errors.append(line.rstrip())
    with open(os.path.join(out, "measurements.txt"), "w", encoding="utf-8") as fh:
        fh.write("Game log: %s\n\n" % (logs[0] if logs else "no game_stdout.log found"))
        fh.write("\n".join(lines) + "\n")
        fh.write("\nLog lines that name the addon, its model, a texture, a clip or a load problem (%d)\n\n" % len(errors))
        fh.write("\n".join(errors[:200]) + "\n")
        fh.write("\nFrames\n\n%-36s %10s %10s  %s\n" % ("shot", "mean 0-255", "furnace %", "furnace mean rgb"))
        for label, p in kept:
            mean, share, col = stats(p)
            fh.write("%-36s %10.1f %10.3f  %s\n" % (label, mean, 100 * share, col))
    if kept:
        tw, th, lh = 480, 270, 20
        nrows = (len(kept) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * tw, nrows * (th + lh)), (20, 20, 20))
        draw = ImageDraw.Draw(sheet)
        f = font(13)
        for i, (label, p) in enumerate(kept):
            im = Image.open(p).convert("RGB")
            im.thumbnail((tw, th), Image.LANCZOS)
            x, y = (i % cols) * tw, (i // cols) * (th + lh)
            sheet.paste(im, (x + (tw - im.width) // 2, y + (th - im.height) // 2))
            draw.text((x + 4, y + th + 1), label, fill=(230, 230, 230), font=f)
        sheet.save(os.path.join(out, "sheet.png"))
    print("[finals] %d frames -> %s, %d log lines, %d addon/load lines" % (len(kept), out, len(lines), len(errors)))
    return 0 if kept else 1


if __name__ == "__main__":
    sys.exit(main())
