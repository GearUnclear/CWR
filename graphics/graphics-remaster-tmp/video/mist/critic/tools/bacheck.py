"""Does before_after_scene2.mp4 show what it claims?

Part 1 frames 0..659 (BEFORE montage scene 2), part 2 660..1319 (AFTER), part 3 1320..1889
(crop: top = BEFORE old ud clip, bottom = AFTER new ud clip, each 960x360 at x160).
Block (16 px) mean d2 rms in the mist area of each, and spikes over 0.5.
"""
import subprocess, json, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
src = sys.argv[1]
r = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", "format=gray", "-f", "rawvideo", "-"],
                   capture_output=True, check=True)
x = np.frombuffer(r.stdout, np.uint8).reshape(-1, 720, 1280)
print("frames", len(x))


def m(a, B=16):
    a = a.astype(np.float32)
    n, h, w = a.shape
    blk = a[:, :h // B * B, :w // B * B].reshape(n, h // B, B, w // B, B).mean(axis=(2, 4))
    d = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    return dict(rms=round(float(np.sqrt((d ** 2).mean())), 3), spikes=int((np.abs(d) > 0.5).sum()))


# parts 1/2: the mist band of the 1280x720 montage (y 45%..78%, x 15%..100%)
y0, y1, x0 = int(0.45 * 720), int(0.78 * 720), int(0.15 * 1280)
res = dict(part1_before_band=m(x[30:630, y0:y1, x0:]), part2_after_band=m(x[690:1290, y0:y1, x0:]),
           part1_right_half_before=m(x[30:630, y0:y1, 640:]), part2_right_half_after=m(x[690:1290, y0:y1, 640:]),
           part3_top_before=m(x[1325:1885, 40:360, 160:1120]), part3_bottom_after=m(x[1325:1885, 400:720, 160:1120]))
print(json.dumps(res))
