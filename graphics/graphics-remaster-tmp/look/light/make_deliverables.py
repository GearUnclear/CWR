"""Copy the final run into tmp/look/light and build the sheets, pairs and the
timing summary.

    python make_deliverables.py <final run dir> <timing run dir> <variant sheets dir>
"""
import json
import shutil
import statistics
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).resolve().parent
HERO = ["lowsun_fp", "noon_fields", "forest_edge_golden", "sunrise_ridge", "coastline_sunset", "overcast_broken",
        "town_night", "sinai_desert_noon"]
NEW = ["woods_pine2_evening", "blue_hour_coast"]


def font(size):
    for name in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def main():
    run, timing, variants = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    final = OUT / "final"
    final.mkdir(exist_ok=True)
    for v in HERO + NEW:
        for m in ("classic", "off", "on"):
            src = run / f"{v}__{m}.png"
            if src.exists():
                shutil.copy2(src, final / src.name)
    for name in ("timings.md", "timings.csv", "udcap.log"):
        if (run / name).exists():
            shutil.copy2(run / name, final / name)
    # Pairs: Original | Overhaul, 960 px a panel.
    f = font(26)
    for v in HERO + NEW:
        a, b = final / f"{v}__classic.png", final / f"{v}__on.png"
        if not (a.exists() and b.exists()):
            continue
        pa = Image.open(a).convert("RGB").resize((960, 540), Image.LANCZOS)
        pb = Image.open(b).convert("RGB").resize((960, 540), Image.LANCZOS)
        s = Image.new("RGB", (1924, 540), (20, 20, 20))
        s.paste(pa, (0, 0))
        s.paste(pb, (964, 0))
        d = ImageDraw.Draw(s)
        for x, t in ((10, "Original (Classic)"), (974, "Overhaul (photoLight)")):
            d.rectangle([x - 4, 6, x + 270, 40], fill=(0, 0, 0))
            d.text((x, 8), t, fill=(255, 255, 255), font=f)
        s.save(final / f"{v}__pair.png")
    # Timing: off and on interleaved three times per view.
    rows = json.loads((timing / "timings.json").read_text(encoding="utf-8"))
    by = {}
    for r in rows:
        kind = "on" if r["mode"].startswith("on") else "off"
        by.setdefault((r["view"], kind), []).append(r)
    lines = ["# GPU time, photoLight off and on (interleaved three times per view)", "",
             "RTX 4060 Ti, 1920x1080, scale 1.0, 4x MSAA, TAA on; `triGpuTimes` smoothed over 150 frames per shot. "
             "One to three other agents' PoseidonGame processes ran on the same GPU during the run "
             "(gpu_timing.csv), so the frame and world numbers swing; the minimum of the three rounds is the "
             "least disturbed. The camera's own work is in the post sections (composite: the meter pass, its "
             "readback and the camera's composite).", "",
             "| view | mode | frame min | frame med | world min | post min | post med | composite min | composite med |",
             "|---|---|---|---|---|---|---|---|---|"]
    for (v, k), rs in sorted(by.items()):
        def mm(field):
            vals = [x[field] for x in rs if x.get(field) is not None]
            if not vals:
                return "-", "-"
            return f"{min(vals):.2f}", f"{statistics.median(vals):.2f}"
        fr, wo, po, co = mm("gpu_frame_ms"), mm("gpu_world_ms"), mm("gpu_post_ms"), mm("gpu_composite_ms")
        lines.append(f"| {v} | {k} | {fr[0]} | {fr[1]} | {wo[0]} | {po[0]} | {po[1]} | {co[0]} | {co[1]} |")
    (OUT / "timing.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    for p in variants.glob("sheet_t*.png"):
        shutil.copy2(p, OUT / ("variants_" + p.name))
    print("ok")


if __name__ == "__main__":
    main()
