"""fogflow.py  (fable-2)  a motion-first fog check, written independently of agent 1's kit.

    python fogflow.py <clip.mkv> [--from N --to M] [--static] [--label L] [--out DIR] [--amp] [--mp4] [--json F]

Pipeline (per clip):
  1. decode luma (ffmpeg, gray, full res, -vsync 0);
  2. camera: global sub-pixel shift between consecutive frames (phase correlation of the whole half-res
     frame); --static forces 0;
  3. per region (agent 1's five boxes, reused only so regions line up by name): stabilise all frames of a
     chunk to the chunk centre with a cubic warp, then split each pixel's time series into
        hp     = temporal high-pass (minus a sigma 1.5 frame smoothing: keeps > ~5 Hz at 60 fps)
        bp     = temporal band ~1.5..5 Hz (sigma 1.5 minus sigma 6)
     and spatially into
        LOW    = hp smoothed over 12 px (the sheet: the original frame-wide pump lives here)
        MID    = hp band 3..24 px (cell-scale blotches: wind, boil)
        FINE   = hp below 3 px (encode grain / fizz)
        SLOW   = bp band 3..24 px (slow boil / drift)
     Fog that changes without the camera explaining it is what survives stabilisation; vanilla calibrates the
     floor the warp itself leaves (object edges, encode).
  4. TRAVEL: the MID residual's normalised cross-correlation surface between frames t and t+L (L = 1, 2) is
     averaged over all pairs (a consistent travel adds up, random peaks cancel). Reported:
        v1 = (vx, vy)  peak lag of the L=1 surface, px/frame vs the SCENE (y down, x right), sub-pixel
        p1             the averaged correlation at that peak;  z1 = the value at lag 0
        v2, p2         the same at L=2 (a real travel has v2 ~= 2 v1)
        TRAVEL = p1 - max(z1, 0): how much better the pattern matches when shifted than when not
        WIND   = VIS * TRAVEL if |v1| >= 0.75 px else 0   (code values that travel coherently)
     Reading: p1 ~ 0 = fizz that changes every frame; z1 < 0 (and p1 at lag 0) = alternating sheet (the
     original pump); p1 high at |v1| > 0 = travelling structure (wind, or a screen-locked pattern under a pan,
     which travels at -camera vs the scene); p1 high at v1 = 0 = a pattern that persists in place.
  5. VIS = rms (code values) of MID, LOW, SLOW, FINE per region (mean over frames; p95 of MID too).
  --amp writes the owner's-eyes views in --out: amp_<label>.png (2x bay + far valley crop, 4 consecutive
     stabilised frames with the temporal residual amplified x8 added back) and xt_<label>.png (stabilised
     x-t strips, time down, x16). --mp4 writes amp2x_<label>.mp4: the raw 2x crop with the residual x8 added,
     at 60 fps, to watch.
"""
import argparse, json, os, subprocess, sys
import numpy as np
from scipy import ndimage as ndi
from scipy.fft import fft2, ifft2

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080
# y0, y1, x0, x1 as fractions of the frame (valley_mist, frame 328 pose; soldiers excluded). Same boxes as agent 1.
REGIONS = {
    "bay":     (0.56, 0.70, 0.72, 0.97),
    "valley":  (0.54, 0.64, 0.58, 0.68),
    "farhill": (0.45, 0.53, 0.25, 0.55),
    "haze":    (0.47, 0.55, 0.62, 0.97),
    "slope":   (0.56, 0.68, 0.30, 0.48),
}
AMP_CROP = (0.50, 0.68, 0.56, 0.97)   # bay + far valley + the shore under the sun, for the amplified view
MAXLAG = 30


def decode_gray(path, t0=0, t1=None):
    vf = "trim=start_frame=%d%s,setpts=N/FRAME_RATE/TB" % (t0, (":end_frame=%d" % t1) if t1 else "")
    cmd = [FF, "-v", "error", "-i", path, "-vf", vf, "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "gray", "-"]
    out = subprocess.run(cmd, capture_output=True, check=True).stdout
    n = len(out) // (W * H)
    return np.frombuffer(out, np.uint8)[: n * W * H].reshape(n, H, W)


def hann2(h, w):
    return np.outer(np.hanning(h), np.hanning(w)).astype(np.float32)


def subpk(cm, c0, cp):
    d = cm - 2 * c0 + cp
    return 0.0 if abs(d) < 1e-12 else float(np.clip(0.5 * (cm - cp) / d, -0.5, 0.5))


def phasecorr(a, b, win=None):
    """shift d (dx, dy) with b(x) ~= a(x - d), sub-pixel."""
    if win is None:
        win = hann2(*a.shape)
    A = fft2((a - a.mean()) * win)
    B = fft2((b - b.mean()) * win)
    R = B * np.conj(A)
    R /= np.abs(R) + 1e-9
    c = np.real(ifft2(R))
    iy, ix = np.unravel_index(np.argmax(c), c.shape)
    pk = c[iy, ix]
    hh, ww = c.shape
    dy = iy + subpk(c[(iy - 1) % hh, ix], pk, c[(iy + 1) % hh, ix])
    dx = ix + subpk(c[iy, (ix - 1) % ww], pk, c[iy, (ix + 1) % ww])
    if dy > hh / 2:
        dy -= hh
    if dx > ww / 2:
        dx -= ww
    return dx, dy


def camera_shifts(frames):
    """per-frame shift of the scene on screen (frame k-1 -> k), px, from the half-res whole frame."""
    small = frames[:, ::2, ::2].astype(np.float32)
    win = hann2(*small[0].shape)
    out = [(0.0, 0.0)]
    for k in range(len(small) - 1):
        dx, dy = phasecorr(small[k], small[k + 1], win)
        out.append((2 * dx, 2 * dy))
    return np.array(out, np.float32)


def warp(img, dx, dy):
    """move the content by (dx, dy) px (cubic)."""
    return ndi.shift(img, (dy, dx), order=3, mode="nearest")


def band(r, lo=1.5, hi=12.0):
    return ndi.gaussian_filter(r, lo) - ndi.gaussian_filter(r, hi)


def ncc_surface(a, b, win):
    """normalised cross-correlation of a and b at every lag (b(x) ~= a(x - d) peaks at +d), Hann windowed."""
    a = (a - a.mean()) * win
    b = (b - b.mean()) * win
    na = np.sqrt((a * a).sum())
    nb = np.sqrt((b * b).sum())
    c = np.real(ifft2(fft2(b) * np.conj(fft2(a)))) / (na * nb + 1e-9)
    return np.fft.fftshift(c)


def peak(c):
    """peak within +-MAXLAG of the centre of an fftshifted surface: (dx, dy, value, value at lag 0)."""
    hh, ww = c.shape
    cy, cx = hh // 2, ww // 2
    sub = c[cy - MAXLAG: cy + MAXLAG + 1, cx - MAXLAG: cx + MAXLAG + 1]
    iy, ix = np.unravel_index(np.argmax(sub), sub.shape)
    pk = sub[iy, ix]
    dy = iy - MAXLAG + (subpk(sub[iy - 1, ix], pk, sub[iy + 1, ix]) if 0 < iy < sub.shape[0] - 1 else 0.0)
    dx = ix - MAXLAG + (subpk(sub[iy, ix - 1], pk, sub[iy, ix + 1]) if 0 < ix < sub.shape[1] - 1 else 0.0)
    return float(dx), float(dy), float(pk), float(c[cy, cx])


def analyse_region(frames, S, box, chunk=32, pad=20, margin=48):
    """per-region measures; frames uint8 (n,H,W); S cumulative scene shift per frame (n,2)."""
    n = len(frames)
    y0, y1, x0, x1 = int(box[0] * H), int(box[1] * H), int(box[2] * W), int(box[3] * W)
    acc = dict(mid=[], low=[], slow=[], fine=[])
    surf = {1: None, 2: None}
    cnt = {1: 0, 2: 0}
    win = hann2(y1 - y0, x1 - x0)
    for c0 in range(0, n, chunk):
        c1 = min(n, c0 + chunk)
        p0, p1 = max(0, c0 - pad), min(n, c1 + pad)
        cref = (c0 + c1) // 2
        ys, ye = max(0, y0 - margin), min(H, y1 + margin)
        xs, xe = max(0, x0 - margin), min(W, x1 + margin)
        stack = np.empty((p1 - p0, ye - ys, xe - xs), np.float32)
        for k in range(p0, p1):
            d = S[k] - S[cref]
            stack[k - p0] = warp(frames[k, ys:ye, xs:xe].astype(np.float32), -d[0], -d[1])
        sm1 = ndi.gaussian_filter1d(stack, 1.5, axis=0, mode="nearest")
        sm6 = ndi.gaussian_filter1d(stack, 6.0, axis=0, mode="nearest")
        hp = stack - sm1
        bp = sm1 - sm6
        iy0, iy1, ix0, ix1 = y0 - ys, y1 - ys, x0 - xs, x1 - xs
        a, b = c0 - p0, c1 - p0
        mids = []
        for t in range(a, b):
            f = hp[t]
            g1 = ndi.gaussian_filter(f, 1.5)
            g12 = ndi.gaussian_filter(f, 12.0)
            mid = (g1 - g12)[iy0:iy1, ix0:ix1]
            mids.append(mid)
            acc["mid"].append(float(np.sqrt((mid ** 2).mean())))
            acc["low"].append(float(np.sqrt((g12[iy0:iy1, ix0:ix1] ** 2).mean())))
            acc["fine"].append(float(np.sqrt(((f - g1)[iy0:iy1, ix0:ix1] ** 2).mean())))
            sl = band(bp[t])[iy0:iy1, ix0:ix1]
            acc["slow"].append(float(np.sqrt((sl ** 2).mean())))
        for L in (1, 2):
            for t in range(0, len(mids) - L):
                c = ncc_surface(mids[t], mids[t + L], win)
                surf[L] = c if surf[L] is None else surf[L] + c
                cnt[L] += 1
    res = {k: float(np.mean(v)) for k, v in acc.items()}
    res["mid_p95"] = float(np.percentile(acc["mid"], 95))
    for L in (1, 2):
        if cnt[L]:
            dx, dy, pk, z = peak(surf[L] / cnt[L])
        else:
            dx = dy = pk = z = 0.0
        res["vx%d" % L], res["vy%d" % L], res["p%d" % L], res["z%d" % L] = dx, dy, pk, z
    res["travel"] = res["p1"] - max(res["z1"], 0.0)
    res["wind"] = res["mid"] * res["travel"] if np.hypot(res["vx1"], res["vy1"]) >= 0.75 else 0.0
    return res


def amp_views(frames, S, label, outdir, t_amp, amp=8.0, nf=4, mp4=False):
    """owner's-eyes views: amplified stabilised crop of nf consecutive frames, x-t strips, optional mp4."""
    from PIL import Image, ImageDraw
    y0, y1, x0, x1 = [int(v * (H if i < 2 else W)) for i, v in enumerate(AMP_CROP)]
    m = 40
    ys, ye, xs, xe = max(0, y0 - m), min(H, y1 + m), max(0, x0 - m), min(W, x1 + m)
    n = len(frames)
    rows = {"valley": (int(0.59 * H), 6), "bay": (int(0.63 * H), 6), "haze": (int(0.51 * H), 6)}
    strips = {k: [] for k in rows}
    tiles = []
    proc = None
    if mp4:
        proc = subprocess.Popen([FF, "-v", "error", "-y", "-f", "rawvideo", "-pix_fmt", "gray", "-s", "%dx%d" % (2 * (x1 - x0), 2 * (y1 - y0)),
                                 "-r", "60", "-i", "-", "-c:v", "libx264", "-crf", "18", "-pix_fmt", "yuv420p",
                                 os.path.join(outdir, "amp2x_%s.mp4" % label)], stdin=subprocess.PIPE)
    chunk, pad = 32, 8
    for c0 in range(0, n, chunk):
        c1 = min(n, c0 + chunk)
        pp0, pp1 = max(0, c0 - pad), min(n, c1 + pad)
        cref = (c0 + c1) // 2
        st = np.empty((pp1 - pp0, ye - ys, xe - xs), np.float32)
        for k in range(pp0, pp1):
            d = S[k] - S[cref]
            st[k - pp0] = warp(frames[k, ys:ye, xs:xe].astype(np.float32), -d[0], -d[1])
        hp = st - ndi.gaussian_filter1d(st, 1.5, axis=0, mode="nearest")
        for t in range(c0, c1):
            r = ndi.gaussian_filter(hp[t - pp0], 1.0)
            for k, (yy, hh) in rows.items():
                strips[k].append(r[yy - ys - hh // 2: yy - ys + hh // 2, m:m + (x1 - x0)].mean(0))
            if t_amp <= t < t_amp + nf:
                # stabilised to the chunk centre: consecutive tiles share one camera pose
                f = st[t - pp0] + amp * r
                tile = np.clip(f[m:m + (y1 - y0), m:m + (x1 - x0)], 0, 255).astype(np.uint8)
                im = Image.fromarray(tile).resize(((x1 - x0) * 2, (y1 - y0) * 2), Image.BILINEAR)
                ImageDraw.Draw(im).text((8, 6), "%s  frame %d  (temporal residual x%g, stabilised)" % (label, t, amp), fill=255)
                tiles.append(np.asarray(im))
            if proc is not None:
                d = S[t] - S[cref]
                rr = warp(r, d[0], d[1])          # residual back into the raw frame's coordinates
                f = frames[t, ys:ye, xs:xe].astype(np.float32) + amp * rr
                tile = np.clip(f[m:m + (y1 - y0), m:m + (x1 - x0)], 0, 255).astype(np.uint8)
                im = Image.fromarray(tile).resize(((x1 - x0) * 2, (y1 - y0) * 2), Image.BILINEAR)
                ImageDraw.Draw(im).text((8, 6), "%s  f%d  residual x%g" % (label, t, amp), fill=255)
                proc.stdin.write(np.asarray(im).tobytes())
    if proc is not None:
        proc.stdin.close()
        proc.wait()
    if tiles:
        Image.fromarray(np.concatenate(tiles, 0)).save(os.path.join(outdir, "amp_%s.png" % label))
    ims = []
    for k in rows:
        s = np.array(strips[k]) * 16 + 128
        ims.append(np.clip(s, 0, 255).astype(np.uint8))
        ims.append(np.full((4, s.shape[1]), 255, np.uint8))
    xt = np.concatenate(ims, 0)
    im = Image.fromarray(xt)
    ImageDraw.Draw(im).text((6, 2), "%s  x-t of the stabilised residual (valley | bay | haze rows), time down, x16" % label, fill=255)
    im.save(os.path.join(outdir, "xt_%s.png" % label))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clip")
    ap.add_argument("--from", dest="t0", type=int, default=0)
    ap.add_argument("--to", dest="t1", type=int, default=None)
    ap.add_argument("--static", action="store_true")
    ap.add_argument("--label", default=None)
    ap.add_argument("--out", default=None)
    ap.add_argument("--amp", action="store_true")
    ap.add_argument("--mp4", action="store_true")
    ap.add_argument("--amp-at", type=int, default=None, help="first frame (index within the decoded range) of the amplified sheet")
    ap.add_argument("--json", default=None)
    ap.add_argument("--regions", default="bay,valley,farhill,haze,slope")
    a = ap.parse_args()
    label = a.label or os.path.splitext(os.path.basename(a.clip))[0]
    frames = decode_gray(a.clip, a.t0, a.t1)
    n = len(frames)
    sh = np.zeros((n, 2), np.float32) if a.static else camera_shifts(frames)
    S = np.cumsum(sh, 0)
    cam = np.hypot(sh[1:, 0], sh[1:, 1])
    print("%s: %d frames (clip %d..%d), camera |shift| mean %.2f px/frame (x %+.2f, y %+.2f)"
          % (label, n, a.t0, a.t0 + n, cam.mean(), sh[1:, 0].mean(), sh[1:, 1].mean()))
    print("  region   MID    LOW    SLOW   FINE  | v1 (vx,vy)     p1     z1   | v2 (vx,vy)     p2   | TRAVEL  WIND")
    out = {"label": label, "clip": a.clip, "frames": n, "cam_x": float(sh[1:, 0].mean()), "cam_y": float(sh[1:, 1].mean()), "regions": {}}
    for name in a.regions.split(","):
        r = analyse_region(frames, S, REGIONS[name])
        out["regions"][name] = r
        print("  %-8s %.3f  %.3f  %.3f  %.3f | %+5.1f %+5.1f  %+.2f  %+.2f | %+5.1f %+5.1f  %+.2f | %+.2f   %.3f"
              % (name, r["mid"], r["low"], r["slow"], r["fine"], r["vx1"], r["vy1"], r["p1"], r["z1"],
                 r["vx2"], r["vy2"], r["p2"], r["travel"], r["wind"]))
    if a.json:
        with open(a.json, "w") as f:
            json.dump(out, f, indent=1)
    if a.amp or a.mp4:
        outdir = a.out or os.path.dirname(os.path.abspath(a.json or a.clip))
        os.makedirs(outdir, exist_ok=True)
        t_amp = a.amp_at if a.amp_at is not None else n // 2
        amp_views(frames, S, label, outdir, t_amp, mp4=a.mp4)
        print("  wrote amp_%s.png, xt_%s.png%s in %s" % (label, label, ", amp2x_%s.mp4" % label if a.mp4 else "", outdir))


if __name__ == "__main__":
    main()
