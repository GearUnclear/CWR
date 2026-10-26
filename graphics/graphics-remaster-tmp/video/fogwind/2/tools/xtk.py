"""Agent 2 (fogwind): static-camera fog motion checks.

usage: python xtk.py <clip.mkv> [more ...] [--from N] [--to M] [--png prefix] [--gain G]

For a STATIC camera probe (look_probe_mist3_static): every change of the fog is an artefact.
  r(t) = frame(t) - the temporal mean (per pixel), grey, full res.
Per region (same fractions as agent 1's kit):
  std      temporal std of r (code values), and of r after a 6 px / 24 px box blur (cell scale and
           the scale of a few cells: what reads as blobs, banks, wind)
  slow     std of r after a 9-frame temporal box mean (the slow part: boiling / drift, not fizz)
  kw       k-omega travel coherence of r's mid band (box 6 px minus box 48 px): |sum P w k| /
           sqrt(sum P w^2 * sum P |k|^2) over the 3D spectrum. A plane wave travelling one way: 1.
           Standing flicker or isotropic noise: about 0 (noise floor about 1/sqrt(bins)).
  vel      the energy-weighted velocity -sum P w k / sum P |k|^2 (px/frame, x right, y down)
--png PREFIX writes PREFIX_xt.png (x-t slice through the valley/bay row, r * gain + 128, time down)
and PREFIX_yt.png (y-t slice through the bay column).
"""
import argparse, json, subprocess
import numpy as np
from scipy import ndimage

FF = "C:/ffmpeg/bin/ffmpeg.exe"
Y0, Y1 = 400, 820  # rows kept (the band); region rows are offset by Y0
REG = {
    "bay": (0.56, 0.70, 0.72, 0.97),
    "valley": (0.54, 0.64, 0.58, 0.68),
    "farhill": (0.45, 0.53, 0.25, 0.55),
    "haze": (0.47, 0.55, 0.62, 0.97),
    "slope": (0.56, 0.68, 0.30, 0.48),
}


def decode(src, a, b):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", "format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, 1080, 1920)
    return x[a:b, Y0:Y1].astype(np.float32)


def kw(r):
    # r: t, y, x (already mid-banded); Hann window in t
    t = r.shape[0]
    w = np.hanning(t)[:, None, None]
    F = np.fft.fftn(r * w)
    P = np.abs(F) ** 2
    om = np.fft.fftfreq(t)[:, None, None]
    ky = np.fft.fftfreq(r.shape[1])[None, :, None]
    kx = np.fft.fftfreq(r.shape[2])[None, None, :]
    P[0] = 0.0
    sx, sy = (P * om * kx).sum(), (P * om * ky).sum()
    k2 = (P * (kx ** 2 + ky ** 2)).sum()
    w2 = (P * om ** 2).sum()
    coh = float(np.hypot(sx, sy) / np.sqrt(w2 * k2 + 1e-30))
    vel = (float(-sx / (k2 + 1e-30)), float(-sy / (k2 + 1e-30)))
    return coh, vel


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src", nargs="+")
    ap.add_argument("--from", dest="a", type=int, default=15)
    ap.add_argument("--to", dest="b", type=int, default=None)
    ap.add_argument("--png", default="")
    ap.add_argument("--gain", type=float, default=20.0)
    ap.add_argument("--json", default="")
    o = ap.parse_args()
    allres = {}
    for src in o.src:
        x = decode(src, o.a, o.b)
        r = x - x.mean(axis=0, keepdims=True)
        res = {"frames": int(len(x))}
        for name, (y0, y1, x0, x1) in REG.items():
            ys, ye, xs, xe = int(y0 * 1080) - Y0, int(y1 * 1080) - Y0, int(x0 * 1920), int(x1 * 1920)
            rr = r[:, ys:ye, xs:xe]
            b6 = ndimage.uniform_filter(rr, size=(1, 6, 6))
            b24 = ndimage.uniform_filter(rr, size=(1, 24, 24))
            b48 = ndimage.uniform_filter(rr, size=(1, 48, 48))
            slow = ndimage.uniform_filter1d(b6, 9, axis=0)
            mid = (b6 - b48)[:, 24:-24, 24:-24]
            coh, vel = kw(mid - mid.mean(axis=0, keepdims=True))
            res[name] = dict(std=round(float(rr.std()), 3), std6=round(float(b6.std()), 3),
                             std24=round(float(b24.std()), 3), slow=round(float(slow[4:-4].std()), 3),
                             kw=round(coh, 3), vel=[round(vel[0], 2), round(vel[1], 2)])
        allres[src] = res
        print(src)
        for name in REG:
            print("  %-8s %s" % (name, json.dumps(res[name])))
        if o.png:
            from PIL import Image
            pre = o.png if len(o.src) == 1 else o.png + "_" + str(o.src.index(src))
            row = int(0.60 * 1080) - Y0
            xt = r[:, row - 1:row + 2, :].mean(axis=1)
            Image.fromarray(np.clip(xt * o.gain + 128, 0, 255).astype(np.uint8)).save(pre + "_xt.png")
            col = int(0.84 * 1920)
            yt = r[:, 0:400, col - 1:col + 2].mean(axis=2)  # t, y
            Image.fromarray(np.clip(yt * o.gain + 128, 0, 255).astype(np.uint8)).save(pre + "_yt.png")
    if o.json:
        with open(o.json, "w") as f:
            json.dump(allres, f, indent=1)


if __name__ == "__main__":
    main()
