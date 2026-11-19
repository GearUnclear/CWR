"""gen_fire_sounds.py: the wildfire's sound set (POSEIDON_XP_FIRESPREAD).

The sounds the fire spread plays (World/Simulation/FireSpreadWorld.cpp, class
FireSound): a looping crackle bed for burning grass and brush, a looping roar
for an intense head fire or a crown going up, and six one-shot wood pops.
Our own audio: every sound is played by software synthesizers installed on
this machine, from notes and control curves composed here, then processed
through VST effects. No recording, sample library, preset or game data file
is used: the synths start from their default state and every setting that
matters is written by this script.

Two stages, both in REAPER (started with a private settings folder, a copy of
the owner's plugin scan and licence, audio forced to "Dummy Audio", on a
hidden Windows desktop, driven by a Lua job that quits when done).

1. The raw material, played by two synths (each stem alone: one instrument
   on a track, its settings, FX envelopes, a MIDI item):
   - Applied Acoustics Systems Chromaphone 3 (a physical model: a mallet
     striking a resonator). Five kits of single strikes, a note per slot with
     room between them, over a spread of pitches and velocities: tick and
     spark (membranes; twigs, grass stems), crack (a plate; a stick's
     fracture), knock (a beam; the stick's body) and thud (a drum head; a
     branch's weight). The strikes are damped hard so nothing rings at a
     pitch, and Mallet Noise stays off (on a damped resonator it is a
     one-sample spike up to 20 kHz: the electric tick of version 2).
   - Arturia Mini V4 (a Minimoog model). Its noise generator through the
     ladder filter, cutoff and level automated, is the flames: in the bed a
     low flutter (puffing at 3 to 12 Hz) and a soft mid rush; in the roar a
     sub rumble, the rumble, a body that flutters fast and a whoosh rising
     with the surges. A loop's curves are composed over one period and played
     three times; the middle period is kept and the start of the third
     crossfaded into its head (LOOP_XF_S, equal power), so the loop is
     periodic. One more Mini stem, its filter open, is the raw noise every
     noise burst and band below is cut from.
   Chromaphone's own noise exciter is not used: headless it renders silence
   (version 2's hiss, fizz, sizzle and jet came out of it at -107 dBFS).

   The fire is then composed from that material, sample-accurately: the
   kits are cut into single strikes, and every event is built from strikes
   and short bursts of the Mini's noise. Real crackle is a noise burst of a
   millisecond or a few, not a click, so a burst carries each event and a
   strike under it gives the jolt:
   - fizz: one burst under a millisecond about 3 kHz (grass, needles, seeds);
   - crackle: a run of one to six sub-cracks a millisecond or two apart, each
     a strike and a burst about 2 kHz, each weaker than the last;
   - snap: a crack and a knock, a longer burst, then a few crackles trailing
     over tens of ms as the fibres around the break let go.
   Their times come from `fractures`: an inhomogeneous Poisson process
   following a burning intensity that breathes and gusts, each failure
   setting off a cascade of weaker ones (fire crackles in clumps, not
   evenly), their strengths log-normal, eased under a cap. A loop's events
   wrap around its end, so the composed layers are periodic. Loops mix the
   Mini stems and the event layers by RMS at the levels in the designs; the
   pops are composed whole (see design_pops).

2. Processing (the plugins below): a loop goes in three times in a row and
   the middle period is kept, so reverb tails and envelope followers wrap
   around the seam; a 1024-sample crossfade into the following period
   closes it.

Needs:
    REAPER 7 x64          C:\\Program Files\\REAPER (x64)\\reaper.exe, licensed
                          (an unlicensed REAPER opens a nag window nobody can click)
    VST3i: Chromaphone 3 (Applied Acoustics Systems)  the strikes in every crackle, snap and pop
    VST3i: Mini V4 (Arturia)                         flutter, rush, roar, and the noise of every burst
    VST3: Pro-Q 3 (FabFilter)                        EQ: low cut, shelves, bells, high cut
    VST3: Transient Master (Native Instruments)      a little more attack, a shorter tail on the pops
    VST3: Supercharger GT (Native Instruments)       roar saturation and density
    VST3: Raum (Native Instruments)                  short outdoor space
    ffmpeg with libvorbis and libmp3lame             C:\\ffmpeg\\bin\\ffmpeg.exe or on PATH
Avoided (tried headless on this machine, 2026-11-12): Softube Saturation Knob
and Newfangled EQuivocate open a window that waits for a click (REAPER hangs
until the timeout); bx_cleansweep V2 loads but renders silence; NI Solid EQ
shows the same text for every gain and frequency, so its settings cannot be
checked from a job. A render that comes back silent counts as failed.
Chromaphone 3 clips its output near 0 dBFS: its mixer runs at CP_GAIN_DB.

Output (mono, 44.1 kHz, Ogg Vorbis q4): fire_crackle_loop.ogg (10 s loop),
fire_roar_loop.ogg (11.5 s loop, a different length so the two loops do not
repeat together), fire_pop_1.ogg .. fire_pop_6.ogg, and fire_preview.mp3
(30 s: the bed, the roar fading in, pops). Loops are normalized to about
-16 LUFS integrated (BS.1770 K-weighting and gating, computed here), pops
peak at -3 dBFS.

Usage:

    py -3 gen_fire_sounds.py [--out DIR] [--seed N] [--keep-temp] [--no-preview]
                             [--reuse-synths TEMP]

Intermediate files go to a temp folder that is removed at the end
(--keep-temp keeps it and prints where: the stems, the Lua jobs, their logs;
--reuse-synths takes the stems from such a folder and skips stage 1, for
work on the composition and the effects). The seed fixes every note, curve,
setting, event time and gain; the synths' own noise is not seeded, so two
runs differ in the noise but not in the design.
"""
import argparse
import ctypes
import os
import shutil
import subprocess
import sys
import tempfile
import time
import warnings

import numpy as np
from scipy import signal
from scipy.io import wavfile
from scipy.ndimage import minimum_filter1d, uniform_filter1d

SR = 44100
CRACKLE_S = 10.0          # crackle bed loop length (s)
ROAR_S = 11.5             # roar loop length (s); not a multiple of the bed's
LOOP_XF_S = 0.3           # stems: the next period crossfaded into the kept one's head (s)
STEM_CREST_DB = 20.0      # a stem's sample peaks held this far over its RMS (now and then Chromaphone lands a strike 20 dB over the rest)
FINAL_CREST_DB = 6.0      # after the effects: sample peaks held this far over the loudest 5 ms (rounds the strikes' first spike)
SEAM_XF = 1024            # samples cross-faded at a loop's seam after processing
LOOP_LUFS = -16.0         # integrated loudness of the loops
LOOP_CEIL_DB = -1.5       # loops' sample peak ceiling (dBFS)
MAX_LIMIT_DB = 10.0       # most peak limiting a loop may take to reach LOOP_LUFS
POP_PEAK_DB = -3.0        # one-shots' sample peak (dBFS)
POP_MAX_S = 1.2           # one-shot length bounds (s)
POP_MIN_S = 0.3
VORBIS_Q = 4

REAPER_EXE = r'C:\Program Files\REAPER (x64)\reaper.exe'
REAPER_COPY = ('REAPER.ini', 'reaper-vstplugins64.ini', 'reaper-vstshells64.ini',
               'reaper-license.rk', 'reaper-reginfo2.ini')
FFMPEG_CANDIDATES = (r'C:\ffmpeg\bin\ffmpeg.exe', 'ffmpeg')

CHROMA = 'VST3i: Chromaphone 3 (Applied Acoustics Systems)'
MINI = 'VST3i: Mini V4 (Arturia)'
PROQ = 'VST3: Pro-Q 3 (FabFilter)'
TRANSIENT = 'VST3: Transient Master (Native Instruments)'
SUPERCHARGER = 'VST3: Supercharger GT (Native Instruments)'
RAUM = 'VST3: Raum (Native Instruments)'


def proq(*bands):
    """Pro-Q 3 settings from (shape, Hz, dB gain, Q, slope) bands, in order."""
    out = []
    for i, (shape, hz, gain, q, slope) in enumerate(bands, 1):
        b = 'Band %d ' % i
        out += [(b + 'Used', 'Used'), (b + 'Enabled', 'Enabled'), (b + 'Shape', shape),
                (b + 'Dynamics Enabled', 'Dynamics Disabled'), (b + 'Frequency', hz), (b + 'Q', q),
                (b + 'Slope', slope)]
        if gain is not None:
            out.append((b + 'Gain', gain))
    return out


# The plugin chains. A value is what the plugin displays (a number in its
# unit: Hz, dB, %, s, ms), a label for a switch, or ('norm', x) for a raw
# 0..1. Raum's modulation is off so the reverb is time invariant and the
# loops' periods come out identical; Pro-Q 3 runs zero latency (minimum phase).
CHAINS = {
    'crackle': [
        (PROQ, proq(('Low Cut', 70, None, 0.7, '24 dB/oct'),         # no low thump under the grass
                    ('High Cut', 9000, None, 0.7, '12 dB/oct'))),    # heard across a field: the air takes the top
        (RAUM, [('Mode', 'Grounded'), ('Size', 20), ('Decay', 0.55), ('Predelay', 8), ('Diffusion', 60),
                ('Damp', 45), ('Modulation', ('norm', 0.0)), ('High Cut', 6000), ('Mix', 14)]),
    ],
    'roar': [
        (PROQ, proq(('Low Cut', 28, None, 0.7, '24 dB/oct'),         # below hearing, only eats headroom
                    ('Low Shelf', 120, 2.0, 0.7, '12 dB/oct'),       # the weight of a crown fire
                    ('High Cut', 8000, None, 0.7, '12 dB/oct'))),
        (SUPERCHARGER, [('Saturation', 3.0), ('Saturation Mode', 'Mild'), ('Compression', 3.5),
                        ('Attack', 6.0), ('Release', 6.0), ('Character', 4.0), ('Character Mode', 'Fat'),
                        ('Mix', 70)]),
        (RAUM, [('Mode', 'Airy'), ('Size', 45), ('Decay', 1.1), ('Predelay', 13), ('Diffusion', 70),
                ('Damp', 40), ('Modulation', ('norm', 0.0)), ('High Cut', 7000), ('Mix', 15)]),
    ],
    'pop': [
        (TRANSIENT, [('Attack', 5), ('Sustain', -8), ('Limit', 'Off')]),
        (PROQ, proq(('Low Cut', 60, None, 0.7, '18 dB/oct'),
                    ('High Cut', 9000, None, 0.7, '12 dB/oct'))),
        (RAUM, [('Mode', 'Grounded'), ('Size', 25), ('Decay', 0.6), ('Predelay', 12), ('Diffusion', 60),
                ('Damp', 50), ('Modulation', ('norm', 0.0)), ('High Cut', 8000), ('Mix', 16)]),
    ],
}


# ---------------------------------------------------------------- building blocks

def db(x):
    return 20.0 * np.log10(np.maximum(x, 1e-12))


def undb(d):
    return 10.0 ** (d / 20.0)


def lfo(rng, n, fmax, fmin=0.0):
    """A smooth random modulator with period n: every component has a whole
    number of cycles in the loop, so it ends where it starts. Unit std."""
    T = n / SR
    kmin = max(1, int(np.ceil(fmin * T)))
    kmax = max(kmin, int(np.floor(fmax * T)))
    spec = np.zeros(n // 2 + 1, complex)
    k = np.arange(kmin, kmax + 1)
    spec[k] = (rng.standard_normal(len(k)) + 1j * rng.standard_normal(len(k))) / np.sqrt(k)
    x = np.fft.irfft(spec, n)
    return x / (np.std(x) + 1e-20)


def add_at(buf, start, x):
    end = min(len(buf), start + len(x))
    if end > start:
        buf[start:end] += x[:end - start]


# ---------------------------------------------------------------- stage 1: the synth patches

# Chromaphone 3 repeats parameter names across its exciters, resonators and
# effect slots ("A Env: On" names eight different switches), so its settings
# go by (index, name as the plugin reports it); the job checks the name
# before it writes. Only layer A plays; B, the LFO, compressor, EQ and the
# three effect slots are off, so the output is the bare physical model.
CP_LAYER_B = (9, 'Mixer: Layer B On')
CP_OFF = [(210, 'A Env: On'), (211, 'A Env: On'), (233, 'A Env: On'), (240, 'A Env: On'), (366, 'A Env: On'),
          (492, 'A Env: On')]
CP_MALLET_VOL = (145, 'A Env: Mallet Volume')
CP_MALLET_KEY = (146, 'A Env: Mallet Volume Key')
CP_MALLET_DIRECT = (148, 'A Env: Mallet Direct')
CP_NOISE_LFO = (152, 'A Env: Noise Volume LFO')
CP_RES_A = dict(type=(154, 'A Env: Type'), pitch=(155, 'A Env: Pitch'), key=(156, 'A Env: Key'),
                decay=(162, 'A Env: Decay'), decay_key=(163, 'A Env: Decay Key'), decay_vel=(164, 'A Env: Decay Vel'),
                release=(165, 'A Env: Release'), material=(166, 'A Env: Material'), tone=(168, 'A Env: Tone'),
                hit_rdm=(172, 'A Env: Hit Rdm'), on=(178, 'A Env: On'))
CP_RES_B_ON = (199, 'A Env: On')
# The resonator types in switch order (the plugin shows long names cut short,
# "Drumhead%", so they are set by position, read back in the log).
CP_RES_TYPES = ['String', 'Beam', 'Marimba', 'Drumhead', 'Membrane', 'Plate', 'Closed Tube', 'Open Tube', 'Manual']
OFF = ('norm', 0.0)
ON = ('norm', 1.0)


CP_GAIN_DB = -30.0        # Chromaphone clips its own output: a hard strike on a low drum head needs this much room


def cp_base():
    s = [(CP_LAYER_B, 'Off'), ('Mixer: Gain', CP_GAIN_DB), ('A : Pre-Effect Gain', 0.0), ('A Vibrato: Amount', 0.0),
         ('A Unison: On', 'Off'), ('A Arpeggiator: On', 'Off'), (CP_NOISE_LFO, 0.0), (CP_MALLET_KEY, 0.0),
         (CP_RES_B_ON, OFF)]
    return s + [(k, OFF) for k in CP_OFF]


def cp_res(typ, decay, material=0.0, tone=0.0):
    """Resonator A: a model of type typ (Beam, Marimba, Plate, Drumhead,
    Membrane...) tuned by the note, ringing decay s, Material 0 = wood-like
    (the high partials die first)."""
    r = CP_RES_A
    return [(r['type'], ('norm', CP_RES_TYPES.index(typ) / 8.0)), (r['pitch'], 0.0), (r['key'], 1.0), (r['decay'], decay), (r['decay_key'], 0.0),
            (r['decay_vel'], 0.0), (r['release'], 100.0), (r['material'], material), (r['tone'], tone),
            (r['hit_rdm'], 100.0), (r['on'], ON)]


def cp_wood(typ, decay, stiffness, mallet_noise=0.0, hit_rdm=25.0, material=0.0, tone=0.0):
    """A mallet (stiffness = its brightness in Hz, brighter with velocity)
    striking a damped resonator, heard through the resonator only: the bare
    strike (Mallet Direct) stays off. Mallet Noise stays at 0: on a damped
    resonator it comes out as a one-sample spike reaching 20 kHz, and on the
    high notes, where the resonator answers 40 dB quieter, the spike is all
    that is left (the electric tick of the earlier versions). Hit Rdm moves
    the strike point a little from strike to strike; at 60 % a strike near a
    node came out 30 dB quieter than its neighbour."""
    res = [(k, hit_rdm) if k == CP_RES_A['hit_rdm'] else (k, v) for k, v in cp_res(typ, decay, material, tone)]
    return cp_base() + [
        ('A Mallet: On', 'On'), ('A Noise: On', 'Off'), ('A Mallet: Stiffness', stiffness),
        ('A Mallet: Stiffness Vel', 1.0), ('A Mallet: Noise', mallet_noise), ('A Mallet: Color', 50.0),
        (CP_MALLET_VOL, 0.0), ((147, 'A Env: Mallet Volume Vel'), 4.0), (CP_MALLET_DIRECT, OFF)] + res


def mini_noise(cutoff, emphasis=0.0, drive=0.0):
    """Mini V4: the three oscillators silent, its noise generator through the
    24 dB ladder filter (Emphasis = resonance, left at 0 so nothing whistles),
    the amplifier fully open."""
    return [('Osc 1 Volume', 0.0), ('Osc 2 Volume', 0.0), ('Osc 3 Volume', 0.0), ('Noise Volume', 10.0),
            ('Filter Cutoff', cutoff), ('Filter Emphasis', emphasis), ('Filter Contour Amount', 0.0),
            ('VCA Attack', 0.001), ('VCA Sustain', 10.0), ('Velocity Amount', 0.0), ('LFO Amplitude', 0.0),
            ('Drive Amount', drive)]


# ---------------------------------------------------------------- stage 1: the sources

# The kits: Chromaphone strikes, one note at a time with room between them,
# sliced apart afterwards (the first `keep` s of each, faded out over its
# second half: what a damped resonator adds after that is a faint rattle that
# repeats identically strike after strike). A note's pitch and velocity
# change the colour (higher and harder: brighter), Hit Rdm the detail.
#   name: (patch, pitches, velocities, repeats, slot s, keep s)
KITS = {
    'tick': (dict(typ='Membrane', decay=0.04, stiffness=3000), range(60, 85, 4), (70, 100, 127), 2, 0.10, 0.008),
    'spark': (dict(typ='Membrane', decay=0.03, stiffness=6000), range(66, 91, 6), (70, 100, 127), 2, 0.10, 0.006),
    'crack': (dict(typ='Plate', decay=0.06, stiffness=5000), range(44, 65, 4), (90, 127), 2, 0.15, 0.03),
    'knock': (dict(typ='Beam', decay=0.12, stiffness=4000), range(50, 73, 4), (90, 127), 2, 0.20, 0.04),
    'thud': (dict(typ='Drumhead', decay=0.12, stiffness=600), range(36, 53, 4), (100, 127), 2, 0.50, 0.08),
}
NOISE_S = 6.0             # Mini V4 noise, filter open: the raw material for every noise burst and band


def design_sources():
    """The kits' stems (a note per slot) and the noise stem. Returns (stems,
    {kit: note times})."""
    stems, index = [], {}
    for name, (patch, pitches, vels, reps, slot, keep) in KITS.items():
        notes, at, t = [], [], 0.05
        for p in pitches:
            for v in vels:
                for _ in range(reps):
                    notes.append((t, t + 0.02, int(p), int(v)))
                    at.append(t)
                    t += slot
        stems.append(dict(name='kit_' + name, synth=CHROMA, set=cp_wood(**patch), notes=notes, length=t + 0.1))
        index[name] = at
    stems.append(dict(name='noise', synth=MINI, set=mini_noise(18000), notes=[(0.0, NOISE_S - 0.01, 60, 127)],
                      length=NOISE_S))
    return stems, index


class Source:
    """The rendered kits cut into single strikes (each peak-normalized), the
    Mini's noise, and the events built from them."""

    def __init__(self, rng, rendered, index):
        self.rng = rng
        self.grains = {}
        for name, at in index.items():
            x = rendered['kit_' + name]
            slot, keep = KITS[name][4], KITS[name][5]
            out = []
            for t in at:
                seg = x[int((t - 0.002) * SR):int((t + slot - 0.01) * SR)]
                pk = np.max(np.abs(seg)) if len(seg) else 0.0
                if pk < 1e-7:
                    continue
                s0 = max(0, int(np.nonzero(np.abs(seg) > 0.1 * pk)[0][0]) - int(0.0003 * SR))
                g = seg[s0:s0 + int(keep * SR)].copy()
                f = len(g) // 2
                g[-f:] *= 0.5 + 0.5 * np.cos(np.linspace(0, np.pi, f))
                out.append(g / np.max(np.abs(g)))
            if len(out) < len(at) // 2:
                raise RuntimeError('kit %s: only %d of %d strikes sounded' % (name, len(out), len(at)))
            self.grains[name] = out
        n = rendered['noise'][int(0.1 * SR):]
        self.noise = (n - np.mean(n)) / (np.std(n) + 1e-20)

    def grain(self, name):
        g = self.grains[name]
        return g[self.rng.integers(len(g))]

    def band(self, length, lo, hi, order=2):
        """length samples of the noise, band-passed lo..hi Hz, unit peak-ish (unit std)."""
        pad = 512
        i = int(self.rng.integers(0, len(self.noise) - length - pad))
        sos = signal.butter(order, [lo, min(hi, 0.45 * SR)], 'bandpass', fs=SR, output='sos')
        y = signal.sosfilt(sos, self.noise[i:i + length + pad])[pad:]
        return y / (np.std(y) + 1e-20)

    def burst(self, fc, tau, bw=1.2, attack=0.0002):
        """A noise burst: a band bw octaves wide around fc, rising over attack s
        and dying away with time constant tau. Unit peak."""
        L = int((attack + 5.0 * tau) * SR) + 2
        y = self.band(L, fc * 2.0 ** (-bw / 2), fc * 2.0 ** (bw / 2))
        t = np.arange(L) / SR
        y *= np.minimum(t / attack, 1.0) * np.exp(-np.maximum(t - attack, 0.0) / tau)
        return y / (np.max(np.abs(y)) + 1e-20)


def _norm(x):
    return x / (np.max(np.abs(x)) + 1e-20)


def ev_fizz(src):
    """The finest crackle (grass stems, needles, a seed): one noise burst of
    under a millisecond, its band somewhere about 3 kHz, now and then with a
    tiny strike in it."""
    r = src.rng
    x = src.burst(float(np.clip(3000.0 * np.exp(0.35 * r.standard_normal()), 1500, 6500)), r.uniform(0.0003, 0.0012))
    if r.uniform() < 0.4:
        add_at(x, 0, r.uniform(0.2, 0.5) * src.grain('spark')[:len(x)])
    return _norm(x)


def ev_crackle(src, bright=1.0):
    """A crackle: wood fibres failing in a run of one to six sub-cracks a
    millisecond or two apart, each weaker than the last; every sub-crack a
    strike (the jolt) with a short noise burst (the tearing) about 2 kHz."""
    r = src.rng
    out = np.zeros(int(0.045 * SR))
    t, g = 0.0, 1.0
    for _ in range(1 + r.poisson(1.5)):
        i = int(t * SR)
        add_at(out, i, g * r.uniform(0.25, 0.6) * src.grain('spark' if r.uniform() < 0.3 * bright else 'tick'))
        fc = float(np.clip(2200.0 * bright * np.exp(0.4 * r.standard_normal()), 900, 6000))
        add_at(out, i, g * r.uniform(0.6, 1.0) * src.burst(fc, r.uniform(0.0005, 0.0025), bw=1.5))
        t += 0.0003 + r.exponential(0.0015)
        g *= r.uniform(0.35, 0.9)
        if t > 0.03:
            break
    return _norm(out)


def ev_snap(src, trail=3.0):
    """A stick giving way: the fracture (a plate strike, sometimes doubled),
    the stick's own knock, a burst of tearing, then a few crackles as the
    fibres around it let go over the next tens of ms."""
    r = src.rng
    out = np.zeros(int(0.16 * SR))
    add_at(out, 0, 0.7 * src.grain('crack'))
    add_at(out, int(r.uniform(0, 0.001) * SR), r.uniform(0.5, 0.9) * src.grain('knock'))
    add_at(out, 0, r.uniform(0.6, 1.0) * src.burst(float(np.clip(1600 * np.exp(0.3 * r.standard_normal()), 700, 4000)),
                                                    r.uniform(0.002, 0.005), bw=1.5))
    if r.uniform() < 0.5:
        add_at(out, int(r.uniform(0.0015, 0.005) * SR), r.uniform(0.3, 0.6) * src.grain('crack'))
    for _ in range(r.poisson(trail)):
        add_at(out, int((0.004 + r.exponential(0.02)) * SR), r.uniform(0.15, 0.45) * ev_crackle(src))
    return _norm(out)


def place(buf, t, x, loop):
    """Add x into buf at t s (wrapping around the end in a loop)."""
    i = int(round(t * SR))
    if loop:
        np.add.at(buf, (i + np.arange(len(x))) % len(buf), x)
    else:
        add_at(buf, i, x)


def layer(tl, events, make, inten=None, react=0.5, cap=2.0):
    """[(t, strength)] -> one period of events made by make(), each scaled by
    its strength and (a loop) by the burning intensity at its time ** react,
    the product eased under cap (tanh): a loop bed has room for a loud snap,
    not for one 20 dB over the rest."""
    buf = np.zeros(tl.n)
    for t, a in events:
        g = a * (inten[int(t * SR) % len(inten)] ** react if inten is not None else 1.0)
        place(buf, t, cap * np.tanh(g / cap) * make(), tl.loop)
    return buf


# ---------------------------------------------------------------- stage 1: the designs

class Timeline:
    """A loop's timeline: material is designed over one period L and played
    three times over (the render is 2 L + the crossfade long; the middle period
    is kept and the start of the third crossfaded into its head)."""

    def __init__(self, seconds, loop):
        self.L = seconds
        self.loop = loop
        self.length = (2 * seconds + LOOP_XF_S + 0.05) if loop else seconds
        self.n = int(round(seconds * SR))

    def notes(self, ev):
        """ev: [(t, dur, pitch, vel)] over one period -> repeated over the render."""
        reps = (0.0, self.L, 2 * self.L) if self.loop else (0.0,)
        out = []
        for off in reps:
            for t, d, p, v in ev:
                if t + off < self.length - 0.02:
                    out.append((t + off, min(t + off + d, self.length - 0.01), int(p), int(np.clip(round(v), 1, 127))))
        return out

    def curve(self, x, step=0.01):
        """A per-sample curve of one period -> automation points every step s over the render."""
        t = np.arange(0.0, self.length, step)
        idx = (np.round(t * SR).astype(int)) % len(x)
        return list(zip(t, x[idx]))

    def held(self, pitch=60):
        return [(0.0, self.length - 0.01, pitch, 127)]


def fractures(rng, tl, rate, intensity, kids=1.0, spread=0.03):
    """When wood fails: [(time, strength)] over one period. Parents arrive as
    an inhomogeneous Poisson process (thinned by intensity, a per-sample curve
    of mean 1) with log-normal strengths; each sets off a cascade of weaker
    failures (Poisson, more after a strong one) trailing over a few spread s:
    the next fibres letting go. Fire crackles in such bursts, not evenly. In a
    loop the cascades wrap around the end."""
    imax = float(np.max(intensity))
    k = rng.poisson(rate * imax * tl.L)
    times = np.sort(rng.uniform(0, tl.L, k))
    keep = rng.uniform(0, imax, k) < intensity[(times * SR).astype(int) % len(intensity)]
    out = []
    for t0 in times[keep]:
        a = rng.lognormal(0.0, 0.5)
        out.append((t0, a))
        for _ in range(rng.poisson(kids * min(a, 3.0))):
            t = t0 + 0.002 + rng.exponential(spread)
            if tl.loop:
                t %= tl.L
            if t < tl.L - 0.02:
                out.append((t, a * rng.uniform(0.15, 0.7)))
    return sorted(out)


def design_crackle(rng):
    """Burning grass and brush. The flames: Mini V4 noise through its ladder
    filter, a low flutter (cutoff and level puffing) and a soft mid rush that
    swells and dies with the burning. The fuel: fizz (the finest crackle, in
    clumps), crackles and a few snaps, arriving in cascades that follow the
    same burning intensity. Returns (Timeline, synth stems, compose(src) ->
    {layer: one period}, mix dB by stem or layer, RMS-normalized)."""
    tl = Timeline(CRACKLE_S, True)
    n = tl.n
    breath = lfo(rng, n, 0.4, 0.05)                 # the burning breathes over seconds
    gust = lfo(rng, n, 3.0, 0.5)                    # and comes in clumps (tufts catching)
    inten = np.exp(0.4 * breath + 0.6 * gust)
    inten /= np.mean(inten)
    puff = lfo(rng, n, 12.0, 3.0)                   # flames licking
    lick = lfo(rng, n, 9.0, 1.0)
    stems = [
        dict(name='flutter', synth=MINI, set=mini_noise(380), notes=tl.held(48),
             auto=[('Filter Cutoff', tl.curve(380.0 * np.exp(0.4 * puff + 0.25 * breath)), 'num'),
                   ('Noise Volume', tl.curve(np.clip(6.0 + 2.2 * np.tanh(puff) + 1.2 * breath, 1.0, 10.0)), 'num')]),
        dict(name='rush', synth=MINI, set=mini_noise(1500), notes=tl.held(48),
             auto=[('Filter Cutoff', tl.curve(1500.0 * np.exp(0.3 * lick + 0.3 * breath)), 'num'),
                   ('Noise Volume', tl.curve(np.clip(4.5 + 2.0 * np.log(inten) + 1.5 * np.tanh(lick), 0.5, 10.0)), 'num')]),
    ]
    fizz = fractures(rng, tl, 30.0, inten ** 1.5, kids=1.5, spread=0.012)
    crackle = fractures(rng, tl, 7.0, inten, kids=1.2, spread=0.03)
    snaps = fractures(rng, tl, 1.2, inten, kids=0.0)

    def compose(src):
        return dict(fizz=layer(tl, fizz, lambda: ev_fizz(src), inten, 1.0),
                    crackle=layer(tl, crackle, lambda: ev_crackle(src), inten),
                    snap=layer(tl, snaps, lambda: ev_snap(src), inten))
    mix = dict(flutter=-8.0, rush=-12.0, fizz=-11.0, crackle=-7.0, snap=-13.0)
    return tl, stems, compose, mix


def design_roar(rng):
    """A head fire or a crown going up: four Mini V4 noise voices, each its own
    ladder filter and none resonant: a sub rumble, the rumble, a body that
    flutters fast (the turbulence) and a rush that rises with the surges; all
    surging together, each wandering on its own. Under it the fuel going:
    crackles, snaps and fizz, denser and louder in the surges."""
    tl = Timeline(ROAR_S, True)
    n = tl.n
    surge = lfo(rng, n, 0.5, 0.08)                  # the whole fire surging
    own = [lfo(rng, n, f, 0.1) for f in (0.6, 1.2, 2.5, 3.0)]
    puff = lfo(rng, n, 22.0, 4.0)                   # turbulent puffing of the body
    rough = lfo(rng, n, 30.0, 6.0)
    inten = np.exp(0.5 * surge + 0.4 * own[2])
    inten /= np.mean(inten)

    def vol(base, s):
        return np.clip(base + s, 1.0, 10.0)

    stems = [
        dict(name='sub', synth=MINI, set=mini_noise(60), notes=tl.held(36),
             auto=[('Noise Volume', tl.curve(vol(7.0, 1.5 * surge)), 'num')]),
        dict(name='rumble', synth=MINI, set=mini_noise(170), notes=tl.held(36),
             auto=[('Filter Cutoff', tl.curve(170.0 * np.exp(0.3 * surge + 0.25 * own[0])), 'num'),
                   ('Noise Volume', tl.curve(vol(7.0, 1.5 * surge + 0.8 * own[0])), 'num')]),
        dict(name='body', synth=MINI, set=mini_noise(600, drive=2.0), notes=tl.held(36),
             auto=[('Filter Cutoff', tl.curve(600.0 * np.exp(0.3 * surge + 0.35 * own[1] + 0.2 * puff)), 'num'),
                   ('Noise Volume', tl.curve(vol(6.5, 1.5 * surge + 0.8 * own[2] + 1.6 * puff)), 'num')]),
        dict(name='rush', synth=MINI, set=mini_noise(2000), notes=tl.held(36),
             auto=[('Filter Cutoff', tl.curve(2000.0 * np.exp(0.35 * surge + 0.25 * own[3])), 'num'),
                   ('Noise Volume', tl.curve(vol(5.5, 2.5 * surge + 0.8 * own[3] + 1.2 * rough)), 'num')]),
    ]
    crackle = fractures(rng, tl, 14.0, inten, kids=1.2)
    snaps = fractures(rng, tl, 2.5, inten, kids=0.6, spread=0.05)
    fizz = fractures(rng, tl, 20.0, inten ** 2, kids=1.5, spread=0.012)

    def compose(src):
        return dict(crackle=layer(tl, crackle, lambda: ev_crackle(src, bright=0.8), inten, 1.0),
                    snap=layer(tl, snaps, lambda: ev_snap(src, trail=4.0), inten, 1.0),
                    fizz=layer(tl, fizz, lambda: ev_fizz(src), inten, 1.0))
    mix = dict(sub=-8.0, rumble=0.0, body=0.0, rush=-6.0, crackle=-11.0, snap=-11.0, fizz=-22.0)
    return tl, stems, compose, mix


def decay_db(t, tau, attack=0.0):
    """An envelope in dB: rising over attack s, then falling with time constant tau."""
    g = np.exp(-np.maximum(t - attack, 0.0) / tau) * np.clip(t / max(attack, 1e-4), 0.0, 1.0)
    return np.maximum(db(g), -60.0)


def design_pops():
    """Six wood failures, each composed from the sources (strikes of the
    kits, noise bursts and bands), times and gains drawn when composed.
    Returns [(Timeline, compose(src) -> the pop)]."""
    pops = []

    def pop(seconds):
        def deco(fn):
            tl = Timeline(seconds, False)
            pops.append((tl, lambda src: _norm(fn(src, np.zeros(tl.n), np.arange(tl.n) / SR))))
            return fn
        return deco

    def at(buf, t, x):
        add_at(buf, int(round(t * SR)), x)

    def thump(src, tau):
        # the weight of wood landing: a low band of noise (a drum head alone rings at a pitch)
        return src.burst(220.0, tau, bw=2.0, attack=0.001)

    def debris(src, buf, start, count, spread, gain=(0.04, 0.15), kit='tick'):
        r = src.rng
        for _ in range(count):
            hit = r.uniform(0.4, 0.8) * src.grain(kit)
            add_at(hit, 0, r.uniform(0.4, 0.8) * src.burst(2500.0 * np.exp(0.3 * r.standard_normal()),
                                                          r.uniform(0.0004, 0.001))[:len(hit)])
            at(buf, start + r.exponential(spread), r.uniform(*gain) * _norm(hit))

    @pop(0.6)
    def twig(src, buf, t):
        """1 twig snap: a dry stick breaking in two, a few bits falling in the grass."""
        r = src.rng
        at(buf, 0.0, src.grain('crack'))
        at(buf, 0.0, 0.8 * src.grain('spark'))
        at(buf, r.uniform(0.0015, 0.003), 0.6 * src.grain('crack'))
        at(buf, 0.0, 0.45 * src.burst(3500, 0.001))
        at(buf, 0.0005, 0.35 * src.grain('knock'))
        debris(src, buf, 0.03, int(r.integers(5, 9)), 0.09, gain=(0.05, 0.2))
        return buf

    @pop(1.0)
    def branch(src, buf, t):
        """2 branch crack: the main fracture, fibres tearing for 70 ms, the branch's weight, debris."""
        r = src.rng
        at(buf, 0.0, src.grain('crack'))
        at(buf, 0.0, 0.8 * src.grain('knock'))
        at(buf, 0.003, 0.7 * src.grain('crack'))
        at(buf, 0.0, 0.4 * src.burst(1800, 0.003, bw=1.5))
        for _ in range(int(r.integers(10, 17))):
            at(buf, r.uniform(0.002, 0.07), r.uniform(0.15, 0.5) * ev_crackle(src))
        at(buf, 0.004, 0.2 * src.grain('thud'))
        at(buf, 0.003, 0.6 * thump(src, 0.015))
        debris(src, buf, 0.08, 10, 0.2)
        return buf

    @pop(0.8)
    def resin(src, buf, t):
        """3 resin pop: a pocket of sap bursting (a dull thump and a puff of
        gas), then a short wet sizzle sputtering out, and spatter."""
        r = src.rng
        at(buf, 0.0, 0.25 * src.grain('thud'))
        at(buf, 0.0, 0.6 * thump(src, 0.012))
        at(buf, 0.0, 0.6 * src.grain('tick'))
        at(buf, 0.0005, 0.7 * src.burst(700, 0.012, bw=2.0, attack=0.0008))
        sput = np.exp(0.9 * np.tanh(lfo(r, len(t), 45.0, 6.0)))
        hiss = src.band(len(t), 2000, 5000) * undb(decay_db(t, 0.12, 0.008)) * sput
        buf += 0.12 * hiss
        for _ in range(60):
            tt = 0.006 + r.exponential(0.12)
            at(buf, tt, r.uniform(0.08, 0.3) * np.exp(-tt / 0.2) * ev_fizz(src))
        debris(src, buf, 0.02, 14, 0.12, gain=(0.05, 0.25), kit='spark')
        return buf

    @pop(1.15)
    def jet(src, buf, t):
        """4 sap jet: a small pop, then steam hissing out of a split (a band of
        noise wandering about 2.5 kHz, sputtering), two late spits."""
        r = src.rng
        at(buf, 0.0, ev_crackle(src))
        wander = np.log2(2500.0 * np.exp(0.35 * lfo(r, len(t), 3.0, 0.8)))
        jet = np.zeros(len(t))
        for fc in (1600.0, 2600.0, 4200.0):         # three bands, crossfaded as the jet's pitch wanders
            w = np.exp(-0.5 * ((wander - np.log2(fc)) / 0.45) ** 2)
            jet += w * src.band(len(t), fc * 2 ** -0.4, fc * 2 ** 0.4)
        flut = lfo(r, len(t), 35.0, 5.0)
        buf += 0.3 * _norm(jet) * undb(decay_db(t, 0.22, 0.04) + 7.0 * np.tanh(flut))
        at(buf, r.uniform(0.2, 0.3), r.uniform(0.6, 0.9) * ev_crackle(src))
        at(buf, r.uniform(0.45, 0.6), r.uniform(0.4, 0.7) * ev_crackle(src))
        return buf

    @pop(0.9)
    def split(src, buf, t):
        """5 green wood split: four to six cracks running along the grain within 130 ms, fibres between them."""
        r = src.rng
        tt, g = 0.0, 1.0
        for _ in range(int(r.integers(4, 7))):
            at(buf, tt, g * src.grain('crack'))
            at(buf, tt, g * r.uniform(0.4, 0.7) * src.grain('knock'))
            tt += r.uniform(0.012, 0.035)
            g = r.uniform(0.5, 0.95)
        for _ in range(12):
            at(buf, r.uniform(0.0, tt + 0.02), r.uniform(0.1, 0.35) * ev_crackle(src))
        debris(src, buf, tt, 8, 0.15)
        return buf

    @pop(1.2)
    def log(src, buf, t):
        """6 log cracking open: a low thump, the crack, the log's body, then
        embers and bits showering down, thinning out."""
        r = src.rng
        at(buf, 0.0, 0.3 * src.grain('thud'))
        at(buf, 0.0, 0.8 * thump(src, 0.025))
        at(buf, 0.002, src.grain('crack'))
        at(buf, 0.005, 0.7 * src.grain('crack'))
        at(buf, 0.003, 0.6 * src.grain('knock'))
        at(buf, 0.002, 0.35 * src.burst(1200, 0.004, bw=1.5))
        for _ in range(45):
            tt = 0.03 + r.exponential(0.28)
            if tt < 1.1:
                make = ev_fizz if r.uniform() < 0.5 else ev_crackle
                at(buf, tt, r.uniform(0.06, 0.3) * np.exp(-tt / 0.5) * make(src))
        return buf

    return pops


def mix_loop(tl, rendered, layers, mix):
    """A loop's mix: each synth stem's kept period (the start of the next
    crossfaded into its head, equal power: the stems are noise-like and
    uncorrelated across the seam), its sample peaks held STEM_CREST_DB over
    its RMS, and each composed layer (periodic already); each RMS-normalized
    and added at its level. Periodic by construction."""
    n, x = tl.n, int(LOOP_XF_S * SR)
    w = np.sin(0.5 * np.pi * (np.arange(x) + 0.5) / x)
    y = np.zeros(n)
    for name, gain in mix.items():
        if name in layers:
            core = layers[name].copy()
        else:
            s = rendered[name]
            if len(s) < 2 * n + x:
                s = np.pad(s, (0, 2 * n + x - len(s)))
            core = s[n:2 * n].copy()
            core[:x] = s[n:n + x] * w + s[2 * n:2 * n + x] * np.sqrt(1.0 - w * w)
            core -= np.mean(core)
            core = limit(core / (np.sqrt(np.mean(core ** 2)) + 1e-20), STEM_CREST_DB, win_s=0.001, loop=True)
        core -= np.mean(core)
        core /= np.sqrt(np.mean(core ** 2)) + 1e-20
        y += undb(gain) * core
    return y


def tame(x, loop=False):
    """Sample peaks held FINAL_CREST_DB over the loudest 5 ms, so the bodies
    of the crackles, not their first few samples, set the level."""
    loud = np.sqrt(np.max(uniform_filter1d(x ** 2, int(0.005 * SR), mode='wrap' if loop else 'nearest')))
    return limit(x, db(loud) + FINAL_CREST_DB, win_s=0.001, loop=loop)


SYNTH_LUA = r'''
local LOG = io.open(%(log)s, "w")
local function log(...)
  local t = {...}
  for i = 1, #t do t[i] = tostring(t[i]) end
  LOG:write(table.concat(t, "\t"), "\n"); LOG:flush()
end
local STEMS = %(stems)s
local function num(s)
  if not s then return nil end
  local v = s:match("[-+]?%%d+%%.?%%d*")
  if not v then return nil end
  v = tonumber(v)
  if s:find("kHz") then v = v * 1000 end
  return v
end
local function shown(tr, fx, p, nv)
  reaper.TrackFX_SetParamNormalized(tr, fx, p, nv)
  local _, s = reaper.TrackFX_GetFormattedParamValue(tr, fx, p, "")
  return s
end
local function table_of(tr, fx, p)
  -- the number a parameter shows at 401 normalized points, for inverse lookups
  local t = {}
  for k = 0, 400 do t[k] = num(shown(tr, fx, p, k / 400)) end
  return t
end
local function inv(t, target)
  -- the normalized value that shows target (linear between the sampled points;
  -- out of range: the nearest end)
  local best, bd = 0.5, 1e30
  for k = 0, 399 do
    local a, b = t[k], t[k + 1]
    if a and b and a ~= b and (a - target) * (b - target) <= 0 then
      return (k + (target - a) / (b - a)) / 400
    end
    if a and math.abs(a - target) < bd then bd = math.abs(a - target); best = k / 400 end
  end
  if t[400] and math.abs(t[400] - target) < bd then best = 1.0 end
  return best
end
local function set_enum(tr, fx, p, label)
  local hits = {}
  for k = 0, 200 do
    local s = shown(tr, fx, p, k / 200)
    if s:gsub("^%%s+", ""):gsub("%%s+$", "") == label then hits[#hits + 1] = k / 200 end
  end
  if #hits == 0 then error("no setting " .. label) end
  reaper.TrackFX_SetParamNormalized(tr, fx, p, hits[math.floor((#hits + 1) / 2)])
end

for _, st in ipairs(STEMS) do
  local ok, err = pcall(function()
    reaper.InsertTrackAtIndex(0, false)
    local tr = reaper.GetTrack(0, 0)
    local fx = reaper.TrackFX_AddByName(tr, st.synth, false, -1)
    if fx < 0 then error("plugin not found: " .. st.synth) end
    local idx = {}
    for p = 0, reaper.TrackFX_GetNumParams(tr, fx) - 1 do
      local _, pn = reaper.TrackFX_GetParamName(tr, fx, p, "")
      if idx[pn] == nil then idx[pn] = p end
    end
    local function pidx(key)
      if type(key) == "table" then
        local _, pn = reaper.TrackFX_GetParamName(tr, fx, key[1], "")
        if pn ~= key[2] then error("parameter " .. key[1] .. " is " .. pn .. ", not " .. key[2]) end
        return key[1]
      end
      local p = idx[key]
      if p == nil then error("no parameter " .. tostring(key)) end
      return p
    end
    for _, kv in ipairs(st.set) do
      local p = pidx(kv[1])
      if kv[2] == "norm" then reaper.TrackFX_SetParamNormalized(tr, fx, p, kv[3])
      elseif kv[2] == "enum" then set_enum(tr, fx, p, kv[3])
      else reaper.TrackFX_SetParamNormalized(tr, fx, p, inv(table_of(tr, fx, p), kv[3])) end
      local _, pn = reaper.TrackFX_GetParamName(tr, fx, p, "")
      local _, s = reaper.TrackFX_GetFormattedParamValue(tr, fx, p, "")
      log("SET", st.name, pn, s)
    end
    for _, au in ipairs(st.auto) do
      local p = pidx(au[1])
      local t = table_of(tr, fx, p)
      local env = reaper.GetFXEnvelope(tr, fx, p, true)
      if env == nil then error("no envelope for parameter " .. p) end
      local pts, first = au[2], nil
      for i = 1, #pts, 2 do
        local v = inv(t, pts[i + 1])
        if first == nil then first = v end
        reaper.InsertEnvelopePoint(env, pts[i], v, 0, 0, false, true)
      end
      reaper.Envelope_SortPoints(env)
      reaper.TrackFX_SetParamNormalized(tr, fx, p, first)
      log("AUTO", st.name, p, #pts / 2)
    end
    local item = reaper.CreateNewMIDIItemInProj(tr, 0, st.length, false)
    local take = reaper.GetActiveTake(item)
    local nn = st.notes
    for i = 1, #nn, 4 do
      reaper.MIDI_InsertNote(take, false, false, reaper.MIDI_GetPPQPosFromProjTime(take, nn[i]),
        reaper.MIDI_GetPPQPosFromProjTime(take, nn[i + 1]), 0, math.floor(nn[i + 2]), math.floor(nn[i + 3]), true)
    end
    reaper.MIDI_Sort(take)
    log("NOTES", st.name, #nn / 4)
    reaper.GetSetProjectInfo(0, "RENDER_SETTINGS", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_BOUNDSFLAG", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_STARTPOS", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_ENDPOS", st.length, true)
    reaper.GetSetProjectInfo(0, "RENDER_SRATE", %(sr)d, true)
    reaper.GetSetProjectInfo(0, "RENDER_CHANNELS", 2, true)
    reaper.GetSetProjectInfo(0, "RENDER_TAILFLAG", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_ADDTOPROJ", 0, true)
    reaper.GetSetProjectInfo_String(0, "RENDER_FORMAT", "ZXZhdyAAAA==", true)
    reaper.GetSetProjectInfo_String(0, "RENDER_FILE", st.outdir, true)
    reaper.GetSetProjectInfo_String(0, "RENDER_PATTERN", st.name, true)
    reaper.Main_OnCommand(42230, 0)
    log("RENDERED", st.name)
    reaper.DeleteTrack(tr)
  end)
  if not ok then log("ERR", st.name, tostring(err)) end
end
log("QUIT")
reaper.Main_openProject("noprompt:" .. %(empty)s)
reaper.Main_OnCommand(40004, 0)
'''


def _key(k):
    return list(k) if isinstance(k, tuple) else k


def render_synths(tmp, stems):
    """Render every stem: a track with one instrument, its settings, its
    automation (FX envelopes, the points given as displayed values), a MIDI
    item with its notes, rendered alone to a 32-bit float wav. Returns
    ({name: mono array}, log lines, status) or (None, ..., reason)."""
    ini = _private_reaper_res(tmp)
    jd = os.path.join(tmp, 'synth-job')
    od = os.path.join(tmp, 'synth-out')
    os.makedirs(jd, exist_ok=True)
    os.makedirs(od, exist_ok=True)
    empty = os.path.join(jd, 'empty.rpp')
    with open(empty, 'w') as fh:
        fh.write('<REAPER_PROJECT 0.1 "7.20/win64" 0\n  SAMPLERATE %d 1 0\n>\n' % SR)
    lj = []
    for s in stems:
        sets = []
        for k, v in s['set']:
            if isinstance(v, tuple):
                sets.append([_key(k), 'norm', v[1]])
            elif isinstance(v, str):
                sets.append([_key(k), 'enum', v])
            else:
                sets.append([_key(k), 'num', v])
        autos = [[_key(k), [float(u) for p in pts for u in p]] for k, pts, kind in s.get('auto', [])]
        notes = [float(u) for nt in s['notes'] for u in nt]
        lj.append({'name': s['name'], 'synth': s['synth'], 'set': sets, 'auto': autos, 'notes': notes,
                   'length': float(s['length']), 'outdir': od})
    log_path = os.path.join(jd, 'synth.log')
    lua = SYNTH_LUA % dict(log=_lua_value(log_path), stems=_lua_value(lj), sr=SR, empty=_lua_value(empty))
    lua_path = os.path.join(jd, 'synth_job.lua')
    with open(lua_path, 'w', encoding='utf-8') as fh:
        fh.write(lua)
    cmd = '"%s" -newinst -nosplash -ignoreerrors -cfgfile "%s" "%s"' % (REAPER_EXE, ini, lua_path)
    code, secs = _run_hidden(cmd, 900)
    lines = []
    if os.path.isfile(log_path):
        with open(log_path, encoding='utf-8', errors='replace') as fh:
            lines = fh.read().splitlines()
    errs = [ln for ln in lines if ln.startswith('ERR')]
    out = {}
    for s in stems:
        p = os.path.join(od, s['name'] + '.wav')
        if os.path.isfile(p):
            y = read_wav_mono(p)
            if np.max(np.abs(y)) < 1e-9:
                errs.append('ERR\t%s\tsilent' % s['name'])
            else:
                out[s['name']] = y
    status = 'REAPER synths on a private desktop, %.1f s, exit %s, %d of %d stems' % (
        secs, 'timeout' if code is None else '0x%08X' % code, len(out), len(stems))
    if errs or len(out) != len(stems):
        return None, lines, status + '; ' + '; '.join(errs[:6])
    return out, lines, status


# ---------------------------------------------------------------- loudness and levels

def _biquad_shelf(fs):
    G, Q, fc = 3.99984385397, 0.7071752369554193, 1681.9744509555319
    A = 10 ** (G / 40)
    w0 = 2 * np.pi * fc / fs
    al = np.sin(w0) / (2 * Q)
    c = np.cos(w0)
    b = [A * ((A + 1) + (A - 1) * c + 2 * np.sqrt(A) * al), -2 * A * ((A - 1) + (A + 1) * c),
         A * ((A + 1) + (A - 1) * c - 2 * np.sqrt(A) * al)]
    a = [(A + 1) - (A - 1) * c + 2 * np.sqrt(A) * al, 2 * ((A - 1) - (A + 1) * c),
         (A + 1) - (A - 1) * c - 2 * np.sqrt(A) * al]
    return np.array(b) / a[0], np.array(a) / a[0]


def _biquad_hp(fs):
    Q, fc = 0.5003270373253953, 38.13547087613982
    w0 = 2 * np.pi * fc / fs
    al = np.sin(w0) / (2 * Q)
    c = np.cos(w0)
    b = [(1 + c) / 2, -(1 + c), (1 + c) / 2]
    a = [1 + al, -2 * c, 1 - al]
    return np.array(b) / a[0], np.array(a) / a[0]


def lufs(x, loop=False):
    """Integrated loudness (ITU-R BS.1770-4: K-weighting, 400 ms blocks, 75 %
    overlap, absolute and relative gates). A loop is measured played twice so
    the filters start warm."""
    if loop:
        x = np.tile(x, 2)
    b1, a1 = _biquad_shelf(SR)
    b2, a2 = _biquad_hp(SR)
    k = signal.lfilter(b2, a2, signal.lfilter(b1, a1, x))
    if loop:
        k = k[len(k) // 2:]
    blk, hop = int(0.4 * SR), int(0.1 * SR)
    if len(k) < blk:
        k = np.pad(k, (0, blk - len(k)))
    ms = np.array([np.mean(k[i:i + blk] ** 2) for i in range(0, len(k) - blk + 1, hop)])
    l = -0.691 + 10 * np.log10(ms + 1e-20)
    ms = ms[l > -70]
    if len(ms) == 0:
        return -70.0
    rel = -0.691 + 10 * np.log10(np.mean(ms)) - 10
    ms = ms[(-0.691 + 10 * np.log10(ms)) > rel]
    return float(-0.691 + 10 * np.log10(np.mean(ms)))


def limit(x, ceiling_db, win_s=0.004, loop=False):
    """Peak limiter: the gain each sample needs, held over a window and
    smoothed over half of it (a smoothed gain never exceeds the held one, so
    no peak passes). A loop is limited circularly."""
    c = undb(ceiling_db)
    g = np.minimum(1.0, c / np.maximum(np.abs(x), 1e-12))
    w = max(3, int(win_s * SR))
    mode = 'wrap' if loop else 'nearest'
    g = minimum_filter1d(g, 2 * w + 1, mode=mode)
    g = uniform_filter1d(g, w, mode=mode)
    return x * g


def level_loop(x):
    """Gain to LOOP_LUFS, limited at LOOP_CEIL_DB; if that would take more
    than MAX_LIMIT_DB of limiting, the loop stays quieter instead."""
    x = x - np.mean(x)
    gain = LOOP_LUFS - lufs(x, loop=True)
    for _ in range(4):
        over = db(np.max(np.abs(x))) + gain - LOOP_CEIL_DB
        gain_used = gain - max(0.0, over - MAX_LIMIT_DB)
        y = limit(x * undb(gain_used), LOOP_CEIL_DB, loop=True)
        err = LOOP_LUFS - lufs(y, loop=True)
        if abs(err) < 0.1 or gain_used < gain:
            break
        gain += err
    return y


def finish_pop(x):
    """Trim the tail (60 dB under the peak, then 60 ms, so the fade falls on
    the tail and not on a last tick), keep 0.3 to 1.2 s, fade the last 40 ms,
    peak at POP_PEAK_DB."""
    a = np.abs(x)
    thr = np.max(a) * undb(-60)
    last = int(np.nonzero(a > thr)[0][-1]) + int(0.06 * SR)
    L = int(np.clip(last, POP_MIN_S * SR, POP_MAX_S * SR))
    y = np.zeros(L)
    m = min(L, len(x))
    y[:m] = x[:m]
    f = int(0.04 * SR)
    y[-f:] *= 0.5 + 0.5 * np.cos(np.linspace(0, np.pi, f))
    return y * undb(POP_PEAK_DB) / np.max(np.abs(y))


# ---------------------------------------------------------------- stage 2: REAPER

def _lua_value(v):
    if isinstance(v, str):
        return '"%s"' % v.replace('\\', '/').replace('"', '\\"')
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, (int, float, np.floating, np.integer)):
        return repr(float(v))
    if isinstance(v, dict):
        return '{' + ', '.join('%s = %s' % (k, _lua_value(x)) for k, x in v.items()) + '}'
    return '{' + ', '.join(_lua_value(x) for x in v) + '}'


LUA_JOB = r'''
local LOG = io.open(%(log)s, "w")
local function log(...)
  local t = {...}
  for i = 1, #t do t[i] = tostring(t[i]) end
  LOG:write(table.concat(t, "\t"), "\n"); LOG:flush()
end
local JOBS = %(jobs)s
local ok0, mode = reaper.GetAudioDeviceInfo("MODE")
log("AUDIO", tostring(mode))

local function num(s)
  if not s then return nil end
  local v = s:match("[-+]?%%d+%%.?%%d*")
  if not v then return nil end
  v = tonumber(v)
  if s:find("kHz") then v = v * 1000 end
  return v
end
local function shown(tr, fx, p, nv)
  reaper.TrackFX_SetParamNormalized(tr, fx, p, nv)
  local _, s = reaper.TrackFX_GetFormattedParamValue(tr, fx, p, "")
  return s
end
local function set_num(tr, fx, p, target)
  -- the ends of a range may read "Off": step inwards until both ends are numbers
  local lo, hi = 0.0005, 0.9995
  local a, b = num(shown(tr, fx, p, lo)), num(shown(tr, fx, p, hi))
  while a == nil and lo < 0.2 do lo = lo + 0.005; a = num(shown(tr, fx, p, lo)) end
  while b == nil and hi > 0.8 do hi = hi - 0.005; b = num(shown(tr, fx, p, hi)) end
  if a == nil or b == nil then error("not a number: " .. tostring(a) .. " " .. tostring(b)) end
  local inc = b > a
  for i = 1, 40 do
    local m = 0.5 * (lo + hi)
    local v = num(shown(tr, fx, p, m))
    if v == nil or ((v < target) == inc) then lo = m else hi = m end
  end
  reaper.TrackFX_SetParamNormalized(tr, fx, p, 0.5 * (lo + hi))
end
local function set_enum(tr, fx, p, label)
  local hits = {}
  for k = 0, 200 do
    local s = shown(tr, fx, p, k / 200)
    if s:gsub("^%%s+", ""):gsub("%%s+$", "") == label then hits[#hits + 1] = k / 200 end
  end
  if #hits == 0 then error("no setting " .. label) end
  reaper.TrackFX_SetParamNormalized(tr, fx, p, hits[math.floor((#hits + 1) / 2)])
end

for _, job in ipairs(JOBS) do
  local ok, err = pcall(function()
    reaper.InsertTrackAtIndex(0, false)
    local tr = reaper.GetTrack(0, 0)
    reaper.SetOnlyTrackSelected(tr)
    reaper.SetEditCurPos(0, false, false)
    reaper.InsertMedia(job.src, 0)
    local item = reaper.GetTrackMediaItem(tr, 0)
    if item == nil then error("media not inserted: " .. job.src) end
    local len = reaper.GetMediaItemInfo_Value(item, "D_LENGTH")
    reaper.SetMediaItemInfo_Value(item, "D_POSITION", 0)
    reaper.SetMediaItemInfo_Value(item, "D_FADEINLEN", 0)
    reaper.SetMediaItemInfo_Value(item, "D_FADEOUTLEN", 0)
    reaper.SetMediaItemInfo_Value(item, "D_FADEINLEN_AUTO", -1)
    reaper.SetMediaItemInfo_Value(item, "D_FADEOUTLEN_AUTO", -1)
    for _, fxs in ipairs(job.fx) do
      local fx = reaper.TrackFX_AddByName(tr, fxs.name, false, -1)
      if fx < 0 then error("plugin not found: " .. fxs.name) end
      local idx = {}
      for p = 0, reaper.TrackFX_GetNumParams(tr, fx) - 1 do
        local _, pn = reaper.TrackFX_GetParamName(tr, fx, p, "")
        if idx[pn] == nil then idx[pn] = p end
      end
      for _, kv in ipairs(fxs.params) do
        local p = idx[kv[1]]
        if p == nil then error("no parameter " .. kv[1] .. " on " .. fxs.name) end
        if kv[2] == "norm" then reaper.TrackFX_SetParamNormalized(tr, fx, p, kv[3])
        elseif kv[2] == "enum" then set_enum(tr, fx, p, kv[3])
        else set_num(tr, fx, p, kv[3]) end
        local _, s = reaper.TrackFX_GetFormattedParamValue(tr, fx, p, "")
        log("SET", job.name, fxs.name, kv[1], s, string.format("%%.5f", reaper.TrackFX_GetParamNormalized(tr, fx, p)))
      end
    end
    reaper.UpdateArrange()
    log("LEN", job.name, string.format("%%.4f", len))
    reaper.GetSetProjectInfo(0, "RENDER_SETTINGS", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_BOUNDSFLAG", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_STARTPOS", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_ENDPOS", len + job.tail, true)
    reaper.GetSetProjectInfo(0, "RENDER_SRATE", %(sr)d, true)
    reaper.GetSetProjectInfo(0, "RENDER_CHANNELS", 2, true)
    reaper.GetSetProjectInfo(0, "RENDER_TAILFLAG", 0, true)
    reaper.GetSetProjectInfo(0, "RENDER_ADDTOPROJ", 0, true)
    reaper.GetSetProjectInfo_String(0, "RENDER_FORMAT", "ZXZhdyAAAA==", true)
    reaper.GetSetProjectInfo_String(0, "RENDER_FILE", job.outdir, true)
    reaper.GetSetProjectInfo_String(0, "RENDER_PATTERN", job.name, true)
    reaper.Main_OnCommand(42230, 0)
    log("RENDERED", job.name)
    reaper.DeleteTrack(tr)
  end)
  if not ok then log("ERR", job.name, tostring(err)) end
end
log("QUIT")
reaper.Main_openProject("noprompt:" .. %(empty)s)
reaper.Main_OnCommand(40004, 0)
'''


def _run_hidden(cmd, timeout_s):
    """Start cmd on a private Windows desktop (no window shows or takes focus),
    wait up to timeout_s. Returns (exit code or None on timeout, seconds).
    On timeout only the process started here is terminated."""
    if os.name != 'nt':
        p = subprocess.Popen(cmd, shell=True)
        t0 = time.time()
        try:
            return p.wait(timeout_s), time.time() - t0
        except subprocess.TimeoutExpired:
            p.kill()
            return None, time.time() - t0
    from ctypes import wintypes as wt
    k32 = ctypes.WinDLL('kernel32', use_last_error=True)
    u32 = ctypes.WinDLL('user32', use_last_error=True)

    class STARTUPINFOW(ctypes.Structure):
        _fields_ = [('cb', wt.DWORD), ('lpReserved', wt.LPWSTR), ('lpDesktop', wt.LPWSTR), ('lpTitle', wt.LPWSTR),
                    ('dwX', wt.DWORD), ('dwY', wt.DWORD), ('dwXSize', wt.DWORD), ('dwYSize', wt.DWORD),
                    ('dwXCountChars', wt.DWORD), ('dwYCountChars', wt.DWORD), ('dwFillAttribute', wt.DWORD),
                    ('dwFlags', wt.DWORD), ('wShowWindow', wt.WORD), ('cbReserved2', wt.WORD),
                    ('lpReserved2', ctypes.c_void_p), ('hStdInput', wt.HANDLE), ('hStdOutput', wt.HANDLE),
                    ('hStdError', wt.HANDLE)]

    class PROCESS_INFORMATION(ctypes.Structure):
        _fields_ = [('hProcess', wt.HANDLE), ('hThread', wt.HANDLE), ('dwProcessId', wt.DWORD),
                    ('dwThreadId', wt.DWORD)]

    u32.CreateDesktopW.restype = wt.HANDLE
    u32.CreateDesktopW.argtypes = [wt.LPCWSTR, wt.LPCWSTR, ctypes.c_void_p, wt.DWORD, wt.DWORD, ctypes.c_void_p]
    u32.CloseDesktop.argtypes = [wt.HANDLE]
    k32.CreateProcessW.argtypes = [wt.LPCWSTR, wt.LPWSTR, ctypes.c_void_p, ctypes.c_void_p, wt.BOOL, wt.DWORD,
                                   ctypes.c_void_p, wt.LPCWSTR, ctypes.c_void_p, ctypes.c_void_p]
    k32.WaitForSingleObject.argtypes = [wt.HANDLE, wt.DWORD]
    k32.TerminateProcess.argtypes = [wt.HANDLE, wt.UINT]
    k32.GetExitCodeProcess.argtypes = [wt.HANDLE, ctypes.POINTER(wt.DWORD)]
    k32.CloseHandle.argtypes = [wt.HANDLE]
    name = 'udfiresnd%d' % os.getpid()
    hdesk = u32.CreateDesktopW(name, None, None, 0, 0x10000000, None)   # GENERIC_ALL
    si = STARTUPINFOW()
    si.cb = ctypes.sizeof(si)
    if hdesk:
        si.lpDesktop = name
    else:
        print('  (no private desktop, error %d: REAPER will show its window)' % ctypes.get_last_error())
    pi = PROCESS_INFORMATION()
    t0 = time.time()
    if not k32.CreateProcessW(None, ctypes.create_unicode_buffer(cmd), None, None, False, 0, None, None,
                              ctypes.byref(si), ctypes.byref(pi)):
        if hdesk:
            u32.CloseDesktop(hdesk)
        raise OSError('CreateProcess failed: %d' % ctypes.get_last_error())
    code = None
    if k32.WaitForSingleObject(pi.hProcess, int(timeout_s * 1000)) == 0:
        c = wt.DWORD()
        k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(c))
        code = c.value
    else:
        k32.TerminateProcess(pi.hProcess, 1)
        k32.WaitForSingleObject(pi.hProcess, 10000)
    k32.CloseHandle(pi.hThread)
    k32.CloseHandle(pi.hProcess)
    if hdesk:
        u32.CloseDesktop(hdesk)
    return code, time.time() - t0


def _private_reaper_res(tmp):
    """A REAPER settings folder of its own: the owner's plugin scan and licence
    copied (read only), audio forced to Dummy Audio, no splash or update check."""
    src = os.path.join(os.environ.get('APPDATA', ''), 'REAPER')
    res = os.path.join(tmp, 'reaper-res')
    os.makedirs(res, exist_ok=True)
    for f in REAPER_COPY:
        if os.path.isfile(os.path.join(src, f)):
            shutil.copyfile(os.path.join(src, f), os.path.join(res, f))
    ini_path = os.path.join(res, 'REAPER.ini')
    lines = []
    if os.path.isfile(ini_path):
        with open(ini_path, encoding='utf-8', errors='replace') as fh:
            lines = fh.read().splitlines()
    force = {'mode': '4', 'splash': '0', 'verchk': '0', 'loadlastproj': '0', 'lastproject': ''}
    out, seen = [], set()
    for ln in lines:
        k = ln.split('=', 1)[0]
        if '=' in ln and k in force:
            if k in seen:
                continue
            seen.add(k)
            out.append('%s=%s' % (k, force[k]))
        else:
            out.append(ln)
    missing = [k for k in force if k not in seen]
    if missing:
        if '[REAPER]' not in out:
            out.insert(0, '[REAPER]')
        i = out.index('[REAPER]') + 1
        for k in missing:
            out.insert(i, '%s=%s' % (k, force[k]))
    with open(ini_path, 'w', encoding='utf-8') as fh:
        fh.write('\n'.join(out) + '\n')
    return ini_path


def write_wav(path, x):
    wavfile.write(path, SR, np.asarray(x, np.float32))


def read_wav_mono(path):
    with warnings.catch_warnings():             # REAPER's bext chunk
        warnings.simplefilter('ignore', wavfile.WavFileWarning)
        sr, x = wavfile.read(path)
    if x.dtype.kind == 'i':
        x = x / float(2 ** (8 * x.dtype.itemsize - 1))
    x = np.asarray(x, np.float64)
    if x.ndim > 1:
        x = x.mean(axis=1)
    if sr != SR:
        raise ValueError('%s: %d Hz' % (path, sr))
    return x


def render_reaper(tmp, jobs):
    """Render every job through REAPER. jobs: list of (name, raw array, chain
    key, tail s). Returns ({name: processed mono array}, settings log lines,
    a status line) or (None, ..., reason)."""
    if not os.path.isfile(REAPER_EXE):
        return None, [], 'REAPER not found at %s' % REAPER_EXE
    ini = _private_reaper_res(tmp)
    if not os.path.isfile(os.path.join(os.path.dirname(ini), 'reaper-license.rk')):
        return None, [], 'no REAPER licence file to copy (an unlicensed REAPER blocks on its nag window)'
    jd = os.path.join(tmp, 'reaper-job')
    od = os.path.join(tmp, 'reaper-out')
    os.makedirs(jd, exist_ok=True)
    os.makedirs(od, exist_ok=True)
    empty = os.path.join(jd, 'empty.rpp')
    with open(empty, 'w') as fh:
        fh.write('<REAPER_PROJECT 0.1 "7.20/win64" 0\n  SAMPLERATE %d 1 0\n>\n' % SR)
    lua_jobs = []
    for name, raw, chain, tail in jobs:
        src = os.path.join(jd, name + '_raw.wav')
        write_wav(src, raw)
        fx = []
        for plug, params in CHAINS[chain]:
            ps = []
            for pn, v in params:
                if isinstance(v, tuple):
                    ps.append([pn, 'norm', v[1]])
                elif isinstance(v, str):
                    ps.append([pn, 'enum', v])
                else:
                    ps.append([pn, 'num', v])
            fx.append({'name': plug, 'params': ps})
        lua_jobs.append({'name': name, 'src': src, 'outdir': od, 'tail': tail, 'fx': fx})
    log_path = os.path.join(jd, 'job.log')
    lua = LUA_JOB % dict(log=_lua_value(log_path), jobs=_lua_value(lua_jobs), sr=SR, empty=_lua_value(empty))
    lua_path = os.path.join(jd, 'fire_job.lua')
    with open(lua_path, 'w', encoding='utf-8') as fh:
        fh.write(lua)
    cmd = '"%s" -newinst -nosplash -ignoreerrors -cfgfile "%s" "%s"' % (REAPER_EXE, ini, lua_path)
    code, secs = _run_hidden(cmd, 240)
    log_lines = []
    if os.path.isfile(log_path):
        with open(log_path, encoding='utf-8', errors='replace') as fh:
            log_lines = fh.read().splitlines()
    errs = [ln for ln in log_lines if ln.startswith('ERR')]
    out = {}
    for name, raw, chain, tail in jobs:
        p = os.path.join(od, name + '.wav')
        if os.path.isfile(p):
            y = read_wav_mono(p)
            if np.sqrt(np.mean(y ** 2)) < 1e-3 * np.sqrt(np.mean(raw ** 2)):
                errs.append('ERR\t%s\tthe render is silent (a plugin passing no sound)' % name)
            else:
                out[name] = y
    state = 'exit %s' % ('timeout' if code is None else '0x%08X' % code)
    status = 'REAPER headless on a private desktop, %.1f s, %s, %d of %d rendered' % (secs, state, len(out), len(jobs))
    if errs or len(out) != len(jobs):
        return None, log_lines, status + '; ' + '; '.join(errs)
    return out, log_lines, status


# ---------------------------------------------------------------- encoding and checks

def find_ffmpeg():
    for c in FFMPEG_CANDIDATES:
        p = c if os.path.isfile(c) else shutil.which(c)
        if p:
            return p
    sys.exit('ffmpeg not found')


def encode(ffmpeg, wav, out, codec_args):
    subprocess.run([ffmpeg, '-hide_banner', '-loglevel', 'error', '-y', '-i', wav, '-ac', '1', '-ar', str(SR)]
                   + codec_args + ['-map_metadata', '-1', out], check=True)


def decode(ffmpeg, path):
    r = subprocess.run([ffmpeg, '-hide_banner', '-loglevel', 'error', '-i', path, '-f', 'f32le', '-ac', '1',
                        '-ar', str(SR), '-'], check=True, capture_output=True)
    return np.frombuffer(r.stdout, np.float32).astype(np.float64)


def seam_report(x):
    """How the wrap from the last sample to the first compares with the rest of
    the loop: the step across it (as a percentile of all sample steps) and the
    high-frequency energy in a 5 ms window over it (as a percentile of the same
    window everywhere else). A click shows as a percentile near 100."""
    d = np.abs(np.diff(np.concatenate([x, x[:1]])))
    step = d[-1]
    step_pct = 100.0 * np.mean(d < step)
    hf = np.diff(np.concatenate([x[-1:], x]))         # hf[i] = x[i] - x[i-1], circular
    w = int(0.005 * SR)
    e = uniform_filter1d(hf ** 2, w, mode='wrap')
    win_pct = 100.0 * np.mean(e < e[0])
    return step, step_pct, win_pct


def spectrum_numbers(x):
    f, p = signal.welch(np.tile(x, 2) if len(x) < SR else x, SR, nperseg=8192)
    cen = float(np.sum(f * p) / np.sum(p))
    low = float(np.sum(p[(f >= 40) & (f <= 400)]) / np.sum(p))
    band = (f >= 30) & (f <= 500)
    med = signal.medfilt(p, 31)
    tonal = float(db(np.sqrt(np.max(p[band] / (med[band] + 1e-30)))))
    return cen, low, tonal


# ---------------------------------------------------------------- main

def plan(seed):
    """Every design and the stems stage 1 renders for them: (designs {sound:
    (chain, Timeline, stems, compose, mix)}, stems with their full names and
    lengths, the kits' note times)."""
    streams = [np.random.default_rng(s) for s in np.random.SeedSequence(seed).spawn(4)]
    designs = {'fire_crackle_loop': ('crackle',) + design_crackle(streams[0]),
               'fire_roar_loop': ('roar',) + design_roar(streams[1])}
    for i, (tl, compose) in enumerate(design_pops()):
        designs['fire_pop_%d' % (i + 1)] = ('pop', tl, [], compose, None)
    src_stems, index = design_sources()
    stems = [dict(s, name='src__' + s['name']) for s in src_stems]
    for snd, (chain, tl, st, compose, mix) in designs.items():
        stems += [dict(s, name=snd + '__' + s['name'], length=tl.length) for s in st]
    return designs, stems, index


def compose_all(seed, designs, index, rendered):
    """The rendered stems -> {sound: (chain, raw mix, loop?)}. The composing
    draws from its own seeded stream, in a fixed order."""
    rng = np.random.default_rng(np.random.SeedSequence(seed).spawn(5)[4])
    src = Source(rng, {k[5:]: v for k, v in rendered.items() if k.startswith('src__')}, index)
    raw = {}
    for snd, (chain, tl, st, compose, mix) in designs.items():
        if tl.loop:
            mine = {k.split('__', 1)[1]: v for k, v in rendered.items() if k.startswith(snd + '__')}
            x = mix_loop(tl, mine, compose(src), mix)
        else:
            x = compose(src)
        raw[snd] = (chain, x, tl.loop)
    return raw


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--out', default=os.path.dirname(os.path.abspath(__file__)),
                    help='output folder (default: beside this script)')
    ap.add_argument('--seed', type=int, default=1985)
    ap.add_argument('--keep-temp', action='store_true', help='keep the temp folder (stems, rendered wavs, the Lua jobs, logs)')
    ap.add_argument('--no-preview', action='store_true', help='do not write fire_preview.mp3')
    ap.add_argument('--reuse-synths', metavar='TEMP',
                    help='skip stage 1: take the synth stems from a temp folder an earlier run kept (--keep-temp)')
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    ffmpeg = find_ffmpeg()
    if not os.path.isfile(REAPER_EXE):
        sys.exit('REAPER not found at %s' % REAPER_EXE)
    designs, stems, index = plan(a.seed)
    tmp = tempfile.mkdtemp(prefix='ud_fire_sounds_')
    try:
        if a.reuse_synths:
            print('stage 1: the synth stems of an earlier run, from ' + a.reuse_synths)
            rendered = {s['name']: read_wav_mono(os.path.join(a.reuse_synths, 'synth-out', s['name'] + '.wav'))
                        for s in stems}
        else:
            print('stage 1: the synths (%d stems: Chromaphone 3, Mini V4)' % len(stems))
            rendered, lines, status = render_synths(tmp, stems)
            print('  ' + status)
            if rendered is None:
                sys.exit('the synth render failed:\n  ' + '\n  '.join(ln for ln in lines if ln.startswith('ERR')))
        raw = compose_all(a.seed, designs, index, rendered)
        for snd, (chain, tl, st, compose, mix) in designs.items():
            print('  %-18s %s' % (snd, ', '.join('%s %+.0f dB' % kv for kv in mix.items()) if mix else
                                  'composed from the kits and the noise'))

        # Level the raw material so no plugin sees an overload: loops near -22
        # LUFS, pops peaking at -9 dBFS; the final levels are set at the end.
        jobs = []
        for name, (chain, x, loop) in raw.items():
            if loop:
                x = x * undb(-22 - lufs(x, loop=True))
                jobs.append((name, np.tile(x, 3), chain, 0.0))
            else:
                x = x * undb(-9) / np.max(np.abs(x))
                jobs.append((name, np.concatenate([x, np.zeros(int(0.05 * SR))]), chain, 0.8))

        print('stage 2: effects')
        processed, log_lines, status = render_reaper(tmp, jobs)
        print('  ' + status)
        if processed is None:
            sys.exit('the effects render failed')
        print('  plugin settings, read back from the plugins (one job of each chain):')
        chain_of = {name: chain for name, x, chain, tail in jobs}
        shown, last = {}, None
        for ln in log_lines:
            p = ln.split('\t')
            if p[0] != 'SET' or shown.setdefault(chain_of[p[1]], p[1]) != p[1]:
                continue
            if (p[1], p[2]) != last:
                print('    %s: %s' % (chain_of[p[1]], p[2]))
                last = (p[1], p[2])
            print('      %-16s %s' % (p[3], p[4].strip()))

        print('finish, encode, check')
        results = []
        decoded = {}
        for name, (chain, x0, loop) in raw.items():
            y = processed[name]
            if loop:
                n = len(x0)
                y = y[:3 * n] if len(y) >= 3 * n else np.pad(y, (0, 3 * n - len(y)))
                period_err = db(np.sqrt(np.mean((y[n:2 * n] - y[2 * n:]) ** 2)) / np.sqrt(np.mean(y[n:2 * n] ** 2)))
                core = y[n:2 * n].copy()
                w = 0.5 - 0.5 * np.cos(np.linspace(0, np.pi, SEAM_XF))
                core[:SEAM_XF] = y[2 * n:2 * n + SEAM_XF] * (1 - w) + core[:SEAM_XF] * w
                fin = level_loop(tame(core, loop=True))
            else:
                period_err = None
                fin = finish_pop(tame(y))
            wav = os.path.join(tmp, name + '.wav')
            ogg = os.path.join(a.out, name + '.ogg')
            # Vorbis moves peaks by a few tenths of a dB: a pop is set again
            # from its decoded peak, a loop only if it went over -0.5 dBFS.
            for attempt in range(4):
                write_wav(wav, fin)
                encode(ffmpeg, wav, ogg, ['-c:a', 'libvorbis', '-q:a', str(VORBIS_Q)])
                dec = decode(ffmpeg, ogg)
                over = db(np.max(np.abs(dec))) - (POP_PEAK_DB if not loop else -0.5)
                if not ((not loop and abs(over) > 0.1) or (loop and over > 0)) or attempt == 3:
                    break
                fin = fin * undb(-over)
            decoded[name] = dec
            r = dict(name=name, kb=os.path.getsize(ogg) / 1024.0, secs=len(dec) / SR, samples_in=len(fin),
                     samples_out=len(dec), peak=db(np.max(np.abs(dec))), lufs=lufs(dec, loop=loop))
            r['centroid'], r['low'], r['tonal'] = spectrum_numbers(dec)
            if loop:
                r['period_err'] = period_err
                r['seam_pcm'] = seam_report(fin)
                r['seam_ogg'] = seam_report(dec)
                m = min(len(dec), len(fin))
                e = dec[:m] - fin[:m]
                w20 = int(0.02 * SR)
                r['edge_err'] = (db(np.sqrt(np.mean(e[:w20] ** 2))), db(np.sqrt(np.mean(e[-w20:] ** 2))),
                                 db(np.sqrt(np.mean(e ** 2))))
            results.append(r)

        if not a.no_preview:
            prev = make_preview(decoded)
            wav = os.path.join(tmp, 'fire_preview.wav')
            write_wav(wav, prev)
            mp3 = os.path.join(a.out, 'fire_preview.mp3')
            encode(ffmpeg, wav, mp3, ['-c:a', 'libmp3lame', '-q:a', '2'])
            print('  fire_preview.mp3  %.0f KB, %.1f s, %.1f LUFS' % (os.path.getsize(mp3) / 1024, len(prev) / SR, lufs(prev)))

        # tonal: the highest narrow peak over the local median spectrum, 30 to
        # 500 Hz (a hum would stand 15 dB or more out); a loop's figure only,
        # the pops ring on purpose.
        print('\n%-22s %6s %7s %8s %7s %9s %7s %7s' % ('file', 'KB', 's', 'peak dB', 'LUFS', 'centroid', '40-400', 'tonal'))
        for r in results:
            tonal = '%5.1fdB' % r['tonal'] if 'seam_ogg' in r else '    -'
            print('%-22s %6.1f %7.3f %8.2f %7.2f %7.0fHz %6.0f%% %7s' % (
                r['name'] + '.ogg', r['kb'], r['secs'], r['peak'], r['lufs'], r['centroid'], 100 * r['low'], tonal))
        for r in results:
            if 'seam_ogg' in r:
                sp, so = r['seam_pcm'], r['seam_ogg']
                print('%s: periods 2 and 3 of the render differ by %.1f dB; seam step %.4f (pctl %.1f), 5 ms HF window '
                      'pctl %.1f before encoding, step %.4f (pctl %.1f) window pctl %.1f after; Vorbis error first/last '
                      '20 ms %.1f/%.1f dB vs %.1f dB overall; samples %d -> %d' % (
                          r['name'], r['period_err'], sp[0], sp[1], sp[2], so[0], so[1], so[2],
                          r['edge_err'][0], r['edge_err'][1], r['edge_err'][2], r['samples_in'], r['samples_out']))
    finally:
        if a.keep_temp:
            print('temp kept: ' + tmp)
        else:
            shutil.rmtree(tmp, ignore_errors=True)


def make_preview(dec):
    """30 s: the crackle bed throughout, the roar fading in from 6 to 16 s, pops
    scattered over it."""
    L = int(30 * SR)
    t = np.arange(L) / SR
    bed = np.resize(dec['fire_crackle_loop'], L)
    roar = np.resize(dec['fire_roar_loop'], L)
    ramp = np.clip((t - 6.0) / 10.0, 0, 1)
    ramp = 0.5 - 0.5 * np.cos(np.pi * ramp)
    y = undb(-1) * bed + undb(-1) * ramp * roar
    for k, (ts, g) in enumerate([(2.3, -6), (4.9, -9), (8.1, -5), (11.6, -8), (14.2, -7), (17.9, -6),
                                 (21.4, -4), (24.0, -8), (26.8, -5)]):
        p = dec['fire_pop_%d' % (k % 6 + 1)]
        add_at(y, int(ts * SR), undb(g) * p)
    y *= undb(-14 - lufs(y))
    y = limit(y, -1.0)
    f = int(0.5 * SR)
    y[:f] *= np.linspace(0, 1, f)
    y[-f:] *= np.linspace(1, 0, f)
    return y


if __name__ == '__main__':
    main()
