"""Agent 4: compare probe clips on the coherent mist mode.

usage: python compare.py <name=clip.mkv> ... [--from 30]
Prints per clip: bandmetric-style band rms, the core's coherent d2 rms (rows 9-12,
cols 16-31 block mean), and the mean level of the core and the upper sky.
"""
import subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H, B = 480, 270, 15
args = [a for a in sys.argv[1:] if not a.startswith("--")]
a0 = 30
if "--from" in sys.argv:
    a0 = int(sys.argv[sys.argv.index("--from") + 1])
    args = [a for a in args if a != str(a0)]
for item in args:
    name, src = item.split("=", 1)
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"scale={W}:{H}:flags=area,format=gray",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)[a0:]
    T = len(x)
    blk = x[:, :H // B * B, :W // B * B].reshape(T, H // B, B, W // B, B).mean(axis=(2, 4))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    band = rms[8:14, 5:32].mean()
    core = d2[:, 9:13, 16:32].mean(axis=(1, 2))
    hor = d2[:, 6:9, 20:32].mean(axis=(1, 2))
    c = np.corrcoef(core, hor)[0, 1]
    print("%-10s frames %3d band %.3f core coherent d2 rms %.3f |core| median %.3f  horizon-sky coh %.3f corr %+.2f  levels core %.1f sky %.1f" % (
        name, T, band, np.sqrt((core ** 2).mean()), np.median(np.abs(core)), np.sqrt((hor ** 2).mean()), c,
        blk[:, 9:13, 16:32].mean(), blk[:, 0:6].mean()))
