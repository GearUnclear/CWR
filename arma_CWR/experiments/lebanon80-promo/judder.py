"""Judder check for promo clips: the per-frame camera log and the picture itself.

    python judder.py <run folder> [--clips <clips dir>] [--only <prefix>] [--px 1.0]

Log (PCF lines of pcStep, one per simulated frame while a clip records):
  - steps: the game time advances 1/30 s every line and the engine frame counter by 1 (a missed or doubled
    camera step shows as a gap or a repeat);
  - tremor: frames where the kaiju camera tremor (kjShakeGain at the camera) is above 0.0005;
  - jerk: the largest second difference of the eye, the target and the raw subject position (cm per frame
    per frame); a step in the subject's position shows as an isolated spike.
Picture (the delivered mp4, 15 head frames already cut): the whole-frame shift between consecutive frames
by phase correlation on a 640 x 360 grey copy, and its second difference. A single-frame judder is a
second-difference spike over --px pixels (in 640-wide pixels) that its neighbours do not share.
"""
import argparse
import glob
import json
import os
import re
import subprocess

import numpy as np

from gate import pcf_lines

from promopaths import FF, ROOT
W, H = 640, 360


def frames(mp4):
    p = subprocess.Popen([FF, '-v', 'error', '-i', mp4, '-vf', f'scale={W}:{H}', '-pix_fmt', 'gray', '-f', 'rawvideo', '-'],
                         stdout=subprocess.PIPE)
    while True:
        b = p.stdout.read(W * H)
        if len(b) < W * H:
            break
        yield np.frombuffer(b, np.uint8).reshape(H, W).astype(np.float32)


WIN = np.outer(np.hanning(H), np.hanning(W)).astype(np.float32)


def shift(a, b):
    fa = np.fft.rfft2((a - a.mean()) * WIN)
    fb = np.fft.rfft2((b - b.mean()) * WIN)
    r = fa * np.conj(fb)
    r /= np.abs(r) + 1e-6
    c = np.fft.irfft2(r, s=a.shape)
    y, x = np.unravel_index(np.argmax(c), c.shape)

    def sub(cm, c0, cp):
        d = cm - 2 * c0 + cp
        return 0.0 if abs(d) < 1e-9 else 0.5 * (cm - cp) / d
    dx = x + sub(c[y, x - 1], c[y, x], c[y, (x + 1) % W])
    dy = y + sub(c[y - 1, x], c[y, x], c[(y + 1) % H, x])
    if dx > W / 2:
        dx -= W
    if dy > H / 2:
        dy -= H
    return dx, dy


def video(mp4, px):
    prev = None
    s = []
    for f in frames(mp4):
        if prev is not None:
            s.append(shift(f, prev))
        prev = f
    s = np.array(s) if s else np.zeros((0, 2))
    if len(s) < 3:
        return dict(frames=len(s) + 1, spikes=[], max_jerk=0.0)
    j = s[2:] - 2 * s[1:-1] + s[:-2]
    jm = np.hypot(j[:, 0], j[:, 1])
    spikes = []
    for i in range(len(jm)):
        nb = max(jm[i - 1] if i > 0 else 0, jm[i + 1] if i + 1 < len(jm) else 0)
        if jm[i] > px and jm[i] > 1.5 * min(nb, jm[i]) and jm[i] > 0.6 * nb:
            spikes.append((i + 1, round(float(jm[i]), 2)))
    return dict(frames=len(s) + 1, spikes=spikes, max_jerk=round(float(jm.max()), 2),
                mean_shift=round(float(np.hypot(s[:, 0], s[:, 1]).mean()), 2))


def log_checks(rows, head):
    rows = sorted(rows, key=lambda r: r['n'])
    dts = [b['t'] - a['t'] for a, b in zip(rows, rows[1:])]
    dfs = [b['frame'] - a['frame'] for a, b in zip(rows, rows[1:])]
    bad_dt = [i for i, d in enumerate(dts) if abs(d - 1 / 30) > 0.004]
    bad_df = [i for i, d in enumerate(dfs) if d != 1]
    gain = [r['n'] - head for r in rows if r['gain'] > 0.0005 and r['n'] >= head]

    def jerk(key):
        p = np.array([r[key] for r in rows])
        if len(p) < 3:
            return 0.0
        j = p[2:] - 2 * p[1:-1] + p[:-2]
        return round(float(np.abs(j).max() * 100), 2)
    return dict(lines=len(rows), bad_dt=len(bad_dt), bad_frame=len(bad_df), tremor_frames=len(gain),
                tremor_first=(gain[0] if gain else None), max_gain=round(max([r['gain'] for r in rows] or [0]), 4),
                jerk_eye_cm=jerk('eye'), jerk_tgt_cm=jerk('tgt'), jerk_subj_cm=jerk('raw'))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('run')
    ap.add_argument('--clips', default=os.path.join(ROOT, 'clips'))
    ap.add_argument('--only', default='')
    ap.add_argument('--px', type=float, default=1.0)
    ap.add_argument('--head', type=int, default=15)
    a = ap.parse_args()
    rows = pcf_lines(a.run)
    out = {}
    for name in sorted(rows):
        if not name.startswith(a.only):
            continue
        lc = log_checks(rows[name], a.head)
        mp4 = os.path.join(a.clips, name + '.mp4')
        vc = video(mp4, a.px) if os.path.exists(mp4) else {}
        out[name] = dict(log=lc, video=vc)
        print('%s\n   log: %s\n   video: frames %s, max jerk %s px, spikes %s' % (
            name, json.dumps(lc), vc.get('frames'), vc.get('max_jerk'), vc.get('spikes', [])[:12]))
    os.makedirs(os.path.join(ROOT, 'work', 'judder'), exist_ok=True)
    json.dump(out, open(os.path.join(ROOT, 'work', 'judder', os.path.basename(a.run.rstrip('\\/')) + '.json'), 'w'), indent=1)


if __name__ == '__main__':
    main()
