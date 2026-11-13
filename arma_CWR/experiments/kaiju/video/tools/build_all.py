"""The whole edit, in order. No game is launched.

    python build_all.py [--no-phone] [--reuse-segments] [--no-strips]

make_edl (the cut, from cut.py) -> make_cards (cards, captions, gate, dust) -> mix_audio -> assemble
-> phone -> review.
"""
import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def step(name, *args):
    print('== %s %s' % (name, ' '.join(args)), flush=True)
    r = subprocess.run([sys.executable, os.path.join(HERE, name)] + list(args), cwd=HERE)
    if r.returncode != 0:
        raise SystemExit('%s failed' % name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--no-phone', action='store_true')
    ap.add_argument('--reuse-segments', action='store_true')
    ap.add_argument('--no-strips', action='store_true')
    a = ap.parse_args()
    step('make_edl.py')
    step('make_cards.py')
    step('mix_audio.py')
    step('assemble.py', *(['--reuse-segments'] if a.reuse_segments else []))
    if not a.no_phone:
        step('phone.py')
    step('review.py', *(['--no-strips'] if a.no_strips else []))
    step('make_script.py')


if __name__ == '__main__':
    main()
