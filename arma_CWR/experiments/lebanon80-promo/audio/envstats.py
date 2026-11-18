"""RMS envelope stats: a residual vocal stem against the instrumental, split by where the original vocal was.

    python envstats.py <original vocal stem.wav> <instrumental.wav> <residual.wav> [--sections 0-14.6,14.6-50 ...]

Envelopes are 0.4 s RMS windows every 0.1 s, in dBFS. The original vocal stem is "active" above -35 dBFS
and "quiet" under -60. A clean instrumental keeps the residual near its noise floor in the active windows
(the residual is the vocal stem of a second separation run on the instrumental). --sections adds the same
stats per time range (seconds), for example per section of the beat map. Prints JSON.
"""
import argparse
import json

import numpy as np
import soundfile as sf

WIN_S, HOP_S = 0.4, 0.1


def env(path):
    """dBFS envelope of a mono mix of the file"""
    x, sr = sf.read(path, dtype='float64', always_2d=True)
    x = x.mean(1)
    win, hop = int(WIN_S * sr), int(HOP_S * sr)
    n = 1 + (len(x) - win) // hop
    idx = np.arange(win)[None] + hop * np.arange(n)[:, None]
    r = np.sqrt((x[idx] ** 2).mean(1))
    return 20 * np.log10(np.maximum(r, 1e-10))


def t(e):
    """window centre times in seconds"""
    return np.arange(len(e)) * HOP_S + WIN_S / 2


def st(a):
    if len(a) == 0:
        return None
    return dict(median=round(float(np.median(a)), 1), p95=round(float(np.percentile(a, 95)), 1), max=round(float(a.max()), 1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('orig_vocal')
    ap.add_argument('inst')
    ap.add_argument('resid')
    ap.add_argument('--sections', default='')
    a = ap.parse_args()
    ev, ei, er = env(a.orig_vocal), env(a.inst), env(a.resid)
    n = min(len(ev), len(ei), len(er))
    ev, ei, er = ev[:n], ei[:n], er[:n]
    tt = t(ev)
    active = ev > -35
    quiet = ev < -60
    out = dict(orig_vocal_active_frames=int(active.sum()), orig_vocal_quiet_frames=int(quiet.sum()),
               orig_vocal_in_active=st(ev[active]), inst_in_active=st(ei[active]),
               orig_vocal_minus_inst_in_active=st((ev - ei)[active]),
               resid_in_active=st(er[active]), resid_in_quiet=st(er[quiet]),
               resid_minus_inst_in_active=st((er - ei)[active]), resid_minus_inst_in_quiet=st((er - ei)[quiet]),
               resid_whole=st(er))
    secs = [tuple(float(v) for v in s.split('-')) for s in a.sections.split(',') if s]
    if secs:
        out['per_section'] = {f"{lo:.1f}-{hi:.1f}": dict(orig_vocal=st(ev[(tt >= lo) & (tt < hi)]), resid=st(er[(tt >= lo) & (tt < hi)]),
                                                         inst=st(ei[(tt >= lo) & (tt < hi)])) for lo, hi in secs}
    print(json.dumps(out, indent=1))


if __name__ == '__main__':
    main()
