import os, numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = os.path.normpath(os.path.join(HERE, '..'))
ASSETS = os.path.join(BASE, 'assets')
FRAMES = os.path.join(HERE, 'frames')
FONTS = 'D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts'

RED_BORDER = (175, 78, 46)
RED_TITLE = (171, 68, 51)
CREAM = (234, 214, 179)
HUD_GREEN = (63, 148, 61)
W, H = 1280, 720

def font(size, name='cwr_title.ttf'):
    return ImageFont.truetype(os.path.join(FONTS, name), size)

def poster_native():
    return Image.open(os.path.join(ASSETS, 'poster_native.png')).convert('RGB')

def nn(img, k):
    return img.resize((img.width * k, img.height * k), Image.NEAREST)

def _fill_small_holes(mask, maxarea=8):
    h, w = mask.shape
    seen = np.zeros_like(mask)
    out = mask.copy()
    for y in range(h):
        for x in range(w):
            if mask[y, x] or seen[y, x]:
                continue
            stack = [(y, x)]; seen[y, x] = True; comp = []; edge = False
            while stack:
                cy, cx = stack.pop(); comp.append((cy, cx))
                if cy in (0, h - 1) or cx in (0, w - 1):
                    edge = True
                for ny, nx in ((cy+1, cx), (cy-1, cx), (cy, cx+1), (cy, cx-1)):
                    if 0 <= ny < h and 0 <= nx < w and not mask[ny, nx] and not seen[ny, nx]:
                        seen[ny, nx] = True; stack.append((ny, nx))
            if not edge and len(comp) <= maxarea:
                for cy, cx in comp:
                    out[cy, cx] = True
    return out

def title_mask():
    """Boolean mask of the poster title 'UsluDur!' on the native grid (221 x 123)."""
    a = np.asarray(Image.open(os.path.join(ASSETS, 'title_native.png')))
    return _fill_small_holes(a[:, :, 3] > 0)

def title_rgba(k, color=RED_TITLE, outline=None, outline_px=1):
    """Title at integer scale k; optional outline (in native pixels) in another colour."""
    m = title_mask()
    h, w = m.shape
    pad = outline_px if outline else 0
    big = np.zeros((h + 2*pad, w + 2*pad), bool)
    big[pad:pad+h, pad:pad+w] = m
    rgba = np.zeros(big.shape + (4,), np.uint8)
    if outline:
        o = big.copy()
        for dy in range(-outline_px, outline_px + 1):
            for dx in range(-outline_px, outline_px + 1):
                o |= np.roll(np.roll(big, dy, 0), dx, 1)
        rgba[o] = tuple(outline) + (255,)
    rgba[big] = tuple(color) + (255,)
    return nn(Image.fromarray(rgba, 'RGBA'), k)

def frame(name):
    return Image.open(os.path.join(FRAMES, name)).convert('RGB')

def fit_crop(img, box, size):
    """Crop box (x0,y0,x1,y1) from a full-res frame and scale to size (no colour change)."""
    return img.crop(box).resize(size, Image.LANCZOS)

def text(draw, xy, s, f, fill, spacing=0, anchor='ls'):
    """Draw text with extra letter spacing; xy is the left baseline (or right if anchor='rs')."""
    widths = [f.getlength(ch) for ch in s]
    total = sum(widths) + spacing * (len(s) - 1)
    x, y = xy
    if anchor == 'rs':
        x -= total
    for ch, wd in zip(s, widths):
        draw.text((x, y), ch, font=f, fill=fill, anchor='ls')
        x += wd + spacing
    return total

def save(img, stem):
    p = os.path.join(BASE, stem + '.png')
    img.save(p, optimize=True)
    j = os.path.join(BASE, stem + '.jpg')
    for q in (95, 92, 90, 88, 85):
        img.save(j, quality=q, subsampling=0 if q >= 90 else 2, optimize=True)
        if os.path.getsize(j) < 2_000_000:
            break
    return p, j
