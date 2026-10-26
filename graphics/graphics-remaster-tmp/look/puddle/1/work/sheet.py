"""Compare sheet: rows = the five views, columns = round 3 | first rework | this
(full frame scaled to 640x360), then 2x crops of the tree base and the open
street of town_night_rain. Usage: python sheet.py <mine dir with tri/> <out.png>"""
import sys
from PIL import Image, ImageDraw, ImageFont

GR = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
R3 = GR + 'fix/seam/shots_4/tri/'
R4 = GR + 'fix4/shine/shots_3/tri/'
mine = sys.argv[1].rstrip('/') + '/'
out = sys.argv[2]

views = [
    ('town_night_rain', 'look_probe_f2_abel/000_town_night_rain__ovh.png', 'look_shine_abel/000_town_night_rain__ovh.png'),
    ('seam_b', 'look_probe_f2_abel/001_seam_b__ovh.png', 'look_shine_abel/001_seam_b__ovh.png'),
    ('seam_d', 'look_probe_f2_abel/003_seam_d__ovh.png', 'look_shine_abel/002_seam_d__ovh.png'),
    ('seam_day', 'look_probe_f2_abel/004_seam_day__ovh.png', 'look_shine_abel/003_seam_day__ovh.png'),
    ('rain_day', 'look_probe_f2_eden/000_rain_day__ovh.png', 'look_shine_eden/000_rain_day__ovh.png'),
]
cols = ['round 3 (shiny)', 'first rework', 'this (physically based)']
W, H = 640, 360
crops = [('2x tree base', (780, 609, 1101, 789)), ('2x open street', (330, 789, 651, 969))]
lab = 24
rows = len(views) + len(crops)
sheet = Image.new('RGB', (W * 3, lab + rows * H), (16, 16, 16))
d = ImageDraw.Draw(sheet)
try:
    font = ImageFont.truetype('arial.ttf', 16)
except OSError:
    font = ImageFont.load_default()
d.text((8, 3), 'only puddles reflect: ' + ' | '.join(cols) + '  (rows: views, then 2x crops of town_night_rain)', fill=(230, 230, 230), font=font)


def load(c, rel3, rel4):
    if c == 0:
        return Image.open(R3 + rel3).convert('RGB')
    if c == 1:
        return Image.open(R4 + rel4).convert('RGB')
    return Image.open(mine + rel4).convert('RGB')


tnr = []
for r, (name, rel3, rel4) in enumerate(views):
    for c in range(3):
        im = load(c, rel3, rel4)
        if r == 0:
            tnr.append(im)
        sheet.paste(im.resize((W, H), Image.LANCZOS), (c * W, lab + r * H))
        d.rectangle([c * W, lab + r * H, c * W + 300, lab + r * H + 20], fill=(0, 0, 0))
        d.text((c * W + 5, lab + r * H + 2), name + ': ' + cols[c], fill=(255, 200, 40), font=font)
        if r == 0:
            for (cn, box), colr in zip(crops, [(255, 200, 40), (60, 190, 230)]):
                b = [c * W + box[0] // 3, lab + box[1] // 3, c * W + box[2] // 3, lab + box[3] // 3]
                d.rectangle(b, outline=colr, width=1)
for k, (cn, box) in enumerate(crops):
    r = len(views) + k
    for c in range(3):
        cr = tnr[c].crop(box).resize((W, int(W * (box[3] - box[1]) / (box[2] - box[0]))), Image.LANCZOS)
        sheet.paste(cr.crop((0, 0, W, H)), (c * W, lab + r * H))
        d.rectangle([c * W, lab + r * H, c * W + 200, lab + r * H + 20], fill=(0, 0, 0))
        d.text((c * W + 5, lab + r * H + 2), cn, fill=(255, 200, 40), font=font)
sheet.save(out)
print('wrote', out, sheet.size)
