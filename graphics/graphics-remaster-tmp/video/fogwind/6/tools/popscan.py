import sys, subprocess, numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"; W, H = 1920, 1080
x0, y0, x1, y1 = [int(v) for v in sys.argv[1].split(",")]
for spec in sys.argv[2:]:
    path, lab = spec.rsplit(":", 1)
    p = subprocess.run([FF, "-v", "error", "-i", path, "-vf", "crop=%d:%d:%d:%d,format=gray" % (x1 - x0, y1 - y0, x0, y0), "-f", "rawvideo", "-pix_fmt", "gray", "-"], capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8).reshape(-1, y1 - y0, x1 - x0).astype(np.float32)
    m = a.mean((1, 2)); d2 = m[2:] - 2 * m[1:-1] + m[:-2]
    top = np.argsort(-np.abs(d2))[:8] + 1
    print("%-12s region-mean d2 rms %.3f  top frames %s" % (lab, d2[14:].std(), ", ".join("%d(%.2f)" % (t, d2[t - 1]) for t in sorted(top))))
