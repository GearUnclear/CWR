#!/usr/bin/env python
"""fable-3: why does a sub-cell jitter swing the far haze so much?

A 1-D model of the shipped froxel injection along one ray, with the real slice table
(FogDepthK: first slice 0.9 m at 64 slices, exponential to zFar) and the real mist profile
(fogMediumAt: shape = 1 below the layer's top, exp(-above / fall) over it, fall 18 m; the
clear haze exp(-y / 1200 m)).  For each slice: the slice MEAN of sigma (what the cell should
hold) against the POINT sample at depth offset u in [0, 1) (the shipped estimator), the
y-offset of +-0.5 cell (the xy jitter, 12 px cells at fov 0.7), and fixed estimators (1 tap at
the slice mid, 2/3-point Gauss-Legendre, 4 stratified taps, the analytic integral).

Run with plain python.  No inputs: the scene numbers are the valley_mist staging (eye 1.75 m
over a ridge, pitch -6, vertical fov 2 atan(0.525), 64 slices, zFar = view distance 4000 m).
"""
import math
import numpy as np

ZFAR = 4000.0
SLICES = 64
FIRST = 0.9
CELL_PX = 12
TAN_HALF_V = 0.525  # fov 0.7 * topFOV 0.75
DEG_PER_PX = math.degrees(math.atan(TAN_HALF_V / 540.0))  # at the centre
CELL_DEG = CELL_PX * DEG_PER_PX

FALL = 18.0
SIGMA_MIST = 2.0e-3
HAZE0 = 6.0e-4
HAZE_H = 1200.0


def depth_k(zfar, slices, first):
    lo, hi = 1e-3, 30.0
    t = 1.0 / slices
    for _ in range(80):
        mid = 0.5 * (lo + hi)
        if zfar * (math.exp(mid * t) - 1.0) / (math.exp(mid) - 1.0) > first:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


K = depth_k(ZFAR, SLICES, FIRST)


def slice_depth(t):
    return ZFAR * (math.exp(K * t) - 1.0) / (math.exp(K) - 1.0)


def sigma_mist(y, top):
    above = y - top
    return SIGMA_MIST * (1.0 if above <= 0.0 else math.exp(-above / FALL))


def sigma_haze(y):
    return HAZE0 * math.exp(-max(y, -50.0) / HAZE_H)


def ray_y(eye_y, elev_deg, t):
    return eye_y + math.sin(math.radians(elev_deg)) * t


def mean_sigma(eye_y, elev, top, t0, t1, n=4000):
    ts = np.linspace(t0, t1, n)
    ys = eye_y + math.sin(math.radians(elev)) * ts
    sm = np.where(ys <= top, SIGMA_MIST, SIGMA_MIST * np.exp(-np.maximum(ys - top, 0.0) / FALL))
    sh = HAZE0 * np.exp(-np.maximum(ys, -50.0) / HAZE_H)
    return float(sm.mean()), float(sh.mean())


def point_sigma(eye_y, elev, top, t):
    y = ray_y(eye_y, elev, t)
    return sigma_mist(y, top), sigma_haze(y)


def ground_hit(eye_y, elev, ground_y):
    s = math.sin(math.radians(elev))
    if s >= 0:
        return ZFAR * 4
    return (ground_y - eye_y) / s


def column(eye_y, elev, top, ground_y, estimator):
    """Front-to-back S (air = 1, lit = 1) and T along the ray up to the ground, with per-slice
    sigma from `estimator(t0, t1) -> sigma` (mist + haze)."""
    S, T = 0.0, 1.0
    t_end = min(ground_hit(eye_y, elev, ground_y), ZFAR)
    for s in range(SLICES):
        z0, z1 = slice_depth(s / SLICES), slice_depth((s + 1) / SLICES)
        if z0 >= t_end:
            break
        dt = min(z1, t_end) - z0
        sig = estimator(z0, z1)
        x = sig * dt
        tr = math.exp(-x)
        through = dt * (1 - 0.5 * x) if x < 1e-3 else (1 - tr) / sig
        S += T * sig * through
        T *= tr
    return S, T


def main():
    print("slice table: zFar %.0f, %d slices, k = %.4f" % (ZFAR, SLICES, K))
    print("cell: %d px = %.3f deg at the centre (%.4f deg/px)" % (CELL_PX, CELL_DEG, DEG_PER_PX))
    print("\n  s   z0 (m)   z1 (m)   dz (m)   dz*sin(5deg) (m)  dz*sin(10deg) (m)")
    for s in range(0, SLICES, 4):
        z0, z1 = slice_depth(s / SLICES), slice_depth((s + 1) / SLICES)
        print("%3d %8.1f %8.1f %8.1f %12.1f %16.1f" % (s, z0, z1, z1 - z0,
                                                      (z1 - z0) * math.sin(math.radians(5)),
                                                      (z1 - z0) * math.sin(math.radians(10))))
    # The valley_mist geometry: eye 1.75 m over a ridge, the valley floor below it, the mist
    # layer filling the valley to `top` over the floor.  Three cases bracket the shot.
    cases = [
        # name, eye over the floor (m), layer top over the floor (m), ground under the ray (m over the floor)
        ("ridge 80 m up, layer 30 m, floor", 80.0, 30.0, 0.0),
        ("ridge 120 m up, layer 35 m, floor", 120.0, 35.0, 0.0),
        ("ridge 150 m up, layer 40 m, bay at 0", 150.0, 40.0, 0.0),
    ]
    elevs = [-3.0, -5.0, -8.0, -11.0, -14.0]
    print("\nPER-SLICE SENSITIVITY of the point sample (the shipped estimator), slices whose span")
    print("crosses the mist layer's top. swing = (max - min over the depth offset u in [0,1)) / slice mean,")
    print("ySwing = the same for the y offset +-0.5 cell at the slice mid depth (the xy jitter),")
    print("mid = 1 fixed tap at the slice mid (error vs mean), gl2 = 2-point Gauss-Legendre, st4 = 4 stratified")
    print("fixed taps at (i+0.5)/4, an = the analytic integral (exact).  Errors in % of the slice mean (mist+haze).")
    for name, eye, top, ground in cases:
        print("\n== %s ==" % name)
        for elev in elevs:
            hit = ground_hit(eye, elev, ground)
            print(" elev %5.1f deg: ray meets the ground at %.0f m" % (elev, hit))
            # which slices cross the top
            crossed = []
            for s in range(SLICES):
                z0, z1 = slice_depth(s / SLICES), slice_depth((s + 1) / SLICES)
                if z0 >= hit:
                    break
                y0, y1 = ray_y(eye, elev, z0), ray_y(eye, elev, z1)
                within = (y1 - top) < 3 * FALL and (y0 - top) > -1e9  # the mist matters in this slice
                if not within:
                    continue
                mm, mh = mean_sigma(eye, elev, top, z0, min(z1, hit))
                mean = mm + mh
                us = np.linspace(0, 0.999, 200)
                pts = [sum(point_sigma(eye, elev, top, z0 + u * (z1 - z0))) for u in us]
                swing = (max(pts) - min(pts)) / mean
                rms = float(np.sqrt(np.mean((np.array(pts) - mean) ** 2))) / mean
                zm = 0.5 * (z0 + z1)
                # y jitter: +-0.5 cell = +-CELL_DEG/2 of elevation
                ys = [sum(point_sigma(eye, elev + dd, top, zm)) for dd in (-CELL_DEG / 2, 0.0, CELL_DEG / 2)]
                yswing = (max(ys) - min(ys)) / mean
                mid = (sum(point_sigma(eye, elev, top, zm)) - mean) / mean
                g = 0.5 / math.sqrt(3)
                gl2 = (0.5 * sum(sum(point_sigma(eye, elev, top, z0 + (0.5 + sg * g) * (z1 - z0))) for sg in (-1, 1)) - mean) / mean
                st4 = (0.25 * sum(sum(point_sigma(eye, elev, top, z0 + (i + 0.5) / 4 * (z1 - z0))) for i in range(4)) - mean) / mean
                crossed.append((s, z0, z1, y0 - top, y1 - top, mean, swing, rms, yswing, mid, gl2, st4))
            print("   s     z0     z1   above0  above1   mean/m   swing   rms  ySwing    mid    gl2    st4")
            for c in crossed:
                print("  %2d %6.0f %6.0f %7.1f %7.1f %8.2e %6.0f%% %5.0f%% %6.0f%% %6.0f%% %6.0f%% %6.0f%%" % (
                    c[0], c[1], c[2], c[3], c[4], c[5], 100 * c[6], 100 * c[7], 100 * c[8], 100 * c[9], 100 * c[10], 100 * c[11]))
            # whole-column effect: the pixel's S and 1-T for the jittered point column vs exact
            def est_exact(z0, z1):
                mm, mh = mean_sigma(eye, elev, top, z0, min(z1, hit), n=600)
                return mm + mh
            S_ex, T_ex = column(eye, elev, top, ground, est_exact)
            Ss, Ts = [], []
            for u in np.linspace(0, 0.999, 64):
                est = lambda z0, z1, u=u: sum(point_sigma(eye, elev, top, min(z0 + u * (z1 - z0), hit - 1e-3)))
                S, T = column(eye, elev, top, ground, est)
                Ss.append(S)
                Ts.append(T)
            print("   column: exact S %.3f, 1-T %.3f | one-offset-for-all-slices (frame-wide): S %.3f..%.3f (swing %.0f%%), 1-T %.3f..%.3f" % (
                S_ex, 1 - T_ex, min(Ss), max(Ss), 100 * (max(Ss) - min(Ss)) / max(S_ex, 1e-9), 1 - max(Ts), 1 - min(Ts)))
            # per-slice independent offsets (per-cell): rms over random draws
            rng = np.random.default_rng(1)
            Sr = []
            for _ in range(64):
                us = iter(rng.random(SLICES))
                est = lambda z0, z1, us=us: sum(point_sigma(eye, elev, top, min(z0 + next(us) * (z1 - z0), hit - 1e-3)))
                S, T = column(eye, elev, top, ground, est)
                Sr.append(S)
            print("   column: independent offsets per slice: S rms %.1f%% of exact" % (100 * float(np.std(Sr)) / max(S_ex, 1e-9)))


if __name__ == "__main__":
    main()
