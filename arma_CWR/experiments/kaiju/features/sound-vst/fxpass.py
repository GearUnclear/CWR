r"""Runs a wav through effect chains in headless REAPER and returns each chain's
output on its own (100 percent processed where the effect allows), so the
build can blend them by number afterwards.

    from fxpass import run_chains
    outs = run_chains('roar_a_bus', wav_path, ['burner', 'plate', 'ring', 'raum'], tail=6)
    # -> {'burner': '...\renders\fx\roar_a_bus__burner.wav', ...}

A chain is a list of (plugin name, {'preset': Arturia factory preset name,
'params': {index or name: normalized value}}).
"""
import os

import sndlib as S
from arturia import chunk
from rjob import Job, NOTES

FX = os.path.join(NOTES, 'renders', 'fx')

COLDFIRE = 'VST3: Dist COLDFIRE (Arturia)'
OBJEQ = 'VST3: Objeq Delay (Applied Acoustics Systems)'
FREAK = 'VST3: Freak (Native Instruments)'
RAUM = 'VST3: Raum (Native Instruments)'
INTENSITY = 'VST3: Rev INTENSITY (Arturia)'
FRAGMENTS = 'VST3: Efx FRAGMENTS (Arturia)'
MS20 = 'VST3: Filter MS-20 (Arturia)'
DIRT = 'VST3: Dirt (Native Instruments)'
TAPE201 = 'VST3: Delay TAPE-201 (Arturia)'
PLATE140 = 'VST3: Rev PLATE-140 (Arturia)'


def objeq(kind, freq, decay, material=0.3, formant=0.5):
    """Objeq Delay used as a plain resonating object: no LFO, no filter, no
    delay line, processed signal only. kind: 0 beam, 1/3 drumhead, 2/3 plate, 1 string.
    freq: 0..1 over 20 Hz..4 kHz (0.2 = 58 Hz, 0.4 = 167 Hz, 0.6 = 480 Hz)."""
    return (OBJEQ, {'params': {11: 0.0, 16: 0.0, 36: 0.0, 22: 1.0, 18: kind, 17: freq, 19: decay,
                               20: material, 21: formant, 37: 1.0, 38: 0.55}})


def raum(mode, size, decay, predelay=0.1, damp=0.4, highcut=0.6):
    """Raum reverb, wet only. mode: 0 grounded, 0.5 airy, 1 cosmic. decay: 0.5 = 2.8 s, 0.8 = 10 s."""
    return (RAUM, {'params': {8: mode, 10: size, 11: decay, 1: predelay, 14: damp, 6: highcut, 7: 1.0, 9: 0.7,
                              13: 0.3}})


def freak(freq, harmonics=0.2, feedback=0.0, kind=0.5, mode=0.5):
    """Freak ring modulator / frequency shifter. freq: 0.4 = -40 Hz, 0.6 = +40 Hz, 0.8 = 1.08 kHz (coarse)."""
    return (FREAK, {'params': {0: 1.0, 1: freq, 5: kind, 6: harmonics, 7: feedback, 8: 1.0, 9: mode}})


CHAINS = {
    # distortion with comb filters: "tortures incoming signal and creates resonant sounds" (factory text)
    'burner': [(COLDFIRE, {'preset': 'Scream Burner'})],
    'metallurgy': [(COLDFIRE, {'preset': 'Industrial Metallurgy'})],
    'shredded': [(COLDFIRE, {'preset': 'Completely Shredded'})],
    'void': [(COLDFIRE, {'preset': 'Voice From The Void'})],
    'destroyer': [(COLDFIRE, {'preset': 'The Destroyer'})],
    'heavy': [(COLDFIRE, {'preset': 'Heavy But Complex'})],
    'radiofail': [(COLDFIRE, {'preset': 'Radio Fail'})],
    'radioscream': [(COLDFIRE, {'preset': 'Radio Scream'})],
    'longdistance': [(COLDFIRE, {'preset': 'Long Distance Voice'})],
    # the sound ringing a big metal plate / a girder / a hollow drum
    'plate': [objeq(2 / 3, 0.27, 0.8, 0.35)],
    'plate_hi': [objeq(2 / 3, 0.5, 0.7, 0.5)],
    'beam': [objeq(0.0, 0.33, 0.75, 0.4)],
    'drum': [objeq(1 / 3, 0.2, 0.7, 0.25)],
    # ring modulation: low (a gargling rattle on the voice) and high (metal sidebands)
    'ring_low': [freak(0.62, 0.3, 0.2)],
    'ring_hi': [freak(0.72, 0.4, 0.3)],
    'radio': [freak(0.7, 0.3, 0.0, 0.5, 0.0)],
    # space
    'raum_big': [raum(0.5, 1.0, 0.72, 0.15, 0.45, 0.55)],
    'raum_ground': [raum(0.0, 0.9, 0.62, 0.1, 0.5, 0.5)],
    'raum_cosmic': [raum(1.0, 1.0, 0.8, 0.2, 0.4, 0.5)],
    'misty': [(INTENSITY, {'preset': 'Misty Mountains'})],
    'aftermath': [(INTENSITY, {'preset': 'Aftermath'})],
    # the burner feeding a plate, then a big space: the "furnace" chain
    'furnace': [(COLDFIRE, {'preset': 'Scream Burner'}), objeq(2 / 3, 0.3, 0.75, 0.4)],
    'grain_dark': [(FRAGMENTS, {'preset': 'Dark Cloud'})],
    'grain_gristle': [(FRAGMENTS, {'preset': 'Gristle'})],
}

PRESET_FOLDER = {COLDFIRE: 'Dist COLDFIRE', INTENSITY: 'Rev INTENSITY', FRAGMENTS: 'Efx FRAGMENTS',
                 MS20: 'Filter MS-20', TAPE201: 'Delay TAPE-201', PLATE140: 'Rev PLATE-140'}


def run_chains(tag, wav, names, tail=4.0, force=False, timeout=600, chains=None):
    os.makedirs(FX, exist_ok=True)
    chains = chains or CHAINS
    outs = {n: os.path.join(FX, f'{tag}__{n}.wav') for n in names}
    todo = [n for n in names if force or not os.path.exists(outs[n])]
    if not todo:
        return outs
    x, sr = S.load(wav)
    length = len(x) / sr + tail
    j = Job('fx_' + tag)
    tracks = []
    for n in todo:
        t = j.track(n)
        j.audio(t, wav, 0.0)
        for (plug, opt) in chains[n]:
            f = j.fx(t, plug, chunk=chunk(PRESET_FOLDER[plug], opt['preset']) if opt.get('preset') else None)
            for k, v in (opt.get('params') or {}).items():
                j.set(t, f, k, v)
        tracks.append(t)
    for i, n in enumerate(todo):
        for k, u in enumerate(tracks):
            j.mute(u, k != i)
        j.render(length, name=os.path.join('fx', f'{tag}__{n}'))
    j.run(timeout=timeout)
    return outs
