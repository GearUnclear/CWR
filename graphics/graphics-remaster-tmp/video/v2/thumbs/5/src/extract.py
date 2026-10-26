import subprocess, sys, os
V2 = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
OUT = r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/5/cand"
FF = r"C:/ffmpeg/bin/ffmpeg.exe"
counts = dict(opening=990, valley_mist=570, pine_shafts=570, bay_reflections=570, outpost=600,
  cloud_shadows=510, sinai_camp=540, ambush=570, convoy=540, checkpoint=540, town_patrol=570,
  rain_town=570, supply_run=600, firefight=630)
clips = sys.argv[1].split(",")
fr = [0.1,0.3,0.5,0.7,0.9]
for c in clips:
    n = counts[c]
    idx = [int(n*f) for f in fr]
    for side in ("ud","vanilla"):
        src = f"{V2}/{side}/{c}/{c}__{side}.mkv"
        sel = "+".join(f"eq(n\,{i})" for i in idx)
        os.makedirs(f"{OUT}/{c}", exist_ok=True)
        cmd = [FF,"-v","error","-y","-i",src,"-vf",f"select={sel}","-vsync","0",f"{OUT}/{c}/{side}_%d.png"]
        subprocess.run(cmd, check=True)
        # rename to frame indices
        for k,i in enumerate(idx):
            a = f"{OUT}/{c}/{side}_{k+1}.png"; b = f"{OUT}/{c}/{side}_f{i:04d}.png"
            if os.path.exists(b): os.remove(b)
            os.rename(a,b)
    print(c, idx, flush=True)
