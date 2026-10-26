"""Per-frame difference between two renders of the montage (old vs new).

usage: python montdiff.py <old.mp4> <new.mp4> <out.json>
Decodes both at 240x135 grey, prints frame ranges where mean |diff| > 0.25 and the max outside them.
"""
import json, subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 240, 135


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W)


a = load(sys.argv[1])
b = load(sys.argv[2])
n = min(len(a), len(b))
d = np.abs(a[:n].astype(np.int16) - b[:n].astype(np.int16)).reshape(n, -1).mean(axis=1)
mx = np.abs(a[:n].astype(np.int16) - b[:n].astype(np.int16)).reshape(n, -1).max(axis=1)
on = d > 0.25
runs = []
i = 0
while i < n:
    if on[i]:
        j = i
        while j + 1 < n and on[j + 1]:
            j += 1
        runs.append([i, j, round(i / 60, 2), round(j / 60, 2), round(float(d[i:j + 1].max()), 2)])
        i = j + 1
    else:
        i += 1
res = dict(frames=[len(a), len(b)], runs=runs,
           identical_frames=int((mx == 0).sum()),
           max_mean_outside_900_1480=round(float(np.concatenate([d[:900], d[1480:]]).max()), 3),
           max_pix_outside_900_1480=int(np.concatenate([mx[:900], mx[1480:]]).max()),
           mean_in_906_1476=round(float(d[906:1477].mean()), 3))
print(json.dumps(res))
json.dump(dict(res, per_frame=[round(float(v), 3) for v in d]), open(sys.argv[3], "w"))
