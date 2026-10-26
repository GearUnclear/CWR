"""Shared helpers for thumbnail designer 4 (poster meets the game)."""
import os, subprocess
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from scipy import ndimage as ndi

ROOT = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/4'
V2 = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2'
FR = ROOT + '/frames'
AS = ROOT + '/assets'
FONTS = 'D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts'
FFMPEG = 'C:/ffmpeg/bin/ffmpeg.exe'

RED = (175, 78, 46)       # poster border
INK_RED = (172, 70, 50)   # poster lettering
CREAM = (234, 214, 179)   # poster paper
INK = (48, 40, 40)        # poster outline ink
HUD = (63, 148, 61)       # trailer accent


def frame(clip, side, n):
    p = f'{FR}/{clip}_{side}_{n:04d}.png'
    if not os.path.exists(p):
        src = f'{V2}/{side}/{clip}/{clip}__{side}.mkv'
        subprocess.run([FFMPEG, '-v', 'error', '-y', '-i', src, '-vf', 'select=eq(n' + chr(92) + f',{n})',
                        '-vsync', '0', '-frames:v', '1', p], check=True)
    return Image.open(p).convert('RGB')


def crop_box(img, x0, y0, w, h, out_w, out_h):
    """Crop a region (no colour change) and scale it to out size."""
    return img.crop((x0, y0, x0 + w, y0 + h)).resize((out_w, out_h), Image.LANCZOS)


def mask(name):
    return np.array(Image.open(f'{AS}/{name}.png').convert('L')) > 127


def letter_alpha(name, height):
    """Poster lettering mask scaled to height, keeping its pixel steps."""
    m = mask(name)
    h, w = m.shape
    k = max(1, int(np.ceil(height / h)) + 1)
    big = Image.fromarray((m * 255).astype(np.uint8)).resize((w * k, h * k), Image.NEAREST)
    width = round(w * height / h)
    return big.resize((width, height), Image.LANCZOS)


def lettering(name, height, color=INK_RED, outline=0, outline_color=CREAM, pad=None):
    a = letter_alpha(name, height)
    if pad is None:
        pad = outline + 2
    A = np.zeros((a.size[1] + 2 * pad, a.size[0] + 2 * pad), np.float32)
    A[pad:pad + a.size[1], pad:pad + a.size[0]] = np.array(a, np.float32) / 255
    out = np.zeros(A.shape + (4,), np.float32)
    if outline > 0:
        yy, xx = np.mgrid[-outline:outline + 1, -outline:outline + 1]
        disk = (xx * xx + yy * yy) <= outline * outline
        O = ndi.grey_dilation(A, footprint=disk)
        out[..., :3] = np.array(outline_color, np.float32)
        out[..., 3] = O
    fill = np.array(color, np.float32)
    # composite fill over outline
    fa = A[..., None]
    base_a = out[..., 3:4]
    out_a = fa + base_a * (1 - fa)
    rgb = (fill * fa + out[..., :3] * base_a * (1 - fa)) / np.maximum(out_a, 1e-6)
    res = np.concatenate([rgb, out_a], -1)
    return Image.fromarray(np.clip(res * [1, 1, 1, 255], 0, 255).astype(np.uint8), 'RGBA')


def fighter(scale, y_cut=None):
    """Poster fighter cutout, nearest-neighbour scaled (pixel art stays pixel art)."""
    f = Image.open(f'{AS}/fighter_raw.png').convert('RGBA')
    bb = f.getbbox()
    f = f.crop(bb)
    if y_cut is not None:
        f = f.crop((0, 0, f.size[0], min(f.size[1], y_cut)))
    k = int(np.ceil(scale))
    big = f.resize((f.size[0] * k, f.size[1] * k), Image.NEAREST)
    if k != scale:
        big = big.resize((round(f.size[0] * scale), round(f.size[1] * scale)), Image.LANCZOS)
    return big


def font(name, size):
    return ImageFont.truetype(f'{FONTS}/{name}', size)


def text_spaced(draw, xy, s, fnt, fill, tracking=0, anchor_center=False):
    widths = [draw.textlength(ch, font=fnt) for ch in s]
    total = sum(widths) + tracking * (len(s) - 1)
    x, y = xy
    if anchor_center:
        x = x - total / 2
    for ch, w in zip(s, widths):
        draw.text((x, y), ch, font=fnt, fill=fill)
        x += w + tracking
    return total


def text_width(draw, s, fnt, tracking=0):
    return sum(draw.textlength(ch, font=fnt) for ch in s) + tracking * (len(s) - 1)


def save(img, name):
    img = img.convert('RGB')
    png = f'{ROOT}/{name}.png'
    jpg = f'{ROOT}/{name}.jpg'
    img.save(png, optimize=True)
    for q in (95, 92, 90, 88, 85):
        img.save(jpg, quality=q, subsampling=0 if q >= 92 else 2, optimize=True)
        if os.path.getsize(jpg) < 1_900_000:
            break
    return png, jpg


def smalls(img, name):
    """Write the two judging sizes for one image (for my own look)."""
    a = img.convert('RGB').resize((360, 202), Image.LANCZOS)
    b = img.convert('RGB').resize((168, 94), Image.LANCZOS)
    a.save(f'{ROOT}/check/{name}_360.png')
    b.save(f'{ROOT}/check/{name}_168.png')
    # an at-a-glance strip: 360 and 168 side by side, plus 168 shown 2x nearest
    strip = Image.new('RGB', (360 + 10 + 168 + 10 + 336, 202), (40, 40, 40))
    strip.paste(a, (0, 0)); strip.paste(b, (370, 0)); strip.paste(b.resize((336, 188), Image.NEAREST), (548, 0))
    strip.save(f'{ROOT}/check/{name}_strip.png')
