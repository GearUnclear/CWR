"""V1 poster plate: the poster's red frame and lettering, a real Uslu dur! frame as the picture."""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
from lib import *

W, H, B = 1280, 720, 22
img = Image.new('RGB', (W, H), RED)
d = ImageDraw.Draw(img)
d.rectangle((B, B, W - B - 1, H - B - 1), fill=CREAM)

PX = 360  # plate left edge
pw, ph = W - B - PX, H - 2 * B
src = frame('valley_mist', 'ud', 350)
cw = int(sys.argv[1]) if len(sys.argv) > 1 else 1220
ch = round(cw * ph / pw)
X0 = int(sys.argv[2]) if len(sys.argv) > 2 else 1920 - cw
Y0 = int(sys.argv[3]) if len(sys.argv) > 3 else 70
plate = crop_box(src, X0, Y0, cw, ch, pw, ph)
img.paste(plate, (PX, B))

LH = 276
u = lettering('letter_uslu', LH, pad=0)
du = lettering('letter_dur', LH, pad=0)
colw = PX - B
y0 = B + 26
img.paste(u, (B + (colw - u.size[0]) // 2, y0), u)
img.paste(du, (B + (colw - du.size[0]) // 2, y0 + LH + 18), du)

f = font('cwr_title.ttf', 34)
label = 'PART 1: GRAPHICS'
tw = text_width(d, label, f, 3)
text_spaced(d, (B + colw / 2, H - B - 64), label, f, INK, 3, anchor_center=True)

TAG = sys.argv[4] if len(sys.argv) > 4 else ''
save(img, 'v1_poster_plate' + TAG)
smalls(img, 'v1' + TAG)
print('done')
