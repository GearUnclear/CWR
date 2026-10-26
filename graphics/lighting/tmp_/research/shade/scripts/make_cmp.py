"""Labelled side-by-side comparison strips for the shade track.

    python make_cmp.py OUT.png "Title" PANEL [PANEL ...]

PANEL = run_dir|view|label[|caption[|gpu_run|gpu_label]] (| separated: paths hold ':').
gpu_run/gpu_label read the GPU time of another (timed) shot, captioned as such. The GPU frame time of each panel is read
from that run's udcap.log (the event=shot line of that label); variant shots
that were not timed show "GPU: not timed (same cost as <timed label>)" only if
the caller passes it in the caption. Panels are laid out left to right at full
960x640, two rows when there are more than three.
"""
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ANSI = re.compile(r"\x1b\[[0-9;]*m")


def font(size):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def gpu_ms(run, view, label):
    log = Path(run) / "udcap.log"
    if not log.exists():
        return None
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        line = ANSI.sub("", line)
        if f"view={view};event=shot;label={view}__{label};" in line:
            m = re.search(r"gpu=frame ([\d.]+) ms", line)
            if m:
                return float(m.group(1))
    return None


def main():
    out, title, panels = sys.argv[1], sys.argv[2], sys.argv[3:]
    ims = []
    for p in panels:
        parts = p.split("|")
        run, view, label = parts[0], parts[1], parts[2]
        cap = parts[3] if len(parts) > 3 else label
        im = Image.open(Path(run) / f"{view}__{label}.png").convert("RGB")
        if len(parts) > 5:
            ms = gpu_ms(parts[4], view, parts[5])
            note = f" (timed shot: {parts[5]})"
        else:
            ms = gpu_ms(run, view, label)
            note = ""
        ims.append((im, cap, ms, note))
    cols = len(ims) if len(ims) <= 3 else (len(ims) + 1) // 2
    rows = 1 if len(ims) <= 3 else 2
    W, H = ims[0][0].size
    pad, top = 6, 40
    sheet = Image.new("RGB", (cols * W + (cols - 1) * pad, top + rows * H + (rows - 1) * pad), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((10, 6), title, fill=(235, 235, 235), font=font(22))
    f = font(18)
    for i, (im, cap, ms, note) in enumerate(ims):
        x = (i % cols) * (W + pad)
        y = top + (i // cols) * (H + pad)
        sheet.paste(im, (x, y))
        tw = d.textlength(cap, font=f)
        d.rectangle((x + 6, y + 6, x + 14 + tw, y + 32), fill=(0, 0, 0))
        d.text((x + 10, y + 8), cap, fill=(255, 255, 255), font=f)
        g = f"GPU frame {ms:.2f} ms{note}" if ms is not None else "GPU frame: not timed"
        tw = d.textlength(g, font=f)
        d.rectangle((x + 6, y + H - 32, x + 14 + tw, y + H - 6), fill=(0, 0, 0))
        d.text((x + 10, y + H - 30), g, fill=(255, 255, 255), font=f)
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
