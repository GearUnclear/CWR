r"""Round 2 (owner's verdict on round 1: "CMI V is a good route but they all pretty
much sound too far away for the close version. foot fall close is terrible.
birds are bad the rest are good", then "i need new roars bro").

    python make_round2.py [--only name,name] [--no-pack]

What it builds, into <notes>\round2:
  * eight new roars made from nothing, four per creature (the Starved Bear and
    the dog-and-lizard), each as a CLOSE version (dry, hard onset, full top)
    and a far version derived from it;
  * three new footfalls from nothing, and a far footfall from the first;
  * two new bird takes (the Fairlight's own wing-flap and rook recordings);
  * close versions of the sounds he called good (furnace, beam charge, beam,
    beam loop, exhale): the round 1 sound without its reverb, plus near detail.
Close means: no reverb, no echoes, no fade-in, extra energy above 4 kHz that
follows the sound itself (hoarseness, crackle, hiss), more body at 120 to 450 Hz.

Sources are the same as round 1 (stems.py strips, group cmi2 added: the CMI V
library at and near its natural pitch) and the same headless REAPER effects
pass (fxpass.py), minus every reverb. Measurements against round 1 go to
round2\measurements.md. The addon is rebuilt in tmp\@kj_roar_vst after the
round 1 pbo has been copied to round2\backup.
"""
import argparse
import json
import os
import shutil
import subprocess

import numpy as np
from scipy import signal
import soundfile as sf

import make_sounds as M
import sndlib as S
from rjob import NOTES
from strips import note

SR = S.SR
HERE = os.path.dirname(os.path.abspath(__file__))
R2 = os.path.join(NOTES, 'round2')
BUILD = os.path.join(R2, 'build')
PREV = os.path.join(R2, 'previews')
R1WAV = os.path.join(NOTES, 'build', 'wav')
R1SND = os.path.join(NOTES, 'build', 'snd')


# ---------------------------------------------------------------------------
# closeness
# ---------------------------------------------------------------------------
def rms_db(x):
    return 20 * np.log10(np.sqrt(np.mean(x ** 2)) + 1e-12)


def L(src, *a, **k):
    """A layer (make_sounds.lay) whose end never clicks."""
    y = M.lay(src, *a, **k)
    n = int(0.03 * SR)
    if len(y) > n:
        y[-n:] *= np.linspace(1, 0, n)
    return y


def tile(x, n):
    reps = int(np.ceil(n / len(x)))
    return np.tile(x, reps)[:n]


def noise_bed(n, seed=0):
    """Instrument noise for breath and hiss: ARP 2600 "Fire" and CS-80 "Big Wind"."""
    a = note('arp_fire', 48)[SR:]
    b = note('cs_wind', 60)[SR:]
    off = (seed * 7919) % (len(a) // 2)
    return tile(a[off:], n) / (np.std(a) + 1e-12) + 0.7 * tile(b, n) / (np.std(b) + 1e-12)


def hoarse(x, db=-15, hp=3000, seed=0):
    """Hoarseness: noise above 3 kHz that is switched by the pulses of the
    voice itself, the way breath tears through vocal folds that no longer
    close. Up close this is the part of a cry that distance removes first."""
    e = S.lp(np.abs(S.hp(x, 100)), 90)
    y = S.hp(noise_bed(len(x), seed), hp) * e
    return S.rms_norm(y, rms_db(x) + db)


def excite(x, db=-17):
    """New overtones above 4 kHz made from what the sound has at 1.5 to 4 kHz (grit)."""
    h = S.hp(x, 1500)
    h = h / (np.max(np.abs(h)) + 1e-12)
    return S.rms_norm(S.hp(np.tanh(8 * h), 4000), rms_db(x) + db)


def close_master(x, body=0.7, hoarse_db=-15, excite_db=-17, drive=1.3, seed=0, level=-17.0):
    x = x + body * S.bp(x, 120, 450)
    x = x + hoarse(x, hoarse_db, seed=seed) + excite(x, excite_db)
    return S.soft(S.rms_norm(x, level), drive)


def finish(x, t_out=0.08, db=-60):
    x = S.hp(x - np.mean(x), 16)
    thr = 10 ** (db / 20) * np.max(np.abs(x))
    idx = np.where(np.abs(x) > thr)[0]
    x = x[: min(len(x), idx[-1] + int(0.05 * SR))]        # the start is never trimmed: no silence before the onset
    return S.fade(x, 0.002, t_out)


def far_from(core, seed, rt60=5.5):
    """The far version of a close sound: top gone, chest kept, valley reverb, three hill echoes."""
    rng = np.random.default_rng(seed)
    x = S.lp(core, 900, order=4) + 0.5 * S.bp(core, 70, 320)
    x = S.reverb(x, rt60, 0.5, rng, predelay=0.08, lp_fc=1100, hp_fc=50,
                 taps=((1.05, 0.5, 700), (2.3, 0.34, 500), (3.9, 0.22, 380)))
    return M.tidy(x, t_in=0.3, t_out=0.5, db=-58)


def no_reverb_fx():
    """make_sounds.fx with every reverb send removed (the cached effect returns of round 1 are reused)."""
    orig = M.fx

    def fx(tag, x, sends, tail=4.0, dry=1.0):
        sends = {k: v for k, v in sends.items() if not k.startswith('raum') and k != 'misty'}
        return orig(tag, x, sends, tail=tail, dry=dry)
    return orig, fx


def antiphase(n, rate, depth, sign):
    """Two voices that do not agree: when one swells the other is pushed down."""
    t = np.arange(n) / SR
    return 1 - depth * (0.5 + 0.5 * sign * np.sin(2 * np.pi * rate * t))


def padto(x, n):
    return x[:n] if len(x) >= n else np.pad(x, (0, n - len(x)))


# ---------------------------------------------------------------------------
# new roars. Each returns (tonal layers, breath and noise layers, effect sends, sub pitch, sub level, breaks)
# ---------------------------------------------------------------------------
def bear1():
    """The bawl, 8 s."""
    tonal = [
        L(note('cmi_cow', 48), 0.25, st=[(0, -6), (0.5, -1), (2.3, 0), (2.38, -5), (2.55, 0), (3.6, -2), (4.9, -8)],
          env=[(0, 0.3), (0.25, 1), (3.6, 0.9), (4.9, 0)], gain=2),
        L(note('c2_cow', 60), 0.3, st=[(0, -12), (0.5, -8), (2.3, -7), (3.0, -9)],
          env=[(0, 0), (0.2, 1), (2.4, 0.8), (3.0, 0)], gain=-3),
        L(note('vox_male', 46), 0.4, st=[(0, -9), (0.6, -6), (2.3, -5), (2.4, -9), (2.55, -5.5), (4.5, -8),
                                         (5.5, -13)], skip=1.5,
          env=[(0, 0), (0.4, 1), (4.4, 0.8), (5.5, 0)], gain=-4),
        L(note('c2_wildboar', 60), 1.2, st=[(0, -12), (0.6, -9), (1.1, -8.5)],
          env=[(0, 0), (0.3, 1), (1.05, 1), (1.1, 0)], gain=-5),
        L(note('brs_growl', 31), 4.6, st=[(0, -4), (3.2, -10)], skip=1.0,
          env=[(0, 0), (0.5, 1), (2.2, 0.6), (3.2, 0)], gain=-5),
    ]
    rest = [
        L(note('c2_snort', 60), 0.0, st=-7, env=[(0, 1), (0.5, 0.6), (0.9, 0)], gain=-2),
        L(note('c2_mongrel', 60), 0.0, st=-8, env=[(0, 1), (1.2, 0.7), (1.8, 0)], gain=-3),
        L(note('c2_steam', 65), 2.25, st=-3, hp=800, env=[(0, 0), (0.05, 1), (0.3, 0.5), (0.6, 0)], gain=-6),
        L(note('brs_beast', 29), 0.0, st=-2, skip=6.0, env=[(0, 0.8), (0.5, 1), (5.0, 0.8), (7.5, 0)], gain=-4),
        L(note('arp_fire', 48), 0.2, skip=2.0, lp=3500, hp=200, env=[(0, 0.3), (1.5, 1), (5.0, 0.7), (7.8, 0)],
          gain=-12),
        L(note('chr_rattle', 36), 0.3, st=[(0, 0), (8, -6)], env=[(0, 0.3), (2, 0.6), (5, 1), (6.5, 0.8), (8.0, 0)],
          gain=-12),
        L(note('c2_crackle1', 60), 4.8, st=-5, env=[(0, 0), (0.3, 1), (2.0, 0.6), (3.0, 0)], gain=-10),
    ]
    return tonal, rest, {'burner': -8, 'shredded': (-12, 1500, None), 'drum': -14}, \
        [(0, 42), (2.3, 38), (5, 30), (8, 24)], -27, [(2.3, 0.85, 0.28)]


def bear2():
    """The moan, 9.5 s: two phrases with a rattling breath between, the second weaker and cracked."""
    tonal = [
        L(note('brs_dreary', 29), 0.1, st=[(0, -2), (1.0, 1), (2.5, 0), (3.8, -5)], skip=0.5,
          env=[(0, 0.5), (0.3, 1), (3.0, 0.9), (3.8, 0)], gain=0),
        L(note('cmi_cow', 36), 0.1, st=[(0, 3), (1.0, 5), (2.5, 4), (3.8, -1)],
          env=[(0, 0.5), (0.3, 1), (3.0, 0.9), (3.8, 0)], gain=0),
        L(note('vox_male', 41), 0.3, st=[(0, -8), (1, -5), (2.5, -6), (3.6, -11)], skip=1.5,
          env=[(0, 0), (0.5, 1), (2.8, 0.8), (3.6, 0)], gain=-3),
        L(note('brs_dreary', 29), 4.7, st=[(0, -5), (0.8, -3), (1.5, -4), (1.6, -8), (1.75, -4.5), (2.6, -6),
                                           (3.9, -12)], skip=4.0,
          env=[(0, 0), (0.3, 0.9), (2.6, 0.7), (3.9, 0)], gain=-2),
        L(note('cmi_cow', 41), 4.7, st=[(0, -2), (0.8, 0), (2.6, -2), (3.9, -8)],
          env=[(0, 0), (0.3, 0.9), (2.6, 0.7), (3.9, 0)], gain=-1),
    ]
    rest = [
        L(note('c2_snort', 53), 0.0, st=-6, env=[(0, 1), (0.8, 0.5), (1.4, 0)], gain=-4),
        L(note('c2_door', 60), 0.0, st=-10, env=[(0, 1), (1.5, 0.6), (2.5, 0)], gain=-6),
        L(note('cs_wind', 60), 3.7, st=[(0, -6), (1.0, 4)], skip=1.0, hp=500, env=[(0, 0), (0.7, 1), (1.0, 0)],
          gain=-10),
        L(note('chr_rattle', 43), 3.7, env=[(0, 0), (0.6, 1), (1.0, 0)], gain=-10),
        L(note('c2_door', 53), 5.0, st=-12, env=[(0, 0), (0.5, 1), (3.0, 0.7), (4.2, 0)], gain=-7),
        L(note('cmi_seal', 41), 8.3, st=-3, take=0.9, env=[(0, 1), (0.6, 0.7), (0.9, 0)], gain=-5),
        L(note('brs_beast', 24), 0.0, st=-2, skip=3.0, env=[(0, 0.7), (1, 1), (8.5, 0.7), (9.5, 0)], gain=-6),
        L(note('arp_fire', 36), 0.0, skip=1.0, lp=2500,
          env=[(0, 0.4), (2, 1), (3.8, 0.5), (4.7, 1), (8.6, 0.5), (9.6, 0)], gain=-14),
    ]
    return tonal, rest, {'burner': -9, 'metallurgy': (-12, 400, None), 'drum': -13}, \
        [(0, 38), (3.8, 30), (4.7, 34), (9.5, 22)], -27, [(6.3, 0.8, 0.2)]


def bear3():
    """The bark, 2.6 s: a reaction."""
    tonal = [
        L(note('cmi_dog', 41), 0.0, st=-4, env=[(0, 1), (1.2, 0.8), (1.9, 0)], gain=1),
        L(note('c2_cow', 65), 0.05, st=[(0, -6), (0.3, -3), (1.0, -5), (1.6, -11)],
          env=[(0, 0.6), (0.1, 1), (1.0, 0.8), (1.6, 0)], gain=-1),
        L(note('brs_aven', 36), 0.0, st=-5, env=[(0, 1), (1.0, 0.8), (1.8, 0)], gain=-3),
    ]
    rest = [
        L(note('c2_mongrel', 60), 0.0, st=-6, env=[(0, 1), (1.5, 0.7), (2.4, 0)], gain=-2),
        L(note('c2_snort', 60), 0.0, st=-4, env=[(0, 1), (0.4, 0.5), (0.8, 0)], gain=-3),
        L(note('chr_rattle', 36), 1.2, env=[(0, 0), (0.3, 1), (1.0, 0.5), (1.5, 0)], gain=-9),
        L(note('c2_steam', 60), 1.3, st=-5, hp=500, env=[(0, 0), (0.2, 1), (0.9, 0.4), (1.3, 0)], gain=-9),
        L(note('brs_beast', 29), 0.0, st=-2, skip=6.0, env=[(0, 1), (1.5, 0.7), (2.5, 0)], gain=-5),
    ]
    return tonal, rest, {'burner': -7, 'shredded': (-12, 1500, None)}, [(0, 45), (2.6, 28)], -27, []


def bear4():
    """The furnace roar, 7 s: the voice gives out at 3.1 s and the fire in the chest roars on alone."""
    tonal = [
        L(note('c2_wildboar', 53), 0.15, st=[(0, -16), (0.6, -12), (1.8, -11), (1.9, -15), (2.05, -11.5), (3.0, -12),
                                             (3.1, -17)],
          env=[(0, 0.4), (0.3, 1), (3.0, 1), (3.1, 0)], gain=0),
        L(note('cmi_cow', 41), 0.1, st=[(0, -2), (0.6, 2), (3.0, 1), (3.2, -6)],
          env=[(0, 0.5), (0.3, 1), (3.0, 1), (3.25, 0)], gain=1),
        L(note('vox_hgc', 41), 0.2, st=[(0, -7), (0.6, -4), (3.0, -5), (3.2, -10)], skip=1.0,
          env=[(0, 0.3), (0.3, 1), (3.0, 0.9), (3.25, 0)], gain=-5),
        L(note('c2_wildboar', 60), 4.3, st=[(0, -14), (0.4, -12), (1.0, -16)],
          env=[(0, 0), (0.15, 0.7), (0.7, 0.5), (1.0, 0)], gain=-8),
    ]
    rest = [
        L(note('c2_bomb1', 60), 0.0, st=-5, env=[(0, 1), (0.8, 0.5), (1.6, 0)], gain=-4),
        L(note('c2_mongrel', 53), 0.0, st=-9, env=[(0, 1), (2, 0.8), (4.5, 0)], gain=-2),
        L(note('arp_fire', 48), 0.0, skip=3.0, lp=4500, hp=150,
          env=[(0, 0.4), (1.5, 0.8), (3.1, 1.0), (4.2, 1.0), (6.0, 0.4), (7.0, 0)], gain=-7),
        L(note('brs_beast', 24), 0.0, st=-2, skip=5.0, env=[(0, 0.8), (3.1, 1), (4.5, 0.9), (7.0, 0)], gain=-3),
        L(note('chr_flue', 31), 2.8, st=-7, skip=1.0, env=[(0, 0), (0.5, 1), (2.5, 0.6), (4.0, 0)], gain=-9),
        L(note('chr_crackle', 60), 3.0, skip=0.5, env=[(0, 0), (0.3, 1), (2.5, 0.8), (4.0, 0)], gain=-11),
        L(note('c2_crackle1', 65), 3.3, st=-3, env=[(0, 1), (1.5, 0.6), (2.0, 0)], gain=-9),
    ]
    return tonal, rest, {'burner': -6, 'metallurgy': (-10, 300, None), 'drum': -14}, \
        [(0, 40), (3.1, 34), (7, 24)], -27, [(1.9, 0.8, 0.2)]


def dog1():
    """Howl, scream, choke, 8 s."""
    tonal = [
        L(note('cmi_wolf', 48), 0.35, st=[(0, -6), (0.6, -2), (2.4, -1), (3.0, 3)],
          env=[(0, 0), (0.3, 1), (2.5, 1), (3.1, 0)], gain=0),
        L(note('cmi_wolf', 43), 0.9, st=[(0, -3), (1.5, -4.5), (3.5, -3), (4.2, -9)],
          env=[(0, 0), (0.5, 0.8), (3.6, 0.8), (4.2, 0)], gain=-4),
        L(note('c2_whinny', 60), 2.7, st=[(0, -3), (0.5, 2), (1.6, 1), (2.0, -6)],
          env=[(0, 0), (0.4, 1), (1.6, 1), (2.0, 0)], gain=0),
        L(note('cmi_scream2', 55), 2.9, st=[(0, -2), (0.4, 2), (1.8, 1.5), (2.2, -8)],
          env=[(0, 0), (0.3, 1), (1.7, 1), (2.2, 0)], gain=-2),
        L(note('c2_pigs', 60), 3.3, st=[(0, -4), (1.5, -2), (1.9, -9)],
          env=[(0, 0), (0.3, 1), (1.5, 0.9), (1.9, 0)], gain=-5),
        L(note('wnd_beast', 55), 3.0, st=-2, skip=5.0, env=[(0, 0), (0.4, 1), (1.6, 0.8), (2.0, 0)], gain=-7),
        L(note('cmi_wolf', 48), 6.2, st=[(0, 2), (1.5, -3)], skip=2.6, hp=300,
          env=[(0, 0), (0.3, 0.5), (1.0, 0.3), (1.5, 0)], gain=-9),
    ]
    rest = [
        L(note('c2_rip', 60), 0.0, st=-4, env=[(0, 1), (0.3, 0.5), (0.7, 0)], gain=-4),
        L(note('c2_dog', 60), 0.0, st=-3, env=[(0, 1), (0.5, 0.6), (0.9, 0)], gain=-1),
        L(note('c2_steam', 60), 0.1, st=-2, hp=600, env=[(0, 0.8), (2.5, 0.5), (3.1, 0)], gain=-7),
        L(note('c2_steam', 53), 3.0, st=-3, hp=600, env=[(0, 0), (0.3, 0.7), (2.5, 0.5), (3.4, 0)], gain=-8),
        L(note('chr_rattle', 43), 0.0, st=[(0, 5), (8, -4)], env=[(0, 0.8), (5.5, 0.6), (8.0, 0)], gain=-10),
        L(note('c2_gurgle', 60), 4.9, st=-8,
          env=[(0, 1), (0.5, 0.4), (0.9, 1), (1.4, 0.3), (1.9, 0.8), (2.6, 0)], gain=-1),
        L(note('c2_mud', 60), 5.0, st=-3, env=[(0, 1), (1.2, 0.6), (2.4, 0)], gain=-6),
        L(note('cmi_dog', 36), 4.85, st=-2, take=1.6, env=[(0, 1), (0.8, 0.5), (1.6, 0)], gain=-3),
    ]
    return tonal, rest, {'ring_low': -11, 'metallurgy': (-10, 300, None), 'shredded': (-13, 1500, None)}, \
        [(0, 48), (3, 44), (4.9, 36), (8, 26)], -29, [(4.85, 0.7, 0.15)]


def dog2():
    """Three yelps and a whine, 2.8 s: a reaction."""
    tonal, rest = [], []
    for k, t in enumerate((0.0, 0.55, 1.05)):
        tonal.append(L(note('c2_dog', 72), t, st=[(0, 5 - 2 * k), (0.12, 7 - 2 * k), (0.35, -2 - 2 * k)],
                       env=[(0, 1), (0.25, 0.8), (0.4, 0)], gain=-2 * k))
        tonal.append(L(note('c2_whinny', 72), t + 0.02, st=[(0, -k), (0.3, -5 - k)],
                       env=[(0, 1), (0.2, 0.6), (0.32, 0)], gain=-4 - 2 * k))
    for t in (0.33, 0.85):
        rest.append(L(note('c2_rip', 72), t, st=-2, env=[(0, 1), (0.12, 0.4), (0.2, 0)], gain=-5))
    tonal.append(L(note('cmi_wolf', 48), 1.4, st=[(0, 4), (0.5, 2), (1.3, -6)], skip=0.3,
                   env=[(0, 0.8), (0.8, 0.6), (1.3, 0)], gain=-3))
    rest += [
        L(note('chr_rattle', 43), 1.3, st=3, env=[(0, 0.6), (0.8, 1), (1.4, 0)], gain=-9),
        L(note('c2_steam', 65), 1.5, hp=800, env=[(0, 0), (0.3, 1), (1.2, 0)], gain=-10),
        L(note('c2_mongrel', 60), 0.0, st=-5, env=[(0, 1), (1.5, 0.6), (2.4, 0)], gain=-6),
    ]
    return tonal, rest, {'ring_low': -13, 'shredded': (-12, 1500, None)}, [(0, 50), (2.8, 34)], -30, []


def dog3():
    """The hiss, 6 s: the reptile leads; a dog's whine is strangled inside it."""
    n = int(6.2 * SR)
    t = np.arange(n) / SR
    f = 24 - 11 * np.clip(t / 5.5, 0, 1)
    rattle = 1 - 0.8 * (0.5 + 0.5 * np.sin(2 * np.pi * np.cumsum(f) / SR)) ** 2     # the gular rattle: slowing pulses
    lows = S.mix(
        L(note('c2_mongrel', 53), 0.0, st=-12, env=[(0, 1), (4.5, 1), (6.0, 0)], gain=-1),
        L(note('brs_beast', 29), 0.0, skip=6.0, env=[(0, 1), (4.5, 1), (6.0, 0)], gain=-5),
    )
    lows = padto(lows, n) * rattle
    tonal = [
        L(note('cmi_wolf', 48), 1.0, st=[(0, 3), (1, 5), (1.8, 4), (1.85, 0), (2.0, 4.5), (3.0, 2)], hp=350,
          env=[(0, 0), (0.4, 0.7), (1.8, 0.8), (1.85, 0.2), (2.0, 0.7), (3.0, 0)], gain=-7),
        L(note('c2_dog', 60), 4.6, st=-2, env=[(0, 1), (0.5, 0.5), (0.9, 0)], gain=0),
    ]
    rest = [
        lows,
        L(note('chr_rattle', 36), 0.0, st=[(0, 4), (6, -5)], env=[(0, 1), (4.5, 0.8), (6.0, 0)], gain=-8),
        L(note('c2_crackle1', 60), 0.0, st=-2, gain=-9),
        L(note('c2_crackle1', 60), 2.4, st=-2, gain=-9),
        L(note('c2_steam', 60), 0.0, st=-1, hp=500, env=[(0, 0.7), (2.0, 1), (2.9, 0.8)], gain=-4),
        L(note('c2_steam', 53), 2.6, st=-2, hp=500, env=[(0, 0), (0.3, 1), (2.4, 1.0), (3.3, 0)], gain=-3),
        L(note('cs_wind', 60), 0.0, skip=1.0, hp=1500, env=[(0, 0.5), (4.5, 1), (5.8, 0)], gain=-9),
        L(note('c2_rip', 60), 4.6, st=-3, env=[(0, 1), (0.5, 0)], gain=-4),
        L(note('c2_gurgle', 65), 5.0, st=-6, env=[(0, 1), (0.4, 0.3), (0.7, 0.8), (1.0, 0)], gain=-4),
    ]
    return tonal, rest, {'ring_low': -12, 'shredded': (-11, 2000, None)}, [(0, 44), (6, 30)], -30, []


def dog4():
    """Two howls, 9 s: two throats a tritone apart, a joint scream, a wet choke."""
    tonal = [
        L(note('cmi_wolf', 43), 0.2, st=[(0, -2), (0.8, 2), (3.0, 1), (3.1, 5), (3.25, -1), (4.2, -3), (5.0, -9)],
          env=[(0, 0.3), (0.4, 1), (4.2, 0.8), (5.0, 0)], gain=0),
        L(note('cmi_wolf', 41), 0.9, st=[(0, -9), (1.2, -5), (2.5, -6.5), (4.0, -4), (5.5, -8), (6.5, -14)],
          env=[(0, 0), (0.6, 0.9), (5.2, 0.8), (6.5, 0)], gain=-2),
        L(note('cmi_scream1', 55), 4.5, st=[(0, -5), (0.5, 0), (1.4, 0.5), (1.9, -7)],
          env=[(0, 0), (0.3, 1), (1.4, 1), (1.9, 0)], gain=-1),
        L(note('c2_whinny', 60), 4.7, st=[(0, -2), (0.4, 1), (1.3, 0), (1.7, -8)],
          env=[(0, 0), (0.2, 0.9), (1.3, 0.9), (1.7, 0)], gain=-3),
        L(note('vox_ah_hi', 75), 4.6, st=[(0, -6), (0.5, -2), (1.4, -2.5), (1.9, -9)], skip=0.6,
          env=[(0, 0), (0.3, 1), (1.4, 0.9), (1.9, 0)], gain=-6),
    ]
    rest = [
        L(note('cmi_multidog', 48), 0.0, st=2, take=0.8, env=[(0, 1), (0.5, 0.6), (0.8, 0)], gain=-2),
        L(note('c2_mongrel', 53), 0.0, st=-10, env=[(0, 1), (5.0, 0.8), (5.7, 0)], gain=-6),
        L(note('c2_steam', 60), 0.0, st=-3, hp=600, env=[(0, 0.6), (1.5, 0.3), (3.3, 0)], gain=-10),
        L(note('c2_rip', 53), 3.0, st=-6, env=[(0, 1), (0.6, 0.3), (1.2, 0)], gain=-7),
        L(note('c2_steam', 53), 6.2, st=-4, hp=500, env=[(0, 0), (0.2, 1), (2.2, 0.4), (2.9, 0)], gain=-6),
        L(note('c2_gurgle', 53), 6.4, st=-9,
          env=[(0, 1), (0.6, 0.3), (1.1, 0.9), (1.8, 0.2), (2.3, 0.6), (2.6, 0)], gain=-2),
        L(note('chr_rattle', 36), 6.3, st=[(0, 2), (2.7, -6)], env=[(0, 0.5), (0.5, 1), (2.0, 0.6), (2.7, 0)],
          gain=-8),
        L(note('c2_mud', 53), 6.5, st=-4, env=[(0, 1), (1.5, 0.5), (2.5, 0)], gain=-7),
    ]
    return tonal, rest, {'ring_low': -12, 'metallurgy': (-11, 300, None), 'beam': -15}, \
        [(0, 46), (4.6, 42), (6.4, 34), (9, 26)], -29, [(3.05, 0.6, 0.12)]


ROARS = {'bear1': bear1, 'bear2': bear2, 'bear3': bear3, 'bear4': bear4,
         'dog1': dog1, 'dog2': dog2, 'dog3': dog3, 'dog4': dog4}
_cores = {}


def roar_close(name):
    if name in _cores:
        return _cores[name]
    tonal, rest, sends, sub_pts, sub_db, breaks = ROARS[name]()
    ton = S.mix(*tonal)
    for (at, depth, width) in breaks:                      # the voice gives out; the breath does not
        ton = ton * M.voice_break(len(ton), at, depth, width)
    oth = S.mix(*rest)
    if name.startswith('dog'):                              # the two throats push each other down in turn
        ton = ton * antiphase(len(ton), 4.3, 0.35, 1)
        oth = oth * antiphase(len(oth), 4.3, 0.3, -1)
    bus = S.mix(ton, oth)
    wet = M.fx('r2_' + name, bus, sends, tail=0.6)          # no reverb in any chain used here
    wet = wet[: len(bus) + int(0.25 * SR)]
    sub = M.sub_layer(sub_pts, len(bus) / SR, bus, sub_db)
    x = S.mix(close_master(wet, seed=len(name) + ord(name[-1])), sub)
    x = finish(x)
    _cores[name] = x
    return x


# ---------------------------------------------------------------------------
# footfall, from nothing. Dry: no reverb anywhere.
# ---------------------------------------------------------------------------
def thud(f0, f1, tau_f, tau_a, dur=1.2, drive=2.5):
    """The weight: a sine falling from f0 to f1 with a 2 ms attack, overdriven
    so it has overtones a small speaker can show."""
    t = np.arange(int(dur * SR)) / SR
    f = f1 + (f0 - f1) * np.exp(-t / tau_f)
    x = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t / tau_a)
    x[: int(0.002 * SR)] *= np.linspace(0, 1, int(0.002 * SR))
    return np.tanh(drive * x) / np.tanh(drive)


def grains(rng, srcs, count, t0, t1, power, len_rng, st_rng, lvl0, fall_db, dur):
    """Rubble: many short pieces of the Fairlight's impact and gravel
    recordings, each with a sharp start, thinning out and getting quieter."""
    out = np.zeros(int(dur * SR))
    for _ in range(count):
        at = t0 + (t1 - t0) * rng.uniform() ** power
        nm, p = srcs[rng.integers(len(srcs))]
        s = note(nm, p)
        ln = rng.uniform(*len_rng)
        st = rng.uniform(*st_rng)
        need = int(ln * SR * 2 ** (st / 12)) + 4
        a = rng.integers(0, max(1, len(s) - need))
        seg = S.tape(s[a: a + need], 2 ** (st / 12))[: int(ln * SR)]
        if len(seg) < 64 or np.max(np.abs(seg)) < 1e-5:
            continue
        w = np.exp(-np.arange(len(seg)) / (len(seg) / 3.0))
        k = int(0.0015 * SR)
        w[:k] *= np.linspace(0, 1, k)
        seg = seg * w
        g = 10 ** ((lvl0 - fall_db * (at - t0) / (t1 - t0) - rng.uniform(0, 8)) / 20)
        seg = seg / (np.sqrt(np.mean(seg ** 2)) + 1e-9) * g
        i = int(at * SR)
        m = min(len(seg), len(out) - i)
        if m > 0:
            out[i:i + m] += seg[:m]
    return out


def lvl(x, db):
    return S.rms_norm(x, db)


def spray(dur, level, seed):
    """Dirt and dust thrown up: a burst of hiss above 3 kHz that is gone in under a second."""
    n = int(dur * SR)
    y = S.hp(noise_bed(n, seed), 3000) * S.env([(0, 1), (0.04, 0.6), (0.25, 0.25), (dur, 0)], dur)[:n]
    return S.rms_norm(y, level)


def step_master(x, dur):
    x = padto(x, int(dur * SR))
    hi = S.hp(x, 400)
    x = x + 0.6 * S.bp(x, 70, 220) + excite(hi, -6)
    x = S.soft(S.norm(x, 1.0), 1.6)
    x = S.hp(x - np.mean(x), 18)
    return S.fade(x, 0.0005, 0.4)


def step1():
    """Earth: a crack, a dry thud, gravel thrown out, stones coming down."""
    rng = np.random.default_rng(611)
    d = 3.2
    parts = [
        L(note('c2_stick1', 72), 0.0, take=0.12, env=[(0, 1), (0.12, 0)], level=-12),
        L(note('c2_rifle1', 72), 0.0, take=0.08, hp=300, env=[(0, 1), (0.08, 0)], level=-15),
        lvl(thud(100, 36, 0.05, 0.30), -11),
        L(note('c2_bomb1', 60), 0.0, st=-5, env=[(0, 1), (0.5, 0.4), (1.2, 0)], level=-15),
        L(note('c2_kick1', 72), 0.0, st=-2, env=[(0, 1), (0.6, 0)], level=-14),
        L(note('c2_door', 72), 0.0, st=-4, take=0.6, env=[(0, 1), (0.6, 0)], level=-18),
        grains(rng, [('c2_sandwalk', 72), ('c2_sandwalk', 65), ('c2_crackle1', 72), ('c2_hooves', 72),
                     ('c2_stick1', 65), ('c2_rip', 72), ('c2_breakgls', 72)], 160, 0.02, 1.4, 2.0, (0.03, 0.12), (-8, 4), -21, 20, d),
        spray(0.9, -22, 1),
        L(note('c2_rip', 72), 0.0, take=0.25, env=[(0, 1), (0.25, 0)], level=-17),
        grains(rng, [('c2_hooves', 65), ('c2_hooves', 60), ('c2_stick1', 60), ('c2_door', 72)], 22, 0.5, 2.4, 1.3,
               (0.08, 0.25), (-12, -3), -30, 14, d),
        L(note('arp_fire', 48), 0.0, skip=2.0, hp=1200, take=1.8,
          env=[(0, 1), (0.15, 0.5), (0.8, 0.15), (1.8, 0)], level=-30),
        L(note('c2_sandwalk', 60), 0.03, st=-3, env=[(0, 0.8), (1.2, 0.4), (2.2, 0)], level=-27),
    ]
    return step_master(S.mix(*parts), d)


def step2():
    """Street: the thud under a car's worth of metal and glass, then things rattling."""
    rng = np.random.default_rng(612)
    d = 3.4
    parts = [
        L(note('c2_shtgun1', 72), 0.0, take=0.15, env=[(0, 1), (0.15, 0)], level=-12),
        L(note('c2_shatter', 72), 0.0, take=0.3, env=[(0, 1), (0.3, 0)], level=-16),
        lvl(thud(90, 40, 0.04, 0.26), -11),
        L(note('c2_carsmash', 60), 0.0, st=-4, env=[(0, 1), (0.6, 0.5), (1.4, 0)], level=-14),
        L(note('c2_jaildoor', 72), 0.0, st=-5, take=0.8, env=[(0, 1), (0.8, 0)], level=-17),
        grains(rng, [('c2_smash', 72), ('c2_smash', 65), ('c2_shatter', 72), ('c2_breakgls', 72),
                     ('c2_glasmash', 72), ('c2_stick1', 72)], 150, 0.03, 1.6, 2.0, (0.03, 0.14), (-4, 7), -20, 18, d),
        spray(0.7, -24, 2),
        L(note('c2_glasmash', 72), 0.0, take=0.5, env=[(0, 1), (0.5, 0)], level=-16),
        grains(rng, [('c2_gedclang', 72), ('c2_gedclang', 65), ('c2_breakgls', 72), ('chr_tick', 84),
                     ('chr_tick', 91)], 34, 0.3, 2.6, 1.2, (0.1, 0.35), (-4, 6), -28, 10, d),
    ]
    return step_master(S.mix(*parts), d)


def step3():
    """Heel and sole: two blows a tenth of a second apart, the second the heavier, into wet ground."""
    rng = np.random.default_rng(613)
    d = 3.6
    parts = [
        L(note('c2_stick1', 65), 0.0, take=0.15, env=[(0, 1), (0.15, 0)], level=-12),
        spray(0.8, -26, 3),
        L(note('c2_rip', 65), 0.12, take=0.4, env=[(0, 1), (0.4, 0)], level=-19),
        lvl(thud(85, 40, 0.05, 0.2), -11),
        S.pad(lvl(thud(75, 30, 0.08, 0.5), -10), before=0.12),
        L(note('c2_rifle1', 65), 0.12, take=0.1, hp=200, env=[(0, 1), (0.1, 0)], level=-17),
        L(note('c2_bomb1', 53), 0.12, st=-7, env=[(0, 1), (0.7, 0.4), (1.6, 0)], level=-14),
        L(note('c2_kick1', 60), 0.12, env=[(0, 1), (0.9, 0)], level=-13),
        L(note('c2_mud', 60), 0.1, st=-2, env=[(0, 1), (0.8, 0.5), (1.6, 0)], level=-17),
        grains(rng, [('c2_hooves', 60), ('c2_hooves', 53), ('c2_door', 65), ('c2_stick1', 60),
                     ('c2_crackle1', 60), ('c2_sandwalk', 65)], 90, 0.15, 2.0, 1.6, (0.06, 0.2), (-12, 0), -22, 16, d),
        L(note('c2_door', 53), 0.1, st=-14, lp=300, env=[(0, 0), (0.3, 0.6), (1.5, 0.3), (2.8, 0)], level=-24),
        L(note('c2_sandwalk', 53), 0.15, st=-5, env=[(0, 0.8), (1.5, 0.4), (2.6, 0)], level=-27),
    ]
    return step_master(S.mix(*parts), d)


def step_far():
    rng = np.random.default_rng(610)
    x = step1()
    x = S.lp(S.soft(S.norm(x, 1.0), 2.0), 260, order=4)
    x = S.reverb(x, 4.0, 0.45, rng, predelay=0.06, lp_fc=500, hp_fc=30, taps=((1.4, 0.4, 260),))
    return M.tidy(x[: int(6.6 * SR)], t_in=0.05, t_out=1.2, db=-58)


# ---------------------------------------------------------------------------
# birds: the Fairlight's wing-flap and rook recordings
# ---------------------------------------------------------------------------
def birds(wings, cries, seed, cry_lp=None):
    rng = np.random.default_rng(seed)
    d = 6.4
    out = np.zeros(int(d * SR) + 2 * SR)
    for _ in range(wings):                                 # each copy is one bird's wings, leaving
        src = note('c2_wngflaps', (72, 65, 60)[rng.integers(3)])
        seg = S.tape(src, 2 ** (rng.uniform(1, 9) / 12))
        t = np.arange(len(seg)) / SR
        seg = seg * np.exp(-t / rng.uniform(1.2, 2.4))
        seg = S.fade(seg, 0.002, 0.5)
        at = 0.0 if _ < 5 else 2.9 * rng.uniform() ** 1.2        # they do not all go at once; the last leave late
        i = int(at * SR)
        m = min(len(seg), len(out) - i)
        out[i:i + m] += S.rms_norm(seg, -24 - rng.uniform(0, 6) - 3.5 * at)[:m]
    wing_only = out.copy()
    pool = [('c2_rooks', 72), ('c2_rooks', 65), ('c2_crow', 72), ('cmi_bird2', 84), ('cmi_bird4', 84),
            ('cmi_bird2', 79)]
    for k in range(cries):                                 # whole alarm calls, not chopped
        nm, p = pool[rng.integers(len(pool))]
        seg = S.tape(note(nm, p), 2 ** (rng.uniform(-2, 3) / 12))
        seg = S.fade(seg, 0.005, 0.05)
        if cry_lp:
            seg = S.lp(seg, cry_lp)
        at = 0.1 + 3.6 * rng.uniform() ** 1.5
        i = int(at * SR)
        m = min(len(seg), len(out) - i)
        out[i:i + m] += S.rms_norm(seg, -23 - rng.uniform(0, 6) - 3.0 * at)[:m]
    n = int(d * SR)
    air = S.hp(noise_bed(n, seed), 3500) * S.lp(np.abs(S.hp(wing_only[:n], 300)), 60)   # feathers: hiss on every beat
    x = S.hp(out[:n], 150)
    x = x + S.rms_norm(air, rms_db(x) - 9) + excite(x, -14)
    return S.fade(S.rms_norm(x, -21), 0.002, 1.5)


# ---------------------------------------------------------------------------
# close versions of what he called good: the round 1 sound, dry, plus near detail
# ---------------------------------------------------------------------------
def with_detail(base, detail, body=0.5, exc=-16, level=-17.0, drive=1.3):
    n = len(base)
    x = base + padto(detail, n)
    x = x + body * S.bp(x, 120, 450) + excite(x, exc)
    return S.soft(S.rms_norm(x, level), drive)


def loop_detail(base, d, **k):
    """with_detail for a loop: worked on three copies end to end and the middle one kept, so no filter starts at the seam."""
    n = len(base)
    return with_detail(np.tile(base, 3), np.tile(padto(d, n), 3), **k)[n: 2 * n]


def furnace_close():
    rng = np.random.default_rng(78)
    base = M.make_furnace()                                # 8.0 s loop, no reverb in it
    n0 = int(10.5 * SR)
    t = np.arange(n0) / SR
    breath = 0.62 + 0.38 * np.sin(2 * np.pi * t / 4.0 - np.pi / 2)
    flick = np.clip(1 + 0.25 * S.lp(rng.standard_normal(n0), 9) / 0.05, 0.3, 1.8)
    d = S.mix(
        padto(L(note('arp_fire', 48), 0, skip=1.0, hp=1800, level=-27), n0) * breath ** 1.5 * flick,
        padto(L(note('chr_crackle', 72), 0, skip=0.3, level=-28), n0) * (0.5 + 0.5 * breath),
        padto(L(note('chr_crackle', 84), 0, skip=0.3, level=-31), n0),
        tile(L(note('c2_crackle1', 72), 0, level=-29), n0) * (0.4 + 0.6 * breath),
    )
    d = S.loopable(padto(d, n0), 2.5)
    return loop_detail(S.rms_norm(base, -18), d, body=0.4, exc=-18, level=-17.5)


def beam_charge_close():
    base = M.make_beam_charge()
    n = len(base)
    u = np.arange(n) / n
    d = S.mix(
        padto(L(note('cs_wind', 60), 0, skip=1.0, hp=2500, level=-22), n) * u ** 2,
        padto(L(note('arp_fire', 48), 0, skip=4.0, hp=4000, level=-24), n) * u ** 2.5,
        padto(L(note('chr_crackle', 84), 0, skip=0.3, level=-24), n) * u ** 1.5,
        tile(L(note('c2_crackle1', 72), 0, level=-26), n) * u,
        L(note('c2_gedclang', 60), 0.0, st=-3, take=0.5, env=[(0, 1), (0.5, 0)], level=-30),   # the ribs open
        L(note('c2_stick1', 72), 0.0, take=0.1, env=[(0, 1), (0.1, 0)], level=-30),
    )
    x = with_detail(S.rms_norm(base, -18), d, body=0.4, exc=-15, level=-17)
    return S.fade(x - np.mean(x), 0.001, 0.003)


def beam_detail(n, hold_env):
    return S.mix(
        padto(L(note('syn_fmsiren', 60), 0, skip=0.5, hp=3000, level=-32), n) * hold_env,
        padto(L(note('chr_crackle', 84), 0, skip=0.3, level=-33), n) * hold_env,
        padto(L(note('arp_fire', 48), 0, skip=5.0, hp=5000, level=-34), n) * hold_env,
    )


def beam_close():
    orig, fx = no_reverb_fx()
    M.fx = fx
    try:
        base = M.make_beam()
    finally:
        M.fx = orig
    n = len(base)
    e = padto(S.env([(0, 1), (4.4, 1), (5.4, 0)], 5.4), n)
    crack = S.mix(
        L(note('c2_rifle1', 72), 0.0, take=0.25, env=[(0, 1), (0.25, 0)], level=-16),
        L(note('c2_breakgls', 72), 0.0, take=0.5, env=[(0, 1), (0.5, 0)], level=-20),
        L(note('c2_shatter', 65), 0.0, take=0.4, env=[(0, 1), (0.4, 0)], level=-20),
    )
    x = with_detail(S.rms_norm(base, -18), S.mix(beam_detail(n, e), crack), body=0.4, exc=-20, level=-17)
    return S.fade(x, 0.0005, 0.3)


def beam_loop_close():
    base = M.make_beam_loop()                              # 3.0 s loop
    n0 = int(4.2 * SR)
    d = S.loopable(beam_detail(n0, np.ones(n0)), 1.2)
    return loop_detail(S.rms_norm(base, -18), d, body=0.4, exc=-20, level=-17)


def exhale_close():
    orig, fx = no_reverb_fx()
    M.fx = fx
    try:
        base = M.make_exhale()
    finally:
        M.fx = orig
    n = len(base)
    e = padto(S.env([(0, 0), (0.4, 1), (1.9, 0.35), (2.5, 0.85), (3.8, 0.2), (4.4, 0.6), (5.5, 0.12), (6.0, 0.35),
                     (7.4, 0.04), (8.5, 0)], 8.5), n)
    d = S.mix(
        tile(L(note('cmi_steam', 60), 0, hp=1500, level=-24), n) * e,
        padto(L(note('arp_fire', 48), 0, skip=6.0, hp=3500, level=-26), n) * e,
        padto(L(note('chr_rattle', 43), 0, st=[(0, 2), (8.5, -7)], level=-25), n) * e,
        L(note('c2_gurgle', 60), 2.45, st=-5, env=[(0, 1), (0.8, 0.4), (1.3, 0)], level=-25),
        L(note('c2_gurgle', 65), 4.35, st=-6, env=[(0, 1), (0.6, 0.4), (1.0, 0)], level=-27),
    )
    x = with_detail(S.rms_norm(base, -20), d, body=0.4, exc=-16, level=-20, drive=1.1)
    return S.fade(x, 0.03, 0.5)


# ---------------------------------------------------------------------------
# measuring near against far
# ---------------------------------------------------------------------------
def env5(x, sr):
    k = max(1, int(sr * 0.005))
    e = np.sqrt(np.convolve(x ** 2, np.ones(k) / k, mode='same') + 1e-20)
    return 20 * np.log10(e), k


def nearness(x, sr):
    """The numbers distance changes: share of energy above 4 kHz, how fast it
    reaches full level, and how long it hangs on after the body has ended."""
    spec = np.abs(np.fft.rfft(x)) ** 2
    f = np.fft.rfftfreq(len(x), 1 / sr)
    hf = float(np.sum(spec[f >= 4000]) / (np.sum(spec) + 1e-20))
    body = float(np.sum(spec[(f >= 120) & (f < 450)]) / (np.sum(spec) + 1e-20))
    e, _ = env5(x, sr)
    top = e.max()
    attack = int(np.argmax(e >= top - 6)) / sr
    above = np.where(e >= top - 10)[0]
    last = above[-1]
    below = np.where(e[last:] < top - 50)[0]
    tail = (below[0] if len(below) else len(e) - last) / sr
    crest = 20 * np.log10(np.max(np.abs(x)) / (np.sqrt(np.mean(x ** 2)) + 1e-12))
    return {'above_4k_pct': round(100 * hf, 2), 'body_120_450_pct': round(100 * body, 1),
            'attack_ms': round(1000 * attack, 1), 'tail_s': round(tail, 2), 'crest_db': round(crest, 1),
            'seconds': round(len(x) / sr, 2), 'rms_db': round(rms_db(x), 1)}


def round1_roar_wet_db():
    """Reverb against direct sound in the round 1 roar (take A), in dB."""
    core = M.roar_core('a')                                # effect returns come from the cache
    rng = np.random.default_rng(1985)
    out = M.outdoors(core, rng)
    wet = out - padto(core, len(out))
    r2 = np.sum(wet ** 2) / np.sum(core ** 2)
    r1 = 10 ** (-9 / 10)                                   # the Raum return was mixed 9 dB under the voice
    return round(10 * np.log10(r1 + r2), 1)


# ---------------------------------------------------------------------------
# build
# ---------------------------------------------------------------------------
# name -> (maker, sample rate, loop, round 1 sound it answers, reverb-to-direct of that round 1 sound in dB or None)
def table():
    t = {}
    for r in ROARS:
        t['roar_' + r] = ((lambda r=r: roar_close(r)), 44100, False, 'roar', 'roar')
        t['roar_' + r + '_far'] = ((lambda r=r: far_from(roar_close(r), 2000 + len(r))), 22050, False, 'roar_far',
                                   None)
    t['step'] = (step1, 44100, False, 'step', -15)
    t['step_2'] = (step2, 44100, False, 'step', -15)
    t['step_3'] = (step3, 44100, False, 'step', -15)
    t['step_far'] = (step_far, 22050, False, 'step_far', None)
    t['birds_1'] = ((lambda: birds(18, 9, 31)), 44100, False, 'birds', None)
    t['birds'] = ((lambda: birds(26, 3, 32, cry_lp=2500)), 44100, False, 'birds', None)
    t['furnace'] = (furnace_close, 44100, True, 'furnace', 'none')
    t['beam_charge'] = (beam_charge_close, 44100, False, 'beam_charge', 'none')
    t['beam'] = (beam_close, 44100, False, 'beam', -15)
    t['beam_loop'] = (beam_loop_close, 44100, True, 'beam_loop', 'none')
    t['exhale'] = (exhale_close, 44100, False, 'exhale', -14)
    return t


DEFAULT_ROAR = 'bear1'
CLASSES = (
    [('KJ_Roar', 'roar', 'db+50', 'the default roar (a copy of the Starved Bear, take 1), close'),
     ('KJ_RoarFar', 'roar_far', 'db+56', 'its far version'),
     ('KJ_RoarMid', 'roar_mid', 'db+50', 'round 1 roar (take A, with its own reverb): a middle distance')]
    + [(f'KJ_RoarBear{i}', f'roar_bear{i}', 'db+50', f'Starved Bear, take {i}, close') for i in range(1, 5)]
    + [(f'KJ_RoarBear{i}Far', f'roar_bear{i}_far', 'db+56', f'Starved Bear, take {i}, far') for i in range(1, 5)]
    + [(f'KJ_RoarDog{i}', f'roar_dog{i}', 'db+50', f'dog and lizard, take {i}, close') for i in range(1, 5)]
    + [(f'KJ_RoarDog{i}Far', f'roar_dog{i}_far', 'db+56', f'dog and lizard, take {i}, far') for i in range(1, 5)]
    + [('KJ_Step', 'step', 'db+40', 'footfall, take 1 (earth), close'),
       ('KJ_Step2', 'step_2', 'db+40', 'footfall, take 2 (street), close'),
       ('KJ_Step3', 'step_3', 'db+40', 'footfall, take 3 (heel and sole), close'),
       ('KJ_StepFar', 'step_far', 'db+46', 'footfall, far (from take 1)'),
       ('KJ_Furnace', 'furnace', 'db+20', 'furnace drone, close, seamless 8 s loop'),
       ('KJ_BeamCharge', 'beam_charge', 'db+42', 'beam charge, close'),
       ('KJ_Beam', 'beam', 'db+50', 'beam, close'),
       ('KJ_BeamLoop', 'beam_loop', 'db+50', 'beam body, close, seamless 3 s loop'),
       ('KJ_Exhale', 'exhale', 'db+36', 'spent exhale, close'),
       ('KJ_Static', 'static', 'db-3', 'radio static (round 1, unchanged)'),
       ('KJ_StaticFaint', 'static', 'db-16', 'radio static, faint'),
       ('KJ_Birds', 'birds', 'db+14', 'birds: mostly wings (the recommended take)'),
       ('KJ_Birds1', 'birds_1', 'db+14', 'birds: wings and alarm calls')])


def write_config():
    out = ['// kj_roar (VST set, round 2): Dedushka\'s sounds. A drop-in replacement for the first',
           '// set\'s kj_roar addon: the same CfgPatches class and the same twelve CfgSounds classes the',
           '// roar scripts use, plus one class per roar take for the two creatures, the other footfall',
           '// takes and the other bird take. Mount @kj_roar_vst INSTEAD of @kj_roar, never both.',
           '// This file is written by ../../make_round2.py (write_config); edit the table there.',
           '// Volumes as in the first set: db+50 has a 533 m reference distance, db+56 754 m,',
           '// db+40 300 m, db+20 95 m (Audio/Shared/AudioMath.hpp:8-21).', '',
           'class CfgPatches', '{', '    class kj_roar', '    {', '        units[] = {};', '        weapons[] = {};',
           '        requiredVersion = 1.30;', '        requiredAddons[] = {};', '    };', '};', '',
           'class CfgSounds', '{']
    for (cls, f, vol, text) in CLASSES:
        out += [f'    // {text}', f'    class {cls}', '    {', f'        name = "{cls}";',
                f'        sound[] = {{"\\kj_roar\\snd\\{f}.wss", {vol}, 1}};', '        titles[] = {};', '    };']
    out += ['};', '']
    with open(os.path.join(HERE, 'addon', 'kj_roar', 'config.cpp'), 'w', newline='\n') as fh:
        fh.write('\n'.join(out))


def mp3(x, sr, path):
    tmp = path[:-4] + '.tmp.wav'
    if sr != SR:
        x = signal.resample_poly(x, SR, sr)
    sf.write(tmp, np.clip(x, -0.98, 0.98), SR, subtype='PCM_16')
    subprocess.run([M.FFMPEG, '-hide_banner', '-loglevel', 'error', '-y', '-i', tmp, '-ac', '1', '-codec:a',
                    'libmp3lame', '-q:a', '2', path], check=True)
    os.remove(tmp)


def beep():
    t = np.arange(int(0.25 * SR)) / SR
    return np.concatenate([np.zeros(int(0.7 * SR)), 0.1 * np.sin(2 * np.pi * 1000 * t) * np.hanning(len(t)),
                           np.zeros(int(0.7 * SR))])


def at44(path):
    x, sr = sf.read(path)
    return signal.resample_poly(x, SR, sr) if sr != SR else x


def seq(items):
    out = []
    for i, x in enumerate(items):
        if i:
            out.append(beep())
        out.append(x)
    return np.concatenate(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--only', default='')
    ap.add_argument('--no-pack', action='store_true')
    a = ap.parse_args()
    only = [s for s in a.only.split(',') if s]
    wav_dir, snd_dir, pic_dir = os.path.join(BUILD, 'wav'), os.path.join(BUILD, 'snd'), os.path.join(PREV, 'pictures')
    for d in (wav_dir, snd_dir, pic_dir, os.path.join(R2, 'backup')):
        os.makedirs(d, exist_ok=True)
    rep_path = os.path.join(BUILD, 'report.json')
    report = json.load(open(rep_path)) if os.path.exists(rep_path) else {}
    tab = table()
    r1cache = {}
    roar_wet = None
    for name, (maker, sr, loop, ref, wet) in tab.items():
        if only and not any(name == o or name.startswith(o) for o in only):
            continue
        x = S.norm(maker(), 0.89)
        if sr != SR:
            x = S.norm(signal.resample_poly(x, sr, SR), 0.89)
        x = np.clip(x, -0.98, 0.98)
        sf.write(os.path.join(wav_dir, name + '.wav'), x, sr, subtype='PCM_16')
        data, _ = sf.read(os.path.join(wav_dir, name + '.wav'), dtype='int16')
        S.write_wss(os.path.join(snd_dir, name + '.wss'), data, sr)
        m = nearness(x, sr)
        if loop:
            m['loop'] = S.loop_seam(x, sr)
        if ref not in r1cache:
            rx, rsr = sf.read(os.path.join(R1WAV, ref + '.wav'))
            r1cache[ref] = (rx, rsr, nearness(rx, rsr))
        rx, rsr, rm = r1cache[ref]
        if wet == 'roar':
            roar_wet = roar_wet if roar_wet is not None else round1_roar_wet_db()
            wet = roar_wet
        m['round1'] = dict(rm, name=ref, reverb_to_direct_db=wet)
        m['reverb_to_direct_db'] = 'no reverb' if not name.endswith('_far') else 'far version'
        report[name] = m
        print(f"{name:16s} {m['seconds']:5.2f}s  >4k {m['above_4k_pct']:5.2f}% (r1 {rm['above_4k_pct']:.2f})  "
              f"attack {m['attack_ms']:6.1f} ms (r1 {rm['attack_ms']:.0f})  tail {m['tail_s']:.2f}s (r1 {rm['tail_s']:.2f})  "
              f"body {m['body_120_450_pct']:.0f}% (r1 {rm['body_120_450_pct']:.0f})  crest {m['crest_db']:.1f} (r1 {rm['crest_db']:.1f})"
              + (f"  seam {m['loop']['seam_jump']:.4f}/{m['loop']['largest_inside_jump']:.3f}" if loop else ''))
        S.picture([(rx, rsr, f'ROUND 1: {ref}  {len(rx) / rsr:.1f} s'), (x, sr, f'ROUND 2: {name}  {len(x) / sr:.1f} s')],
                  os.path.join(pic_dir, name + '.png'), top=10000.0)
        label = name
        px = np.concatenate([x, x, x]) if loop else x
        mp3(px, sr, os.path.join(PREV, label + '.mp3'))
    json.dump(report, open(rep_path, 'w'), indent=1)
    # the measurement table
    rows = ['| Sound | Above 4 kHz % (round 1) | Attack ms (round 1) | Tail s (round 1) | Reverb to direct dB (round 1) | 120-450 Hz % (round 1) | Crest dB (round 1) | Length s |',
            '|---|---|---|---|---|---|---|---|']
    for name in tab:
        if name not in report or name.endswith('_far'):
            continue
        m = report[name]
        r = m['round1']
        rows.append(f"| {name} (vs {r['name']}) | {m['above_4k_pct']} ({r['above_4k_pct']}) | {m['attack_ms']} ({r['attack_ms']}) | "
                    f"{m['tail_s']} ({r['tail_s']}) | none ({r['reverb_to_direct_db']}) | {m['body_120_450_pct']} ({r['body_120_450_pct']}) | "
                    f"{m['crest_db']} ({r['crest_db']}) | {m['seconds']} |")
    with open(os.path.join(R2, 'measurements.md'), 'w') as fh:
        fh.write('\n'.join(rows) + '\n')
    if only or a.no_pack:
        return
    # comparison previews
    w = lambda n: at44(os.path.join(wav_dir, n + '.wav'))
    r1 = lambda n: at44(os.path.join(R1WAV, n + '.wav'))
    mp3(seq([w(f'roar_bear{i}') for i in range(1, 5)]), SR, os.path.join(PREV, 'roars_bear.mp3'))
    mp3(seq([w(f'roar_dog{i}') for i in range(1, 5)]), SR, os.path.join(PREV, 'roars_doglizard.mp3'))
    mp3(seq([r1('roar'), w('roar_' + DEFAULT_ROAR), r1('beam'), w('beam'), r1('furnace'), w('furnace')]), SR,
        os.path.join(PREV, 'AB_close.mp3'))
    mp3(seq([r1('step'), w('step'), w('step_2'), w('step_3')]), SR, os.path.join(PREV, 'footfall_takes.mp3'))
    mp3(seq([r1('birds'), w('birds_1'), w('birds')]), SR, os.path.join(PREV, 'birds_takes.mp3'))
    # the addon: default roar, unchanged round 1 files, config, backup, pack
    for src, dst in ((f'roar_{DEFAULT_ROAR}', 'roar'), (f'roar_{DEFAULT_ROAR}_far', 'roar_far')):
        shutil.copy(os.path.join(snd_dir, src + '.wss'), os.path.join(snd_dir, dst + '.wss'))
        report[dst] = report[src]
    shutil.copy(os.path.join(R1SND, 'static.wss'), os.path.join(snd_dir, 'static.wss'))
    shutil.copy(os.path.join(R1SND, 'roar.wss'), os.path.join(snd_dir, 'roar_mid.wss'))
    r1rep = json.load(open(os.path.join(NOTES, 'build', 'report.json')))
    report['static'] = {'seconds': r1rep['static']['seconds']}
    report['roar_mid'] = {'seconds': r1rep['roar']['seconds']}
    json.dump(report, open(rep_path, 'w'), indent=1)
    write_config()
    live = os.path.join(M.MOD, 'addons', 'kj_roar.pbo')
    keep = os.path.join(R2, 'backup', 'kj_roar.round1.pbo')
    if os.path.exists(live) and not os.path.exists(keep):
        shutil.copy2(live, keep)
        print('[round2] round 1 pbo kept as', keep)
    if not os.path.exists(keep):
        raise SystemExit('no round 1 pbo backup; not rebuilding the addon')
    M.BUILD = BUILD
    M.pack(snd_dir)


if __name__ == '__main__':
    main()
