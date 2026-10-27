#!/usr/bin/env python
"""Fog motion check (fogwind swarm, agent 1): FLICKER, MOTION (wind) and GRAIN of the fog in a clip.

usage:
  python fogmotion.py <clip.mkv | frames_dir> [more clips ...] [--from N] [--to M]
                      [--regions bay,valley,farhill,haze] [--json out.json] [--surf out_prefix]

Run with plain `python` (numpy/scipy in the user site; never `python -I`).

Per clip, frames decoded full-res grey (Y). For each region (fractions of the 1920x1080 frame):

  camera   the frame-to-frame image shift (phase correlation of the region; px/frame at 1080p).
           A static camera gives about 0.

  D_t      the camera-compensated residual: I(t+1) - warp(I(t), camera shift). The fog's own change.
           Everything the camera explains is removed; parallax and animated objects remain
           (the same in vanilla, so always compare against vanilla).

  FLICKER  `band` = bandmetric.py's number (15 px blocks of 480x270, rows 8-13, cols 5-31: d2 rms).
           Per region `lowd2` = rms of the temporal 2nd difference of the region's 60 px block means.

  MOTION   the residual's mid band (DoG sigma 2..16 px: periods about 12..100 px, the froxel cell
           scale) for consecutive frames is cross-correlated after warping D_t by the next camera
           shift, so a pattern that is fixed in the scene peaks at (0,0) and a travelling one at its
           velocity relative to the scene.
             mid      rms of the mid-band residual (code values, 0-255)
             rho      the normalised correlation at the best lag (-1..1). Travelling pattern: high
                      (0.3..0.9). In-place flicker/noise: <= 0 at lag 0, small elsewhere.
             rho0     the correlation at lag (0,0) (in-place: negative for white-in-time noise)
             vel      the best lag (dx, dy) px/frame relative to the scene (y down); |vel| > 0.5
                      with rho high = drift / wind.
             rho_c / vslow  the best correlation within |lag| <= 2 px and its sub-pixel lag: a SLOW creep
                      (< 1 px/frame) or a pattern evolving in place shows rho_c > 0 (vanilla and white
                      noise sit at -0.3..-0.6 because consecutive differences of noise anti-correlate).
             wind     mid * max(rho_off, 0), rho_off = best correlation with |lag| >= 0.75 px:
                      the amplitude of the travelling part (code values per frame).
  BOIL     with --static only (camera held; also skips the camera estimate, which the wind biases): `boil` = rms of the mid band around its time
           mean (codes; any in-place change of the fog at the cell scale), `slow` = the same after a 6-frame
           boxcar (slow swimming / boiling the eye follows). The jitter-frozen floor is the reference.
  GRAIN    the residual's fine band (I - gauss 1.5 px): `fine` rms in code values, and `fine0`, its
           lag-0 correlation between consecutive frames (negative = fizz that comes and goes).

  The numbers are only meaningful against vanilla (and the original/per-cell clips) on the same
  frames and regions: encode noise, parallax and animated soldiers set a floor.

  --surf PREFIX writes PREFIX_<region>.png: the mean mid-band correlation surface (+-16 px, x8
  enlarged, red = positive, blue = negative, the scene's zero at the centre cross).
"""
import argparse, glob, json, os, shutil, subprocess, sys
import numpy as np
from scipy import ndimage

# ffmpeg: the FFMPEG environment variable, else the one on PATH.
FF = os.environ.get("FFMPEG") or shutil.which("ffmpeg") or "ffmpeg"
FW, FH = 1920, 1080
# y0, y1, x0, x1 as fractions of the frame (valley_mist at the frame 328 pose; soldiers excluded)
REGIONS = {
    "bay": (0.56, 0.70, 0.72, 0.97),      # water and the far shore under the sun: haze over water
    "valley": (0.54, 0.64, 0.58, 0.68),   # the valley mist between the soldiers and the bay
    "farhill": (0.45, 0.53, 0.25, 0.55),  # the far hills' haze
    "haze": (0.47, 0.55, 0.62, 0.97),     # horizon haze, far shore and the bright air under the sun
    "slope": (0.56, 0.68, 0.30, 0.48),    # the near valley slope left of the soldier
}
LAG = 16


def decode(src, a, b, box):
    y0, y1, x0, x1 = box
    if os.path.isdir(src):
        from PIL import Image
        files = sorted(glob.glob(os.path.join(src, "*.png")) + glob.glob(os.path.join(src, "*.bmp")))[a:b]
        return np.stack([np.asarray(Image.open(f).convert("L"), np.float32)[y0:y1, x0:x1] for f in files])
    vf = f"crop={x1 - x0}:{y1 - y0}:{x0}:{y0},format=gray"
    if a or b:
        vf = f"trim=start_frame={a}" + (f":end_frame={b}" if b else "") + ",setpts=PTS-STARTPTS," + vf
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-vsync", "0",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, y1 - y0, x1 - x0).astype(np.float32)


def hann(h, w):
    return np.outer(np.hanning(h), np.hanning(w)).astype(np.float32)


def peak_subpix(r):
    h, w = r.shape
    iy, ix = np.unravel_index(np.argmax(r), r.shape)
    def para(m, c, p):
        d = m - 2 * c + p
        return 0.0 if abs(d) < 1e-12 else 0.5 * (m - p) / d
    dy = para(r[(iy - 1) % h, ix], r[iy, ix], r[(iy + 1) % h, ix])
    dx = para(r[iy, (ix - 1) % w], r[iy, ix], r[iy, (ix + 1) % w])
    py = iy + dy
    px = ix + dx
    if py > h / 2: py -= h
    if px > w / 2: px -= w
    return py, px


def phase_shift(a, b, win):
    """shift d (dy, dx) such that b(x) ~ a(x - d)."""
    fa = np.fft.rfft2((a - a.mean()) * win)
    fb = np.fft.rfft2((b - b.mean()) * win)
    r = fb * np.conj(fa)
    r /= np.abs(r) + 1e-9
    c = np.fft.irfft2(r, s=a.shape)
    d = np.array(peak_subpix(c))
    # refine with two Lucas-Kanade steps on the smoothed images (parabolic peaks are biased)
    sa = ndimage.gaussian_filter(a, 1.0)
    sb = ndimage.gaussian_filter(b, 1.0)
    m = 24
    for _ in range(3):
        wa = warp(sa, tuple(d))[m:-m, m:-m]
        bb = sb[m:-m, m:-m]
        gy, gx = np.gradient(wa)
        it = bb - wa
        A = np.array([[(gy * gy).sum(), (gy * gx).sum()], [(gy * gx).sum(), (gx * gx).sum()]])
        rhs = -np.array([(gy * it).sum(), (gx * it).sum()])
        try:
            step = np.linalg.solve(A, rhs)
        except np.linalg.LinAlgError:
            break
        if np.abs(step).max() > 1.0:
            break
        d = d + step
    return float(d[0]), float(d[1])


def warp(img, d):
    return ndimage.shift(img, d, order=3, mode="nearest", prefilter=True)


def xcorr_local(a, b, lag=LAG):
    """normalised cross-correlation c[dy, dx] = sum a(x) b(x + d) / sqrt(sum a^2 sum b^2), |d| <= lag."""
    h, w = a.shape
    fa = np.fft.rfft2(a, s=(h + lag, w + lag))
    fb = np.fft.rfft2(b, s=(h + lag, w + lag))
    c = np.fft.irfft2(np.conj(fa) * fb, s=(h + lag, w + lag))
    c = np.roll(np.roll(c, lag, 0), lag, 1)[:2 * lag + 1, :2 * lag + 1]
    n = np.sqrt((a * a).sum() * (b * b).sum()) + 1e-9
    return c / n


def bandmetric_band(src, a, b):
    """bandmetric.py's band number (480x270 grey, 15 px blocks, rows 8-13, cols 5-31)."""
    W, H, B = 480, 270, 15
    if os.path.isdir(src):
        return None
    vf = f"scale={W}:{H}:flags=area,format=gray"
    if a or b:
        vf = f"trim=start_frame={a}" + (f":end_frame={b}" if b else "") + ",setpts=PTS-STARTPTS," + vf
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-vsync", "0",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    blk = x[:, :H // B * B, :W // B * B].reshape(len(x), H // B, B, W // B, B).mean(axis=(2, 4))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    return round(float(rms[8:14, 5:32].mean()), 3), round(float(rms[0:6].mean()), 3)


def analyse_region(x, name, static=False):
    T, h, w = x.shape
    win = hann(h, w)
    m = 20  # margin cut after warps
    if static:
        shifts = [(0.0, 0.0)] * (T - 1)
    else:
        shifts = [phase_shift(x[t], x[t + 1], win) for t in range(T - 1)]
    D = []
    for t in range(T - 1):
        d = x[t + 1] - warp(x[t], shifts[t])
        D.append(d)
    D = np.stack(D)
    def mid(d):
        return ndimage.gaussian_filter(d, 2.0) - ndimage.gaussian_filter(d, 16.0)
    def fine(d):
        return d - ndimage.gaussian_filter(d, 1.5)
    inner = (slice(m, h - m), slice(m, w - m))
    Dm = np.stack([mid(d) for d in D])
    Df = np.stack([fine(d) for d in D])
    surf = np.zeros((2 * LAG + 1, 2 * LAG + 1))
    f0 = []
    for t in range(T - 2):
        a = warp(Dm[t], shifts[t + 1])[inner]
        b = Dm[t + 1][inner]
        surf += xcorr_local(a, b)
        af = warp(Df[t], shifts[t + 1])[inner]
        bf = Df[t + 1][inner]
        f0.append((af * bf).sum() / (np.sqrt((af * af).sum() * (bf * bf).sum()) + 1e-9))
    surf /= max(T - 2, 1)
    iy, ix = np.unravel_index(np.argmax(surf), surf.shape)
    yy, xx = np.mgrid[-LAG:LAG + 1, -LAG:LAG + 1]
    off = np.hypot(yy, xx) >= 0.75
    so = np.where(off, surf, -9)
    oy, ox = np.unravel_index(np.argmax(so), so.shape)
    rho_off = float(surf[oy, ox])
    # slow drift: the sub-pixel peak inside |lag| <= 2 px (a pattern creeping < 1 px/frame)
    c = surf[LAG - 2:LAG + 3, LAG - 2:LAG + 3]
    cy, cx = np.unravel_index(np.argmax(c), c.shape)
    def _para(a, b, d):
        q = a - 2 * b + d
        return 0.0 if abs(q) < 1e-12 or not (0 < cy < 4 and 0 < cx < 4) else 0.5 * (a - d) / q
    sy = cy - 2 + (_para(c[cy - 1, cx], c[cy, cx], c[cy + 1, cx]) if 0 < cy < 4 else 0.0)
    sx = cx - 2 + (_para(c[cy, cx - 1], c[cy, cx], c[cy, cx + 1]) if 0 < cx < 4 else 0.0)
    rho_c = float(c.max())
    midrms = float(np.sqrt((Dm[:, m:h - m, m:w - m] ** 2).mean()))
    finerms = float(np.sqrt((Df[:, m:h - m, m:w - m] ** 2).mean()))
    # low-frequency flicker of the region: 60 px block means, d2 rms
    B = 60
    gy, gx = h // B, w // B
    if gy and gx:
        blk = x[:, :gy * B, :gx * B].reshape(T, gy, B, gx, B).mean(axis=(2, 4))
        d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
        lowd2 = float(np.sqrt((d2 ** 2).mean()))
    else:
        lowd2 = float("nan")
    sh = np.array(shifts)
    # static camera only: BOIL = in-place fluctuation of the mid band around its time mean (rms, codes);
    # SLOW = the same after a 6-frame boxcar in time (swimming / slow boiling the eye follows)
    boil = slow = float("nan")
    if static:
        M = np.stack([mid(f) for f in x])[:, m:h - m, m:w - m]
        M = M - M.mean(axis=0, keepdims=True)
        boil = float(np.sqrt((M ** 2).mean()))
        if T > 12:
            k6 = ndimage.uniform_filter1d(M, 6, axis=0)[3:-3]
            slow = float(np.sqrt((k6 ** 2).mean()))
    res = dict(boil=round(boil, 3), slow=round(slow, 3), mean=round(float(x.mean()), 2), camera=[round(float(sh[:, 1].mean()), 2), round(float(sh[:, 0].mean()), 2)],
               lowd2=round(lowd2, 3), mid=round(midrms, 3), rho=round(float(surf[iy, ix]), 3),
               rho0=round(float(surf[LAG, LAG]), 3), vel=[int(ix - LAG), int(iy - LAG)],
               rho_off=round(rho_off, 3), vel_off=[int(ox - LAG), int(oy - LAG)],
               wind=round(midrms * max(rho_off, 0.0), 3), rho_c=round(rho_c, 3), vslow=[round(float(sx), 2), round(float(sy), 2)],
               fine=round(finerms, 3), fine0=round(float(np.mean(f0)), 3))
    return res, surf


def surf_png(surf, path):
    from PIL import Image
    s = surf / (np.abs(surf).max() + 1e-9)
    rgb = np.zeros(s.shape + (3,), np.uint8)
    rgb[..., 0] = np.clip(255 * s, 0, 255)
    rgb[..., 2] = np.clip(-255 * s, 0, 255)
    rgb[..., 1] = np.clip(60 * np.abs(s), 0, 255)
    img = np.kron(rgb, np.ones((8, 8, 1), np.uint8))
    c = LAG * 8 + 4
    img[c, :, :] = np.maximum(img[c, :, :], 90)
    img[:, c, :] = np.maximum(img[:, c, :], 90)
    Image.fromarray(img).save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--from", dest="a", type=int, default=0)
    ap.add_argument("--to", dest="b", type=int, default=0)
    ap.add_argument("--regions", default="bay,valley,farhill,haze,slope")
    ap.add_argument("--json", default="")
    ap.add_argument("--surf", default="")
    ap.add_argument("--noband", action="store_true")
    ap.add_argument("--static", action="store_true", help="camera held: no camera estimate (the wind biases it), adds boil/slow")
    o = ap.parse_args()
    out = []
    for src in o.clips:
        rec = dict(src=src, frm=o.a, to=o.b)
        if not o.noband:
            bb = bandmetric_band(src, o.a, o.b)
            if bb:
                rec["band"], rec["sky"] = bb
        regs = {}
        for name in o.regions.split(","):
            y0, y1, x0, x1 = REGIONS[name]
            box = (int(y0 * FH), int(y1 * FH), int(x0 * FW), int(x1 * FW))
            x = decode(src, o.a, o.b, box)
            r, surf = analyse_region(x, name, o.static)
            regs[name] = r
            if o.surf:
                tag = (os.path.basename(os.path.dirname(os.path.abspath(src))) + '_' + os.path.splitext(os.path.basename(src))[0]).replace(' ', '_')
                surf_png(surf, f"{o.surf}_{tag}_{name}.png")
        rec["regions"] = regs
        rec["frames"] = int(len(x))
        out.append(rec)
        print(f"== {src}  frames {rec['frames']}  band {rec.get('band')}  sky {rec.get('sky')}")
        print(f"   {'region':8s} {'mean':>6s} {'camera':>12s} {'lowd2':>6s} {'mid':>6s} {'rho':>6s} {'rho0':>6s} {'vel':>8s} "
              f"{'rho_off':>7s} {'vel_off':>8s} {'WIND':>6s} {'fine':>6s} {'fine0':>6s} {'boil':>6s} {'slow':>6s} {'rho_c':>6s} {'vslow':>13s}")
        for k, r in regs.items():
            print(f"   {k:8s} {r['mean']:6.1f} {str(r['camera']):>12s} {r['lowd2']:6.3f} {r['mid']:6.3f} {r['rho']:6.3f} "
                  f"{r['rho0']:6.3f} {str(r['vel']):>8s} {r['rho_off']:7.3f} {str(r['vel_off']):>8s} "
                  f"{r['wind']:6.3f} {r['fine']:6.3f} {r['fine0']:6.3f} {r['boil']:6.3f} {r['slow']:6.3f} {r['rho_c']:6.3f} {(str(r['vslow']) if r['rho_c'] > 0 and max(abs(v) for v in r['vslow']) < 1.99 else '-'):>13s}")
        sys.stdout.flush()
    if o.json:
        with open(o.json, "w") as f:
            json.dump(out, f, indent=1)


if __name__ == "__main__":
    main()
