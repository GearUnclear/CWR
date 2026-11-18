"""Scouting sheets: one row block per clip, a thumbnail every STEP seconds, labelled.

python scout_sheets.py [--step 1.0] [--per 3] clipprefix ...
Writes <UD_PROMO_DIR>/work/edit/scout/sheet_<first>_<last>.jpg. Thumbnails only; nothing else is kept.
"""
import argparse
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from promopaths import FF, FONT_DIR, FP, PROMO  # noqa: E402

ROOT = PROMO
CLIPS = ROOT / "clips"
OUT = ROOT / "work" / "edit" / "scout"
FONT = FONT_DIR / "cwr_mono.ttf"
TW, TH = 320, 180
COLS = 7


def clip_path(prefix):
    m = sorted(p for p in CLIPS.glob(prefix + "_*.mp4"))
    if not m:
        m = sorted(CLIPS.glob(prefix + "*.mp4"))
    return m[0]


def frames(path, step, start=0.0, end=None):
    dur = float(subprocess.check_output([FP,"-v", "error", "-show_entries", "format=duration",
                                         "-of", "csv=p=0", str(path)]).decode().strip())
    end = dur - 0.05 if end is None else end
    times = []
    t = start
    while t <= end + 1e-6:
        times.append(round(t, 3))
        t += step
    out = []
    for t in times:
        raw = subprocess.check_output([FF,"-v", "error", "-ss", f"{t:.3f}", "-i", str(path), "-frames:v", "1",
                                       "-vf", f"scale={TW}:{TH}", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"])
        if len(raw) == TW * TH * 3:
            out.append((t, Image.frombytes("RGB", (TW, TH), raw)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--step", type=float, default=1.0)
    ap.add_argument("--start", type=float, default=0.0)
    ap.add_argument("--end", type=float, default=None)
    ap.add_argument("--per", type=int, default=3)
    ap.add_argument("--name", default=None)
    a = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    font = ImageFont.truetype(str(FONT), 18)
    groups = [a.clips[i:i + a.per] for i in range(0, len(a.clips), a.per)]
    for g in groups:
        blocks = []
        for c in g:
            p = clip_path(c)
            fr = frames(p, a.step, a.start, a.end)
            rows = (len(fr) + COLS - 1) // COLS
            blk = Image.new("RGB", (COLS * TW, 26 + rows * TH), (20, 20, 20))
            d = ImageDraw.Draw(blk)
            d.text((6, 3), p.name, fill=(255, 220, 120), font=font)
            for i, (t, im) in enumerate(fr):
                x, y = (i % COLS) * TW, 26 + (i // COLS) * TH
                blk.paste(im, (x, y))
                for k in (1, 2):
                    d.line([x + TW * k // 3, y, x + TW * k // 3, y + TH], fill=(255, 255, 255, 90), width=1)
                    d.line([x, y + TH * k // 3, x + TW, y + TH * k // 3], fill=(255, 255, 255, 90), width=1)
                d.rectangle([x, y, x + 62, y + 22], fill=(0, 0, 0))
                d.text((x + 4, y + 2), f"{t:.1f}", fill=(255, 255, 255), font=font)
            blocks.append(blk)
        H = sum(b.height for b in blocks) + 6 * len(blocks)
        sheet = Image.new("RGB", (COLS * TW, H), (0, 0, 0))
        y = 0
        for b in blocks:
            sheet.paste(b, (0, y))
            y += b.height + 6
        name = a.name or f"sheet_{g[0]}_{g[-1]}"
        sheet.save(OUT / f"{name}.jpg", quality=80)
        print(OUT / f"{name}.jpg")


if __name__ == "__main__":
    sys.exit(main())
