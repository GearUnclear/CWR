"""Three-way (or four-way) comparison sheets from a collected run.

    python make_best.py <run_dir> <out_dir> view=variant[+variant2][@x0,y0,x1,y1] ...

For each argument writes <out_dir>/BEST_<view>.png: Classic | Remastered
(current) | Remastered + foliage <variant> [| <variant2>], full frames, GPU frame
ms on each panel (from the run's UDCAP shot lines), and, when a crop box is
given, a second row with the same crop of each panel at 2x.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
from collect import collect, font, label_box  # noqa: E402

TITLES = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "vH2": "Remastered + volumetric crowns (vH2)",
    "vJ": "Remastered + volumetric crowns, mild (vJ)",
    "vHc": "vH2 + canopy map (vHc, C++)",
    "vHs": "vH2 + canopy sky occlusion (vHs, C++)",
    "vG": "Remastered + volumetric crowns (vG)",
    "vH": "Remastered + volumetric crowns + sky rim (vH)",
    "vI": "Remastered + volumetric crowns, strong (vI)",
}


def panel(run, view, lab, rows, crop=None):
    im = Image.open(run / f"{view}__{lab}.png").convert("RGB")
    if crop:
        im = im.crop(crop)
        s = 960 / im.width
        im = im.resize((960, int(im.height * s)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    label_box(d, (10, 8), TITLES.get(lab, lab) + (" (zoom)" if crop else ""), font(19))
    r = rows.get((view, lab))
    if r and r["frame"] is not None and not crop:
        label_box(d, (10, im.height - 26), f"GPU frame {r['frame']:.2f} ms (world {r['world']:.2f})", font(14))
    return im


def main():
    run, out = Path(sys.argv[1]), Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    _, _, rows = collect(run)
    for spec in sys.argv[3:]:
        view, rest = spec.split("=", 1)
        crop = None
        if "@" in rest:
            rest, box = rest.split("@", 1)
            crop = tuple(int(v) for v in box.split(","))
        labs = ["classic", "remastered"] + rest.split("+")
        top = [panel(run, view, l, rows) for l in labs]
        bottom = [panel(run, view, l, rows, crop) for l in labs] if crop else []
        w, h = top[0].size
        hb = bottom[0].height if bottom else 0
        gap, th = 6, 36
        sheet = Image.new("RGB", (len(labs) * w + (len(labs) - 1) * gap, th + h + (hb + gap if bottom else 0)),
                          (18, 18, 18))
        ImageDraw.Draw(sheet).text((8, 7), f"{view} ({run.name}): " + " | ".join(TITLES.get(l, l) for l in labs),
                                   font=font(20), fill=(235, 235, 235))
        for i, im in enumerate(top):
            sheet.paste(im, (i * (w + gap), th))
        for i, im in enumerate(bottom):
            sheet.paste(im, (i * (w + gap), th + h + gap))
        dst = out / f"BEST_{view}.png"
        sheet.save(dst)
        print(dst)


if __name__ == "__main__":
    main()
