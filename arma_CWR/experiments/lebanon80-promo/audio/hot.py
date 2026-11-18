"""Lists the stretches where a residual stem is above a level: where a voice may still be heard.

    python hot.py <original vocal stem.wav> <residual.wav> <threshold dBFS>

The residual is what is left of the voice in the instrumental (for example the vocal stem of a second
separation run on the instrumental). Prints each run of 0.4 s windows over the threshold with the
residual's peak and the original vocal stem's peak there (envstats.py's envelope).
"""
import sys

from envstats import env, t

ev = env(sys.argv[1])
er = env(sys.argv[2])
thr = float(sys.argv[3])
tt = t(er)
hot = er > thr
runs = []
s = None
for i, h in enumerate(hot):
    if h and s is None:
        s = i
    if (not h or i == len(hot) - 1) and s is not None:
        runs.append((tt[s], tt[i], er[s:i + 1].max(), ev[s:i + 1].max()))
        s = None
for a, b, m, v in runs:
    print(f"{a:7.2f}-{b:7.2f} resid_max={m:6.1f} origvocal_max={v:6.1f}")
print('runs', len(runs))
