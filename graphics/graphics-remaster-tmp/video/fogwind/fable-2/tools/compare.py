"""compare.py (fable-2): one sheet, one row per clip, K consecutive frames of a zoomed crop with the temporal
residual amplified, so several candidates can be compared by eye at the same frame.  Held-camera clips only
(no stabilisation here).

    python compare.py --out sheet.png --frame F [--k 3] [--amp 10] [--zoom 3] [--crop y0,y1,x0,x1] LABEL=clip.mkv ...

The crop default is the valley mist and the near shore under the sun (fractions of the 1920x1080 frame).
"""
import argparse, subprocess, sys
import numpy as np
from scipy import ndimage as ndi
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080


def decode(path, t0, t1):
    vf = "trim=start_frame=%d:end_frame=%d,setpts=N/FRAME_RATE/TB" % (t0, t1)
    out = subprocess.run([FF, "-v", "error", "-i", path, "-vf", vf, "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "gray", "-"],
                         capture_output=True, check=True).stdout
    n = len(out) // (W * H)
    return np.frombuffer(out, np.uint8)[: n * W * H].reshape(n, H, W).astype(np.float32)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--frame", type=int, required=True)
    ap.add_argument("--k", type=int, default=3)
    ap.add_argument("--amp", type=float, default=10.0)
    ap.add_argument("--zoom", type=int, default=3)
    ap.add_argument("--crop", default="0.52,0.66,0.60,0.86")
    ap.add_argument("clips", nargs="+")
    a = ap.parse_args()
    cy0, cy1, cx0, cx1 = [float(v) for v in a.crop.split(",")]
    y0, y1, x0, x1 = int(cy0 * H), int(cy1 * H), int(cx0 * W), int(cx1 * W)
    rows = []
    for spec in a.clips:
        label, path = spec.split("=", 1)
        t0 = max(0, a.frame - 8)
        fr = decode(path, t0, a.frame + a.k + 8)[:, y0:y1, x0:x1]
        sm = ndi.gaussian_filter1d(fr, 1.5, axis=0, mode="nearest")
        hp = fr - sm
        tiles = []
        for k in range(a.frame - t0, a.frame - t0 + a.k):
            f = fr[k] + a.amp * ndi.gaussian_filter(hp[k], 1.0)
            tile = np.clip(f, 0, 255).astype(np.uint8)
            im = Image.fromarray(tile).resize(((x1 - x0) * a.zoom, (y1 - y0) * a.zoom), Image.BILINEAR)
            ImageDraw.Draw(im).text((6, 4), "%s  f%d  x%g" % (label, t0 + k, a.amp), fill=255)
            tiles.append(np.asarray(im))
        # last tile: the plain frame (look), and the residual alone x amp (grey 128 = 0)
        plain = Image.fromarray(np.clip(fr[a.frame - t0], 0, 255).astype(np.uint8)).resize(((x1 - x0) * a.zoom, (y1 - y0) * a.zoom), Image.BILINEAR)
        ImageDraw.Draw(plain).text((6, 4), "%s  f%d  plain" % (label, a.frame), fill=255)
        tiles.append(np.asarray(plain))
        res = Image.fromarray(np.clip(128 + 3 * a.amp * ndi.gaussian_filter(hp[a.frame - t0], 1.0), 0, 255).astype(np.uint8)).resize(
            ((x1 - x0) * a.zoom, (y1 - y0) * a.zoom), Image.BILINEAR)
        ImageDraw.Draw(res).text((6, 4), "%s  f%d  residual alone x%g" % (label, a.frame, 3 * a.amp), fill=255)
        tiles.append(np.asarray(res))
        row = np.concatenate(tiles, 1)
        rows.append(row)
        rows.append(np.full((6, row.shape[1]), 255, np.uint8))
    Image.fromarray(np.concatenate(rows, 0)).save(a.out)
    print("wrote", a.out)


if __name__ == "__main__":
    main()
