"""Agent 5: mean |frame difference| per box over a frame range of mkv clips (static camera: any change = artefact).
usage: python churn.py a b clip.mkv=label ...   (frame indices a..b, 0-based)"""
import subprocess, sys, numpy as np

boxes = {"haze_l": (470, 560, 400, 950), "bay": (600, 700, 1300, 1900), "farhill": (440, 500, 1000, 1500)}


def mkv(path, a, b):
    vf = "select=between(n" + chr(92) + ",%d" % a + chr(92) + ",%d)" % b
    raw = subprocess.run(["C:/ffmpeg/bin/ffmpeg.exe", "-loglevel", "error", "-i", path, "-vf", vf,
                          "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "gray", "-"], capture_output=True).stdout
    return [f.astype(float) for f in np.frombuffer(raw, np.uint8).reshape(-1, 1080, 1920)]


a, b = int(sys.argv[1]), int(sys.argv[2])
for spec in sys.argv[3:]:
    path, lab = spec.split("=") if "=" in spec else (spec, spec[-30:])
    fr = mkv(path, a, b)
    out = []
    for k, (y0, y1, x0, x1) in boxes.items():
        d = [np.abs(fr[i + 1][y0:y1, x0:x1] - fr[i][y0:y1, x0:x1]).mean() for i in range(len(fr) - 1)]
        out.append(f"{k} {np.mean(d):.3f}/{np.max(d):.3f}")
    print(f"{lab:16s} n={len(fr)} " + " | ".join(out))
