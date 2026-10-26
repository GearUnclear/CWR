import subprocess, sys, os
V2 = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
OUT = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/5/cand"
FF = r"C:/ffmpeg/bin/ffmpeg.exe"
c = sys.argv[1]; idx = [int(x) for x in sys.argv[2].split(",")]
sides = sys.argv[3].split(",") if len(sys.argv)>3 else ["ud","vanilla"]
for side in sides:
    src = f"{V2}/{side}/{c}/{c}__{side}.mkv"
    sel = "+".join("eq(n" + chr(92) + f",{i})" for i in idx)
    os.makedirs(f"{OUT}/{c}", exist_ok=True)
    subprocess.run([FF,"-v","error","-y","-i",src,"-vf",f"select={sel}","-vsync","0",f"{OUT}/{c}/tmp_{side}_%d.png"], check=True)
    for k,i in enumerate(idx):
        a = f"{OUT}/{c}/tmp_{side}_{k+1}.png"; b = f"{OUT}/{c}/{side}_f{i:04d}.png"
        if os.path.exists(b): os.remove(b)
        os.rename(a,b)
print("done", c, idx)
