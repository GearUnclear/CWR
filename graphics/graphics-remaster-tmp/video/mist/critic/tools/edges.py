"""Screen-edge strips of the mist band (newly revealed cells have no fog history): x 0-48 and 1872-1920, y 486-842.
16 px block d2 rms and spikes, plus the band interior 960-1008 for reference."""
import subprocess, sys, json
import numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"
for p in sys.argv[1:]:
    name, src = p.split("=", 1)
    r = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", "crop=1920:352:0:486,format=gray",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(r.stdout, np.uint8).reshape(-1, 352, 1920)
    out = {}
    for k, (a, b) in (("left", (0, 48)), ("right", (1872, 1920)), ("mid", (960, 1008))):
        s = x[:, :, a:b].astype(np.float32)
        n = len(s)
        blk = s.reshape(n, 22, 16, 3, 16).mean(axis=(2, 4))
        d = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
        out[k] = [round(float(np.sqrt((d ** 2).mean())), 3), int((np.abs(d) > 0.5).sum())]
    print(name, json.dumps(out), flush=True)
