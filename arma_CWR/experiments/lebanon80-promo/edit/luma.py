"""Per-shot luma of the final cut, read against the EDL: the check behind the night rules.

    python luma.py [<final mp4>]        default <UD_PROMO_DIR>/ud-lebanon80-promo-1080p.mp4

Decodes the cut at 480 x 270 and prints, per shot of work/edit/edl.json: mean Y, its normalised value,
the 1st, 10th, 50th, 90th and 99th percentile and the share of the frame under 3 %, under 8 % and over
50 % of the range (limited range, so black is Y 16). A true-night shot should sit near black with its
light in a small pool; a night shot whose p10 is well above 16 to 17 was lifted somewhere (the grade, an
exposure match, a moon) and fails the night rules in ../README.md. Saves the per-frame table as
work/edit/luma.npy.
"""
import json
import subprocess
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from promopaths import FF, PROMO  # noqa: E402

V = Path(sys.argv[1]) if len(sys.argv) > 1 else PROMO / "ud-lebanon80-promo-1080p.mp4"
EDIT = PROMO / "work" / "edit"
EDL = EDIT / "edl.json"
W, H = 480, 270


def main():
    p = subprocess.Popen([FF, '-v', 'error', '-i', str(V), '-vf', f'scale={W}:{H}:flags=area', '-pix_fmt', 'yuv420p',
                          '-f', 'rawvideo', '-'], stdout=subprocess.PIPE)
    fs = W * H * 3 // 2
    frames = []
    while True:
        b = p.stdout.read(fs)
        if len(b) < fs:
            break
        y = np.frombuffer(b, np.uint8, W * H)
        n = (y.astype(np.float32) - 16) / 219
        frames.append((y.mean(), np.percentile(y, 1), np.percentile(y, 10), np.percentile(y, 50), np.percentile(y, 90),
                       np.percentile(y, 99), (n < 0.03).mean(), (n < 0.08).mean(), (n > 0.5).mean()))
    A = np.array(frames)
    EDIT.mkdir(parents=True, exist_ok=True)
    np.save(EDIT / 'luma.npy', A)
    edl = json.loads(EDL.read_text())
    shots = edl['shots'] if isinstance(edl, dict) else edl
    f0 = 0
    print('n  t      clip   frames gamma | Ymean  norm  | p1 p10 p50 p90 p99 | <3% <8% >50%')
    for s in shots:
        n = s['frames']
        m = A[f0:f0 + n].mean(0)
        print('%2d %7.2f %-6s %4d %5s | %5.1f %5.3f | %3.0f %3.0f %3.0f %3.0f %3.0f | %4.2f %4.2f %4.2f%s' % (
            s.get('n', 0), f0 / 30, s['clip'][:6], n, s.get('gamma_used', '-'), m[0], (m[0] - 16) / 219, m[1], m[2], m[3],
            m[4], m[5], m[6], m[7], m[8], '  N' if s.get('night') else ''))
        f0 += n
    print(len(A), 'frames decoded,', f0, 'in the EDL')


if __name__ == '__main__':
    main()
