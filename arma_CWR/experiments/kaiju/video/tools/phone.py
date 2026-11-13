"""The phone copy of the film: 720 x 1280, two-pass, sized to stay under 20 MB.

    python phone.py [--film dedushka_9x16.mp4] [--mb 18.5]
"""
import argparse
import os

from edit_common import FINAL, FP, WORK, ff, run


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--film', default='dedushka_9x16.mp4')
    ap.add_argument('--mb', type=float, default=18.5)
    a = ap.parse_args()
    src = os.path.join(FINAL, a.film)
    out = os.path.join(FINAL, os.path.splitext(a.film)[0] + '_phone.mp4')
    dur = float(run([FP, '-v', 'error', '-show_entries', 'format=duration', '-of', 'csv=p=0', src]).stdout.strip())
    audio = 128
    video = int(a.mb * 8 * 1024 / dur - audio - 12)
    log = os.path.join(WORK, 'phone2pass')
    vf = 'hqdn3d=2.5:2:4:4,scale=720:1280:flags=lanczos'
    common = ['-i', src, '-vf', vf, '-c:v', 'libx264', '-preset', 'slow', '-b:v', '%dk' % video, '-maxrate', '%dk' % int(video * 1.6),
              '-bufsize', '%dk' % (video * 3), '-pix_fmt', 'yuv420p', '-passlogfile', log]
    ff(*(common + ['-pass', '1', '-an', '-f', 'null', 'NUL']))
    ff(*(common + ['-pass', '2', '-c:a', 'aac', '-b:a', '%dk' % audio, '-movflags', '+faststart', out]))
    size = os.path.getsize(out) / 1024 / 1024
    print('%s: %.1f MB (video %d kbit/s, %.1f s)' % (out, size, video, dur))
    if size >= 20:
        raise SystemExit('over 20 MB: lower --mb')


if __name__ == '__main__':
    main()
