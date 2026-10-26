#!/usr/bin/env python
"""Agent 1: how the band flicker is spread over temporal frequency (what the eye sees at 60 fps).

  python spectrum.py <a.mkv>:<from>:<to> [<b.mkv>:<from>:<to> ...]

For each clip window (frames read at 480x270 grey like bandmetric): block means of the mist
band (bandmetric's rows 8-13, cols 5-31), each block detrended with a 31-frame moving
average (removes the pan and the slow drift), then:
  coh_rms   RMS over time of the band-averaged detrended signal (the coherent sheet, levels)
  blk_rms   mean over blocks of each block's detrended RMS (levels)
  low/high  share of the coherent signal's power below / above 10 Hz (60 fps time base)
  p2p_max   the largest frame-to-frame step of the coherent signal (levels)
"""
import subprocess
import sys

import numpy as np

W, H, B = 480, 270, 15
FF = "C:/ffmpeg/bin/ffmpeg.exe"


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"scale={W}:{H}:flags=area,format=gray",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


def main():
    for arg in sys.argv[1:]:
        src, a, b = arg.rsplit(":", 2)
        x = load(src)[int(a):int(b)]
        gy, gx = H // B, W // B
        blk = x[:, :gy * B, :gx * B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))[:, 8:14, 5:32]
        k = 31
        ker = np.ones(k) / k
        flat = blk.reshape(len(blk), -1)
        trend = np.stack([np.convolve(np.pad(flat[:, j], k // 2, mode="edge"), ker, mode="valid") for j in range(flat.shape[1])], 1)
        hp = (flat - trend)[k // 2:-(k // 2)]
        coh = hp.mean(axis=1)
        f = np.fft.rfftfreq(len(coh), 1 / 60.0)
        pw = np.abs(np.fft.rfft(coh - coh.mean())) ** 2
        low = pw[(f > 0) & (f < 10)].sum() / max(1e-9, pw[f > 0].sum())
        print(f"{src.split('/')[-1]}[{a}:{b}] frames={len(coh)} coh_rms={coh.std():.3f} blk_rms={hp.std(axis=0).mean():.3f} "
              f"low<10Hz={low:.2f} high>=10Hz={1 - low:.2f} p2p_max={np.abs(np.diff(coh)).max():.2f} p2p_rms={np.sqrt((np.diff(coh) ** 2).mean()):.3f}")


if __name__ == "__main__":
    main()
