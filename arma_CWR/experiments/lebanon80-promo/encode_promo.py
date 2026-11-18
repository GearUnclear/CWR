"""Encodes the triRecord dumps of one promo shoot run to H.264 clips, then deletes the dumps.

    python encode_promo.py <run folder> [--keep] [--fps 30] [--head 15]

Every <clip>.raw (raw RGB24, rows bottom-up) with its <clip>.raw.txt sidecar under the run folder
becomes clips\\<clip>.mp4 (libx264 crf 16, yuv420p, no audio) in
<UD_PROMO_DIR>\\clips (promopaths.py), and a contact sheet (first, middle and
last frame) in ..\\work\\sheets\\<clip>.jpg for review. The dump is deleted once the mp4 reads back
with the right frame count (raw recordings are never kept). A clip named *_discard is deleted unencoded.
The first --head frames of every dump (the camera held on its first pose while the scene finishes
arriving, promolib.HEAD) are cut: the delivered clip starts on the first moving frame.
"""
import argparse
import glob
import os
import subprocess

from promopaths import FF, FP, ROOT


def sidecar(raw):
    with open(raw + '.txt', encoding='utf-8') as f:
        first = f.readline().split()
    hdr = dict(kv.split('=', 1) for kv in first if '=' in kv)
    return int(hdr['w']), int(hdr['h']), int(hdr['frames'])


def count_frames(mp4):
    r = subprocess.run([FP, '-v', 'error', '-count_frames', '-select_streams', 'v:0', '-show_entries',
                        'stream=nb_read_frames', '-of', 'default=nk=1:nw=1', mp4], capture_output=True, text=True)
    try:
        return int(r.stdout.strip())
    except ValueError:
        return -1


def low_priority():
    """idle priority for this process and the ffmpeg it starts (they inherit it), so encoding never competes
    with a game another session is timing"""
    try:
        import ctypes
        k = ctypes.windll.kernel32
        k.SetPriorityClass(k.GetCurrentProcess(), 0x40)
    except Exception:
        pass


def main():
    low_priority()
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('--keep', action='store_true')
    ap.add_argument('--fps', type=int, default=30)
    ap.add_argument('--head', type=int, default=15)
    a = ap.parse_args()
    clips = os.path.join(ROOT, 'clips')
    sheets = os.path.join(ROOT, 'work', 'sheets')
    os.makedirs(clips, exist_ok=True)
    os.makedirs(sheets, exist_ok=True)
    for raw in sorted(glob.glob(os.path.join(a.src, '**', '*.raw'), recursive=True)):
        name = os.path.splitext(os.path.basename(raw))[0]
        if not os.path.exists(raw + '.txt'):
            print('no sidecar (recording unfinished?):', raw)
            continue
        if name.endswith('_discard'):
            os.remove(raw)
            os.remove(raw + '.txt')
            continue
        w, h, n = sidecar(raw)
        size = os.path.getsize(raw)
        if size != w * h * 3 * n:
            print('size mismatch, kept:', raw, size, w * h * 3 * n)
            continue
        out = os.path.join(clips, name + '.mp4')
        head = a.head if n > a.head + 30 else 0
        vf = 'vflip' + (f',trim=start_frame={head},setpts=PTS-STARTPTS' if head else '')
        subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-y', '-f', 'rawvideo', '-pix_fmt', 'rgb24',
                        '-s', f'{w}x{h}', '-framerate', str(a.fps), '-i', raw, '-vf', vf, '-an',
                        '-c:v', 'libx264', '-preset', 'slow', '-crf', '16', '-pix_fmt', 'yuv420p',
                        '-movflags', '+faststart', out], check=True)
        got = count_frames(out)
        n -= head
        sheet = os.path.join(sheets, name + '.jpg')
        mid = max(0, n // 2)
        subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-y', '-i', out, '-vf',
                        f"select='eq(n\\,0)+eq(n\\,{mid})+eq(n\\,{n - 1})',scale=640:-1,tile=3x1",
                        '-frames:v', '1', '-q:v', '3', sheet], check=False)
        ok = got == n
        print(f'{name}: {n} frames {w}x{h} (head {head} cut) -> {out} (read back {got}){"" if ok else " MISMATCH, dump kept"}')
        if ok and not a.keep:
            os.remove(raw)
            os.remove(raw + '.txt')


if __name__ == '__main__':
    main()
