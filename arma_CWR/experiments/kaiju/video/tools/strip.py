"""A frame strip of one clip, for reading motion.

    python strip.py <clip.mp4> [--step 4] [--h 480] [--cols 10] [--crop w:h:x:y] [--start 0] [--count 0] [--out file.jpg]

--step 4 is 7.5 frames a second of a 30 fps clip. Output: wip\strips\<clip>_s<step>.jpg
"""
import argparse
import os
import subprocess

FF = r'C:\ffmpeg\bin\ffmpeg.exe'
WIP = r'D:\Arma_CWA_worktrees\kaiju-notes\video\wip\strips'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('clip')
    ap.add_argument('--step', type=int, default=4)
    ap.add_argument('--h', type=int, default=480)
    ap.add_argument('--cols', type=int, default=10)
    ap.add_argument('--crop', default='')
    ap.add_argument('--start', type=int, default=0)
    ap.add_argument('--count', type=int, default=0)
    ap.add_argument('--out', default='')
    a = ap.parse_args()
    os.makedirs(WIP, exist_ok=True)
    name = os.path.splitext(os.path.basename(a.clip))[0]
    tag = os.path.basename(os.path.dirname(os.path.abspath(a.clip)))
    out = a.out or os.path.join(WIP, '%s_%s_s%d%s.jpg' % (tag, name, a.step, '_c' if a.crop else ''))
    sel = "select='gte(n,%d)*not(mod(n-%d,%d))%s'" % (a.start, a.start, a.step, ('*lt(n,%d)' % (a.start + a.count)) if a.count else '')
    vf = [sel]
    if a.crop:
        vf.append('crop=' + a.crop)
    vf.append('scale=-2:%d' % a.h)
    vf.append('tile=%dx40:padding=2' % a.cols)
    subprocess.run([FF, '-y', '-loglevel', 'error', '-i', a.clip, '-vf', ','.join(vf), '-frames:v', '1', '-vsync', '0', out], check=True)
    # trim the empty rows of the tile
    from PIL import Image
    im = Image.open(out)
    bbox = im.getbbox()
    if bbox:
        im.crop((0, 0, im.width, bbox[3])).save(out, quality=86)
    print(out)


if __name__ == '__main__':
    main()
