"""The owner's comparison video: the same 2x crop (bay and far valley) of ORIGINAL, LAST FIX and NEW
one after the other at real speed, then the full NEW scene at 720p. 1280x720, 60 fps, H.264.

python owner_video.py <original.mkv> <lastfix.mkv> <new.mkv> <out.mp4> [--from 250 --n 270 --box 1160,440,640,360]
"""
import argparse
import subprocess

FF = "C:/ffmpeg/bin/ffmpeg.exe"
FONT = "C\\:/Windows/Fonts/arial.ttf"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("original")
    ap.add_argument("lastfix")
    ap.add_argument("new")
    ap.add_argument("out")
    ap.add_argument("--from", dest="first", type=int, default=250)
    ap.add_argument("--n", type=int, default=270)
    ap.add_argument("--box", default="1160,440,640,360")
    ap.add_argument("--crf", type=int, default=18)
    o = ap.parse_args()
    x, y, w, h = [int(v) for v in o.box.split(",")]
    a, b = o.first, o.first + o.n
    lab = [
        "ORIGINAL (before any fix): the haze pumps",
        "LAST FIX: the wind in the fog",
        "NEW",
    ]
    text = "fontfile='{f}':fontsize=22:fontcolor=white:x=16:y=14:shadowcolor=black@0.8:shadowx=1:shadowy=1".format(f=FONT)
    parts = []
    for i, t in enumerate(lab):
        parts.append(
            "[{i}:v]trim=start_frame={a}:end_frame={b},setpts=PTS-STARTPTS,crop={w}:{h}:{x}:{y},"
            "scale=1280:720:flags=neighbor,setsar=1,drawtext={text}:text='{t}  (2x crop, real speed)'[p{i}]".format(
                i=i, a=a, b=b, w=w, h=h, x=x, y=y, text=text, t=t.replace(":", "\\:")))
    parts.append("[2:v]trim=start_frame=0:end_frame=570,setpts=PTS-STARTPTS,scale=1280:720:flags=lanczos,setsar=1,"
                 "drawtext={text}:text='NEW, the full shot'[p3]".format(text=text))
    parts.append("[p0][p1][p2][p3]concat=n=4:v=1:a=0,format=yuv420p[v]")
    cmd = [FF, "-hide_banner", "-loglevel", "error", "-y", "-i", o.original, "-i", o.lastfix, "-i", o.new,
           "-filter_complex", ";".join(parts), "-map", "[v]", "-r", "60", "-c:v", "libx264", "-preset", "slow",
           "-crf", str(o.crf), "-movflags", "+faststart", o.out]
    print(" ".join(cmd))
    subprocess.check_call(cmd)


if __name__ == "__main__":
    main()
