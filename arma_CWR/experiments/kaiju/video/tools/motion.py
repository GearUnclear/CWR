"""Numbers that go with reading a clip's frames: where the picture jumps.

    python motion.py <clip.mp4> [more clips] [--json out.json]

For each clip, per frame: the mean brightness of the sky third, the middle third and the ground third
(a step in one band alone is light popping, not motion), and the mean change from the frame before
(a spike is a pop: a level of detail switching, a jump of position, a light going on or off). Prints
the steps and spikes with their times; the strips show what they are.
"""
import argparse
import json
import subprocess

import numpy as np

FF = r'C:\ffmpeg\bin\ffmpeg.exe'


def read(clip, w=90, h=160):
    p = subprocess.run([FF, '-loglevel', 'error', '-i', clip, '-vf', 'scale=%d:%d,format=gray' % (w, h), '-f', 'rawvideo', '-'], capture_output=True)
    return np.frombuffer(p.stdout, dtype=np.uint8).reshape(-1, h, w).astype(np.float32)


def analyse(clip):
    a = read(clip)
    n, h, w = a.shape
    bands = {'sky': a[:, :h // 3], 'middle': a[:, h // 3:2 * h // 3], 'ground': a[:, 2 * h // 3:]}
    out = {'clip': clip, 'frames': int(n), 'steps': [], 'spikes': []}
    for name, b in bands.items():
        m = b.mean(axis=(1, 2))
        d = np.diff(m)
        for i in np.where(np.abs(d) > 2.5)[0]:
            out['steps'].append(dict(band=name, frame=int(i + 1), t=round((i + 1) / 30.0, 2), change=round(float(d[i]), 1), level=round(float(m[i + 1]), 1)))
        out[name + '_range'] = [round(float(m.min()), 1), round(float(m.max()), 1)]
    diff = np.abs(np.diff(a, axis=0)).mean(axis=(1, 2))
    med = float(np.median(diff)) + 1e-6
    out['diff_median'] = round(med, 3)
    for i in np.where(diff > max(4 * med, med + 1.5))[0]:
        out['spikes'].append(dict(frame=int(i + 1), t=round((i + 1) / 30.0, 2), change=round(float(diff[i]), 2), times_median=round(float(diff[i] / med), 1)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('clips', nargs='+')
    ap.add_argument('--json', default='')
    a = ap.parse_args()
    res = []
    for c in a.clips:
        r = analyse(c)
        res.append(r)
        print('%s: %d frames; sky %s, middle %s, ground %s; median frame change %.2f' % (c.replace('\\', '/').split('/')[-1], r['frames'], r['sky_range'], r['middle_range'], r['ground_range'], r['diff_median']))
        steps = sorted(r['steps'], key=lambda s: s['frame'])
        if steps:
            print('  brightness steps (band, time, change): ' + ', '.join('%s %.2fs %+.0f' % (s['band'], s['t'], s['change']) for s in steps[:24]) + (' ... %d in all' % len(steps) if len(steps) > 24 else ''))
        if r['spikes']:
            print('  picture jumps (time, times the median change): ' + ', '.join('%.2fs x%.0f' % (s['t'], s['times_median']) for s in r['spikes'][:24]) + (' ... %d in all' % len(r['spikes']) if len(r['spikes']) > 24 else ''))
    if a.json:
        json.dump(res, open(a.json, 'w'), indent=1)


if __name__ == '__main__':
    main()
