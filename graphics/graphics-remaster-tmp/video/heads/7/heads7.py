"""Agent 7 head-wobble probe on the already shot v2 clips.

    python heads7.py strip <mkv> <x> <y> <w> <h> <start> <count> <out.png> [cols]
    python heads7.py measure <mkv> <start> <count> <name>:<hx>,<hy>,<hs>:<tx>,<ty>,<ts> ...

measure: for each man, the head box (centre hx,hy, half size hs) and a torso box
(tx,ty,ts) in the frame `start`; both are tracked frame to frame by integer SSD
search (+-R px); the head's motion relative to the torso, frame to frame, is the
wobble (camera motion and whole-body sway cancel). Prints per man: mean |dRel|
(px/frame), p95, the share of frames with |dRel| >= 1 px, the sign flips of the
relative motion (twitch back and forth), and the head's residual appearance
change after alignment (MAD, 0..255) versus the torso's.
"""
import subprocess
import sys

import numpy as np
from PIL import Image

R = 6


def read_frames(mkv, x, y, w, h, start, count):
    import os, glob
    if os.path.isdir(mkv):
        # a tri frame tree (*__fNNNN.png, pre-gamma on vanilla): gamma 1.2 as collect_frames does
        fs = sorted(glob.glob(os.path.join(mkv, "**", "*__f*.png"), recursive=True), key=lambda f: f[-9:])
        fs = fs[start:start + count]
        g = float(os.environ.get("H7GAMMA", "1.2"))
        lut = (255.0 * (np.arange(256) / 255.0) ** (1.0 / g)).round().clip(0, 255).astype(np.uint8)
        return np.stack([lut[np.asarray(Image.open(f).convert("RGB"))[y:y + h, x:x + w]] for f in fs])
    cmd = ["ffmpeg", "-v", "error", "-ss", f"{start / 60:.4f}", "-i", mkv, "-frames:v", str(count),
           "-vf", f"crop={w}:{h}:{x}:{y}", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"]
    raw = subprocess.run(cmd, capture_output=True, check=True).stdout
    n = len(raw) // (w * h * 3)
    return np.frombuffer(raw[: n * w * h * 3], np.uint8).reshape(n, h, w, 3)


def strip(mkv, x, y, w, h, start, count, out, cols=8):
    fr = read_frames(mkv, x, y, w, h, start, count)
    rows = (len(fr) + cols - 1) // cols
    sheet = np.zeros((rows * h, cols * w, 3), np.uint8)
    for i, f in enumerate(fr):
        r, c = divmod(i, cols)
        sheet[r * h:(r + 1) * h, c * w:(c + 1) * w] = f
    Image.fromarray(sheet).save(out)
    print(out, len(fr), "frames")


def best_shift(prev, cur, cx, cy, s):
    a = prev[cy - s:cy + s, cx - s:cx + s]
    best = None
    for dy in range(-R, R + 1):
        for dx in range(-R, R + 1):
            b = cur[cy - s + dy:cy + s + dy, cx - s + dx:cx + s + dx]
            if b.shape != a.shape:
                continue
            e = float(np.mean((a - b) ** 2))
            if best is None or e < best[0]:
                best = (e, dx, dy)
    e, dx, dy = best
    mad = float(np.mean(np.abs(a - cur[cy - s + dy:cy + s + dy, cx - s + dx:cx + s + dx])))
    return dx, dy, mad


def measure(mkv, start, count, men):
    pad = 2 * R + 4
    xs = [m[1][0] - m[1][2] for m in men] + [m[2][0] - m[2][2] for m in men]
    ys = [m[1][1] - m[1][2] for m in men] + [m[2][1] - m[2][2] for m in men]
    xe = [m[1][0] + m[1][2] for m in men] + [m[2][0] + m[2][2] for m in men]
    ye = [m[1][1] + m[1][2] for m in men] + [m[2][1] + m[2][2] for m in men]
    # room for drift: up to 3 px a frame of camera motion
    drift = min(300, 3 * count)
    x0 = max(0, min(xs) - pad - drift)
    y0 = max(0, min(ys) - pad - drift)
    x1 = min(1920, max(xe) + pad + drift)
    y1 = min(1080, max(ye) + pad + drift)
    fr = read_frames(mkv, x0, y0, x1 - x0, y1 - y0, start, count).astype(np.float32).mean(axis=3)
    out = {}
    for name, hb, tb in men:
        hx, hy, hs = hb[0] - x0, hb[1] - y0, hb[2]
        tx, ty, ts = tb[0] - x0, tb[1] - y0, tb[2]
        rel = []
        hv, tv = [], []
        hm, tm = [], []
        for k in range(1, len(fr)):
            try:
                tdx, tdy, tmad = best_shift(fr[k - 1], fr[k], tx, ty, ts)
                hdx, hdy, hmad = best_shift(fr[k - 1], fr[k], hx, hy, hs)
            except Exception:
                break
            rel.append((hdx - tdx, hdy - tdy))
            hv.append((hdx, hdy))
            tv.append((tdx, tdy))
            hm.append(hmad)
            tm.append(tmad)
            tx += tdx
            ty += tdy
            hx += hdx
            hy += hdy
            if not (hs + R < hx < fr.shape[2] - hs - R and hs + R < hy < fr.shape[1] - hs - R
                    and ts + R < tx < fr.shape[2] - ts - R and ts + R < ty < fr.shape[1] - ts - R):
                break
        rel = np.array(rel, np.float32) if rel else np.zeros((0, 2), np.float32)
        mag = np.hypot(rel[:, 0], rel[:, 1]) if len(rel) else np.zeros(0)
        flips = 0
        for axis in (0, 1):
            v = rel[:, axis][rel[:, axis] != 0]
            flips += int(np.sum(np.sign(v[1:]) != np.sign(v[:-1]))) if len(v) > 1 else 0
        def jerk(v):
            v = np.array(v, np.float32)
            if len(v) < 3:
                return 0.0
            a = v[1:] - v[:-1]
            return float(np.sqrt(np.mean(np.sum(a * a, axis=1))))
        def events(v):
            v = np.array(v, np.float32)
            if len(v) < 3:
                return 0.0
            a = np.hypot(*(v[1:] - v[:-1]).T)
            return float(np.sum(a > 1.5)) * 100.0 / len(a)
        hmA = np.array(hm, np.float32)
        tmA = np.array(tm, np.float32)
        spikes = int(np.sum(hmA > 3 * max(0.3, float(np.median(hmA))))) if len(hmA) else 0
        spikeFrames = [int(i + 1) for i in np.nonzero(hmA > 3 * max(0.3, float(np.median(hmA))))[0]] if len(hmA) else []
        turn = float(np.mean(hmA > np.maximum(2 * tmA, 2.0))) * 100 if len(hmA) else 0.0
        out[name] = dict(turn=turn, hm=[round(float(v), 2) for v in hm], tm=[round(float(v), 2) for v in tm], spikes=spikes, spikeFrames=spikeFrames, headEv=events(rel), headJerk=jerk(hv), torsoJerk=jerk(tv), n=len(rel), mean=float(mag.mean()) if len(mag) else 0, p95=float(np.percentile(mag, 95)) if len(mag) else 0,
                         moving=float(np.mean(mag >= 1)) if len(mag) else 0, flips=flips,
                         headMAD=float(np.mean(hm)) if hm else 0, torsoMAD=float(np.mean(tm)) if tm else 0,
                         hs=hs)
        o = out[name]
        print(f"{name:28s} n={o['n']:4d} relMean={o['mean']:.2f}px p95={o['p95']:.2f} moving={o['moving']:.2f} "
              f"turn/100f={o['turn']:.1f} spikes={o['spikes']:3d} flips={o['flips']:3d} relEv/100f={o['headEv']:.1f} headJerk={o['headJerk']:.2f} torsoJerk={o['torsoJerk']:.2f} headMAD={o['headMAD']:.2f} torsoMAD={o['torsoMAD']:.2f} headHalf={hs}")
    import json, os
    tag = os.environ.get("H7TAG")
    if tag:
        json.dump(out, open(f"h7_{tag}.json", "w"), indent=1)
    return out


def parse_box(s):
    return [int(float(v)) for v in s.split(",")]


if __name__ == "__main__":
    if sys.argv[1] == "strip":
        a = sys.argv[2:]
        strip(a[0], *[int(v) for v in a[1:7]], a[7], int(a[8]) if len(a) > 8 else 8)
    else:
        mkv, start, count = sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
        men = []
        for spec in sys.argv[5:]:
            name, hb, tb = spec.split(":")
            men.append((name, parse_box(hb), parse_box(tb)))
        measure(mkv, start, count, men)
