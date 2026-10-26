"""Builds labelled side-by-side comparisons from a research capture run.

compare.py <run dir> <out dir> <view> <label>[=Title] <label>[=Title] ... [--name NAME] [--crop x0,y0,x1,y1]
Finds <seq>_<view>__<label>.png under <run dir>/tri, reads the GPU frame and
volumetric ("shafts") times from the UDCAP shot lines, and writes
<out dir>/<NAME or view>.png with one panel per label.
"""
import glob
import os
import re
import sys

from PIL import Image, ImageDraw, ImageFont


def find_png(run, view, label):
    pats = glob.glob(os.path.join(run, 'tri', '**', '*_%s__%s.png' % (view, label)), recursive=True)
    pats = [p for p in pats if re.search(r'(^|[\\/])\d+_%s__%s\.png$' % (re.escape(view), re.escape(label)), p)]
    if not pats:
        return None
    return sorted(pats, key=os.path.getmtime)[-1]


def timings(run):
    out = {}
    for log in glob.glob(os.path.join(run, 'tri', '**', 'game_stdout.log'), recursive=True):
        with open(log, encoding='utf-8', errors='replace') as f:
            for line in f:
                if 'UDCAP;' not in line or 'event=shot' not in line:
                    continue
                m = re.search(r'label=([^;]+);', line)
                g = re.search(r'gpu=(.*)$', line)
                if not m or not g:
                    continue
                gs = g.group(1)
                fr = re.search(r'frame ([\d.]+) ms', gs)
                sh = re.search(r'shafts ([\d.]+)', gs)
                wo = re.search(r'world ([\d.]+)', gs)
                out[m.group(1)] = dict(frame=fr.group(1) if fr else '?', shafts=sh.group(1) if sh else None,
                                       world=wo.group(1) if wo else '?')
    return out


def font(sz):
    for f in ('arialbd.ttf', 'arial.ttf', 'DejaVuSans-Bold.ttf'):
        try:
            return ImageFont.truetype(f, sz)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    args = sys.argv[1:]
    run, outdir, view = args[0], args[1], args[2]
    rest = args[3:]
    name = view
    crop = None
    labels = []
    i = 0
    while i < len(rest):
        if rest[i] == '--name':
            name = rest[i + 1]
            i += 2
            continue
        if rest[i] == '--crop':
            crop = tuple(int(v) for v in rest[i + 1].split(','))
            i += 2
            continue
        labels.append(rest[i])
        i += 1
    t = timings(run)
    panels = []
    for lab in labels:
        key, _, title = lab.partition('=')
        title = title or key
        p = find_png(run, view, key)
        if not p:
            print('missing', view, key)
            continue
        im = Image.open(p).convert('RGB')
        if crop:
            im = im.crop(crop)
        tm = t.get('%s__%s' % (view, key), {})
        cap = 'GPU frame %s ms' % tm.get('frame', '?')
        if tm.get('shafts'):
            cap += '  (vol/shafts %s)' % tm['shafts']
        if key.endswith('_dbg'):
            cap = 'scattering only (debug view)'
        panels.append((title, cap, im))
    if not panels:
        sys.exit(1)
    w = sum(p[2].width for p in panels) + 8 * (len(panels) - 1)
    h = max(p[2].height for p in panels) + 40
    sheet = Image.new('RGB', (w, h), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), name, fill=(235, 235, 235), font=font(20))
    x = 0
    f1, f2 = font(20), font(15)
    for title, cap, im in panels:
        sheet.paste(im, (x, 40))
        tb = d.textbbox((0, 0), title, font=f1)
        d.rectangle((x + 6, 46, x + 14 + tb[2], 50 + tb[3] + 4), fill=(0, 0, 0))
        d.text((x + 10, 48), title, fill=(255, 255, 255), font=f1)
        cb = d.textbbox((0, 0), cap, font=f2)
        d.rectangle((x + 6, h - 28, x + 14 + cb[2], h - 6), fill=(0, 0, 0))
        d.text((x + 10, h - 26), cap, fill=(255, 255, 255), font=f2)
        x += im.width + 8
    os.makedirs(outdir, exist_ok=True)
    out = os.path.join(outdir, name + '.png')
    sheet.save(out)
    print('wrote', out, sheet.size)


if __name__ == '__main__':
    main()
