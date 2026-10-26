"""Per-clip mp4s and key-frame strips from a munition run (orchestrator tool).
usage: python clips_from_run.py <run dir with tri/look_probe_mun_*> <out dir> [label]
Writes <out>/<time>_<clip>.mp4 (30 fps) and <out>/<time>_<clip>_strip.png
(frames at -100, +33, +100, +200, +333, +500, +1000, +2000, +4000 ms from the spawn; PRE = 8)."""
import re, subprocess, sys, shutil, tempfile
from pathlib import Path
from PIL import Image, ImageDraw
run, out = Path(sys.argv[1]), Path(sys.argv[2]); label = sys.argv[3] if len(sys.argv) > 3 else ""
out.mkdir(parents=True, exist_ok=True)
PRE = 8; MS = [-100, 33, 100, 200, 333, 500, 1000, 2000, 4000]
for d in sorted(run.glob("tri/look_probe_mun_*")):
    clips = {}
    for p in sorted(d.glob("*.png")):
        m = re.search(r"mun_(\w+?)_(he|grenade|heat|rocketB|rocket|fuel)_f(\d+)\.png$", p.name)
        if m: clips.setdefault((m.group(1), m.group(2)), []).append((int(m.group(3)), p))
    for (t, c), fr in clips.items():
        fr.sort(); tmp = Path(tempfile.mkdtemp())
        for i, (_, p) in enumerate(fr, 1): shutil.copyfile(p, tmp / f"f{i:03d}.png")
        mp4 = out / f"{t}_{c}.mp4"
        subprocess.run(["C:/ffmpeg/bin/ffmpeg.exe", "-v", "error", "-y", "-framerate", "30", "-i", str(tmp / "f%03d.png"),
                        "-c:v", "libx264", "-crf", "18", "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(mp4)], check=True)
        W, H = 320, 213; strip = Image.new("RGB", (W * len(MS), H + 18), (20, 20, 20)); dr = ImageDraw.Draw(strip)
        dr.text((4, 2), f"{label} {t} {c}", fill=(255, 210, 90))
        for k, ms in enumerate(MS):
            f = PRE + round(ms * 30 / 1000)
            if 1 <= f <= len(fr):
                im = Image.open(fr[f - 1][1]).convert("RGB").resize((W, H)); ImageDraw.Draw(im).text((4, 4), f"{ms:+d} ms", fill=(255, 230, 0))
                strip.paste(im, (k * W, 18))
        strip.save(out / f"{t}_{c}_strip.png"); shutil.rmtree(tmp)
        print(t, c, len(fr), "frames ->", mp4.name)
