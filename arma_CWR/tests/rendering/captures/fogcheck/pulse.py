"""Whole-region brightness pulse check: python pulse.py clip[@first] ... [--n 105]
Per region: the region-mean series, detrended (quadratic), its rms and the strongest period (frames)."""
import sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm
args = sys.argv[1:]
n = 105
if "--n" in args:
    i = args.index("--n"); n = int(args[i + 1]); del args[i:i + 2]
REG = {"bay": fm.REGIONS["bay"], "valley": fm.REGIONS["valley"], "farhill": fm.REGIONS["farhill"], "haze": fm.REGIONS["haze"]}
for spec in args:
    src, first = (spec.rsplit("@", 1) if "@" in spec else (spec, "15"))
    first = int(first)
    out = []
    for name, (y0, y1, x0, x1) in REG.items():
        x = fm.decode(src, first, first + n, (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920)))
        s = x.mean(axis=(1, 2))
        t = np.arange(len(s))
        s = s - np.polyval(np.polyfit(t, s, 2), t)
        P = np.abs(np.fft.rfft(s * np.hanning(len(s)))) ** 2
        f = np.fft.rfftfreq(len(s))
        k = 2 + int(np.argmax(P[2:]))
        out.append(f"{name} rms {np.sqrt((s ** 2).mean()):.3f} peak {1 / f[k]:.1f} fr")
    print(f"{os.path.basename(src)}@{first}: " + " | ".join(out))
