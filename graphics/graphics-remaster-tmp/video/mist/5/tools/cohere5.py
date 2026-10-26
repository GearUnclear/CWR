"""Agent 5: coherent-sheet measure of a probe clip, next to bandmetric.

usage: python cohere5.py <clip.mkv> [<clip.mkv> ...] [--from 30]

Per clip: frames scaled to 480x270 grey, 15 px blocks (bandmetric's grid).
  band      = bandmetric's band (mean over band blocks of the RMS of d2)
  coh       = RMS over time of the band-MEAN d2 (the whole sheet moving
              together; the visible flicker per agents 3, 4 and 6)
  coh_valley / coh_bay / coh_spit = the same over three sub-regions of the
              480x270 frame (valley slope x 215-340 y 167-205; bay x 345-455
              y 165-195; far spit x 335-455 y 150-162)
  incoh     = RMS of the band blocks' d2 after removing the band-mean d2
Run with plain python.
"""
import argparse, json, subprocess
import numpy as np

W, H, B = 480, 270, 15
FF = "C:/ffmpeg/bin/ffmpeg.exe"
REG = dict(valley=(215, 340, 167, 205), bay=(345, 455, 165, 195), spit=(335, 455, 150, 162))


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


def d2(s):
    return s[1:-1] - 0.5 * (s[:-2] + s[2:])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src", nargs="+")
    ap.add_argument("--from", dest="a", type=int, default=30)
    ap.add_argument("--to", dest="b", type=int, default=None)
    o = ap.parse_args()
    for src in o.src:
        x = load(src)[o.a:o.b]
        gy, gx = H // B, W // B
        blk = x[:, :gy * B, :gx * B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
        bd = d2(blk)[:, 8:14, 5:32]
        band = float(np.sqrt((bd ** 2).mean(axis=0)).mean())
        m = bd.reshape(len(bd), -1).mean(axis=1)
        coh = float(np.sqrt((m ** 2).mean()))
        inc = float(np.sqrt(((bd - m[:, None, None]) ** 2).mean()))
        res = dict(src=src.split("/")[-1].split("\\")[-1], frames=len(x), band=round(band, 3), coh=round(coh, 3),
                   incoh=round(inc, 3))
        for k, (x0, x1, y0, y1) in REG.items():
            s = x[:, y0:y1, x0:x1].mean(axis=(1, 2))
            res["coh_" + k] = round(float(np.sqrt((d2(s) ** 2).mean())), 3)
        print(json.dumps(res))


if __name__ == "__main__":
    main()
