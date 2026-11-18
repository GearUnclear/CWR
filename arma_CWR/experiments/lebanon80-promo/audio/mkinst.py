"""Sums the drums, bass and other stems of separate.py into the instrumental (32-bit float, the stems' rate).

    python mkinst.py <stem dir> <out.wav> [--prefix p_]
"""
import argparse

import numpy as np
import soundfile as sf

ap = argparse.ArgumentParser()
ap.add_argument('stems')
ap.add_argument('out')
ap.add_argument('--prefix', default='')
a = ap.parse_args()
parts = [sf.read(f"{a.stems}/{a.prefix}{n}.wav", dtype='float64') for n in ('drums', 'bass', 'other')]
sr = parts[0][1]
assert all(p[1] == sr for p in parts), 'stems differ in rate'
s = sum(p[0] for p in parts)
peak = float(np.abs(s).max())
print('samples', s.shape, 'rate', sr, 'peak', peak, 'peak dBFS', 20 * np.log10(peak))
sf.write(a.out, s.astype(np.float32), sr, subtype='FLOAT')
