r"""Reads a texres_limits run: frame times, memory and the pictures of every panel.

  python analyze_limits.py <tri output dir> <report dir>

Writes
  limits.txt             per panel: the slowest frame of the first look (overview) and of the
                         first close-up (when the full size level is asked for), the frame
                         profiler line, GPU memory of the game process before and after
                         (Windows "GPU Process Memory" counter, sampled every 0.4 s by
                         run-texres.ps1), and whether other games or Deadlock were up
                         (then the timings are contended)
  limits_overviews.png   every panel from 14 m
  limits_closeups.png    every panel from 0.45 m, the middle of the frame at full size
  limits_far.png         the eleven-level sheet from 50 m to 800 m (which level is on screen)
"""
import bisect
import datetime
import os
import re
import sys

from PIL import Image, ImageDraw, ImageFont


def font(px):
    try:
        return ImageFont.truetype("arial.ttf", px)
    except OSError:
        return ImageFont.load_default()


def ts(text):
    return datetime.datetime.strptime(text, "%Y-%m-%d %H:%M:%S.%f")


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    run_dir, out_dir = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    os.makedirs(out_dir, exist_ok=True)
    log = None
    frames = {}
    for root, _d, files in os.walk(run_dir):
        for f in files:
            if f == "game_stdout.log":
                log = os.path.join(root, f)
            m = re.match(r"\d+_(l_.+)\.png$", f)
            if m:
                frames[m.group(1)] = os.path.join(root, f)
    gpu = []
    csv = os.path.join(run_dir, "gpu_mem.csv")
    if os.path.isfile(csv):
        with open(csv, "r") as fh:
            next(fh, None)
            for line in fh:
                p = line.strip().split(",")
                if len(p) >= 4:
                    gpu.append((ts(p[0]), int(p[2]), int(p[3]), int(p[4]) if len(p) > 4 else -1, int(p[5]) if len(p) > 5 else -1))
    times = [g[0] for g in gpu]

    def gpu_at(t):
        if not gpu:
            return None
        i = min(len(gpu) - 1, bisect.bisect_left(times, t))
        return gpu[i]

    rows = []
    errors = []
    rx = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\].*?\"TEXRES (.*)\"\s*$")
    with open(log, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = rx.match(line)
            if m:
                rows.append((ts(m.group(1)), m.group(2)))
            elif re.search(r"Cannot load|Extreme texture|LZW|Checksum|Texture init failed|glTex\w+ FAILED|bad pixel format|not 2\^n", line):
                errors.append(re.sub(r"\x1b\[[0-9;]*m", "", line.strip())[:260])
    out = []
    contended = any(g[3] > 1 or g[4] > 0 for g in gpu)
    out.append("Timings and memory are %s." % ("CONTENDED: another game or Deadlock was up during the run" if contended
                                               else "from a run with no other game and no Deadlock on the machine" if gpu and gpu[0][3] >= 0
                                               else "of unknown contention (the sampler did not record it)"))
    out.append("")
    prev_gpu = None
    for t, text in rows:
        g = gpu_at(t)
        mem = ""
        if g:
            mem = "  gpu dedicated %.0f MiB" % (g[1] / 1048576.0)
            if prev_gpu is not None:
                mem += " (%+.1f)" % ((g[1] - prev_gpu) / 1048576.0)
            prev_gpu = g[1]
        m = re.search(r"frame=([0-9.]+)/([0-9.]+)/([0-9.]+)", text)
        slow = ("  slowest frame %s ms (average %s)" % (m.group(3), m.group(1))) if m else ""
        head = text.split(":")[0]
        out.append("%s  %s%s%s" % (t.strftime("%H:%M:%S.%f")[:-3], head, slow, mem))
        if not m:
            out.append("      " + text)
    out.append("")
    out.append("Texture errors in the game log:")
    out += ["  " + e for e in errors] or ["  none"]
    with open(os.path.join(out_dir, "limits.txt"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(out) + "\n")
    print("\n".join(out))

    ids = []
    for name in sorted(frames):
        m = re.match(r"l_(.+)_a_overview$", name)
        if m:
            ids.append(m.group(1))
    order = ["d1_1024", "d1_2048", "d1_4096", "d5_2048", "a8_1024", "png_2048", "png_4096", "tga_1024", "jpg_2048",
             "d1m_4096", "a4_1024", "d1_8192w"]
    ids = [i for i in order if i in ids] + [i for i in ids if i not in order]
    f = font(22)

    def grid(kind, cell, crop, name, scale=1):
        cols = 4
        rws = (len(ids) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * (cell[0] + 6) + 6, rws * (cell[1] + 34) + 6), (20, 20, 20))
        d = ImageDraw.Draw(sheet)
        for k, pid in enumerate(ids):
            key = "l_%s_%s" % (pid, kind)
            x = 6 + (k % cols) * (cell[0] + 6)
            y = 6 + (k // cols) * (cell[1] + 34)
            d.text((x, y), pid, fill=(255, 255, 255), font=f)
            if key not in frames:
                continue
            im = Image.open(frames[key]).convert("RGB")
            if crop:
                w, h = im.size
                cw, ch = cell[0] // scale, cell[1] // scale
                im = im.crop(((w - cw) // 2, (h - ch) // 2, (w + cw) // 2, (h + ch) // 2)).resize(cell, Image.NEAREST)
            else:
                im = im.resize(cell, Image.LANCZOS)
            sheet.paste(im, (x, y + 28))
        sheet.save(os.path.join(out_dir, name))

    grid("a_overview", (480, 270), False, "limits_overviews.png")
    grid("b_close_left", (480, 270), True, "limits_closeups.png", scale=2)
    grid("c_close_right", (480, 270), True, "limits_closeups_right.png", scale=2)
    grid("d_ramps", (480, 270), False, "limits_ramps.png")
    far = sorted(n for n in frames if "_far_" in n)
    if far:
        cell = (320, 180)
        sheet = Image.new("RGB", (len(far) * (cell[0] + 6) + 6, cell[1] + 40), (20, 20, 20))
        d = ImageDraw.Draw(sheet)
        for k, n in enumerate(far):
            im = Image.open(frames[n]).convert("RGB")
            w, h = im.size
            # the panel is in the middle of the frame, a little above centre
            im = im.crop((w // 2 - 160, h // 2 - 110, w // 2 + 160, h // 2 + 70))
            x = 6 + k * (cell[0] + 6)
            sheet.paste(im, (x, 34))
            d.text((x, 4), n.split("_far_")[1], fill=(255, 255, 255), font=f)
        sheet.save(os.path.join(out_dir, "limits_far.png"))


if __name__ == "__main__":
    main()
