r"""Ears for an agent that has none: measuring and drawing sounds, and the few
signal tools the build needs after REAPER has rendered the raw material.

numpy, scipy, soundfile and Pillow only.
"""
import os
import struct

import numpy as np
from scipy import signal
import soundfile as sf
from PIL import Image, ImageDraw

SR = 44100
BANDS = [(0, 60), (60, 250), (250, 1000), (1000, 4000), (4000, 11000)]


# ---------------------------------------------------------------------------
# reading
# ---------------------------------------------------------------------------
def load(path):
    """Returns (stereo or mono array as float, sample rate)."""
    x, sr = sf.read(path, dtype='float64')
    return x, sr


def fold(x):
    """Stereo to mono the way the game hears it (the mean of left and right).
    Returns (mono, loss_db): loss is how much quieter the mono sum is than the
    two channels on their own; a big number means the channels cancel."""
    if x.ndim == 1:
        return x, 0.0
    m = x.mean(axis=1)
    side = np.sqrt(np.mean(x ** 2) + 1e-20)
    mid = np.sqrt(np.mean(m ** 2) + 1e-20)
    return m, float(20 * np.log10(mid / side))


def load_mono(path, sr=SR):
    x, r = load(path)
    m, loss = fold(x)
    if r != sr:
        m = signal.resample_poly(m, sr, r)
    return m, loss


# ---------------------------------------------------------------------------
# measuring
# ---------------------------------------------------------------------------
def measure(x, sr=SR):
    spec = np.abs(np.fft.rfft(x)) ** 2
    f = np.fft.rfftfreq(len(x), 1 / sr)
    tot = np.sum(spec) + 1e-20
    bands = {f"{lo}-{hi}Hz": round(float(np.sum(spec[(f >= lo) & (f < hi)]) / tot), 4) for lo, hi in BANDS}
    k = int(sr * 0.25)
    rms = np.array([np.sqrt(np.mean(x[i: i + k] ** 2)) for i in range(0, len(x) - k + 1, k)])
    env_db = np.round(20 * np.log10(rms / (rms.max() + 1e-12) + 1e-6), 1).tolist()
    edge = int(sr * 0.02)
    return {
        'seconds': round(len(x) / sr, 3),
        'sample_rate': sr,
        'peak': round(float(np.max(np.abs(x))), 4),
        'rms_db': round(float(20 * np.log10(np.sqrt(np.mean(x ** 2)) + 1e-12)), 2),
        'centroid_hz': round(float(np.sum(f * spec) / tot), 1),
        'band_energy': bands,
        'start_db': round(float(20 * np.log10(np.max(np.abs(x[:edge])) + 1e-9)), 1),
        'end_db': round(float(20 * np.log10(np.max(np.abs(x[-edge:])) + 1e-9)), 1),
        'dc': round(float(np.mean(x)), 5),
        'envelope_db_per_250ms': env_db,
    }


def roughness(x, sr=SR):
    """How much the loudness flutters at 20 to 90 Hz (growl, flutter-tongue,
    rattle), as a fraction of the mean level. A pure held tone gives about 0."""
    e = np.abs(signal.hilbert(x))
    e = signal.sosfilt(signal.butter(2, 200, 'low', fs=sr, output='sos'), e)
    b = signal.sosfilt(signal.butter(2, [20, 90], 'band', fs=sr, output='sos'), e)
    return float(np.sqrt(np.mean(b ** 2)) / (np.mean(e) + 1e-12))


def noisiness(x, sr=SR):
    """Spectral flatness between 200 Hz and 6 kHz: near 0 for a clean tone,
    toward 1 for hiss."""
    f, p = signal.welch(x, sr, nperseg=4096)
    m = (f > 200) & (f < 6000)
    p = p[m] + 1e-20
    return float(np.exp(np.mean(np.log(p))) / np.mean(p))


def describe(x, sr=SR):
    m = measure(x, sr)
    b = m['band_energy']
    return (f"{m['seconds']:5.1f}s peak {m['peak']:.2f} rms {m['rms_db']:6.1f} cen {m['centroid_hz']:6.0f}Hz "
            f"sub {b['0-60Hz']:.2f} low {b['60-250Hz']:.2f} mid {b['250-1000Hz']:.2f} hi {b['1000-4000Hz']:.2f} "
            f"air {b['4000-11000Hz']:.2f} rough {roughness(x, sr):.2f} noise {noisiness(x, sr):.2f}")


def loop_seam(x, sr=SR):
    """For a loop: the jump from the last sample to the first, against the
    biggest jump between neighbours inside the file, and the level either side
    of the seam. A seam jump no larger than the inside jumps cannot click."""
    inside = float(np.max(np.abs(np.diff(x))))
    seam = float(abs(x[0] - x[-1]))
    k = int(sr * 0.1)
    a = 20 * np.log10(np.sqrt(np.mean(x[:k] ** 2)) + 1e-12)
    b = 20 * np.log10(np.sqrt(np.mean(x[-k:] ** 2)) + 1e-12)
    # spectrum either side of the seam
    fa = np.abs(np.fft.rfft(x[:4096] * np.hanning(4096)))
    fb = np.abs(np.fft.rfft(x[-4096:] * np.hanning(4096)))
    sim = float(np.sum(fa * fb) / (np.sqrt(np.sum(fa ** 2) * np.sum(fb ** 2)) + 1e-20))
    return {'seam_jump': round(seam, 5), 'largest_inside_jump': round(inside, 5),
            'level_first_100ms_db': round(float(a), 1), 'level_last_100ms_db': round(float(b), 1),
            'spectrum_match': round(sim, 3)}


# ---------------------------------------------------------------------------
# drawing
# ---------------------------------------------------------------------------
def _panel(x, sr, label, top=6000.0, h=300, env_h=80):
    hop = int(sr * 0.01)
    f, t, s = signal.spectrogram(x, sr, nperseg=2048, noverlap=2048 - hop, window='hann')
    db = 10 * np.log10(s + 1e-12)
    db -= db.max()
    rows = np.linspace(0, 1, h) ** 2 * min(top, sr / 2 - 1)
    idx = np.clip(np.searchsorted(f, rows), 0, len(f) - 1)
    img = db[idx, :][::-1]
    v = np.clip((img + 70) / 70, 0, 1)
    rgb = np.zeros(v.shape + (3,), dtype=np.uint8)
    rgb[..., 0] = (255 * np.clip(v * 1.6, 0, 1)).astype(np.uint8)
    rgb[..., 1] = (255 * np.clip(v * 1.6 - 0.45, 0, 1)).astype(np.uint8)
    rgb[..., 2] = (255 * np.clip(v * 1.6 - 1.0, 0, 1) + 40 * (1 - v)).astype(np.uint8)
    w = rgb.shape[1]
    canvas = Image.new('RGB', (w + 60, h + env_h + 34), (16, 16, 20))
    canvas.paste(Image.fromarray(rgb), (50, 10))
    d = ImageDraw.Draw(canvas)
    for hz in (60, 250, 1000, 2000, 4000, 6000):
        if hz > top:
            continue
        y = 10 + h - 1 - int(np.sqrt(hz / top) * (h - 1))
        d.line([(46, y), (50, y)], fill=(200, 200, 200))
        d.text((2, y - 5), f"{hz}", fill=(200, 200, 200))
    rms = np.array([np.sqrt(np.mean(x[i: i + hop] ** 2) + 1e-12) for i in range(0, len(x) - hop, hop)])
    e = np.clip((20 * np.log10(rms / rms.max()) + 50) / 50, 0, 1)
    base = 10 + h + 10 + env_h
    pts = [(50 + i, base - int(e[i] * (env_h - 4))) for i in range(min(len(e), w))]
    if len(pts) > 1:
        d.line(pts, fill=(255, 190, 90))
    for sec in range(0, int(len(x) / sr) + 1):
        xx = 50 + sec * 100
        d.line([(xx, base), (xx, base + 4)], fill=(200, 200, 200))
        d.text((xx - 3, base + 6), f"{sec}", fill=(200, 200, 200))
    d.text((54, 12), label, fill=(255, 255, 255))
    return canvas


def picture(panels, out, top=6000.0):
    """panels: list of (samples, sample_rate, label). Stacked top to bottom,
    one pixel per 10 ms, 0 to 6 kHz on a square-root axis, loudness under each."""
    ims = [_panel(x, sr, label, top) for (x, sr, label) in panels]
    w = max(i.width for i in ims)
    hgt = sum(i.height for i in ims)
    canvas = Image.new('RGB', (w, hgt), (16, 16, 20))
    y = 0
    for i in ims:
        canvas.paste(i, (0, y))
        y += i.height
    canvas.save(out)


# ---------------------------------------------------------------------------
# tools
# ---------------------------------------------------------------------------
def n_of(sec, sr=SR):
    return int(round(sec * sr))


def env(points, dur, sr=SR):
    t = np.arange(n_of(dur, sr)) / sr
    return np.interp(t, [p[0] for p in points], [p[1] for p in points])


def lp(x, fc, order=2, sr=SR):
    return signal.sosfilt(signal.butter(order, fc, 'low', fs=sr, output='sos'), x)


def hp(x, fc, order=2, sr=SR):
    return signal.sosfilt(signal.butter(order, fc, 'high', fs=sr, output='sos'), x)


def bp(x, lo, hi, order=2, sr=SR):
    return signal.sosfilt(signal.butter(order, [lo, hi], 'band', fs=sr, output='sos'), x)


def norm(x, peak=0.89):
    return x * (peak / (np.max(np.abs(x)) + 1e-12))


def rms_norm(x, db):
    return x * (10 ** (db / 20) / (np.sqrt(np.mean(x ** 2)) + 1e-12))


def soft(x, drive):
    return np.tanh(x * drive) / np.tanh(drive)


def fade(x, t_in=0.005, t_out=0.02, sr=SR):
    x = x.copy()
    a, b = n_of(t_in, sr), n_of(t_out, sr)
    if a > 0:
        x[:a] *= np.linspace(0, 1, a)
    if b > 0:
        x[-b:] *= np.linspace(1, 0, b)
    return x


def pad(x, before=0.0, after=0.0, sr=SR):
    return np.concatenate([np.zeros(n_of(before, sr)), x, np.zeros(n_of(after, sr))])


def mix(*parts):
    n = max(len(p) for p in parts)
    out = np.zeros(n)
    for p in parts:
        out[:len(p)] += p
    return out


def at(x, sec, sr=SR):
    return pad(x, before=sec, sr=sr)


def tape(x, speed):
    """Tape varispeed with a speed that changes over time. speed is a number
    or an array over the OUTPUT time (1 = as recorded, 0.5 = half speed, an
    octave down, twice as long). Pitch and formants move together, as on tape;
    that is what makes a slowed voice sound like a large body."""
    if np.isscalar(speed):
        n = int(len(x) / speed)
        pos = np.arange(n) * speed
    else:
        pos = np.cumsum(speed)
        pos = pos[pos < len(x) - 1]
    return np.interp(pos, np.arange(len(x)), x)


def speed_curve(points, dur, sr=SR):
    """Speed over output time from (time, semitones) points; 0 semitones = speed 1."""
    st = env(points, dur, sr)
    return 2.0 ** (st / 12.0)


def ringmod(x, freq, depth=1.0, sr=SR):
    """Ring modulation by a sine (freq may be an array)."""
    ph = 2 * np.pi * np.cumsum(np.broadcast_to(freq, x.shape)) / sr
    return x * ((1 - depth) + depth * np.sin(ph))


def reverb(x, rt60, wet, rng, predelay=0.03, lp_fc=3500, hp_fc=60, tail=None, taps=(), sr=SR):
    """Open-air reverb (decaying filtered noise, duller as it dies) plus
    discrete echoes (time, gain, lowpass) for hills and town walls."""
    tail = rt60 * 1.1 if tail is None else tail
    n = n_of(tail, sr)
    t = np.arange(n) / sr
    ir = rng.standard_normal(n) * 10 ** (-3 * t / rt60)
    ir = hp(lp(ir, lp_fc, sr=sr), hp_fc, sr=sr)
    ir = ir * 0.6 + lp(ir, lp_fc * 0.35, sr=sr) * 0.9 * np.clip(t / (rt60 * 0.5), 0, 1)
    ir = np.concatenate([np.zeros(n_of(predelay, sr)), ir])
    ir /= np.sqrt(np.sum(ir ** 2)) + 1e-12
    wet_sig = signal.fftconvolve(x, ir)
    dry = np.concatenate([x, np.zeros(len(wet_sig) - len(x))])
    out = dry * (1 - wet) + wet_sig * wet * 1.6
    for (dt, g, fc) in taps:
        echo = lp(x, fc, sr=sr) * g
        echo_r = signal.fftconvolve(echo, ir[: n_of(min(tail, 1.5), sr)]) * 0.8
        need = n_of(dt, sr) + len(echo_r)
        if need > len(out):
            out = np.concatenate([out, np.zeros(need - len(out))])
        out[n_of(dt, sr): n_of(dt, sr) + len(echo)] += echo
        out[n_of(dt, sr): n_of(dt, sr) + len(echo_r)] += echo_r * g
    return out


def loopable(x, xfade, sr=SR):
    """Seamless loop: the last xfade seconds are folded over the start with an
    equal-power crossfade, so the end of the file runs into its beginning."""
    k = n_of(xfade, sr)
    body = x[: len(x) - k].copy()
    tail = x[len(x) - k:]
    w = np.linspace(0, 1, k)
    body[:k] = body[:k] * np.sqrt(w) + tail * np.sqrt(1 - w)
    return body


def trim_silence(x, db=-60, keep=0.02, sr=SR):
    thr = 10 ** (db / 20) * np.max(np.abs(x))
    idx = np.where(np.abs(x) > thr)[0]
    if len(idx) == 0:
        return x
    a = max(0, idx[0] - n_of(keep, sr))
    b = min(len(x), idx[-1] + n_of(keep, sr))
    return x[a:b]


def write_wss(path, pcm16, sr):
    """Cold War Assault .wss, uncompressed: "WSS0", a delta-pack byte (0 =
    plain PCM), three reserved bytes, an 18 byte WAVEFORMATEX, then samples
    (the same writer as the first set: Audio/Streaming/WaveLoaders.cpp:22-63)."""
    head = b'WSS0' + bytes([0, 0, 0, 0])
    fmt = struct.pack('<HHIIHHH', 1, 1, sr, sr * 2, 2, 16, 0)
    with open(path, 'wb') as fh:
        fh.write(head + fmt + pcm16.astype('<i2').tobytes())
