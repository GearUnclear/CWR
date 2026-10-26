"""Side-by-side animated GIF of a captured sequence (one panel per mode).

    python make_gif.py <run_dir> <view> <modes> <suffixes> <out.gif> [--ms 220] [--names "c=Classic;x=..."]

Example: python make_gif.py run6 ffh crx 0,b01,b02,b03,b04,b05,b06,b08,b10,b12 ffh_boom.gif
The frames are slowed down: the shots are 1 to 15 sim frames (about 11 ms
each at the capture's frame rate) apart, the GIF shows each for --ms.
"""
import argparse
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
import collect as C  # noqa: E402

NAMES = {"c": "Classic", "r": "Remastered (current)", "x": "Remastered + combat lights", "o": "combat lights, run 5",
         "w": "combat lights, no shadow", "s": "round 6 (smoke 1)"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("modes")
    ap.add_argument("suffixes")
    ap.add_argument("out")
    ap.add_argument("--ms", type=int, default=220)
    ap.add_argument("--scale", type=float, default=0.5)
    ap.add_argument("--names", default="")
    a = ap.parse_args()
    names = dict(NAMES)
    for kv in filter(None, a.names.split(";")):
        k, v = kv.split("=", 1)
        names[k] = v
    run = Path(a.run)
    frames = []
    for suf in a.suffixes.split(","):
        panels = [C.panel(run / "shots" / f"{a.view}_{m}_{suf}.png", names[m], suf, a.scale) for m in a.modes]
        w, h = panels[0].size
        im = Image.new("RGB", (len(panels) * w + (len(panels) - 1) * 4, h), (24, 24, 24))
        for i, p in enumerate(panels):
            im.paste(p, (i * (w + 4), 0))
        frames.append(im.convert("P", palette=Image.ADAPTIVE, colors=255))
    frames[0].save(run / a.out, save_all=True, append_images=frames[1:], duration=a.ms, loop=0)
    print("wrote", run / a.out, len(frames), "frames")


if __name__ == "__main__":
    main()
