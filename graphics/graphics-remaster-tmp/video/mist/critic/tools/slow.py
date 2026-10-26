"""Is the new clip's slow (1-6 Hz) band wander global (exposure) or the fog?

usage: python slow.py <name>=<clip.mkv> ... --out <json>
Region means per frame at 480x270 grey: band (rows 135..234 = y 45..78%, x 72..480), sky (y 0..30%),
whole frame. For each: rms of (box5 - box31) residual ('mid'), of (x - box5) ('hf'), and the
correlation of band mid with sky mid. Also the band/sky ratio series' mid rms (a pure gain cancels).
"""
import json, subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 480, 270


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


def box(x, k):
    pad = k // 2
    xp = np.concatenate([np.repeat(x[:1], pad, 0), x, np.repeat(x[-1:], pad, 0)], 0)
    c = np.cumsum(np.concatenate([np.zeros_like(xp[:1]), xp], 0), 0)
    return (c[k:] - c[:-k]) / k


def mid(s):
    return (box(s, 5) - box(s, 31))[15:-15]


out = {}
args = sys.argv[1:]
dst = args[args.index("--out") + 1]
for p in [a for a in args if "=" in a]:
    name, src = p.split("=", 1)
    x = load(src)
    band = x[:, 121:211, 72:].mean(axis=(1, 2)).astype(np.float64)
    sky = x[:, 0:81, :].mean(axis=(1, 2)).astype(np.float64)
    full = x.mean(axis=(1, 2)).astype(np.float64)
    # a narrower patch in the mist core (centre right), the worst area
    core = x[:, 135:195, 250:370].mean(axis=(1, 2)).astype(np.float64)
    r = dict(n=len(x))
    for k, s in (("band", band), ("sky", sky), ("full", full), ("core", core)):
        r[k + "_mid"] = round(float(np.sqrt((mid(s) ** 2).mean())), 4)
        r[k + "_hf"] = round(float(np.sqrt(((s - box(s, 5))[15:-15] ** 2).mean())), 4)
    mb, ms = mid(band), mid(sky)
    r["corr_band_sky_mid"] = round(float(np.corrcoef(mb, ms)[0, 1]), 3)
    lr = np.log(band) - np.log(sky)
    r["logratio_band_sky_mid_x1000"] = round(float(np.sqrt((mid(lr) ** 2).mean()) * 1000), 3)
    r["log_band_mid_x1000"] = round(float(np.sqrt((mid(np.log(band)) ** 2).mean()) * 1000), 3)
    r["log_sky_mid_x1000"] = round(float(np.sqrt((mid(np.log(sky)) ** 2).mean()) * 1000), 3)
    # spectrum of the detrended band mean (minus box31), power in bins
    det = (band - box(band, 31))[15:-15]
    f = np.fft.rfftfreq(len(det), 1 / 60)
    P = np.abs(np.fft.rfft(det * np.hanning(len(det)))) ** 2
    bins = [(0.5, 1), (1, 2), (2, 4), (4, 8), (8, 15), (15, 30)]
    r["band_power"] = {f"{a}-{b}Hz": round(float(P[(f >= a) & (f < b)].sum()), 2) for a, b in bins}
    r["series_band"] = [round(float(v), 3) for v in band]
    r["series_sky"] = [round(float(v), 3) for v in sky]
    r["series_core"] = [round(float(v), 3) for v in core]
    out[name] = r
    print(name, json.dumps({k: v for k, v in r.items() if not k.startswith("series")}), flush=True)
json.dump(out, open(dst, "w"))
