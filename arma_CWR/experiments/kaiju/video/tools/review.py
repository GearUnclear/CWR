"""Review pictures of the finished cut, for an editor who can only look at frames.

    python review.py [--film name.mp4]

Writes final\\contact_sheet.png (one frame per slot, the middle one, with slot name and timecodes),
wip\\review\\cuts_NN.jpg (the three frames before and after every cut) and wip\\review\\slot_<name>.jpg
(a strip of each slot at 5 frames a second).
"""
import argparse
import os
import subprocess

from PIL import Image, ImageDraw, ImageFont

from edit_common import FF, FINAL, FONTS, FPS, WIP, load_edl

OUT = os.path.join(WIP, 'review')


def grab(film, frames, h):
    """The named frames of the film as PIL images, height h."""
    sel = '+'.join('eq(n,%d)' % f for f in frames)
    tmp = os.path.join(OUT, '_grab_%03d.png')
    for f in os.listdir(OUT):
        if f.startswith('_grab_'):
            os.remove(os.path.join(OUT, f))
    subprocess.run([FF, '-y', '-loglevel', 'error', '-i', film, '-vf', "select='%s',scale=-2:%d" % (sel, h), '-vsync', '0', tmp], check=True)
    ims = []
    for i in range(len(frames)):
        p = tmp % (i + 1)
        ims.append(Image.open(p).convert('RGB'))
    for f in os.listdir(OUT):
        if f.startswith('_grab_'):
            os.remove(os.path.join(OUT, f))
    return ims


def tc(t):
    return '%d:%05.2f' % (int(t // 60), t % 60)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--film', default='dedushka_9x16.mp4')
    ap.add_argument('--no-strips', action='store_true')
    a = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    film = os.path.join(FINAL, a.film)
    edl = load_edl()
    slots = edl['slots']
    font = ImageFont.truetype(os.path.join(FONTS, 'cwr_mono.ttf'), 20)
    # ---- contact sheet
    h = 640
    mids = [s['frames'] // 2 + int(round(s['t0'] * FPS)) for s in slots]
    ims = grab(film, mids, h)
    w = ims[0].width
    cols = 6
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * (w + 8) + 8, rows * (h + 54) + 8), (14, 14, 13))
    d = ImageDraw.Draw(sheet)
    for i, (im, s) in enumerate(zip(ims, slots)):
        x = 8 + (i % cols) * (w + 8)
        y = 8 + (i // cols) * (h + 54)
        sheet.paste(im, (x, y))
        d.text((x, y + h + 4), '%02d %s' % (i + 1, s['name']), font=font, fill=(232, 226, 206))
        d.text((x, y + h + 26), '%s to %s  %.1f s' % (tc(s['t0']), tc(s['t1']), s['len']), font=font, fill=(168, 162, 146))
    sheet.save(os.path.join(FINAL, 'contact_sheet.png'))
    print('contact sheet: %d slots' % len(slots))
    if a.no_strips:
        return
    # ---- the cuts: three frames each side
    h = 420
    cuts = [int(round(s['t0'] * FPS)) for s in slots[1:]]
    per = 5
    for k in range(0, len(cuts), per):
        part = cuts[k:k + per]
        frames = []
        for c in part:
            frames += [c - 3, c - 2, c - 1, c, c + 1, c + 2]
        ims = grab(film, frames, h)
        w = ims[0].width
        sheet = Image.new('RGB', (6 * (w + 4) + 30, len(part) * (h + 30)), (14, 14, 13))
        d = ImageDraw.Draw(sheet)
        for r, c in enumerate(part):
            for j in range(6):
                sheet.paste(ims[r * 6 + j], (j * (w + 4) + (26 if j >= 3 else 0), r * (h + 30)))
            d.text((4, r * (h + 30) + h + 4), 'cut %d at %s: %s | %s' % (k + r + 1, tc(c / FPS), slots[k + r]['name'], slots[k + r + 1]['name']), font=font, fill=(232, 226, 206))
        sheet.save(os.path.join(OUT, 'cuts_%02d.jpg' % (k // per + 1)), quality=88)
    # ---- each slot at 5 frames a second
    h = 384
    for s in slots:
        f0 = int(round(s['t0'] * FPS))
        frames = list(range(f0, f0 + s['frames'], 6))
        ims = grab(film, frames, h)
        w = ims[0].width
        cols = 12
        rows = (len(ims) + cols - 1) // cols
        sheet = Image.new('RGB', (cols * (w + 2), rows * (h + 2)), (14, 14, 13))
        for i, im in enumerate(ims):
            sheet.paste(im, ((i % cols) * (w + 2), (i // cols) * (h + 2)))
        sheet.save(os.path.join(OUT, 'slot_%s.jpg' % s['name']), quality=86)
    print('cut strips and slot strips in %s' % OUT)


if __name__ == '__main__':
    main()
