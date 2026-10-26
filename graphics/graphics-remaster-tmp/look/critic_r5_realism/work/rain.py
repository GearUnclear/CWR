from PIL import Image, ImageDraw, ImageFont, ImageStat
L = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
font = ImageFont.truetype('arial.ttf', 13)
W = 320
def tile(p, lab):
    im = Image.open(p).convert('RGB')
    h = round(im.size[1] * W / im.size[0])
    t = im.resize((W, h), Image.LANCZOS)
    g = im.convert('L'); w, hh = g.size
    lo = ImageStat.Stat(g.crop((0, hh // 2, w, hh))).mean[0]
    c = Image.new('RGB', (W, h + 18), (30, 30, 30)); c.paste(t, (0, 18))
    ImageDraw.Draw(c).text((3, 2), '%s low %.1f' % (lab, lo), fill=(255, 255, 255), font=font)
    return c
rows = []
rows.append([(L + 'director/OWNER_wet_seam.webp', 'OWNER report'), (L + 'ref/landed/town_night_rain.png', 'landed'), (L + 'director/round4/final/town_night_rain__overhaul.png', 'R4'), (L + 'director/final/town_night_rain__overhaul.png', 'R5 final')])
for v in ['town_night_rain', 'seam_b', 'seam_d', 'seam_day', 'rain_day']:
    rows.append([(L + 'ref/vanilla/%s.png' % v if v in ('town_night_rain', 'rain_day') else None, 'vanilla ' + v), (L + 'ref/landed/%s.png' % v if v in ('town_night_rain', 'rain_day') else None, 'landed'), (L + 'puddle/4/final/%s.png' % v, 'puddle4 own'), (L + 'director/final/rainviews/%s.png' % v, 'R5 rainview ' + v)])
out = []
for r in rows:
    ts = [tile(p, l) if p else Image.new('RGB', (W, 198), (30, 30, 30)) for p, l in r]
    h = max(t.size[1] for t in ts)
    R = Image.new('RGB', (4 * W + 12, h), (0, 0, 0))
    for i, t in enumerate(ts): R.paste(t, (i * (W + 4), 0))
    out.append(R)
H = sum(r.size[1] + 4 for r in out)
S = Image.new('RGB', (out[0].size[0], H)); y = 0
for r in out: S.paste(r, (0, y)); y += r.size[1] + 4
S.save(L + 'critic_r5_realism/thumb5_rain.png'); print(S.size)
