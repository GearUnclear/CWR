"""Cards, caption overlays and the film-gate furniture for the Dedushka film record.

    python make_cards.py            # everything, from work\\edl.json (written by make_edl.py)

Writes work\\cards\\<name>.png (full frame cards), work\\captions\\<id>.png (transparent overlays,
one per caption), work\\gate.png (the soft dark edge of the film gate with the archive mark) and
work\\dust\\NNN.png (a loop of dust, hairs and scratches). Fonts: the OFL guerrilla fonts.
No em dashes anywhere.
"""
import json
import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter, ImageFont

from edit_common import FONTS, H, W, WORK, load_edl

INK = (232, 226, 206)        # the off-white of an old typed card
DIM = (168, 162, 146)
RED = (190, 52, 34)          # stamp red
ORANGE = (236, 120, 36)      # the furnace
BLACK = (9, 9, 8)


def font(name, size):
    return ImageFont.truetype(os.path.join(FONTS, 'cwr_%s.ttf' % name), size)


def text_w(d, s, f, track=0):
    return d.textlength(s, font=f) + track * max(0, len(s) - 1)


def draw_tracked(d, xy, s, f, fill, track=0):
    x, y = xy
    if not track:
        d.text((x, y), s, font=f, fill=fill)
        return
    for ch in s:
        d.text((x, y), ch, font=f, fill=fill)
        x += d.textlength(ch, font=f) + track


def paper_noise(im, amount=10, seed=1):
    rnd = random.Random(seed)
    px = im.load()
    for _ in range(W * H // 28):
        x, y = rnd.randrange(W), rnd.randrange(H)
        r, g, b = px[x, y][:3]
        k = rnd.randint(-amount, amount)
        px[x, y] = (max(0, min(255, r + k)), max(0, min(255, g + k)), max(0, min(255, b + k)))
    return im


def stamp(text, size=64, angle=-11, colour=RED, box=True):
    f = font('title', size)
    tmp = Image.new('RGBA', (10, 10))
    d = ImageDraw.Draw(tmp)
    w = int(text_w(d, text, f, 6)) + 60
    h = size + 56
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if box:
        d.rectangle([4, 4, w - 5, h - 5], outline=colour + (235,), width=6)
    draw_tracked(d, (30, 14), text, f, colour + (235,), 6)
    # worn ink: knock holes in the alpha
    rnd = random.Random(7)
    a = im.split()[3]
    pa = a.load()
    for _ in range(w * h // 5):
        x, y = rnd.randrange(w), rnd.randrange(h)
        pa[x, y] = int(pa[x, y] * rnd.choice([0.2, 0.5, 0.8]))
    a = a.filter(ImageFilter.GaussianBlur(0.6))
    im.putalpha(a)
    return im.rotate(angle, expand=True, resample=Image.BICUBIC)


def card_leader():
    im = Image.new('RGB', (W, H), BLACK)
    d = ImageDraw.Draw(im)
    mono = font('mono', 40)
    small = font('mono', 30)
    big = font('title', 132)
    y = 330
    for s in ['МИНИСТЕРСТВО ОБОРОНЫ СССР', 'MINISTRY OF DEFENCE OF THE USSR']:
        d.text(((W - text_w(d, s, small)) / 2, y), s, font=small, fill=DIM)
        y += 44
    y += 22
    s = 'MILITARY FILM UNIT, EVERON GARRISON'
    d.text(((W - text_w(d, s, small)) / 2, y), s, font=small, fill=DIM)
    y += 120
    d.line([(150, y), (W - 150, y)], fill=INK, width=3)
    y += 44
    s = 'FILM RECORD'
    draw_tracked(d, ((W - text_w(d, s, big, 8)) / 2, y), s, big, INK, 8)
    y += 176
    s = 'No. 0417'
    f2 = font('title', 96)
    draw_tracked(d, ((W - text_w(d, s, f2, 6)) / 2, y), s, f2, INK, 6)
    y += 150
    d.line([(150, y), (W - 150, y)], fill=INK, width=3)
    y += 70
    for s in ['REEL 2 OF 3', 'EVERON, 11 TO 14 JUNE 1985', '', 'SOUND: UNIT RECORDER,', 'NOT SYNCHRONISED', '', 'NOT FOR EXHIBITION']:
        if s:
            d.text(((W - text_w(d, s, mono)) / 2, y), s, font=mono, fill=INK)
        y += 58
    st = stamp('СЕКРЕТНО', 84, -9)
    im.paste(st, (W - st.width - 90, 1470), st)
    return paper_noise(im, 9, 3)


def card_brand():
    im = Image.new('RGB', (W, H), BLACK)
    d = ImageDraw.Draw(im)
    big = font('title', 236)
    s = 'Uslu dur!'
    w = text_w(d, s, big)
    y = 760
    d.rectangle([150, y - 60, W - 150, y - 52], fill=ORANGE)
    d.text(((W - w) / 2, y - 30), s, font=big, fill=INK)
    d.rectangle([150, y + 330, W - 150, y + 338], fill=ORANGE)
    small = font('mono', 36)
    s2 = 'ДЕДУШКА'
    draw_tracked(d, ((W - text_w(d, s2, small, 14)) / 2, y + 384), s2, small, DIM, 14)
    # the music credit (the owner's track for this demonstration cut; licence not known to us)
    tiny = font('mono', 28)
    for i, s3 in enumerate(['MUSIC', "\"Dream Steppin'\" by Two People"]):
        d.text(((W - text_w(d, s3, tiny)) / 2, 1640 + i * 44), s3, font=tiny, fill=DIM if i == 0 else INK)
    return paper_noise(im, 8, 5)


def card_black():
    return paper_noise(Image.new('RGB', (W, H), BLACK), 6, 9)


def caption(cap):
    """A typed annotation in the lower third: an optional header line (small, tracked, with a rule
    under it) and up to three body lines. Off-white with a soft dark halo so it reads on any shot."""
    head = cap.get('head', '')
    lines = cap.get('lines', [])
    fh = font('mono', 34)
    fb = font('mono', int(cap.get('size', 50)))
    im = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    x = 84
    lh = int(fb.size * 1.32)
    total = (56 if head else 0) + lh * len(lines)
    y = int(cap.get('y', 1500)) - total // 2
    items = []
    if head:
        items.append(('h', x, y, head))
        y += 56
    for s in lines:
        items.append(('b', x, y, s))
        y += lh
    # halo
    halo = Image.new('L', (W, H), 0)
    hd = ImageDraw.Draw(halo)
    for kind, tx, ty, s in items:
        f = fh if kind == 'h' else fb
        for dx in (-3, 0, 3):
            for dy in (-3, 0, 3):
                if kind == 'h':
                    draw_tracked(hd, (tx + dx, ty + dy), s, f, 255, 3)
                else:
                    hd.text((tx + dx, ty + dy), s, font=f, fill=255)
    halo = halo.filter(ImageFilter.GaussianBlur(7)).point(lambda v: min(255, int(v * 2.2)))
    im.paste(Image.new('RGBA', (W, H), (0, 0, 0, 215)), (0, 0), halo)
    d = ImageDraw.Draw(im)
    for kind, tx, ty, s in items:
        if kind == 'h':
            draw_tracked(d, (tx, ty), s, fh, ORANGE if cap.get('hot') else DIM, 3)
            wline = text_w(d, s, fh, 3)
            d.line([(tx, ty + 46), (tx + wline, ty + 46)], fill=(DIM if not cap.get('hot') else ORANGE) + (200,), width=2)
        else:
            d.text((tx, ty), s, font=fb, fill=INK, stroke_width=1, stroke_fill=INK)
    return im


def gate():
    """The film gate: soft dark rounded edge, slightly uneven, and the archive mark top left."""
    rnd = random.Random(11)
    mask = Image.new('L', (W, H), 0)
    d = ImageDraw.Draw(mask)
    d.rounded_rectangle([14, 12, W - 15, H - 13], radius=46, fill=255)
    # a nick and a hair in the gate
    d.polygon([(W - 15, 640), (W - 30, 652), (W - 15, 668)], fill=0)
    mask = mask.filter(ImageFilter.GaussianBlur(5))
    im = Image.new('RGBA', (W, H), (4, 4, 3, 255))
    im.putalpha(mask.point(lambda v: 255 - v))
    d = ImageDraw.Draw(im)
    f = font('mono', 22)
    draw_tracked(d, (48, 40), 'КИНОДОКУМЕНТ 0417 / 2', f, (232, 226, 206, 105), 2)
    _ = rnd
    return im


def dust(n=48):
    out = os.path.join(WORK, 'dust')
    os.makedirs(out, exist_ok=True)
    rnd = random.Random(1985)
    scratch_x = [rnd.randrange(60, W - 60) for _ in range(3)]
    for i in range(n):
        im = Image.new('L', (W, H), 255)
        d = ImageDraw.Draw(im)
        for _ in range(rnd.randint(1, 5)):                      # dust specks
            x, y, r = rnd.randrange(W), rnd.randrange(H), rnd.choice([1, 1, 2, 2, 3])
            d.ellipse([x - r, y - r, x + r, y + r], fill=rnd.randint(40, 120))
        if rnd.random() < 0.10:                                  # a hair
            x, y = rnd.randrange(W), rnd.randrange(H)
            pts = [(x, y)]
            a = rnd.random() * 6.28
            for _ in range(14):
                a += rnd.uniform(-0.5, 0.5)
                x += 9 * math.cos(a)
                y += 9 * math.sin(a)
                pts.append((x, y))
            d.line(pts, fill=70, width=2)
        for k, sx in enumerate(scratch_x):                       # a running scratch that comes and goes
            if (i // 12 + k) % 3 == 0 and rnd.random() < 0.8:
                x = sx + rnd.randint(-2, 2)
                d.line([(x, 0), (x + rnd.randint(-3, 3), H)], fill=rnd.randint(150, 205), width=1)
        im = im.filter(ImageFilter.GaussianBlur(0.6))
        im.save(os.path.join(out, '%03d.png' % i))


def main():
    cards = os.path.join(WORK, 'cards')
    caps = os.path.join(WORK, 'captions')
    os.makedirs(cards, exist_ok=True)
    os.makedirs(caps, exist_ok=True)
    card_leader().save(os.path.join(cards, 'leader.png'))
    card_brand().save(os.path.join(cards, 'brand.png'))
    card_black().save(os.path.join(cards, 'black.png'))
    gate().save(os.path.join(WORK, 'gate.png'))
    dust()
    try:
        edl = load_edl()
    except FileNotFoundError:
        edl = {'captions': []}
    for cap in edl.get('captions', []):
        caption(cap).save(os.path.join(caps, cap['id'] + '.png'))
    # a legibility preview: every caption over mid grey and over black
    prev = Image.new('RGB', (W // 2 * max(1, len(edl.get('captions', []))), H // 2), (60, 58, 52))
    for i, cap in enumerate(edl.get('captions', [])):
        bg = Image.new('RGB', (W, H), (120, 112, 100) if i % 2 else (20, 18, 16))
        c = Image.open(os.path.join(caps, cap['id'] + '.png'))
        bg.paste(c, (0, 0), c)
        prev.paste(bg.resize((W // 2, H // 2)), (i * W // 2, 0))
    prev.save(os.path.join(WORK, 'captions_preview.jpg'), quality=85)
    print('cards: leader, brand, black; gate; dust loop; %d captions' % len(edl.get('captions', [])))
    json.dump({'ok': True}, open(os.path.join(WORK, 'cards.json'), 'w'))


if __name__ == '__main__':
    main()
