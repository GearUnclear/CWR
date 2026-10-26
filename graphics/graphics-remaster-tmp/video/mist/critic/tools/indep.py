"""Critic's independent flicker measures for valley_mist (not bandmetric).

usage: python indep.py <name>=<clip.mkv> [...] --out <json>

For each clip, decodes the mist band at NATIVE resolution (no downscale):
crop x 288..1920, y 486..842 (screen 15%..100% x, 45%..78% y), luma.
Measures:
  pix_d2      per-pixel temporal second difference RMS (mean over pixels), plus p99
  blk<N>_d2   d2 RMS of block means at block sizes N = 4, 16, 64, 178 px
  coh_d2      d2 RMS of the whole crop mean (the 'one sheet' mode), and of thirds
  slow_<lo>   RMS of block-mean residuals in temporal bands (moving-average differences):
              'hf' = series minus 5-frame box (above ~6 Hz), 'mid' = 5-box minus 31-box
              (about 1-6 Hz, the 'breathing' that d2 under-weights)
  frames per-frame |coherent d2| top list
"""
import json, subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
X0, Y0, W, H = 288, 486, 1632, 356


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"crop={W}:{H}:{X0}:{Y0},format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W)


def box(x, k):
    # centred moving average along axis 0, edge-padded
    pad = k // 2
    xp = np.concatenate([np.repeat(x[:1], pad, 0), x, np.repeat(x[-1:], pad, 0)], 0)
    c = np.cumsum(np.concatenate([np.zeros_like(xp[:1]), xp], 0), 0)
    return (c[k:] - c[:-k]) / k


def d2rms(s):
    d = s[1:-1] - 0.5 * (s[:-2] + s[2:])
    return float(np.sqrt((d ** 2).mean())), d


def analyse(src, a=0, b=None):
    x = load(src)[a:b]
    n = len(x)
    res = dict(src=src, frames=n, range=[a, a + n])
    # per-pixel d2 in strips to save memory
    acc = np.zeros((H, W), np.float64)
    for t in range(1, n - 1):
        d = x[t].astype(np.float32) - 0.5 * (x[t - 1].astype(np.float32) + x[t + 1].astype(np.float32))
        acc += d.astype(np.float64) ** 2
    pr = np.sqrt(acc / (n - 2))
    res["pix_d2"] = round(float(pr.mean()), 3)
    res["pix_d2_p99"] = round(float(np.percentile(pr, 99)), 3)
    res["pix_d2_median"] = round(float(np.median(pr)), 3)
    for B in (4, 16, 64, 178):
        gy, gx = H // B, W // B
        blk = x[:, :gy * B, :gx * B].reshape(n, gy, B, gx, B).mean(axis=(2, 4), dtype=np.float32)
        r, d = d2rms(blk)
        res[f"blk{B}_d2"] = round(r, 3)
        res[f"blk{B}_d2_maxblock"] = round(float(np.sqrt((d ** 2).mean(axis=0)).max()), 3)
        if B == 64:
            hf = blk - box(blk, 5)
            mid = box(blk, 5) - box(blk, 31)
            res["blk64_hf"] = round(float(np.sqrt((hf[15:-15] ** 2).mean())), 3)
            res["blk64_mid"] = round(float(np.sqrt((mid[15:-15] ** 2).mean())), 3)
    m = x.reshape(n, -1).mean(axis=1).astype(np.float64)
    r, d = d2rms(m)
    res["coh_d2"] = round(r, 4)
    res["coh_mid"] = round(float(np.sqrt(((box(m, 5) - box(m, 31))[15:-15] ** 2).mean())), 4)
    res["coh_hf"] = round(float(np.sqrt(((m - box(m, 5))[15:-15] ** 2).mean())), 4)
    thirds = []
    for i in range(3):
        mm = x[:, :, i * W // 3:(i + 1) * W // 3].reshape(n, -1).mean(axis=1).astype(np.float64)
        thirds.append(round(d2rms(mm)[0], 4))
    res["coh_d2_thirds"] = thirds
    top = np.argsort(np.abs(d))[-6:][::-1]
    res["coh_worst"] = [(int(i) + 1 + a, round(float(d[i]), 3)) for i in top]
    res["band_mean"] = [round(float(m.min()), 2), round(float(m.max()), 2)]
    return res


def main():
    args = [s for s in sys.argv[1:]]
    out = args[args.index("--out") + 1]
    a = int(args[args.index("--from") + 1]) if "--from" in args else 0
    b = int(args[args.index("--to") + 1]) if "--to" in args else None
    pairs = [s for s in args if "=" in s and not s.startswith("--")]
    allres = {}
    for p in pairs:
        name, src = p.split("=", 1)
        r = analyse(src, a, b)
        print(name, json.dumps(r), flush=True)
        allres[name] = r
    with open(out, "w") as f:
        json.dump(allres, f, indent=1)


if __name__ == "__main__":
    main()
