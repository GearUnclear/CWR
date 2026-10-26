from PIL import Image, ImageDraw, ImageFont, ImageStat, ImageOps
L = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
font = ImageFont.truetype('arial.ttf', 14)
def P(kind, v):
    return {'vanilla': L + 'ref/vanilla/%s.png' % v, 'landed': L + 'ref/landed/%s.png' % v,
            'R4': L + 'director/round4/final/%s__overhaul.png' % v, 'R5': L + 'director/final/%s__overhaul.png' % v}[kind]
def strip(v, box, scale, kinds=('vanilla', 'landed', 'R4', 'R5'), name=None, rel=True):
    tiles = []
    for k in kinds:
        im = Image.open(P(k, v)).convert('RGB')
        w, h = im.size
        b = tuple(int(round(c * (w if i % 2 == 0 else h))) for i, c in enumerate(box)) if rel else box
        c = im.crop(b)
        st = ImageStat.Stat(c)
        c = c.resize((int(c.size[0] * scale), int(c.size[1] * scale)), Image.LANCZOS if scale < 1 else Image.NEAREST)
        t = Image.new('RGB', (c.size[0], c.size[1] + 20), (30, 30, 30)); t.paste(c, (0, 20))
        ImageDraw.Draw(t).text((3, 2), '%s %s rgb(%d,%d,%d) sd %.1f' % (k, v, *st.mean, sum(st.stddev) / 3), fill=(255, 255, 255), font=font)
        tiles.append(t)
    W = sum(t.size[0] + 4 for t in tiles); H = max(t.size[1] for t in tiles)
    S = Image.new('RGB', (W, H))
    x = 0
    for t in tiles: S.paste(t, (x, 0)); x += t.size[0] + 4
    if name: S.save(L + 'critic_r5_realism/' + name)
    return S
def stack(strips, name):
    W = max(s.size[0] for s in strips); H = sum(s.size[1] + 6 for s in strips)
    S = Image.new('RGB', (W, H)); y = 0
    for s in strips: S.paste(s, (0, y)); y += s.size[1] + 6
    S.save(L + 'critic_r5_realism/' + name); print(name, S.size)
# firefights: horizon band x 0..1, y 0.38..0.62 at 960x640, scale 0.5 -> keep at ~1x
stack([strip('firefight_1', (0.0, 0.40, 1.0, 0.58), 0.66), strip('firefight_2', (0.0, 0.40, 1.0, 0.58), 0.66), strip('firefight_3', (0.0, 0.40, 1.0, 0.58), 0.66)], 'firefights_horizon_4way.png')
stack([strip('firefight_2', (0.05, 0.42, 0.35, 0.56), 2.0, kinds=('landed', 'R4', 'R5')), strip('firefight_1', (0.25, 0.42, 0.55, 0.56), 2.0, kinds=('landed', 'R4', 'R5')), strip('firefight_3', (0.0, 0.40, 0.55, 0.56), 1.2, kinds=('landed', 'R4', 'R5'))], 'firefights_bursts_2x.png')
# mist right-hand block
stack([strip('optics_mist_hud', (0.70, 0.35, 1.0, 0.92), 0.6), strip('optics_mist_sight', (0.70, 0.35, 1.0, 0.92), 0.6)], 'mist_right_4way.png')
