"""sheet_small.png: every variant at 360x202 (phone home feed) and 168x94 (sidebar),
on YouTube's dark and light page colours, plus the Shorts cover at feed sizes."""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
from lib import *

names = [('v1_poster_plate', 'V1 poster plate'), ('v2_fighter_outpost', 'V2 fighter meets the outpost'),
         ('v3_poster_split', 'V3 poster split')]
PAD = 16
cap = font('cwr_body.ttf', 14)
rowh = 202 + 24
W = PAD + 360 + PAD + 168 + PAD
blockw = W
sw = 3 * blockw if False else None
# two panels (dark, light) side by side, rows = variants
panel_w = W
panel_h = PAD + len(names) * (rowh + PAD)
short_w = PAD + 135 + PAD + 90 + PAD
sheet = Image.new('RGB', (2 * panel_w + short_w, panel_h), (15, 15, 15))
d = ImageDraw.Draw(sheet)
for pi, (bg, fg) in enumerate([((15, 15, 15), (220, 220, 220)), ((255, 255, 255), (30, 30, 30))]):
    ox = pi * panel_w
    d.rectangle((ox, 0, ox + panel_w - 1, panel_h - 1), fill=bg)
    for i, (n, label) in enumerate(names):
        im = Image.open(f'{ROOT}/{n}.png').convert('RGB')
        y = PAD + i * (rowh + PAD)
        d.text((ox + PAD, y), label, font=cap, fill=fg)
        sheet.paste(im.resize((360, 202), Image.LANCZOS), (ox + PAD, y + 22))
        sheet.paste(im.resize((168, 94), Image.LANCZOS), (ox + PAD + 360 + PAD, y + 22))
ox = 2 * panel_w
d.rectangle((ox, 0, ox + short_w - 1, panel_h - 1), fill=(15, 15, 15))
sc = Image.open(f'{ROOT}/shorts_cover.png').convert('RGB')
d.text((ox + PAD, PAD), 'Shorts cover', font=cap, fill=(220, 220, 220))
sheet.paste(sc.resize((135, 240), Image.LANCZOS), (ox + PAD, PAD + 22))
sheet.paste(sc.resize((90, 160), Image.LANCZOS), (ox + PAD + 135 + PAD, PAD + 22))
sheet.save(f'{ROOT}/sheet_small.png')
print(sheet.size)
