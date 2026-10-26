"""Motion strip + GIF for the temporal accumulation (night run, added-light debug view).

python motion.py <run dir> <view> <prefix_t> <prefix_n> <out stem>

Frames <view>__<prefix>0..6: 0 is the settled view, 1..6 strafe 0.3 m right per
frame. Left = temporal accumulation on, right = off (single-frame trace + blur).
Prints a high-frequency noise measure per frame (std of image minus its 5x5 box
blur, 8-bit levels) for both.
"""
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter

sys.path.insert(0, str(Path(__file__).resolve().parent))
from compare import font, hstack, panel  # noqa: E402


def load(run, view, label):
    for png in run.glob("tri/**/*.png"):
        m = re.match(r"\d+_(.+?)__(.+)\.png$", png.name)
        if m and m.group(1) == view and m.group(2) == label:
            return Image.open(png).convert("RGB")
    raise SystemExit(f"missing {label}")


def noise(img):
    a = np.asarray(img).astype(float).mean(axis=2)
    b = np.asarray(img.filter(ImageFilter.BoxBlur(2))).astype(float).mean(axis=2)
    return float((a - b).std())


def main():
    run, view, pt, pn, stem = Path(sys.argv[1]), sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]
    f1, f2 = font(16), font(14)
    frames, rows = [], []
    for i in range(7):
        t = load(run, view, f"{pt}{i}")
        n = load(run, view, f"{pn}{i}")
        rows.append((i, noise(t), noise(n)))
        pair = hstack([
            panel(t.resize((640, 427)), f"temporal ON  frame {i}", None, f1, f2),
            panel(n.resize((640, 427)), f"temporal OFF frame {i}", None, f1, f2),
        ])
        frames.append(pair)
    frames[0].save(f"{stem}.gif", save_all=True, append_images=frames[1:], duration=400, loop=0)
    strip = Image.new("RGB", (frames[0].size[0], frames[0].size[1] * 4), (0, 0, 0))
    for k, i in enumerate((0, 2, 4, 6)):
        strip.paste(frames[i], (0, k * frames[0].size[1]))
    strip.save(f"{stem}_strip.png")
    for i, a, b in rows:
        print(f"frame {i}: hf-noise temporal {a:.2f}  no-temporal {b:.2f}")


if __name__ == "__main__":
    main()
