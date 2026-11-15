"""Encodes the finished slots of a take while the take is still running, so a long take never holds
more than one slot's frames on disk.

    python encode_live.py <tag prefix, for example main2>

Waits for kaiju-notes\\video\\runs\\<tag>-* to appear, then every 20 s reads the game log: a slot is
finished when a later slot has started (a KVC line of another slot). Each
finished slot is handed to encode_shots.py --only <slot> (mp4 per camera, frames deleted). Ends when
the take has ended; the queue's own encode then finds nothing left to do but the logs.
"""
import glob
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
RUNS = r'D:\Arma_CWA_worktrees\kaiju-notes\video\runs'


def main():
    tag = sys.argv[1]
    run = None
    while run is None:
        c = sorted(glob.glob(os.path.join(RUNS, tag + '-*')))
        if c:
            run = c[-1]
        else:
            time.sleep(10)
    print('watching', run, flush=True)
    done = []
    idle = 0
    while True:
        logs = glob.glob(os.path.join(run, '**', 'game_stdout.log'), recursive=True)
        order = []
        ended = False
        if logs:
            for line in open(logs[0], encoding='utf-8', errors='replace'):
                m = re.search(r'KVC (\S+?)-\S+ eye', line)
                if m and m.group(1) not in order:
                    order.append(m.group(1))
                if 'KV END' in line:
                    ended = True
        finished = order[:-1]          # the last slot is left to the queue's own encode after the game has gone
        todo = [s for s in finished if s not in done]
        for s in todo:
            r = subprocess.run([sys.executable, os.path.join(HERE, 'encode_shots.py'), run, '--only', s], capture_output=True, text=True)
            print('\n'.join(l for l in r.stdout.splitlines() if l.startswith(s) or 'FAILED' in l), flush=True)
            done.append(s)
        if ended:
            break
        # the take is gone without an end line (crash, timeout): stop after five quiet minutes
        newest = os.path.getmtime(logs[0]) if logs else 0
        idle = idle + 1 if (logs and time.time() - newest > 60) else 0      # (before the game starts there is no log)
        if idle > 15:
            print('no new file for five minutes: stopping', flush=True)
            break
        time.sleep(20)
    print('LIVE-DONE', done, flush=True)


if __name__ == '__main__':
    main()
