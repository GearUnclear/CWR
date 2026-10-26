"""Pan sequence strip and GIFs: rows = variants, columns = frames of the pan.

pan_strip.py <run dir> <out dir> <view> <name> <crop x0,y0,x1,y1 or -> <label>=<title> ...
Frames are <view>__<label>_pNN; the strip shows every frame of each label in
order, and one GIF per label (all frames, 250 ms each) is written beside it.
"""
import glob
import os
import re
import sys

from PIL import Image, ImageDraw, ImageFont


def font(sz):
    for f in ('arialbd.ttf', 'arial.ttf'):
        try:
            return ImageFont.truetype(f, sz)
        except OSError:
            pass
    return ImageFont.load_default()


run, outdir, view, name, crop = sys.argv[1:6]
crop = None if crop == '-' else tuple(int(v) for v in crop.split(','))
rows = []
for spec in sys.argv[6:]:
    lab, _, title = spec.partition('=')
    files = glob.glob(os.path.join(run, 'tri', '**', '*_%s__%s_p*.png' % (view, lab)), recursive=True)
    files.sort(key=lambda p: int(re.search(r'_p(\d+)\.png$', p).group(1)))
    ims = []
    for p in files:
        im = Image.open(p).convert('RGB')
        if crop:
            im = im.crop(crop)
        ims.append((re.search(r'_p(\d+)\.png$', p).group(1), im))
    rows.append((title or lab, lab, ims))
    gif = [im for _, im in ims]
    if gif:
        gif[0].save(os.path.join(outdir, '%s_%s.gif' % (name, lab)), save_all=True, append_images=gif[1:],
                    duration=250, loop=0)
cw = max(im.width for _, _, ims in rows for _, im in ims)
ch = max(im.height for _, _, ims in rows for _, im in ims)
ncol = max(len(ims) for _, _, ims in rows)
W = ncol * cw + (ncol - 1) * 6
H = 36 + len(rows) * (ch + 6)
sheet = Image.new('RGB', (W, H), (24, 24, 24))
d = ImageDraw.Draw(sheet)
d.text((10, 8), name + '  (columns: pan frames, 0.3 deg per frame)', fill=(235, 235, 235), font=font(18))
f1 = font(16)
for r, (title, lab, ims) in enumerate(rows):
    y = 36 + r * (ch + 6)
    for c, (fr, im) in enumerate(ims):
        x = c * (cw + 6)
        sheet.paste(im, (x, y))
        t = '%s  frame %s' % (title, fr)
        tb = d.textbbox((0, 0), t, font=f1)
        d.rectangle((x + 4, y + 4, x + 12 + tb[2], y + 8 + tb[3]), fill=(0, 0, 0))
        d.text((x + 8, y + 6), t, fill=(255, 255, 255), font=f1)
out = os.path.join(outdir, name + '.png')
sheet.save(out)
print('wrote', out, sheet.size)
