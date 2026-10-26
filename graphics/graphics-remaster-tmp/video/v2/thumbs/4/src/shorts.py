"""Shorts cover 1080x1920: the poster rebuilt with game material. The poster's lettering on its
cream paper, a real Uslu dur! frame (BMP against the low sun) as the picture, the poster's
pixel-art fighter in front, all inside the poster's red frame."""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
from lib import *

W, H, B = 1080, 1920, 40
img = Image.new('RGB', (W, H), RED)
d = ImageDraw.Draw(img)
d.rectangle((B, B, W - B - 1, H - B - 1), fill=CREAM)

LW = W - 2 * B - 60
m = mask('letter_words')
LH = round(LW * m.shape[0] / m.shape[1])
lt = lettering('letter_words', LH, pad=0)
ly = B + 34
img.paste(lt, ((W - lt.size[0]) // 2, ly), lt)

f = font('cwr_title.ttf', 46)
label = 'PART 1: GRAPHICS'
lab_y = ly + LH + 22
text_spaced(d, (W / 2, lab_y), label, f, INK, 5, anchor_center=True)

PY = lab_y + 80
pw, ph = W - 2 * B, H - B - PY
src = frame('outpost', 'ud', 400)
ch = 1080; cw = round(ch * pw / ph)
x0 = int(sys.argv[1]) if len(sys.argv) > 1 else 603
plate = crop_box(src, x0, 0, cw, ch, pw, ph)
img.paste(plate, (B, PY))
sc = ph / ch
print('plate', pw, ph, 'crop', cw, 'sun at', B + (1040 - x0) * sc, PY + 250 * sc)

img = img.convert('RGBA')
fg = fighter(3.0)
canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
fx = B - 30
fy = H - B - fg.size[1] + 60
canvas.paste(fg, (fx, fy), fg)
inner = Image.new('L', (W, H), 0)
ImageDraw.Draw(inner).rectangle((B, PY, W - B - 1, H - B - 1), fill=255)
a = np.array(canvas); a[..., 3] = np.minimum(a[..., 3], np.array(inner)); canvas = Image.fromarray(a)
img.alpha_composite(canvas)
print('fighter', fg.size, 'top', fy)

save(img, 'shorts_cover')
img.convert('RGB').resize((270, 480), Image.LANCZOS).save(f'{ROOT}/check/shorts_270.png')
img.convert('RGB').resize((135, 240), Image.LANCZOS).save(f'{ROOT}/check/shorts_135.png')
print('done')
