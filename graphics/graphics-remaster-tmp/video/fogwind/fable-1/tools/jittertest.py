"""fable-1: prototype of the unit test's two metrics on a C++-like mirror of the froxel jitter.

A grid of cells (nx x ny) over many frames. For each scheme the per-cell offset o_n(x, y)
(in cells, -0.5..0.5) of one axis is fed through the history EMA (share 0.12, read at the
cell centre: r_n = 0.88 r_{n-1} + 0.12 o_n). Two metrics on the residual field r_n:

  PUMP   = std over frames of the mean of r_n over a 5x5 block (what the apply's lookup and
           the eye average): the frame-wide scheme swings it, a per-cell scheme does not.
  TRAVEL = mean over frames of the normalised cross-correlation between r_{n+1} and r_n
           shifted by (dx, dy) cells, maximised over nonzero lags |dx|,|dy| <= 3:
           a pattern that slides one cell per frame scores high; a standing or white one low.
  BOIL   = rms of r_n per cell (the in-place change the history leaves) for reference.

Schemes: framewide (pre-d91), ign (d91a08e0), white (Act25-class hash rotation), fixed
(q2b-class: no temporal jitter). Plain python, no numpy dependency needed but used if present.
"""
import math
import sys

try:
    import numpy as np
except ImportError:  # pragma: no cover
    np = None

SHARE = 0.12
GOLD = 0.6180339887
PLASTIC_X = 0.7548776662
PLASTIC_Y = 0.5698402910


def fract(x):
    return x - math.floor(x)


def ign(x, y):
    return fract(52.9829189 * fract(x * 0.06711056 + y * 0.00583715))


def hash_u(x):
    x &= 0xFFFFFFFF
    x ^= x >> 16
    x = (x * 0x7FEB352D) & 0xFFFFFFFF
    x ^= x >> 15
    x = (x * 0x846CA68B) & 0xFFFFFFFF
    x ^= x >> 16
    return x


def hash3(x, y, z):
    return (hash_u(x + hash_u(y + hash_u(z))) >> 8) / 16777216.0


def offset(scheme, x, y, s, frame):
    """One axis (depth) offset in the cell, -0.5..0.5, of cell (x, y), slice s, at frame."""
    jz = fract(GOLD * frame)
    if scheme == 'framewide':
        return jz - 0.5
    if scheme == 'ign':
        return fract(jz + ign(x + 0.5, y + 0.5) + 0.618034 * s) - 0.5
    if scheme == 'white':
        return fract(jz + hash3(x, y, s + 7)) - 0.5
    if scheme == 'fixed':
        return 0.0  # two fixed taps at 1/4 and 3/4 of the slice: nothing moves between frames
    raise ValueError(scheme)


def run(scheme, nx=48, ny=48, s=40, frames=400, warm=100):
    r = [[0.0] * nx for _ in range(ny)]
    pump_vals = []
    travel_best = {}
    boil2 = 0.0
    n_boil = 0
    prev = None
    lags = [(dx, dy) for dx in range(-3, 4) for dy in range(-3, 4) if (dx, dy) != (0, 0)]
    for f in range(frames):
        for y in range(ny):
            row = r[y]
            for x in range(nx):
                o = offset(scheme, x, y, s, f)
                row[x] += SHARE * (o - row[x])
        if f < warm:
            prev = [row[:] for row in r]
            continue
        # pump: a 5x5 block in the middle
        blk = 0.0
        for y in range(20, 25):
            for x in range(20, 25):
                blk += r[y][x]
        pump_vals.append(blk / 25.0)
        # boil
        for y in range(ny):
            for x in range(nx):
                boil2 += r[y][x] ** 2
                n_boil += 1
        # travel: correlation of r_now(x, y) with r_prev(x + dx, y + dy) over the interior
        cur = r
        def corr(dx, dy):
            sa = sb = saa = sbb = sab = 0.0
            n = 0
            for y in range(4, ny - 4):
                for x in range(4, nx - 4):
                    a = cur[y][x]
                    b = prev[y + dy][x + dx]
                    sa += a; sb += b; saa += a * a; sbb += b * b; sab += a * b; n += 1
            va = saa / n - (sa / n) ** 2
            vb = sbb / n - (sb / n) ** 2
            if va < 1e-18 or vb < 1e-18:
                return 0.0
            return (sab / n - (sa / n) * (sb / n)) / math.sqrt(va * vb)
        for (dx, dy) in lags:
            travel_best[(dx, dy)] = travel_best.get((dx, dy), 0.0) + corr(dx, dy)
        prev = [row[:] for row in r]
    nf = frames - warm
    m = sum(pump_vals) / nf
    pump = math.sqrt(sum((v - m) ** 2 for v in pump_vals) / nf)
    boil = math.sqrt(boil2 / max(n_boil, 1))
    best_lag = max(travel_best, key=lambda k: travel_best[k])
    travel = travel_best[best_lag] / nf
    return pump, travel, best_lag, boil


if __name__ == '__main__':
    schemes = sys.argv[1:] or ['framewide', 'ign', 'white', 'fixed']
    print('scheme      pump(5x5 sd)  travel(max rho at lag)  boil(rms per cell)')
    for sc in schemes:
        pump, travel, lag, boil = run(sc)
        print('%-10s  %.4f        %.3f at %s        %.4f' % (sc, pump, travel, lag, boil))
