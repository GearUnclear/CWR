"""Encodes the stepped-capture frames of one shoot run to one mp4 per slot and camera, writes the
per-frame log of each as JSON, a frame strip (8 frames a second) for reading the motion, and then
deletes the frames. Adapted from the chupacabra tool.

    python encode_shots.py <run folder> [--keep] [--only a,b] [--strip-fps 8]

Frames are <seq>_<slot>-<cam>.png. The game log has one KVF and one KVG line per stepped frame with
its sim time; the step must be 1/30 s (checked). Output:
    kaiju-notes\\video\\shots\\<run tag>\\<slot>-<cam>.mp4     libx264 crf 16, yuv420p, 30 fps, full size
    kaiju-notes\\video\\shots\\<run tag>\\<slot>.json           times, positions, states, footfall counts per frame
    kaiju-notes\\video\\shots\\<run tag>\\events.json           every sound the script set sent (class, sim time, place)
    kaiju-notes\\video\\wip\\strips\\<run tag>_<slot>-<cam>.jpg   the frame strip
    kaiju-notes\\video\\shots\\<run tag>\\stills\\*.png           single stills of the run (kept)
Frames are deleted once an mp4 reads back with the right frame count (raw frames are never kept).
"""
import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import time

from PIL import Image

FF = r'C:\ffmpeg\bin\ffmpeg.exe'
FP = r'C:\ffmpeg\bin\ffprobe.exe'
ROOT = r'D:\Arma_CWA_worktrees\kaiju-notes\video'


def parse_log(log):
    frames = {}
    events = []
    cams = {}
    other = []
    if not os.path.exists(log):
        return frames, events, cams, other
    for line in open(log, encoding='utf-8', errors='replace'):
        m = re.search(r'KVF (\S+) (\d+) ([\d.e+-]+) \[([^\]]*)\] ([\d.e+-]+) (\S*) (\S*) ([\d.e+-]+) (\S*)', line)
        if m:
            p = [float(v) for v in m.group(4).split(',')]
            frames.setdefault(m.group(1), {})[int(m.group(2))] = dict(
                t=float(m.group(3)), pos=p, dir=float(m.group(5)), state=m.group(6).strip('"'), phase=m.group(7).strip('"'),
                steps=int(float(m.group(8))), move=m.group(9).strip('"'))
            continue
        m = re.search(r'KVG (\S+) (\d+) (\S*) (\S*) ([\d.e+-]+) ([\d.e+-]+) ([\d.e+-]+) ([\d.e+-]+) ([\d.e+-]+) (.*)$', line)
        if m:
            f = frames.setdefault(m.group(1), {}).setdefault(int(m.group(2)), {})
            f.update(beam=m.group(3).strip('"'), death=m.group(4).strip('"'), shots=int(float(m.group(5))), health=float(m.group(6)),
                     stage=int(float(m.group(7))), roars=int(float(m.group(8))), shake=float(m.group(9)), furnace=m.group(10).strip().rstrip('"'))
            continue
        m = re.search(r'KVX (\S+) (\d+) ([\d.e+-]+) ([\d.e+-]+) ([\d.e+-]+) ([\d.e+-]+)', line)
        if m:
            f = frames.setdefault(m.group(1), {}).setdefault(int(m.group(2)), {})
            f.update(felled=int(float(m.group(3))), crushed=int(float(m.group(4))), force=int(float(m.group(5))), wrecks=int(float(m.group(6))))
            continue
        m = re.search(r'KVC (\S+) eye \[([^\]]*)\] aim \[([^\]]*)\] fov ([\d.]+) alpha ([\d.]+)', line)
        if m:
            cams[m.group(1)] = dict(eye=[float(v) for v in m.group(2).split(',')], aim=[float(v) for v in m.group(3).split(',')],
                                    fov=float(m.group(4)), alpha=float(m.group(5)))
            continue
        m = re.search(r'KJROAR snd (\S+) sent ([\d.e+-]+) from \[([^\]]*)\]: ([\d.e+-]+) m', line)
        if m:
            events.append(dict(cls=m.group(1), t=float(m.group(2)), pos=[float(v) for v in m.group(3).split(',')], d=float(m.group(4))))
            continue
        m = re.search(r'KJROAR birds (\d+) up at ([\d.e+-]+)', line)
        if m:
            events.append(dict(cls='KJ_Birds', t=float(m.group(2)), pos=[0, 0, 0], d=45.0))
            continue
        m = re.search(r'"(KV[A-Z]* .*|KJD beat .*|KJCOMBAT .*|KJD fight .*|KJBRAIN .*)"?$', line)
        if m and not m.group(1).startswith(('KVF', 'KVG', 'KVC', 'KVH', 'KVX')):
            other.append(m.group(1)[:400])
    return frames, events, cams, other


def strip(files, out, fps, cols=12, h=320):
    step = max(1, round(30 / fps))
    pick = files[::step]
    ims = []
    for p in pick:
        im = Image.open(p).convert('RGB')
        w = round(im.width * h / im.height)
        ims.append(im.resize((w, h), Image.BILINEAR))
    if not ims:
        return
    w = ims[0].width
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * w, rows * h), (16, 16, 16))
    for i, im in enumerate(ims):
        sheet.paste(im, ((i % cols) * w, (i // cols) * h))
    sheet.save(out, quality=88)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('--only', default='')
    ap.add_argument('--keep', action='store_true', help='keep the frames (default: delete once the mp4 checks out)')
    ap.add_argument('--strip-fps', type=float, default=8)
    ap.add_argument('--tag', default='')
    args = ap.parse_args()
    src = os.path.abspath(args.src)
    tag = args.tag or re.sub(r'-\d{4}-\d{6}$', '', os.path.basename(src))
    out = os.path.join(ROOT, 'shots', tag)
    os.makedirs(os.path.join(out, 'stills'), exist_ok=True)
    strips = os.path.join(ROOT, 'wip', 'strips')
    os.makedirs(strips, exist_ok=True)
    for b in glob.glob(os.path.join(src, '**', '*.bmp'), recursive=True):
        try:
            if time.time() - os.path.getmtime(b) > 6:      # a take may still be running: leave what is being written
                os.remove(b)
        except OSError:
            pass
    logs = glob.glob(os.path.join(src, '**', 'game_stdout.log'), recursive=True)
    frames, events, cams, other = parse_log(logs[0]) if logs else ({}, [], {}, [])
    if logs and os.path.abspath(logs[0]) != os.path.abspath(os.path.join(out, 'game_stdout.log')):
        shutil.copy(logs[0], os.path.join(out, 'game_stdout.log'))
    groups = {}
    for p in glob.glob(os.path.join(src, '**', '*.png'), recursive=True):
        m = re.match(r'(\d+)_(.+)\.png$', os.path.basename(p))
        if not m:
            continue
        label = m.group(2)
        if '-' in label and label.split('-')[0] in frames:
            groups.setdefault(label, []).append((int(m.group(1)), p))
        else:
            shutil.move(p, os.path.join(out, 'stills', os.path.basename(p)))
    only = set(x for x in args.only.split(',') if x)
    json.dump(events, open(os.path.join(out, 'events.json'), 'w'), indent=0)
    json.dump(cams, open(os.path.join(out, 'cams.json'), 'w'), indent=1)
    open(os.path.join(out, 'notes.log'), 'w', encoding='utf-8').write('\n'.join(other))
    for slot, fr in sorted(frames.items()):
        ks = sorted(fr)
        ts = [fr[k].get('t', 0) for k in ks]
        steps = [round(b - a, 4) for a, b in zip(ts, ts[1:])]
        bad = [s for s in steps if abs(s - 0.0333) > 0.002]
        json.dump([dict(k=k, **fr[k]) for k in ks], open(os.path.join(out, slot + '.json'), 'w'))
        print('%-14s %4d logged frames, sim %.2f to %.2f, steps %s' % (slot, len(ks), ts[0], ts[-1], 'ok' if not bad else 'BAD %d of %d, e.g. %s' % (len(bad), len(steps), bad[:5])))
    for label, fr in sorted(groups.items()):
        if only and label not in only and label.split('-')[0] not in only:
            continue
        files = [p for _, p in sorted(fr)]
        lst = os.path.join(src, '_list_%s.txt' % label)
        with open(lst, 'w', newline='\n') as f:
            for p in files:
                f.write("file '%s'\nduration 0.0333333\n" % p.replace('\\', '/'))
        mp4 = os.path.join(out, label + '.mp4')
        r = subprocess.run([FF, '-y', '-loglevel', 'error', '-f', 'concat', '-safe', '0', '-i', lst, '-vf', 'fps=30', '-frames:v', str(len(files)),
                            '-c:v', 'libx264', '-crf', '16', '-preset', 'medium', '-pix_fmt', 'yuv420p', '-r', '30', mp4])
        os.remove(lst)
        q = subprocess.run([FP, '-v', 'error', '-count_frames', '-select_streams', 'v:0', '-show_entries', 'stream=nb_read_frames,width,height',
                            '-of', 'csv=p=0', mp4], capture_output=True, text=True)
        got = q.stdout.strip()
        ok = r.returncode == 0 and got.endswith(',%d' % len(files))
        strip(files, os.path.join(strips, '%s_%s.jpg' % (tag, label)), args.strip_fps)
        print('%-22s %4d frames -> %s %s' % (label, len(files), got, 'OK' if ok else 'FAILED'))
        if ok and not args.keep:
            for p in files:
                os.remove(p)
    left = glob.glob(os.path.join(src, '**', '*.png'), recursive=True)
    print('stills kept: %d; frames left in the run folder: %d' % (len(os.listdir(os.path.join(out, 'stills'))), len(left)))


if __name__ == '__main__':
    main()
