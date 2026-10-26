"""Shared helpers for critic1's motion instruments (written from scratch, numpy/scipy only)."""
import numpy as np
from scipy import ndimage

SCRATCH = "C:/Users/Dane/AppData/Local/Temp/claude/D--Arma-CWA/f07c0602-9af9-49db-b0ed-5558491d5e03/scratchpad/cache"
Y0 = 400  # the cache's first row in the frame

# Regions in FULL-FRAME pixel coords (x0, x1, y0, y1), chosen by eye on the frame 285/328 pose,
# soldiers excluded. Converted to cache rows with Y0.
REG = {
    "haze":    (1250, 1900, 470, 560),   # bright horizon air under the sun, far shore line
    "farhill": (480, 1000, 480, 560),    # the far hill's hazed flank
    "valley":  (1160, 1300, 590, 690),   # the valley mist between the standing soldier and the bay
    "bay":     (1420, 1880, 620, 760),   # the water and the far shore under the sun
    "mist2":   (1500, 1900, 760, 830),   # the low mist bank right of the kneeling soldier
}
TEX = (250, 900, 470, 700)  # textured near hillside for the camera estimate


def load(label):
    return np.load(f"{SCRATCH}/{label}.npy", mmap_mode="r")


def crop(a, box, pad=0):
    x0, x1, y0, y1 = box
    return a[..., y0 - Y0 - pad:y1 - Y0 + pad, x0 - pad:x1 + pad]


def hann2(h, w):
    return np.outer(np.hanning(h), np.hanning(w)).astype(np.float32)


def phase_shift(a, b, bp=(1.0, 8.0)):
    """Shift d (dy, dx) with b(x) ~ a(x - d), sub-pixel, by phase correlation of band-passed, windowed images."""
    a = a.astype(np.float32)
    b = b.astype(np.float32)
    if bp is not None:
        a = ndimage.gaussian_filter(a, bp[0]) - ndimage.gaussian_filter(a, bp[1])
        b = ndimage.gaussian_filter(b, bp[0]) - ndimage.gaussian_filter(b, bp[1])
    win = hann2(*a.shape)
    A = np.fft.fft2(a * win)
    B = np.fft.fft2(b * win)
    R = B * np.conj(A)
    R /= np.abs(R) + 1e-9
    r = np.fft.ifft2(R).real
    iy, ix = np.unravel_index(np.argmax(r), r.shape)
    h, w = r.shape

    def par(m1, c, p1):
        den = m1 - 2 * c + p1
        return 0.0 if abs(den) < 1e-12 else 0.5 * (m1 - p1) / den

    dy = iy + par(r[(iy - 1) % h, ix], r[iy, ix], r[(iy + 1) % h, ix])
    dx = ix + par(r[iy, (ix - 1) % w], r[iy, ix], r[iy, (ix + 1) % w])
    if dy > h / 2:
        dy -= h
    if dx > w / 2:
        dx -= w
    return dy, dx


def fshift(img, d):
    """Shift an image by d = (dy, dx) (content moves by +d), cubic spline, edge mode nearest."""
    return ndimage.shift(img.astype(np.float32), d, order=3, mode="nearest")


def bandpass(img, s0, s1):
    img = img.astype(np.float32)
    return ndimage.gaussian_filter(img, s0) - ndimage.gaussian_filter(img, s1)
