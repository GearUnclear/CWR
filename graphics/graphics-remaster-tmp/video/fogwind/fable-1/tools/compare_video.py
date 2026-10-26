"""fable-1: compare.mp4 for the owner. The same 2x crop of the bay and the far valley from three clips,
labelled, one after the other at real speed (60 fps), 1280x720, under 30 MB.

  python compare_video.py out.mp4 "LABEL=clip.mkv:first:count" ...   (first = 0-based frame in the clip)

Crop box at 1080p: x 1100, y 520, w 760, h 280 (agent 1's xtslice default: bay + far valley), shown at
2x (1520x560) scaled to fit 1280 wide, padded to 720 high, the label top left in the game's title font.
"""
import os
import subprocess
import sys
import tempfile

FFMPEG = r"C:/ffmpeg/bin/ffmpeg.exe"
FONT = "D:/Arma_CWA/arma_CWR/thirdparty/guerrilla-fonts/cwr_title.ttf"
BOX = (1100, 520, 760, 280)


def part(label, clip, first, count, out):
    x, y, w, h = BOX
    font = FONT.replace(":", "\\:")
    text = label.replace(":", "\\:").replace("'", "")
    vf = ("select='between(n\\,%d\\,%d)',setpts=N/60/TB,crop=%d:%d:%d:%d,scale=1280:-2:flags=lanczos,"
          "pad=1280:720:0:(720-ih)/2:color=black,"
          "drawtext=fontfile='%s':text='%s':x=24:y=24:fontsize=40:fontcolor=white:box=0,"
          "drawtext=fontfile='%s':text='bay and far valley, 2x, real speed':x=24:y=680:fontsize=22:fontcolor=0xA0A0A0:box=0"
          % (first, first + count - 1, w, h, x, y, font, text, font))
    cmd = [FFMPEG, "-hide_banner", "-loglevel", "error", "-y", "-i", clip, "-vf", vf, "-r", "60", "-frames:v", str(count),
           "-c:v", "libx264", "-preset", "slow", "-crf", "16", "-pix_fmt", "yuv420p", out]
    subprocess.check_call(cmd)


def main():
    out = sys.argv[1]
    specs = sys.argv[2:]
    tmp = tempfile.mkdtemp(prefix="cmp_")
    parts = []
    for i, spec in enumerate(specs):
        label, rest = spec.split("=", 1)
        clip, first, count = rest.rsplit(":", 2)
        p = os.path.join(tmp, "part%d.mp4" % i)
        part(label, clip, int(first), int(count), p)
        parts.append(p)
    lst = os.path.join(tmp, "list.txt")
    with open(lst, "w", encoding="utf-8") as f:
        for p in parts:
            f.write("file '%s'\n" % p.replace("\\", "/"))
    subprocess.check_call([FFMPEG, "-hide_banner", "-loglevel", "error", "-y", "-f", "concat", "-safe", "0", "-i", lst,
                           "-c", "copy", out])
    for p in parts:
        os.remove(p)
    os.remove(lst)
    os.rmdir(tmp)
    print(out, os.path.getsize(out) // 1024, "KB")


if __name__ == "__main__":
    main()
