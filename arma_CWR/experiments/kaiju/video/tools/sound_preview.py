"""A preview, for the owner's ear, of every sound the edit makes from nothing (the editor cannot hear).

    python sound_preview.py

Writes final\\made_here_preview.mp3: wind, sea, camera motor, tank gun near, tank gun far, shell burst,
house coming down, the sub hit, then the whole end hit as the film stacks it; a beep between each.
"""
import os

import numpy as np
import soundfile as sf

import synth
from edit_common import FINAL, SR, WORK, ff


def main():
    gap = np.zeros(int(0.35 * SR))
    beep = synth.beep(0.12, 880) * 0.3
    parts = [
        ('wind bed', synth.wind(5, 11) * 0.5),
        ('sea bed', synth.sea(5, 21) * 0.5),
        ('camera motor', synth.motor(4, 31) * 0.3),
        ('tank gun, near', synth.gun(False, 101)),
        ('tank gun, far', synth.gun(True, 102)),
        ('shell burst', synth.burst(103)),
        ('house coming down', synth.collapse(104)),
        ('sub hit', synth.subhit()),
    ]
    # the end hit as stacked under the brand card
    n = int(4 * SR)
    hit = np.zeros(n)
    for x, g, at in ((synth.subhit(), 1.0, 0), (synth.load('step_3'), 0.9, 0), (synth.load('step_2'), 0.6, 0.02), (synth.load('beam')[:int(1.4 * SR)], 0.7, 0)):
        k = int(at * SR)
        m = min(len(x), n - k)
        hit[k:k + m] += x[:m] * g
    hit = hit / np.abs(hit).max() * 0.97
    parts.append(('the end hit, stacked', hit))
    out = []
    names = []
    t = 0.0
    for name, x in parts:
        out += [beep, gap, x, gap]
        names.append('%5.1f s  %s' % (t + 0.47, name))
        t += (len(beep) + 2 * len(gap) + len(x)) / SR
    y = np.concatenate(out)
    wav = os.path.join(WORK, 'made_here_preview.wav')
    sf.write(wav, y, SR)
    ff('-i', wav, '-c:a', 'libmp3lame', '-b:a', '192k', os.path.join(FINAL, 'made_here_preview.mp3'))
    open(os.path.join(FINAL, 'made_here_preview.txt'), 'w').write('Sounds made here from nothing (tools/synth.py), in the order they play:\n' + '\n'.join(names) + '\n')
    print('\n'.join(names))


if __name__ == '__main__':
    main()
