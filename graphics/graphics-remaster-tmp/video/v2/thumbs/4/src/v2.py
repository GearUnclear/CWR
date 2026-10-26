"""V2 the fighter steps out of the poster: the poster's pixel-art fighter in front of a real
Uslu dur! frame (occupier BMP against the low winter sun), inside the poster's red frame."""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
from lib import *

W, H, B = 1280, 720, 22
img = Image.new('RGB', (W, H), RED)
src = frame('outpost', 'ud', 300)
iw, ih = W - 2 * B, H - 2 * B
# crop: keep sun, tent and BMP, trim some empty foreground
cw = 1800; ch = round(cw * ih / iw)
x0, y0 = 100, 0
plate = crop_box(src, x0, y0, cw, ch, iw, ih)
img.paste(plate, (B, B))
sc = iw / cw
print('sun at', (633 - x0) * sc + B, (238 - y0) * sc + B, 'bmp bow y', (450 - y0) * sc + B)

fg = fighter(2.0)
print('fighter size', fg.size)
fx = 40
fy = H - B - fg.size[1] + 40  # feet below the frame edge
canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
canvas.paste(fg, (fx, fy), fg)
# keep fighter inside the red frame
inner = Image.new('L', (W, H), 0)
ImageDraw.Draw(inner).rectangle((B, B, W - B - 1, H - B - 1), fill=255)
a = np.array(canvas)
a[..., 3] = np.minimum(a[..., 3], np.array(inner))
canvas = Image.fromarray(a)
img = img.convert('RGBA'); img.alpha_composite(canvas)

LH = 228
lt = lettering('letter_words', LH, outline=6, pad=8)
lx = W - B - 28 - lt.size[0]
ly = B + 12
img.alpha_composite(lt, (lx, ly))

d = ImageDraw.Draw(img)
f = font('cwr_title.ttf', 34)
label = 'PART 1: GRAPHICS'
tw = text_width(d, label, f, 3)
LX = lx + lt.size[0] - 8 - tw
LY2 = ly + lt.size[1] + 2
import numpy as _np
reg = _np.array(img.convert('L'))[LY2:LY2 + 44, int(LX):int(LX + tw)]
print('label bg luminance', reg.mean(), reg.min(), reg.max())
text_spaced(d, (LX, LY2), label, f, CREAM, 3)
save(img, 'v2_fighter_outpost')
smalls(img, 'v2')
print('done')
