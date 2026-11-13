r"""Auditions factory presets without ears: plays each one the same way, then
prints numbers per stretch of the note and draws a picture.

    python audition.py "Augmented BRASS" "Growly Horn" "Dreary Tales" ... [--notes 36,48] [--tag brass1]

Each preset gets its own track in one REAPER start and is rendered alone.
The phrase: every note is held 4.5 s; over the hold the Morph macro goes from
0 to 1 and the mod wheel from 0 to full in the last 2 s, so one render shows
what the preset can do. The plugin's own reverb and delay are turned off.
Output: renders\aud_<tag>_<preset>.wav, dumps\aud_<tag>.txt and pictures
under renders\pictures.
"""
import argparse
import os
import re

import numpy as np

import sndlib as S
from arturia import chunk
from rjob import Job, NOTES

VST = {
    'Augmented VOICES': 'VST3i: Augmented VOICES (Arturia)',
    'Augmented BRASS': 'VST3i: Augmented BRASS (Arturia)',
    'Augmented WOODWINDS': 'VST3i: Augmented WOODWINDS (Arturia)',
    'Augmented STRINGS': 'VST3i: Augmented STRINGS (Arturia)',
    'CS-80 V4': 'VST3i: CS-80 V4 (Arturia)',
    'ARP 2600 V3': 'VST3i: ARP 2600 V3 (Arturia)',
    'Buchla Easel V': 'VST3i: Buchla Easel V (Arturia)',
    'Synthi V': 'VST3i: Synthi V (Arturia)',
    'CMI V': 'VST3i: CMI V (Arturia)',
    'Pigments': 'VST3i: Pigments (Arturia)',
    'Mellotron V': 'VST3i: Mellotron V (Arturia)',
    'Modular V3': 'VST3i: Modular V3 (Arturia)',
    'KORG MS-20 V': 'VST3i: KORG MS-20 V (Arturia)',
}


def slug(s):
    return re.sub(r'[^A-Za-z0-9]+', '_', s).strip('_')


def audition(inst, presets, notes=(36, 48), tag=None, hold=4.5, augmented=None, timeout=300):
    tag = tag or slug(inst)
    augmented = inst.startswith('Augmented') if augmented is None else augmented
    j = Job('aud_' + tag)
    length = 0.5 + len(notes) * (hold + 1.5)
    tracks = []
    for p in presets:
        t = j.track(p)
        f = j.fx(t, VST[inst], chunk=chunk(inst, p))
        nts, evs, morph = [], [], []
        for i, n in enumerate(notes):
            a = 0.5 + i * (hold + 1.5)
            nts.append((a, a + hold, n, 115))
            morph += [(a, 0.0), (a + 0.3, 0.0), (a + hold - 0.5, 1.0), (a + hold + 1.2, 1.0), (a + hold + 1.3, 0.0)]
            evs.append((a - 0.2, 0xB0, 1, 0))
            for k in range(21):
                evs.append((a + hold - 2.0 + k * 0.09, 0xB0, 1, int(127 * k / 20)))
            evs.append((a + hold + 1.3, 0xB0, 1, 0))
        if augmented:
            j.set(t, f, 'Reverb', 0.0)
            j.set(t, f, 'Delay', 0.0)
            j.env(t, f, 'Morph', morph)
        j.midi(t, length, notes=nts, events=evs)
        tracks.append(t)
    wavs = []
    for i, t in enumerate(tracks):
        for k, u in enumerate(tracks):
            j.mute(u, k != i)
        wavs.append(j.render(length, name=f'aud_{tag}_{slug(presets[i])}'))
    code = j.run(timeout=timeout)
    os.makedirs(os.path.join(NOTES, 'renders', 'pictures'), exist_ok=True)
    lines = []
    for p, w in zip(presets, wavs):
        if not os.path.exists(w):
            lines.append(f'{p}: NO RENDER')
            continue
        x, loss = S.load_mono(w)
        if np.max(np.abs(x)) < 1e-5:
            lines.append(f'{p}: SILENT')
            continue
        lines.append(f'== {inst} / {p}   (mono fold loss {loss:.1f} dB)')
        for i, n in enumerate(notes):
            a = 0.5 + i * (hold + 1.5)
            for (lab, t0, t1) in (('start', a, a + 1.5), ('mid', a + 1.5, a + 3.0), ('morphed+wheel', a + 3.0, a + hold)):
                seg = x[int(t0 * S.SR): int(t1 * S.SR)]
                lines.append(f'   note {n} {lab:14s} {S.describe(seg)}')
        S.picture([(x, S.SR, f'{inst} / {p}')], os.path.join(NOTES, 'renders', 'pictures', f'aud_{tag}_{slug(p)}.png'))
    txt = '\n'.join(lines)
    with open(os.path.join(NOTES, 'dumps', f'aud_{tag}.txt'), 'w', encoding='utf-8') as fh:
        fh.write(txt + '\n')
    print(txt)
    return code


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('inst')
    ap.add_argument('presets', nargs='+')
    ap.add_argument('--notes', default='36,48')
    ap.add_argument('--tag', default=None)
    ap.add_argument('--timeout', type=int, default=300)
    a = ap.parse_args()
    audition(a.inst, a.presets, tuple(int(n) for n in a.notes.split(',')), a.tag, timeout=a.timeout)
