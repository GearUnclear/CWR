"""Sounds made here from nothing (numpy), and the loader for the kaiju sound sets.

Every function returns a mono float array at 48 kHz, peak at or under 1. Nothing is downloaded.
The kaiju sounds themselves (roars, footfalls, furnace, beam) come from the REAPER and VST set
(kaiju-notes\\features\\sound-vst\\round2\\build\\wav); see CREDITS.txt in final\\.
"""
import os

import numpy as np
import soundfile as sf
from scipy import signal

from edit_common import SND_R1, SND_R2, SR

_cache = {}


def load(name):
    """A kaiju sound by file stem (round 2 first, then round 1), mono, 48 kHz."""
    if name in _cache:
        return _cache[name]
    for d in (SND_R2, SND_R1):
        p = os.path.join(d, name + '.wav')
        if os.path.exists(p):
            x, sr = sf.read(p, dtype='float64')
            if x.ndim > 1:
                x = x.mean(axis=1)
            if sr != SR:
                g = np.gcd(SR, sr)
                x = signal.resample_poly(x, SR // g, sr // g)
            _cache[name] = x
            return x
    raise FileNotFoundError(name)


def _rng(seed):
    return np.random.default_rng(seed)


def _bp(x, lo, hi, order=2):
    sos = signal.butter(order, [lo, hi], btype='band', fs=SR, output='sos')
    return signal.sosfilt(sos, x)


def _lp(x, hi, order=2):
    return signal.sosfilt(signal.butter(order, hi, btype='low', fs=SR, output='sos'), x)


def _hp(x, lo, order=2):
    return signal.sosfilt(signal.butter(order, lo, btype='high', fs=SR, output='sos'), x)


def _norm(x, peak=0.9):
    m = np.abs(x).max()
    return x * (peak / m) if m > 0 else x


def brown(n, seed):
    w = _rng(seed).standard_normal(n)
    b = np.cumsum(w)
    b = _hp(b, 20)
    return _norm(b)


def wind(dur, seed=1):
    """An open-ground wind bed: band-limited brown noise with slow gusts."""
    n = int(dur * SR)
    x = _bp(brown(n, seed), 45, 420)
    t = np.arange(n) / SR
    r = _rng(seed + 7)
    gust = 0.62 + 0.2 * np.sin(2 * np.pi * 0.07 * t + r.uniform(0, 6)) + 0.14 * np.sin(2 * np.pi * 0.19 * t + r.uniform(0, 6)) + 0.06 * np.sin(2 * np.pi * 0.43 * t)
    hiss = _bp(r.standard_normal(n), 900, 3200) * 0.05 * gust
    return _norm(x * gust + hiss)


def sea(dur, seed=2):
    """Water against a hull or a shore at night: slow swells of filtered noise."""
    n = int(dur * SR)
    r = _rng(seed)
    t = np.arange(n) / SR
    x = _lp(r.standard_normal(n), 1400)
    swell = 0.45 + 0.4 * np.maximum(0, np.sin(2 * np.pi * 0.21 * t + 1.0)) ** 2 + 0.25 * np.maximum(0, np.sin(2 * np.pi * 0.13 * t + 2.2)) ** 3
    low = _bp(brown(n, seed + 3), 40, 180) * 0.6
    return _norm(x * swell * 0.7 + low)


def motor(dur, seed=3):
    """The film camera: a clockwork purr at 24 frames a second, with a little wow."""
    n = int(dur * SR)
    t = np.arange(n) / SR
    r = _rng(seed)
    rate = 24 * (1 + 0.012 * np.sin(2 * np.pi * 0.31 * t))
    ph = np.cumsum(rate) / SR
    click = (np.mod(ph, 1) < 0.08).astype(float)
    click = click * (0.7 + 0.3 * r.standard_normal(n))
    x = _bp(click, 500, 3600) + 0.25 * np.sin(2 * np.pi * 96 * t) * 0.3 + _bp(r.standard_normal(n), 1500, 5000) * 0.03
    return _norm(x)


def beep(dur=0.25, f=1000):
    n = int(dur * SR)
    t = np.arange(n) / SR
    e = np.minimum(1, np.minimum(t / 0.004, (dur - t) / 0.004))
    return 0.9 * np.sin(2 * np.pi * f * t) * e


def gun(far=False, seed=4):
    """A tank gun: crack, boom, rolling tail. far: the top taken off, a longer roll."""
    dur = 3.2 if far else 2.4
    n = int(dur * SR)
    t = np.arange(n) / SR
    r = _rng(seed)
    crack = r.standard_normal(n) * np.exp(-t / 0.012)
    boom_f = 34 + 46 * np.exp(-t / 0.09)
    boom = np.sin(2 * np.pi * np.cumsum(boom_f) / SR) * np.exp(-t / 0.42)
    tail = _lp(r.standard_normal(n), 700) * np.exp(-t / (1.1 if far else 0.7)) * np.minimum(1, t / 0.02)
    x = 0.8 * crack + 1.0 * np.tanh(2.2 * boom) + 0.55 * tail
    if far:
        x = _lp(x, 520, 2)
        echo = np.zeros(n)
        d = int(0.61 * SR)
        echo[d:] = x[:-d] * 0.35
        x = x + echo
    return _norm(x)


def burst(seed=5):
    """A shell bursting on something very large: a flat crack and a short thud."""
    dur = 1.6
    n = int(dur * SR)
    t = np.arange(n) / SR
    r = _rng(seed)
    crack = _bp(r.standard_normal(n), 300, 5200) * np.exp(-t / 0.05)
    body = _lp(r.standard_normal(n), 900) * np.exp(-t / 0.33)
    thud = np.sin(2 * np.pi * np.cumsum(40 + 50 * np.exp(-t / 0.05)) / SR) * np.exp(-t / 0.25)
    return _norm(0.9 * crack + 0.7 * body + 0.8 * thud)


def collapse(seed=6, dur=3.4):
    """A house coming down: a rising rumble, a break, then masonry settling."""
    n = int(dur * SR)
    t = np.arange(n) / SR
    r = _rng(seed)
    env = np.minimum(1, t / 0.35) * np.exp(-np.maximum(0, t - 0.5) / 0.9)
    rumble = _bp(brown(n, seed), 30, 260) * env
    grit = _bp(r.standard_normal(n), 400, 3800) * env * 0.35
    x = rumble + grit
    for _ in range(26):                                  # bricks and timbers
        at = r.uniform(0.25, dur - 0.5)
        k = int(at * SR)
        ln = int(r.uniform(0.02, 0.09) * SR)
        f = r.uniform(120, 900)
        tt = np.arange(ln) / SR
        x[k:k + ln] += r.uniform(0.15, 0.5) * np.sin(2 * np.pi * f * tt) * np.exp(-tt / 0.02) * np.exp(-(at - 0.3) / 1.2)
    return _norm(x)


def subhit(seed=8, dur=2.6):
    """The hit under the brand card: a falling sub with a hard front."""
    n = int(dur * SR)
    t = np.arange(n) / SR
    r = _rng(seed)
    f = 30 + 62 * np.exp(-t / 0.16)
    sub = np.tanh(3.0 * np.sin(2 * np.pi * np.cumsum(f) / SR)) * np.exp(-t / 0.75)
    front = r.standard_normal(n) * np.exp(-t / 0.018)
    grit = _lp(r.standard_normal(n), 1100) * np.exp(-t / 0.3)
    return _norm(sub + 0.7 * front + 0.5 * grit)


def far_version(x, cutoff=420, tail=0.35, seed=9):
    """A sound heard from far off: the top taken away and a short smear."""
    y = _lp(x, cutoff, 2)
    n = len(y)
    out = np.copy(y)
    for d, g in ((0.09, 0.5), (0.21, 0.35), (0.37, 0.22)):
        k = int(d * SR)
        if k < n:
            out[k:] += y[:-k] * g * tail / 0.35
    return _norm(out)
