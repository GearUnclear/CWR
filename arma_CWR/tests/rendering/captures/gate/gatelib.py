"""Shared helpers for the gate tools (vgate.py, motion.py, owner_sheet.py, bench_report.py).

Every gate script runs under `python -I` (isolated: no script dir, no cwd, no PYTHON*
variables on sys.path). numpy, scipy and Pillow live in the user site on this machine,
which -I also hides, so each script starts with the same two lines:

    import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import gatelib  # noqa: E402  (adds the user site back, explicitly)

Importing this module appends the user site-packages directory (and only that) to
sys.path and puts the parent capture rig folder (capgen.py, views.py, banddiff.py) on it.
Nothing from the current directory is ever importable.
"""
import os
import re
import site
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CAPTURES = os.path.dirname(HERE)
ENGINE_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", "..", ".."))  # arma_CWR
GIT_ROOT = os.path.dirname(ENGINE_ROOT)

for _p in (HERE, CAPTURES):
    if _p not in sys.path:
        sys.path.insert(0, _p)
try:
    _us = site.getusersitepackages()
    if _us and os.path.isdir(_us) and _us not in sys.path:
        sys.path.append(_us)
except Exception:  # pragma: no cover
    pass

# ---------------------------------------------------------------------------- UDCAP logs

UDCAP_RE = re.compile(r"UDCAP;(.*?)\"?\s*$")
TS_RE = re.compile(r"^\[([^\]]+)\]")


def udcap_lines(log_path):
    """Yield (timestamp, dict) for every UDCAP line of a game_stdout.log (or udcap.log).
    gpu= and perf= stay raw strings (they hold ';'-free but '|' and ':' rich text)."""
    with open(log_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            i = line.find("UDCAP;")
            if i < 0:
                continue
            body = line[i + 6:].rstrip().rstrip('"')
            ts = TS_RE.match(line)
            d = {}
            head, sep, gpu = body.partition(";gpu=")
            if sep:
                d["gpu"] = gpu
            head, sep, perf = head.partition(";perf=")
            if sep:
                d["perf"] = perf
            head, sep, feat = head.partition(";feat=")
            if sep:
                d["feat"] = feat
            for kv in head.split(";"):
                if "=" in kv:
                    k, v = kv.split("=", 1)
                    d.setdefault(k, v)
            yield (ts.group(1) if ts else ""), d


FEAT_NUM_RE = re.compile(r"\b(rain|wet|fog|sun|night|sky)=(-?[\d.]+)")


def feat_values(feat):
    return {k: float(v) for k, v in FEAT_NUM_RE.findall(feat or "")}


def shot_key(name):
    """`003_valley_0515_reset.png` -> `valley_0515_reset` (tri prefixes a sequence number)."""
    base = os.path.basename(name)
    base = re.sub(r"\.(png|bmp)$", "", base, flags=re.I)
    return re.sub(r"^\d+_", "", base)


# ---------------------------------------------------------------------------- fixed settle (GATE-2)
# The draw-count settle (triSettleScene [300, 30, 2]) stops once 30 frames drew within 2% of
# each other, so a change that moves the draw count (decal batching: combat_fires UNSETTLED:300
# off, OK:106 on) is shot 100 to 200 simulated frames later or earlier than its baseline and
# fires and smoke differ for that reason alone. The generators' --fixed-settle N writes
# capture_lib's fixed settle instead: exactly N frames (N - tail, a runner-side `triWait`
# under the capture hold for the worker-thread builds, the last `tail` frames, which must be
# idle). It needs triDeterministic on from the setup: without the capture hold the main loop
# keeps running frames of its own during the wait.

FIXED_SETTLE_TAIL = 30  # capture_lib.sqf capSettleTail
FIXED_SETTLE_MAX_WAIT_MS = 10000  # the capture hold lets go after 15 s without a tri command


def fixed_settle_error(frames, wait_ms, has_deterministic):
    """'' when --fixed-settle frames / --settle-wait-ms wait_ms can be used, else why not."""
    if not frames:
        return ""
    if not has_deterministic:
        return "--fixed-settle needs --has-deterministic (the capture hold keeps the loop's own frames off)"
    if not FIXED_SETTLE_TAIL < frames <= 600 + FIXED_SETTLE_TAIL:
        return f"--fixed-settle must be {FIXED_SETTLE_TAIL + 1} to {600 + FIXED_SETTLE_TAIL} frames"
    if not 0 <= wait_ms <= FIXED_SETTLE_MAX_WAIT_MS:
        return f"--settle-wait-ms must be 0 to {FIXED_SETTLE_MAX_WAIT_MS}"
    return ""


def fixed_settle_setup(frames, wait_ms):
    """The setup statement that turns capture_lib's fixed settle on."""
    return (f"call {{ capFixedSettle = {int(frames)}; capSettleWaitMs = {int(wait_ms)}; "
            'format ["view=setup;event=fixedframes;frames=%1;tail=%2;wait_ms=%3", capFixedSettle, capSettleTail, '
            "capSettleWaitMs] call capLog }")


def mode_lines(mode, frames=0, wait_ms=0):
    """The statements that put the view in a lighting mode: capMode alone (draw-count settle),
    or with frames > 0 capMode, the wall-clock pause and capModeTail (fixed settle)."""
    out = [f'"{mode}" call capMode']
    if frames:
        if wait_ms:
            out.append(f"triWait {int(wait_ms)}")
        out.append("[] call capModeTail")
    return out


# ---------------------------------------------------------------------------- images


def np():
    import numpy
    return numpy


def load_rgb(path):
    """uint8 HxWx3 array."""
    from PIL import Image
    import numpy
    with Image.open(path) as im:
        return numpy.asarray(im.convert("RGB"))


def srgb_to_linear(x):
    import numpy
    x = x.astype(numpy.float32) / 255.0
    return numpy.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4)


def rgb_to_xyz(rgb8):
    import numpy
    lin = srgb_to_linear(rgb8)
    m = numpy.array([[0.4124564, 0.3575761, 0.1804375],
                     [0.2126729, 0.7151522, 0.0721750],
                     [0.0193339, 0.1191920, 0.9503041]], numpy.float32)
    return lin @ m.T


def rgb_to_lab(rgb8):
    """CIELAB (D65) of an sRGB uint8 image, float32 HxWx3 (L 0..100)."""
    import numpy
    xyz = rgb_to_xyz(rgb8) / numpy.array([0.95047, 1.0, 1.08883], numpy.float32)
    e = 216 / 24389
    k = 24389 / 27
    f = numpy.where(xyz > e, numpy.cbrt(xyz), (k * xyz + 16) / 116)
    L = 116 * f[..., 1] - 16
    a = 500 * (f[..., 0] - f[..., 1])
    b = 200 * (f[..., 1] - f[..., 2])
    return numpy.stack([L, a, b], axis=-1)


def luma(rgb8):
    import numpy
    x = rgb8.astype(numpy.float32)
    return 0.299 * x[..., 0] + 0.587 * x[..., 1] + 0.114 * x[..., 2]


def ssim(a8, b8, sigma=1.5):
    """Mean SSIM on luma (Wang 2004, Gaussian window), inputs uint8 RGB or grey."""
    import numpy
    from scipy import ndimage
    x = luma(a8) if a8.ndim == 3 else a8.astype(numpy.float32)
    y = luma(b8) if b8.ndim == 3 else b8.astype(numpy.float32)
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    g = lambda z: ndimage.gaussian_filter(z, sigma, truncate=3.5)  # noqa: E731
    mx, my = g(x), g(y)
    sxx = g(x * x) - mx * mx
    syy = g(y * y) - my * my
    sxy = g(x * y) - mx * my
    s = ((2 * mx * my + c1) * (2 * sxy + c2)) / ((mx * mx + my * my + c1) * (sxx + syy + c2))
    return float(s.mean())


def resize(rgb8, width):
    from PIL import Image
    import numpy
    h = max(1, round(rgb8.shape[0] * width / rgb8.shape[1]))
    return numpy.asarray(Image.fromarray(rgb8).resize((width, h), Image.LANCZOS))


def delta_e(a8, b8, width=320, blur=1.0):
    """(mean, p99) CIE76 dE of the two images at `width` px after a light Gaussian blur."""
    import numpy
    from scipy import ndimage
    a, b = resize(a8, width), resize(b8, width)
    la, lb = rgb_to_lab(a), rgb_to_lab(b)
    if blur:
        la = numpy.stack([ndimage.gaussian_filter(la[..., c], blur) for c in range(3)], -1)
        lb = numpy.stack([ndimage.gaussian_filter(lb[..., c], blur) for c in range(3)], -1)
    d = numpy.sqrt(((la - lb) ** 2).sum(-1))
    return float(d.mean()), float(numpy.percentile(d, 99))


def cct_mccamy(rgb8):
    """Correlated colour temperature (K) of the mean linear colour of an image region."""
    import numpy
    xyz = rgb_to_xyz(rgb8).reshape(-1, 3).mean(0)
    s = xyz.sum()
    if s <= 1e-9:
        return float("nan")
    x, y = xyz[0] / s, xyz[1] / s
    n = (x - 0.3320) / (0.1858 - y)
    return float(449 * n ** 3 + 3525 * n ** 2 + 6823.3 * n + 5520.33)


def box_px(box, shape):
    """Box as fractions (y0, y1, x0, x1) -> pixel slices for an HxW(xC) array."""
    h, w = shape[0], shape[1]
    y0, y1, x0, x1 = box
    return slice(int(round(y0 * h)), max(int(round(y1 * h)), int(round(y0 * h)) + 1)), \
        slice(int(round(x0 * w)), max(int(round(x1 * w)), int(round(x0 * w)) + 1))


def font(size):
    from PIL import ImageFont
    for name in ("segoeui.ttf", "arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def sha256(path, chunk=1 << 20):
    import hashlib
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def ffmpeg():
    import shutil
    return os.environ.get("FFMPEG") or shutil.which("ffmpeg") or r"C:\ffmpeg\bin\ffmpeg.exe"


def write_json(path, obj):
    import json
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(obj, f, indent=1, sort_keys=False)
        f.write("\n")


def write_text(path, text):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
