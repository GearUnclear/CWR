"""Agent 4 (fogwind): look check. Same frames, same camera: mean level per region of each clip
against a reference clip, and a crop sheet (frame k of each clip, plus |diff| x8 against the reference).

python look4.py <out.png> <ref.mkv>:<label> <clip.mkv>:<label> ... [--frame 50] [--box x,y,w,h] [--from 15]
"""
import argparse, os, sys
import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit")
import subprocess

FF = "C:/ffmpeg/bin/ffmpeg.exe"
REGIONS = {
    "bay": (0.56, 0.70, 0.72, 0.97),
    "valley": (0.54, 0.64, 0.58, 0.68),
    "farhill": (0.45, 0.53, 0.25, 0.55),
    "haze": (0.47, 0.55, 0.62, 0.97),
    "slope": (0.56, 0.68, 0.30, 0.48),
    "sky": (0.20, 0.35, 0.55, 0.95),
}


def decode_rgb(src, a, b):
    cmd = [FF, "-v", "error", "-i", src, "-vf", f"select='between(n\\,{a}\\,{b - 1})'", "-vsync", "0",
           "-f", "rawvideo", "-pix_fmt", "rgb24", "-"]
    raw = subprocess.run(cmd, capture_output=True, check=True).stdout
    n = len(raw) // (1920 * 1080 * 3)
    return np.frombuffer(raw[: n * 1920 * 1080 * 3], np.uint8).reshape(n, 1080, 1920, 3)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--frame", type=int, default=50)
    ap.add_argument("--box", default="850,470,900,320")
    ap.add_argument("--from", dest="a", type=int, default=15)
    ap.add_argument("--to", dest="b", type=int, default=120)
    o = ap.parse_args()
    x, y, w, h = [int(v) for v in o.box.split(",")]
    clips = [c.rsplit(":", 1) for c in o.clips]
    means, crops = [], []
    for path, lab in clips:
        fr = decode_rgb(path, o.a, o.b).astype(np.float32)
        means.append(fr.mean(axis=0))
        k = o.frame - o.a
        crops.append((lab, fr[k, y:y + h, x:x + w]))
    ref = means[0]
    print(f"look vs {clips[0][1]} (mean over frames {o.a}..{o.b - 1}; level diff in code values, luma)")
    print("clip".ljust(14) + "".join(r.rjust(9) for r in REGIONS))
    for (path, lab), m in zip(clips, means):
        row = lab.ljust(14)
        for r, (y0, y1, x0, x1) in REGIONS.items():
            sl = (slice(int(y0 * 1080), int(y1 * 1080)), slice(int(x0 * 1920), int(x1 * 1920)))
            lum = lambda im: (im[sl] @ np.array([0.2126, 0.7152, 0.0722])).mean()
            row += f"{lum(m) - lum(ref):9.2f}" if m is not ref else f"{lum(m):9.1f}"
        print(row)
    rows = []
    for lab, c in crops:
        d = np.clip(np.abs(c - crops[0][1]) * 8, 0, 255)
        rows.append(np.concatenate([c, d], axis=1).astype(np.uint8))
    sheet = np.concatenate(rows, axis=0)
    im = Image.fromarray(sheet)
    dr = ImageDraw.Draw(im)
    for i, (lab, _) in enumerate(crops):
        dr.rectangle([0, i * h, 8 * len(lab) + 8, i * h + 14], fill=(0, 0, 0))
        dr.text((4, i * h + 2), lab, fill=(255, 255, 0))
        dr.text((w + 4, i * h + 2), "|diff vs ref| x8", fill=(255, 255, 0))
    im.save(o.out)
    print("wrote", o.out)


if __name__ == "__main__":
    main()
