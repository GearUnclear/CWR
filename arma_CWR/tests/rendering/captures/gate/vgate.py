"""vgate: the still-image comparator of the visual gate (PERF-PLAN 3.3, P0-8, P0-9).

Run it only through the heavy-work door (it is image work):
    powershell -File <squad>\\bin\\squad-heavy.ps1 -Kind analysis -Id <item> -- python -I vgate.py ...

Subcommands
  compare A B   compare the shots of two run folders (tri trees, perf-bench boots, run-vgate sets)
      --claim exact|pit|invisible|look   what the item claims (default: report the class only)
      --noise <noise.json>    the per-view boot-to-boot floor of the golden build (`vgate.py noise`)
      --label-a X --label-b Y shots <view>__X in A against <view>__Y in B (default: same names)
      --filter S              only shot keys containing S (e.g. _reset)
      --out DIR               report.md, report.json, per-shot diff and cluster crops, blind sheets
      --allow-no-manifest     bootstrap only (shots from run-captures.ps1 have no manifest.json)
      --expect-differ         the job says the builds differ: refuse equal build ids or equal files
      --declare-objects       the item declares an objects= change (else >5% refuses)
  noise B1 B2 B3 ...  build a noise file from >= 2 boots of ONE build (all pairs)
      --build-id ID --out FILE [--filter S]
  pair a.png b.png    one pair, printed (no refusals; for quick looks)

Classes, best first (3.3): IDENTICAL (0 differing pixels) > PI-T (pixels over 8 and over 2
within the view's floor, any small shift spread over the frame, every cluster over 8 no
bigger than the floor's) > WITHIN-NOISE (every band's max, px>2, px>8 within the floor) >
NEAR (SSIM >= 0.995 full size, CIELAB dE mean <= 0.5 and p99 <= 3 at 320 px) > CHANGED.
Claims: exact passes only IDENTICAL; pit passes IDENTICAL or PI-T, and says EXPLAIN when the
counts are inside the floor but a cluster or a concentrated shift needs a crop explained;
invisible needs WITHIN-NOISE or better before the judges look; look only reports.
Rain views (rain or wet in the shot's feat=, or a rain view name) are never exact-gated:
their claim result is PERCEPTUAL plus the rain-rule tripwire (3.3 rain views).
Refusals (3.3): an UNSETTLED view, lit not OK, mode not remastered, objects= off by more
than 5% (undeclared), a missing manifest, equal build ids or equal files where the job says
they differ, and a dry shot taken after the session drew rain (P0-17).
Tripwires (P0-9; thresholds ESTIMATED, calibrated in P0-11): only darker, blue wash, crushed
shade, flattened crowns, evening warmth, rain rule, mustard ground, sharpness loss. A
tripwire forces a judge to look; it never passes anything.
Without --noise the PI-T and WITHIN-NOISE limits fall back to M2's dry floor (M2 5: fail
above 30 px over 8 or 2,000 px over 2) and the report says so.
"""
import argparse
import json
import math
import os
import random
import re
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402
import golden_views as gv  # noqa: E402
import numpy as np  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402
from scipy import ndimage  # noqa: E402

BANDS = ((0, 250), (250, 350), (350, 640))  # banddiff.py's sky / horizon / ground rows of a 640-row frame
BAND_NAMES = ("sky", "horizon", "ground")
M2_FLOOR = {"over8": 16, "over2": 1600, "max": 25}  # M2 5, dry views, 3 boots
PIT_ABS = {"over8": 30, "over2": 2000}  # M2's suggested fail line
NEAR = {"ssim": 0.995, "de_mean": 0.5, "de_p99": 3.0}
TILE = 60  # global-shift grid: 32 x 18 tiles at 1080p
SKIP_RE = re.compile(r"(__pair|contact|sheet|diff|blink|_thumb)", re.I)


# ----------------------------------------------------------------------------- inputs

def find_shots(root, flt=""):
    """key -> png path for every capture under root (tri's NNN_<label>.png or <view>__<label>.png)."""
    out = {}
    for dp, _, fs in os.walk(root):
        if os.sep + "user" in dp or os.sep + "cache" in dp:
            continue
        for f in fs:
            if not f.lower().endswith(".png") or SKIP_RE.search(f):
                continue
            k = gl.shot_key(f)
            if flt and flt not in k:
                continue
            p = os.path.join(dp, f)
            if k in out and os.path.getmtime(out[k]) >= os.path.getmtime(p):
                continue
            out[k] = p
    return out


def view_of(key):
    v = key.split("__")[0]
    return re.sub(r"_(no)?reset$", "", v)


def shot_meta(root):
    """label -> {lit, mode, objects, settle, feat, after_rain, log} from the game logs under root."""
    meta = {}
    for dp, _, fs in os.walk(root):
        for f in fs:
            if f not in ("game_stdout.log",):
                continue
            lp = os.path.join(dp, f)
            settle, rain_seen, rain_view_seen = {}, False, False
            for _, d in gl.udcap_lines(lp):
                ev = d.get("event")
                view = d.get("view", "")
                if ev == "begin" and gv.is_rain(re.sub(r"_r\d+$", "", view)):
                    rain_view_seen = True
                if ev == "mode":
                    settle[view] = d.get("settle", "")
                if ev == "shot":
                    fv = gl.feat_values(d.get("feat", ""))
                    is_wet = fv.get("rain", 0) > 0 or fv.get("wet", 0) > 0
                    lab = d.get("label", "")
                    meta[lab] = dict(lit=d.get("lit", ""), mode=d.get("mode", ""), objects=_int(d.get("objects")),
                                     draws=_int(d.get("draws")), settle=settle.get(view, ""), feat=d.get("feat", ""),
                                     rain=fv.get("rain", 0.0), wet=fv.get("wet", 0.0), view=view, log=lp,
                                     after_rain=(rain_seen or rain_view_seen) and not is_wet and not gv.is_rain(view_of(lab)),
                                     build=d.get("build", ""))
                    if is_wet:
                        rain_seen = True
    return meta


def _int(x):
    try:
        return int(float(x))
    except (TypeError, ValueError):
        return None


def find_manifest(path):
    """manifest.json in the shot's folder or up to four parents (perf-bench boot, run-vgate set)."""
    p = os.path.dirname(os.path.abspath(path))
    for _ in range(5):
        m = os.path.join(p, "manifest.json")
        if os.path.exists(m):
            try:
                with open(m, encoding="utf-8-sig") as f:
                    return m, json.load(f)
            except (OSError, ValueError):
                return m, {}
        p = os.path.dirname(p)
    return None, None


def build_id(man):
    if not man:
        return ""
    for k in ("build_id",):
        if man.get(k):
            return man[k]
    exe = man.get("exe") or {}
    return exe.get("build_id") or (("exe-" + exe["sha256"][:12]) if exe.get("sha256") else "")


# ----------------------------------------------------------------------------- metrics

def diff_core(a, b):
    """The exact numbers: per pixel max channel |a-b| and the counts banddiff and imgdiff use."""
    if a.shape != b.shape:
        raise ValueError(f"sizes differ: {a.shape} vs {b.shape}")
    ai, bi = a.astype(np.int16), b.astype(np.int16)
    d = np.abs(ai - bi).max(axis=2)
    h = d.shape[0]
    bands = []
    for (lo, hi), n in zip(BANDS, BAND_NAMES):
        s = d[int(round(lo * h / 640)):int(round(hi * h / 640))]
        bands.append(dict(band=n, max=int(s.max()), over2=int((s > 2).sum()), over8=int((s > 8).sum()), pixels=int(s.size)))
    signed = (bi - ai).reshape(-1, 3).mean(axis=0)
    # The same counts after removing one global per-channel offset (the median signed difference):
    # a timing-dependent exposure shift of 1-2 levels (M2 5) mostly vanishes here, a local change
    # does not. PI-T judges px>2 on this when the shift is spread over the frame.
    off = np.median((bi - ai).reshape(-1, 3), axis=0).astype(np.int16)
    dc = np.abs(bi - ai - off).max(axis=2)
    return d, dict(max=int(d.max()), over0=int((d > 0).sum()), over2=int((d > 2).sum()), over8=int((d > 8).sum()),
                   over2_comp=int((dc > 2).sum()), offset=[int(x) for x in off],
                   mean_abs=float(np.abs(ai - bi).mean()), pixels=int(d.size), bands=bands,
                   signed_mean=[round(float(x), 3) for x in signed])


def shift_spread(d):
    """Is a small difference spread over the frame (a timing shift) or concentrated (a change)?
    Tiles of 60 px; spread = share of tiles with > 5% differing pixels; top = share of all
    differing pixels in the densest 5% of tiles."""
    h, w = d.shape
    gy, gx = h // TILE, w // TILE
    m = (d[:gy * TILE, :gx * TILE] > 0).reshape(gy, TILE, gx, TILE).mean(axis=(1, 3))
    n1 = float((d > 0).mean())
    flat = np.sort(m.ravel())[::-1]
    k = max(1, int(round(0.05 * flat.size)))
    tot = flat.sum()
    top = float(flat[:k].sum() / tot) if tot > 0 else 0.0
    spread = float((m > 0.05).mean())
    shifted = n1 > 0.005
    ok = (not shifted) or (spread >= 0.25 and top < 0.5)
    return dict(frac_changed=round(n1, 5), tiles_over5pct=round(spread, 3), top5pct_share=round(top, 3),
                shifted=shifted, spread_ok=ok)


def clusters(d, thr=8, grow=2, limit=50):
    mask = d > thr
    if not mask.any():
        return []
    lab, n = ndimage.label(ndimage.binary_dilation(mask, iterations=grow), structure=np.ones((3, 3)))
    out = []
    objs = ndimage.find_objects(lab)
    sizes = ndimage.sum(mask, lab, index=np.arange(1, n + 1))
    order = np.argsort(sizes)[::-1][:limit]
    for i in order:
        sl = objs[i]
        out.append(dict(px=int(sizes[i]), y0=sl[0].start, y1=sl[0].stop, x0=sl[1].start, x1=sl[1].stop,
                        max=int(d[sl][lab[sl] == i + 1].max())))
    return out


def perceptual(a, b):
    s = gl.ssim(a, b)
    dm, dp = gl.delta_e(a, b, 320)
    return dict(ssim=round(s, 5), de_mean=round(dm, 3), de_p99=round(dp, 3))


def all_metrics(a, b):
    d, core = diff_core(a, b)
    core["shift"] = shift_spread(d)
    cl = clusters(d)
    core["clusters"] = cl
    core["largest_cluster"] = cl[0]["px"] if cl else 0
    core.update(perceptual(a, b))
    return d, core


# ----------------------------------------------------------------------------- tripwires (P0-9)

def lab_stats(rgb):
    L = gl.rgb_to_lab(rgb)
    C = np.sqrt(L[..., 1] ** 2 + L[..., 2] ** 2)
    return dict(L=float(L[..., 0].mean()), Lstd=float(L[..., 0].std()), a=float(L[..., 1].mean()),
                b=float(L[..., 2].mean()), C=float(C.mean()))


def hue(a, b):
    return math.degrees(math.atan2(b, a)) % 360


def otsu_ratio(y):
    v = y.ravel()
    if v.size < 50:
        return None
    hist, edges = np.histogram(v, bins=64)
    w = hist.cumsum()
    mu = (hist * edges[:-1]).cumsum()
    tot, mt = w[-1], mu[-1]
    with np.errstate(divide="ignore", invalid="ignore"):
        sb = (mt * w - mu * tot) ** 2 / (w * (tot - w))
    t = edges[int(np.nanargmax(sb[:-1]))]
    lo, hi = v[v <= t], v[v > t]
    if lo.size < 10 or hi.size < 10 or hi.mean() <= 0:
        return None
    return float(lo.mean() / hi.mean())


def lap_energy(y):
    """Laplacian energy over the box's own variance: a blur lowers it, a pure darkening or
    brightening (which scales both) does not."""
    y = y.astype(np.float32)
    return float(ndimage.laplace(y).var() / (y.var() + 1e-6))


def streak_energy(y):
    """Vertical streaks: horizontal gradient of the image smoothed along y (sigma 6 px)."""
    s = ndimage.gaussian_filter1d(y.astype(np.float32), 6, axis=0)
    return float(np.abs(np.diff(s, axis=1)).mean())


def tripwires(view, a, b, outdir=None, key=None):
    res = []
    boxes = gv.boxes(view)

    def crop(box):
        sy, sx = gl.box_px(box, a.shape)
        return a[sy, sx], b[sy, sx]

    def add(name, va, vb, fired, rule, box=None):
        r = dict(name=name, a=None if va is None else round(va, 4), b=None if vb is None else round(vb, 4),
                 fired=bool(fired), rule=rule)
        if box is not None:
            r["box"] = box
            if fired and outdir:
                ca, cb = crop(box)
                p = os.path.join(outdir, f"{key}__tw_{name}.png")
                side_by_side([ca, cb], ["A", "B"], p, zoom=2)
                r["crop"] = p
        res.append(r)

    fa, fb = lab_stats(a), lab_stats(b)
    darker = (fb["L"] - fa["L"] < -2) and not (fb["C"] > fa["C"] or fb["Lstd"] > fa["Lstd"])
    add("only_darker", fa["L"], fb["L"], darker, "frame mean L* down > 2 with no chroma or contrast rise")
    gb = boxes.get("ground", [])
    gbw = False
    gba = gbb = None
    if gb:
        ga, gbx = crop(gb[0])
        gba, gbb = lab_stats(ga), lab_stats(gbx)
        gbw = gbb["b"] - gba["b"] < -1.0
    add("blue_wash", fa["b"], fb["b"], (fb["b"] - fa["b"] < -1.5) or gbw,
        "frame mean b* down > 1.5, or the ground box's b* down > 1.0", gb[0] if gb else None)
    if gba:
        ha, hb = hue(gba["a"], gba["b"]), hue(gbb["a"], gbb["b"])
        # Yellow sits near h_ab 95 deg (mustard about 85-95, sRGB yellow 102). A near-neutral ground
        # (noon fields: C* about 7, h_ab about 97) yellows by gaining b* with little hue movement,
        # so the tripwire also fires on b* up more than 3 inside the yellow sector (estimated).
        toward = abs(ha - 95) - abs(hb - 95)
        yellowing = (gbb["b"] - gba["b"] > 3.0) and 70 <= hb <= 110
        add("mustard_ground", ha, hb, toward > 4 or yellowing,
            f"ground hue more than 4 deg toward yellow, or b* up > 3 in the yellow sector (b* {gba['b']:.1f} -> {gbb['b']:.1f})", gb[0])
    else:
        res.append(dict(name="mustard_ground", fired=False, rule="no ground box"))
    hulls = boxes.get("hull", [])
    if hulls:
        ra = otsu_ratio(gl.luma(crop(hulls[0])[0]))
        rb = otsu_ratio(gl.luma(crop(hulls[0])[1]))
        fired = ra is not None and rb is not None and (rb < ra - 0.03)
        add("crushed_shade", ra, rb, fired, "hull shade/lit (Otsu split in the hull box) drops > 0.03 "
            "(the absolute 0.58-0.62 band needs CAL-1's measured boxes)", hulls[0])
    else:
        res.append(dict(name="crushed_shade", fired=False, rule="no hull box"))
    crowns = boxes.get("crown", [])
    if crowns:
        sa = np.mean([lab_stats(crop(c)[0])["Lstd"] for c in crowns])
        sb = np.mean([lab_stats(crop(c)[1])["Lstd"] for c in crowns])
        add("flattened_crowns", float(sa), float(sb), sb < 0.9 * sa, "crown L* stddev down > 10%", crowns[0])
    else:
        res.append(dict(name="flattened_crowns", fired=False, rule="no crown box"))
    if gv.is_golden_hour(view):
        fired = False
        vals = []
        for kind in ("ground", "sky"):
            for bx in boxes.get(kind, [])[:1]:
                ca, cb = crop(bx)
                ta, tb = gl.cct_mccamy(ca), gl.cct_mccamy(cb)
                vals.append((kind, ta, tb))
                if tb - ta > 150:
                    fired = True
        add("evening_warmth", vals[0][1] if vals else None, vals[0][2] if vals else None, fired,
            "lit ground or sky CCT up > 150 K at a golden-hour view (" + ", ".join(f"{k} {x:.0f}->{y:.0f} K" for k, x, y in vals) + ")",
            (boxes.get("sky") or boxes.get("ground") or [None])[0])
    tex = boxes.get("texture", [])
    if tex:
        ea = np.mean([lap_energy(gl.luma(crop(t)[0])) for t in tex])
        eb = np.mean([lap_energy(gl.luma(crop(t)[1])) for t in tex])
        add("sharpness_loss", float(ea), float(eb), eb < 0.9 * ea, "Laplacian energy in texture boxes down > 10%", tex[0])
    wets = boxes.get("wet", [])
    if wets and gv.is_rain(view):
        ca, cb = crop(wets[0])
        ya, yb = gl.luma(ca), gl.luma(cb)
        thr = np.percentile(ya, 99.5)
        na, nb = float((ya > thr).sum()), float((yb > thr).sum())
        sa, sb = streak_energy(ya), streak_energy(yb)
        add("rain_rule", na, nb, nb > 1.2 * max(na, 1.0) or sb > 1.15 * sa,
            f"wet-ground pixels above A's 99.5th percentile up > 20%, or vertical streak energy up > 15% "
            f"(streak {sa:.3f} -> {sb:.3f}; no puddle mask yet, the box is all wet ground)", wets[0])
    return res


# ----------------------------------------------------------------------------- pictures

def heat(d, gain=8):
    v = np.clip(d.astype(np.float32) * gain, 0, 255).astype(np.uint8)
    rgb = np.stack([v, (v * 0.5).astype(np.uint8), 255 - v], axis=-1)
    rgb[d == 0] = (0, 0, 0)
    return rgb


def side_by_side(imgs, labels, path, zoom=1, gap=6):
    ims = [Image.fromarray(i) for i in imgs]
    if zoom != 1:
        ims = [i.resize((i.width * zoom, i.height * zoom), Image.NEAREST) for i in ims]
    h = max(i.height for i in ims) + 18
    w = sum(i.width for i in ims) + gap * (len(ims) - 1)
    out = Image.new("RGB", (w, h), (24, 24, 26))
    dr = ImageDraw.Draw(out)
    x = 0
    f = gl.font(12)
    for im, lab in zip(ims, labels):
        out.paste(im, (x, 18))
        dr.text((x + 2, 2), lab, fill=(230, 210, 120), font=f)
        x += im.width + gap
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    out.save(path)


def cluster_crops(key, a, b, d, cl, outdir, n=12, pad=24):
    paths = []
    for i, c in enumerate(cl[:n]):
        y0, y1 = max(0, c["y0"] - pad), min(a.shape[0], c["y1"] + pad)
        x0, x1 = max(0, c["x0"] - pad), min(a.shape[1], c["x1"] + pad)
        z = max(1, min(4, 240 // max(1, max(y1 - y0, x1 - x0))))
        p = os.path.join(outdir, key, f"cluster{i:02d}_{c['px']}px.png")
        side_by_side([a[y0:y1, x0:x1], b[y0:y1, x0:x1], heat(d[y0:y1, x0:x1])], ["A", "B", "|d| x8"], p, zoom=z)
        paths.append(p)
    return paths


def blink(a, b, path, width=960):
    fa = Image.fromarray(gl.resize(a, width))
    fb = Image.fromarray(gl.resize(b, width))
    fa.save(path, save_all=True, append_images=[fb], duration=600, loop=0)


def blind_sheet(pairs, outdir, width=320, seed=None):
    """The Realism judge's thumbnail test: every view on one page, sides unlabelled and randomly
    swapped. The key (which side is B) goes to a separate file the judge does not get."""
    rng = random.Random(seed if seed is not None else time.time_ns())
    rows, key = [], {}
    for i, (k, a, b) in enumerate(pairs):
        swap = rng.random() < 0.5
        l, r = (b, a) if swap else (a, b)
        key[f"{i + 1:02d}"] = dict(shot=k, right_is=("A" if swap else "B"))
        rows.append((f"{i + 1:02d}", gl.resize(l, width), gl.resize(r, width)))
    if not rows:
        return None
    rh = max(r[1].shape[0] for r in rows)
    pad, lab = 8, 16
    W = 2 * width + 3 * pad
    H = len(rows) * (rh + lab + pad) + pad
    sheet = Image.new("RGB", (W, H), (24, 24, 26))
    dr = ImageDraw.Draw(sheet)
    f = gl.font(12)
    for j, (n, l, r) in enumerate(rows):
        y = pad + j * (rh + lab + pad)
        dr.text((pad, y), n, fill=(230, 210, 120), font=f)
        sheet.paste(Image.fromarray(l), (pad, y + lab))
        sheet.paste(Image.fromarray(r), (2 * pad + width, y + lab))
    p = os.path.join(outdir, f"thumbs_blind_{width}.png")
    sheet.save(p)
    gl.write_json(os.path.join(outdir, "thumbs_blind_key.json.private"), key)
    return p


# ----------------------------------------------------------------------------- verdicts

def floor_for(noise, key):
    if not noise:
        return None
    ks = noise.get("keys", {})
    if key in ks:
        return ks[key]["max"]
    v = view_of(key)
    for k, e in ks.items():
        if view_of(k) == v and e.get("view_only_ok", True):
            return e["max"]
    return None


def classify(m, fl, rain):
    """The ladder of 3.3 for one shot. Returns (class, pit dict, notes)."""
    notes = []
    if fl is None:
        f8, f2, fc = M2_FLOOR["over8"], M2_FLOOR["over2"], 20
        notes.append("no noise file entry: M2's provisional dry floor (16 px over 8, 1,600 over 2)")
    else:
        f8, f2, fc = fl["over8"], fl["over2"], max(fl.get("largest_cluster", 0), 1)
    lim8, lim2 = max(2 * f8, PIT_ABS["over8"]), max(int(1.25 * f2), PIT_ABS["over2"])
    # (2) a small shift is fine when it is spread over the frame, or when no more of the frame
    # moved than moves between two boots of one build (dry views change 1-25% of their pixels by
    # a level or two from boot to boot, often in a few tiles: foliage, the horizon).
    fl_frac = (fl or {}).get("frac_changed")
    shift_ok = m["shift"]["spread_ok"] or (fl_frac is not None and m["shift"]["frac_changed"] <= 1.5 * fl_frac + 0.005)
    o2 = m["over2_comp"] if (m["shift"]["shifted"] and m["shift"]["spread_ok"]) else m["over2"]
    pit = dict(limit_over8=lim8, limit_over2=lim2, floor_cluster=fc, over2_judged=o2,
               counts_ok=m["over8"] <= lim8 and o2 <= lim2,
               shift_ok=shift_ok,
               clusters_over_floor=[c for c in m["clusters"] if c["px"] > max(fc, 4)])
    pit["pass"] = pit["counts_ok"] and pit["shift_ok"] and not pit["clusters_over_floor"]
    within = False
    if fl is not None and "bands" in fl:
        within = all(b["max"] <= fb["max"] and b["over2"] <= fb["over2"] and b["over8"] <= fb["over8"]
                     for b, fb in zip(m["bands"], fl["bands"]))
    elif fl is None:
        within = m["over8"] <= M2_FLOOR["over8"] and m["over2"] <= M2_FLOOR["over2"] and m["max"] <= M2_FLOOR["max"]
    near = m["ssim"] >= NEAR["ssim"] and m["de_mean"] <= NEAR["de_mean"] and m["de_p99"] <= NEAR["de_p99"]
    if m["over0"] == 0:
        cls = "IDENTICAL"
    elif rain:
        cls = "NEAR" if near else "CHANGED"
        notes.append("rain view: not exact-gateable until P0-6 seeds the rain path; perceptual only")
    elif pit["pass"]:
        cls = "PI-T"
    elif within:
        cls = "WITHIN-NOISE"
    elif near:
        cls = "NEAR"
    else:
        cls = "CHANGED"
    return cls, pit, notes


def claim_result(claim, cls, pit, rain, tw_fired):
    if not claim:
        return None
    if rain and claim in ("exact", "pit", "invisible"):
        return "PERCEPTUAL" + (" (tripwire fired)" if tw_fired else "")
    if claim == "exact":
        return "PASS" if cls == "IDENTICAL" else "FAIL"
    if claim == "pit":
        if cls in ("IDENTICAL", "PI-T"):
            return "PASS"
        if pit["counts_ok"]:
            return "EXPLAIN"
        return "FAIL"
    if claim == "invisible":
        ok = cls in ("IDENTICAL", "PI-T", "WITHIN-NOISE")
        return ("JUDGES" if ok else "FAIL") + (" (tripwire fired)" if tw_fired else "")
    return "JUDGES" + (" (tripwire fired)" if tw_fired else "")


# ----------------------------------------------------------------------------- commands

def cmd_compare(o):
    A, B = find_shots(o.a, o.filter), find_shots(o.b, o.filter)
    ma, mb = shot_meta(o.a), shot_meta(o.b)
    noise = json.load(open(o.noise, encoding="utf-8")) if o.noise else None
    out = o.out
    os.makedirs(out, exist_ok=True)
    pairs = []
    for ka in sorted(A):
        kb = ka
        if o.label_a and o.label_b:
            if not ka.endswith("__" + o.label_a):
                continue
            kb = ka[: -len(o.label_a)] + o.label_b
        if kb in B:
            pairs.append((ka, kb))
    rep = dict(a=o.a, b=o.b, claim=o.claim, noise=o.noise, created=time.strftime("%Y-%m-%d %H:%M:%S"), shots=[],
               refusals=[], missing=sorted(set(A) - {p[0] for p in pairs}))
    man_a, man_b = find_manifest(next(iter(A.values()))) if A else (None, None), \
        find_manifest(next(iter(B.values()))) if B else (None, None)
    bid_a, bid_b = build_id(man_a[1]), build_id(man_b[1])
    rep["build_a"], rep["build_b"] = bid_a, bid_b
    if not o.allow_no_manifest and (not man_a[0] or not man_b[0]):
        rep["refusals"].append("missing manifest.json (pass --allow-no-manifest only for bootstrap run-captures sets)")
    if o.expect_differ and bid_a and bid_a == bid_b:
        rep["refusals"].append(f"the job says the builds differ but both sides are {bid_a}")
    thumbs = []
    for ka, kb in pairs:
        view = view_of(ka)
        a, b = gl.load_rgb(A[ka]), gl.load_rgb(B[kb])
        ra, rb = ma.get(ka, {}), mb.get(kb, {})
        rain = gv.is_rain(view) or ra.get("rain", 0) > 0 or ra.get("wet", 0) > 0 or rb.get("rain", 0) > 0 or rb.get("wet", 0) > 0
        refuse = []
        for side, r in (("A", ra), ("B", rb)):
            if not r:
                if not o.allow_no_manifest:
                    refuse.append(f"{side}: no event=shot line for this label")
                continue
            if r.get("settle", "").startswith("UNSETTLED"):
                refuse.append(f"{side}: view UNSETTLED ({r['settle']})")
            if r.get("lit") not in ("OK", ""):
                refuse.append(f"{side}: lit={r.get('lit')}")
            # Pixel claims (exact, pit) are mode-agnostic: a Classic or all-off baseline row may be
            # compared byte for byte (W1-3). Judged claims (invisible, look) need remastered (3.4).
            if r.get("mode") and r["mode"] != "remastered" and not o.any_mode and o.claim in ("invisible", "look"):
                refuse.append(f"{side}: mode={r['mode']} (judged claims need remastered; --any-mode to override)")
        if ra.get("mode") and rb.get("mode") and ra["mode"] != rb["mode"]:
            refuse.append(f"modes differ: A {ra['mode']}, B {rb['mode']}")
            if r.get("after_rain"):
                refuse.append(f"{side}: dry shot after the session drew rain (P0-17)")
        oa, ob = ra.get("objects"), rb.get("objects")
        counts_equal = (oa, ra.get("draws")) == (ob, rb.get("draws")) if (ra and rb) else None
        if o.strict_counts and counts_equal is False and not rain:  # rain particles vary from boot to boot
            refuse.append(f"objects/draws differ: A {oa}/{ra.get('draws')}, B {ob}/{rb.get('draws')} (--strict-counts)")
        obj_note = []
        if oa and ob and abs(ob - oa) > 0.05 * oa and not o.declare_objects:
            # Rain and splash particles count as objects and vary from boot to boot; rain views are
            # perceptual-only anyway, so there it is a note, elsewhere a refusal (3.3).
            (obj_note if rain else refuse).append(f"objects= {oa} -> {ob} (over 5%, undeclared)")
        same_file = os.path.abspath(A[ka]) == os.path.abspath(B[kb]) or gl.sha256(A[ka]) == gl.sha256(B[kb])
        if o.expect_differ and same_file:
            refuse.append("candidate file is the golden file (same hash) while the job says the builds differ")
        d, m = all_metrics(a, b)
        fl = floor_for(noise, ka)
        cls, pit, notes = classify(m, fl, rain)
        notes += obj_note
        tw = tripwires(view, a, b, os.path.join(out, "tripwires"), ka)
        fired = [t["name"] for t in tw if t.get("fired")]
        sdir = os.path.join(out, ka)
        os.makedirs(sdir, exist_ok=True)
        Image.fromarray(heat(d)).save(os.path.join(sdir, "diff_x8.png"))
        crops = cluster_crops(ka, a, b, d, m["clusters"], out) if m["clusters"] else []
        if not o.no_blink:
            blink(a, b, os.path.join(sdir, "blink.gif"))
        res = claim_result(o.claim, cls, pit, rain, bool(fired))
        if refuse:
            res = "REFUSED"
        m_small = dict(m)
        m_small["clusters"] = m["clusters"][:12]
        rep["shots"].append(dict(key=ka, key_b=kb, view=view, a=A[ka], b=B[kb], rain=rain, cls=cls, claim_result=res,
                                 counts_equal=counts_equal,
                                 refusals=[r for r in refuse if r], notes=notes, metrics=m_small, pit=dict(pit, clusters_over_floor=pit["clusters_over_floor"][:12]),
                                 floor=fl, tripwires=tw, tripwires_fired=fired, cluster_crops=crops,
                                 meta_a={k: ra.get(k) for k in ("lit", "mode", "objects", "draws", "settle", "rain", "wet", "after_rain")},
                                 meta_b={k: rb.get(k) for k in ("lit", "mode", "objects", "draws", "settle", "rain", "wet", "after_rain")}))
        thumbs.append((ka, a, b))
        print(f"{ka}: {cls} claim={res} max={m['max']} >2={m['over2']} >8={m['over8']} ssim={m['ssim']} "
              f"dE={m['de_mean']}/{m['de_p99']} tripwires={','.join(fired) or '-'}" + (f" REFUSED: {'; '.join(r for r in refuse if r)}" if refuse else ""))
        sys.stdout.flush()
    if thumbs and not o.no_sheets:
        rep["blind_320"] = blind_sheet(thumbs, out, 320, o.seed)
    if rep["refusals"]:
        for s in rep["shots"]:
            s["claim_result"] = "REFUSED"
    gl.write_json(os.path.join(out, "report.json"), rep)
    gl.write_text(os.path.join(out, "report.md"), report_md(rep))
    worst = [s for s in rep["shots"] if s["claim_result"] in ("FAIL", "REFUSED", "EXPLAIN")]
    print(f"vgate: {len(rep['shots'])} shots, {len(worst)} not passing, refusals: {'; '.join(rep['refusals']) or 'none'}; {out}")
    return 1 if rep["refusals"] or any(s["claim_result"] in ("FAIL", "REFUSED") for s in rep["shots"]) else 0


def report_md(rep):
    L = [f"# vgate: {rep['a']} against {rep['b']}", "",
         f"claim: {rep['claim'] or '(none, classes only)'}; noise file: {rep['noise'] or 'none (M2 provisional floor)'}; "
         f"builds: A {rep['build_a'] or '?'}, B {rep['build_b'] or '?'}", ""]
    if rep["refusals"]:
        L += ["**REFUSED (whole set):** " + "; ".join(rep["refusals"]), ""]
    L += ["| shot | class | claim | max | px>2 | px>8 | largest cluster | shift (tiles, top5%) | SSIM | dE mean/p99 | objects/draws A, B | tripwires | refusals |",
          "|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for s in rep["shots"]:
        m = s["metrics"]
        L.append(f"| {s['key']} | {s['cls']} | {s['claim_result'] or ''} | {m['max']} | {m['over2']:,} | {m['over8']:,} | "
                 f"{m['largest_cluster']} | {m['shift']['tiles_over5pct']}, {m['shift']['top5pct_share']} | {m['ssim']} | "
                 f"{m['de_mean']}/{m['de_p99']} | {s['meta_a'].get('objects')}/{s['meta_a'].get('draws')}, {s['meta_b'].get('objects')}/{s['meta_b'].get('draws')}"
                 f"{'' if s.get('counts_equal') in (True, None) else ' DIFFER'} | {', '.join(s['tripwires_fired']) or '-'} | {'; '.join(s['refusals']) or '-'} |")
    L += ["", "PI-T limits per shot: max(2 x floor px>8, 30) and max(1.25 x floor px>2, 2,000); clusters over 8 "
          "bigger than the floor's largest need an explanation (crops under <shot>/cluster*.png).", ""]
    for s in rep["shots"]:
        if s["cluster_crops"] or s["notes"] or s["tripwires_fired"]:
            L.append(f"- {s['key']}: " + "; ".join(s["notes"] + [f"tripwire {t}" for t in s["tripwires_fired"]] +
                                                  [f"{len(s['cluster_crops'])} cluster crops"]))
    return "\n".join(L) + "\n"


def cmd_noise(o):
    boots = [find_shots(b, o.filter) for b in o.boots]
    keys = sorted(set.intersection(*[set(b) for b in boots])) if boots else []
    res = dict(build_id=o.build_id, boots=o.boots, created=time.strftime("%Y-%m-%d %H:%M:%S"), filter=o.filter,
               rule="per key: the maximum over every boot pair of each count", keys={})
    for k in keys:
        view = view_of(k)
        imgs = [gl.load_rgb(b[k]) for b in boots]
        pairs = []
        for i in range(len(imgs)):
            for j in range(i + 1, len(imgs)):
                d, m = all_metrics(imgs[i], imgs[j])
                pairs.append(dict(pair=f"{i + 1}-{j + 1}", max=m["max"], over2=m["over2"], over8=m["over8"], over0=m["over0"],
                                  largest_cluster=m["largest_cluster"], ssim=m["ssim"], de_mean=m["de_mean"], de_p99=m["de_p99"],
                                  bands=m["bands"], shift=m["shift"]))
        mx = dict(max=max(p["max"] for p in pairs), over2=max(p["over2"] for p in pairs), over8=max(p["over8"] for p in pairs),
                  largest_cluster=max(p["largest_cluster"] for p in pairs), ssim_min=min(p["ssim"] for p in pairs),
                  frac_changed=max(p["shift"]["frac_changed"] for p in pairs),
                  top5pct_share=max(p["shift"]["top5pct_share"] for p in pairs),
                  de_mean=max(p["de_mean"] for p in pairs), de_p99=max(p["de_p99"] for p in pairs),
                  bands=[dict(band=n, max=max(p["bands"][i]["max"] for p in pairs), over2=max(p["bands"][i]["over2"] for p in pairs),
                              over8=max(p["bands"][i]["over8"] for p in pairs)) for i, n in enumerate(BAND_NAMES)])
        res["keys"][k] = dict(view=view, rain=gv.is_rain(view), pairs=pairs, max=mx)
        print(f"{k}: " + " | ".join(f"{p['pair']} {p['max']}/{p['over2']:,}/{p['over8']:,}" for p in pairs)
              + f"  (largest cluster {mx['largest_cluster']}, ssim min {mx['ssim_min']})")
        sys.stdout.flush()
    gl.write_json(o.out, res)
    print("wrote", o.out)
    return 0


def cmd_pair(o):
    a, b = gl.load_rgb(o.a), gl.load_rgb(o.b)
    d, m = all_metrics(a, b)
    m["clusters"] = m["clusters"][:5]
    print(json.dumps(m, indent=1))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("compare")
    c.add_argument("a")
    c.add_argument("b")
    c.add_argument("--claim", default="", choices=["", "exact", "pit", "invisible", "look"])
    c.add_argument("--noise", default="")
    c.add_argument("--label-a", default="")
    c.add_argument("--label-b", default="")
    c.add_argument("--filter", default="")
    c.add_argument("--out", required=True)
    c.add_argument("--allow-no-manifest", action="store_true")
    c.add_argument("--expect-differ", action="store_true")
    c.add_argument("--declare-objects", action="store_true")
    c.add_argument("--strict-counts", action="store_true", help="refuse any objects= or draws= difference (items that cull nothing)")
    c.add_argument("--any-mode", action="store_true", help="accept shots whose mode is not remastered (Classic rows)")
    c.add_argument("--no-blink", action="store_true")
    c.add_argument("--no-sheets", action="store_true")
    c.add_argument("--seed", type=int, default=None)
    n = sub.add_parser("noise")
    n.add_argument("boots", nargs="+")
    n.add_argument("--build-id", required=True)
    n.add_argument("--out", required=True)
    n.add_argument("--filter", default="")
    p = sub.add_parser("pair")
    p.add_argument("a")
    p.add_argument("b")
    o = ap.parse_args()
    return {"compare": cmd_compare, "noise": cmd_noise, "pair": cmd_pair}[o.cmd](o)


if __name__ == "__main__":
    sys.exit(main())
