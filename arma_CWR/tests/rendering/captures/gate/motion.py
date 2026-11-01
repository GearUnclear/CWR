"""motion.py: the motion metric pack and the motion screener's still products (PERF-PLAN P0-10,
3.4 item 5, 3.6). Generalises fogcheck/fogmotion.py (flicker, travel, boil, grain per region,
camera-compensated), bandmetric.py (the block flicker band) and xtslice.py (x-t / y-t slices)
to any clip, and adds shimmer, LOD popping and ghosting. Its numbers are TRIPWIRES, never
approvals: a firing tripwire stops a T item before the owner sheet, a quiet one approves
nothing (3.6). Image work: run it through squad-heavy.ps1 -Kind analysis.

    python -I motion.py metrics <clip> [<clip> ...] [--ref <golden clip>] [--regions SPEC]
                                [--preset valley_mist|default] [--static] [--from N] [--to M]
                                [--json out.json]
    python -I motion.py stills <clip>[:label] [<clip>[:label] ...] --out DIR [--from N] [--n 120]
                               [--box x,y,w,h] [--row 0.62] [--col 0.5] [--gain 16]
    python -I motion.py screen --cand <clip> --ref <golden clip> [--vanilla <clip>] --out DIR
                               [--preset P | --regions SPEC] [--static] [--from N] [--to M]

Clips: FFV1 .mkv (P0-10's lossless encode; never judge a crf 18 encode), any ffmpeg-readable
file, or a folder of numbered PNG/BMP frames. Regions: "name=y0,y1,x0,x1;..." as fractions,
or a preset from golden_views.MOTION_REGIONS.

Per region (fogmotion's definitions, see its docstring): lowd2 (60 px block flicker), mid, rho,
rho_off (travel correlation), vel_off, wind (= mid x max(rho_off, 0)), fine, fine0, and with
--static boil and slow. New here:
  shimmer   static camera: mean per-pixel temporal stddev of the fine band (I - gauss 1.5 px);
            moving camera: the camera-compensated fine residual rms (fogmotion's `fine`).
  echo      ghosting: mean correlation of the mid-band residual with the residual 2 to 4 frames
            earlier, carried along by the camera (a trailing copy shows as a positive echo).
  pops      LOD popping: per 100 frames, 30 px blocks of the camera-compensated residual (480 x
            270 grey) with a one-frame spike over max(6 codes, 5 x the block's median) whose
            neighbours stay under half of it (a step that does not flicker back).
Whole frame: band and sky of bandmetric.py (480 x 270, 15 px blocks, rows 8-13, cols 5-31).

Tripwires against --ref (thresholds ESTIMATED; the absolute bars are PERF-PLAN 3.4's
TRAVEL <= 0.05 and LOW <= 0.08 from the F2 work, read here as rho_off and lowd2):
  travel   rho_off > max(0.05, ref + 0.03)
  flicker  lowd2 > max(0.08, 1.5 x ref) or band > ref + 0.05
  boil     (static) boil > 1.5 x ref + 0.01
  grain    fine0 < ref - 0.15 (fizz that comes and goes) or fine > 1.3 x ref
  shimmer  shimmer > 1.25 x ref
  ghost    echo > ref + 0.10
  popping  pops > ref + 2 per 100 frames
Calibrate on the tuning half only (JUDGING.md "Calibration"); never on the held-out pair.

Still products (`stills`, `screen`), for agents who cannot watch a clip at real speed:
  contact4x4_<label>.png        16 consecutive frames (1-frame step) of the box, 4 x 4
  contact4x4_resid_<label>.png  the camera-compensated difference of the same frames, x gain
  velocity_<label>.png          per 60 px block, motion relative to the camera over the clip
                                (hue = direction, brightness = speed; travel shows as colour)
  residual_<label>.png          mean |camera-compensated difference| heat map
  xt.png, yt.png                RAW | STAB | RESID slices of a row and a column, clips stacked
"""
import argparse
import glob
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402
import golden_views as gv  # noqa: E402
import numpy as np  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402
from scipy import ndimage  # noqa: E402

sys.path.insert(0, os.path.join(gl.CAPTURES, "fogcheck"))
os.environ.setdefault("FFMPEG", gl.ffmpeg())
import fogmotion as fm  # noqa: E402

FF = os.environ["FFMPEG"]
BARS = dict(travel_abs=0.05, low_abs=0.08)


# ----------------------------------------------------------------------------- decoding

def clip_size(src):
    if os.path.isdir(src):
        fs = frame_files(src)
        with Image.open(fs[0]) as im:
            return im.size
    probe = os.path.join(os.path.dirname(FF), "ffprobe.exe" if FF.lower().endswith(".exe") else "ffprobe")
    try:
        out = subprocess.run([probe, "-v", "error", "-select_streams", "v:0", "-show_entries", "stream=width,height",
                              "-of", "csv=p=0", src], capture_output=True, text=True, check=True).stdout.strip()
        w, h = (int(x) for x in out.split(",")[:2])
        return w, h
    except Exception:
        return 1920, 1080


def frame_files(d):
    return sorted(glob.glob(os.path.join(d, "*.png")) + glob.glob(os.path.join(d, "*.bmp")))


def decode_box(src, a, b, box):
    """Grey float32 T x h x w of the pixel box (y0, y1, x0, x1); frames [a, b)."""
    return fm.decode(src, a, b, box)


def decode_scaled(src, a, b, w, h, gray=True):
    if os.path.isdir(src):
        fs = frame_files(src)[a:(b or None)]
        mode = "L" if gray else "RGB"
        return np.stack([np.asarray(Image.open(f).convert(mode).resize((w, h), Image.BOX), np.float32) for f in fs])
    vf = f"scale={w}:{h}:flags=area,format={'gray' if gray else 'rgb24'}"
    if a or b:
        vf = f"trim=start_frame={a}" + (f":end_frame={b}" if b else "") + ",setpts=PTS-STARTPTS," + vf
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-vsync", "0", "-f", "rawvideo",
                        "-"], capture_output=True, check=True)
    c = 1 if gray else 3
    x = np.frombuffer(p.stdout, np.uint8).reshape((-1, h, w) if gray else (-1, h, w, 3))
    return x.astype(np.float32) if gray else x


def parse_regions(spec, preset, size):
    if spec:
        regs = {}
        for part in spec.split(";"):
            if part.strip():
                n, v = part.split("=")
                regs[n.strip()] = tuple(float(t) for t in v.split(","))
    else:
        regs = gv.motion_regions(preset or "default") if preset else dict(gv.MOTION_REGIONS["default"])
    W, H = size
    return {n: (int(y0 * H), int(y1 * H), int(x0 * W), int(x1 * W)) for n, (y0, y1, x0, x1) in regs.items()}


# ----------------------------------------------------------------------------- metrics

def echo_and_shimmer(x, static):
    """echo (ghosting) and shimmer for one region stack (T x h x w grey)."""
    T, h, w = x.shape
    win = fm.hann(h, w)
    shifts = [(0.0, 0.0)] * (T - 1) if static else [fm.phase_shift(x[t], x[t + 1], win) for t in range(T - 1)]
    D = np.stack([x[t + 1] - fm.warp(x[t], shifts[t]) for t in range(T - 1)])
    mid = np.stack([ndimage.gaussian_filter(d, 2.0) - ndimage.gaussian_filter(d, 16.0) for d in D])
    m = 20
    inner = (slice(m, h - m), slice(m, w - m))
    cum = np.cumsum(np.array(shifts), axis=0) if len(shifts) else np.zeros((0, 2))
    ech = []
    for k in (2, 3, 4):
        for t in range(k, len(mid)):
            s = cum[t - 1] - cum[t - k - 1] if t - k - 1 >= 0 else cum[t - 1]
            a = fm.warp(mid[t - k], tuple(s))[inner]
            bb = mid[t][inner]
            ech.append(float((a * bb).sum() / (np.sqrt((a * a).sum() * (bb * bb).sum()) + 1e-9)))
    echo = float(np.mean(ech)) if ech else float("nan")
    if static:
        fine = np.stack([f - ndimage.gaussian_filter(f, 1.5) for f in x])[:, m:h - m, m:w - m]
        shimmer = float(fine.std(axis=0).mean())
    else:
        fine = np.stack([d - ndimage.gaussian_filter(d, 1.5) for d in D])[:, m:h - m, m:w - m]
        shimmer = float(np.sqrt((fine ** 2).mean()))
    return round(echo, 4), round(shimmer, 4)


def pops(src, a, b, static, W=480, H=270, B=8):
    """LOD popping counter on the whole frame at 480 x 270 (8 px blocks = 32 px at 1080p)."""
    x = decode_scaled(src, a, b, W, H)
    T = len(x)
    if T < 4:
        return float("nan"), []
    win = fm.hann(H, W)
    shifts = [(0.0, 0.0)] * (T - 1) if static else [fm.phase_shift(x[t], x[t + 1], win) for t in range(T - 1)]
    R = np.abs(np.stack([x[t + 1] - fm.warp(x[t], shifts[t]) for t in range(T - 1)]))
    m = 8
    R = R[:, m:H - m, m:W - m]
    gy, gx = R.shape[1] // B, R.shape[2] // B
    blk = R[:, :gy * B, :gx * B].reshape(len(R), gy, B, gx, B).mean(axis=(2, 4))
    medb = np.median(blk, axis=0)
    thr = np.maximum(6.0, 5.0 * medb)
    ev = []
    for t in range(1, len(blk) - 1):
        hit = (blk[t] > thr) & (blk[t - 1] < 0.5 * blk[t]) & (blk[t + 1] < 0.5 * blk[t])
        for yy, xx in zip(*np.nonzero(hit)):
            ev.append(dict(frame=int(t + 1 + a), y=int((yy * B + m) * 1080 / H), x=int((xx * B + m) * 1920 / W),
                           size=round(float(blk[t, yy, xx]), 1)))
    return round(100.0 * len(ev) / max(1, len(blk)), 3), ev[:50]


def metrics(src, regs, a, b, static, band=True):
    rec = dict(src=src, frm=a, to=b, static=static, regions={})
    if band and not os.path.isdir(src):
        bb = fm.bandmetric_band(src, a, b)
        if bb:
            rec["band"], rec["sky"] = bb
    for name, box in regs.items():
        x = decode_box(src, a, b, box)
        r, _ = fm.analyse_region(x, name, static)
        r["echo"], r["shimmer"] = echo_and_shimmer(x, static)
        rec["regions"][name] = r
        rec["frames"] = int(len(x))
    rec["pops_per100"], rec["pop_events"] = pops(src, a, b, static)
    return rec


def tripwires(c, r):
    out = []

    def add(region, name, cv, rv, fired, rule):
        out.append(dict(region=region, name=name, cand=cv, ref=rv, fired=bool(fired), rule=rule))

    if c.get("band") is not None and r.get("band") is not None:
        add("frame", "flicker_band", c["band"], r["band"], c["band"] > r["band"] + 0.05, "band > ref + 0.05")
    for n, cr in c["regions"].items():
        rr = r["regions"].get(n)
        if not rr:
            continue
        add(n, "travel", cr["rho_off"], rr["rho_off"], cr["rho_off"] > max(BARS["travel_abs"], rr["rho_off"] + 0.03),
            "rho_off > max(0.05, ref + 0.03)")
        add(n, "flicker", cr["lowd2"], rr["lowd2"], cr["lowd2"] > max(BARS["low_abs"], 1.5 * rr["lowd2"]),
            "lowd2 > max(0.08, 1.5 x ref)")
        if c.get("static") and np.isfinite(cr.get("boil", np.nan)) and np.isfinite(rr.get("boil", np.nan)):
            add(n, "boil", cr["boil"], rr["boil"], cr["boil"] > 1.5 * rr["boil"] + 0.01, "boil > 1.5 x ref + 0.01")
        add(n, "grain", [cr["fine"], cr["fine0"]], [rr["fine"], rr["fine0"]],
            cr["fine0"] < rr["fine0"] - 0.15 or cr["fine"] > 1.3 * rr["fine"], "fine0 < ref - 0.15 or fine > 1.3 x ref")
        add(n, "shimmer", cr["shimmer"], rr["shimmer"], cr["shimmer"] > 1.25 * rr["shimmer"], "shimmer > 1.25 x ref")
        add(n, "ghost", cr["echo"], rr["echo"], cr["echo"] > rr["echo"] + 0.10, "echo > ref + 0.10")
    if np.isfinite(c.get("pops_per100", np.nan)) and np.isfinite(r.get("pops_per100", np.nan)):
        add("frame", "popping", c["pops_per100"], r["pops_per100"], c["pops_per100"] > r["pops_per100"] + 2,
            "pops > ref + 2 per 100 frames")
    return out


def print_rec(rec):
    print(f"== {rec['src']}  frames {rec.get('frames')}  band {rec.get('band')}  sky {rec.get('sky')}  "
          f"pops/100 {rec.get('pops_per100')}")
    print(f"   {'region':8s} {'mean':>6s} {'lowd2':>6s} {'mid':>6s} {'rho_off':>7s} {'vel_off':>9s} {'wind':>6s} "
          f"{'fine':>6s} {'fine0':>6s} {'boil':>6s} {'shimmer':>7s} {'echo':>6s}")
    for k, r in rec["regions"].items():
        print(f"   {k:8s} {r['mean']:6.1f} {r['lowd2']:6.3f} {r['mid']:6.3f} {r['rho_off']:7.3f} {str(r['vel_off']):>9s} "
              f"{r['wind']:6.3f} {r['fine']:6.3f} {r['fine0']:6.3f} {r['boil']:6.3f} {r['shimmer']:7.3f} {r['echo']:6.3f}")
    sys.stdout.flush()


# ----------------------------------------------------------------------------- still products

def label_img(img, text):
    im = Image.fromarray(img).convert("RGB")
    d = ImageDraw.Draw(im)
    f = gl.font(13)
    d.rectangle([0, 0, 8 * len(text) + 10, 17], fill=(0, 0, 0))
    d.text((4, 1), text, fill=(255, 230, 90), font=f)
    return np.asarray(im)


def contact(frames, labels, cols=4, tile_w=480):
    tiles = []
    for f, l in zip(frames, labels):
        im = Image.fromarray(np.clip(f, 0, 255).astype(np.uint8)).convert("RGB")
        im = im.resize((tile_w, max(1, round(im.height * tile_w / im.width))), Image.LANCZOS)
        tiles.append(label_img(np.asarray(im), l))
    th, tw = tiles[0].shape[:2]
    rows = (len(tiles) + cols - 1) // cols
    sheet = np.full((rows * (th + 4), cols * (tw + 4), 3), 24, np.uint8)
    for i, t in enumerate(tiles):
        r, c = divmod(i, cols)
        sheet[r * (th + 4):r * (th + 4) + th, c * (tw + 4):c * (tw + 4) + tw] = t
    return sheet


def velocity_map(x, block=30):
    """x: T x H x W grey at half resolution. Per block, the mean motion relative to the camera."""
    T, H, W = x.shape
    win = fm.hann(H, W)
    cam = [fm.phase_shift(x[t], x[t + 1], win) for t in range(T - 1)]
    gy, gx = H // block, W // block
    acc = np.zeros((gy, gx, 2))
    n = 0
    bw = fm.hann(block * 2, block * 2)
    for t in range(0, T - 1, max(1, (T - 1) // 40)):
        for yy in range(gy):
            for xx in range(gx):
                y0 = min(max(0, yy * block - block // 2), H - 2 * block)
                x0 = min(max(0, xx * block - block // 2), W - 2 * block)
                p = x[t, y0:y0 + 2 * block, x0:x0 + 2 * block]
                q = x[t + 1, y0:y0 + 2 * block, x0:x0 + 2 * block]
                if p.std() < 1.0:
                    continue
                fa = np.fft.rfft2((p - p.mean()) * bw)
                fb = np.fft.rfft2((q - q.mean()) * bw)
                r = fb * np.conj(fa)
                r /= np.abs(r) + 1e-9
                c = np.fft.irfft2(r, s=p.shape)
                dy, dx = fm.peak_subpix(c)
                acc[yy, xx] += (dy - cam[t][0], dx - cam[t][1])
        n += 1
    v = acc / max(n, 1)
    mag = np.hypot(v[..., 0], v[..., 1])
    ang = (np.degrees(np.arctan2(v[..., 0], v[..., 1])) % 360) / 360.0
    sat = np.ones_like(mag)
    val = np.clip(mag / 1.5, 0, 1)  # 1.5 px/frame (at half resolution) = full brightness
    hsv = np.stack([ang, sat, val], -1)
    rgb = np.asarray(Image.fromarray((hsv * 255).astype(np.uint8), "HSV").convert("RGB"))
    big = np.kron(rgb, np.ones((block, block, 1), np.uint8))
    return big, float(mag.mean()), float(mag.max())


def stills(clips, out, a, n, box, row, col, gain):
    os.makedirs(out, exist_ok=True)
    made = []
    xt_rows, yt_cols = [], []
    for spec in clips:
        src, lab = (spec.rsplit(":", 1) if ":" in spec[2:] else (spec, os.path.splitext(os.path.basename(spec))[0]))
        W, H = clip_size(src)
        bx, by, bw, bh = box if box else (0, 0, W, H)
        # 4 x 4 contact sheets at a 1-frame step
        fr = decode_box(src, a, a + 17, (by, by + bh, bx, bx + bw))
        sh = [fm.phase_shift(fr[t], fr[t + 1], fm.hann(*fr[t].shape)) for t in range(16)]
        res = [np.clip(128 + gain * (fr[t + 1] - fm.warp(fr[t], sh[t])), 0, 255) for t in range(16)]
        p1 = os.path.join(out, f"contact4x4_{lab}.png")
        Image.fromarray(contact(fr[:16], [f"{lab} f{a + i}" for i in range(16)])).save(p1)
        p2 = os.path.join(out, f"contact4x4_resid_{lab}.png")
        Image.fromarray(contact(res, [f"resid x{gain:g} f{a + i + 1}" for i in range(16)])).save(p2)
        # velocity and residual maps at half resolution
        x = decode_scaled(src, a, a + n, W // 2, H // 2)
        vm, vmean, vmax = velocity_map(x)
        # the first frame dimmed underneath, so the colours can be placed (parallax shows in every
        # clip: always read the candidate's map against the golden's)
        bg = np.clip(x[0][:vm.shape[0], :vm.shape[1]], 0, 255).astype(np.float32)
        vm = np.clip(0.7 * vm + 0.3 * bg[..., None], 0, 255).astype(np.uint8)
        p3 = os.path.join(out, f"velocity_{lab}.png")
        Image.fromarray(label_img(vm, f"{lab}: motion relative to the camera, mean {vmean:.2f} max {vmax:.2f} px/frame (half res)")).save(p3)
        win = fm.hann(H // 2, W // 2)
        R = np.mean([np.abs(x[t + 1] - fm.warp(x[t], fm.phase_shift(x[t], x[t + 1], win))) for t in range(0, len(x) - 1, 2)], axis=0)
        heat = np.clip(R * 16, 0, 255).astype(np.uint8)
        p4 = os.path.join(out, f"residual_{lab}.png")
        Image.fromarray(label_img(np.stack([heat, heat // 2, 255 - heat], -1), f"{lab}: mean |camera-compensated difference| x16")).save(p4)
        made += [p1, p2, p3, p4]
        # x-t and y-t slices (xtslice.py's RAW | STAB | RESID), time down / right
        ry = int(row * H)
        st = decode_box(src, a, a + n, (max(0, ry - 60), min(H, ry + 60), 0, W))
        shx = np.array([fm.phase_shift(st[t], st[t + 1], fm.hann(*st[t].shape)) for t in range(len(st) - 1)])
        mid = st.shape[1] // 2
        line = st[:, mid - 2:mid + 2, :].mean(axis=1)
        cum = np.concatenate([[0.0], np.cumsum(shx[:, 1])])
        stab = np.stack([ndimage.shift(line[t], -cum[t], order=3, mode="nearest") for t in range(len(line))])
        rs = np.stack([st[t + 1] - fm.warp(st[t], tuple(shx[t])) for t in range(len(shx))])[:, mid - 2:mid + 2, :].mean(axis=1)

        def norm(v):
            lo, hi = np.percentile(v, 1), np.percentile(v, 99)
            return np.clip(255 * (v - lo) / (hi - lo + 1e-6), 0, 255).astype(np.uint8)
        res8 = np.concatenate([np.full((1, W), 128, np.uint8), np.clip(128 + gain * rs, 0, 255).astype(np.uint8)])
        gap = np.zeros((len(line), 6), np.uint8)
        xt = np.concatenate([norm(line), gap, norm(stab), gap, res8], axis=1)
        xt_rows.append(label_img(np.repeat(xt, 2, axis=0), f"{lab}  x-t row y={ry}: RAW | STAB | RESID x{gain:g}  frames {a}-{a + len(line) - 1}"))
        cx = int(col * W)
        sc = decode_box(src, a, a + n, (0, H, max(0, cx - 60), min(W, cx + 60)))
        shy = np.array([fm.phase_shift(sc[t], sc[t + 1], fm.hann(*sc[t].shape)) for t in range(len(sc) - 1)])
        mc = sc.shape[2] // 2
        colv = sc[:, :, mc - 2:mc + 2].mean(axis=2)
        cumy = np.concatenate([[0.0], np.cumsum(shy[:, 0])])
        stabc = np.stack([ndimage.shift(colv[t], -cumy[t], order=3, mode="nearest") for t in range(len(colv))])
        rc = np.stack([sc[t + 1] - fm.warp(sc[t], tuple(shy[t])) for t in range(len(shy))])[:, :, mc - 2:mc + 2].mean(axis=2)
        rc8 = np.concatenate([np.full((1, H), 128, np.uint8), np.clip(128 + gain * rc, 0, 255).astype(np.uint8)])
        gapc = np.zeros((6, len(colv)), np.uint8)
        yt = np.concatenate([norm(colv).T, gapc, norm(stabc).T, gapc, rc8.T], axis=0)
        yt_cols.append(label_img(np.repeat(yt, 2, axis=1), f"{lab}  y-t col x={cx}: RAW / STAB / RESID (time ->)"))
        print(f"{lab}: stills written; camera x-t mean shift {shx[:, 1].mean():.2f} px/frame; velocity mean {vmean:.2f}")
        sys.stdout.flush()

    def stackv(imgs):
        w = max(i.shape[1] for i in imgs)
        return np.concatenate([np.pad(i, ((0, 8), (0, w - i.shape[1]), (0, 0))) for i in imgs], axis=0)
    p5, p6 = os.path.join(out, "xt.png"), os.path.join(out, "yt.png")
    Image.fromarray(stackv(xt_rows)).save(p5)
    Image.fromarray(stackv(yt_cols)).save(p6)
    return made + [p5, p6]


# ----------------------------------------------------------------------------- commands

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    m = sub.add_parser("metrics")
    m.add_argument("clips", nargs="+")
    m.add_argument("--ref", default="")
    s = sub.add_parser("stills")
    s.add_argument("clips", nargs="+")
    sc = sub.add_parser("screen")
    sc.add_argument("--cand", required=True)
    sc.add_argument("--ref", required=True)
    sc.add_argument("--vanilla", default="")
    for p in (m, s, sc):
        p.add_argument("--from", dest="a", type=int, default=0)
        p.add_argument("--to", dest="b", type=int, default=0)
        p.add_argument("--json", default="")
        p.add_argument("--out", default="")
    for p in (m, sc):
        p.add_argument("--regions", default="")
        p.add_argument("--preset", default="")
        p.add_argument("--static", action="store_true")
        p.add_argument("--noband", action="store_true")
    for p in (s, sc):
        p.add_argument("--n", type=int, default=120)
        p.add_argument("--box", default="")
        p.add_argument("--row", type=float, default=0.62)
        p.add_argument("--col", type=float, default=0.5)
        p.add_argument("--gain", type=float, default=16.0)
    o = ap.parse_args()
    if o.cmd == "stills":
        box = tuple(int(v) for v in o.box.split(",")) if o.box else None
        stills(o.clips, o.out or ".", o.a, o.n, box, o.row, o.col, o.gain)
        return 0
    if o.cmd == "metrics":
        regs = parse_regions(o.regions, o.preset, clip_size(o.clips[0]))
        recs = [metrics(c, regs, o.a, o.b, o.static, not o.noband) for c in o.clips]
        for r in recs:
            print_rec(r)
        out = dict(clips=recs)
        if o.ref:
            rr = metrics(o.ref, regs, o.a, o.b, o.static, not o.noband)
            print_rec(rr)
            out["ref"] = rr
            out["tripwires"] = {c["src"]: tripwires(c, rr) for c in recs}
            for src, tw in out["tripwires"].items():
                fired = [f"{t['region']}:{t['name']}" for t in tw if t["fired"]]
                print(f"tripwires {os.path.basename(src)} vs ref: {', '.join(fired) or 'quiet'}")
        if o.json:
            gl.write_json(o.json, out)
        return 0
    # screen: metrics + tripwires + still products for candidate, golden (and vanilla)
    os.makedirs(o.out or ".", exist_ok=True)
    regs = parse_regions(o.regions, o.preset, clip_size(o.cand))
    rc = metrics(o.cand, regs, o.a, o.b, o.static, not o.noband)
    rr = metrics(o.ref, regs, o.a, o.b, o.static, not o.noband)
    print_rec(rc)
    print_rec(rr)
    tw = tripwires(rc, rr)
    res = dict(cand=rc, ref=rr, tripwires=tw, verdict="REJECT-CANDIDATE (tripwire)" if any(t["fired"] for t in tw)
               else "NO TRIPWIRE (not an approval: owner motion sheet next)")
    if o.vanilla:
        rv = metrics(o.vanilla, regs, o.a, o.b, o.static, not o.noband)
        res["vanilla"] = rv
        res["tripwires_vs_vanilla"] = tripwires(rc, rv)
    clips = [f"{o.ref}:golden", f"{o.cand}:candidate"] + ([f"{o.vanilla}:vanilla"] if o.vanilla else [])
    box = tuple(int(v) for v in o.box.split(",")) if o.box else None
    res["stills"] = stills(clips, o.out or ".", o.a, o.n, box, o.row, o.col, o.gain)
    gl.write_json(os.path.join(o.out or ".", "motion.json"), res)
    print("verdict:", res["verdict"], "; fired:", ", ".join(f"{t['region']}:{t['name']}" for t in tw if t["fired"]) or "none")
    return 0


if __name__ == "__main__":
    sys.exit(main())
