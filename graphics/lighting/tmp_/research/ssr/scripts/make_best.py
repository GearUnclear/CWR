"""Copy the chosen three-way comparisons of a run to BEST_<view>.png and build a GIF of the
bay_offshore motion sequence.

    python make_best.py <run_dir> <out_dir>
"""
import sys, shutil
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

run, out = Path(sys.argv[1]), Path(sys.argv[2])
best = {
    "bay_offshore": "ssrcalm",
    "town_rain": "ssrwet",
    "town_wet_dusk": "ssrwet",
    "coastline_sunset": "ssrw06",
    "overcast_heavy_rain": "ssr",
    "lowsun_fp": "ssrwet",
}
for view, var in best.items():
    src = run / f"cmp_{view}__{var}.png"
    if src.exists():
        shutil.copy2(src, out / f"BEST_{view}.png")
        print("BEST", view, var)
    else:
        print("missing", src)

shots = run / "shots"
frames = [shots / f"bay_offshore__seq{k}.png" for k in range(4)]
if all(f.exists() for f in frames):
    try:
        font = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 18)
    except OSError:
        font = ImageFont.load_default()
    ims = []
    for k, f in enumerate(frames):
        im = Image.open(f).convert("RGB")
        d = ImageDraw.Draw(im)
        t = f"Remastered + SSR (calm water), sim t + {k * 0.25:.2f} s"
        bb = d.textbbox((0, 0), t, font=font)
        d.rectangle((6, 6, 14 + bb[2], 12 + bb[3]), fill=(0, 0, 0))
        d.text((10, 6), t, fill=(255, 255, 255), font=font)
        ims.append(im.convert("P", palette=Image.ADAPTIVE, colors=255))
    g = out / "bay_offshore_sequence.gif"
    ims[0].save(g, save_all=True, append_images=ims[1:], duration=250, loop=0)
    print("GIF", g)
