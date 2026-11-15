r"""Dedushka's sounds, second set: built from the owner's VST instruments.

    python make_sounds.py [--only roar,step] [--roar-take a|b|c|d] [--force-fx] [--no-pack]

Three stages:
  1. stems.py (run it first) has rendered "strips" out of the instruments in
     headless REAPER: factory presets of Arturia CMI V, Augmented VOICES /
     BRASS / WOODWINDS / STRINGS, Synthi V, ARP 2600 V3, KORG MS-20 V, CS-80 V4,
     Modular V3, and Chromaphone 3 patches programmed in stems.py.
  2. Here every sound is assembled from notes cut out of those strips: slowed
     and bent like tape (pitch and formants together), shaped, layered. The
     layered sound is then sent back through headless REAPER for the effects
     (Arturia Dist COLDFIRE, AAS Objeq Delay as a resonating plate or drum,
     Native Instruments Freak ring modulator and Raum reverb, Arturia Rev
     INTENSITY), each effect coming back on its own so it can be blended.
  3. Finish: sub-bass and the open-air echoes (numpy, the same reverb the
     first set used), level, 16 bit mono wav + .wss, measurements, pictures
     against the first set, mp3 previews, the pbo.

Nothing is downloaded and no sample pack is used: the only sources are the
instruments' own factory content and the numpy sub and echoes.

Output under <notes>\build (wav, snd, report.json, the pbo), <notes>\previews.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys

import numpy as np
from scipy import signal
import soundfile as sf

import sndlib as S
from fxpass import run_chains
from rjob import NOTES
from strips import note

SR = S.SR
HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(NOTES, 'build')
PREV = os.path.join(NOTES, 'previews')
FIRST = r'D:\Arma_CWA_worktrees\kaiju-notes\features\roar\build'
TOOLS = r'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe'
FFMPEG = r'C:\ffmpeg\bin\ffmpeg.exe'
MOD = os.path.normpath(os.path.join(HERE, r'..\..\..\..\..\tmp\@kj_roar_vst'))
FORCE_FX = False
INFO = {}          # per sound: what the build learned (effect fold-down losses and so on)


# ---------------------------------------------------------------------------
# building blocks
# ---------------------------------------------------------------------------
def lay(src, start=0.0, st=None, env=None, gain=0.0, level=-20.0, hp=None, lp=None, skip=0.0, take=None):
    """One layer: a stretch of a strip note, bent like tape, shaped, placed.
    st: semitones (a number, or (time, semitones) points over the layer's own
    time); env: (time, gain) points, the layer ends where the last point is 0."""
    x = src[int(skip * SR):]
    if take:
        x = x[: int(take * SR)]
    if st is not None:
        if np.isscalar(st):
            x = S.tape(x, 2.0 ** (st / 12.0))
        else:
            x = S.tape(x, S.speed_curve(st, st[-1][0]))
    if hp:
        x = S.hp(x, hp)
    if lp:
        x = S.lp(x, lp)
    if env:
        e = S.env(env, len(x) / SR)
        n = min(len(e), len(x))
        x = x[:n] * e[:n]
        if env[-1][1] == 0:
            x = x[: int(env[-1][0] * SR)]
    x = S.rms_norm(x, level + gain)
    return S.pad(x, before=start)


def shudder(n, t0, t1, f0, f1, depth):
    """A tremor that sets in between t0 and t1: the loudness shakes, slower
    and slower (f0 to f1 Hz). The sob at the end of a cry."""
    t = np.arange(n) / SR
    k = np.clip((t - t0) / (t1 - t0), 0, 1)
    f = f0 + (f1 - f0) * k
    ph = 2 * np.pi * np.cumsum(f) / SR
    return 1 - depth * k * (0.5 + 0.5 * np.sin(ph))


def voice_break(n, at, depth=0.8, width=0.2):
    """The voice cracks: the loudness drops out for a tenth of a second."""
    t = np.arange(n) / SR
    return 1 - depth * np.exp(-0.5 * ((t - at - width / 2) / (width / 2.5)) ** 2)


def follower(x, fc=5.0):
    e = S.lp(np.abs(x), fc, order=2)
    e = np.clip(e, 0, None)
    return e / (e.max() + 1e-12)


def sub_layer(freq_pts, dur, like, level):
    """Sub-bass you feel: a sine with a little second and third harmonic (so
    small speakers still hint at it), riding the loudness of `like`."""
    f = S.env(freq_pts, dur)
    ph = 2 * np.pi * np.cumsum(f) / SR
    s = np.sin(ph) + 0.28 * np.sin(2 * ph + 0.4) + 0.10 * np.sin(3 * ph + 1.1)
    e = follower(like, 3.0)
    n = min(len(s), len(e))
    return S.rms_norm(s[:n] * e[:n], level)


def fx(tag, x, sends, tail=4.0, dry=1.0):
    """Sends x through effect chains in headless REAPER (fxpass.py) and mixes
    the returns under the dry sound. sends: {chain name: dB relative to the
    dry level}. Each return is folded to mono; the loss is recorded."""
    h = hashlib.md5(np.round(x * 32767).astype(np.int16).tobytes()).hexdigest()[:8]
    name = f'{tag}_{h}'
    os.makedirs(os.path.join(NOTES, 'renders', 'fx'), exist_ok=True)
    src = os.path.join(NOTES, 'renders', 'fx', f'in_{name}.wav')
    peak = np.max(np.abs(x)) + 1e-12
    sf.write(src, x / peak * 0.5, SR, subtype='PCM_24')
    outs = run_chains(name, src, list(sends), tail=tail, force=FORCE_FX)
    ref = 20 * np.log10(np.sqrt(np.mean(x ** 2)) + 1e-12)
    parts = [x * dry]
    info = INFO.setdefault(tag, {})
    for chain, spec in sends.items():
        db, hpf, lpf = spec if isinstance(spec, tuple) else (spec, None, None)
        if not os.path.exists(outs[chain]):
            raise SystemExit(f'{tag}: REAPER did not render the {chain} chain')
        y, loss = S.load_mono(outs[chain])
        if hpf:
            y = S.hp(y, hpf)
        if lpf:
            y = S.lp(y, lpf)
        if np.max(np.abs(y)) < 1e-6:
            print(f'    [{tag}] chain {chain} came back silent; left out')
            info[chain] = 'silent'
            continue
        body = y[: len(x)]
        g = 10 ** ((ref + db) / 20) / (np.sqrt(np.mean(body ** 2)) + 1e-12)
        parts.append(y * g)
        info[chain] = {'send_db': db, 'highpass_hz': hpf, 'lowpass_hz': lpf, 'mono_fold_loss_db': round(loss, 1)}
    return S.mix(*parts)


def outdoors(x, rng, rt60=3.2, wet=0.22, taps=((1.05, 0.28, 1500), (2.3, 0.19, 950), (3.9, 0.12, 620)), lp_fc=3000):
    """The island answering: an open-air tail and three slap echoes off the
    hills, each later, quieter and duller (the first set's reverb)."""
    return S.reverb(x, rt60, wet, rng, predelay=0.05, lp_fc=lp_fc, hp_fc=70, taps=taps)


def tidy(x, t_in=0.004, t_out=0.05, db=-62):
    x = x - np.mean(x)
    x = S.hp(x, 16)
    x = S.trim_silence(x, db=db, keep=0.03)
    return S.fade(x, t_in, t_out)


def first_set(name):
    p = os.path.join(FIRST, 'wav', name + '.wav')
    x, sr = sf.read(p)
    return x, sr


# ---------------------------------------------------------------------------
# the roar: three takes (and a fourth that keeps the first set's voice)
# ---------------------------------------------------------------------------
def roar_bus_a():
    """Take A, "the howl that breaks". A scream goes up, a long howl takes
    over, cracks once, and sags and shakes to nothing, over a low bellow."""
    L = [
        # the scream that opens it: an elephant's trumpet, slowed, sliding up into the note and falling off it
        lay(note('cmi_elephant', 48), 0.0, st=[(0, -14), (0.5, -8), (1.0, -7), (2.4, -7.5), (3.3, -13)],
            env=[(0, 0), (0.12, 1), (2.3, 0.85), (3.3, 0)], gain=-1),
        # the almost-human part: a woman's sung "ah" (Augmented VOICES), slowed until the throat is huge;
        # her voice breaks at 3.4 s and then sinks an octave. A second voice a semitone off grinds against it.
        lay(note('vox_ah_hi', 70), 0.6, st=[(0, -10), (0.6, -5), (2.6, -5.5), (2.68, -9), (2.85, -5.5), (4.5, -7),
                                            (6.0, -12), (7.0, -17)], skip=0.6,
            env=[(0, 0), (0.5, 1), (4.6, 0.9), (7.0, 0)], gain=-1),
        lay(note('vox_ah_hi', 75), 1.0, st=[(0, -13), (0.8, -9), (3.0, -9.5), (5.0, -11), (6.5, -16)], skip=0.8,
            env=[(0, 0), (0.6, 1), (4.2, 0.8), (6.5, 0)], gain=-6),
        # the wail: a wolf's howl, slowed; it breaks at the same moment, then droops
        lay(note('cmi_wolf', 43), 0.9, st=[(0, -8), (0.7, -5), (2.5, -5.5), (2.58, -10), (2.75, -5.8), (4.0, -7),
                                          (5.6, -12), (6.6, -17)],
            env=[(0, 0), (0.6, 1), (4.6, 0.9), (6.6, 0)], gain=0),
        # the bellow under it: a cow's low, slowed to a growl you feel
        lay(note('cmi_cow', 36), 0.15, st=[(0, -1), (1.0, 0), (4.5, -1), (7.6, -5)],
            env=[(0, 0), (0.5, 1), (5.5, 0.8), (7.6, 0)], gain=2, hp=45),
        # two more voices an octave down, drifting apart
        lay(note('vox_ah', 53), 1.2, st=[(0, -14), (1.2, -12), (4.5, -12.5), (7.0, -17)], skip=1.0,
            env=[(0, 0), (0.8, 1), (4.5, 0.8), (7.0, 0)], gain=-6),
        lay(note('vox_ah', 54), 1.3, st=[(0, -13), (1.5, -11), (4.5, -12), (7.0, -15)], skip=1.2,
            env=[(0, 0), (0.8, 1), (4.5, 0.8), (7.0, 0)], gain=-7),
        # brass: a low growling horn and a sub-heavy trombone under everything, a flutter-tongued burst at the start
        lay(note('brs_growl', 31), 0.1, st=[(0, -5), (1, -3), (5, -4), (8, -9)],
            env=[(0, 0), (0.6, 1), (5.5, 0.8), (8.0, 0)], gain=0),
        lay(note('brs_beast', 29), 0.2, st=-2, skip=5.0, env=[(0, 0), (0.8, 1), (5.5, 0.8), (8.2, 0)], gain=-3),
        lay(note('brs_aven', 36), 0.0, st=-5, env=[(0, 1), (1.6, 1), (2.6, 0)], gain=-2),
        # woodwind torn into flutter and noise: the rasp on top of the wail
        lay(note('wnd_beast', 55), 2.0, st=[(0, -7), (1, -5), (3.5, -6), (5, -10)], skip=4.0,
            env=[(0, 0), (0.8, 1), (3.5, 0.8), (5.0, 0)], gain=-6),
        # metal: bowed metal strings, a bowed plate, and one thin squeal that sags and outlasts the voice
        lay(note('str_metal', 60), 0.4, st=[(0, -3), (6, -8)], skip=1.0, hp=500,
            env=[(0, 0), (0.6, 1), (4.0, 0.7), (7.0, 0)], gain=-12),
        lay(note('chr_scrape', 31), 0.3, st=-7, env=[(0, 0), (1.0, 1), (5.5, 0.7), (7.5, 0)], gain=-13),
        lay(note('syn_windmill', 60), 1.5, st=[(0, -5), (3, -6), (6, -9), (9, -14)], skip=1.0, hp=800,
            env=[(0, 0), (1.5, 1), (5.0, 0.8), (8.5, 0)], gain=-17),
    ]
    return S.mix(*L)


def roar_bus_b():
    """Take B, "the bellow": a metal shriek, a long wail, and a low bellow to
    finish (the three-part shape the 2014 film used)."""
    L = [
        lay(note('cmi_screech', 60), 0.0, st=[(0, -6), (0.3, 2), (1.2, 0), (1.9, -5)],
            env=[(0, 0), (0.05, 1), (1.2, 0.8), (1.9, 0)], gain=0),
        lay(note('cmi_scream1', 55), 0.05, st=[(0, -8), (0.4, -1), (1.6, -2), (2.6, -7)],
            env=[(0, 0), (0.1, 1), (1.7, 0.8), (2.6, 0)], gain=-1),
        lay(note('str_metal', 60), 0.0, st=-2, skip=1.0, env=[(0, 0), (0.1, 1), (1.5, 0.6), (2.5, 0)], gain=-6),
        lay(note('vox_ah_hi', 75), 0.0, st=[(0, -6), (0.3, 0), (1.2, -2), (1.9, -8)], skip=0.5,
            env=[(0, 0), (0.06, 1), (1.2, 0.8), (1.9, 0)], gain=-3),
        # and a sung voice riding the wail, slowed
        lay(note('vox_massive_hi', 65), 1.2, st=[(0, -8), (1, -6), (3.5, -7), (4.6, -11)], skip=0.8,
            env=[(0, 0), (0.5, 1), (3.6, 0.9), (4.6, 0)], gain=-4),
        # the wail: the elephant again, much slower, with a distorted solo brass on the same line
        lay(note('cmi_elephant', 36), 1.1, st=[(0, -2), (1, 0), (3.5, -1), (4.6, -5)],
            env=[(0, 0), (0.5, 1), (3.6, 0.9), (4.6, 0)], gain=1),
        lay(note('brs_dreary', 36), 1.0, st=[(0, -3), (1, -1), (3.5, -2), (5, -7)], skip=0.5,
            env=[(0, 0), (0.5, 1), (3.8, 0.8), (5.0, 0)], gain=-2),
        lay(note('wnd_beast', 48), 2.0, st=-4, skip=4.0, env=[(0, 0), (0.6, 1), (2.5, 0.8), (3.6, 0)], gain=-6),
        # the bellow that ends it
        lay(note('cmi_cow', 31), 4.6, st=[(0, 2), (1, 0), (4.4, -6)],
            env=[(0, 0), (0.4, 1), (2.5, 0.9), (4.4, 0)], gain=1),
        lay(note('brs_beast', 29), 4.4, st=[(0, 0), (4.6, -5)], skip=6.0,
            env=[(0, 0), (0.6, 1), (3.0, 0.8), (4.6, 0)], gain=-1),
        lay(note('cmi_multidog', 29), 5.2, st=-3, env=[(0, 0), (0.1, 1), (2.5, 0.7), (3.8, 0)], gain=-4),
        lay(note('brs_growl', 31), 0.3, st=-5, env=[(0, 0), (1.0, 1), (7.0, 0.8), (9.0, 0)], gain=-5),
    ]
    return S.mix(*L)


def roar_bus_c():
    """Take C, "two throats": two barking coughs, then two screams that do
    not agree with each other, and a thin whimper at the end. More pain,
    less rage (he is two animals fused)."""
    L = [
        lay(note('cmi_seal', 36), 0.0, st=-4, env=[(0, 1), (1.9, 1), (2.2, 0)], gain=-1),
        lay(note('cmi_scream2', 48), 1.7, st=[(0, -9), (0.5, -3), (2.0, -2), (2.1, -6), (2.25, -2.5), (3.6, -4),
                                             (4.6, -9)],
            env=[(0, 0), (0.2, 1), (3.6, 0.9), (4.6, 0)], gain=1),
        lay(note('cmi_scream1', 43), 2.0, st=[(0, -6), (0.6, -1), (2.4, -2), (4.2, -4), (5.2, -10)],
            env=[(0, 0), (0.3, 1), (4.0, 0.9), (5.2, 0)], gain=0),
        # two sung voices, a semitone apart and slowed: the second throat
        lay(note('vox_ah_hi', 65), 1.9, st=[(0, -9), (0.5, -4), (2.0, -3.5), (2.1, -7), (2.25, -4), (3.6, -5),
                                           (5.0, -11)], skip=0.6,
            env=[(0, 0), (0.3, 1), (3.8, 0.9), (5.0, 0)], gain=-2),
        lay(note('vox_ah_hi', 70), 2.2, st=[(0, -11), (0.6, -8), (2.4, -8.5), (4.0, -10), (5.0, -15)], skip=0.9,
            env=[(0, 0), (0.3, 1), (3.8, 0.8), (5.0, 0)], gain=-5),
        lay(note('vox_male', 46), 1.5, st=[(0, -9), (1, -7), (5, -8), (7, -12)], skip=1.5,
            env=[(0, 0), (1, 1), (5, 0.8), (7, 0)], gain=-5),
        lay(note('vox_male', 47), 1.6, st=[(0, -8), (1.2, -7), (5, -9), (7, -11)], skip=1.5,
            env=[(0, 0), (1, 1), (5, 0.8), (7, 0)], gain=-5),
        lay(note('str_cello', 36), 1.0, st=-7, skip=3.0, env=[(0, 0), (1, 1), (6, 0.8), (7.5, 0)], gain=-5),
        lay(note('vox_hgc', 41), 1.8, st=-5, skip=1.0, env=[(0, 0), (0.3, 1), (3.5, 0.7), (5, 0)], gain=-6),
        lay(note('cmi_cow', 41), 1.6, st=-2, env=[(0, 0), (0.6, 1), (4.5, 0.7), (6.0, 0)], gain=-3),
        lay(note('cmi_wolf', 48), 5.6, st=[(0, -1), (1.2, 0), (2.4, -2), (3.6, -7)],
            env=[(0, 0), (0.4, 0.8), (2.6, 0.6), (3.6, 0)], gain=-3),
    ]
    return S.mix(*L)


def roar_bus_d():
    """Take D: the first set's synthesised roar (its voice, as built by
    numbers), with the new animal bed under it, for the new metal chains."""
    old, sr = first_set('roar')
    old = old[: int(8.6 * sr)] * S.env([(0, 1), (7.6, 1), (8.6, 0)], 8.6)[: int(8.6 * sr)]
    L = [
        S.rms_norm(old, -17),
        lay(note('cmi_cow', 36), 0.15, st=[(0, -1), (1.0, 0), (4.5, -1), (7.6, -5)],
            env=[(0, 0), (0.5, 1), (5.5, 0.8), (7.6, 0)], gain=-3),
        lay(note('wnd_beast', 55), 1.5, st=[(0, -7), (1, -5), (3.5, -6), (5, -10)], skip=4.0,
            env=[(0, 0), (0.8, 1), (3.5, 0.8), (5.0, 0)], gain=-6),
        lay(note('brs_aven', 36), 0.0, st=-5, env=[(0, 1), (1.6, 1), (2.6, 0)], gain=-4),
    ]
    return S.mix(*L)


# take -> (bus, effect sends, sub-bass pitch, seed, loudness shape over the whole cry,
#          when the voice breaks, the shudder at the end (from, to, Hz at the start, Hz at the end, depth))
# The shape, the break and the shudder are put on after the distortion, which would otherwise iron them flat;
# the reverb and the echoes come after that and fill the gaps the way a valley would.
ROAR_TAKES = {
    'a': (roar_bus_a, {'burner': -7, 'metallurgy': (-8, 300, None), 'shredded': (-13, 1200, None), 'plate': -9,
                       'ring_low': -14, 'raum_big': -9},
          [(0, 36), (1, 33), (5, 28), (8.6, 22)], 1985,
          [(0, 0.0), (0.06, 0.35), (0.5, 0.7), (1.1, 1.0), (3.3, 1.0), (3.7, 0.8), (4.6, 0.92), (6.5, 0.6),
           (8.6, 0.32), (30, 0.32)],
          [3.4], (5.6, 8.4, 7.0, 3.5, 0.6)),
    'b': (roar_bus_b, {'burner': -5, 'metallurgy': (-10, 300, None), 'shredded': (-12, 1200, None), 'drum': -9,
                       'plate': -11, 'raum_big': -9},
          [(0, 40), (1.5, 34), (5, 30), (9.2, 20)], 1986,
          [(0, 0.0), (0.03, 0.85), (0.4, 1.0), (1.4, 0.7), (2.4, 0.9), (4.6, 1.0), (5.2, 0.75), (6.0, 0.95),
           (8.5, 0.55), (9.5, 0.3), (30, 0.3)],
          [], (6.8, 9.2, 6.0, 3.0, 0.55)),
    'c': (roar_bus_c, {'metallurgy': (-9, 200, None), 'shredded': (-14, 1500, None), 'ring_hi': -14, 'beam': -9,
                       'raum_cosmic': -10},
          [(0, 34), (2, 33), (6, 28), (9.4, 24)], 1987,
          [(0, 0.0), (0.05, 0.75), (0.7, 0.75), (0.85, 0.3), (1.0, 0.75), (1.6, 0.6), (1.75, 0.25), (2.2, 1.0),
           (5.5, 1.0), (7.0, 0.65), (9.4, 0.35), (30, 0.35)],
          [3.8, 5.6], (6.2, 9.2, 7.5, 4.0, 0.65)),
    'd': (roar_bus_d, {'burner': -8, 'plate': -8, 'ring_low': -12, 'raum_big': -11},
          [(0, 31), (8.6, 23)], 1988, [(0, 1.0), (30, 1.0)], [], None),
}
_roar_cache = {}


def roar_core(take):
    """The roar before the island answers: voice, effects and sub."""
    if take in _roar_cache:
        return _roar_cache[take]
    bus_fn, sends, sub_pts, seed, shape, breaks, shud = ROAR_TAKES[take]
    bus = bus_fn()
    wet = fx('roar_' + take, bus, sends, tail=7.0)
    wet = wet + 0.7 * S.bp(wet, 80, 260)                   # a little more chest
    wet = S.soft(S.rms_norm(wet, -15.5), 1.8)
    dur = len(bus) / SR
    sub = sub_layer(sub_pts, dur, bus, -27.0)
    core = S.mix(wet, sub)
    core = core * S.env(shape, len(core) / SR)[: len(core)]
    for at in breaks:
        core = core * voice_break(len(core), at)
    if shud:
        core = core * shudder(len(core), *shud)
    _roar_cache[take] = core
    return core


def make_roar(take='a'):
    core = roar_core(take)
    rng = np.random.default_rng(ROAR_TAKES[take][3])
    x = outdoors(core, rng)
    x = S.soft(S.norm(x, 1.0), 1.5)
    x = tidy(x, t_out=0.3)
    if len(x) > int(14.0 * SR):                            # keep it near the first set's length
        x = S.fade(x[: int(14.0 * SR)], 0.004, 2.5)
    return x


def make_roar_far(take='a'):
    """The same roar after kilometres of air: the top gone, the chest left,
    a valley's worth of reverb and three hill echoes."""
    core = roar_core(take)
    rng = np.random.default_rng(ROAR_TAKES[take][3])
    x = S.lp(core, 900, order=4)
    x = x + 0.5 * S.bp(core, 70, 320)
    x = S.reverb(x, 5.5, 0.5, rng, predelay=0.08, lp_fc=1100, hp_fc=50,
                 taps=((1.05, 0.5, 700), (2.3, 0.34, 500), (3.9, 0.22, 380)))
    return tidy(x, t_in=0.3, t_out=0.5, db=-58)


# ---------------------------------------------------------------------------
# footfall
# ---------------------------------------------------------------------------
def step_parts(rng):
    t = np.arange(int(1.1 * SR)) / SR
    f = 27 + 43 * np.exp(-t / 0.16)
    drop = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t / 0.42)
    drop += 0.3 * np.sin(4 * np.pi * np.cumsum(f) / SR) * np.exp(-t / 0.2)
    blow = [
        # a struck drumhead on a tube (Chromaphone), slowed: the blow itself
        lay(note('chr_thud', 19), 0.0, st=-5, lp=500, gain=0),
        # a cell door slamming, slowed until it is ground: weight and a metal edge
        lay(note('cmi_jaildoor', 41), 0.0, st=-3, env=[(0, 1), (2.5, 0.5), (4.0, 0)], gain=0),
        lay(note('cmi_door', 48), 0.02, st=-3, take=2.4, env=[(0, 1), (1.2, 0.5), (2.2, 0)], gain=-2),
        S.rms_norm(drop, -22),
    ]
    rumble = [
        # the ground still moving: a gong slowed to a groan, and low brass swelling a second late
        lay(note('cmi_gong1', 36), 0.05, st=-9, lp=300, env=[(0, 0), (0.3, 0.8), (2.0, 0.5), (4.5, 0)], gain=-8),
        lay(note('brs_doom', 29), 0.6, st=-7, skip=0.5, lp=220,
            env=[(0, 0), (0.7, 1), (2.2, 0.5), (3.6, 0)], gain=-8),
    ]
    debris = [
        # what breaks under it, and grit settling
        lay(note('cmi_ambcrash', 36), 0.03, hp=200, take=2.2, env=[(0, 1), (0.8, 0.5), (2.2, 0)], gain=-4),
        lay(note('chr_slab', 31), 0.0, st=-5, take=2.5, env=[(0, 1), (0.6, 0.4), (2.5, 0)], gain=-9),
        lay(note('chr_crackle', 48), 0.12, take=3.0, env=[(0, 1), (1.0, 0.5), (3.0, 0)], gain=-13),
        lay(note('arp_fire', 36), 0.4, skip=2.0, hp=600, take=3.0,
            env=[(0, 0), (0.3, 1), (1.2, 0.4), (3.0, 0)], gain=-21),
    ]
    return S.mix(*blow), S.mix(*rumble), S.mix(*debris)


def make_step():
    rng = np.random.default_rng(610)
    blow, rumble, debris = step_parts(rng)
    x = S.mix(blow, rumble, debris)
    x = fx('step', x, {'drum': -10, 'raum_ground': -15}, tail=2.0)
    x = x[: int(4.9 * SR)]
    # the blow, then the ground still moving (it swells again about a second later), then gone
    x = x * S.env([(0, 1), (0.3, 0.75), (0.8, 0.4), (1.3, 0.5), (2.2, 0.28), (3.4, 0.1), (4.9, 0)], 4.9)[: len(x)]
    x = S.soft(S.rms_norm(x, -19), 1.6)
    return tidy(x, t_in=0.001, t_out=0.6)


def make_step_far():
    rng = np.random.default_rng(610)
    blow, rumble, _ = step_parts(rng)
    x = S.soft(S.rms_norm(S.mix(blow, rumble), -14), 2.5)   # overdriven, so the far thump keeps some overtones
    x = S.lp(x, 260, order=4)
    x = S.reverb(x, 4.0, 0.45, rng, predelay=0.06, lp_fc=500, hp_fc=30, taps=((1.4, 0.4, 260),))
    x = x[: int(6.6 * SR)]
    return tidy(x, t_in=0.05, t_out=1.2, db=-58)


# ---------------------------------------------------------------------------
# furnace drone (seamless loop)
# ---------------------------------------------------------------------------
def make_furnace():
    rng = np.random.default_rng(77)
    dur, xf = 10.5, 2.5                                   # 8.0 s after the crossfade
    n = int(dur * SR)
    t = np.arange(n) / SR
    breath = 0.62 + 0.38 * np.sin(2 * np.pi * t / 4.0 - np.pi / 2)   # in and out every four seconds
    flick = 1 + 0.25 * S.lp(rng.standard_normal(n), 9) / 0.05
    flick = np.clip(flick, 0.3, 1.8)

    def cut(x):
        x = x[:n] if len(x) >= n else np.pad(x, (0, n - len(x)))
        return x

    drone = [
        # a sub-heavy trombone that turns to noise (Augmented BRASS "Tamed Beast"), a tone-wheel of a reactor
        cut(lay(note('brs_beast', 24), 0, st=-2, skip=2.0, hp=40, gain=0)),
        cut(lay(note('cmi_radioactivity', 48), 0, skip=0.5, gain=-5)),
        cut(lay(note('mod_firecrack', 48), 0, skip=1.0, gain=-6)),
    ]
    fire = [
        # the fire itself: ARP 2600 "Fire" noise and air roaring in a tube (Chromaphone)
        cut(lay(note('arp_fire', 48), 0, skip=1.0, hp=250, lp=2600, gain=-7)),
        cut(lay(note('chr_flue', 24), 0, st=-12, skip=1.0, lp=900, gain=-11)),
    ]
    crackle = cut(lay(note('chr_crackle', 48), 0, skip=0.5, gain=-12))
    x = S.mix(*drone) * (0.55 + 0.45 * breath) + S.mix(*fire) * breath ** 2 * flick + crackle * (0.5 + 0.5 * breath)
    x = fx('furnace', x, {'furnace': -13, 'drum': -14}, tail=0.5)[:n]
    x = x - np.mean(x)
    x = S.hp(x, 18)
    x = S.loopable(x, xf)
    return S.soft(S.rms_norm(x, -18), 1.3)


# ---------------------------------------------------------------------------
# the beam
# ---------------------------------------------------------------------------
def make_beam_charge():
    rng = np.random.default_rng(404)
    dur = 4.2
    n = int(dur * SR)
    t = np.arange(n) / SR
    u = t / dur
    st = -26 + 50 * u ** 1.6                                # two octaves under to two over, faster and faster
    speed = 2.0 ** (st / 12.0)
    src = S.mix(
        lay(note('vox_fire', 60), 0, skip=1.0, gain=0),      # a dirty formant voice
        lay(note('str_metal', 60), 0, skip=1.0, gain=-3),    # bowed metal strings
        lay(note('ms_peak', 72), 0, skip=0.5, gain=-8),      # a screaming MS-20 filter
        lay(note('wnd_beast', 60), 0, skip=3.0, gain=-6),    # flutter-tongue
    )
    whine = S.tape(src, speed)[:n]
    whine = np.pad(whine, (0, n - len(whine)))
    rate = 5 * (14.0 ** (u ** 1.3))                         # the ribs: a throb that becomes a buzz
    trem = 1 - (0.6 - 0.25 * u) * (0.5 + 0.5 * np.sin(2 * np.pi * np.cumsum(rate) / SR))
    whine = whine * trem
    air = lay(note('cs_wind', 60), 0, st=[(0, -12), (dur, 12)], skip=1.0, gain=-9)[:n]
    air = np.pad(air, (0, n - len(air))) * u ** 1.5
    leak = lay(note('mod_airleak', 72), 0, st=[(0, -9), (dur, 7)], skip=0.5, gain=-13)[:n]
    leak = np.pad(leak, (0, n - len(leak))) * u ** 2
    chest = lay(note('brs_beast', 24), 0, skip=3.0, lp=220, gain=-8)[:n]
    chest = np.pad(chest, (0, n - len(chest))) * (0.3 + 0.7 * u)
    x = (whine + air + leak + chest) * S.env([(0, 0), (0.3, 0.25), (3.2, 0.8), (4.15, 1.0), (dur, 1.0)], dur)[:n]
    x = fx('beam_charge', x, {'radioscream': -9, 'ring_hi': -14, 'plate_hi': -12}, tail=0.3)[:n]
    x = S.soft(S.rms_norm(x, -18), 1.4)
    return S.fade(x - np.mean(x), 0.01, 0.003)             # a hard edge for the beam's crack to land on


def beam_body(dur, rng):
    n = int(dur * SR)
    wob = S.lp(rng.standard_normal(n + SR), 6)[:n]
    wob = wob / (np.max(np.abs(wob)) + 1e-12)
    line = S.mix(
        lay(note('cs_fblead', 84), 0, skip=0.5, gain=0),     # CS-80 feedback lead, the thin line
        lay(note('ms_peak', 84), 0, skip=0.5, gain=-3),      # MS-20 screaming filter
        lay(note('syn_windmill', 60), 0, skip=1.0, gain=-9), # Synthi feedback squeal
    )
    line = S.tape(line, 2.0 ** ((12 + 0.35 * wob) / 12.0))[:n]   # an octave up, never still
    line = np.pad(line, (0, n - len(line)))
    arc = lay(note('syn_fmsiren', 60), 0, skip=0.5, hp=1500, gain=-14)[:n]
    arc = np.pad(arc, (0, n - len(arc))) * (0.6 + 0.4 * np.abs(wob))
    t = np.arange(n) / SR
    hum = S.rms_norm(np.sin(2 * np.pi * 50 * t) + 0.5 * np.sin(2 * np.pi * 100 * t + 0.7), -30)
    body = S.mix(
        lay(note('brs_beast', 24), 0, skip=5.0, lp=320, gain=-2)[:n],
        lay(note('mod_firecrack', 36), 0, skip=1.0, gain=-7)[:n],
        lay(note('cmi_radioactivity', 36), 0, skip=1.0, gain=-9)[:n],
        hum,
    )
    body = np.pad(body, (0, max(0, n - len(body))))[:n] * (1 - 0.3 * (0.5 + 0.5 * np.sin(2 * np.pi * 3.1 * t)))
    return line, arc, body


def make_beam():
    rng = np.random.default_rng(405)
    hold, fall = 4.4, 1.0
    dur = hold + fall
    line, arc, body = beam_body(dur, rng)
    e = S.env([(0, 1), (hold, 1), (dur, 0)], dur)
    n = len(e)
    crack = S.mix(
        lay(note('chr_slab', 43), 0, st=5, take=1.0, env=[(0, 1), (0.25, 0.4), (1.2, 0)], gain=-1),
        lay(note('cmi_ambcrash', 48), 0, st=7, hp=500, take=0.6, env=[(0, 1), (0.2, 0.4), (0.8, 0)], gain=-3),
        lay(note('cmi_steam', 60), 0, take=0.5, env=[(0, 1), (0.5, 0)], gain=-7),
    )
    x = S.mix((line[:n] * 1.0 + arc[:n] + body[:n]) * e, crack)
    x = fx('beam', x, {'radioscream': -10, 'burner': -11, 'plate_hi': -13, 'raum_ground': -15}, tail=2.0)
    x = S.soft(S.rms_norm(x, -17), 1.5)
    x = x[: int(7.3 * SR)]
    return tidy(x, t_in=0.0005, t_out=0.5)


def make_beam_loop():
    rng = np.random.default_rng(405)
    dur, xf = 4.2, 1.2
    line, arc, body = beam_body(dur, rng)
    x = line + arc + body
    x = fx('beam_loop', x, {'radioscream': -10, 'burner': -11, 'plate_hi': -13}, tail=0.2)[: len(line)]
    x = S.hp(x - np.mean(x), 18)
    x = S.loopable(x, xf)
    return S.soft(S.rms_norm(x, -17), 1.5)


# ---------------------------------------------------------------------------
# spent: the long breath out
# ---------------------------------------------------------------------------
def make_exhale():
    rng = np.random.default_rng(12)
    dur = 8.5
    pushes = [(0, 0), (0.4, 1), (1.9, 0.35), (2.5, 0.85), (3.8, 0.2), (4.4, 0.6), (5.5, 0.12), (6.0, 0.35),
              (7.4, 0.04), (dur, 0)]
    e = S.env(pushes, dur)
    n = len(e)

    def cut(x):
        x = x[:n] if len(x) >= n else np.pad(x, (0, n - len(x)))
        return x

    breath = S.mix(
        cut(lay(note('cmi_steam', 48), 0, st=-3, hp=250, gain=0)),             # steam, slowed: the air itself
        cut(lay(note('cmi_steam', 60), 0, st=-7, hp=400, gain=-4)),
        cut(lay(note('cs_wind', 60), 0, st=-4, skip=1.0, hp=300, gain=-2)),    # CS-80 "Big Wind"
        cut(lay(note('arp_fire', 48), 0, skip=2.0, hp=500, gain=-7)),          # hiss
        cut(lay(note('syn_breathing', 36), 0, skip=0.5, gain=-8)),             # Synthi "Heavy Breathing"
        cut(lay(note('chr_flue', 31), 0, st=-5, skip=1.0, lp=1200, gain=-12)), # air in a tube: the hollow chest
    ) * e
    wheeze = S.mix(
        cut(lay(note('wnd_air', 60), 0, st=[(0, 2), (3, -1), (dur, -6)], skip=1.0, gain=-14)),
        cut(lay(note('mod_airleak', 60), 0, st=[(0, -5), (dur, -12)], skip=0.5, gain=-17)),
    ) * e * (0.5 + 0.5 * np.abs(S.lp(rng.standard_normal(n), 3)) / 0.03).clip(0, 1.5)
    moan = S.mix(
        cut(lay(note('brs_growl', 31), 0, st=[(0, -3), (dur, -12)], skip=0.5, gain=-10)),   # a horn sinking
        cut(lay(note('cmi_cow', 41), 0, st=[(0, -2), (6.0, -9)], hp=60, gain=-12)),
    ) * S.env([(0, 0), (0.8, 1), (4.5, 0.5), (7.5, 0.0), (dur, 0)], dur) * (0.4 + 0.6 * e)
    rattle = cut(lay(note('chr_rattle', 31), 0, st=[(0, 0), (dur, -9)], gain=-11)) * e   # something loose, slowing
    ticks = np.zeros(n)
    for k in range(9):                                                           # cooling metal
        at = 5.9 + 2.4 * (k / 9) ** 0.8 + rng.uniform(-0.05, 0.05)
        tk = lay(note('chr_tick', [84, 91, 96, 91, 84][k % 5]), 0, take=0.12, env=[(0, 1), (0.12, 0)],
                 gain=-24 - 0.6 * k)
        i = int(at * SR)
        m = min(len(tk), n - i)
        if m > 0:
            ticks[i:i + m] += tk[:m]
    x = S.mix(breath, wheeze, moan, rattle, ticks)
    x = fx('exhale', x, {'drum': -11, 'raum_ground': -14}, tail=1.0)
    x = S.rms_norm(x, -20)
    x = x[: int(dur * SR)]
    return tidy(x, t_in=0.05, t_out=0.8, db=-70)


# ---------------------------------------------------------------------------
# presence: radio static, birds
# ---------------------------------------------------------------------------
def make_static():
    rng = np.random.default_rng(9)
    dur = 2.8
    n = int(dur * SR)
    t = np.arange(n) / SR

    def cut(x):
        x = x[:n] if len(x) >= n else np.pad(x, (0, n - len(x)))
        return x

    hiss = S.mix(
        cut(lay(note('arp_mars', 48), 0, skip=1.5, gain=0)),          # ARP 2600 "Signals From Mars"
        cut(lay(note('cmi_radiotower', 60), 0, skip=1.0, gain=-3)),   # CMI "The Old Radiotower"
        cut(lay(note('arp_fire', 48), 0, skip=3.0, gain=-5)),         # plain hiss
        cut(lay(note('ms_ion', 60), 0, skip=1.0, gain=-9)),           # MS-20 "Ion Storm"
    )
    crack = cut(lay(note('chr_crackle', 72), 0, skip=0.5, gain=-6)) * (0.3 + 0.7 * (t / dur) ** 1.5)   # thickens
    drift = 2.0 ** ((7 + 5 * np.sin(2 * np.pi * 0.31 * t + 1.0) - 6 * (t / dur) ** 2) / 12.0)
    whistle = S.tape(note('cs_fblead', 84)[int(0.5 * SR):], drift)[:n]                 # a carrier dragged off tune
    whistle = S.rms_norm(np.pad(whistle, (0, n - len(whistle))), -34)
    x = (hiss + crack + whistle) * (1 - 0.25 * (0.5 + 0.5 * np.sin(2 * np.pi * 2.7 * t)))
    x = fx('static', x, {'radiofail': -6, 'radio': -6, 'longdistance': -9}, tail=0.2)[:n]
    x = S.bp(x, 300, 3400, order=3)
    sq = S.env([(0, 0), (0.02, 1), (2.56, 1), (2.6, 0.0), (dur, 0)], dur)[:n]      # the squelch closes
    x = x * sq
    click = np.zeros(n)
    i = int(2.6 * SR)
    burst = rng.standard_normal(int(0.012 * SR)) * np.linspace(1, 0, int(0.012 * SR)) ** 2
    click[i:i + len(burst)] = S.rms_norm(burst, -16)
    x = S.rms_norm(x, -16) + S.bp(click, 500, 3000)
    return S.fade(x, 0.002, 0.02)


def make_birds():
    rng = np.random.default_rng(31)
    dur = 6.4
    n = int(dur * SR)
    out = np.zeros(n + SR)
    strips_ = [('cmi_bird1', (84, 79, 72)), ('cmi_bird2', (84, 79, 72)), ('cmi_bird3', (84, 79)),
               ('cmi_bird4', (84, 79, 72))]
    for k in range(170):                                   # short cries, thinning out as the flock leaves
        at = 5.8 * rng.uniform() ** 2.2
        nm, notes = strips_[rng.integers(len(strips_))]
        src = note(nm, notes[rng.integers(len(notes))])
        ln = rng.uniform(0.05, 0.19)
        a = rng.integers(0, max(1, len(src) - int(ln * SR * 2.2)))
        seg = src[a: a + int(ln * SR * 2.2)]
        up = rng.uniform(-1, 9)
        m = int(ln * SR)
        sp = 2.0 ** ((up - 3.5 * np.linspace(0, 1, m) ** 1.5) / 12.0)        # each cry falls a little
        seg = S.tape(seg, sp)[:m]
        seg = seg * np.hanning(len(seg))
        g = 10 ** ((-rng.uniform(0, 10) - 16 * at / 5.8) / 20)
        i = int(at * SR)
        out[i:i + len(seg)] += S.rms_norm(seg, -20) * g
    for k in range(10):                                    # Synthi "Bird Patch" warbles and a few gull calls
        at = 4.5 * rng.uniform() ** 1.3
        src = note('syn_birdpatch', (60, 72)[k % 2])
        a = rng.integers(SR, len(src) - SR)
        m = int(rng.uniform(0.08, 0.2) * SR)
        seg = S.tape(src[a: a + 3 * m], 2.0 ** ((5 - 7 * np.linspace(0, 1, m)) / 12.0))[:m]
        seg = seg * np.hanning(len(seg))
        i = int(at * SR)
        out[i:i + len(seg)] += S.rms_norm(seg, -31 - 8 * at / 4.5)
    for k in range(3):
        at = (0.3, 1.6, 3.2)[k]
        src = note('syn_seagulls', 72)
        a = int((1.0 + 3.0 * k) * SR)
        seg = src[a: a + int(0.9 * SR)]
        seg = seg * np.hanning(len(seg))
        i = int(at * SR)
        out[i:i + len(seg)] += S.rms_norm(seg, -27 - 3 * k)
    noise = note('arp_fire', 48)
    for b in range(12):                                    # wingbeats, out of step with each other
        rate = rng.uniform(6.5, 11.0)
        t0 = rng.uniform(0, 1.2)
        t1 = t0 + rng.uniform(2.0, 4.5)
        g0 = rng.uniform(-40, -34)
        at = t0
        while at < min(t1, dur - 0.1):
            m = int(rng.uniform(0.06, 0.09) * SR)
            a = rng.integers(SR, len(noise) - SR)
            seg = S.bp(noise[a: a + m * 3], 200, 1100)[m: 2 * m] * np.hanning(m)
            i = int(at * SR)
            out[i:i + m] += S.rms_norm(seg, g0 - 10 * (at - t0) / (t1 - t0))
            at += 1.0 / rate * rng.uniform(0.9, 1.1)
    x = out[:n]
    x = S.hp(x, 180)
    return S.fade(S.rms_norm(x, -22), 0.01, 1.2)


# ---------------------------------------------------------------------------
# table: name -> (maker, game sample rate, is a loop)
# ---------------------------------------------------------------------------
def table(roar_take):
    return {
        'roar': (lambda: make_roar(roar_take), 44100, False),
        'roar_far': (lambda: make_roar_far(roar_take), 22050, False),
        'step': (make_step, 22050, False),
        'step_far': (make_step_far, 22050, False),
        'furnace': (make_furnace, 22050, True),
        'beam_charge': (make_beam_charge, 44100, False),
        'beam': (make_beam, 44100, False),
        'beam_loop': (make_beam_loop, 44100, True),
        'exhale': (make_exhale, 44100, False),
        'static': (make_static, 22050, False),
        'birds': (make_birds, 44100, False),
        'roar_b': (lambda: make_roar('b' if roar_take != 'b' else 'a'), 44100, False),
        'roar_c': (lambda: make_roar('c' if roar_take != 'c' else 'a'), 44100, False),
        'roar_d': (lambda: make_roar('d' if roar_take != 'd' else 'a'), 44100, False),
    }


ORDER = ['roar', 'roar_far', 'step', 'step_far', 'furnace', 'beam_charge', 'beam', 'beam_loop', 'exhale',
         'static', 'birds', 'roar_b', 'roar_c', 'roar_d']


def run(cmd):
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode != 0:
        raise SystemExit(f'failed: {cmd}\n{p.stdout}\n{p.stderr}')
    return p.stdout


def main():
    global FORCE_FX
    ap = argparse.ArgumentParser()
    ap.add_argument('--only', default='')
    ap.add_argument('--roar-take', default='a')
    ap.add_argument('--force-fx', action='store_true')
    ap.add_argument('--no-pack', action='store_true')
    a = ap.parse_args()
    FORCE_FX = a.force_fx
    only = [s for s in a.only.split(',') if s]
    wav_dir = os.path.join(BUILD, 'wav')
    snd_dir = os.path.join(BUILD, 'snd')
    pic_dir = os.path.join(PREV, 'pictures')
    for d in (wav_dir, snd_dir, pic_dir):
        os.makedirs(d, exist_ok=True)
    rep_path = os.path.join(BUILD, 'report.json')
    report = {}
    if os.path.exists(rep_path):
        with open(rep_path) as fh:
            report = json.load(fh)
    with open(os.path.join(FIRST, 'synth.json')) as fh:
        first = json.load(fh)
    tab = table(a.roar_take)
    for name in ORDER:
        if only and name not in only:
            continue
        maker, sr, loop = tab[name]
        x = maker()
        if not np.all(np.isfinite(x)):
            raise SystemExit(f'{name}: non-finite samples')
        x = S.norm(x, 0.89)
        if sr != SR:
            if loop:
                # resample with the loop wrapped round, so the filter's edges do not put a click at the seam
                k = 4096
                y = signal.resample_poly(np.concatenate([x[-k:], x, x[:k]]), sr, SR)
                k2 = k * sr // SR
                x = y[k2: len(y) - k2]
            else:
                x = signal.resample_poly(x, sr, SR)
            x = S.norm(x, 0.89)
        x = np.clip(x, -0.98, 0.98)
        sf.write(os.path.join(wav_dir, name + '.wav'), x, sr, subtype='PCM_16')
        data, _ = sf.read(os.path.join(wav_dir, name + '.wav'), dtype='int16')
        S.write_wss(os.path.join(snd_dir, name + '.wss'), data, sr)
        m = S.measure(x, sr)
        m['roughness'] = round(S.roughness(x, sr), 3)
        m['noisiness'] = round(S.noisiness(x, sr), 3)
        if loop:
            m['loop'] = S.loop_seam(x, sr)
        tag = {'roar': 'roar_' + a.roar_take, 'roar_far': 'roar_' + a.roar_take, 'roar_b': 'roar_b',
               'roar_c': 'roar_c', 'roar_d': 'roar_d', 'step_far': 'step'}.get(name, name)
        m['effects'] = INFO.get(tag, {})
        ref = first.get(name if name in first else 'roar')
        m['first_set'] = {k: ref[k] for k in ('seconds', 'rms_db', 'centroid_hz', 'band_energy')}
        report[name] = m
        b = m['band_energy']
        print(f"{name:12s} {m['seconds']:6.2f} s (first {ref['seconds']:5.2f})  rms {m['rms_db']:6.1f} (first "
              f"{ref['rms_db']:6.1f})  cen {m['centroid_hz']:6.0f} (first {ref['centroid_hz']:6.0f})  "
              f"sub {b['0-60Hz']:.2f} low {b['60-250Hz']:.2f} mid {b['250-1000Hz']:.2f} hi {b['1000-4000Hz']:.2f} "
              f"air {b['4000-11000Hz']:.2f}  rough {m['roughness']:.2f}  ends {m['start_db']:.0f}/{m['end_db']:.0f} dB"
              + (f"  seam {m['loop']['seam_jump']:.4f} vs {m['loop']['largest_inside_jump']:.4f}" if loop else ''))
        # picture: the first set's version on top, this one under it
        fname = name if name in first else 'roar'
        fx_, fsr = first_set(fname)
        S.picture([(fx_, fsr, f'FIRST SET (numerical synthesis): {fname}  {len(fx_) / fsr:.1f} s'),
                   (x, sr, f'VST SET: {name}  {len(x) / sr:.1f} s')], os.path.join(pic_dir, name + '.png'))
    with open(rep_path, 'w') as fh:
        json.dump(report, fh, indent=1)
    if a.no_pack:
        return
    previews(wav_dir)
    pack(snd_dir)


def previews(wav_dir):
    os.makedirs(PREV, exist_ok=True)
    tmp = os.path.join(BUILD, 'tmp')
    os.makedirs(tmp, exist_ok=True)
    loops = {'furnace', 'beam_loop'}
    files = []
    for i, name in enumerate(ORDER, 1):
        src = os.path.join(wav_dir, name + '.wav')
        if not os.path.exists(src):
            continue
        x, sr = sf.read(src)
        if name in loops:                                   # three times round, so a bad seam can be heard
            x = np.concatenate([x, x, x])
        if sr != SR:
            x = signal.resample_poly(x, SR, sr)
        w = os.path.join(tmp, name + '.wav')
        sf.write(w, x, SR, subtype='PCM_16')
        files.append((name, w))
        label = name if not name.startswith('roar_') or name == 'roar_far' else 'roar_take_' + name[-1]
        run([FFMPEG, '-hide_banner', '-loglevel', 'error', '-y', '-i', w, '-ac', '1', '-codec:a', 'libmp3lame',
             '-q:a', '2', os.path.join(PREV, f'{i:02d}_{label}.mp3')])
    gap = np.zeros(SR)
    allw = np.concatenate([np.concatenate([sf.read(w)[0], gap]) for _, w in files])
    sf.write(os.path.join(tmp, 'all.wav'), allw, SR, subtype='PCM_16')
    run([FFMPEG, '-hide_banner', '-loglevel', 'error', '-y', '-i', os.path.join(tmp, 'all.wav'), '-ac', '1',
         '-codec:a', 'libmp3lame', '-q:a', '2', os.path.join(PREV, '00_all_in_order.mp3')])
    # A/B: the first set's roar, a beep, then each take of this set
    t = np.arange(int(0.25 * SR)) / SR
    beep = np.concatenate([np.zeros(int(0.7 * SR)), 0.1 * np.sin(2 * np.pi * 1000 * t) * np.hanning(len(t)),
                           np.zeros(int(0.7 * SR))])
    old, osr = first_set('roar')
    seq = [old]
    for name in ('roar', 'roar_b', 'roar_c', 'roar_d'):
        w = os.path.join(tmp, name + '.wav')
        if os.path.exists(w):
            seq += [beep, sf.read(w)[0]]
    sf.write(os.path.join(tmp, 'ab.wav'), np.concatenate(seq), SR, subtype='PCM_16')
    run([FFMPEG, '-hide_banner', '-loglevel', 'error', '-y', '-i', os.path.join(tmp, 'ab.wav'), '-ac', '1',
         '-codec:a', 'libmp3lame', '-q:a', '2', os.path.join(PREV, 'AB_roar.mp3')])
    shutil.rmtree(tmp)
    print('[make_sounds] previews in', PREV)


def pack(snd_dir):
    import re
    cfg_src = os.path.join(HERE, 'addon', 'kj_roar', 'config.cpp')
    with open(cfg_src) as fh:
        cfg = fh.read()
    named = sorted(set(re.findall(r'\\kj_roar\\snd\\([A-Za-z0-9_]+)\.wss', cfg)))
    with open(os.path.join(BUILD, 'report.json')) as fh:
        report = json.load(fh)
    lines = []
    for n in named:
        wss = os.path.join(snd_dir, n + '.wss')
        if not os.path.exists(wss):
            raise SystemExit(f'config.cpp names snd\\{n}.wss but it was not built')
        out = run([TOOLS, 'sound', 'inspect', wss])
        m = re.search(r'Duration:\s*([0-9.]+)s', out)
        r = re.search(r'Sample Rate:\s*(\d+)', out)
        if not m or abs(float(m.group(1)) - report[n]['seconds']) > 0.02:
            raise SystemExit(f'{n}.wss: the engine loader disagrees about the length:\n{out}')
        lines.append(f'{n:12s} {float(m.group(1)):6.2f} s  {r.group(1) if r else "?"} Hz 16 bit mono  engine loader ok')
    with open(os.path.join(BUILD, 'inspect.log'), 'w') as fh:
        fh.write('\n'.join(lines) + '\n')
    print('\n'.join('    ' + s for s in lines))
    stage = os.path.join(BUILD, 'stage', 'kj_roar')
    if os.path.exists(stage):
        shutil.rmtree(stage)
    os.makedirs(os.path.join(stage, 'snd'))
    shutil.copy(cfg_src, os.path.join(stage, 'config.cpp'))
    for n in named:
        shutil.copy(os.path.join(snd_dir, n + '.wss'), os.path.join(stage, 'snd', n + '.wss'))
    addons = os.path.join(BUILD, '@kj_roar_vst', 'addons')
    os.makedirs(addons, exist_ok=True)
    pbo = os.path.join(addons, 'kj_roar.pbo')
    if os.path.exists(pbo):
        os.remove(pbo)
    with open(os.path.join(BUILD, 'pack.log'), 'w') as fh:
        fh.write(run([TOOLS, 'pbo', 'pack', stage, pbo]))
        fh.write(run([TOOLS, 'pbo', 'list', pbo]))
    dst = os.path.join(MOD, 'addons')
    os.makedirs(dst, exist_ok=True)
    shutil.copy(pbo, os.path.join(dst, 'kj_roar.pbo'))
    print(f'[make_sounds] packed {pbo} ({os.path.getsize(pbo) // 1024} KB) and copied it to {dst}')


if __name__ == '__main__':
    main()
