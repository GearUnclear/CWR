"""V3 poster split: the poster's frame and lettering; the picture is the trailer's comparison,
vanilla left of a thin divider, Uslu dur! right (same frame index, same camera)."""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
from lib import *

W, H, B = 1280, 720, 22
img = Image.new('RGB', (W, H), RED)
d = ImageDraw.Draw(img)
d.rectangle((B, B, W - B - 1, H - B - 1), fill=CREAM)

PX = 360
pw, ph = W - B - PX, H - 2 * B
N = 400
ud = frame('outpost', 'ud', N)
va = frame('outpost', 'vanilla', N)
cw = 1240; ch = round(cw * ph / pw)
x0, y0 = 400, 60
pu = crop_box(ud, x0, y0, cw, ch, pw, ph)
pv = crop_box(va, x0, y0, cw, ch, pw, ph)
DIV = int(sys.argv[1]) if len(sys.argv) > 1 else 330   # divider x inside the plate
plate = pu.copy()
plate.paste(pv.crop((0, 0, DIV, ph)), (0, 0))
pd = ImageDraw.Draw(plate)
pd.line([(DIV, 0), (DIV, ph)], fill=(245, 240, 228), width=3)
fl = font('cwr_title.ttf', 26)
tw = text_width(pd, 'VANILLA', fl, 3)
LY = ph - 52
text_spaced(pd, (DIV - 14 - tw, LY), 'VANILLA', fl, (245, 240, 228), 3)
text_spaced(pd, (DIV + 14, LY), 'USLU DUR!', fl, (245, 240, 228), 3)
img.paste(plate, (PX, B))

LH = 276
u = lettering('letter_uslu', LH, pad=0)
du = lettering('letter_dur', LH, pad=0)
colw = PX - B
yy = B + 26
img.paste(u, (B + (colw - u.size[0]) // 2, yy), u)
img.paste(du, (B + (colw - du.size[0]) // 2, yy + LH + 18), du)
f = font('cwr_title.ttf', 34)
text_spaced(d, (B + colw / 2, H - B - 64), 'PART 1: GRAPHICS', f, INK, 3, anchor_center=True)

save(img, 'v3_poster_split')
smalls(img, 'v3')
print('done')
