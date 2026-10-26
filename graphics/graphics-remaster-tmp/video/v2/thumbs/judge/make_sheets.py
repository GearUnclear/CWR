"""Owner contact sheets for the thumbnail swarm.

owner_sheet.png        landscape thumbnails at 360x202, laid out like a YouTube home feed (dark theme)
owner_sheet_small.png  the same at 168x94, laid out like the YouTube watch-page sidebar
shorts_sheet.png       the Shorts covers at 180x320, laid out like a Shorts shelf
Each picture carries a small plain number outside its top-left corner.
The "2:13" badge mimics YouTube's duration overlay (the video is 133.4 s) so the
bottom-right corner can be judged as it will really look.
"""
from PIL import Image, ImageDraw, ImageFont

T = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/"
OUT = T + "judge/"
FONT = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/cwr_body.ttf"

LANDSCAPE = [
    "1/split1_dawn_ridge.png",
    "1/split2_night_road.png",
    "1/split3_bay.png",
    "2/2a_poster_beside_outpost.png",
    "2/2b_poster_frame_outpost.png",
    "2/2c_poster_column_split.png",
    "3/v1_outpost_sun.png",
    "3/v2_dawn_ridge.png",
    "3/v3_town_fire.png",
    "4/v2_fighter_outpost.png",
    "4/v1_poster_plate.png",
    "4/v3_poster_split.png",
    "5/thumb_stack_triptych.png",
]
SHORTS = [
    "2/2_shorts_cover.png",
    "4/shorts_cover.png",
    "5/shorts_cover_stack.png",
    "5/shorts_cover_hero.png",
    "5/shorts_cover_stack_night.png",
    "5/shorts_cover_poster.png",
]
TITLE = "Uslu dur! trailer, part 1: graphics"
BG = (15, 15, 15)
FG = (241, 241, 241)
MUTED = (170, 170, 170)


def font(px):
    return ImageFont.truetype(FONT, px)


def rounded(im, r):
    m = Image.new("L", (im.width * 4, im.height * 4), 0)
    ImageDraw.Draw(m).rounded_rectangle([0, 0, m.width - 1, m.height - 1], r * 4, fill=255)
    m = m.resize(im.size, Image.LANCZOS)
    return im, m


def badge(im, text, px, pad, r):
    d = ImageDraw.Draw(im, "RGBA")
    f = font(px)
    bb = d.textbbox((0, 0), text, font=f)
    w, h = bb[2] - bb[0], bb[3] - bb[1]
    x1, y1 = im.width - pad, im.height - pad
    x0, y0 = x1 - w - 2 * (pad - 1), y1 - h - 2 * (pad - 2)
    d.rounded_rectangle([x0, y0, x1, y1], r, fill=(0, 0, 0, 205))
    d.text((x0 + pad - 1 - bb[0], y0 + pad - 2 - bb[1]), text, font=f, fill=(255, 255, 255, 255))


def thumb(path, size, radius, dur=None):
    im = Image.open(T + path).convert("RGB").resize(size, Image.LANCZOS)
    if dur:
        badge(im, *dur)
    return rounded(im, radius)


def feed_sheet():
    W, H = 360, 202
    cols, gx, gy, mx, my = 4, 16, 18, 28, 28
    rows = (len(LANDSCAPE) + cols - 1) // cols
    num_h, meta_h = 18, 46
    cell_h = num_h + H + meta_h
    sheet = Image.new("RGB", (2 * mx + cols * W + (cols - 1) * gx, 2 * my + rows * cell_h + (rows - 1) * gy), BG)
    d = ImageDraw.Draw(sheet)
    for i, p in enumerate(LANDSCAPE):
        c, r = i % cols, i // cols
        x, y = mx + c * (W + gx), my + r * (cell_h + gy)
        d.text((x, y), str(i + 1), font=font(13), fill=MUTED)
        im, m = thumb(p, (W, H), 8, ("2:13", 12, 5, 4))
        sheet.paste(im, (x, y + num_h), m)
        d.text((x, y + num_h + H + 10), TITLE, font=font(15), fill=FG)
        d.text((x, y + num_h + H + 30), "Uslu dur!", font=font(12), fill=MUTED)
    sheet.save(OUT + "owner_sheet.png", optimize=True)
    return sheet


def sidebar_sheet():
    W, H = 168, 94
    cols, mx, my, gx, gy = 3, 24, 24, 28, 12
    num_w, text_w = 22, 150
    cell_w = num_w + W + 8 + text_w
    rows = (len(LANDSCAPE) + cols - 1) // cols
    sheet = Image.new("RGB", (2 * mx + cols * cell_w + (cols - 1) * gx, 2 * my + rows * H + (rows - 1) * gy), BG)
    d = ImageDraw.Draw(sheet)
    for i, p in enumerate(LANDSCAPE):
        c, r = i % rows, i // rows
        x, y = mx + (i // rows) * (cell_w + gx), my + (i % rows) * (H + gy)
        d.text((x, y), str(i + 1), font=font(12), fill=MUTED)
        im, m = thumb(p, (W, H), 6, ("2:13", 10, 4, 3))
        sheet.paste(im, (x + num_w, y), m)
        tx = x + num_w + W + 8
        d.text((tx, y + 1), "Uslu dur! trailer, part 1:", font=font(13), fill=FG)
        d.text((tx, y + 19), "graphics", font=font(13), fill=FG)
        d.text((tx, y + 40), "Uslu dur!", font=font(11), fill=MUTED)
    sheet.save(OUT + "owner_sheet_small.png", optimize=True)
    return sheet


def shorts_sheet():
    W, H = 180, 320
    cols, mx, my, gx = len(SHORTS), 28, 28, 14
    num_h, meta_h = 18, 40
    sheet = Image.new("RGB", (2 * mx + cols * W + (cols - 1) * gx, 2 * my + num_h + H + meta_h), BG)
    d = ImageDraw.Draw(sheet)
    for i, p in enumerate(SHORTS):
        x, y = mx + i * (W + gx), my
        d.text((x, y), "S" + str(i + 1), font=font(13), fill=MUTED)
        im, m = thumb(p, (W, H), 8)
        sheet.paste(im, (x, y + num_h), m)
        d.text((x, y + num_h + H + 8), "Uslu dur! trailer,", font=font(13), fill=FG)
        d.text((x, y + num_h + H + 25), "part 1: graphics", font=font(13), fill=FG)
    sheet.save(OUT + "shorts_sheet.png", optimize=True)
    return sheet


if __name__ == "__main__":
    feed_sheet()
    sidebar_sheet()
    shorts_sheet()
    print("ok")
