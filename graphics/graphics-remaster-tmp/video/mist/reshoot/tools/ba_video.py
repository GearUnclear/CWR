"""Before/after video of trailer scene 2 (valley_mist) for the owner.

usage: python ba_video.py --before-scene <mp4> --after-scene <mp4> --before-ud <mkv> --after-ud <mkv> --out <mp4>

1280x720, 60 fps, H.264, no audio:
  part 1  the old montage's scene 2 (the 11 s cut 14.5-25.5 s), label BEFORE
  part 2  the new montage's scene 2 (same cut), label AFTER
  part 3  the overhaul side alone, the bay and the far valley (x 960-1920, y 470-830 of 1920x1080)
          at full size, the old clip on top (BEFORE) and the new clip below (AFTER), all 570 frames
"""
import argparse
import subprocess

FF = "C:/ffmpeg/bin/ffmpeg.exe"
FONT = "D\\:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/cwr_body.ttf"


def txt(text, x, y, size=26):
    return (f"drawtext=fontfile='{FONT}':text='{text}':x={x}:y={y}:fontsize={size}:fontcolor=0xF0F1F3:"
            f"box=1:boxcolor=black@0.6:boxborderw=8")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--before-scene", required=True)
    ap.add_argument("--after-scene", required=True)
    ap.add_argument("--before-ud", required=True)
    ap.add_argument("--after-ud", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--crf", default="21")
    ap.add_argument("--t", default="", help="test: limit every input to this many seconds")
    a = ap.parse_args()
    norm = "fps=60,setsar=1,format=yuv420p"
    fc = ";".join([
        f"[0:v]scale=1280:720:flags=lanczos,{txt('BEFORE', 24, 'h-th-28')},{norm}[s1]",
        f"[1:v]scale=1280:720:flags=lanczos,{txt('AFTER', 24, 'h-th-28')},{norm}[s2]",
        f"[2:v]crop=960:360:960:470,{txt('BEFORE', 14, 14, 22)}[c0]",
        f"[3:v]crop=960:360:960:470,{txt('AFTER', 14, 14, 22)}[c1]",
        f"[c0][c1]vstack=inputs=2,pad=1280:720:160:0:color=0x141414,"
        f"{txt('overhaul side only', 'w-tw-14', 14, 18)},{norm}[s3]",
        "[s1][s2][s3]concat=n=3:v=1:a=0[v]",
    ])
    cmd = [FF, "-hide_banner", "-loglevel", "error", "-y",
           *[x for f in (a.before_scene, a.after_scene, a.before_ud, a.after_ud)
             for x in ((["-t", a.t] if a.t else []) + ["-i", f])],
           "-filter_complex", fc, "-map", "[v]", "-an",
           "-c:v", "libx264", "-preset", "slow", "-crf", a.crf, "-maxrate", "6M", "-bufsize", "12M",
           "-pix_fmt", "yuv420p", "-movflags", "+faststart", a.out]
    subprocess.run(cmd, check=True)
    print("wrote", a.out)


if __name__ == "__main__":
    main()
