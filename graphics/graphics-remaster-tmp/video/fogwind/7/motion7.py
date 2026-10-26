"""Agent 7 motion metrics for the fog band (static and moving probes).

python motion7.py <video> [--crop W:H:X:Y] [--from N] [--to M] [--xt out.png] [--label name]

The video is decoded to grey (a full 1920x1080 mkv is cropped to the fog band, default 1280:440:640:400;
a *-band-gray.mkv is already that crop). Prints one json line:
  d1      mean |Y(t) - Y(t-1)| (code values) after camera compensation (global shift, phase correlation)
  d1lp    the same on 8x8 block means (low frequency: pumping, travelling bands)
  hp      temporal std of the fine part (Y minus its 8 px box blur): grain / fizz
  vel     median |velocity| of the residual pattern relative to the scene (px/frame), from the
          cross-correlation of consecutive compensated differences; coh = its mean peak correlation
          (0 = no coherent travelling pattern, near 1 = a pattern that keeps its shape and moves: wind)
  shift   mean camera shift (px/frame)
"""
import argparse, json, subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def probe_size(src):
    p = subprocess.run([FF.replace("ffmpeg.exe", "ffprobe.exe"), "-v", "error", "-select_streams", "v:0",
                        "-show_entries", "stream=width,height", "-of", "csv=p=0", src], capture_output=True, text=True)
    w, h = p.stdout.strip().split(",")[:2]
    return int(w), int(h)


def load(src, crop, a, b):
    w, h = probe_size(src)
    vf = "format=gray"
    if w == 1280 and h == 440 and crop == "800:240:1120:470":
        crop = "800:240:480:70"
        w = 1920
    if w == 1920 and crop:
        cw, ch, cx, cy = (int(v) for v in crop.split(":"))
        vf = "crop=%d:%d:%d:%d,format=gray" % (cw, ch, cx, cy)
        w, h = cw, ch
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, h, w).astype(np.float32)
    return x[a:b]


def box(img, k):
    c = np.cumsum(np.cumsum(np.pad(img, ((1, 0), (1, 0))), 0), 1)
    h, w = img.shape
    out = np.zeros_like(img)
    r = k // 2
    ys0 = np.clip(np.arange(h) - r, 0, h); ys1 = np.clip(np.arange(h) + r + 1, 0, h)
    xs0 = np.clip(np.arange(w) - r, 0, w); xs1 = np.clip(np.arange(w) + r + 1, 0, w)
    s = c[ys1][:, xs1] - c[ys0][:, xs1] - c[ys1][:, xs0] + c[ys0][:, xs0]
    n = (ys1 - ys0)[:, None] * (xs1 - xs0)[None, :]
    return s / n


def window(h, w):
    return np.outer(np.hanning(h), np.hanning(w)).astype(np.float32)


def phase_shift(a, b, win):
    """Shift d such that b(x) ~ a(x - d)."""
    A = np.fft.rfft2((a - a.mean()) * win)
    B = np.fft.rfft2((b - b.mean()) * win)
    R = B * np.conj(A)
    R /= np.abs(R) + 1e-6
    r = np.fft.irfft2(R, s=a.shape)
    return peak(r)


def peak(r):
    h, w = r.shape
    i = np.unravel_index(np.argmax(r), r.shape)
    def sub(m1, c, p1):
        d = m1 - 2 * c + p1
        return 0.0 if abs(d) < 1e-12 else 0.5 * (m1 - p1) / d
    y, x = i
    dy = sub(r[(y - 1) % h, x], r[y, x], r[(y + 1) % h, x])
    dx = sub(r[y, (x - 1) % w], r[y, x], r[y, (x + 1) % w])
    py = y + dy; px = x + dx
    if py > h / 2: py -= h
    if px > w / 2: px -= w
    return np.array([py, px]), r[y, x]


def warp(img, d):
    """img shifted by d (subpixel, Fourier)."""
    h, w = img.shape
    fy = np.fft.fftfreq(h)[:, None]
    fx = np.fft.rfftfreq(w)[None, :]
    F = np.fft.rfft2(img)
    F *= np.exp(-2j * np.pi * (fy * d[0] + fx * d[1]))
    return np.fft.irfft2(F, s=img.shape)


def ncc_peak(a, b, maxd=12):
    """Normalised cross-correlation of a and b over shifts up to maxd: (shift of b relative to a, peak)."""
    a = a - a.mean(); b = b - b.mean()
    na = np.sqrt((a * a).sum()); nb = np.sqrt((b * b).sum())
    if na < 1e-6 or nb < 1e-6:
        return np.array([0.0, 0.0]), 0.0
    A = np.fft.rfft2(a); B = np.fft.rfft2(b)
    r = np.fft.irfft2(B * np.conj(A), s=a.shape) / (na * nb)
    h, w = r.shape
    rr = np.full_like(r, -1.0)
    ys = list(range(0, maxd + 1)) + list(range(h - maxd, h))
    xs = list(range(0, maxd + 1)) + list(range(w - maxd, w))
    rr[np.ix_(ys, xs)] = r[np.ix_(ys, xs)]
    return peak(rr)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("--crop", default="800:240:1120:470")
    ap.add_argument("--from", dest="a", type=int, default=0)
    ap.add_argument("--to", dest="b", type=int, default=None)
    ap.add_argument("--xt", default="")
    ap.add_argument("--label", default="")
    ap.add_argument("--static", action="store_true", help="no camera compensation")
    o = ap.parse_args()
    x = load(o.src, o.crop, o.a, o.b)
    n, h, w = x.shape
    win = window(h, w)
    d1, d1lp, shifts, vels, cohs = [], [], [], [], []
    hp = []
    res_prev = None
    lp_prev = None
    for t in range(1, n):
        a, b = x[t - 1], x[t]
        d = np.zeros(2) if o.static else phase_shift(a, b, win)[0]
        shifts.append(d)
        aw = warp(a, d) if not o.static else a
        m = 16
        r = (b - aw)[m:-m, m:-m]
        d1.append(np.abs(r).mean())
        lp = box(r, 8)
        d1lp.append(np.abs(lp).mean())
        if res_prev is not None:
            # the residual pattern of this step against the last one, the last carried by the camera
            prev_c = warp(np.pad(res_prev, m), d)[m:-m, m:-m]
            v, c = ncc_peak(box(prev_c, 4), box(r, 4))
            vels.append(np.hypot(*v)); cohs.append(c)
        res_prev = r
    fine = x - np.stack([box(f, 8) for f in x]) if o.static else None
    out = dict(label=o.label, src=o.src, frames=int(n),
               d1=round(float(np.mean(d1)), 4), d1lp=round(float(np.mean(d1lp)), 4),
               d1lp_p95=round(float(np.percentile(d1lp, 95)), 4),
               shift=round(float(np.mean([np.hypot(*s) for s in shifts])), 3),
               vel=round(float(np.median(vels)), 3) if vels else None,
               coh=round(float(np.mean(cohs)), 3) if cohs else None)
    if fine is not None:
        out["hp"] = round(float(fine[:, 16:-16, 16:-16].std(axis=0).mean()), 4)
        out["tstd_lp"] = round(float(np.stack([box(f, 8) for f in x])[:, 16:-16, 16:-16].std(axis=0).mean()), 4)
    print(json.dumps(out))
    if o.xt:
        from PIL import Image
        rows = [int(h * 0.35), int(h * 0.6)]
        strips = []
        for ry in rows:
            s = x[:, ry, :]
            strips.append(s)
            # amplified temporal difference strip (x8 around grey)
            dd = np.clip(128 + 8 * np.diff(s, axis=0), 0, 255)
            strips.append(np.vstack([np.full((1, w), 128), dd]))
        img = np.vstack([np.vstack([s, np.zeros((4, w))]) for s in strips])
        Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).save(o.xt)


if __name__ == "__main__":
    main()
