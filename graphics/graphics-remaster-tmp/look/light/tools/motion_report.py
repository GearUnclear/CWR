"""The motion captures (gen_look_motion.py): per frame the camera's exposure
and target from the log, per screenshot the frame's mean luma; a strip of the
screenshots (on above off) and a plot of exposure against frame.

    python motion_report.py <run dir> <out dir>
"""
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

run, out = Path(sys.argv[1]), Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
log = (run / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines()
data = {}
for line in log:
    if "event=motion" not in line:
        continue
    mv = re.search(r"move=(\w+);mode=(\w+);frame=(\d+);offset=([-0-9.]+)", line)
    ev = re.search(r"\| cam ev=([-0-9.]+) target=([-0-9.]+)", line)
    key = (mv.group(1), mv.group(2))
    data.setdefault(key, []).append((int(mv.group(3)), float(mv.group(4)), float(ev.group(1)) if ev else None,
                                     float(ev.group(2)) if ev else None))


def font(size):
    for name in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


report = []
for move in sorted({k[0] for k in data}):
    shots = {}
    for mode in ("on", "off"):
        files = sorted(run.glob(f"{move}__{mode}_*.png"))
        shots[mode] = files
    # Exposure track (on).
    rows = data.get((move, "on"), [])
    evs = [r[2] for r in rows if r[2] is not None]
    steps = np.abs(np.diff(evs)) if len(evs) > 1 else np.array([0.0])
    report.append(f"{move}: {len(rows)} frames, ev {min(evs):.2f}..{max(evs):.2f}, largest step a frame "
                  f"{steps.max():.3f} EV, median step {np.median(steps):.4f} EV, last 30 frames ev range "
                  f"{(max(evs[-30:]) - min(evs[-30:])):.3f} EV")
    # Luma of each screenshot, and the change between neighbours, on and off.
    for mode in ("on", "off"):
        lum = []
        for p in shots[mode]:
            a = np.asarray(Image.open(p).convert("RGB")).astype(np.float32)
            lum.append(float((0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]).mean()))
        if lum:
            report.append(f"  {mode}: screenshot mean luma " + " ".join(f"{x:.0f}" for x in lum))
    # Strip: on above off, 240 px a frame.
    w, h = 240, 135
    cols = max(len(shots["on"]), len(shots["off"]))
    strip = Image.new("RGB", (cols * (w + 2), 2 * (h + 2) + 20), (20, 20, 20))
    d = ImageDraw.Draw(strip)
    d.text((4, 2), f"{move}: photoLight on (top), off (bottom), every 10 frames", fill=(255, 255, 255), font=font(14))
    for row, mode in enumerate(("on", "off")):
        for i, p in enumerate(shots[mode]):
            im = Image.open(p).convert("RGB").resize((w, h), Image.LANCZOS)
            strip.paste(im, (i * (w + 2), 20 + row * (h + 2)))
    strip.save(out / f"motion_{move}_strip.png")
    # Plot: exposure and target against frame (on).
    pw, ph = 900, 260
    plot = Image.new("RGB", (pw, ph), (250, 250, 250))
    d = ImageDraw.Draw(plot)
    if rows:
        lo = min(min(r[2] for r in rows), min(r[3] for r in rows)) - 0.2
        hi = max(max(r[2] for r in rows), max(r[3] for r in rows)) + 0.2
        n = len(rows)

        def xy(i, v):
            return 40 + (pw - 60) * i / max(n - 1, 1), ph - 30 - (ph - 50) * (v - lo) / max(hi - lo, 1e-6)

        for col, idx in (((200, 120, 0), 3), ((0, 70, 200), 2)):
            pts = [xy(i, r[idx]) for i, r in enumerate(rows)]
            d.line(pts, fill=col, width=2)
        d.text((44, 6), f"{move}: exposure (blue) and target (orange), log2, per frame; {lo:.1f}..{hi:.1f}",
               fill=(0, 0, 0), font=font(14))
    plot.save(out / f"motion_{move}_ev.png")
(out / "motion.txt").write_text("\n".join(report) + "\n", encoding="utf-8")
print("\n".join(report))
