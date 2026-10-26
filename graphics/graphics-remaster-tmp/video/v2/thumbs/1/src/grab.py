import subprocess, sys, os
V2 = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
OUT = V2 + "/thumbs/1/frames"
FF = "C:/ffmpeg/bin/ffmpeg.exe"
clip = sys.argv[1]
for n in [int(x) for x in sys.argv[2].split(",")]:
    for side in ("vanilla", "ud"):
        dst = f"{OUT}/{clip}_{side}_f{n:04d}.png"
        if os.path.exists(dst):
            continue
        src = f"{V2}/{side}/{clip}/{clip}__{side}.mkv"
        subprocess.run([FF, "-v", "error", "-y", "-i", src, "-vf", "select=eq(n\," + str(n) + ")", "-vsync", "0", "-frames:v", "1", dst], check=True)
print("ok")
