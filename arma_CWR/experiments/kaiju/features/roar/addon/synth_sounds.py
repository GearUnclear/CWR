"""Dedushka's sounds, made from nothing: oscillators, noise and filters only.

    python synth_sounds.py <out_dir> [--only name,name]

Writes <out_dir>/wav/<name>.wav (44.1 kHz or 22.05 kHz, 16 bit, mono), the
masters every other file is cut from, and <out_dir>/synth.json (duration,
peak, RMS and band energies per sound, so a build can check the result
without ears). No samples, no recordings, no downloads: numpy and scipy.

The roar follows the layer recipe in kaiju-notes/research/04 (friction, a
slowed animal voice, a metal screech, sub-bass), bent toward pain: the voice
cracks, the long note sags and ends in a shudder, and a thin metal keening
hangs on after the body of the roar is gone. He is a suffering test animal.

Everything is seeded, so a rebuild gives the same files.
"""
import argparse
import json
import os
import sys

import numpy as np
from scipy import signal
import soundfile as sf

SR = 44100


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def n_of(sec):
    return int(round(sec * SR))


def env(points, dur, curve=1.0):
    """Piecewise-linear envelope through (time, value) points, dur seconds."""
    t = np.arange(n_of(dur)) / SR
    ts = [p[0] for p in points]
    vs = [p[1] for p in points]
    e = np.interp(t, ts, vs)
    if curve != 1.0:
        e = np.sign(e) * np.abs(e) ** curve
    return e


def lp(x, fc, order=2):
    return signal.sosfilt(signal.butter(order, fc, 'low', fs=SR, output='sos'), x)


def hp(x, fc, order=2):
    return signal.sosfilt(signal.butter(order, fc, 'high', fs=SR, output='sos'), x)


def bp(x, lo, hi, order=2):
    return signal.sosfilt(signal.butter(order, [lo, hi], 'band', fs=SR, output='sos'), x)


def reson(x, f, q, gain=1.0):
    b, a = signal.iirpeak(f, q, fs=SR)
    return gain * signal.lfilter(b, a, x)


def white(n, rng):
    return rng.standard_normal(n)


def pink(n, rng):
    """Pink noise by shaping white noise in the frequency domain."""
    spec = np.fft.rfft(rng.standard_normal(n))
    f = np.fft.rfftfreq(n, 1 / SR)
    f[0] = f[1]
    x = np.fft.irfft(spec / np.sqrt(f), n)
    return x / (np.max(np.abs(x)) + 1e-12)


def brown(n, rng):
    x = np.cumsum(rng.standard_normal(n))
    x = hp(x, 12)
    return x / (np.max(np.abs(x)) + 1e-12)


def slow_noise(n, rate_hz, rng):
    """Smooth random control signal in -1..1 that wanders at about rate_hz."""
    k = max(4, int(n / SR * rate_hz * 2) + 4)
    pts = rng.uniform(-1, 1, k)
    return np.interp(np.linspace(0, k - 1, n), np.arange(k), pts)


def osc(freq, shape='sin', phase0=0.0):
    """Oscillator from a per-sample frequency array."""
    ph = phase0 + np.cumsum(freq) / SR
    if shape == 'sin':
        return np.sin(2 * np.pi * ph)
    fr = ph - np.floor(ph)
    if shape == 'saw':
        return 2 * fr - 1
    if shape == 'tri':
        return 4 * np.abs(fr - 0.5) - 1
    raise ValueError(shape)


def varispeed(x, factor):
    """Tape slow-down: factor 3 plays three times slower and 1.6 octaves lower."""
    n = int(len(x) * factor)
    return np.interp(np.linspace(0, len(x) - 1, n), np.arange(len(x)), x)


def soft(x, drive):
    return np.tanh(x * drive) / np.tanh(drive)


def norm(x, peak=0.89):
    return x * (peak / (np.max(np.abs(x)) + 1e-12))


def fade(x, t_in=0.005, t_out=0.02):
    x = x.copy()
    a, b = n_of(t_in), n_of(t_out)
    if a > 0:
        x[:a] *= np.linspace(0, 1, a)
    if b > 0:
        x[-b:] *= np.linspace(1, 0, b)
    return x


def pad(x, sec_before=0.0, sec_after=0.0):
    return np.concatenate([np.zeros(n_of(sec_before)), x, np.zeros(n_of(sec_after))])


def mix(*parts):
    n = max(len(p) for p in parts)
    out = np.zeros(n)
    for p in parts:
        out[:len(p)] += p
    return out


def reverb(x, rt60, wet, rng, predelay=0.03, lp_fc=3500, hp_fc=60, tail=None, taps=()):
    """Open-air reverb: decaying filtered noise, plus optional discrete echoes
    (time, gain, lowpass) for far hills and town walls answering."""
    tail = rt60 * 1.1 if tail is None else tail
    n = n_of(tail)
    t = np.arange(n) / SR
    ir = white(n, rng) * 10 ** (-3 * t / rt60)
    ir = hp(lp(ir, lp_fc), hp_fc)
    # the tail of an outdoor reverb gets duller as it decays
    ir = ir * 0.6 + lp(ir, lp_fc * 0.35) * 0.9 * np.clip(t / (rt60 * 0.5), 0, 1)
    ir = np.concatenate([np.zeros(n_of(predelay)), ir])
    ir /= np.sqrt(np.sum(ir ** 2)) + 1e-12
    wet_sig = signal.fftconvolve(x, ir)
    dry = np.concatenate([x, np.zeros(len(wet_sig) - len(x))])
    out = dry * (1 - wet) + wet_sig * wet * 1.6
    for (dt, g, fc) in taps:
        echo = lp(x, fc) * g
        echo_r = signal.fftconvolve(echo, ir[: n_of(min(tail, 1.5))]) * 0.8
        need = n_of(dt) + len(echo_r)
        if need > len(out):
            out = np.concatenate([out, np.zeros(need - len(out))])
        out[n_of(dt): n_of(dt) + len(echo)] += echo
        out[n_of(dt): n_of(dt) + len(echo_r)] += echo_r * g
    return out


def loopable(x, xfade):
    """Make x loop without a click: fold the last xfade seconds over the start."""
    k = n_of(xfade)
    body = x[: len(x) - k].copy()
    tail = x[len(x) - k:]
    w = np.linspace(0, 1, k)
    body[:k] = body[:k] * np.sqrt(w) + tail * np.sqrt(1 - w)
    return body


# ---------------------------------------------------------------------------
# the voice: a small animal's cry, then slowed on tape
# ---------------------------------------------------------------------------
def glottal_voice(f0, rough, breath, rng):
    """Pulse train with jitter, shimmer and period doubling (rough 0..1), plus
    breath noise gated by the pulses. f0 is a per-sample array in Hz."""
    n = len(f0)
    jit = 1 + 0.035 * slow_noise(n, 45, rng) + 0.02 * slow_noise(n, 8, rng)
    ph = np.cumsum(f0 * jit) / SR
    fr = ph - np.floor(ph)
    cyc = np.floor(ph).astype(np.int64)
    # asymmetric pulse: slow opening, fast closing
    pulse = np.where(fr < 0.62, np.sin(np.pi * fr / 0.62) ** 2, 0.0)
    pulse = np.diff(pulse, prepend=0.0) * 30
    # period doubling: every other cycle is weaker and later -> a growl an octave down
    odd = (cyc % 2).astype(float)
    sub = 1 - rough * 0.75 * odd
    # every third cycle also drops when it is very rough (a ragged crack)
    third = ((cyc % 3) == 0).astype(float)
    sub *= 1 - np.clip(rough - 0.6, 0, 1) * 0.8 * third
    shim = 1 + 0.18 * slow_noise(n, 30, rng) * (0.4 + rough)
    src = pulse * sub * shim
    asp = hp(white(n, rng), 900) * (0.25 + 0.75 * (fr < 0.62)) * breath
    return src + asp * 0.35


def vowel(x, formants):
    out = np.zeros(len(x))
    for (f, q, g) in formants:
        out += reson(x, f, q, g)
    return out


def animal_voice(rng):
    """About 2.9 s of a cornered animal's scream at its own pitch. Slowed 3.1x
    it becomes a 9 s bellow between 60 and 170 Hz with glacial vibrato."""
    dur = 2.9
    n = n_of(dur)
    t = np.arange(n) / SR
    # pitch: a rise, a held scream that breaks twice, a long sagging fall
    f0 = env([(0.0, 215), (0.18, 300), (0.42, 495), (0.62, 520), (0.66, 380), (0.74, 505),
              (1.15, 470), (1.19, 300), (1.30, 440), (1.75, 395), (2.25, 290), (2.9, 185)], dur)
    f0 *= 1 + 0.035 * np.sin(2 * np.pi * 6.3 * t + 2 * slow_noise(n, 2, rng))   # vibrato
    f0 *= 1 + 0.05 * slow_noise(n, 11, rng) * env([(0, 0.2), (1.7, 0.4), (2.9, 1.0)], dur)  # failing control
    rough = env([(0.0, 0.5), (0.3, 0.15), (0.6, 0.25), (0.64, 0.95), (0.74, 0.3), (1.13, 0.35),
                 (1.18, 1.0), (1.32, 0.45), (1.9, 0.5), (2.4, 0.8), (2.9, 0.95)], dur)
    breath = env([(0.0, 0.9), (0.3, 0.3), (1.8, 0.4), (2.9, 1.0)], dur)
    src = glottal_voice(f0, rough, breath, rng)
    # mouth opens (a), holds, closes to a moan (o -> u)
    va = vowel(src, [(760, 6, 1.0), (1250, 8, 0.7), (2550, 10, 0.35), (3500, 10, 0.2)])
    vo = vowel(src, [(520, 6, 1.0), (900, 8, 0.6), (2400, 10, 0.25), (3300, 10, 0.12)])
    vu = vowel(src, [(340, 5, 1.0), (700, 7, 0.45), (2200, 9, 0.12)])
    wa = env([(0, 0.0), (0.35, 1.0), (1.2, 0.9), (1.8, 0.2), (2.9, 0.0)], dur)
    wu = env([(0, 0.6), (0.3, 0.0), (1.6, 0.1), (2.3, 0.7), (2.9, 1.0)], dur)
    wo = np.clip(1 - wa - wu, 0, 1)
    v = va * wa + vo * wo + vu * wu
    amp = env([(0.0, 0.0), (0.06, 0.35), (0.4, 1.0), (0.62, 1.0), (0.66, 0.55), (0.76, 1.0),
               (1.15, 0.95), (1.19, 0.5), (1.32, 0.9), (1.9, 0.7), (2.5, 0.38), (2.9, 0.0)], dur)
    # the sob: an irregular tremor that takes over the falling note
    trem_rate = 5.0 + 1.8 * slow_noise(n, 1.5, rng)
    trem = 0.5 + 0.5 * np.sin(2 * np.pi * np.cumsum(trem_rate) / SR)
    depth = env([(0, 0.0), (1.6, 0.05), (2.1, 0.5), (2.9, 0.85)], dur)
    amp *= 1 - depth * trem
    return v * amp


# ---------------------------------------------------------------------------
# the roar
# ---------------------------------------------------------------------------
def friction_layer(dur, rng):
    """A dry hide dragged along a slack string: stick-slip sawtooth with missed
    cycles, ringing a big hollow body."""
    n = n_of(dur)
    f = env([(0, 46), (0.9, 61), (2.2, 73), (4.6, 66), (6.5, 52), (dur, 41)], dur)
    # per-cycle jitter and slips
    ph = np.cumsum(f * (1 + 0.1 * slow_noise(n, 60, rng))) / SR
    fr = ph - np.floor(ph)
    cyc = np.floor(ph).astype(np.int64)
    slip = rng.random(cyc.max() + 2) < 0.13
    saw = (2 * fr - 1) * np.where(slip[cyc], 0.25, 1.0)
    rosin = bp(white(n, rng), 700, 3200) * fr ** 2          # the scrape inside each stick
    x = saw + rosin * 0.9
    body = (reson(x, 98, 9, 1.0) + reson(x, 147, 10, 0.8) + reson(x, 233, 12, 0.6) +
            reson(x, 392, 14, 0.45) + reson(x, 610, 12, 0.3) + lp(x, 1800) * 0.25)
    amp = env([(0, 0), (0.25, 0.6), (1.0, 1.0), (4.8, 0.85), (6.4, 0.5), (dur, 0.0)], dur)
    amp *= 1 + 0.25 * slow_noise(n, 9, rng)
    return body * amp


def metal_layer(dur, rng):
    """Steel dragged on steel: inharmonic partials in beating pairs, chattering.
    A shriek on the attack, then a thin keening that droops and outlasts the voice."""
    n = n_of(dur)
    t = np.arange(n) / SR
    glide = env([(0, 0.80), (0.5, 1.0), (1.3, 1.07), (4.5, 1.0), (6.0, 0.86), (dur, 0.70)], dur)
    out = np.zeros(n)
    base = 1090.0
    for i, r in enumerate([1.0, 1.47, 1.93, 2.56, 3.31, 4.12, 5.2]):
        drift = 1 + 0.012 * slow_noise(n, 3 + i, rng)
        f = base * r * glide * drift
        a = (1.0 / (1 + 0.55 * i))
        out += a * (osc(f) + 0.8 * osc(f * 1.0043, phase0=rng.random()))
    chatter = np.clip(lp(white(n, rng), 70) * 9, -1, 1) ** 2            # stick-slip of the metal
    scrape = bp(white(n, rng), 2200, 7500) * 0.5
    x = out * (0.35 + 0.65 * chatter) + scrape * chatter
    shriek = env([(0, 0), (0.25, 0.1), (0.75, 1.0), (1.5, 0.75), (2.4, 0.3), (4.6, 0.22),
                  (5.4, 0.55), (6.6, 0.42), (8.0, 0.2), (dur, 0.0)], dur)
    # the keening: one narrow, failing whine riding over the end
    kf = env([(0, 930), (4.4, 930), (5.3, 985), (7.0, 820), (dur, 640)], dur)
    kf *= 1 + 0.02 * np.sin(2 * np.pi * 5.1 * t) + 0.015 * slow_noise(n, 6, rng)
    keen = (osc(kf) + 0.5 * osc(kf * 2.01) + 0.25 * osc(kf * 3.03))
    keen *= env([(0, 0), (4.2, 0), (5.2, 0.5), (6.8, 0.6), (8.4, 0.32), (dur, 0.0)], dur)
    keen *= 0.75 + 0.25 * slow_noise(n, 14, rng)
    return x * shriek + keen * 0.55


def sub_layer(dur, rng):
    n = n_of(dur)
    t = np.arange(n) / SR
    f = env([(0, 44), (0.35, 33), (2.0, 31), (5.5, 28), (dur, 23)], dur)
    s = osc(f) + 0.4 * osc(f * 2) + 0.15 * osc(f * 3)
    amp = env([(0, 0), (0.12, 1.0), (0.5, 0.7), (1.2, 1.0), (4.8, 0.9), (6.5, 0.55), (dur, 0.0)], dur)
    amp *= 1 - env([(0, 0), (5.5, 0), (7.0, 0.5), (dur, 0.7)], dur) * (0.5 + 0.5 * np.sin(2 * np.pi * 5.2 * t))
    return s * amp


def air_layer(dur, rng):
    """Breath through a huge wet throat, and the shuddering exhale at the end."""
    n = n_of(dur)
    x = bp(pink(n, rng), 140, 1900) + 0.5 * bp(white(n, rng), 1500, 5200)
    amp = env([(0, 0.25), (0.3, 0.5), (1.0, 1.0), (4.8, 0.8), (6.2, 0.6), (7.6, 0.75), (dur, 0.0)], dur)
    flut = 0.6 + 0.4 * np.clip(lp(white(n, rng), 22) * 6, -1, 1)
    shud = 1 - env([(0, 0), (6.0, 0), (7.2, 0.7), (dur, 0.9)], dur) * (0.5 + 0.5 * osc(np.full(n, 6.4)))
    return x * amp * flut * shud


def make_roar_dry(rng):
    voice = varispeed(animal_voice(rng), 3.1)             # about 9.0 s
    dur = len(voice) / SR
    # tape slow-down also dulls the top; give some of it back
    voice = voice + 0.9 * hp(voice, 900)
    fr = friction_layer(dur, rng)[: len(voice)]
    me = metal_layer(dur, rng)[: len(voice)]
    su = sub_layer(dur, rng)[: len(voice)]
    ai = air_layer(dur, rng)[: len(voice)]
    # The voice leads: the other layers swell and break with it, so the cracks
    # and the long sag are heard and the roar is not one flat wall. The metal
    # keeps its own shape (its keening outlasts the voice).
    venv = lp(np.abs(norm(voice, 1.0)), 5)
    venv = np.clip(venv / (np.max(venv) + 1e-12), 0, 1) ** 0.6
    lead = 0.25 + 0.75 * venv
    x = (norm(voice, 1.0) * 1.0 + norm(fr, 1.0) * 0.6 * lead + norm(me, 1.0) * 0.9 * (0.55 + 0.45 * venv) +
         norm(su, 1.0) * 0.36 * lead + norm(ai, 1.0) * 0.32 * (0.4 + 0.6 * venv))
    x = soft(x * 0.9, 1.4)                                 # one throat, not five layers
    x = hp(x, 24) + 0.1 * lp(x, 110)                       # chest, but not so much that small speakers go silent
    return fade(x, 0.01, 0.25)


def make_roar(rng):
    dry = make_roar_dry(rng)
    out = reverb(dry, rt60=2.6, wet=0.2, rng=rng, predelay=0.07, lp_fc=4200,
                 taps=[(0.62, 0.16, 1800)])
    return fade(norm(out), 0.01, 0.6)


def make_roar_far(rng):
    """The same roar two or three kilometres off: the air has eaten the top, the
    valley answers, and what arrives is mostly the long low body of it."""
    dry = make_roar_dry(rng)
    x = lp(dry, 900, 2) * 0.9 + bp(dry, 70, 320, 2) * 0.8 + bp(dry, 800, 1500) * 0.15
    out = reverb(x, rt60=5.5, wet=0.5, rng=rng, predelay=0.12, lp_fc=1400, hp_fc=35,
                 taps=[(1.05, 0.42, 700), (2.3, 0.26, 450), (3.9, 0.14, 300)])
    return fade(norm(out), 0.3, 1.5)


# ---------------------------------------------------------------------------
# footfall
# ---------------------------------------------------------------------------
def make_step(rng, far=False):
    dur = 3.4
    n = n_of(dur)
    t = np.arange(n) / SR
    # the blow: a pitch drop from 70 to 27 Hz
    f = 27 + 43 * np.exp(-t / 0.07)
    thump = osc(f) * np.exp(-t / 0.3) * (1 - np.exp(-t / 0.004))
    thump += 0.15 * osc(f * 0.5) * np.exp(-t / 0.7)
    # harmonics, so the blow still reads on speakers that cannot reach 30 Hz
    thump += 1.0 * osc(f * 2) * np.exp(-t / 0.3) + 0.9 * osc(f * 3) * np.exp(-t / 0.22) + 0.6 * osc(f * 5) * np.exp(-t / 0.12)
    # the slap of the sole and what breaks under it
    slap = lp(white(n, rng), 420) * np.exp(-t / 0.07) * 2.0
    # the ground as a drumhead: two low knocks rung by the slap
    slap = slap + (reson(slap, 96, 5, 3.0) + reson(slap, 152, 6, 2.2))
    crunch = bp(white(n, rng), 300, 2600) * np.exp(-t / 0.16) * (0.5 + 0.5 * np.clip(lp(white(n, rng), 40) * 8, -1, 1) ** 2)
    # the late rumble: the ground still moving after the foot has stopped
    rum = bp(brown(n, rng), 26, 75, 2) + 0.8 * bp(pink(n, rng), 70, 190, 2)
    rum_env = env([(0, 0), (0.5, 0.04), (1.0, 1.0), (1.7, 0.75), (2.7, 0.25), (dur, 0.0)], dur)
    rum = norm(rum, 1.0) * rum_env * (0.7 + 0.3 * slow_noise(n, 7, rng))
    # loose things settling
    k = rng.random(n) < (40 / SR)
    grit = bp(k * rng.uniform(0.3, 1.0, n), 900, 4200) * env([(0, 0), (0.25, 1.0), (1.2, 0.5), (2.6, 0.0), (dur, 0)], dur) * 6
    if far:
        x = thump * 1.0 + rum * 0.9 + slap * 0.15
        x = lp(x, 230, 2)
        x = reverb(x, rt60=2.8, wet=0.4, rng=rng, predelay=0.1, lp_fc=400, hp_fc=20, taps=[(0.8, 0.25, 120)])
        return fade(norm(soft(x, 1.3)), 0.002, 0.8)
    x = thump * 0.8 + slap * 1.0 + crunch * 0.8 + rum * 1.0 + grit * 0.25
    x = soft(x, 1.6)
    x = reverb(x, rt60=1.3, wet=0.14, rng=rng, predelay=0.04, lp_fc=2500, hp_fc=20)
    return fade(norm(x), 0.001, 0.5)


# ---------------------------------------------------------------------------
# furnace drone (loop)
# ---------------------------------------------------------------------------
def make_furnace(rng):
    dur = 8.0
    xf = 1.5
    n = n_of(dur + xf)
    t = np.arange(n) / SR
    drone = np.zeros(n)
    for (f, a, det) in [(36.7, 0.45, 0.13), (55.0, 0.7, 0.21), (73.4, 0.7, 0.17), (110.3, 0.45, 0.3), (146.8, 0.25, 0.4)]:
        drone += a * (osc(np.full(n, f), 'saw') + osc(np.full(n, f + det), 'saw', 0.3) + osc(np.full(n, f - det * 0.7), 'tri', 0.6))
    drone = lp(drone, 460, 2)
    # fire: a roar of air through the ribs, fluttering
    fire = bp(brown(n, rng), 90, 950) * 1.0 + bp(pink(n, rng), 600, 2600) * 0.2
    fl = 0.55 + 0.45 * np.clip(lp(white(n, rng), 9) * 5, -1, 1)
    fire *= fl
    # the breath: the whole thing swells about once every four seconds (period divides the loop)
    swell = 0.72 + 0.28 * np.sin(2 * np.pi * t / 4.0 - 1.2)
    # crackle: sparse sharp ticks, like fat in a fire
    ticks = (rng.random(n) < (9 / SR)) * rng.uniform(0.2, 1.0, n)
    crack = hp(signal.lfilter([1], [1, -0.93], ticks), 1500) * 0.5
    # something wet working inside
    gur = reson(white(n, rng), 180, 4) * np.clip(lp(white(n, rng), 3.5) * 5, 0, 1) ** 2 * 0.25
    x = (norm(drone, 1.0) * 0.7 + norm(fire, 1.0) * 0.8) * swell + crack * 0.25 + gur
    x = soft(x, 1.4)
    return norm(loopable(x, xf), 0.8)


# ---------------------------------------------------------------------------
# beam: charge whine, the beam, loop body
# ---------------------------------------------------------------------------
def make_beam_charge(rng):
    dur = 4.2
    n = n_of(dur)
    t = np.arange(n) / SR
    u = t / dur
    f = 190 * (3600 / 190) ** (u ** 1.25)                       # rising whine, accelerating
    whine = osc(f) + 0.5 * osc(f * 1.5, 'saw') * 0.4 + 0.35 * osc(f * 2.01) + 0.2 * osc(f * 0.5)
    whine = lp(whine, 9000)
    # the ribs: a tremolo that speeds up from a throb to a buzz
    tr = 3 * (38 / 3) ** u
    throb = 0.55 + 0.45 * osc(tr)
    # air drawn in: a band of noise sweeping upward
    air = np.zeros(n)
    blk = n_of(0.05)
    nz = white(n + blk, rng)
    for i in range(0, n, blk):
        fc = 250 * (6000 / 250) ** ((i / n) ** 1.1)
        seg = bp(nz[i: i + blk * 2], fc * 0.7, min(fc * 1.4, 18000))[:blk]
        air[i: i + len(seg[: n - i])] = seg[: n - i]
    sub = osc(np.full(n, 34.0)) * u ** 2
    amp = u ** 1.6
    x = whine * amp * throb * 0.8 + air * amp * 0.55 + sub * 0.22
    x = soft(x, 1.5)
    # ends on a hard edge: the beam's first crack lands on top of it
    x[-n_of(0.012):] *= np.linspace(1, 0, n_of(0.012))
    return norm(x)


def beam_body(dur, rng):
    n = n_of(dur)
    t = np.arange(n) / SR
    # a fused, harsh tone: FM with a noisy modulator, never still
    mod = osc(np.full(n, 137.0)) * 620 + lp(white(n, rng), 300) * 900
    car = osc(1870 + mod) + 0.6 * osc(2805 + mod * 1.4) + 0.4 * osc(935 + mod * 0.5)
    arc = hp(white(n, rng), 3000) * (0.5 + 0.5 * np.sign(osc(np.full(n, 100.0))))   # mains-rate arcing
    hum = osc(np.full(n, 50.0), 'saw') * 0.5 + osc(np.full(n, 100.0), 'saw') * 0.3
    hum = lp(hum, 700)
    sub = osc(np.full(n, 37.0))
    wob = 0.8 + 0.2 * slow_noise(n, 13, rng)
    return (car * 0.5 + arc * 0.3 + hum * 0.5 + sub * 0.22) * wob


def make_beam(rng):
    dur = 5.5
    n = n_of(dur)
    t = np.arange(n) / SR
    body = beam_body(dur, rng)
    crack = white(n, rng) * np.exp(-t / 0.035) * 2.2 + osc(60 + 300 * np.exp(-t / 0.05)) * np.exp(-t / 0.25) * 0.8
    amp = env([(0, 1.0), (4.4, 1.0), (4.9, 0.55), (dur, 0.0)], dur)
    # as it dies the tone falls away
    x = soft((body + crack) * amp, 1.7)
    x = reverb(x, rt60=1.6, wet=0.12, rng=rng, predelay=0.05, lp_fc=5000)
    return fade(norm(x), 0.0005, 0.4)


def make_beam_loop(rng):
    x = soft(beam_body(3.0 + 0.6, rng), 1.7)
    return norm(loopable(x, 0.6), 0.85)


# ---------------------------------------------------------------------------
# the spent exhale
# ---------------------------------------------------------------------------
def make_exhale(rng):
    dur = 6.5
    n = n_of(dur)
    t = np.arange(n) / SR
    u = t / dur
    # breath: noise through a throat that closes as the air runs out
    nz = pink(n, rng)
    b1 = bp(nz, 260, 900) * env([(0, 1.0), (3.0, 0.8), (dur, 0.5)], dur)
    b2 = bp(white(n, rng), 900, 3000) * env([(0, 0.7), (2.0, 0.4), (dur, 0.1)], dur)
    breath = b1 + b2 * 0.6
    # wheeze: narrow whistles with no steady pitch
    wz = np.zeros(n)
    for (f0, f1, a) in [(1240, 780, 1.0), (1710, 1150, 0.6), (860, 610, 0.7)]:
        f = f0 * (f1 / f0) ** u * (1 + 0.03 * slow_noise(n, 5, rng) + 0.012 * np.sin(2 * np.pi * 9 * t))
        wz += a * osc(f) * (0.4 + 0.6 * np.clip(slow_noise(n, 2.5, rng) + 0.3, 0, 1))
    # rattle: something loose in the chest, flapping at an uneven rate
    rate = 21 + 6 * slow_noise(n, 1.2, rng) - 8 * u
    gate = (osc(rate) > 0.55).astype(float)
    gate = lp(gate, 180)
    rattle = bp(white(n, rng), 120, 700) * gate
    # a low moan under it, sinking
    moan = osc(52 * (36 / 52) ** u * (1 + 0.02 * slow_noise(n, 3, rng))) * 0.6 + osc(104 * (70 / 104) ** u) * 0.2
    amp = env([(0, 0), (0.25, 1.0), (1.2, 0.85), (3.5, 0.55), (5.2, 0.28), (dur, 0.0)], dur)
    # the breath comes out in failing pushes
    push = 0.65 + 0.35 * np.sin(2 * np.pi * (0.9 * t + 0.25 * t ** 2 / dur))
    x = (breath * 0.9 + wz * 0.16 + rattle * 0.55 + moan * 0.14) * amp * push
    # embers: cooling ticks at the very end
    ticks = (rng.random(n) < (7 / SR)) * rng.uniform(0.3, 1.0, n) * env([(0, 0), (2.5, 0.2), (5.0, 1.0), (dur, 0.8)], dur)
    x += hp(signal.lfilter([1], [1, -0.9], ticks), 2000) * 0.25
    x = soft(x, 1.3)
    x = reverb(x, rt60=1.8, wet=0.15, rng=rng, predelay=0.05, lp_fc=3500)
    return fade(norm(x), 0.01, 0.6)


# ---------------------------------------------------------------------------
# presence: radio static, birds going up
# ---------------------------------------------------------------------------
def make_static(rng):
    dur = 2.8
    n = n_of(dur)
    t = np.arange(n) / SR
    hiss = bp(white(n, rng), 320, 3300, 3)
    # crackle that thickens as he gets nearer: sparse pops into a tearing sound
    dens = env([(0, 40), (1.0, 300), (2.0, 900), (dur, 200)], dur) / SR
    pops = (rng.random(n) < dens) * rng.uniform(-1, 1, n)
    crackle = bp(signal.lfilter([1], [1, -0.7], pops), 500, 3300) * 3
    # a carrier whistle dragged off tune (heterodyne), and a slow 2 Hz throb: his step in the set
    wf = 1450 + 500 * slow_noise(n, 1.5, rng)
    whistle = osc(wf) * (0.3 + 0.3 * slow_noise(n, 4, rng))
    throb = 0.55 + 0.45 * np.sin(2 * np.pi * 1.9 * t) ** 2
    amp = env([(0, 0), (0.03, 0.7), (1.8, 1.0), (2.5, 0.8), (2.56, 0.0), (dur, 0.0)], dur)   # squelch closes
    x = (hiss * 0.8 + crackle * 0.7 + whistle * 0.18) * amp * throb
    click = np.zeros(n)
    click[n_of(2.56): n_of(2.56) + 60] = np.linspace(1, 0, 60)
    x = bp(soft(x + click * 2, 2.0), 300, 3400, 2)             # a small speaker
    return fade(norm(x, 0.8), 0.001, 0.05)


def make_birds(rng):
    """A flock going up in alarm: many short falling chirps and a rush of wings."""
    dur = 5.0
    n = n_of(dur)
    x = np.zeros(n)
    count = 150
    starts = np.sort(rng.beta(1.3, 3.0, count) * (dur - 0.4))
    for s in starts:
        d = rng.uniform(0.04, 0.13)
        m = n_of(d)
        tt = np.arange(m) / SR
        f0 = rng.uniform(2600, 5200)
        f1 = f0 * rng.uniform(0.55, 0.9)
        f = f0 * (f1 / f0) ** (tt / d) * (1 + 0.04 * np.sin(2 * np.pi * rng.uniform(30, 70) * tt))
        c = osc(f) * np.sin(np.pi * tt / d) ** 2
        c += 0.3 * osc(f * 2) * np.sin(np.pi * tt / d) ** 2
        far = rng.uniform(0.15, 1.0)                 # some birds are further off
        i = n_of(s)
        x[i: i + m] += c[: n - i] * far
        if rng.random() < 0.45:                      # a doubled call
            j = i + m + n_of(rng.uniform(0.01, 0.05))
            if j + m < n:
                x[j: j + m] += c * far * 0.7
    # wings: bursts of soft noise at wingbeat rate, many birds out of step
    wing = np.zeros(n)
    for _ in range(14):
        rate = rng.uniform(9, 15)
        ph = rng.random()
        beat = np.clip(osc(np.full(n, rate), 'sin', ph), 0, 1) ** 3
        st = rng.uniform(0.0, 1.2)
        e = env([(0, 0), (st, 0), (st + 0.3, 1.0), (st + 2.2, 0.4), (dur, 0.0)], dur)
        wing += bp(white(n, rng), 500, 2800) * beat * e * rng.uniform(0.4, 1.0)
    out = x * 0.5 + norm(wing, 1.0) * 0.5
    out *= env([(0, 0.6), (0.4, 1.0), (2.5, 0.6), (dur, 0.0)], dur)       # they leave
    out = reverb(out, rt60=1.2, wet=0.12, rng=rng, predelay=0.03, lp_fc=6000, hp_fc=300)
    return fade(norm(out, 0.8), 0.005, 0.5)


# ---------------------------------------------------------------------------
# table of sounds: name -> (maker, game sample rate, seed)
# ---------------------------------------------------------------------------
SOUNDS = {
    'roar':        (make_roar, 44100, 1985),
    'roar_far':    (make_roar_far, 22050, 1985),     # same seed: the same roar, heard from far off
    'step':        (lambda r: make_step(r, False), 22050, 610),
    'step_far':    (lambda r: make_step(r, True), 22050, 610),
    'furnace':     (make_furnace, 22050, 77),
    'beam_charge': (make_beam_charge, 44100, 404),
    'beam':        (make_beam, 44100, 405),
    'beam_loop':   (make_beam_loop, 44100, 405),
    'exhale':      (make_exhale, 44100, 12),
    'static':      (make_static, 22050, 9),
    'birds':       (make_birds, 44100, 31),
}

BANDS = [(0, 60), (60, 250), (250, 1000), (1000, 4000), (4000, 11000)]


def measure(x, sr):
    spec = np.abs(np.fft.rfft(x)) ** 2
    f = np.fft.rfftfreq(len(x), 1 / sr)
    tot = np.sum(spec) + 1e-20
    bands = {f"{lo}-{hi}Hz": round(float(np.sum(spec[(f >= lo) & (f < hi)]) / tot), 4) for lo, hi in BANDS}
    # loudness envelope in 0.25 s steps (dB below the loudest step)
    k = int(sr * 0.25)
    rms = np.array([np.sqrt(np.mean(x[i: i + k] ** 2)) for i in range(0, len(x) - k + 1, k)])
    env_db = np.round(20 * np.log10(rms / (rms.max() + 1e-12) + 1e-6), 1).tolist()
    return {
        'seconds': round(len(x) / sr, 3),
        'sample_rate': sr,
        'peak': round(float(np.max(np.abs(x))), 4),
        'rms_db': round(float(20 * np.log10(np.sqrt(np.mean(x ** 2)) + 1e-12)), 2),
        'centroid_hz': round(float(np.sum(f * spec) / tot), 1),
        'band_energy': bands,
        'envelope_db_per_250ms': env_db,
    }


def write_wss(path, pcm16, sr):
    """Cold War Assault .wss, uncompressed: "WSS0", a delta-pack byte (0 = plain
    PCM), three reserved bytes, an 18 byte WAVEFORMATEX, then the samples
    (Audio/Streaming/WaveLoaders.cpp:22-63, Audio/Core/Format/RiffFile.hpp:27-35)."""
    import struct
    head = b'WSS0' + bytes([0, 0, 0, 0])
    fmt = struct.pack('<HHIIHHH', 1, 1, sr, sr * 2, 2, 16, 0)
    with open(path, 'wb') as fh:
        fh.write(head + fmt + pcm16.astype('<i2').tobytes())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    ap.add_argument('--only', default='')
    a = ap.parse_args()
    wav_dir = os.path.join(a.out, 'wav')
    os.makedirs(wav_dir, exist_ok=True)
    only = [s for s in a.only.split(',') if s]
    report = {}
    rep_path = os.path.join(a.out, 'synth.json')
    if only and os.path.exists(rep_path):
        with open(rep_path) as fh:
            report = json.load(fh)
    for name, (maker, sr, seed) in SOUNDS.items():
        if only and name not in only:
            continue
        x = maker(np.random.default_rng(seed))
        if not np.all(np.isfinite(x)):
            sys.exit(f"{name}: non-finite samples")
        if sr != SR:
            x = signal.resample_poly(x, sr, SR)
        x = np.clip(x, -0.98, 0.98)
        sf.write(os.path.join(wav_dir, name + '.wav'), x, sr, subtype='PCM_16')
        report[name] = measure(x, sr)
        print(f"{name:12s} {report[name]['seconds']:6.2f} s  {sr} Hz  peak {report[name]['peak']:.2f}  "
              f"rms {report[name]['rms_db']:6.1f} dB  centroid {report[name]['centroid_hz']:7.1f} Hz")
    with open(rep_path, 'w') as fh:
        json.dump(report, fh, indent=1)
    # the game's own format: every master as a .wss (see write_wss)
    snd_dir = os.path.join(a.out, 'snd')
    os.makedirs(snd_dir, exist_ok=True)
    for name in SOUNDS:
        src = os.path.join(wav_dir, name + '.wav')
        if os.path.exists(src):
            data, sr = sf.read(src, dtype='int16')
            write_wss(os.path.join(snd_dir, name + '.wss'), data, sr)


if __name__ == '__main__':
    main()
