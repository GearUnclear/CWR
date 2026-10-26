"""Strafe GIF + strip for the v8 headlights_bounce run: camera 0.25 m right per
frame for 8 frames. Left = Remastered (gi=0), right = + SSGI x1 (hue albedo,
temporal accumulation on). Prints the M113 region mean per frame for both and a
high-frequency noise measure on the hull (std of image minus its 5x5 box blur).

python motion_v8.py <run dir> <out stem>
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter

sys.path.insert(0, str(Path(__file__).resolve().parent))
from compare import font, hstack, panel  # noqa: E402
from motion import load  # noqa: E402


def hull_stats(img):
    a = np.asarray(img).astype(float).mean(axis=2)[230:420, 0:330]
    b = np.asarray(img.filter(ImageFilter.BoxBlur(2))).astype(float).mean(axis=2)[230:420, 0:330]
    return a.mean(), float((a - b).std())


def main():
    run, stem = Path(sys.argv[1]), sys.argv[2]
    v = "headlights_bounce"
    f1, f2 = font(16), font(14)
    frames = []
    for i in range(9):
        r = load(run, v, f"strafe_rem{i}")
        g = load(run, v, f"strafe_m1s1_chroma{i}")
        (rm, rn), (gm, gn) = hull_stats(r), hull_stats(g)
        print(f"frame {i}: hull mean rem {rm:.1f} gi {gm:.1f} | hull hf-noise rem {rn:.2f} gi {gn:.2f}")
        frames.append(hstack([
            panel(r.resize((640, 427)), f"Remastered  frame {i}", None, f1, f2),
            panel(g.resize((640, 427)), f"+SSGI x1 temporal  frame {i}", None, f1, f2),
        ]))
    frames[0].save(f"{stem}.gif", save_all=True, append_images=frames[1:], duration=300, loop=0)
    strip = Image.new("RGB", (frames[0].size[0], frames[0].size[1] * 3), (0, 0, 0))
    for k, i in enumerate((0, 4, 8)):
        strip.paste(frames[i], (0, k * frames[0].size[1]))
    strip.save(f"{stem}_strip.png")


if __name__ == "__main__":
    main()
