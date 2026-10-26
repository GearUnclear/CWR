"""Thumbnail composer for designer 3 (hero shot direction).

The game frame is only cropped and scaled (LANCZOS). Type is drawn on top.
An optional soft shadow under the glyphs (and an optional corner gradient) only
touches pixels right behind the text, in areas that are not the demonstrated
effect; both are set per variant and logged in REPORT.txt.
"""
import sys, os
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

FONTS = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts"
CAND = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/3/work/cand"
OUTDIR = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/3"

HUD_GREEN = (63, 148, 61)
WHITE = (238, 238, 234)
LABEL = (226, 226, 220)
SS = 2  # supersampling factor for type
W, H = 1280, 720


def font_title(px):
    return ImageFont.truetype(f"{FONTS}/cwr_title.ttf", px)


def font_body(px, bold=True):
    f = ImageFont.truetype(f"{FONTS}/cwr_body.ttf", px)
    if bold:
        f.set_variation_by_name("Bold")
    return f


def tracked_width(font, text, track):
    w = 0.0
    for i, ch in enumerate(text):
        w += font.getlength(ch)
        if i < len(text) - 1:
            w += track
    return w


def draw_tracked(draw, xy, text, font, track, fill, stroke=0):
    x, y = xy
    for ch in text:
        draw.text((x, y), ch, font=font, fill=fill, stroke_width=stroke, stroke_fill=fill)
        x += font.getlength(ch) + track


def ink_box(font, text):
    # vertical ink extents of caps relative to the draw origin
    b = font.getbbox(text)
    return b[1], b[3]


def make_type_layer(spec):
    """Returns (rgba layer at SS scale, glyph mask at SS scale)."""
    Wt, Ht = W * SS, H * SS
    layer = Image.new("RGBA", (Wt, Ht), (0, 0, 0, 0))
    mask = Image.new("L", (Wt, Ht), 0)
    d = ImageDraw.Draw(layer)
    dm = ImageDraw.Draw(mask)

    bsize = int(spec["brand_px"] * SS)
    bfont = font_title(bsize)
    btrack = spec.get("brand_track", 0.10) * bsize
    bstroke = int(round(spec.get("brand_bold", 1.0) * SS))
    brand = spec.get("brand", "USLU DUR!")
    bw = tracked_width(bfont, brand, btrack)
    btop, bbot = ink_box(bfont, brand)

    lsize = int(spec["label_px"] * SS)
    lfont = font_body(lsize)
    ltrack = spec.get("label_track", 0.16) * lsize
    label = spec.get("label", "PART 1: GRAPHICS")
    lw = tracked_width(lfont, label, ltrack)
    ltop, lbot = ink_box(lfont, label)

    align = spec.get("align", "left")
    ax = spec["x"] * SS  # anchor x (left edge, center or right edge)
    y = spec["y"] * SS  # top of brand ink

    def xpos(width):
        if align == "left":
            return ax
        if align == "center":
            return ax - width / 2
        return ax - width

    bx = xpos(bw)
    by = y - btop
    rule_gap = spec.get("rule_gap", 22) * SS
    rule_len = spec.get("rule_len", 96) * SS
    rule_th = spec.get("rule_th", 5) * SS
    label_gap = spec.get("label_gap", 20) * SS
    ry = y + (bbot - btop) + rule_gap
    rx = xpos(rule_len)
    # left aligned rule sits under the first letter's ink
    if align == "left":
        rx = bx + bfont.getbbox(brand[0])[0]
    ly = ry + rule_th + label_gap
    lx = xpos(lw)
    if align == "left":
        lx = bx + bfont.getbbox(brand[0])[0] - lfont.getbbox(label[0])[0]

    for dd, fill_b, fill_r, fill_l in ((d, WHITE + (255,), HUD_GREEN + (255,), LABEL + (255,)),
                                       (dm, 255, 255, 255)):
        draw_tracked(dd, (bx, by), brand, bfont, btrack, fill_b, bstroke)
        if spec.get("rule", True):
            dd.rectangle([rx, ry, rx + rule_len, ry + rule_th], fill=fill_r)
        if label:
            draw_tracked(dd, (lx, ly - ltop), label, lfont, ltrack, fill_l, 0)
    return layer, mask


def compose(spec):
    src = Image.open(f"{CAND}/{spec['frame']}").convert("RGB")
    x0, y0, cw = spec["crop"]
    ch = cw * 9 / 16
    img = src.crop((x0, y0, x0 + cw, int(round(y0 + ch)))).resize((W, H), Image.LANCZOS)
    base = img.convert("RGBA")

    layer, mask = make_type_layer(spec)

    # optional soft corner gradient behind text (outside the demonstrated effect)
    g = spec.get("grad")
    if g:
        gx, gy, rad, strength = g  # centre (1x px), radius, max darkening 0..1
        yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
        dist = np.sqrt(((xx - gx) / rad[0]) ** 2 + ((yy - gy) / rad[1]) ** 2)
        a = np.clip(1.0 - dist, 0, 1) ** 1.6 * strength
        dark = Image.fromarray((a * 255).astype(np.uint8), "L")
        shade = Image.new("RGBA", (W, H), (0, 0, 0, 255))
        shade.putalpha(dark)
        base = Image.alpha_composite(base, shade)

    sh = spec.get("shadow")
    if sh:
        blur, opacity, dy = sh
        m = mask.filter(ImageFilter.GaussianBlur(blur * SS))
        m = m.resize((W, H), Image.LANCZOS)
        m = m.point(lambda v: int(v * opacity))
        shade = Image.new("RGBA", (W, H), (0, 0, 0, 255))
        shade.putalpha(m)
        shifted = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        shifted.paste(shade, (0, dy), shade)
        base = Image.alpha_composite(base, shifted)

    type1x = layer.resize((W, H), Image.LANCZOS)
    out = Image.alpha_composite(base, type1x).convert("RGB")
    return out


def save_all(name, im):
    png = f"{OUTDIR}/{name}.png"
    jpg = f"{OUTDIR}/{name}.jpg"
    im.save(png, optimize=True)
    q = 97
    while True:
        im.save(jpg, quality=q, subsampling=0 if q >= 90 else 2, optimize=True)
        if os.path.getsize(jpg) < 1_900_000 or q <= 70:
            break
        q -= 3
    # small previews
    im.resize((360, 202), Image.LANCZOS).save(f"{OUTDIR}/work/{name}_360.png")
    im.resize((168, 94), Image.LANCZOS).save(f"{OUTDIR}/work/{name}_168.png")
    return png, jpg, q
