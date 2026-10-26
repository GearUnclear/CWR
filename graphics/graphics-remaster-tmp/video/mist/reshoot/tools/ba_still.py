"""Before/after still for the valley_mist reshoot.

usage: python ba_still.py --before <old ud mkv> --after <new ud mkv> --frame 328 --out <png>

Rows (clip frame numbers are 1-based, as in bandmetric.py):
  1. the bay and the far valley (x 1280-1920, y 560-800 of 1920x1080) at 2x, BEFORE | AFTER, same frame
  2. BEFORE: 6 consecutive frames (frame-2 .. frame+3) of the mist band (x 720-1920, y 470-860), as shot
  3. BEFORE: the change from the previous frame, x20 around grey (a flat grey tile = nothing changed)
  4. AFTER: the same 6 frames
  5. AFTER: the change from the previous frame, x20
"""
import argparse
import subprocess

import numpy as np
from PIL import Image, ImageDraw, ImageFont

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080
FONT = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/cwr_body.ttf"


def frames(mkv, nums):
    lo, hi = min(nums) - 1, max(nums) - 1          # 0-based frame indices
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", mkv, "-vf",
                        f"select='between(n\\,{lo}\\,{hi})'", "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W, 3)
    return {n: x[n - 1 - lo].astype(np.float32) for n in nums}


def font(size):
    try:
        return ImageFont.truetype(FONT, size)
    except OSError:
        return ImageFont.load_default()


def label(img, text, size=22):
    d = ImageDraw.Draw(img)
    f = font(size)
    box = d.textbbox((0, 0), text, font=f)
    d.rectangle([0, 0, box[2] + 14, box[3] + 10], fill=(0, 0, 0))
    d.text((7, 4), text, fill=(240, 241, 243), font=f)
    return img


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--before", required=True)
    ap.add_argument("--after", required=True)
    ap.add_argument("--frame", type=int, default=328)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    m = a.frame
    strip = list(range(m - 2, m + 4))               # 6 consecutive frames
    need = sorted(set([strip[0] - 1] + strip))
    fb, fa = frames(a.before, need), frames(a.after, need)
    sheet_w = 2560
    rows = []
    # row 1: 2x crops of the same frame
    cx0, cx1, cy0, cy1 = 1280, 1920, 560, 800
    pair = Image.new("RGB", (sheet_w, (cy1 - cy0) * 2))
    for i, (name, fr) in enumerate((("BEFORE", fb), ("AFTER", fa))):
        c = fr[m][cy0:cy1, cx0:cx1].clip(0, 255).astype(np.uint8)
        t = Image.fromarray(c).resize(((cx1 - cx0) * 2, (cy1 - cy0) * 2), Image.NEAREST)
        pair.paste(label(t, f"{name}   frame {m}, bay and far valley at 2x", 26), (i * (cx1 - cx0) * 2, 0))
    rows.append(pair)
    # rows 2-5: 6-frame strips of the band and their frame-to-frame change x20
    bx0, bx1, by0, by1 = 720, 1920, 470, 860
    tw = sheet_w // len(strip)
    th = round((by1 - by0) * tw / (bx1 - bx0))
    for name, fr in (("BEFORE", fb), ("AFTER", fa)):
        r_img = Image.new("RGB", (sheet_w, th))
        r_d = Image.new("RGB", (sheet_w, th))
        for ci, n in enumerate(strip):
            c = fr[n][by0:by1, bx0:bx1]
            t = Image.fromarray(c.clip(0, 255).astype(np.uint8)).resize((tw, th), Image.BOX)
            mean = c.mean()
            r_img.paste(label(t, f"{name} f{n}  band mean {mean:.1f}", 17), (ci * tw, 0))
            d1 = (fr[n].mean(axis=2) - fr[n - 1].mean(axis=2))[by0:by1, bx0:bx1]
            g = (128 + 20 * d1).clip(0, 255).astype(np.uint8)
            t = Image.fromarray(g).resize((tw, th), Image.BOX).convert("RGB")
            r_d.paste(label(t, f"{name} f{n} minus f{n - 1}, x20", 17), (ci * tw, 0))
        rows += [r_img, r_d]
    out = Image.new("RGB", (sheet_w, sum(r.height for r in rows) + 6 * (len(rows) - 1)), (20, 20, 20))
    y = 0
    for r in rows:
        out.paste(r, (0, y))
        y += r.height + 6
    out.save(a.out)
    print("wrote", a.out, out.size)


if __name__ == "__main__":
    main()
