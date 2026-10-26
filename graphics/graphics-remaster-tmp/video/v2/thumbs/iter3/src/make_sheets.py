"""Sheets for the thumbnail-3 iterations (same look as thumbs/judge/make_sheets.py).

feed_sheet.png     all at 360x202, laid out like the YouTube home feed (dark theme)
sidebar_sheet.png  all at 168x94, laid out like the watch-page sidebar
big_sheet.png      the new ones at 640x360 (plus 3 for reference), to judge details
The label sits outside each picture, above its top-left corner. The "2:13" badge
imitates YouTube's duration overlay (the video is 133.4 s).
"""
from PIL import Image, ImageDraw, ImageFont

D = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/iter3/"
FONT = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/cwr_body.ttf"
ALL = ["3", "3a", "3b", "3c", "3d", "3e", "3f", "3g", "3h", "3i", "3j", "3k"]
TITLE = "Uslu dur! trailer, part 1: graphics"
BG = (15, 15, 15)
FG = (241, 241, 241)
MUTED = (170, 170, 170)


def font(px):
    return ImageFont.truetype(FONT, px)


def mask(size, r):
    m = Image.new("L", (size[0] * 4, size[1] * 4), 0)
    ImageDraw.Draw(m).rounded_rectangle([0, 0, m.width - 1, m.height - 1], r * 4, fill=255)
    return m.resize(size, Image.LANCZOS)


def badge(im, text, px, pad, r):
    d = ImageDraw.Draw(im, "RGBA")
    f = font(px)
    bb = d.textbbox((0, 0), text, font=f)
    w, h = bb[2] - bb[0], bb[3] - bb[1]
    x1, y1 = im.width - pad, im.height - pad
    x0, y0 = x1 - w - 2 * (pad - 1), y1 - h - 2 * (pad - 2)
    d.rounded_rectangle([x0, y0, x1, y1], r, fill=(0, 0, 0, 205))
    d.text((x0 + pad - 1 - bb[0], y0 + pad - 2 - bb[1]), text, font=f, fill=(255, 255, 255, 255))


def thumb(name, size, radius, dur=None):
    im = Image.open(D + name + ".png").convert("RGB").resize(size, Image.LANCZOS)
    if dur:
        badge(im, *dur)
    return im, mask(size, radius)


def feed_sheet():
    W, H = 360, 202
    cols, gx, gy, mx, my = 4, 16, 18, 28, 28
    rows = (len(ALL) + cols - 1) // cols
    num_h, meta_h = 20, 46
    cell_h = num_h + H + meta_h
    sheet = Image.new("RGB", (2 * mx + cols * W + (cols - 1) * gx, 2 * my + rows * cell_h + (rows - 1) * gy), BG)
    d = ImageDraw.Draw(sheet)
    for i, n in enumerate(ALL):
        c, r = i % cols, i // cols
        x, y = mx + c * (W + gx), my + r * (cell_h + gy)
        d.text((x, y), n + ("  (picked original)" if n == "3" else ""), font=font(14), fill=MUTED)
        im, m = thumb(n, (W, H), 8, ("2:13", 12, 5, 4))
        sheet.paste(im, (x, y + num_h), m)
        d.text((x, y + num_h + H + 10), TITLE, font=font(15), fill=FG)
        d.text((x, y + num_h + H + 30), "Uslu dur!", font=font(12), fill=MUTED)
    sheet.save(D + "feed_sheet.png", optimize=True)


def sidebar_sheet():
    W, H = 168, 94
    cols, mx, my, gx, gy = 3, 24, 24, 28, 12
    num_w, text_w = 26, 150
    cell_w = num_w + W + 8 + text_w
    rows = (len(ALL) + cols - 1) // cols
    sheet = Image.new("RGB", (2 * mx + cols * cell_w + (cols - 1) * gx, 2 * my + rows * H + (rows - 1) * gy), BG)
    d = ImageDraw.Draw(sheet)
    for i, n in enumerate(ALL):
        x, y = mx + (i // rows) * (cell_w + gx), my + (i % rows) * (H + gy)
        d.text((x, y), n, font=font(12), fill=MUTED)
        im, m = thumb(n, (W, H), 6, ("2:13", 10, 4, 3))
        sheet.paste(im, (x + num_w, y), m)
        tx = x + num_w + W + 8
        d.text((tx, y + 1), "Uslu dur! trailer, part 1:", font=font(13), fill=FG)
        d.text((tx, y + 19), "graphics", font=font(13), fill=FG)
        d.text((tx, y + 40), "Uslu dur!", font=font(11), fill=MUTED)
    sheet.save(D + "sidebar_sheet.png", optimize=True)


def big_sheet():
    W, H = 640, 360
    cols, gx, gy, mx, my, num_h = 3, 20, 16, 24, 24, 24
    rows = (len(ALL) + cols - 1) // cols
    sheet = Image.new("RGB", (2 * mx + cols * W + (cols - 1) * gx, 2 * my + rows * (num_h + H) + (rows - 1) * gy), BG)
    d = ImageDraw.Draw(sheet)
    for i, n in enumerate(ALL):
        c, r = i % cols, i // cols
        x, y = mx + c * (W + gx), my + r * (num_h + H + gy)
        d.text((x, y), n + ("  (picked original, for reference)" if n == "3" else ""), font=font(16), fill=MUTED)
        im = Image.open(D + n + ".png").convert("RGB").resize((W, H), Image.LANCZOS)
        sheet.paste(im, (x, y + num_h))
    sheet.save(D + "big_sheet.png", optimize=True)


if __name__ == "__main__":
    feed_sheet()
    sidebar_sheet()
    big_sheet()
    print("ok")
