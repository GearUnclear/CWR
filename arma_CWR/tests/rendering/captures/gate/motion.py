"""motion.py: the motion metric pack and the motion screener's still products (PERF-PLAN P0-10,
3.4 item 5, 3.6). Generalises fogcheck/fogmotion.py (flicker, travel, boil, grain per region,
camera-compensated), bandmetric.py (the block flicker band) and xtslice.py (x-t / y-t slices)
to any clip, and adds shimmer, LOD popping and ghosting. Its numbers are TRIPWIRES, never
approvals: a firing tripwire stops a T item before the owner sheet, a quiet one approves
nothing (3.6). Image work: run it through squad-heavy.ps1 -Kind analysis.

    python -I motion.py metrics <clip> [<clip> ...] [--ref <golden clip>] [--regions SPEC]
                                [--preset valley_mist|default] [--static] [--from N] [--to M]
                                [--json out.json] [--jobs N]
    python -I motion.py stills <clip>[:label] [<clip>[:label] ...] --out DIR [--from N] [--n 120]
                               [--box x,y,w,h] [--row 0.62] [--col 0.5] [--gain 16] [--jobs N]
    python -I motion.py screen --cand <clip> --ref <golden clip> [--vanilla <clip>] --out DIR
                               [--preset P | --regions SPEC] [--static] [--from N] [--to M] [--jobs N]

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

Speed (MOTION-FAST, 2026-11-01). The arithmetic is fogmotion's and the first motion.py's, value
for value; what changed is how often and where it runs:
  - every value is computed once: the first version ran each region's camera estimate, residual
    and mid band twice (analyse_region, then echo_and_shimmer again) and its fine band twice
    (fine and shimmer are the same number for a moving camera), and decoded the 480 x 270
    stack twice (band and pops); stills ran the half-resolution camera estimate twice;
  - an image warped several times (the 1 px smoothed frame in the 3 Lucas-Kanade steps, a mid
    band frame warped for the travel pair and 3 echo lags) gets its cubic spline coefficients
    once; the warp is scipy's own zoom_shift call on scipy's own pre-padded spline_filter
    output (bit-identical to ndimage.shift, checked at start; falls back to ndimage.shift);
  - frames are decoded once per stack into shared memory and frame chunks run in --jobs worker
    processes (default: env MOTION_JOBS, else min(16, cores - 4); --jobs 1 runs in-process).
  Only the reductions that summed a whole float32 stack in one go (mid, fine, shimmer, the
  region mean) are now summed per frame in float64, so they can differ from the first version in
  about the 6th significant digit (more exact, not less). Everything else is bit-identical.
"""
import argparse
import concurrent.futures as cf
import glob
import json
import math
import multiprocessing as mp
import os
import subprocess
import sys
import time
from multiprocessing import shared_memory

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
CHUNK = 64  # frames per worker task


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


def _ff_gray(src, vf, a, b, h, w):
    """uint8 T x h x w from ffmpeg (the first version's filter strings, so the same bytes)."""
    if a or b:
        vf = f"trim=start_frame={a}" + (f":end_frame={b}" if b else "") + ",setpts=PTS-STARTPTS," + vf
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-vsync", "0", "-f", "rawvideo",
                        "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, h, w)


def decode_box_u8(src, a, b, box):
    """Grey uint8 T x h x w of the pixel box (y0, y1, x0, x1); frames [a, b) (fm.decode's bytes)."""
    y0, y1, x0, x1 = box
    if os.path.isdir(src):
        return fm.decode(src, a, b, box).astype(np.uint8)
    return _ff_gray(src, f"crop={x1 - x0}:{y1 - y0}:{x0}:{y0},format=gray", a, b, y1 - y0, x1 - x0)


def decode_scaled_u8(src, a, b, w, h):
    if os.path.isdir(src):
        fs = frame_files(src)[a:(b or None)]
        return np.stack([np.asarray(Image.open(f).convert("L").resize((w, h), Image.BOX), np.uint8) for f in fs])
    return _ff_gray(src, f"scale={w}:{h}:flags=area,format=gray", a, b, h, w)


def decode_box(src, a, b, box):
    """Grey float32 T x h x w of the pixel box (y0, y1, x0, x1); frames [a, b)."""
    return fm.decode(src, a, b, box)


def decode_scaled(src, a, b, w, h, gray=True):
    if gray:
        return decode_scaled_u8(src, a, b, w, h).astype(np.float32)
    if os.path.isdir(src):
        fs = frame_files(src)[a:(b or None)]
        return np.stack([np.asarray(Image.open(f).convert("RGB").resize((w, h), Image.BOX), np.float32) for f in fs])
    vf = f"scale={w}:{h}:flags=area,format=rgb24"
    if a or b:
        vf = f"trim=start_frame={a}" + (f":end_frame={b}" if b else "") + ",setpts=PTS-STARTPTS," + vf
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-vsync", "0", "-f", "rawvideo",
                        "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape((-1, h, w, 3))


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


# ----------------------------------------------------------------------------- exact fast kernels

_NPAD = 12  # scipy's pre-padding for mode="nearest" (ndimage._interpolation._prepad_for_spline_filter)
try:
    from scipy.ndimage import _nd_image as _NDI
    from scipy.ndimage import _ni_support as _NIS
    _NEAREST = _NIS._extend_mode_to_code("nearest")
except Exception:  # pragma: no cover
    _NDI = None
_FAST_WARP = None


def _fast_warp_ok():
    """True when the cached-spline warp reproduces ndimage.shift(order=3, mode='nearest') bit for bit."""
    global _FAST_WARP
    if _FAST_WARP is None:
        ok = _NDI is not None
        if ok:
            try:
                rng = np.random.default_rng(7)
                img = (rng.random((41, 67)) * 255).astype(np.float32)
                coef = _spline_coef(img)
                for d in ((0.0, 0.0), (0.37, -1.62), (-3.2, 2.71), (13.4, -15.25), (np.float64(0.5), np.float64(-0.25))):
                    if not np.array_equal(_zoom_shift(coef, img.shape, img.dtype, d), fm.warp(img, d)):
                        ok = False
                        break
            except Exception:
                ok = False
        if not ok:
            print("motion: cached-spline warp disabled (scipy differs); using ndimage.shift", file=sys.stderr)
        _FAST_WARP = ok
    return _FAST_WARP


def _spline_coef(img):
    """What ndimage.shift(img, d, order=3, mode='nearest') builds before it interpolates."""
    return ndimage.spline_filter(np.pad(img, _NPAD, mode="edge"), 3, output=np.float64, mode="nearest")


def _zoom_shift(coef, shape, dtype, d):
    out = np.zeros(shape, dtype=np.dtype(dtype).name)
    sh = np.asarray([-ii for ii in _NIS._normalize_sequence(d, len(shape))], dtype=np.float64)
    if not sh.flags.contiguous:
        sh = sh.copy()
    _NDI.zoom_shift(coef, None, sh, out, 3, _NEAREST, 0.0, _NPAD, False)
    return out


class Spline:
    """An image to be warped by several shifts: the spline coefficients are built once, on the
    first warp. warp(d) == fm.warp(img, d), bit for bit."""
    __slots__ = ("img", "coef")

    def __init__(self, img):
        self.img = img
        self.coef = None

    def warp(self, d):
        if not _fast_warp_ok():
            return fm.warp(self.img, d)
        if self.coef is None:
            self.coef = _spline_coef(self.img)
        return _zoom_shift(self.coef, self.img.shape, self.img.dtype, d)


class _PF:
    """One frame's part of fm.phase_shift: its windowed spectrum and 1 px smoothed image (shared
    by the pair before and the pair after it) and that image's spline (3 warps per pair)."""
    __slots__ = ("shape", "F", "S", "sp")

    def __init__(self, x, win):
        self.shape = x.shape
        self.F = np.fft.rfft2((x - x.mean()) * win)
        self.S = ndimage.gaussian_filter(x, 1.0)
        self.sp = Spline(self.S)


def phase_shift_pf(pa, pb):
    """fm.phase_shift(a, b, win), from the two frames' cached parts (same operations, same order)."""
    r = pb.F * np.conj(pa.F)
    r /= np.abs(r) + 1e-9
    c = np.fft.irfft2(r, s=pa.shape)
    d = np.array(fm.peak_subpix(c))
    sb = pb.S
    m = 24
    for _ in range(3):
        wa = pa.sp.warp(tuple(d))[m:-m, m:-m]
        bb = sb[m:-m, m:-m]
        gy, gx = np.gradient(wa)
        it = bb - wa
        A = np.array([[(gy * gy).sum(), (gy * gx).sum()], [(gy * gx).sum(), (gx * gx).sum()]])
        rhs = -np.array([(gy * it).sum(), (gx * it).sum()])
        try:
            step = np.linalg.solve(A, rhs)
        except np.linalg.LinAlgError:
            break
        if np.abs(step).max() > 1.0:
            break
        d = d + step
    return float(d[0]), float(d[1])


def shifts_range(get, t0, t1, win):
    """[fm.phase_shift(x[t], x[t + 1], win) for t in range(t0, t1)]."""
    out = []
    if t1 <= t0:
        return out
    prev = _PF(get(t0), win)
    for t in range(t0, t1):
        nxt = _PF(get(t + 1), win)
        out.append(phase_shift_pf(prev, nxt))
        prev = nxt
    return out


# ----------------------------------------------------------------------------- shared frames

_ATTACHED = {}


def _attach(spec):
    name, shape, dt = spec
    v = _ATTACHED.get(name)
    if v is None:
        shm = shared_memory.SharedMemory(name=name)
        v = (shm, np.ndarray(shape, np.dtype(dt), buffer=shm.buf))
        _ATTACHED[name] = v
    return v[1]


def _getter(X):
    return lambda t: X[t].astype(np.float32)


# ----------------------------------------------------------------------------- worker tasks

def _t_shifts(spec, t0, t1, static, extra):
    """Camera shifts of pairs (t, t + 1), t in [t0, t1) (static: zero); `extra` = 'region' adds the
    60 px block means and the pixel sum of frames [t0, t1); 'pops' the popping blocks of the pairs."""
    X = _attach(spec)
    T, h, w = X.shape
    get = _getter(X)
    tp = min(t1, T - 1)
    sh = [(0.0, 0.0)] * max(0, tp - t0) if static else shifts_range(get, t0, tp, fm.hann(h, w))
    res = dict(shifts=sh)
    if extra == "region":
        B = 60
        gy, gx = h // B, w // B
        res["blk"] = [get(t)[:gy * B, :gx * B].reshape(gy, B, gx, B).mean(axis=(1, 3)) for t in range(t0, t1)] \
            if gy and gx else []
        res["sum"] = int(X[t0:t1].sum(dtype=np.int64))
    elif extra == "pops":
        m, B = 8, 8
        blk = []
        for t, s in zip(range(t0, tp), sh):
            R = np.abs(get(t + 1) - fm.warp(get(t), s))[m:h - m, m:w - m]
            gy, gx = R.shape[0] // B, R.shape[1] // B
            blk.append(R[:gy * B, :gx * B].reshape(gy, B, gx, B).mean(axis=(1, 3)))
        res["blk"] = blk
    return res


def _t_resid(spec, shifts, cum, t0, t1):
    """The residual products of D[j] = x[j + 1] - warp(x[j], shift j) for j in [t0, t1): the mid and
    fine band sums of squares, the travel surface xcorr and fine0 of pair (j - 1, j), and the echo
    correlations of D[j] with D[j - k], k = 2, 3, 4 (D from t0 - 4 on is recomputed locally)."""
    X = _attach(spec)
    T, h, w = X.shape
    get = _getter(X)
    m = 20
    inner = (slice(m, h - m), slice(m, w - m))
    out = dict(m2=0.0, f2=0.0, xc={}, f0={}, ech={2: {}, 3: {}, 4: {}})
    hist = {}
    for j in range(max(0, t0 - 4), t1):
        D = get(j + 1) - fm.warp(get(j), shifts[j])
        Dm = ndimage.gaussian_filter(D, 2.0) - ndimage.gaussian_filter(D, 16.0)
        Df = D - ndimage.gaussian_filter(D, 1.5)
        if j >= t0:
            b = Dm[inner]
            bf = Df[inner]
            out["m2"] += float(np.square(b, dtype=np.float64).sum())
            out["f2"] += float(np.square(bf, dtype=np.float64).sum())
            if j >= 1:
                pm, pf = hist[j - 1]
                a = pm.warp(shifts[j])[inner]
                out["xc"][j - 1] = fm.xcorr_local(a, b)
                af = pf.warp(shifts[j])[inner]
                out["f0"][j - 1] = (af * bf).sum() / (np.sqrt((af * af).sum() * (bf * bf).sum()) + 1e-9)
            for k in (2, 3, 4):
                if j >= k:
                    s = cum[j - 1] - cum[j - k - 1] if j - k - 1 >= 0 else cum[j - 1]
                    a = hist[j - k][0].warp(tuple(s))[inner]
                    out["ech"][k][j] = float((a * b).sum() / (np.sqrt((a * a).sum() * (b * b).sum()) + 1e-9))
        hist[j] = (Spline(Dm), Spline(Df))
        hist.pop(j - 4, None)
    return out


def _t_static(spec):
    """--static only: boil, slow and the static shimmer (whole-stack statistics, as before)."""
    x = _attach(spec).astype(np.float32)
    T, h, w = x.shape
    m = 20
    M = np.stack([ndimage.gaussian_filter(f, 2.0) - ndimage.gaussian_filter(f, 16.0) for f in x])[:, m:h - m, m:w - m]
    M = M - M.mean(axis=0, keepdims=True)
    boil = float(np.sqrt((M ** 2).mean()))
    slow = float("nan")
    if T > 12:
        k6 = ndimage.uniform_filter1d(M, 6, axis=0)[3:-3]
        slow = float(np.sqrt((k6 ** 2).mean()))
    del M
    fine = np.stack([f - ndimage.gaussian_filter(f, 1.5) for f in x])[:, m:h - m, m:w - m]
    shimmer = float(fine.std(axis=0).mean())
    return dict(boil=boil, slow=slow, shimmer=shimmer)


def _t_contact(spec, sh, lab, a, gain, out):
    fr = _attach(spec).astype(np.float32)
    res = [np.clip(128 + gain * (fr[t + 1] - fm.warp(fr[t], sh[t])), 0, 255) for t in range(16)]
    p1 = os.path.join(out, f"contact4x4_{lab}.png")
    Image.fromarray(contact(fr[:16], [f"{lab} f{a + i}" for i in range(16)])).save(p1)
    p2 = os.path.join(out, f"contact4x4_resid_{lab}.png")
    Image.fromarray(contact(res, [f"resid x{gain:g} f{a + i + 1}" for i in range(16)])).save(p2)
    return [p1, p2]


def _t_velres(spec, cam, lab, out):
    x = _attach(spec).astype(np.float32)
    vm, vmean, vmax = velocity_map(x, cam=cam)
    # the first frame dimmed underneath, so the colours can be placed (parallax shows in every
    # clip: always read the candidate's map against the golden's)
    bg = np.clip(x[0][:vm.shape[0], :vm.shape[1]], 0, 255).astype(np.float32)
    vm = np.clip(0.7 * vm + 0.3 * bg[..., None], 0, 255).astype(np.uint8)
    p3 = os.path.join(out, f"velocity_{lab}.png")
    Image.fromarray(label_img(vm, f"{lab}: motion relative to the camera, mean {vmean:.2f} max {vmax:.2f} px/frame (half res)")).save(p3)
    R = np.mean([np.abs(x[t + 1] - fm.warp(x[t], cam[t])) for t in range(0, len(x) - 1, 2)], axis=0)
    heat = np.clip(R * 16, 0, 255).astype(np.uint8)
    p4 = os.path.join(out, f"residual_{lab}.png")
    Image.fromarray(label_img(np.stack([heat, heat // 2, 255 - heat], -1), f"{lab}: mean |camera-compensated difference| x16")).save(p4)
    return dict(made=[p3, p4], vmean=vmean)


def _norm8(v):
    lo, hi = np.percentile(v, 1), np.percentile(v, 99)
    return np.clip(255 * (v - lo) / (hi - lo + 1e-6), 0, 255).astype(np.uint8)


def _t_xt(spec, shx, lab, ry, a, gain):
    """x-t slice (xtslice.py's RAW | STAB | RESID), time down."""
    st = _attach(spec).astype(np.float32)
    W = st.shape[2]
    shx = np.array(shx)
    mid = st.shape[1] // 2
    line = st[:, mid - 2:mid + 2, :].mean(axis=1)
    cum = np.concatenate([[0.0], np.cumsum(shx[:, 1])])
    stab = np.stack([ndimage.shift(line[t], -cum[t], order=3, mode="nearest") for t in range(len(line))])
    rs = np.stack([st[t + 1] - fm.warp(st[t], tuple(shx[t])) for t in range(len(shx))])[:, mid - 2:mid + 2, :].mean(axis=1)
    res8 = np.concatenate([np.full((1, W), 128, np.uint8), np.clip(128 + gain * rs, 0, 255).astype(np.uint8)])
    gap = np.zeros((len(line), 6), np.uint8)
    xt = np.concatenate([_norm8(line), gap, _norm8(stab), gap, res8], axis=1)
    img = label_img(np.repeat(xt, 2, axis=0), f"{lab}  x-t row y={ry}: RAW | STAB | RESID x{gain:g}  frames {a}-{a + len(line) - 1}")
    return dict(img=img, shift=float(shx[:, 1].mean()))


def _t_yt(spec, shy, lab, cx):
    sc = _attach(spec).astype(np.float32)
    H = sc.shape[1]
    shy = np.array(shy)
    mc = sc.shape[2] // 2
    colv = sc[:, :, mc - 2:mc + 2].mean(axis=2)
    cumy = np.concatenate([[0.0], np.cumsum(shy[:, 0])])
    stabc = np.stack([ndimage.shift(colv[t], -cumy[t], order=3, mode="nearest") for t in range(len(colv))])
    return dict(colv=colv, stabc=stabc, rc=np.stack([sc[t + 1] - fm.warp(sc[t], tuple(shy[t])) for t in range(len(shy))])
                [:, :, mc - 2:mc + 2].mean(axis=2), H=H, lab=lab, cx=cx)


def _yt_img(r, gain):
    colv, stabc, rc, H = r["colv"], r["stabc"], r["rc"], r["H"]
    rc8 = np.concatenate([np.full((1, H), 128, np.uint8), np.clip(128 + gain * rc, 0, 255).astype(np.uint8)])
    gapc = np.zeros((6, len(colv)), np.uint8)
    yt = np.concatenate([_norm8(colv).T, gapc, _norm8(stabc).T, gapc, rc8.T], axis=0)
    return label_img(np.repeat(yt, 2, axis=1), f"{r['lab']}  y-t col x={r['cx']}: RAW / STAB / RESID (time ->)")


# ----------------------------------------------------------------------------- scheduler

def default_jobs():
    e = os.environ.get("MOTION_JOBS", "").strip()
    if e.isdigit() and int(e) > 0:
        return int(e)
    return max(1, min(16, (os.cpu_count() or 2) - 4))


class _Runner:
    """Worker processes for the frame tasks, threads for the ffmpeg decodes; every result goes to
    its callback in the main process (which may submit more). jobs == 1: everything inline."""

    def __init__(self, jobs):
        self.jobs = jobs
        self.pool = None
        self.threads = None
        if jobs > 1:
            for k in ("OMP_NUM_THREADS", "OPENBLAS_NUM_THREADS", "MKL_NUM_THREADS"):
                os.environ.setdefault(k, "1")
            self.pool = cf.ProcessPoolExecutor(jobs, mp_context=mp.get_context("spawn"))
            self.threads = cf.ThreadPoolExecutor(6)
        self.pending = {}
        self.shms = []

    def share(self, arr):
        shm = shared_memory.SharedMemory(create=True, size=max(1, arr.nbytes))
        v = np.ndarray(arr.shape, arr.dtype, buffer=shm.buf)
        v[...] = arr
        self.shms.append(shm)
        spec = (shm.name, arr.shape, arr.dtype.str)
        if self.pool is None:
            _ATTACHED[shm.name] = (shm, v)
        return spec

    def task(self, cb, fn, *args):
        if self.pool is None:
            cb(fn(*args))
        else:
            self.pending[self.pool.submit(fn, *args)] = cb

    def decode(self, cb, fn, *args):
        if self.threads is None:
            cb(fn(*args))
        else:
            self.pending[self.threads.submit(fn, *args)] = cb

    def drain(self):
        while self.pending:
            done, _ = cf.wait(list(self.pending), return_when=cf.FIRST_COMPLETED)
            for f in done:
                self.pending.pop(f)(f.result())

    def close(self):
        for f in list(self.pending):
            f.cancel()
        if self.threads:
            self.threads.shutdown(wait=True, cancel_futures=True)
        if self.pool:
            self.pool.shutdown(wait=True, cancel_futures=True)
        for shm in self.shms:
            _ATTACHED.pop(shm.name, None)
            for op in (shm.close, shm.unlink):
                try:
                    op()
                except Exception:
                    pass


def _chunks(n, size=CHUNK):
    return [(s, min(n, s + size)) for s in range(0, n, size)]


def _shift_stack(R, spec, n_pairs, static, extra, done, size=CHUNK):
    """Run _t_shifts over all pairs of a stack in chunks; done(list of chunk results in order)."""
    parts = _chunks(n_pairs + (1 if extra == "region" else 0), size) or [(0, 0)]
    got = {}

    def one(i):
        def cb(r):
            got[i] = r
            if len(got) == len(parts):
                done([got[k] for k in range(len(parts))])
        return cb
    for i, (t0, t1) in enumerate(parts):
        R.task(one(i), _t_shifts, spec, t0, t1, static, extra)


def _region_job(R, src, box, a, b, static, put):
    """Decode a region, then camera shifts (pass 1), residual products (pass 2), then put(rec)."""
    st = {}

    def decoded(X):
        T, h, w = X.shape
        st.update(T=T, h=h, w=w)
        st["spec"] = R.share(X)
        _shift_stack(R, st["spec"], T - 1, static, "region", pass1)

    def pass1(parts):
        st["shifts"] = [s for p in parts for s in p["shifts"]]
        st["blk"] = [x for p in parts for x in p["blk"]]
        st["sum"] = sum(p["sum"] for p in parts)
        sh = st["shifts"]
        cum = np.cumsum(np.array(sh), axis=0) if len(sh) else np.zeros((0, 2))
        rparts = _chunks(st["T"] - 1)
        st["need"] = len(rparts) + (1 if static else 0)
        st["res"] = {}
        for i, (t0, t1) in enumerate(rparts):
            R.task(lambda r, i=i: pass2(i, r), _t_resid, st["spec"], sh, cum, t0, t1)
        if static:
            R.task(lambda r: pass2("static", r), _t_static, st["spec"])
        if not st["need"]:
            put(_finish_region(st))

    def pass2(i, r):
        st["res"][i] = r
        if len(st["res"]) == st["need"]:
            put(_finish_region(st))
    R.decode(decoded, decode_box_u8, src, a, b, box)


def _finish_region(st):
    """fm.analyse_region's summary and motion.py's echo and shimmer from the worker products."""
    T, h, w, static = st["T"], st["h"], st["w"], "static" in st["res"]
    LAG, m = fm.LAG, 20
    parts = [st["res"][k] for k in sorted(k for k in st["res"] if k != "static")]
    xc, f0, ech = {}, {}, {2: {}, 3: {}, 4: {}}
    m2 = f2 = 0.0
    for p in parts:
        xc.update(p["xc"])
        f0.update(p["f0"])
        for k in (2, 3, 4):
            ech[k].update(p["ech"][k])
        m2 += p["m2"]
        f2 += p["f2"]
    surf = np.zeros((2 * LAG + 1, 2 * LAG + 1))
    for t in range(T - 2):
        surf += xc[t]
    surf /= max(T - 2, 1)
    iy, ix = np.unravel_index(np.argmax(surf), surf.shape)
    yy, xx = np.mgrid[-LAG:LAG + 1, -LAG:LAG + 1]
    off = np.hypot(yy, xx) >= 0.75
    so = np.where(off, surf, -9)
    oy, ox = np.unravel_index(np.argmax(so), so.shape)
    rho_off = float(surf[oy, ox])
    c = surf[LAG - 2:LAG + 3, LAG - 2:LAG + 3]
    cy, cx = np.unravel_index(np.argmax(c), c.shape)

    def _para(a, b, d):
        q = a - 2 * b + d
        return 0.0 if abs(q) < 1e-12 or not (0 < cy < 4 and 0 < cx < 4) else 0.5 * (a - d) / q
    sy = cy - 2 + (_para(c[cy - 1, cx], c[cy, cx], c[cy + 1, cx]) if 0 < cy < 4 else 0.0)
    sx = cx - 2 + (_para(c[cy, cx - 1], c[cy, cx], c[cy, cx + 1]) if 0 < cx < 4 else 0.0)
    rho_c = float(c.max())
    n_in = (T - 1) * (h - 2 * m) * (w - 2 * m)
    midrms = math.sqrt(m2 / n_in) if n_in > 0 else float("nan")
    finerms = math.sqrt(f2 / n_in) if n_in > 0 else float("nan")
    if st["blk"]:
        blk = np.stack(st["blk"])
        d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
        lowd2 = float(np.sqrt((d2 ** 2).mean()))
    else:
        lowd2 = float("nan")
    sh = np.array(st["shifts"])
    sr = st["res"].get("static") or {}
    boil, slow = sr.get("boil", float("nan")), sr.get("slow", float("nan"))
    f0l = [f0[t] for t in range(T - 2)]
    res = dict(boil=round(boil, 3), slow=round(slow, 3), mean=round(st["sum"] / float(T * h * w), 2),
               camera=[round(float(sh[:, 1].mean()), 2), round(float(sh[:, 0].mean()), 2)],
               lowd2=round(lowd2, 3), mid=round(midrms, 3), rho=round(float(surf[iy, ix]), 3),
               rho0=round(float(surf[LAG, LAG]), 3), vel=[int(ix - LAG), int(iy - LAG)],
               rho_off=round(rho_off, 3), vel_off=[int(ox - LAG), int(oy - LAG)],
               wind=round(midrms * max(rho_off, 0.0), 3), rho_c=round(rho_c, 3), vslow=[round(float(sx), 2), round(float(sy), 2)],
               fine=round(finerms, 3), fine0=round(float(np.mean(f0l)), 3))
    el = [ech[k][t] for k in (2, 3, 4) for t in range(k, T - 1)]
    res["echo"] = round(float(np.mean(el)) if el else float("nan"), 4)
    res["shimmer"] = round(sr["shimmer"] if static else finerms, 4)
    return dict(region=res, frames=int(T))


def _pops_finish(blks, a, static, W=480, H=270, B=8):
    """pops(): the popping counter from the per-pair 8 px block means of the residual."""
    if len(blks) + 1 < 4:
        return float("nan"), []
    m = 8
    blk = np.stack(blks)
    medb = np.median(blk, axis=0)
    thr = np.maximum(6.0, 5.0 * medb)
    ev = []
    for t in range(1, len(blk) - 1):
        hit = (blk[t] > thr) & (blk[t - 1] < 0.5 * blk[t]) & (blk[t + 1] < 0.5 * blk[t])
        for yy, xx in zip(*np.nonzero(hit)):
            ev.append(dict(frame=int(t + 1 + a), y=int((yy * B + m) * 1080 / H), x=int((xx * B + m) * 1920 / W),
                           size=round(float(blk[t, yy, xx]), 1)))
    return round(100.0 * len(ev) / max(1, len(blk)), 3), ev[:50]


def _band_from(x8):
    """fm.bandmetric_band on an already decoded 480 x 270 grey stack (the same filter string)."""
    W, H, B = 480, 270, 15
    x = x8.astype(np.float32)
    blk = x[:, :H // B * B, :W // B * B].reshape(len(x), H // B, B, W // B, B).mean(axis=(2, 4))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    return round(float(rms[8:14, 5:32].mean()), 3), round(float(rms[0:6].mean()), 3)


def _clip_metrics(R, src, regs, a, b, static, band, put):
    """metrics() of one clip through the runner; put(rec) when complete."""
    got = dict(regions={})

    def finish():
        if len(got["regions"]) < len(regs) or "pops" not in got:
            return
        rec = dict(src=src, frm=a, to=b, static=static, regions={})
        if got.get("band"):
            rec["band"], rec["sky"] = got["band"]
        for n in regs:
            rec["regions"][n] = got["regions"][n]["region"]
            rec["frames"] = got["regions"][n]["frames"]
        rec["pops_per100"], rec["pop_events"] = got["pops"]
        put(rec)

    for name, box in regs.items():
        def reg_done(r, name=name):
            got["regions"][name] = r
            finish()
        _region_job(R, src, box, a, b, static, reg_done)

    def pops_decoded(X):
        if band and not os.path.isdir(src):
            got["band"] = _band_from(X)
        T = X.shape[0]
        if T < 4:
            got["pops"] = (float("nan"), [])
            finish()
            return
        spec = R.share(X)

        def pops_done(parts):
            got["pops"] = _pops_finish([x for p in parts for x in p["blk"]], a, static)
            finish()
        _shift_stack(R, spec, T - 1, static, "pops", pops_done)
    R.decode(pops_decoded, decode_scaled_u8, src, a, b, 480, 270)


def _clip_stills(R, spec_str, i, out, a, n, box, row, col, gain, put):
    """stills() of one clip: contact sheets, velocity and residual maps, x-t and y-t slices."""
    src, lab = (spec_str.rsplit(":", 1) if ":" in spec_str[2:] else (spec_str, os.path.splitext(os.path.basename(spec_str))[0]))
    W, H = clip_size(src)
    bx, by, bw, bh = box if box else (0, 0, W, H)
    got = {}
    want = ("contact", "velres", "xt", "yt")

    def part(key):
        def cb(r):
            got[key] = r
            if all(k in got for k in want):
                put(i, lab, got)
        return cb

    def stack(decoder, args, after):
        def decoded(X):
            spec = R.share(X)
            _shift_stack(R, spec, X.shape[0] - 1, False, None,
                         lambda parts: after(spec, [s for p in parts for s in p["shifts"]]),
                         size=8 if X.shape[1] * X.shape[2] > 1000000 else 32)
        R.decode(decoded, decoder, *args)

    stack(decode_box_u8, (src, a, a + 17, (by, by + bh, bx, bx + bw)),
          lambda spec, sh: R.task(part("contact"), _t_contact, spec, sh, lab, a, gain, out))
    stack(decode_scaled_u8, (src, a, a + n, W // 2, H // 2),
          lambda spec, sh: R.task(part("velres"), _t_velres, spec, sh, lab, out))
    ry = int(row * H)
    stack(decode_box_u8, (src, a, a + n, (max(0, ry - 60), min(H, ry + 60), 0, W)),
          lambda spec, sh: R.task(part("xt"), _t_xt, spec, sh, lab, ry, a, gain))
    cx = int(col * W)
    stack(decode_box_u8, (src, a, a + n, (0, H, max(0, cx - 60), min(W, cx + 60))),
          lambda spec, sh: R.task(part("yt"), _t_yt, spec, sh, lab, cx))


def analyse(srcs, regs, a, b, static, band=True, stills_spec=None, jobs=0):
    """metrics() of every clip in `srcs` and, with stills_spec (dict of stills() arguments), the
    still products, on one worker pool. Returns (recs in srcs order, stills paths or None)."""
    jobs = jobs or default_jobs()
    _fast_warp_ok()
    R = _Runner(jobs)
    recs = [None] * len(srcs)
    st = {}
    try:
        # the region stacks first: they carry most of the work
        for i, src in enumerate(srcs):
            _clip_metrics(R, src, regs, a, b, static, band, lambda rec, i=i: recs.__setitem__(i, rec))
        if stills_spec:
            os.makedirs(stills_spec["out"], exist_ok=True)
            for i, c in enumerate(stills_spec["clips"]):
                _clip_stills(R, c, i, stills_spec["out"], stills_spec["a"], stills_spec["n"], stills_spec["box"],
                             stills_spec["row"], stills_spec["col"], stills_spec["gain"],
                             lambda i, lab, got: st.__setitem__(i, (lab, got)))
        R.drain()
    finally:
        R.close()
    made = None
    if stills_spec:
        made, xt_rows, yt_cols = [], [], []
        for i in range(len(stills_spec["clips"])):
            lab, got = st[i]
            made += got["contact"] + got["velres"]["made"]
            xt_rows.append(got["xt"]["img"])
            yt_cols.append(_yt_img(got["yt"], stills_spec["gain"]))
        out = stills_spec["out"]

        def stackv(imgs):
            w = max(i.shape[1] for i in imgs)
            return np.concatenate([np.pad(i, ((0, 8), (0, w - i.shape[1]), (0, 0))) for i in imgs], axis=0)
        p5, p6 = os.path.join(out, "xt.png"), os.path.join(out, "yt.png")
        Image.fromarray(stackv(xt_rows)).save(p5)
        Image.fromarray(stackv(yt_cols)).save(p6)
        made += [p5, p6]
        st["lines"] = [f"{st[i][0]}: stills written; camera x-t mean shift {st[i][1]['xt']['shift']:.2f} px/frame; "
                       f"velocity mean {st[i][1]['velres']['vmean']:.2f}" for i in range(len(stills_spec["clips"]))]
    return recs, (made, st.get("lines", []))


# ----------------------------------------------------------------------------- metrics

def metrics(src, regs, a, b, static, band=True, jobs=0):
    return analyse([src], regs, a, b, static, band, jobs=jobs)[0][0]


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


def velocity_map(x, block=30, cam=None):
    """x: T x H x W grey at half resolution. Per block, the mean motion relative to the camera.
    cam: the frame-to-frame camera shifts when already known (fm.phase_shift with hann(H, W))."""
    T, H, W = x.shape
    if cam is None:
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


def stills(clips, out, a, n, box, row, col, gain, jobs=0):
    spec = dict(clips=list(clips), out=out, a=a, n=n, box=box, row=row, col=col, gain=gain)
    _, (made, lines) = analyse([], {}, a, 0, False, False, spec, jobs)
    for line in lines:
        print(line)
    sys.stdout.flush()
    return made


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
        p.add_argument("--jobs", type=int, default=0, help="worker processes (0: env MOTION_JOBS, else min(16, cores - 4))")
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
    t_start = time.perf_counter()
    jobs = o.jobs or default_jobs()
    if o.cmd == "stills":
        box = tuple(int(v) for v in o.box.split(",")) if o.box else None
        stills(o.clips, o.out or ".", o.a, o.n, box, o.row, o.col, o.gain, jobs)
        print(f"motion: stills of {len(o.clips)} clip(s) in {time.perf_counter() - t_start:.1f} s, {jobs} worker(s)")
        return 0
    if o.cmd == "metrics":
        regs = parse_regions(o.regions, o.preset, clip_size(o.clips[0]))
        srcs = list(o.clips) + ([o.ref] if o.ref else [])
        allr, _ = analyse(srcs, regs, o.a, o.b, o.static, not o.noband, jobs=jobs)
        recs = allr[:len(o.clips)]
        for r in recs:
            print_rec(r)
        out = dict(clips=recs)
        if o.ref:
            rr = allr[-1]
            print_rec(rr)
            out["ref"] = rr
            out["tripwires"] = {c["src"]: tripwires(c, rr) for c in recs}
            for src, tw in out["tripwires"].items():
                fired = [f"{t['region']}:{t['name']}" for t in tw if t["fired"]]
                print(f"tripwires {os.path.basename(src)} vs ref: {', '.join(fired) or 'quiet'}")
        if o.json:
            gl.write_json(o.json, out)
        print(f"motion: metrics of {len(srcs)} clip(s) in {time.perf_counter() - t_start:.1f} s, {jobs} worker(s)")
        return 0
    # screen: metrics + tripwires + still products for candidate, golden (and vanilla), one pool
    os.makedirs(o.out or ".", exist_ok=True)
    regs = parse_regions(o.regions, o.preset, clip_size(o.cand))
    srcs = [o.cand, o.ref] + ([o.vanilla] if o.vanilla else [])
    clips = [f"{o.ref}:golden", f"{o.cand}:candidate"] + ([f"{o.vanilla}:vanilla"] if o.vanilla else [])
    box = tuple(int(v) for v in o.box.split(",")) if o.box else None
    spec = dict(clips=clips, out=o.out or ".", a=o.a, n=o.n, box=box, row=o.row, col=o.col, gain=o.gain)
    recs, (made, lines) = analyse(srcs, regs, o.a, o.b, o.static, not o.noband, spec, jobs)
    rc, rr = recs[0], recs[1]
    print_rec(rc)
    print_rec(rr)
    tw = tripwires(rc, rr)
    res = dict(cand=rc, ref=rr, tripwires=tw, verdict="REJECT-CANDIDATE (tripwire)" if any(t["fired"] for t in tw)
               else "NO TRIPWIRE (not an approval: owner motion sheet next)")
    if o.vanilla:
        rv = recs[2]
        res["vanilla"] = rv
        res["tripwires_vs_vanilla"] = tripwires(rc, rv)
    for line in lines:
        print(line)
    res["stills"] = made
    gl.write_json(os.path.join(o.out or ".", "motion.json"), res)
    print(f"motion: screen of {len(srcs)} clip(s) in {time.perf_counter() - t_start:.1f} s, {jobs} worker(s)")
    print("verdict:", res["verdict"], "; fired:", ", ".join(f"{t['region']}:{t['name']}" for t in tw if t["fired"]) or "none")
    return 0


if __name__ == "__main__":
    sys.exit(main())
