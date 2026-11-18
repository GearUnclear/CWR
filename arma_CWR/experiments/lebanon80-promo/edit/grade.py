"""The transfer of build.py's grade, measured: a grey ramp pushed through the same filters.

    python grade.py

The ramp (RGB grey 0..255) is encoded like a clip (rgb24 to yuv420p, bt601 limited range) and then graded
with build.grade_chain for the night chain and for day gammas 0.88 and 1.00 (the clamp range of the
exposure match), vignette and grain left out because they depend on position and time. Prints the source Y
and the graded Y per grey level. The rule it checks: the grade never lifts black (a graded Y may not sit
above the source Y at the dark end). This is how the first cut's lift was found: its curve started at
0/0.012 with grain c0s=4 and a match that could brighten (gamma up to 1.30), which put the night's darkest
tenth well over black. Run it again after any change to the grade in build.py.
"""
import subprocess
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build import grade_chain  # noqa: E402
from promopaths import FF  # noqa: E402

RAMP = np.repeat(np.arange(256, dtype=np.uint8)[None, :, None], 16, 0).repeat(3, 2).tobytes()


def run(vf):
    r = subprocess.run([FF, '-v', 'error', '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-s', '256x16', '-i', '-', '-vf', vf,
                        '-frames:v', '1', '-f', 'rawvideo', '-pix_fmt', 'yuv420p', '-'], input=RAMP, capture_output=True)
    return np.frombuffer(r.stdout, np.uint8, 256 * 16).reshape(16, 256)[8].astype(int)


def chain(night, gamma):
    parts = [p for p in grade_chain(night, gamma, 0, 0, 1).split(',') if not p.startswith(('vignette', 'noise'))]
    return 'format=yuv420p,' + ','.join(parts)


def main():
    src = run('format=yuv420p')
    out = [(n, run(c)) for n, c in (('night', chain(True, 1.0)), ('day 0.88', chain(False, 0.88)),
                                    ('day 1.00', chain(False, 1.0)))]
    print('grey  srcY | ' + ' '.join('%8s' % n for n, _ in out))
    for v in (0, 2, 4, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 96, 128, 192, 255):
        print('%4d  %4d | ' % (v, src[v]) + ' '.join('%8d' % y[v] for _, y in out))
    lifted = [(n, v, int(src[v]), int(y[v])) for n, y in out for v in range(33) if y[v] > src[v]]
    if lifted:
        print('LIFT at the dark end (chain, grey, src Y, graded Y):', lifted[:12])
        return 1
    print('no lift: every graded Y at grey 0..32 is at or under its source')
    return 0


if __name__ == '__main__':
    sys.exit(main())
