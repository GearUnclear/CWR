"""Three-way comparison (Classic | Remastered | Remastered + prototype) from one perf run.

usage: make_threeway.py <game_stdout.log> <shot dir> <out.png> <title> <regime> \
           <shot prefix> <cfgA>:<labelA> <cfgB>:<labelB> <cfgC>:<labelC> [--diff <png>]

Numbers on each panel are the medians over the interleaved rounds of <regime><r>_<cfg>
(GPU frame, GPU world, CPU frame, draw calls). Shots are <shot dir>/NNN_<prefix>_<cfg>.png.
With --diff, also writes an amplified |B - C| difference image (x8) and prints pixel counts.
"""
import statistics as st
import sys
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).parent))
from summarize_rounds import parse  # noqa: E402


def med(vals):
    vals = [v for v in vals if v is not None]
    return st.median(vals) if vals else None


def main():
    args = sys.argv[1:]
    diff_out = None
    if "--diff" in args:
        i = args.index("--diff")
        diff_out = args[i + 1]
        del args[i:i + 2]
    log, shotdir, out, title, regime, prefix = args[:6]
    panels = [a.split(":", 1) for a in args[6:]]
    rows = parse(log)
    ims, labels = [], []
    for cfg, label in panels:
        rs = [r for r in rows if r["cfg"].startswith(regime) and r["cfg"].split("_", 1)[-1] == cfg
              and r["cfg"][len(regime):].split("_", 1)[0].isdigit()]
        shots = sorted(Path(shotdir).glob(f"*_{prefix}_{cfg}.png"))
        if not shots:
            raise SystemExit(f"no shot for {prefix}_{cfg} in {shotdir}")
        ims.append(Image.open(shots[0]).convert("RGB"))
        gf, gw, cf, calls = (med([r[k] for r in rs]) for k in ("gframe", "gworld", "cframe", "calls"))
        wmin = med([r.get("xmin_world") for r in rs])
        line2 = f"draws {calls:.0f}" if calls is not None else ""
        line3 = (f"GPU frame {gf:.2f} ms (world {gw:.2f}, best {wmin:.2f})" if gf is not None else "")
        line4 = f"CPU frame {cf:.1f} ms  (medians of {len(rs)} rounds)" if cf is not None else ""
        labels.append([label, line2, line3, line4])
        print(cfg, label, line2, line3, line4, shots[0].name)
    w, h = ims[0].size
    gap, top = 8, 40
    W = len(ims) * w + (len(ims) - 1) * gap
    canvas = Image.new("RGB", (W, h + top), (24, 24, 24))
    d = ImageDraw.Draw(canvas)
    try:
        font = ImageFont.truetype("arialbd.ttf", 20)
        small = ImageFont.truetype("arialbd.ttf", 16)
    except OSError:
        font = small = ImageFont.load_default()
    d.text((10, 9), title, fill=(235, 235, 235), font=font)
    for i, (im, lines) in enumerate(zip(ims, labels)):
        x = i * (w + gap)
        canvas.paste(im, (x, top))
        tw = d.textlength(lines[0], font=font)
        d.rectangle([x + 6, top + 6, x + 14 + tw, top + 34], fill=(0, 0, 0))
        d.text((x + 10, top + 9), lines[0], fill=(255, 255, 255), font=font)
        extra = [s for s in lines[1:] if s]
        for j, s in enumerate(extra):
            y = top + h - 26 * (len(extra) - j) - 4
            ew = d.textlength(s, font=small)
            d.rectangle([x + 6, y - 2, x + 14 + ew, y + 22], fill=(0, 0, 0))
            d.text((x + 10, y), s, fill=(255, 255, 255), font=small)
    canvas.save(out)
    print("wrote", out, canvas.size)
    if diff_out and len(ims) >= 3:
        dif = ImageChops.difference(ims[1], ims[2])
        px = list(dif.getdata())
        m = [max(p) for p in px]
        print(f"B vs C: max {max(m)}, >2: {sum(1 for v in m if v > 2)}, >8: {sum(1 for v in m if v > 8)} of {len(m)}")
        dif.point(lambda v: min(255, v * 8)).save(diff_out)


if __name__ == "__main__":
    main()
