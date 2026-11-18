"""After a take: wait for its encodes, then the pin/settle check, the thirds gate (and thirds sheet), the judder
check and the pop scan, one summary line per clip.

    python post_take.py <run folder> [--no-wait]

The pop scan is popscan.py (960 x 540 grey, 20 px blocks, a block flags when its frame-to-frame change is a
one-frame step over 1.5 x both neighbours); a clip with a frame of 40 or more flagged blocks (3 percent of
the picture) is listed for a look: a whole-scene pop, a judder, or an explosion.
Results: <UD_PROMO_DIR>\\work\\post\\<run>.popscan.json.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time

from popscan import scan
from promopaths import ROOT

HERE = os.path.dirname(os.path.abspath(__file__))


def popscan(path):
    """(frame, flagged blocks) for every frame with a one-frame step"""
    return [(r[0], r[1]) for r in scan(path)[0]]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('run')
    ap.add_argument('--no-wait', action='store_true')
    a = ap.parse_args()
    run = a.run.rstrip('\\/')
    base = os.path.basename(run)
    take = re.sub(r'-\d{4}-\d{6}$', '', base)
    shots = json.load(open(os.path.join(HERE, take + '.shots.json'), encoding='utf-8'))
    names = [c['name'] for c in shots['clips']]
    enc = os.path.join(run, 'encode.log')
    t0 = time.time()
    import glob
    # the encoder deletes each dump once its mp4 reads back: done when no dump is left (or one was kept)
    while not a.no_wait and time.time() - t0 < 1800:
        raws = glob.glob(os.path.join(run, '**', '*.raw'), recursive=True)
        log = open(enc, encoding='utf-8', errors='replace').read() if os.path.exists(enc) else ''
        if os.path.exists(enc) and (not raws or all(os.path.basename(r)[:-4] + ':' in log for r in raws)) and not os.path.exists(os.path.join(run, '.encoding')):
            if 'frames' in log or not raws:
                break
        time.sleep(5)
    print('==== check_run')
    subprocess.run([sys.executable, os.path.join(HERE, 'check_run.py'), run])
    print('==== gate')
    subprocess.run([sys.executable, os.path.join(HERE, 'gate.py'), run])
    print('==== judder')
    jd = subprocess.run([sys.executable, os.path.join(HERE, 'judder.py'), run], capture_output=True, text=True).stdout
    print(jd)
    print('==== popscan')
    out = {}
    for n in names:
        mp4 = os.path.join(ROOT, 'clips', n + '.mp4')
        if not os.path.exists(mp4):
            print('%-55s missing' % n)
            continue
        r = popscan(mp4)
        mx = max([c for _, c in r] or [0])
        big = [(f, c) for f, c in r if c >= 40]
        out[n] = dict(popframes=len(r), blocks=sum(c for _, c in r), max_blocks=mx, big=big)
        print('%-55s popframes %3d blocks %4d max/frame %3d %s' % (n, len(r), out[n]['blocks'], mx, ('LOOK ' + str(big[:6])) if big else ''))
    os.makedirs(os.path.join(ROOT, 'work', 'post'), exist_ok=True)
    json.dump(out, open(os.path.join(ROOT, 'work', 'post', base + '.popscan.json'), 'w'), indent=1)


if __name__ == '__main__':
    main()
