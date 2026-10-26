from PIL import Image, ImageDraw, ImageFont, ImageOps, ImageStat
L = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
font = ImageFont.truetype('arial.ttf', 14)
srcs = [('OWNER report', L + 'director/OWNER_wet_seam.webp'), ('landed', L + 'ref/landed/town_night_rain.png'), ('R4', L + 'director/round4/final/town_night_rain__overhaul.png'),
        ('R5 final', L + 'director/final/town_night_rain__overhaul.png'), ('R5 rainview', L + 'director/final/rainviews/town_night_rain.png'), ('puddle4 own', L + 'puddle/4/final/town_night_rain.png')]
box = (760, 520, 1300, 900)
rows = []
for eq in (False, True):
    ts = []
    for lab, p in srcs:
        c = Image.open(p).convert('RGB').crop(box)
        if eq: c = ImageOps.equalize(c)
        c = c.resize((c.size[0] * 2 // 3, c.size[1] * 2 // 3), Image.LANCZOS)
        t = Image.new('RGB', (c.size[0], c.size[1] + 20), (30, 30, 30)); t.paste(c, (0, 20))
        ImageDraw.Draw(t).text((3, 2), lab + (' equalized' if eq else ''), fill=(255, 255, 255), font=font); ts.append(t)
    R = Image.new('RGB', (len(ts) * (ts[0].size[0] + 4), ts[0].size[1]))
    for i, t in enumerate(ts): R.paste(t, (i * (t.size[0] + 4), 0))
    rows.append(R)
S = Image.new('RGB', (rows[0].size[0], sum(r.size[1] + 4 for r in rows))); y = 0
for r in rows: S.paste(r, (0, y)); y += r.size[1] + 4
S.save(L + 'critic_r5_realism/seam_owner_region_r5.png'); print(S.size)
