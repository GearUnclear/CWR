import sys
from PIL import Image, ImageDraw, ImageFont, ImageStat
L = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
font = ImageFont.truetype('arial.ttf', 14)
W = int(sys.argv[2])
views = sys.argv[3].split(',')
kinds = sys.argv[4].split(',') if len(sys.argv) > 4 else ['landed', 'R5']
def P(k, v):
    return {'vanilla': L + 'ref/vanilla/%s.png' % v, 'landed': L + 'ref/landed/%s.png' % v,
            'R4': L + 'director/round4/final/%s__overhaul.png' % v, 'R5': L + 'director/final/%s__overhaul.png' % v}[k]
rows = []
for v in views:
    ts = []
    for k in kinds:
        im = Image.open(P(k, v)).convert('RGB')
        h = round(im.size[1] * W / im.size[0])
        t = Image.new('RGB', (W, h + 20), (30, 30, 30)); t.paste(im.resize((W, h), Image.LANCZOS), (0, 20))
        ImageDraw.Draw(t).text((3, 2), '%s %s' % (k, v), fill=(255, 255, 255), font=font); ts.append(t)
    R = Image.new('RGB', (len(ts) * (W + 4), max(t.size[1] for t in ts)))
    for i, t in enumerate(ts): R.paste(t, (i * (W + 4), 0))
    rows.append(R)
S = Image.new('RGB', (rows[0].size[0], sum(r.size[1] + 4 for r in rows))); y = 0
for r in rows: S.paste(r, (0, y)); y += r.size[1] + 4
S.save(L + 'critic_r5_realism/' + sys.argv[1]); print(S.size)
