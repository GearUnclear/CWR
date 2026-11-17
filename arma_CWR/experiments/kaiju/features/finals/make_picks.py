r"""Makes the owner's pictures from the collected finals frames:

  pick_a.png, pick_d.png   the best six in-game frames of each model, one-line captions
  side_by_side.png         A and D from the same cameras, one row per camera

  python make_picks.py [--notes D:\Arma_CWA_worktrees\kaiju-notes\features\finals]

Frames are looked up by label in shots\<tag>\ for the tags listed in SOURCES (later tags win, so a
re-shoot replaces a frame). Edit PICKS and PAIRS to change what is shown. Requires Pillow.
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

NOTES = r"D:\Arma_CWA_worktrees\kaiju-notes\features\finals"
if "--notes" in sys.argv:
    NOTES = sys.argv[sys.argv.index("--notes") + 1]

SOURCES = {"a": ["a_photo1"], "d": ["d_photo1"]}
TITLE = {"a": "Candidate A, the Starved Bear. In game, 1920 x 1080, the player's lens, statue build, five DXT1 sheets (four 4096, one 2048).",
         "d": "Candidate D, the dog and the lizard. In game, 1920 x 1080, the player's lens, statue build, five DXT1 sheets (four 4096, one 2048)."}
PICKS = {"a": [], "d": []}     # filled in below once the frames have been looked at
PAIRS = []

try:
    from picks_table import PICKS, PAIRS, SOURCES  # noqa: F401,F811  (picks_table.py beside this file)
except ImportError:
    pass


def font(size, bold=False):
    try:
        return ImageFont.truetype(r"C:\Windows\Fonts\%s" % ("segoeuib.ttf" if bold else "segoeui.ttf"), size)
    except OSError:
        return ImageFont.load_default()


def find(letter, label):
    if "/" in label:
        p = os.path.join(NOTES, "shots", label.replace("/", os.sep) + ".png")
        if os.path.isfile(p):
            return p
        raise SystemExit("no frame %s" % p)
    for tag in reversed(SOURCES[letter]):
        p = os.path.join(NOTES, "shots", tag, label + ".png")
        if os.path.isfile(p):
            return p
    raise SystemExit("no frame %s for %s in %s" % (label, letter, SOURCES[letter]))


def cell(path, w, h):
    im = Image.open(path).convert("RGB")
    if im.size != (w, h):
        im = im.resize((w, h), Image.LANCZOS)
    return im


def pick_sheet(letter):
    w, h, cap, top = 1280, 720, 46, 64
    items = PICKS[letter]
    cols, rows = 2, (len(items) + 1) // 2
    sheet = Image.new("RGB", (cols * w, top + rows * (h + cap)), (18, 18, 20))
    d = ImageDraw.Draw(sheet)
    d.text((16, 14), TITLE[letter], fill=(240, 240, 240), font=font(30, True))
    for i, (label, caption) in enumerate(items):
        x, y = (i % cols) * w, top + (i // cols) * (h + cap)
        sheet.paste(cell(find(letter, label), w, h), (x, y))
        d.text((x + 14, y + h + 6), "%d. %s" % (i + 1, caption), fill=(235, 235, 235), font=font(24))
    out = os.path.join(NOTES, "pick_%s.png" % letter)
    sheet.save(out)
    print("wrote", out, sheet.size)


def side_by_side():
    w, h, cap, top = 960, 540, 36, 64
    sheet = Image.new("RGB", (2 * w, top + len(PAIRS) * (h + cap)), (18, 18, 20))
    d = ImageDraw.Draw(sheet)
    d.text((16, 14), "A, the Starved Bear", fill=(240, 240, 240), font=font(30, True))
    d.text((w + 16, 14), "D, the dog and the lizard", fill=(240, 240, 240), font=font(30, True))
    for i, (label, caption, tag_a, tag_d) in enumerate(PAIRS):
        y = top + i * (h + cap)
        sheet.paste(cell(find("a", tag_a + "/" + label), w, h), (0, y))
        sheet.paste(cell(find("d", tag_d + "/" + label), w, h), (w, y))
        d.text((14, y + h + 4), "%d. %s" % (i + 1, caption), fill=(235, 235, 235), font=font(21))
    out = os.path.join(NOTES, "side_by_side.png")
    sheet.save(out)
    print("wrote", out, sheet.size)


if __name__ == "__main__":
    for letter in ("a", "d"):
        if PICKS[letter]:
            pick_sheet(letter)
    if PAIRS:
        side_by_side()
