"""Puts the frames of motion_render.py together: one strip per move and camera, with captions.

  python motion_sheet.py <frames dir> <out.png> [--moves walk,stomp] [--cams quarter,side] [--cell 256]
                         [--title text] [--gif walk.gif]

Each row is one move seen from one camera, phases left to right. --gif writes the turntable frames
(turntable_NN.png) as an animated GIF beside the sheet.
"""
import argparse
import glob
import os
import re

from PIL import Image, ImageDraw, ImageFont

CAPTION = {
    "idle": "DedIdle: breath and a slow shift of weight",
    "walk": "DedWalk: one cycle, eight phases",
    "turnl": "DedTurnL: stepping round to his left",
    "turnr": "DedTurnR: stepping round to his right",
    "roar": "DedRoar: curl down, head thrown back, the cry, sag",
    "beamrise": "DedBeamRise: the stoop unrolls, the ribs spread",
    "beamhold": "DedBeamHold: rigid, a fine tremor",
    "beamlower": "DedBeamLower: lets go, sinks, recovers",
    "stomp": "DedStomp: weight over, foot up, driven down, home",
    "swat": "DedSwat: arm wound back, carried across the front",
    "flinch": "DedFlinch: a shudder, feet planted",
    "dormant": "DedDormant: deeper stoop, slow breath",
    "die": "DedDying: he goes over onto his chest; head, arms and hanging parts give way",
}
CAMNAME = {"quarter": "front right quarter", "quarterl": "front left quarter", "side": "his right side",
           "sidel": "his left side", "front": "front", "back": "behind", "backq": "behind his right",
           "feet": "low, at the right foot", "feetl": "low, at the left foot", "hip": "the right hip from behind",
           "waist": "the waist from in front", "man": "a man's eye height"}


def font(size):
    for name in ("segoeui.ttf", "arial.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("frames")
    ap.add_argument("out")
    ap.add_argument("--moves", default="")
    ap.add_argument("--cams", default="")
    ap.add_argument("--cell", type=int, default=256)
    ap.add_argument("--title", default="")
    ap.add_argument("--gif", default="")
    a = ap.parse_args()
    rows = {}
    for p in sorted(glob.glob(os.path.join(a.frames, "*.png"))):
        m = re.match(r"([a-z]+)_([a-z]+)_(\d+)\.png$", os.path.basename(p))
        if m and m.group(1) != "turntable":
            rows.setdefault((m.group(1), m.group(2)), []).append(p)
    moves = [m for m in a.moves.split(",") if m] or list(CAPTION)
    cams = [c for c in a.cams.split(",") if c]
    keys = [(m, c) for m in moves for c in (cams or sorted({k[1] for k in rows if k[0] == m})) if (m, c) in rows]
    cell = a.cell
    ncol = max(len(rows[k]) for k in keys)
    cap_h = 20
    head = 34 if a.title else 0
    sheet = Image.new("RGB", (ncol * cell, head + len(keys) * (cell + cap_h)), (22, 23, 26))
    d = ImageDraw.Draw(sheet)
    if a.title:
        d.text((8, 6), a.title, fill=(235, 235, 235), font=font(18))
    f = font(13)
    for r, k in enumerate(keys):
        y = head + r * (cell + cap_h)
        for c, p in enumerate(rows[k]):
            im = Image.open(p).convert("RGB").resize((cell, cell), Image.LANCZOS)
            sheet.paste(im, (c * cell, y))
        d.text((6, y + cell + 2), "%s. Camera: %s." % (CAPTION.get(k[0], k[0]), CAMNAME.get(k[1], k[1])), fill=(210, 210, 210), font=f)
    sheet.save(a.out)
    print("wrote", a.out, sheet.size)
    if a.gif:
        tt = sorted(glob.glob(os.path.join(a.frames, "turntable_*.png")))
        if tt:
            ims = [Image.open(p).convert("P", palette=Image.ADAPTIVE, colors=128) for p in tt]
            ims[0].save(a.gif, save_all=True, append_images=ims[1:], duration=220, loop=0)
            print("wrote", a.gif, len(ims))


if __name__ == "__main__":
    main()
