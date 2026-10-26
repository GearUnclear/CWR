from PIL import Image, ImageDraw, ImageFont, ImageFilter
import numpy as np
CAND = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/5/cand"
OUT = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/5"
FONTS = r"D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts"
GREEN = (63,148,61)
WHITE = (242,240,234)
def font(size, kind="title"):
    return ImageFont.truetype(f"{FONTS}/cwr_{kind}.ttf", size)
def frame(clip, side, f):
    return Image.open(f"{CAND}/{clip}/{side}_f{f:04d}.png").convert("RGB")
def crop(img, x0, y0, w, h, outw, outh):
    # crop a source region, scale to output (no grading)
    return img.crop((x0, y0, x0+w, y0+h)).resize((outw, outh), Image.LANCZOS)
def text_size(txt, fnt, track=0):
    w = 0
    for i,ch in enumerate(txt):
        w += fnt.getlength(ch) + (track if i < len(txt)-1 else 0)
    asc, desc = fnt.getmetrics()
    return w, asc
def draw_text(canvas, xy, txt, fnt, fill=WHITE, track=0, shadow=0.55, blur=None, anchor="ls"):
    """Draw tracked text with a soft blurred shadow (no box). xy is the baseline-left
    (anchor ls) or baseline-center (anchor ms) or baseline-right (rs)."""
    w, asc = text_size(txt, fnt, track)
    x, y = xy
    if anchor == "ms": x = x - w/2
    elif anchor == "rs": x = x - w
    W, H = canvas.size
    if shadow:
        layer = Image.new("L", (W, H), 0)
        d = ImageDraw.Draw(layer)
        cx = x
        for ch in txt:
            d.text((cx, y), ch, font=fnt, fill=255, anchor="ls")
            cx += fnt.getlength(ch) + track
        b = blur if blur is not None else max(2, fnt.size//10)
        layer = layer.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.GaussianBlur(b))
        a = (np.asarray(layer).astype(np.float32)/255.0*shadow)
        arr = np.asarray(canvas).astype(np.float32)
        arr = arr*(1-a[...,None])
        canvas.paste(Image.fromarray(arr.clip(0,255).astype(np.uint8)))
    d = ImageDraw.Draw(canvas)
    cx = x
    for ch in txt:
        d.text((cx, y), ch, font=fnt, fill=fill, anchor="ls")
        cx += fnt.getlength(ch) + track
    return w
def save_both(img, name):
    p = f"{OUT}/{name}.png"; j = f"{OUT}/{name}.jpg"
    img.save(p, optimize=True)
    q = 92
    while True:
        img.save(j, quality=q, subsampling=0 if q>=85 else 2, optimize=True)
        import os
        if os.path.getsize(j) < 1_900_000 or q <= 60: break
        q -= 4
    return p, j
def small_views(img, name):
    W,H = img.size
    S = r"C:/Users/Dane/AppData/Local/Temp/claude/D--Arma-CWA/5d8095f8-8e74-4ade-8eb5-abff8cad78dc/scratchpad/t5"
    if W > H:
        a = img.resize((360,202), Image.LANCZOS); b = img.resize((168,94), Image.LANCZOS)
    else:
        a = img.resize((180,320), Image.LANCZOS); b = img.resize((108,192), Image.LANCZOS)
    sh = Image.new("RGB", (a.width+b.width+30, max(a.height,b.height)+20), (40,40,40))
    sh.paste(a,(10,10)); sh.paste(b,(a.width+20,10))
    sh.save(f"{S}/small_{name}.png")
