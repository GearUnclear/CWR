"""Designer 2 (THE POSTER): three 1280x720 thumbnails built around the owner's pixel-art poster.
Footage is only cropped and scaled (Lanczos); the poster only rebuilt on its native grid and
scaled by whole numbers with nearest neighbour."""
import os
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
from lib import *

OFFWHITE = (235, 238, 245)   # the trailer's label white

def border(c, t=24, b=24, l=24, r=24, col=RED_BORDER):
    d = ImageDraw.Draw(c)
    d.rectangle((0, 0, W - 1, t - 1), fill=col); d.rectangle((0, H - b, W - 1, H - 1), fill=col)
    d.rectangle((0, 0, l - 1, H - 1), fill=col); d.rectangle((W - r, 0, W - 1, H - 1), fill=col)

def label(c, xy, s, size, fill, spacing, anchor='ls', shadow=True):
    """Small upper-case label like the trailer's: glyphs only, a faint glyph shadow, no box."""
    if shadow:
        sh = Image.new('L', c.size, 0)
        text(ImageDraw.Draw(sh), (xy[0], xy[1] + 1), s, font(size), 255, spacing, anchor)
        sh = sh.filter(ImageFilter.GaussianBlur(1.6)).point(lambda v: v * 0.35)
        c.paste((0, 0, 0), (0, 0), sh)
    text(ImageDraw.Draw(c), xy, s, font(size), fill, spacing, anchor)

# ---------------------------------------------------------------- A: poster beside the real thing
def variant_a():
    c = Image.new('RGB', (W, H), RED_BORDER)
    # poster on its native grid x2 (= its own pixel size): native cols 1..253, rows 13..372
    p = nn(poster_native().crop((1, 13, 254, 373)), 2)            # 506 x 720
    c.paste(p, (0, 0))
    fw = W - p.width                                              # 774
    x0, y0, ch = 680, 40, 900                                     # a little tighter than full height
    c.paste(fit_crop(frame('full_outpost_420_ud.png'), (x0, y0, x0 + round(ch * fw / H), y0 + ch), (fw, H)), (p.width, 0))
    label(c, (p.width + 34, H - 38), 'PART 1: GRAPHICS', 36, CREAM, 3)
    return c

# ---------------------------------------------------------------- B: the poster's border and title on a real frame
def variant_b():
    c = fit_crop(frame('full_outpost_510_ud.png'), (0, 0, 1920, 1080), (W, H))
    bb = 72
    border(c, b=bb)
    t = title_rgba(2, RED_TITLE, outline=CREAM)                   # 1 native px cream keyline
    c.paste(t, (24 + 38, 24 + 30), t)
    f = font(40); s = 'PART 1: GRAPHICS'; sp = 5
    wtot = sum(f.getlength(ch) for ch in s) + sp * (len(s) - 1)
    text(ImageDraw.Draw(c), ((W - wtot) / 2, H - bb / 2 + 15), s, f, CREAM, spacing=sp)
    return c

# ---------------------------------------------------------------- C: poster column beside a vanilla | Uslu dur! split
def variant_c():
    bw, col_w = 24, 270
    fw, fh = W - 2 * bw - col_w, H - 2 * bw                      # 962 x 672
    cw = round(1080 * fw / fh); x0 = 187
    box = (x0, 0, x0 + cw, 1080)
    v = np.asarray(fit_crop(frame('full_valley_mist_450_vanilla.png'), box, (fw, fh))).astype(np.float32)
    u = np.asarray(fit_crop(frame('full_valley_mist_450_ud.png'), box, (fw, fh))).astype(np.float32)
    # slanted divider like the trailer's (top further right than bottom)
    xt, xb = 400.0, 271.0
    ys = np.arange(fh)[:, None]; xs = np.arange(fw)[None, :]
    xl = xt + (xb - xt) * ys / (fh - 1)
    a = np.clip(xs - xl + 0.5, 0, 1)[..., None]                  # 1 = Uslu dur! side
    foot = v * (1 - a) + u * a
    # thin line, about 2 px, antialiased
    cos = 1 / np.sqrt(1 + ((xb - xt) / fh) ** 2)
    dist = np.abs(xs - xl) * cos
    la = np.clip(1.5 - dist, 0, 1)[..., None] * 0.9
    foot = foot * (1 - la) + np.array(OFFWHITE, np.float32) * la
    foot = Image.fromarray(np.clip(foot + 0.5, 0, 255).astype(np.uint8))
    label(foot, (int(xt - 129 * 62 / 672) - 20, 62), 'VANILLA', 30, OFFWHITE, 6, anchor='rs')
    c = Image.new('RGB', (W, H), RED_BORDER)
    c.paste(foot, (bw, bw))
    cx = bw + fw
    d = ImageDraw.Draw(c)
    d.rectangle((cx, bw, cx + col_w - 1, H - bw - 1), fill=CREAM)
    t = title_rgba(2)
    w1 = t.crop((0, 0, 107 * 2, t.height)); w2 = t.crop((110 * 2, 0, t.width, t.height))
    top = bw + 40
    c.paste(w1, (cx + (col_w - w1.width) // 2, top), w1)
    c.paste(w2, (cx + (col_w - w2.width) // 2, top + t.height + 18), w2)
    return c

# ---------------------------------------------------------------- Shorts cover 1080x1920: poster over the real frame
def shorts_cover():
    c = Image.new('RGB', (1080, 1920), RED_BORDER)
    p = nn(poster_native().crop((1, 13, 254, 373)), 3)            # 759 x 1080
    c.paste(p, ((1080 - p.width) // 2, 0))
    strip = 84                                                    # red caption strip under the poster
    fh = 1920 - 1080 - strip                                      # 756
    cw = round(1080 * 1080 / fh); x0 = 1920 - cw
    c.paste(fit_crop(frame('full_outpost_420_ud.png'), (x0, 0, x0 + cw, 1080), (1080, fh)), (0, 1080 + strip))
    f = font(50); s = 'PART 1: GRAPHICS'; sp = 6
    wtot = sum(f.getlength(ch) for ch in s) + sp * (len(s) - 1)
    text(ImageDraw.Draw(c), ((1080 - wtot) / 2, 1080 + strip / 2 + 18), s, f, CREAM, spacing=sp)
    return c

if __name__ == '__main__':
    import sys
    outs = {}
    for stem, fn in (('2a_poster_beside_outpost', variant_a),
                     ('2b_poster_frame_outpost', variant_b),
                     ('2c_poster_column_split', variant_c)):
        img = fn()
        assert img.size == (W, H)
        outs[stem] = save(img, stem)
        print(stem, outs[stem], os.path.getsize(outs[stem][1]))
    sc = shorts_cover(); assert sc.size == (1080, 1920)
    sc.save(os.path.join(BASE, '2_shorts_cover.png'), optimize=True)
    print('shorts', sc.size)
