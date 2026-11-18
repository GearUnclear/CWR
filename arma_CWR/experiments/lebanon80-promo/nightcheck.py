"""Night checks (night-diagnosis.md, shoot items 6 and 7) for every delivered clip with "_night_" in its name.

    python nightcheck.py

- Levels, ungraded, on the encoded clip's own Y codes (limited range, black = 16): YMIN (the lowest code in the
  clip), YLOW (the 10th percentile, averaged over frames), YAVG, the share of the frame within 2 codes of black
  (Y <= 18) and the 99th percentile (the lit pool). ffmpeg's signalstats reads the same plane; here the Y plane
  is taken with extractplanes so the codes are not rescaled.
- The feature gate: the "PROMO feat" line logged after the clip's settle must read night=1.00 and a
  moonShadows gate that is off; "moon-key" fails the take.
Writes work\\nightcheck.json and prints one line per clip.
"""
import glob
import json
import os
import re
import subprocess

import numpy as np

from promopaths import FF, FP, ROOT

ANSI = re.compile(r'\x1b\[[0-9;]*m')


def levels(mp4, step=10):
    r = subprocess.run([FP, '-v', 'error', '-select_streams', 'v:0', '-show_entries', 'stream=width,height', '-of', 'csv=p=0', mp4],
                       capture_output=True, text=True)
    w, h = [int(x) for x in r.stdout.strip().split(',')]
    p = subprocess.Popen([FF, '-v', 'error', '-i', mp4, '-vf', "select='not(mod(n\\,%d))',extractplanes=y" % step, '-vsync', '0',
                          '-f', 'rawvideo', '-pix_fmt', 'gray', '-'], stdout=subprocess.PIPE)
    mins, lows, avgs, blk, hi = [], [], [], [], []
    while True:
        b = p.stdout.read(w * h)
        if len(b) < w * h:
            break
        y = np.frombuffer(b, np.uint8)
        mins.append(int(y.min()))
        lows.append(float(np.percentile(y, 10)))
        avgs.append(float(y.mean()))
        blk.append(float((y <= 18).mean()))
        hi.append(float(np.percentile(y, 99)))
    return dict(ymin=min(mins), ylow=round(float(np.mean(lows)), 1), yavg=round(float(np.mean(avgs)), 1),
                near_black=round(float(np.mean(blk)), 2), y99=round(float(np.mean(hi)), 1))


def feats():
    out = {}
    for run in sorted(glob.glob(os.path.join(ROOT, 'work', 'runs', 'promo_*')), key=os.path.getmtime):
        feat = None
        for f in glob.glob(os.path.join(run, '*', 'game_stdout.log')):
            for ln in open(f, encoding='utf-8', errors='replace'):
                ln = ANSI.sub('', ln)
                if '"PROMO feat ' in ln:
                    feat = ln.split('"PROMO feat ', 1)[1]
                m = re.search(r'"PROMO rec (\S+) ', ln)
                if m and feat:
                    out[m.group(1)] = feat
                    feat = None
    return out


def main():
    fs = feats()
    res = {}
    for mp4 in sorted(glob.glob(os.path.join(ROOT, 'clips', '*_night_*.mp4'))):
        name = os.path.basename(mp4)[:-4]
        lv = levels(mp4)
        f = fs.get(name, '')
        night = re.search(r'night=([0-9.]+)', f)
        moon = re.search(r'moonShadows=([^ ]+)', f)
        sun = re.search(r'sun=(-?[0-9.]+)', f)
        gate = dict(night=night.group(1) if night else None, sun=sun.group(1) if sun else None, moonShadows=moon.group(1) if moon else None)
        gate['pass'] = bool(night and night.group(1) == '1.00' and moon and ':off:' in moon.group(1))
        res[name] = dict(levels=lv, gate=gate)
        print('%-52s YMIN %3d YLOW %5.1f YAVG %5.1f near-black %.2f Y99 %5.1f | sun %s night %s moonShadows %s %s' % (
            name, lv['ymin'], lv['ylow'], lv['yavg'], lv['near_black'], lv['y99'], gate['sun'], gate['night'], gate['moonShadows'],
            'PASS' if gate['pass'] else 'FAIL'))
    json.dump(res, open(os.path.join(ROOT, 'work', 'nightcheck.json'), 'w'), indent=1)


if __name__ == '__main__':
    main()
