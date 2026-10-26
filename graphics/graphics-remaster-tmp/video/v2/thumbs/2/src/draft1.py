import os
from lib import *
from PIL import Image, ImageDraw
OUT = os.path.join(BASE, 'drafts'); os.makedirs(OUT, exist_ok=True)

# A: poster | outpost (n=375)
def A(x0=430, fr='full_outpost_375_ud.png', label=True):
    c = Image.new('RGB', (W, H), RED_BORDER)
    p = nn(poster_native().crop((1, 13, 254, 373)), 2)   # 506 x 720
    c.paste(p, (0, 0))
    fw = W - p.width
    img = fit_crop(frame(fr), (x0, 0, x0 + round(1080 * fw / H), 1080), (fw, H))
    c.paste(img, (p.width, 0))
    if label:
        d = ImageDraw.Draw(c)
        text(d, (W - 40, H - 40), 'PART 1: GRAPHICS', font(38), CREAM, spacing=3, anchor='rs')
    return c

# B: full outpost (n=450) with the poster border and title
def B(k=2, bw=22):
    c = fit_crop(frame('full_outpost_450_ud.png'), (0, 0, 1920, 1080), (W, H))
    d = ImageDraw.Draw(c)
    t = title_rgba(k)
    c.paste(t, (bw + 40, bw + 30), t)
    text(d, (bw + 44, bw + 30 + t.height + 52), 'PART 1: GRAPHICS', font(40), RED_TITLE, spacing=4)
    d.rectangle((0, 0, W - 1, bw - 1), fill=RED_BORDER)
    d.rectangle((0, H - bw, W - 1, H - 1), fill=RED_BORDER)
    d.rectangle((0, 0, bw - 1, H - 1), fill=RED_BORDER)
    d.rectangle((W - bw, 0, W - 1, H - 1), fill=RED_BORDER)
    return c

# C: vanilla | UD split, valley mist n=150
def C(split=600, bw=22):
    v = fit_crop(frame('full_valley_mist_150_vanilla.png'), (0, 0, 1920, 1080), (W, H))
    u = fit_crop(frame('full_valley_mist_150_ud.png'), (0, 0, 1920, 1080), (W, H))
    c = v.copy(); c.paste(u.crop((split, 0, W, H)), (split, 0))
    d = ImageDraw.Draw(c)
    d.rectangle((split - 1, 0, split, H), fill=CREAM)
    t = title_rgba(2, RED_TITLE, outline=CREAM)
    c.paste(t, (split + 30, bw + 26), t)
    text(d, (split - 18, H - bw - 28), 'VANILLA', font(34), CREAM, spacing=3, anchor='rs')
    for box in ((0, 0, W - 1, bw - 1), (0, H - bw, W - 1, H - 1), (0, 0, bw - 1, H - 1), (W - bw, 0, W - 1, H - 1)):
        d.rectangle(box, fill=RED_BORDER)
    return c

for n, im in (('A', A()), ('B', B()), ('C', C())):
    im.save(os.path.join(OUT, n + '.png'))
print('ok')
