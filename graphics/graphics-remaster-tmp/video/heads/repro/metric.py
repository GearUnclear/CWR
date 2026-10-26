#!/usr/bin/env python
"""Head-wobble metric for the v2 video frames (repro probes and real clips).

Two ways to find the men:
  * repro probes (gen_repro.py): the probe shoots an empty "plate" frame after
    the clip; |frame - plate| gives each man's silhouette, so the man, head
    and torso boxes are found automatically.
  * real clips: pass --boxes "x,y,w,h;x,y,w,h" (whole-man boxes in the first
    analysed frame, pixels) and --track; each man box is followed frame to
    frame by phase correlation of the whole man (camera motion), and the head
    is measured RELATIVE to its body.

Per head and per consecutive frame pair it reports:
  shift_px      sub-pixel translation of the head crop (phase correlation),
                minus the body's translation in --track mode. Silhouette and
                texture moving together: a head turning/bobbing.
  sil_px        (plate mode) head silhouette centroid shift, px
  sil_chg       (plate mode) 1 - IoU of the head silhouette, frame to frame
  shade         mean |luma change| (0-255) over head pixels in both frames
                (plate mode: the silhouette intersection; track mode: the head
                box after following the body). Shading/texture change.
  torso_*       the same for the torso box (a control: does the whole man
                move, or only the head?)
  bg            mean |luma change| of a man-free background patch (global
                flicker / exposure control)
Wobble verdict per head: mean and max of shift_px and sil_px; the number of
frame pairs whose head shift exceeds 0.5 px ("twitches").

Usage:
  python metric.py <dir with *__fNNNN.png (searched recursively)> [--out DIR]
  python metric.py <clip.mkv> --boxes "x,y,w,h;..." --track [--start 0 --count 120] [--out DIR]
Outputs metric.json, a table on stdout, and crops_<side>_<i>.png (the head
crops of 60 consecutive frames, 3x, left to right, top to bottom) plus
sil_<i>.png (silhouette outline overlay strip) in --out.
"""
import argparse
import glob
import json
import os
import re
import subprocess
import sys

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage

W, H = 1920, 1080


def luma(rgb):
    return rgb[..., 0] * 0.299 + rgb[..., 1] * 0.587 + rgb[..., 2] * 0.114


GAMMA = 1.0


def load_png(p):
    a = np.asarray(Image.open(p).convert("RGB"), dtype=np.float32)
    if GAMMA != 1.0:
        # the vanilla collector's offline gamma pass (pow(c, 1/1.2)), so raw
        # vanilla PNGs compare with the collected vanilla mkvs
        a = 255.0 * np.power(a / 255.0, 1.0 / GAMMA)
    return a


def frames_from_dir(d):
    files = glob.glob(os.path.join(d, "**", "*__f[0-9][0-9][0-9][0-9].png"), recursive=True)
    def key(p):
        m = re.search(r"__f(\d{4})\.png$", p)
        return int(m.group(1))
    files = sorted(set(files), key=key)
    # tri may keep retries; keep the newest file per frame number
    by = {}
    for f in files:
        by.setdefault(key(f), []).append(f)
    files = [max(v, key=os.path.getmtime) for k, v in sorted(by.items())]
    plate = glob.glob(os.path.join(d, "**", "*__plate.png"), recursive=True)
    plate = max(plate, key=os.path.getmtime) if plate else None
    return files, plate


def frames_from_mkv(path, start, count):
    cmd = ["ffmpeg", "-v", "error", "-i", path, "-vf", f"select=gte(n\\,{start})", "-vsync", "0",
           "-frames:v", str(count), "-f", "rawvideo", "-pix_fmt", "rgb24", "-"]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    n = W * H * 3
    while True:
        buf = p.stdout.read(n)
        if len(buf) < n:
            break
        yield np.frombuffer(buf, dtype=np.uint8).reshape(H, W, 3).astype(np.float32)
    p.wait()


def phase_shift(a, b):
    """Sub-pixel (dy, dx) that moves a onto b."""
    if a.shape[0] < 4 or a.shape[1] < 4:
        return 0.0, 0.0
    wy = np.hanning(a.shape[0])[:, None]
    wx = np.hanning(a.shape[1])[None, :]
    w = wy * wx
    A = np.fft.fft2((a - a.mean()) * w)
    B = np.fft.fft2((b - b.mean()) * w)
    R = B * np.conj(A)
    R /= np.abs(R) + 1e-9
    c = np.real(np.fft.ifft2(R))
    py, px = np.unravel_index(np.argmax(c), c.shape)
    def sub(cm, c0, cp):
        d = cm - 2 * c0 + cp
        return 0.0 if abs(d) < 1e-12 else 0.5 * (cm - cp) / d
    hy, hx = c.shape
    dy = py + sub(c[(py - 1) % hy, px], c[py, px], c[(py + 1) % hy, px])
    dx = px + sub(c[py, (px - 1) % hx], c[py, px], c[py, (px + 1) % hx])
    if dy > hy / 2:
        dy -= hy
    if dx > hx / 2:
        dx -= hx
    return float(dy), float(dx)


def crop(img, box, off=(0.0, 0.0)):
    x, y, w, h = box
    x = int(round(x + off[1]))
    y = int(round(y + off[0]))
    x = max(0, min(W - w, x))
    y = max(0, min(H - h, y))
    return img[y:y + h, x:x + w]


def auto_boxes(plate, probe_frames, thr=14.0):
    """Man boxes from |frame - plate| over a few frames (union)."""
    m = np.zeros((H, W), bool)
    for f in probe_frames:
        d = np.abs(f - plate).max(axis=2)
        m |= d > thr
    m = ndimage.binary_opening(m, iterations=1)
    m = ndimage.binary_closing(m, iterations=2)
    lab, n = ndimage.label(m)
    men = []
    for i, sl in enumerate(ndimage.find_objects(lab), 1):
        ys, xs = sl
        h = ys.stop - ys.start
        w = xs.stop - xs.start
        area = int((lab[sl] == i).sum())
        if area < 120 or h < 25 or h < 1.2 * w * 0.6:
            continue
        men.append((xs.start, ys.start, w, h, area))
    men.sort(key=lambda b: b[0])
    return men, m


def head_torso(man):
    x, y, w, h = man[:4]
    hh = max(8, int(round(0.17 * h)))
    # the head's x range: columns with mask pixels near the top are found later;
    # a box centred on the man, as wide as the head is tall plus a margin
    hw = max(10, int(round(hh * 1.5)))
    return hh, hw


def refine_head(mask, man):
    """Head box around the top of the man's silhouette."""
    x, y, w, h = man[:4]
    hh, hw = head_torso(man)
    top = mask[y:y + hh, x:x + w]
    cols = np.where(top.any(axis=0))[0]
    cx = x + (cols.mean() if len(cols) else w / 2)
    pad = max(3, hh // 4)
    head = (int(cx - hw / 2), max(0, y - pad), hw, hh + 2 * pad)
    ty = y + int(0.22 * h)
    torso = (x, ty, w, max(8, int(0.30 * h)))
    return head, torso


def bg_box(mask):
    """A 64x64 man-free patch in the lower-middle of the frame."""
    for (x, y) in [(900, 900), (300, 900), (1500, 900), (900, 700), (200, 600), (1600, 600)]:
        if not mask[y:y + 64, x:x + 64].any():
            return (x, y, 64, 64)
    return (0, H - 64, 64, 64)


def tile_strip(crops, scale=3, cols=10):
    if not crops:
        return None
    h, w = crops[0].shape[:2]
    rows = (len(crops) + cols - 1) // cols
    out = Image.new("RGB", (cols * w * scale + (cols + 1) * 2, rows * h * scale + (rows + 1) * 2), (40, 40, 40))
    for i, c in enumerate(crops):
        im = Image.fromarray(np.clip(c, 0, 255).astype(np.uint8)).resize((w * scale, h * scale), Image.NEAREST)
        r, k = divmod(i, cols)
        out.paste(im, (2 + k * (w * scale + 2), 2 + r * (h * scale + 2)))
    return out


def skin_mask(c):
    r, g, b = c[..., 0], c[..., 1], c[..., 2]
    mx = np.maximum(np.maximum(r, g), b)
    mn = np.minimum(np.minimum(r, g), b)
    # face skin under daylight, dusk and fire light: red leads, clearly saturated
    return (r > 60) & (r > g + 8) & (g > b) & ((mx - mn) > 18)


def yaw_proxy(f, fm, box, off):
    """Head yaw proxy (px): horizontal centroid of face-skin pixels minus the
    horizontal centroid of the head (plate silhouette, or the box centre in
    track mode). Turning the head left/right slides the face across the head."""
    c = crop(f, box, off)
    sk = skin_mask(c)
    if fm is not None:
        hm = crop(fm, box)
        sk &= hm
        hx = np.nonzero(hm)[1].mean() if hm.any() else c.shape[1] / 2
    else:
        hx = c.shape[1] / 2
    if sk.sum() < 4:
        return None
    return float(np.nonzero(sk)[1].mean() - hx)


def yaw_flips(y, thr=0.8, n=None, window=20):
    """Left-right flips of the head yaw proxy, robust to render noise: the
    series is median-filtered over 3 frames, a TURN is a frame step of at
    least `thr` px, and a FLIP is a turn followed within `window` frames by a
    turn the other way (a look out and back). Returns (flips, turns)."""
    y = list(y[:n] if n else y)
    v = [np.nan if q is None else q for q in y]
    m = []
    for k in range(len(v)):
        w = [q for q in v[max(0, k - 1):k + 2] if not np.isnan(q)]
        m.append(float(np.median(w)) if w else np.nan)
    turns = []
    for k in range(1, len(m)):
        if np.isnan(m[k]) or np.isnan(m[k - 1]):
            continue
        d = m[k] - m[k - 1]
        if abs(d) >= thr:
            turns.append((k, 1 if d > 0 else -1))
    flips = sum(1 for (k0, s0), (k1, s1) in zip(turns[:-1], turns[1:]) if s0 != s1 and k1 - k0 <= window)
    return int(flips), len(turns)


def stats(v):
    v = np.asarray(v, float)
    if v.size == 0:
        return {"mean": None, "p95": None, "max": None}
    return {"mean": round(float(v.mean()), 3), "p95": round(float(np.percentile(v, 95)), 3), "max": round(float(v.max()), 3)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("--out", default=None)
    ap.add_argument("--boxes", default=None, help='man boxes "x,y,w,h;..." (first analysed frame)')
    ap.add_argument("--track", action="store_true", help="follow each man box by its body motion")
    ap.add_argument("--start", type=int, default=0)
    ap.add_argument("--count", type=int, default=120)
    ap.add_argument("--thr", type=float, default=14.0)
    ap.add_argument("--label", default=None)
    ap.add_argument("--gamma", type=float, default=1.0, help="apply pow(c, 1/g) to PNG frames (vanilla raw shots: 1.2)")
    a = ap.parse_args()
    global GAMMA
    GAMMA = a.gamma
    out = a.out or (a.src if os.path.isdir(a.src) else os.path.dirname(a.src))
    os.makedirs(out, exist_ok=True)
    label = a.label or os.path.basename(os.path.normpath(a.src))

    plate = None
    if os.path.isdir(a.src):
        files, plate_p = frames_from_dir(a.src)
        files = files[a.start:a.start + a.count]
        if not files:
            sys.exit("no *__fNNNN.png frames under " + a.src)
        plate = load_png(plate_p) if (plate_p and not a.boxes) else None
        frame_iter = (load_png(f) for f in files)
        nfr = len(files)
        first = load_png(files[0])
        probe = [first, load_png(files[nfr // 2]), load_png(files[-1])]
    else:
        frs = list(frames_from_mkv(a.src, a.start, a.count))
        nfr = len(frs)
        frame_iter = iter(frs)
        first = frs[0]
        probe = [first, frs[nfr // 2], frs[-1]]

    if plate is not None:
        men, mask = auto_boxes(plate, probe, a.thr)
        mode = "plate"
    else:
        if not a.boxes:
            sys.exit("no plate frame: pass --boxes")
        men = [tuple(int(v) for v in b.split(",")) + (0,) for b in a.boxes.split(";") if b.strip()]
        mask = np.zeros((H, W), bool)
        for (x, y, w, h, _) in men:
            mask[y:y + h, x:x + w] = True
        mode = "track" if a.track else "boxes"
    heads, torsos = [], []
    for m in men:
        hd, ts = refine_head(mask, m)
        heads.append(hd)
        torsos.append(ts)
    bgb = bg_box(mask)
    plate_l = luma(plate) if plate is not None else None

    nm = len(men)
    off = [np.zeros(2) for _ in range(nm)]
    rec = [{"shift": [], "sil": [], "silchg": [], "shade": [], "t_shift": [], "t_shade": [], "series": []} for _ in range(nm)]
    bg = []
    crops = [[] for _ in range(nm)]
    sils = [[] for _ in range(nm)]
    yaw = [[] for _ in range(nm)]
    prev = None
    prev_m = None
    for k, f in enumerate(frame_iter):
        L = luma(f)
        fm = (np.abs(f - plate).max(axis=2) > a.thr) if plate is not None else None
        for i in range(nm):
            yaw[i].append(yaw_proxy(f, fm, heads[i], off[i]))
        for i in range(nm):
            if len(crops[i]) < 60:
                crops[i].append(crop(f, heads[i], off[i]).copy())
                if fm is not None:
                    c = crop(f, heads[i]).copy()
                    edge = crop(fm, heads[i]) ^ ndimage.binary_erosion(crop(fm, heads[i]))
                    c[edge] = [255, 0, 255]
                    sils[i].append(c)
        if prev is not None:
            bg.append(float(np.abs(crop(L, bgb) - crop(prev, bgb)).mean()))
            for i in range(nm):
                o0 = off[i].copy()
                body_dy, body_dx = 0.0, 0.0
                if mode == "track":
                    body_dy, body_dx = phase_shift(crop(prev, men[i][:4], o0), crop(L, men[i][:4], o0))
                    if abs(body_dy) > 40 or abs(body_dx) > 40:
                        body_dy, body_dx = 0.0, 0.0
                    off[i] = o0 + np.array([body_dy, body_dx])
                if fm is not None:
                    # plate mode: correlate the foreground only (frame - plate on the
                    # silhouette), so the static background cannot pin the peak at 0
                    def fg(Lx, mx, b):
                        return np.where(crop(mx, b), crop(Lx, b) - crop(plate_l, b) + 128.0, 0.0)
                    dy, dx = phase_shift(fg(prev, prev_m, heads[i]), fg(L, fm, heads[i]))
                    tdy, tdx = phase_shift(fg(prev, prev_m, torsos[i]), fg(L, fm, torsos[i]))
                else:
                    dy, dx = phase_shift(crop(prev, heads[i], o0), crop(L, heads[i], o0))
                    tdy, tdx = phase_shift(crop(prev, torsos[i], o0), crop(L, torsos[i], o0))
                rdy, rdx = dy - body_dy, dx - body_dx
                if mode == "track":
                    tdy, tdx = tdy - body_dy, tdx - body_dx
                sh = float(np.hypot(rdy, rdx))
                rec[i]["shift"].append(sh)
                rec[i]["t_shift"].append(float(np.hypot(tdy, tdx)))
                a0 = crop(prev, heads[i], o0)
                a1 = crop(L, heads[i], off[i])
                t0 = crop(prev, torsos[i], o0)
                t1 = crop(L, torsos[i], off[i])
                if fm is not None:
                    m0 = crop(prev_m, heads[i])
                    m1 = crop(fm, heads[i])
                    both = m0 & m1
                    union = m0 | m1
                    rec[i]["silchg"].append(float(1 - both.sum() / max(1, union.sum())))
                    c0 = np.array(ndimage.center_of_mass(m0)) if m0.any() else np.zeros(2)
                    c1 = np.array(ndimage.center_of_mass(m1)) if m1.any() else np.zeros(2)
                    rec[i]["sil"].append(float(np.hypot(*(c1 - c0))))
                    rec[i]["shade"].append(float(np.abs(a1 - a0)[both].mean()) if both.any() else 0.0)
                    tm = crop(prev_m, torsos[i]) & crop(fm, torsos[i])
                    rec[i]["t_shade"].append(float(np.abs(t1 - t0)[tm].mean()) if tm.any() else 0.0)
                else:
                    rec[i]["shade"].append(float(np.abs(a1 - a0).mean()))
                    rec[i]["t_shade"].append(float(np.abs(t1 - t0).mean()))
                rec[i]["series"].append([k, round(rdx, 2), round(rdy, 2), round(rec[i]["shade"][-1], 2)])
        prev = L
        prev_m = fm

    res = {"src": a.src, "label": label, "mode": mode, "frames": nfr, "bg_box": bgb, "bg_mean_abs": stats(bg), "heads": []}
    lines = [f"{label}: mode={mode} frames={nfr} men={nm} bg|dL| mean={stats(bg)['mean']}"]
    lines.append("  i  man box             head box           shift px mean/p95/max   twitch>0.5  sil px mean/max  silchg mean  shade mean/max   torso shift mean  torso shade  JUMPS(shade>5)  shade p50")
    for i in range(nm):
        r = rec[i]
        s = stats(r["shift"])
        twitch = int(sum(1 for v in r["shift"] if v > 0.5))
        jumps = int(sum(1 for v in r["shade"] if v > 5.0))
        shade_p50 = round(float(np.median(r["shade"])), 3) if r["shade"] else None
        sil = stats(r["sil"])
        sc = stats(r["silchg"])
        sd = stats(r["shade"])
        ts = stats(r["t_shift"])
        tsd = stats(r["t_shade"])
        yv = [v for v in yaw[i] if v is not None]
        fl60, mv60 = yaw_flips(yaw[i], n=60)
        flall, mvall = yaw_flips(yaw[i])
        ysteps = [abs(b - a0) for a0, b in zip(yaw[i][:-1], yaw[i][1:]) if a0 is not None and b is not None]
        ystat = stats(ysteps)
        res.setdefault("yaw", []).append({"i": i, "flips_first60": fl60, "turns_first60": mv60, "flips_all": flall,
                                          "turns_all": mvall, "yaw_step_px": ystat,
                                          "yaw_range_px": round(max(yv) - min(yv), 2) if yv else None,
                                          "skin_frames": len(yv), "series": [None if v is None else round(v, 2) for v in yaw[i]]})
        res["heads"].append({"i": i, "man_box": list(men[i][:4]), "head_box": list(heads[i]), "torso_box": list(torsos[i]),
                             "shift_px": s, "twitches_over_0p5px": twitch, "sil_px": sil, "sil_change": sc,
                             "shade": sd, "shade_p50": shade_p50, "jumps_shade_over_5": jumps, "torso_shift_px": ts, "torso_shade": tsd, "series_k_dx_dy_shade": r["series"]})
        lines.append(f"  {i}  {str(list(men[i][:4])):19s} {str(list(heads[i])):18s} {s['mean']}/{s['p95']}/{s['max']}".ljust(68)
                     + f"{twitch:4d}        {sil['mean']}/{sil['max']}".ljust(30) + f"{sc['mean']}".ljust(13)
                     + f"{sd['mean']}/{sd['max']}".ljust(17) + f"{ts['mean']}".ljust(18) + f"{tsd['mean']}".ljust(13)
                     + f"{jumps:4d}       {shade_p50}")
        st = tile_strip(crops[i])
        if st is not None:
            st.save(os.path.join(out, f"crops_{label}_{i}.png"))
        if sils[i]:
            tile_strip(sils[i]).save(os.path.join(out, f"sil_{label}_{i}.png"))
    # an overview with the boxes drawn on the first frame
    lines.append("  YAW (face-skin centroid minus head centroid, px; median-3, turn >= 0.8 px, flip = turn back within 20 f): i  FLIPS first60 (turns)  FLIPS all (turns)  step mean/p95/max  range  skin frames")
    for y in res.get("yaw", []):
        st = y["yaw_step_px"]
        lines.append(f"     {y['i']}  {y['flips_first60']:3d} ({y['turns_first60']:3d})          {y['flips_all']:3d} ({y['turns_all']:3d})   "
                     f"{st['mean']}/{st['p95']}/{st['max']}".ljust(30) + f"  {y['yaw_range_px']}  {y['skin_frames']}")
    ov = Image.fromarray(np.clip(first, 0, 255).astype(np.uint8))
    dr = ImageDraw.Draw(ov)
    for i in range(nm):
        x, y, w, h = men[i][:4]
        dr.rectangle([x, y, x + w, y + h], outline=(0, 255, 0))
        x, y, w, h = heads[i]
        dr.rectangle([x, y, x + w, y + h], outline=(255, 0, 255))
        dr.text((x, max(0, y - 12)), str(i), fill=(255, 255, 0))
    x, y, w, h = bgb
    dr.rectangle([x, y, x + w, y + h], outline=(0, 160, 255))
    ov.save(os.path.join(out, f"boxes_{label}.png"))
    with open(os.path.join(out, f"metric_{label}.json"), "w") as fh:
        json.dump(res, fh, indent=1)
    print("\n".join(lines))


if __name__ == "__main__":
    main()
