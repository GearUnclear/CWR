"""B5 lane image tools.

    python b5img.py strip <out.png> <img> [<img> ...] [--labels a,b,c] [--w 960] [--crop l,t,r,b]
    python b5img.py metrics <run dir> [<run dir> ...]  (water region per view, markdown on stdout)
    python b5img.py motion <run dir>                   (frame-to-frame change of the water region)
    python b5img.py diff <imgA> <imgB> [--box l,t,r,b] (mean / max / count over 2 levels)

Water region per view (1920x1080 captures): the rows below the horizon, the
beach strip at the bottom left out where the view shows it.
"""
import colorsys
import json
import math
import sys
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageStat

WATER = {
    "water_coastline_sunset": (0, 520, 1920, 1080),
    "water_sea_noon": (0, 330, 1920, 1080),
    "water_shore_midsun": (0, 120, 1920, 1080),
    "water_night": (0, 520, 1920, 1080),
    "water_motion": (0, 520, 1920, 1080),
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def opt(args, name, default=None):
    if name in args:
        i = args.index(name)
        v = args[i + 1]
        del args[i:i + 2]
        return v
    return default


def strip(args):
    labels = opt(args, "--labels")
    w = int(opt(args, "--w", "960"))
    crop = opt(args, "--crop")
    out, imgs = args[0], args[1:]
    labels = labels.split(",") if labels else [Path(p).stem for p in imgs]
    panels = []
    for p in imgs:
        im = Image.open(p).convert("RGB")
        if crop:
            im = im.crop(tuple(int(x) for x in crop.split(",")))
        h = round(im.height * w / im.width)
        panels.append(im.resize((w, h), Image.LANCZOS))
    gap = 6
    H = max(p.height for p in panels)
    sheet = Image.new("RGB", (len(panels) * w + gap * (len(panels) - 1), H), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    f = font(18)
    for i, (p, lab) in enumerate(zip(panels, labels)):
        x = i * (w + gap)
        sheet.paste(p, (x, 0))
        box = d.textbbox((x + 8, 6), lab, font=f)
        d.rectangle([box[0] - 4, box[1] - 3, box[2] + 4, box[3] + 3], fill=(0, 0, 0))
        d.text((x + 8, 6), lab, font=f, fill=(255, 255, 255))
    sheet.save(out)
    print(out, sheet.size)


def region_stats(img, box):
    rgb = img.convert("RGB").crop(box)
    st = ImageStat.Stat(rgb)
    r, g, b = st.mean
    h, l, s = colorsys.rgb_to_hls(r / 255, g / 255, b / 255)
    gray = rgb.convert("L")
    mean = ImageStat.Stat(gray).mean[0]
    blur = gray.filter(ImageFilter.BoxBlur(4))
    fine = ImageStat.Stat(ImageChops.subtract(gray, blur, 1.0, 128)).stddev[0]
    wave = ImageStat.Stat(gray.filter(ImageFilter.BoxBlur(2))).stddev[0]
    return dict(mean=mean, rgb=(r, g, b), hue=h * 360.0, sat=s, fine=fine, wave=wave)


def hue_diff(a, b):
    d = abs(a - b) % 360.0
    return min(d, 360.0 - d)


def shots(run, view):
    """label -> png of one view: the run folder's copies, else tri's tree."""
    out = {}
    for p in sorted(Path(run, "tri").rglob(f"*_{view}__*.png")):
        out[p.stem.split(f"{view}__", 1)[1]] = p
    for p in sorted(Path(run).glob(f"{view}__*.png")):
        lab = p.stem.split(f"{view}__", 1)[1]
        if lab != "pair":
            out[lab] = p
    return out


def metrics(args):
    rows = ["| run | view | shot | mean | rgb | hue deg | hue vs classic | sat | fine sd | wave sd |",
            "|---|---|---|---|---|---|---|---|---|---|"]
    for run in args:
        run = Path(run)
        for view, box in WATER.items():
            if view == "water_motion":
                continue
            stats = {}
            found = shots(run, view)
            order = ["classic", "off", "on"] + sorted(k for k in found if k not in ("classic", "off", "on"))
            for shot in order:
                if shot in found:
                    stats[shot] = region_stats(Image.open(found[shot]), box)
            for shot, s in stats.items():
                hv = f"{hue_diff(s['hue'], stats['classic']['hue']):.1f}" if "classic" in stats else ""
                rgb = ", ".join(f"{c:.0f}" for c in s["rgb"])
                rows.append(f"| {run.name} | {view} | {shot} | {s['mean']:.1f} | {rgb} | {s['hue']:.1f} | {hv} | "
                            f"{s['sat']:.3f} | {s['fine']:.2f} | {s['wave']:.2f} |")
    print("\n".join(rows))


def frames(run, label):
    tri = Path(run) / "tri"
    out = {}
    for p in tri.rglob(f"*_water_motion__{label}__f*.png"):
        k = int(p.stem.rsplit("__f", 1)[1])
        out[k] = p
    return [out[k] for k in sorted(out)]


def motion(args):
    run = args[0]
    box = WATER["water_motion"]
    res = {}
    for label in ("off", "on"):
        fs = frames(run, label)
        prev = None
        ds = []
        for p in fs:
            im = Image.open(p).convert("L").crop(box)
            if prev is not None:
                ds.append(ImageStat.Stat(ImageChops.difference(im, prev)).mean[0])
            prev = im
        res[label] = ds
        if ds:
            mean = sum(ds) / len(ds)
            sd = math.sqrt(sum((d - mean) ** 2 for d in ds) / len(ds))
            srt = sorted(ds)
            med = srt[len(srt) // 2]
            print(f"{label}: frames={len(fs)} mean={mean:.3f} sd={sd:.3f} cv={sd / max(mean, 1e-9):.2f} "
                  f"median={med:.3f} max={max(ds):.3f} max/median={max(ds) / max(med, 1e-9):.2f}")
            print("   " + " ".join(f"{d:.2f}" for d in ds))
    (Path(run) / "motion.json").write_text(json.dumps(res, indent=1), encoding="utf-8")


def diff(args):
    box = opt(args, "--box")
    a = Image.open(args[0]).convert("RGB")
    b = Image.open(args[1]).convert("RGB")
    if box:
        bx = tuple(int(x) for x in box.split(","))
        a, b = a.crop(bx), b.crop(bx)
    d = ImageChops.difference(a, b)
    px = list(d.get_flattened_data()) if hasattr(d, "get_flattened_data") else list(d.getdata())
    m = [max(p) for p in px]
    over = sum(1 for v in m if v > 2)
    print(f"mean={sum(m) / len(m):.4f} max={max(m)} over2={over} of {len(m)}")


if __name__ == "__main__":
    cmd, rest = sys.argv[1], sys.argv[2:]
    {"strip": strip, "metrics": metrics, "motion": motion, "diff": diff}[cmd](rest)
