r"""Collects the frames of an integration run into kaiju-notes\integration\shots and makes a contact
sheet with captions.

    python make_sheet.py <run folder> <out folder> --tag dusk [--sheet demo_sheet.png] [--title "..."]
                         [--only 01,03b,...] [--cols 4]

<run folder> is one run's output (it is searched for *.png, the warm-up frame is skipped). Each
frame is copied to <out folder>\<tag>_<name>.png; its caption comes from the CAPTIONS table below
(by the frame's number prefix) and is written to <out folder>\CAPTIONS.txt (one block per tag,
replaced on a rerun). Needs Pillow.
"""
import argparse
import os
import re
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont

CAPTIONS = {
    "01": "GLOW. Dormant west of Morton, seen from 1.85 km out on the bay through a long lens, past the object distance: the engine's far draw (on the stock binary he is not drawn here).",
    "01c": "The same from 2.3 km, past the view distance (the far pass proper). By night his furnace is dark there: engine request 10.",
    "01b": "Dormant, from the road in behind him.",
    "02": "WAKE. He has risen out of dormancy; the furnace has come up.",
    "03": "THROUGH. He sets off toward the town.",
    "03b": "Walking in: every footfall has its dust, thump and shake; the houses in his line come down a beat before his toes.",
    "03c": "Among the houses at the west end of the row, from the garrison's street.",
    "04": "ROAR. From among the garrison, 55 m from his feet; the sections break and run.",
    "05": "FIRE. The armour fires on him (scripted damage credited to the tank, no weapon fired): he shudders, then turns to it.",
    "06": "BEAM. He has straightened; the furnace climbs to white through the charge.",
    "07": "The beam cuts the town, from the open ground north of the row. His feet stay planted; the line swings from the chest.",
    "07b": "The cut from 70 m up.",
    "07c": "The beam from behind the armour it was aimed at.",
    "08": "SPENT. He stands spent among the fires, the furnace a dull red (from the north).",
    "08b": "The town after the cut, from above.",
    "08c": "Spent, from the west end of the street, 27 m from his feet.",
    "L01": "LIVE FIRE (weapons fired). The first shots: a T-80's 120 mm HEAT, eight seconds after the garrison is told where he is.",
    "L02": "The square while he is hit (he is out of frame to the west, woken, stopped and shuddering): the riflemen lie in the grass and do not fire. Only the tanks do.",
    "L03": "He has answered with the beam on the armour that hurt him.",
    "b00": "A man stands inside his right leg and a live T-80 inside his left, the crush loop stopped: he does not move (the collision shape is lifted).",
    "b01": "STOMP, ordered through the brain: the foot of the clip is up over the tank.",
    "b02": "The blow: the tank under the foot is flattened, the man in the kill zone dead, the dust ring runs out.",
    "b03": "SWAT: the arm of the clip comes round toward the helicopter hung in its sweep.",
    "b04": "The blow: the helicopter is struck and thrown.",
    "b05": "DEAD (a script's killing blow, the combat feature in the set): the body lies where it fell; the brain, the furnace light and the drone have stopped. The ribs still glow: they are a fullbright texture no script can put out.",
    "13": "THE FIGHT (weapons fired). Made the garrison's enemy and left to himself, he takes the first rounds of two tank platoons.",
    "14": "Dying: he staggers and the furnace flares to white.",
    "14b": "He falls.",
    "14c": "The body, 25 seconds after the fall. It stays where it fell.",
    "14d": "The body from above.",
    "T01": "ONE TANK (weapons fired): an M1A1 300 m east of him fires by its own choice.",
    "T02": "After six rounds: he has shuddered and turned to it; the hit signal named the tank and its bearing.",
    "12": "KILL. Spent among the fires, he takes the reserve platoon's volleys (staged: no weapon fired; each a tank round through the combat feature). From the north.",
    "12b": "Dying: he staggers and the furnace flares to white.",
    "12c": "He falls: the dust of the fall rolls out through the fires and hides him.",
    "12d": "The body lies in the dust and the smoke of the fires (from the north). It stays where it fell.",
    "12e": "The same from the road in, 300 m west: only the fires show at dusk.",
    "12f": "From above, a quarter of a minute after the fall: the dust still hangs over the body.",
    "09": "SEA. He has turned south and walks down toward the bay.",
    "09b": "Down to the shore.",
    "10": "Wading out (the engine's wading trait; on the stock binary he stops at the water's edge).",
    "10b": "In the bay, from the water.",
    "11": "DORMANT in the bay, from far out: the glow on the water.",
}


def caption_for(name):
    m = re.match(r"([A-Za-z]?\d+[a-z]?)_", name)
    return CAPTIONS.get(m.group(1), "") if m else ""


def font(size):
    for f in ("segoeui.ttf", "arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def wrap(draw, text, fnt, width):
    lines, cur = [], ""
    for w in text.split():
        t = (cur + " " + w).strip()
        if draw.textlength(t, font=fnt) <= width:
            cur = t
        else:
            lines.append(cur)
            cur = w
    if cur:
        lines.append(cur)
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--sheet", default="")
    ap.add_argument("--title", default="")
    ap.add_argument("--only", default="")
    ap.add_argument("--cols", type=int, default=4)
    ap.add_argument("--cell", type=int, default=640)
    ap.add_argument("--cap", action="append", default=[], help="KEY=caption for this run (a frame whose content differs by run)")
    a = ap.parse_args()

    for c in a.cap:
        k, _, v = c.partition("=")
        CAPTIONS[k.strip()] = v.strip()
    frames = []
    for root, _, files in os.walk(a.run):
        for f in files:
            if f.lower().endswith(".png") and "warmup" not in f and "discard" not in f:
                frames.append(os.path.join(root, f))
    frames.sort(key=lambda p: os.path.basename(p))
    if not frames:
        sys.exit("no frames under " + a.run)
    os.makedirs(a.out, exist_ok=True)
    rows = []
    for p in frames:
        name = os.path.splitext(os.path.basename(p))[0]
        name = re.sub(r"^\d{3}_", "", name)  # tri numbers its frames; the beat prefix is ours
        dst = os.path.join(a.out, "%s_%s.png" % (a.tag, name))
        shutil.copyfile(p, dst)
        rows.append((name, dst, caption_for(name)))

    cap_path = os.path.join(a.out, "CAPTIONS.txt")
    old = open(cap_path, encoding="utf-8").read() if os.path.exists(cap_path) else ""
    block = "[%s] %s\n" % (a.tag, a.title) + "".join("%s_%s.png  %s\n" % (a.tag, n, c) for n, _, c in rows) + "\n"
    old = re.sub(r"\[%s\].*?\n\n" % re.escape(a.tag), "", old, flags=re.S)
    with open(cap_path, "w", encoding="utf-8") as f:
        f.write(old + block)

    if a.sheet:
        only = [s.strip() for s in a.only.split(",") if s.strip()]
        pick = [r for r in rows if not only or any(r[0].startswith(o + "_") for o in only)]
        cols = a.cols
        cw = a.cell
        ch = int(cw * 9 / 16)
        fnt, tf = font(15), font(26)
        tmp = ImageDraw.Draw(Image.new("RGB", (10, 10)))
        caps = [wrap(tmp, c, fnt, cw - 16) for _, _, c in pick]
        cap_h = 19 * max(len(c) for c in caps) + 12
        n_rows = (len(pick) + cols - 1) // cols
        top = 54 if a.title else 8
        sheet = Image.new("RGB", (cols * cw + (cols + 1) * 8, top + n_rows * (ch + cap_h + 8) + 8), (18, 18, 18))
        d = ImageDraw.Draw(sheet)
        if a.title:
            d.text((12, 12), a.title, fill=(235, 235, 235), font=tf)
        for i, (name, path, _) in enumerate(pick):
            x = 8 + (i % cols) * (cw + 8)
            y = top + (i // cols) * (ch + cap_h + 8)
            im = Image.open(path).convert("RGB").resize((cw, ch), Image.LANCZOS)
            sheet.paste(im, (x, y))
            for j, line in enumerate(caps[i]):
                d.text((x + 8, y + ch + 6 + 19 * j), line, fill=(215, 215, 215), font=fnt)
        sheet.save(os.path.join(a.out, a.sheet))
        print("sheet: %s (%d frames)" % (os.path.join(a.out, a.sheet), len(pick)))
    print("%d frames to %s" % (len(rows), a.out))


if __name__ == "__main__":
    main()
