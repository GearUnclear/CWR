"""Motion strip + GIF from the nrm_abel_seq shots (research only).

    python strip.py <run_dir> [--crop x0,y0,x1,y1] [--scale 1.5]

Rows = variants (cur, v8cav, v9bold), columns = frames 0..3 (eye walking
forward 0.35 m per frame). Writes <run_dir>/lowsun_fp__motion_strip.png and
one GIF per variant (lowsun_fp__motion_<variant>.gif) of the crop.
"""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw

import compare as cmp

NAMES = {"cur": "Remastered (as shipped)", "v8cav": "+ normal map, micro shadows, cavities", "v9bold": "+ all x2",
         "v1ss": "+ screen-space luminance bump (V1)", "v6mean": "+ all x2, mean-preserving (V6)"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--crop", default="480,340,960,640")
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--variants", default="cur,v8cav,v9bold")
    ap.add_argument("--name", default="motion")
    ap.add_argument("--step", default="0.35")
    a = ap.parse_args()
    run = Path(a.run)
    cmp.collect(run)
    crop = tuple(int(x) for x in a.crop.split(","))
    shots = run / "shots"
    variants = [v for v in a.variants.split(",") if (shots / f"lowsun_fp__seq_{v}_0.png").exists()]
    frames = sorted({int(p.stem.rsplit("_", 1)[1]) for p in shots.glob("lowsun_fp__seq_cur_*.png")})
    tiles = {}
    for v in variants:
        for f in frames:
            im = Image.open(shots / f"lowsun_fp__seq_{v}_{f}.png").convert("RGB").crop(crop)
            if a.scale != 1.0:
                im = im.resize((int(im.width * a.scale), int(im.height * a.scale)), Image.LANCZOS)
            tiles[(v, f)] = im
    w, h = next(iter(tiles.values())).size
    gap, title = 6, 34
    out = Image.new("RGB", (len(frames) * (w + gap) - gap, len(variants) * (h + gap + 24) + title), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((8, 6), f"lowsun_fp motion strip: eye walks {a.step} m forward per frame (crop {crop})", font=cmp.font(20), fill=(235, 235, 235))
    for r, v in enumerate(variants):
        y = title + r * (h + gap + 24)
        d.text((8, y + 2), NAMES.get(v, v), font=cmp.font(17), fill=(235, 235, 235))
        for c, f in enumerate(frames):
            out.paste(tiles[(v, f)], (c * (w + gap), y + 24))
    dst = run / f"lowsun_fp__{a.name}_strip.png"
    out.save(dst)
    print(dst)
    for v in variants:
        seq = [tiles[(v, f)] for f in frames]
        g = run / f"lowsun_fp__{a.name}_{v}.gif"
        seq[0].save(g, save_all=True, append_images=seq[1:], duration=300, loop=0)
        print(g)


if __name__ == "__main__":
    main()
