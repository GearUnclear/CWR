r"""Probes plugins headless: can REAPER load it without a window blocking, how
many parameters and host presets does it show, and does a held note make
sound.

    python probe.py reasynth
    python probe.py inst "VST3i: Augmented VOICES (Arturia)" [tag] [--note 36] [--timeout 240]
    python probe.py fx   "VST3: Dist COLDFIRE (Arturia)" [tag]

Each probe is its own REAPER start, so a plugin that blocks only costs its
own timeout. Results: dumps\<tag>.txt (parameters), renders\probe_<tag>.wav,
and one line appended to dumps\PROBES.txt.
"""
import argparse
import os
import re
import sys

import numpy as np
import soundfile as sf

from rjob import Job, NOTES


def tag_of(name):
    m = re.match(r'^[A-Za-z0-9]+i?: (.*?)( \(.*\))?$', name)
    return re.sub(r'[^A-Za-z0-9]+', '_', (m.group(1) if m else name)).strip('_')


def describe(wav):
    if not os.path.exists(wav):
        return 'NO RENDER'
    x, sr = sf.read(wav)
    if x.ndim == 1:
        x = x[:, None]
    peak = float(np.max(np.abs(x)))
    if peak < 1e-5:
        return 'SILENT'
    m = x.mean(axis=1)
    k = sr // 2
    env = [20 * np.log10(np.sqrt(np.mean(m[i:i + k] ** 2)) + 1e-9) for i in range(0, len(m) - k + 1, k)]
    spec = np.abs(np.fft.rfft(m)) ** 2
    f = np.fft.rfftfreq(len(m), 1 / sr)
    cen = float(np.sum(f * spec) / (np.sum(spec) + 1e-20))
    return f'peak {peak:.3f} centroid {cen:.0f} Hz env(0.5s dB) ' + ' '.join(f'{e:.0f}' for e in env)


def probe(kind, name, tag=None, note=36, timeout=240):
    tag = tag or tag_of(name)
    j = Job('probe_' + tag)
    t = j.track('p')
    if kind == 'fx':
        j.fx(t, 'ReaSynth', params={'Saw mix': 0.6})
    f = j.fx(t, name)
    j.dump(t, f, tag)
    j.midi(t, 6, notes=[(0.5, 4.0, note, 110)])
    wav = j.render(6)
    if os.path.exists(wav):
        os.remove(wav)
    code = j.run(timeout=timeout)
    res = describe(wav)
    nparam = '?'
    dump = os.path.join(NOTES, 'dumps', tag + '.txt')
    if os.path.exists(dump):
        with open(dump, encoding='utf-8', errors='replace') as fh:
            lines = fh.read().splitlines()
        nparam = len(lines) - 2
    line = f'{name}\texit {code}\tparams {nparam}\t{res}'
    print(line)
    with open(os.path.join(NOTES, 'dumps', 'PROBES.txt'), 'a', encoding='utf-8') as fh:
        fh.write(line + '\n')
    return code


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('kind')
    ap.add_argument('name', nargs='?', default='ReaSynth')
    ap.add_argument('tag', nargs='?', default=None)
    ap.add_argument('--note', type=int, default=36)
    ap.add_argument('--timeout', type=int, default=240)
    a = ap.parse_args()
    if a.kind == 'reasynth':
        sys.exit(probe('inst', 'ReaSynth', 'ReaSynth', 45, 90))
    sys.exit(probe(a.kind, a.name, a.tag, a.note, a.timeout))
