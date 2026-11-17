"""Pictures of Dedushka's sounds, for people (and agents) who cannot hear them.

    python spectro.py <build_dir> <out_dir>

Reads <build_dir>/wav/*.wav and writes one PNG per sound: the spectrogram
(0 to 6 kHz on a square-root frequency axis, so the low end has room; darker
is quieter) over the loudness envelope. Time runs left to right, one pixel
per 10 ms. numpy, scipy, soundfile and Pillow only.
"""
import os
import sys

import numpy as np
from scipy import signal
import soundfile as sf
from PIL import Image, ImageDraw


def picture(path, out):
    x, sr = sf.read(path)
    hop = int(sr * 0.01)
    f, t, s = signal.spectrogram(x, sr, nperseg=2048, noverlap=2048 - hop, window='hann')
    db = 10 * np.log10(s + 1e-12)
    db -= db.max()
    top = 6000.0
    h = 320
    rows = np.linspace(0, 1, h) ** 2 * top                     # square-root axis: row -> frequency
    idx = np.clip(np.searchsorted(f, rows), 0, len(f) - 1)
    img = db[idx, :][::-1]                                     # high frequencies at the top
    v = np.clip((img + 70) / 70, 0, 1)
    rgb = np.zeros(v.shape + (3,), dtype=np.uint8)
    rgb[..., 0] = (255 * np.clip(v * 1.6, 0, 1)).astype(np.uint8)
    rgb[..., 1] = (255 * np.clip(v * 1.6 - 0.45, 0, 1)).astype(np.uint8)
    rgb[..., 2] = (255 * np.clip(v * 1.6 - 1.0, 0, 1) + 40 * (1 - v)).astype(np.uint8)
    w = rgb.shape[1]
    env_h = 90
    canvas = Image.new('RGB', (w + 60, h + env_h + 30), (16, 16, 20))
    canvas.paste(Image.fromarray(rgb), (50, 10))
    d = ImageDraw.Draw(canvas)
    for hz in (60, 250, 1000, 2000, 4000, 6000):
        y = 10 + h - 1 - int(np.sqrt(hz / top) * (h - 1))
        d.line([(46, y), (50, y)], fill=(200, 200, 200))
        d.text((2, y - 5), f"{hz}", fill=(200, 200, 200))
    k = hop
    rms = np.array([np.sqrt(np.mean(x[i: i + k] ** 2) + 1e-12) for i in range(0, len(x) - k, k)])
    e = np.clip((20 * np.log10(rms / rms.max()) + 50) / 50, 0, 1)
    base = 10 + h + 10 + env_h
    pts = [(50 + i, base - int(e[i] * (env_h - 4))) for i in range(min(len(e), w))]
    if len(pts) > 1:
        d.line(pts, fill=(255, 190, 90))
    for sec in range(0, int(len(x) / sr) + 1):
        xx = 50 + sec * 100
        d.line([(xx, base), (xx, base + 4)], fill=(200, 200, 200))
        d.text((xx - 3, base + 6), f"{sec}", fill=(200, 200, 200))
    d.text((54, 12), f"{os.path.basename(path)}  {len(x) / sr:.1f} s  {sr} Hz", fill=(255, 255, 255))
    canvas.save(out)


def main():
    build, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    wav = os.path.join(build, 'wav')
    for name in sorted(os.listdir(wav)):
        if name.endswith('.wav'):
            picture(os.path.join(wav, name), os.path.join(out, name[:-4] + '.png'))
            print('wrote', name[:-4] + '.png')


if __name__ == '__main__':
    main()
