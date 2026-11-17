"""frametime.py: per-frame timing for the owner's motion clips (OVERLAY-1, owner request 2026-11-03:
"frame time graph overlays" in the style of a well known console benchmark channel). Three parts:

    python -I frametime.py extract --boot <boot dir> --out <dir> [--pacing-dir <clips\\<side>>] [--step 0.0166667]
    python -I frametime.py synth --frames N --out <x.timing.json> [--seed n] [--cpu 9] [--gpu 7]
    python -I frametime.py strip <a.timing.json|-> [<b> <c>] --out <strip.mkv> [--frames N] [--from 0]
    python -I frametime.py selftest [--out <dir>]

Why a separate timing pass. A recorded clip (triRecord) does not run at real speed: the recorder
reads every frame back, so the frame times of the recorded pass are the recorder's, not the game's.
gen_motion.py --timing-pass after (perf-bench's default for motion jobs) therefore replays each clip
once more without recording: the same mode, the same pin, temporal reset and seed, the same
triCamPath, the same fixed 1/60 s simulation step (triSetFixedStep), and frames run unthrottled
(fpsCap 0, vsync 0, as in every timed bench set), driven by triSimFrames inside ONE statement so
nothing else runs a frame in between. Every chunk of up to 240 frames ends with triRecordState
(for the camera path clock pathT) and the two P0-4 rings (triPerfRing, triGpuRing [n, label]).
Frame alignment is exact, not resampled: under the fixed step the k-th frame after triCamPath shows
path time k/60 s in both passes, and the recorded clip's frame i shows path step i + first (first = 1:
pacing.json path_first = 0.0167), so video frame i takes the timing of path step i + first. The
newest CPU ring record of a chunk is path step P = round(pathT / step); the GPU ring trails by the
GL33 query delay (kGpuFrames = 4: frame k is read back at the start of frame k + 4), measured per
clip from chunk 0 (GPU timing is switched off and on right before the path starts, so chunk 0 holds
n - lag GPU records) and checked against 4. The pass runs 8 frames past the clip so the GPU ring
reaches its last frame. Real pacing (step = wall time) was not used: the camera would then move by
a different amount per frame in every run and the timing frames would no longer be the video's
frames; the per-frame costs (CPU work, GPU frame) do not depend on the pacing.

Series per video frame (null where not measured): cpu_work = total - swap of the CPU frame profiler
(the bench's "CPU work"), cpu_total, gpu_frame (GPU frame time), draws.

extract: reads every game_stdout.log under a boot dir (event=timing_start / timing / timing_done
UDCAP lines; their ring answers are pring= and gring=, never gpu= or perf=, which gatelib.udcap_lines
treats as the line's last field) and the ring CSVs beside it (perfring-tim_<label>_<k>.csv, gpuring-...), writes
<out>/<label>.timing.json per clip label and prints one line per label. Exit 0 (also when the boot
has no timing pass), 1 when a label's alignment check failed (the json is still written, with
aligned false).

synth: synthetic timing for tests ("synthetic": true; every strip drawn from it says SYNTHETIC TEST
DATA in red). strip: renders the graph strip (one 960 px panel per timing file, "-" for a panel
without timing) to a lossless FFV1 file for owner_sheet.py to stack under the three-way clip.
selftest: the renderer and the extractor on synthetic data (no game, no video): exit 0 PASS.

Image work: run it through squad-heavy.ps1 (-Kind analysis or encode).
"""
import argparse
import csv
import glob
import json
import math
import os
import random
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402

STEP = 1.0 / 60.0
GPU_LAG_EXPECTED = 4  # EngineGL33.hpp kGpuFrames
LABEL_PREFIX = "tim_"

# ---------------------------------------------------------------------------------------- extract


def read_ring_csv(path):
    rows = []
    with open(path, encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f):
            rows.append(r)
    return rows


def fnum(s):
    try:
        v = float(s)
    except (TypeError, ValueError):
        return None
    return None if v < 0 else v


def pct(vals, p):
    s = sorted(v for v in vals if v is not None)
    if not s:
        return None
    k = (len(s) - 1) * p / 100.0
    lo = int(k)
    hi = min(lo + 1, len(s) - 1)
    return round(s[lo] + (s[hi] - s[lo]) * (k - lo), 3)


def summarize(series):
    out = {}
    for k in ("cpu_work", "gpu_frame", "cpu_total"):
        v = [x for x in series.get(k, []) if x is not None]
        out[k] = dict(n=len(v), p50=pct(v, 50), p95=pct(v, 95), max=round(max(v), 3) if v else None)
    return out


def extract_boot(boot, out, pacing_dir=None, step=STEP):
    """Returns [(label, json path, ok)]."""
    events = {}
    for log in glob.glob(os.path.join(boot, "**", "game_stdout.log"), recursive=True):
        d0 = os.path.dirname(log)
        for _ts, d in gl.udcap_lines(log):
            ev = d.get("event", "")
            if not ev.startswith("timing"):
                continue
            lab = d.get("clip", "")
            e = events.setdefault(lab, dict(dir=d0, start=None, chunks=[], done=None))
            if ev == "timing_start":
                e["start"] = d
            elif ev == "timing":
                e["chunks"].append(d)
            elif ev == "timing_done":
                e["done"] = d
    res = []
    for lab, e in sorted(events.items()):
        n_frames = int((e["start"] or {}).get("frames", 0) or 0)
        first = 1
        if pacing_dir:
            pj = os.path.join(pacing_dir, f"{lab}.pacing.json")
            if os.path.exists(pj):
                try:
                    pf = json.load(open(pj, encoding="utf-8"))["pacing"]["path_first"]
                    first = max(0, int(round(float(pf) / step)))
                    n_frames = n_frames or int(json.load(open(pj, encoding="utf-8"))["frames"])
                except (KeyError, ValueError, TypeError):
                    pass
        cpu, gpu, problems, chunks = {}, {}, [], []
        lag = None
        cum = 0
        for c in sorted(e["chunks"], key=lambda d: int(d.get("chunk", 0))):
            k = int(c.get("chunk", 0))
            n = int(c.get("n", 0))
            cum += n
            try:
                P = int(round(float(c.get("pathT", "nan")) / step))
            except ValueError:
                P = None
            info = dict(chunk=k, n=n, pathT=c.get("pathT"), P=P, expected_P=cum, cpu=c.get("pring"), gpu=c.get("gring"))
            chunks.append(info)
            if P is None:
                problems.append(f"chunk {k}: no pathT")
                continue
            if P != cum:
                problems.append(f"chunk {k}: path step {P}, {cum} frames run")
            tag = f"{LABEL_PREFIX}{lab}_{k}"
            pc = os.path.join(e["dir"], f"perfring-{tag}.csv")
            gc = os.path.join(e["dir"], f"gpuring-{tag}.csv")
            if os.path.exists(pc):
                rows = read_ring_csv(pc)
                for j, r in enumerate(rows):
                    s = P - (len(rows) - 1 - j)
                    tot, sw = fnum(r.get("total")), fnum(r.get("swap"))
                    cpu[s] = dict(total=tot, work=None if tot is None or sw is None else round(tot - sw, 3),
                                  draws=int(float(r["draws"])) if r.get("draws") not in (None, "") else None)
            else:
                problems.append(f"chunk {k}: no {os.path.basename(pc)}")
            if os.path.exists(gc):
                rows = read_ring_csv(gc)
                if k == 0:
                    lag = n - len(rows)
                L = GPU_LAG_EXPECTED if lag is None else lag
                for j, r in enumerate(rows):
                    gpu[P - L - (len(rows) - 1 - j)] = fnum(r.get("frame"))
            else:
                problems.append(f"chunk {k}: no {os.path.basename(gc)}")
        if lag is not None and lag != GPU_LAG_EXPECTED:
            problems.append(f"GPU lag {lag} frames, expected {GPU_LAG_EXPECTED}")
        if not n_frames:
            n_frames = max(cpu) - first + 1 if cpu else 0
        series = dict(cpu_work=[], cpu_total=[], gpu_frame=[], draws=[])
        for i in range(n_frames):
            s = i + first
            c = cpu.get(s, {})
            series["cpu_work"].append(c.get("work"))
            series["cpu_total"].append(c.get("total"))
            series["draws"].append(c.get("draws"))
            series["gpu_frame"].append(gpu.get(s))
        missing = [k for k in ("cpu_work", "gpu_frame") if any(v is None for v in series[k])]
        for k in missing:
            problems.append(f"{k}: {sum(v is None for v in series[k])} of {n_frames} frames not measured")
        doc = dict(label=lab, source="timing pass: unrecorded replay of the clip's camera path, fixed 1/60 s step, unthrottled",
                   boot=os.path.abspath(boot), fps=60, step=step, frames=n_frames, first_step=first,
                   gpu_lag=lag if lag is not None else GPU_LAG_EXPECTED, chunks=chunks, aligned=not problems, problems=problems,
                   start=e["start"], done=e["done"], stats=summarize(series), **series)
        p = os.path.join(out, f"{lab}.timing.json")
        gl.write_json(p, doc)
        st = doc["stats"]
        print(f"timing {lab}: {n_frames} frames, cpu work p50 {st['cpu_work']['p50']} p95 {st['cpu_work']['p95']} ms, "
              f"gpu p50 {st['gpu_frame']['p50']} p95 {st['gpu_frame']['p95']} ms, "
              f"{'aligned' if not problems else 'PROBLEMS: ' + '; '.join(problems)}")
        res.append((lab, p, not problems))
    if not events:
        print(f"timing: no timing pass in {boot}")
    return res


# ---------------------------------------------------------------------------------------- synth


def synth(frames, seed=1, cpu=9.0, gpu=7.0):
    rnd = random.Random(seed)
    cw, gf = [], []
    for i in range(frames):
        t = i / 60.0
        c = cpu + 1.5 * math.sin(t * 0.9 + seed) + rnd.gauss(0, 0.35)
        g = gpu + 2.0 * math.sin(t * 0.6 + 2 * seed) + rnd.gauss(0, 0.25)
        if rnd.random() < 0.012:
            c += rnd.uniform(8, 30)  # a hitch
        if i % 211 == 100:
            g += 12
        cw.append(round(max(0.5, c), 3))
        gf.append(round(max(0.5, g), 3) if i % 97 != 50 else None)  # an unmeasured GPU frame now and then
    series = dict(cpu_work=cw, cpu_total=[round(c + 0.3, 3) for c in cw], gpu_frame=gf, draws=[1000] * frames)
    return dict(label=f"synthetic-{seed}", synthetic=True, source="SYNTHETIC TEST DATA (frametime.py synth)", fps=60,
                step=STEP, frames=frames, first_step=1, gpu_lag=GPU_LAG_EXPECTED, aligned=True, problems=[],
                stats=summarize(series), **series)


# ---------------------------------------------------------------------------------------- strip

PANEL_W = 960
STRIP_H = 150
WINDOW = 240          # frames shown across a panel (4 s at 60 fps, 4 px per frame)
CURSOR_FRAC = 0.75    # the current frame sits here: 3 s of past, 1 s ahead
Y_MAX = 40.0          # ms at the top of the graph; higher values are drawn at the top edge
SS = 2                # supersampling
BG = (14, 14, 16)
GRID = (52, 52, 58)
C_CPU = (255, 158, 48)
C_GPU = (72, 200, 255)
C_REF = (225, 225, 225)
C_REF2 = (140, 140, 146)
C_WARN = (255, 70, 70)


def load_timing(p):
    if p in (None, "", "-"):
        return None
    with open(p, encoding="utf-8") as f:
        return json.load(f)


def _mix(c, f):
    return tuple(int(BG[k] + (c[k] - BG[k]) * f) for k in range(3))


class StripRenderer:
    """One graph strip per panel, PANEL_W x STRIP_H, side by side; frame(i) -> RGB bytes."""

    def __init__(self, timings, n_panels=None, panel_w=PANEL_W, h=STRIP_H):
        from PIL import Image, ImageDraw
        self.Image, self.ImageDraw = Image, ImageDraw
        self.t = list(timings)
        self.np = n_panels or len(self.t)
        self.pw, self.h = panel_w, h
        s = SS
        self.f_val = gl.font(19 * s)
        self.f_small = gl.font(13 * s)
        self.top = 30 * s
        self.bot = (h - 6) * s
        self.ppf = panel_w * s / WINDOW
        self.cx = int(panel_w * s * CURSOR_FRAC)
        self.bg = self._background()

    def y(self, ms):
        s = SS
        v = min(max(ms, 0.0), Y_MAX)
        return self.bot - (self.bot - self.top) * v / Y_MAX

    def _background(self):
        s = SS
        im = self.Image.new("RGB", (self.pw * s, self.h * s), BG)
        d = self.ImageDraw.Draw(im)
        for ms, col, lab in ((16.7, C_REF, "16.7 ms"), (33.3, C_REF2, "33.3 ms")):
            y = int(self.y(ms))
            for x in range(0, self.pw * s, 14 * s):
                d.line([(x, y), (x + 8 * s, y)], fill=col, width=s)
            d.text((6 * s, y - 15 * s), lab, fill=col, font=self.f_small)
        d.line([(0, int(self.y(0))), (self.pw * s, int(self.y(0)))], fill=GRID, width=s)
        # the frames ahead of the cursor are drawn dimmer over a slightly lighter band
        d.rectangle([self.cx, self.top, self.pw * s, self.bot], fill=(20, 20, 24))
        for ms, col, lab in ((16.7, C_REF, ""), (33.3, C_REF2, "")):
            y = int(self.y(ms))
            for x in range(self.cx - self.cx % (14 * s), self.pw * s, 14 * s):
                d.line([(max(x, self.cx), y), (x + 8 * s, y)], fill=_mix(col, 0.6), width=s)
        return im

    def _series(self, d, t, key, col, i):
        s = SS
        vals = t.get(key) or []
        lo, hi = i - int(self.cx / self.ppf) - 1, i + int((self.pw * s - self.cx) / self.ppf) + 2
        for part, f in ((range(max(lo, 0), min(i + 1, len(vals))), 1.0), (range(max(i, 0), min(hi, len(vals))), 0.45)):
            pts = []
            for j in part:
                v = vals[j]
                if v is None:
                    if len(pts) > 1:
                        d.line(pts, fill=_mix(col, f), width=2 * s, joint="curve")
                    pts = []
                    continue
                pts.append((self.cx + (j - i) * self.ppf, self.y(v)))
            if len(pts) > 1:
                d.line(pts, fill=_mix(col, f), width=2 * s, joint="curve")
            elif len(pts) == 1:
                x, y = pts[0]
                d.ellipse([x - s, y - s, x + s, y + s], fill=_mix(col, f))
        if 0 <= i < len(vals) and vals[i] is not None:
            y = self.y(vals[i])
            d.ellipse([self.cx - 4 * s, y - 4 * s, self.cx + 4 * s, y + 4 * s], fill=col)

    def panel(self, t, i):
        s = SS
        im = self.bg.copy()
        d = self.ImageDraw.Draw(im)
        if t is None:
            d.text((10 * s, 4 * s), "no timing pass for this clip", fill=C_REF2, font=self.f_val)
            return im
        self._series(d, t, "gpu_frame", C_GPU, i)
        self._series(d, t, "cpu_work", C_CPU, i)
        d.line([(self.cx, self.top - 2 * s), (self.cx, self.bot)], fill=(240, 240, 240), width=s)

        def val(key):
            v = (t.get(key) or [None])[i] if 0 <= i < len(t.get(key) or []) else None
            return "--.-" if v is None else f"{v:4.1f}"

        x = 10 * s
        for lab, key, col in (("CPU", "cpu_work", C_CPU), ("GPU", "gpu_frame", C_GPU)):
            txt = f"{lab} {val(key)} ms"
            d.text((x, 4 * s), txt, fill=col, font=self.f_val)
            x += int(d.textlength(txt + "    ", font=self.f_val))
        note = "SYNTHETIC TEST DATA" if t.get("synthetic") else ("TIMING NOT ALIGNED" if t.get("aligned") is False else "")
        if note:
            w = d.textlength(note, font=self.f_val)
            d.text((self.pw * s - w - 10 * s, 4 * s), note, fill=C_WARN, font=self.f_val)
        return im

    def frame(self, i):
        im = self.Image.new("RGB", (self.pw * SS * self.np, self.h * SS), BG)
        for k in range(self.np):
            t = self.t[k] if k < len(self.t) else None
            im.paste(self.panel(t, i), (k * self.pw * SS, 0))
        if SS != 1:
            im = im.resize((self.pw * self.np, self.h), self.Image.LANCZOS)
        return im


def render_strip(timings, out_path, frames, start=0, fps=60):
    """Renders frames [start, start + frames) of the strip to a lossless FFV1 file; returns its path."""
    r = StripRenderer(timings)
    w, h = r.pw * r.np, r.h
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    cmd = [gl.ffmpeg(), "-hide_banner", "-loglevel", "error", "-y", "-f", "rawvideo", "-pix_fmt", "rgb24",
           "-s", f"{w}x{h}", "-r", str(fps), "-i", "-", "-c:v", "ffv1", "-level", "3", "-pix_fmt", "bgr0", out_path]
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    try:
        for i in range(start, start + frames):
            p.stdin.write(r.frame(i).tobytes())
    finally:
        p.stdin.close()
    if p.wait() != 0:
        raise SystemExit(f"frametime: strip encode failed ({p.returncode})")
    return out_path


# ---------------------------------------------------------------------------------------- selftest


def selftest(out):
    import numpy as np
    fails = []
    os.makedirs(out, exist_ok=True)
    # 1. renderer: known values land at known heights, the cursor column is white, the note is drawn
    n = 300
    flat = dict(synthetic=True, cpu_work=[10.0] * n, gpu_frame=[20.0] * n, frames=n)
    r = StripRenderer([flat, None, dict(flat, synthetic=False, aligned=True)])
    im = np.asarray(r.frame(150)).astype(int)
    if im.shape != (STRIP_H, 3 * PANEL_W, 3):
        fails.append(f"strip shape {im.shape}")
    y_cpu = int(round(r.y(10.0) / SS))
    y_gpu = int(round(r.y(20.0) / SS))
    col = 400  # a past frame left of the cursor (frame 0 sits at x = 120 when i = 150)
    near = lambda y, c: min(np.abs(im[yy, col] - np.array(c)).sum() for yy in range(y - 1, y + 2))  # noqa: E731
    if near(y_cpu, C_CPU) > 90:
        fails.append(f"cpu line not at y={y_cpu}: {im[y_cpu - 1:y_cpu + 2, col].tolist()}")
    if near(y_gpu, C_GPU) > 90:
        fails.append(f"gpu line not at y={y_gpu}: {im[y_gpu - 1:y_gpu + 2, col].tolist()}")
    cx = int(PANEL_W * CURSOR_FRAC)
    if im[STRIP_H - 30, cx].min() < 150:
        fails.append(f"cursor column not bright: {im[STRIP_H - 30, cx].tolist()}")
    red = lambda a: int(((a[..., 0] > 200) & (a[..., 1] < 120) & (a[..., 2] < 120)).sum())  # noqa: E731
    if red(im[2:28, PANEL_W - 260:PANEL_W]) < 20:
        fails.append("synthetic note missing on panel 1")
    if red(im[2:28, 3 * PANEL_W - 260:3 * PANEL_W]):
        fails.append("a real-timing panel carries a red note")
    # a value above Y_MAX is drawn at the top edge, not outside
    hi = dict(cpu_work=[90.0] * n, gpu_frame=[None] * n, frames=n)
    im2 = np.asarray(StripRenderer([hi]).frame(150)).astype(int)
    yt = int(round(StripRenderer([hi]).y(Y_MAX) / SS))
    d_top = min(np.abs(im2[yy, col] - np.array(C_CPU)).sum() for yy in range(yt - 1, yt + 2))
    if d_top > 90:
        fails.append(f"clamped value not at the top edge y={yt} ({d_top})")
    StripRenderer([synth(n, 1), None, synth(n, 3)]).frame(200).save(os.path.join(out, "selftest_strip.png"))
    # 2. extractor: a fake boot with a timing pass of 500 frames in chunks 240, 240, 28 (clip 492 + tail 8)
    boot = os.path.join(out, "fakeboot")
    d = os.path.join(boot, "tri", "motion_abel")
    os.makedirs(d, exist_ok=True)
    lab = "clipx__today"
    nclip, tail, lag = 492, 8, 4
    chunks = [240, 240, nclip + tail - 480]
    lines = [f'[t] "UDCAP;view=clipx;event=timing_start;clip={lab};frames={nclip};tail={tail};chunk=240;gpuon=x;pathres=OK"']
    run = 0
    for k, c in enumerate(chunks):
        steps = list(range(run + 1, run + c + 1))  # path steps of this chunk's frames
        run += c
        with open(os.path.join(d, f"perfring-{LABEL_PREFIX}{lab}_{k}.csv"), "w", newline="") as f:
            f.write("i,total,swap,draws\n")
            for j, s in enumerate(steps):
                f.write(f"{j},{s / 10 + 1:.2f},1.00,{s}\n")  # work = s / 10
        gsteps = [s - lag for s in steps if s - lag >= 1]
        with open(os.path.join(d, f"gpuring-{LABEL_PREFIX}{lab}_{k}.csv"), "w", newline="") as f:
            f.write("i,frame,world,pass1,ground,rest,post\n")
            for j, s in enumerate(gsteps):
                f.write(f"{j},{s / 100:.3f},,,,,\n")  # gpu = s / 100
        lines.append(f'[t] "UDCAP;view=clipx;event=timing;clip={lab};chunk={k};n={c};pring=OK:{c};gring=OK:{len(gsteps)};'
                     f'state=OK:written;got=1;want=1;pathT={run * STEP:.4f};serial=1;path=x"')
    lines.append(f'[t] "UDCAP;view=clipx;event=timing_done;clip={lab};frames={nclip}"')
    with open(os.path.join(d, "game_stdout.log"), "w") as f:
        f.write("\n".join(lines) + "\n")
    res = extract_boot(boot, os.path.join(out, "fake_timing"))
    if len(res) != 1 or not res[0][2]:
        fails.append(f"extract: {res}")
    else:
        t = load_timing(res[0][1])
        # video frame i = path step i + 1: work (i + 1) / 10, gpu (i + 1) / 100
        bad = [i for i in range(nclip) if t["cpu_work"][i] is None or abs(t["cpu_work"][i] - (i + 1) / 10) > 0.006
               or t["gpu_frame"][i] is None or abs(t["gpu_frame"][i] - (i + 1) / 100) > 0.0006]
        if bad or t["frames"] != nclip or t["gpu_lag"] != lag:
            fails.append(f"extract alignment: frames {t['frames']} lag {t['gpu_lag']} bad frames {bad[:5]}")
    # a misaligned pass (one stray frame before chunk 1) is reported, not hidden
    with open(os.path.join(d, "game_stdout.log"), encoding="utf-8") as f:
        txt = f.read().replace(f"pathT={480 * STEP:.4f}", f"pathT={481 * STEP:.4f}")
    with open(os.path.join(d, "game_stdout.log"), "w") as f:
        f.write(txt)
    res = extract_boot(boot, os.path.join(out, "fake_timing_bad"))
    if not res or res[0][2]:
        fails.append("extract did not flag a misaligned chunk")
    print("frametime selftest:", "PASS" if not fails else "FAIL")
    for x in fails:
        print("  " + x)
    return 0 if not fails else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    a = sub.add_parser("extract")
    a.add_argument("--boot", required=True)
    a.add_argument("--out", required=True)
    a.add_argument("--pacing-dir", default="")
    a.add_argument("--step", type=float, default=STEP)
    s = sub.add_parser("synth")
    s.add_argument("--frames", type=int, required=True)
    s.add_argument("--out", required=True)
    s.add_argument("--seed", type=int, default=1)
    s.add_argument("--cpu", type=float, default=9.0)
    s.add_argument("--gpu", type=float, default=7.0)
    t = sub.add_parser("strip")
    t.add_argument("timing", nargs="+")
    t.add_argument("--out", required=True)
    t.add_argument("--frames", type=int, default=0)
    t.add_argument("--from", dest="start", type=int, default=0)
    st = sub.add_parser("selftest")
    st.add_argument("--out", default="")
    o = ap.parse_args()
    if o.cmd == "extract":
        res = extract_boot(o.boot, o.out, o.pacing_dir or None, o.step)
        return 0 if all(ok for _l, _p, ok in res) else 1
    if o.cmd == "synth":
        gl.write_json(o.out, synth(o.frames, o.seed, o.cpu, o.gpu))
        return 0
    if o.cmd == "strip":
        ts = [load_timing(p) for p in o.timing]
        n = o.frames or max((t["frames"] for t in ts if t), default=0)
        render_strip(ts, o.out, n, o.start)
        print("strip:", o.out)
        return 0
    if o.cmd == "selftest":
        return selftest(o.out or tempfile.mkdtemp(prefix="frametime-selftest-"))
    return 2


if __name__ == "__main__":
    sys.exit(main())
