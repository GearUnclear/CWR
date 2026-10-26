import json, os, sys
from PIL import Image, ImageDraw, ImageFont, ImageStat
L = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
OUT = L + 'critic_r5_realism/'
idx = json.load(open(L + 'ref/index.json'))
views = [v['view'] for v in idx['views']]
W = 320
try:
    font = ImageFont.truetype('arial.ttf', 13)
except Exception:
    font = ImageFont.load_default()
def src(view):
    return [('vanilla', L + 'ref/vanilla/%s.png' % view),
            ('landed', L + 'ref/landed/%s.png' % view),
            ('R4', L + 'director/round4/final/%s__overhaul.png' % view.replace('__1080', '__1080')),
            ('R5', L + 'director/final/%s__overhaul.png' % view)]
def lum_stats(im):
    g = im.convert('L')
    w, h = g.size
    m = ImageStat.Stat(g).mean[0]
    lo = ImageStat.Stat(g.crop((0, h // 2, w, h))).mean[0]
    hi = ImageStat.Stat(g.crop((0, 0, w, h // 3))).mean[0]
    return m, lo, hi
stats = {}
def row(view):
    tiles = []
    st = {}
    for lab, p in src(view):
        if not os.path.exists(p):
            tiles.append((lab, None)); continue
        im = Image.open(p).convert('RGB')
        st[lab] = [round(x, 1) for x in lum_stats(im)]
        h = round(im.size[1] * W / im.size[0])
        tiles.append((lab, im.resize((W, h), Image.LANCZOS)))
    stats[view] = st
    h = max(t.size[1] for _, t in tiles if t is not None)
    r = Image.new('RGB', (4 * W + 3 * 4, h + 18), (30, 30, 30))
    d = ImageDraw.Draw(r)
    for i, (lab, t) in enumerate(tiles):
        x = i * (W + 4)
        if t is not None:
            r.paste(t, (x, 18))
        s = st.get(lab)
        d.text((x + 3, 2), '%s  %s  mean %s low %s' % (view if i == 0 else '', lab, s[0] if s else '-', s[1] if s else '-'), fill=(255, 255, 255), font=font)
    return r
groups = {
 'thumb5_lowsun': ['woods_pine2_evening', 'sinai_lowsun_fp', 'sinai_lowsun_fp__1080', 'lowsun_fp', 'lowsun_fp__1080', 'pine_rays', 'dawn_valley'],
 'thumb5_sunset': ['sunrise_ridge', 'sunrise_ridge__1080', 'coastline_sunset', 'coastline_sunset__1080', 'forest_edge_golden', 'forest_edge_golden__1080', 'blue_hour_coast'],
 'thumb5_mist_grey': ['optics_mist_hud', 'optics_mist_sight', 'optics_mist_sight_f1', 'rain_day', 'overcast_heavy', 'overcast_heavy__1080', 'overcast_broken', 'overcast_broken__1080'],
 'thumb5_noon': ['noon_fields', 'noon_fields__1080', 'noon_vista', 'town_noon', 'jeep_close_noon', 'sinai_desert_noon', 'sinai_desert_noon__1080', 'forest_morning'],
 'thumb5_night_combat': ['town_night', 'town_night__1080', 'town_fog_night', 'town_night_rain', 'firefight_1', 'firefight_2', 'firefight_3', 'tank_gunner_interior', 'tank_gunner_optics'],
}
used = set(sum(groups.values(), []))
missing = [v for v in views if v not in used]
print('missing from groups', missing)
for name, vs in groups.items():
    rows = [row(v) for v in vs if v in views]
    H = sum(r.size[1] + 4 for r in rows)
    sh = Image.new('RGB', (rows[0].size[0], H), (0, 0, 0))
    y = 0
    for r in rows:
        sh.paste(r, (0, y)); y += r.size[1] + 4
    sh.save(OUT + name + '.png')
    print(name, sh.size)
json.dump(stats, open(OUT + 'work/stats.json', 'w'), indent=1)
for v, s in stats.items():
    print(v, s)
