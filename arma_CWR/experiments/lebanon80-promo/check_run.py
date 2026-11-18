"""Checks one promo run's log: pin, settle and recording state of every clip.

    python check_run.py <run folder> [...]

For every clip (a "PROMO rec <name>" line) the last triSettleScene before it must read "settled" with
the finest detail and the pin, "lod 0.010416667, pin 1" at 1920 wide or "lod 0.0078125, pin 1" at 2560
(popin-diagnosis.md; the width is lodCoef 10 x 2 / window width), and the clip must end "OK:written". Also lists script
errors and the window size. Prints one line per clip and exits 1 when any clip fails.
"""
import glob
import os
import re
import sys

ANSI = re.compile(r'\x1b\[[0-9;]*m')


def lines_of(run):
    for f in sorted(glob.glob(os.path.join(run, '**', 'game_stdout.log'), recursive=True)):
        with open(f, encoding='utf-8', errors='replace') as fh:
            for ln in fh:
                yield ANSI.sub('', ln.rstrip('\n'))


def check(run):
    settle = None
    bad = 0
    out = []
    window = pin = None
    errors = []
    for ln in lines_of(run):
        if 'PROMO window' in ln:
            window = ln.split('PROMO window', 1)[1].strip(' "')
        if 'PROMO pin' in ln:
            pin = ln.split('PROMO pin', 1)[1].strip(' "')
        if '[tri] triSettleScene:' in ln:
            settle = ln.split('[tri] triSettleScene:', 1)[1].strip()
        if re.search(r'\bError\b|error in expression|Undefined variable', ln) and 'SCRIPT' in ln:
            errors.append(ln[-220:])
        m = re.search(r'"PROMO rec (\S+) ', ln)
        if m:
            name = m.group(1)
            # the finest object detail is lodCoef 10 x 2 / window width: 0.010416667 at 1920, 0.0078125 at 2560
            w = int(window.split('x')[0]) if window and 'x' in window else 1920
            finest = {1920: '0.010416667', 2560: '0.0078125'}.get(w, '%.8g' % (20.0 / w))
            ok = bool(settle) and settle.startswith('settled') and ('lod %s, pin 1' % finest) in settle
            out.append([name, ok, settle or 'NO SETTLE', None])
            settle = None
        m = re.search(r'"PROMO done (\S+) (\S+)', ln)
        if m:
            for row in out:
                if row[0] == m.group(1):
                    row[3] = m.group(2).startswith('OK:written')
    print('%s: window %s, pin %s' % (os.path.basename(run.rstrip('\\/')), window, pin))
    for name, ok, st, wr in out:
        fine = ok and wr
        bad += 0 if fine else 1
        print('  %-4s %-55s %s%s' % ('ok' if fine else 'FAIL', name, st[:110], '' if wr else '  [not written]'))
    for e in errors[:10]:
        print('  script error:', e)
    return bad


if __name__ == '__main__':
    n = sum(check(r) for r in sys.argv[1:])
    sys.exit(1 if n else 0)
