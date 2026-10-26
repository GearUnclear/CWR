import subprocess, sys, os, json
from concurrent.futures import ThreadPoolExecutor
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"
V2 = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
OUT = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/3/work/cand"
os.makedirs(OUT, exist_ok=True)


def nframes(clip):
    with open(f"{V2}/ud/{clip}/collect.json") as f:
        return json.load(f)["frames"]


def extract(clip, idxs, side="ud"):
    src = f"{V2}/{side}/{clip}/{clip}__{side}.mkv"
    sel = "+".join(f"eq(n\\,{i})" for i in idxs)
    tmp = f"{OUT}/{clip}_{side}_tmp_%03d.png"
    subprocess.run([FF, "-v", "error", "-y", "-i", src, "-vf", f"select={sel}",
                    "-vsync", "0", tmp], check=True)
    paths = []
    for k, i in enumerate(sorted(idxs)):
        a = f"{OUT}/{clip}_{side}_tmp_{k+1:03d}.png"
        b = f"{OUT}/{clip}_{side}_{i:04d}.png"
        os.replace(a, b)
        paths.append(b)
    return paths


def sheet(clip, paths, side="ud"):
    w, h = 480, 270
    cols = 4
    rows = (len(paths) + cols - 1) // cols
    im = Image.new("RGB", (cols * w, rows * (h + 20)), (0, 0, 0))
    d = ImageDraw.Draw(im)
    for k, p in enumerate(paths):
        t = Image.open(p).convert("RGB").resize((w, h), Image.LANCZOS)
        x, y = (k % cols) * w, (k // cols) * (h + 20)
        im.paste(t, (x, y + 20))
        d.text((x + 4, y + 4), os.path.basename(p), fill=(255, 255, 0))
    im.save(f"{OUT}/../sheet_{clip}_{side}.png")


if __name__ == "__main__":
    clips = sys.argv[1].split(",")
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 8

    def job(clip):
        N = nframes(clip)
        idxs = sorted(set(int(round(i * (N - 1) / (n - 1))) for i in range(n)))
        p = extract(clip, idxs)
        sheet(clip, p)
        return clip

    with ThreadPoolExecutor(4) as ex:
        for c in ex.map(job, clips):
            print("done", c)
