"""Assembles the Dedushka film record from work\\edl.json.

    python assemble.py [--keep-segments] [--reuse-segments] [--no-audio] [--out name]

Each slot is rendered to exactly its frame count (checked by counting decoded frames): a game shot
is trimmed from its clip, moved (slow zoom, gate weave, hand sway, the tremor of each footfall the
log shows), given the two overexposed frames a film camera makes as it runs up, and graded to the
faded stock; a card is held with grain and weave. The segments are joined, the dust loop and the
film gate are laid over everything, the exposure flickers a little, the captions go on top, and the
picture is muxed with work\\mix.wav.

Output: final\\dedushka_9x16.mp4 (1080 x 1920, 30 fps, x264 CRF 17, AAC 256k).
"""
import argparse
import json
import math
import os
import random
import shutil

from edit_common import FF, FINAL, FP, FPS, H, SHOTS, W, WORK, ff, grade_chain, load_edl, nframes, run

SEG = os.path.join(WORK, 'seg')


def motion_exprs(slot, n):
    """x and y offsets (pixels, as ffmpeg expressions in t) and the zoom expression for one slot."""
    mv = slot.get('move', {})
    rnd = random.Random(hash(slot['name']) & 0xffff)
    T = n / FPS
    z0 = mv.get('z0', 1.035)
    z1 = mv.get('z1', z0)
    zoom = '(%g+(%g)*t/%g)' % (z0, z1 - z0, T)
    wv = mv.get('weave', 1.0)
    sw = mv.get('sway', 0.0)
    p = [rnd.uniform(0, 6.28) for _ in range(8)]
    x = ['%g*sin(2*PI*0.9*t+%g)' % (1.4 * wv, p[0]), '%g*sin(2*PI*3.7*t+%g)' % (0.8 * wv, p[1]), '%g*sin(2*PI*11.3*t+%g)' % (0.4 * wv, p[2])]
    y = ['%g*sin(2*PI*1.3*t+%g)' % (2.0 * wv, p[3]), '%g*sin(2*PI*5.1*t+%g)' % (1.0 * wv, p[4]), '%g*sin(2*PI*12.7*t+%g)' % (0.6 * wv, p[5])]
    if sw:
        x += ['%g*sin(2*PI*0.23*t+%g)' % (sw, p[6]), '%g*sin(2*PI*0.61*t+%g)' % (0.5 * sw, p[7])]
        y += ['%g*sin(2*PI*0.29*t+%g)' % (0.8 * sw, p[7]), '%g*sin(2*PI*0.53*t+%g)' % (0.4 * sw, p[6])]
    for (ts, amp, decay, f) in slot.get('shakes', []):
        if ts > T:
            continue
        e = 'gte(t,%g)*%g*exp(-(t-%g)/%g)' % (ts, amp, ts, decay)
        y.append('%s*sin(2*PI*%g*(t-%g))' % (e, f, ts))
        x.append('0.45*%s*sin(2*PI*%g*(t-%g)+1.3)' % (e, f * 0.8, ts))
    # pan and tilt drift (pixels at the output scale), for a slow reframing inside a shot
    px0, px1 = mv.get('px0', 0), mv.get('px1', mv.get('px0', 0))
    py0, py1 = mv.get('py0', 0), mv.get('py1', mv.get('py0', 0))
    x.append('(%g+(%g)*t/%g)' % (px0, px1 - px0, T))
    y.append('(%g+(%g)*t/%g)' % (py0, py1 - py0, T))
    return '+'.join(x), '+'.join(y), zoom


def render_shot(slot, n, out):
    src = os.path.join(SHOTS, slot['run'], slot['clip'] + '.mp4')
    a = int(slot.get('clip_in', 0))
    have = nframes(src)
    if a + n * float(slot.get('speed', 1.0)) > have:
        print('  WARNING %s: clip has %d frames, slot wants %d from %d: the last frame is held' % (slot['name'], have, n, a))
    x, y, zoom = motion_exprs(slot, n)
    night = slot.get('look', 'dusk') == 'night'
    flash = "eq=brightness='0.34*eq(n,0)+0.16*eq(n,1)+0.06*eq(n,2)':eval=frame" if slot.get('flash', True) else 'null'
    pre = slot.get('pre', 'null')                 # an extra filter before the grade (for example a lift for a dark take)
    speed = float(slot.get('speed', 1.0))        # under 1: slowed (a short static take stretched to its slot)
    vf = ("trim=start_frame=%d,setpts=(PTS-STARTPTS)/%g,tpad=stop_mode=clone:stop_duration=30,fps=%d,"
          "scale=w='2*trunc(iw*%s/2)':h='2*trunc(ih*%s/2)':eval=frame:flags=bicubic,"
          "crop=%d:%d:x='(iw-%d)/2+%s':y='(ih-%d)/2+%s',%s,%s,%s,format=yuv420p"
          % (a, speed, FPS, zoom, zoom, W, H, W, x, H, y, pre, flash, grade_chain(night, slot.get('grain', 14), slot.get('sat'))))
    ff('-i', src, '-filter_complex', vf, '-frames:v', str(n), '-an', '-c:v', 'libx264', '-crf', '12', '-preset', 'veryfast', '-pix_fmt', 'yuv420p', '-r', str(FPS), out)


def render_card(slot, n, out):
    png = os.path.join(WORK, 'cards', slot['card'] + '.png')
    x, y, zoom = motion_exprs(slot, n)
    extra = slot.get('pre', 'null')
    vf = ("fps=%d,scale=w='2*trunc(iw*%s/2)':h='2*trunc(ih*%s/2)':eval=frame:flags=bicubic,"
          "crop=%d:%d:x='(iw-%d)/2+%s':y='(ih-%d)/2+%s',%s,noise=c0s=20:c0f=t+u,gblur=sigma=0.75,format=yuv420p"
          % (FPS, zoom, zoom, W, H, W, x, H, y, extra))
    ff('-loop', '1', '-framerate', str(FPS), '-i', png, '-filter_complex', vf, '-frames:v', str(n), '-an', '-c:v', 'libx264', '-crf', '12',
       '-preset', 'veryfast', '-pix_fmt', 'yuv420p', '-r', str(FPS), out)


def dust_loop():
    out = os.path.join(WORK, 'dust.mp4')
    src = os.path.join(WORK, 'dust', '%03d.png')
    if not os.path.exists(out) or os.path.getmtime(out) < os.path.getmtime(os.path.join(WORK, 'dust', '000.png')):
        ff('-framerate', str(FPS), '-i', src, '-c:v', 'libx264', '-crf', '8', '-pix_fmt', 'yuv420p', '-r', str(FPS), out)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--keep-segments', action='store_true')
    ap.add_argument('--reuse-segments', action='store_true')
    ap.add_argument('--no-audio', action='store_true')
    ap.add_argument('--out', default='dedushka_9x16.mp4')
    args = ap.parse_args()
    edl = load_edl()
    os.makedirs(SEG, exist_ok=True)
    segs = []
    for slot in edl['slots']:
        n = slot['frames']
        out = os.path.join(SEG, slot['name'] + '.mp4')
        stamp = out + '.json'
        key = json.dumps(slot, sort_keys=True)
        fresh = args.reuse_segments and os.path.exists(out) and os.path.exists(stamp) and open(stamp).read() == key
        if not fresh:
            if slot['kind'] == 'shot':
                render_shot(slot, n, out)
            else:
                render_card(slot, n, out)
            open(stamp, 'w').write(key)
        got = nframes(out)
        if got != n:
            raise SystemExit('%s: %d frames, wanted %d' % (slot['name'], got, n))
        segs.append(out)
        print('  %-14s %4d frames  %6.2f to %6.2f%s' % (slot['name'], n, slot['t0'], slot['t1'], '  (reused)' if fresh else ''))
    lst = os.path.join(SEG, 'concat.txt')
    with open(lst, 'w', newline='\n') as f:
        for s in segs:
            f.write("file '%s'\n" % s.replace('\\', '/'))
    total = sum(s['frames'] for s in edl['slots'])
    dur = total / FPS
    inputs = ['-f', 'concat', '-safe', '0', '-i', lst, '-stream_loop', '-1', '-i', dust_loop(), '-loop', '1', '-framerate', str(FPS), '-i', os.path.join(WORK, 'gate.png')]
    fc = ["[0:v]format=gbrp[v0]", "[1:v]format=gbrp,trim=duration=%g,setpts=PTS-STARTPTS[d]" % dur,
          "[v0][d]blend=all_mode=multiply:all_opacity=0.8,eq=brightness='0.012*sin(2*PI*t*6.7)+0.007*sin(2*PI*t*15.3+1)+0.004*sin(2*PI*t*23.1)':eval=frame,format=yuv420p[v1]",
          "[v1][2:v]overlay=0:0:shortest=0:eof_action=repeat[v2]"]
    last = 'v2'
    k = 3
    for cap in edl.get('captions', []):
        inputs += ['-loop', '1', '-framerate', str(FPS), '-t', '%g' % dur, '-i', os.path.join(WORK, 'captions', cap['id'] + '.png')]
        fi, fo = cap.get('fade_in', 0.12), cap.get('fade_out', 0.15)
        fc.append("[%d:v]format=rgba,fade=t=in:st=%g:d=%g:alpha=1,fade=t=out:st=%g:d=%g:alpha=1[c%d]" % (k, cap['t0'], fi, cap['t1'] - fo, fo, k))
        fc.append("[%s][c%d]overlay=0:0:enable='between(t,%g,%g)'[v%d]" % (last, k, cap['t0'], cap['t1'], k))
        last = 'v%d' % k
        k += 1
    out = os.path.join(FINAL, args.out)
    cmd = inputs + ['-filter_complex', ';'.join(fc), '-map', '[%s]' % last]
    mix = os.path.join(WORK, 'mix.wav')
    if not args.no_audio and os.path.exists(mix):
        cmd = inputs + ['-i', mix, '-filter_complex', ';'.join(fc), '-map', '[%s]' % last, '-map', '%d:a' % k, '-c:a', 'aac', '-b:a', '256k', '-ar', '48000']
    cmd += ['-frames:v', str(total), '-c:v', 'libx264', '-crf', '21', '-preset', 'slow', '-profile:v', 'high', '-pix_fmt', 'yuv420p', '-r', str(FPS),
            '-movflags', '+faststart', out]
    ff(*cmd)
    got = nframes(out)
    r = run([FP, '-v', 'error', '-show_entries', 'format=duration,size', '-of', 'csv=p=0', out])
    print('%s: %d frames (edl %d), %s' % (out, got, total, r.stdout.strip()))
    if not args.keep_segments and not args.reuse_segments:
        shutil.rmtree(SEG, ignore_errors=True)
    _ = math


if __name__ == '__main__':
    main()
