import os
from lib import *
from PIL import Image, ImageDraw
OUT = os.path.join(BASE, 'drafts'); os.makedirs(OUT, exist_ok=True)

def poster_panel():
    return nn(poster_native().crop((1, 13, 254, 373)), 2)   # 506 x 720

def A(fr, x0):
    c = Image.new('RGB', (W, H), RED_BORDER)
    p = poster_panel(); c.paste(p, (0, 0))
    fw = W - p.width
    c.paste(fit_crop(frame(fr), (x0, 0, x0 + round(1080 * fw / H), 1080), (fw, H)), (p.width, 0))
    d = ImageDraw.Draw(c)
    text(d, (p.width + 34, H - 38), 'PART 1: GRAPHICS', font(36), CREAM, spacing=3)
    return c

def border(c, t=24, b=24, l=24, r=24, col=RED_BORDER):
    d = ImageDraw.Draw(c)
    d.rectangle((0, 0, W - 1, t - 1), fill=col); d.rectangle((0, H - b, W - 1, H - 1), fill=col)
    d.rectangle((0, 0, l - 1, H - 1), fill=col); d.rectangle((W - r, 0, W - 1, H - 1), fill=col)

def B(fr='full_outpost_450_ud.png', box=(100, 0, 1920, 1024), bb=70):
    c = fit_crop(frame(fr), box, (W, H))
    border(c, b=bb)
    t = title_rgba(2); c.paste(t, (24 + 38, 24 + 30), t)
    d = ImageDraw.Draw(c)
    f = font(40); s = 'PART 1: GRAPHICS'
    wtot = sum(f.getlength(ch) for ch in s) + 5 * (len(s) - 1)
    text(d, ((W - wtot) / 2, H - bb / 2 + 15), s, f, CREAM, spacing=5)
    return c

def C(base='valley_mist_150', box=(0, 0, 1920, 1080), split_frac=0.5, col_w=300, side='right'):
    bw = 24
    fw = W - 2 * bw - col_w; fh = H - 2 * bw
    # crop of the same aspect as the footage window
    x0, y0, x1, y1 = box
    v = fit_crop(frame('full_%s_vanilla.png' % base), box, (fw, fh))
    u = fit_crop(frame('full_%s_ud.png' % base), box, (fw, fh))
    s = int(fw * split_frac)
    foot = v.copy(); foot.paste(u.crop((s, 0, fw, fh)), (s, 0))
    d0 = ImageDraw.Draw(foot); d0.rectangle((s - 1, 0, s, fh), fill=CREAM)
    text(d0, (s - 16, 52), 'VANILLA', font(30), CREAM, spacing=3, anchor='rs')
    c = Image.new('RGB', (W, H), RED_BORDER)
    fx = bw if side == 'right' else bw + col_w
    c.paste(foot, (fx, bw))
    cx = W - bw - col_w if side == 'right' else bw
    d = ImageDraw.Draw(c)
    d.rectangle((cx, bw, cx + col_w - 1, H - bw - 1), fill=CREAM)
    t = title_rgba(2)
    # words: native cols 0..106 'Uslu', 110..220 'Dur!'
    u_w = t.crop((0, 0, 107 * 2, t.height)); d_w = t.crop((110 * 2, 0, t.width, t.height))
    gx = cx + (col_w - max(u_w.width, d_w.width)) // 2
    c.paste(u_w, (cx + (col_w - u_w.width) // 2, bw + 34), u_w)
    c.paste(d_w, (cx + (col_w - d_w.width) // 2, bw + 34 + t.height + 16), d_w)
    return c

A('full_outpost_375_ud.png', 430).save(os.path.join(OUT, 'A1.png'))
A('full_outpost_450_ud.png', 600).save(os.path.join(OUT, 'A2.png'))
A('full_valley_mist_150_ud.png', 759).save(os.path.join(OUT, 'A3.png'))
B().save(os.path.join(OUT, 'B1.png'))
C().save(os.path.join(OUT, 'C1.png'))
C(base='outpost_375', box=(150, 0, 1920, 1080), split_frac=0.42).save(os.path.join(OUT, 'C2.png'))
print('ok')
