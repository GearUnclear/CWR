"""Render the fourth director video from trace.json + cues.json (single
view, the round-2 layout with the stability sparklines; card and outro voices
and the outro table follow the comparison renderer of video 3).

Usage:
  python render.py <trace.json> <cues.json> <out.mp4> [--audio-dir DIR] [--preview SECONDS] [--frames-only t1,t2,...]

trace.json comes from parse_trace.py (engine output of a tri run).
cues.json is the commentary: a list of {"sim": s, "text": caption,
"line": raw engine line or "", "rate": sim s per video s while the cue
shows (optional), "flash": [zone name, kind] (optional)}.  A cue's video
duration is its voice clip length (audio-dir/cue_NN.wav) plus padding, or a
reading-time estimate when there is no clip.
"""
import json
import math
import re
import os
import subprocess
import sys
import wave

from PIL import Image, ImageDraw, ImageFont

W, H, FPS = 1920, 1080, 30
FONT_DIR = r"C:\Windows\Fonts"


def font(name, size):
    return ImageFont.truetype(os.path.join(FONT_DIR, name), size)


F_TITLE = font("segoeuib.ttf", 30)
F_H = font("segoeuib.ttf", 22)
F_B = font("segoeui.ttf", 20)
F_S = font("segoeui.ttf", 16)
F_XS = font("segoeui.ttf", 14)
F_BIG = font("segoeuib.ttf", 34)
F_CAP = font("segoeui.ttf", 31)
F_MONO = font("consola.ttf", 17)
F_CARD = font("segoeui.ttf", 30)
F_CARDH = font("segoeuib.ttf", 54)

BG = (14, 20, 28)
PANEL = (22, 30, 41)
LINE = (44, 58, 74)
TXT = (226, 232, 238)
DIM = (140, 154, 168)
OCC = (222, 78, 72)
RES = (88, 190, 110)
NEU = (128, 140, 152)
GOLD = (240, 190, 70)
ORANGE = (245, 140, 50)
BLUE = (110, 170, 240)

MAP_X0, MAP_Y0, MAP_X1, MAP_Y1 = 425, 66, 1500, 900
WX0, WX1, WY0, WY1 = 2700.0, 10300.0, 2450.0, 10250.0


def scale():
    sx = (MAP_X1 - MAP_X0) / (WX1 - WX0)
    sy = (MAP_Y1 - MAP_Y0) / (WY1 - WY0)
    return min(sx, sy)


S = scale()
OX = MAP_X0 + ((MAP_X1 - MAP_X0) - (WX1 - WX0) * S) / 2
OY = MAP_Y0 + ((MAP_Y1 - MAP_Y0) - (WY1 - WY0) * S) / 2


def to_px(x, y):
    return (OX + (x - WX0) * S, OY + (WY1 - y) * S)


def lerp(a, b, f):
    return a + (b - a) * f


def side_color(owner):
    if owner == "EAST":
        return OCC
    if owner == "GUER":
        return RES
    return NEU


def fmt_clock(sim):
    # fictional campaign clock: day 1 starts 07:30 (the template's start)
    total = 7 * 3600 + 30 * 60 + sim
    day = 1 + int(total // 86400)
    total %= 86400
    return "Day %d  %02d:%02d" % (day, int(total // 3600), int((total % 3600) // 60))


def polyline_len(pts):
    return sum(math.dist(a, b) for a, b in zip(pts, pts[1:]))


def point_at(pts, frac):
    if not pts:
        return None
    if len(pts) == 1:
        return pts[0]
    total = polyline_len(pts)
    want = max(0.0, min(1.0, frac)) * total
    acc = 0.0
    for a, b in zip(pts, pts[1:]):
        d = math.dist(a, b)
        if acc + d >= want and d > 0:
            f = (want - acc) / d
            return (lerp(a[0], b[0], f), lerp(a[1], b[1], f))
        acc += d
    return pts[-1]


def wrap(draw, text, fnt, width):
    words = text.split()
    lines, cur = [], ""
    for w in words:
        t = (cur + " " + w).strip()
        if draw.textlength(t, font=fnt) <= width:
            cur = t
        else:
            if cur:
                lines.append(cur)
            cur = w
    if cur:
        lines.append(cur)
    return lines


class Scene:
    def __init__(self, trace):
        self.trace = trace
        self.rows = trace["trace"]
        self.times = [r[0] for r in self.rows]
        self.zones = {int(z[0]): z for z in trace["zones"]}
        self.zone_by_name = {z[1]: z for z in trace["zones"]}
        hq = trace.get("hq")
        self.hq_pos = hq[1] if hq else None
        # road routes: (source, target) -> polyline [(x, y)]
        self.graph = {}
        for g in trace["graph"]:
            r = g[2]
            if r and r[0] is True and isinstance(r[-1], list) and len(r[-1]) > 1:
                self.graph[(g[0], g[1])] = [(p[0], p[1]) for p in r[-1]]
        for rt in trace["routes"]:
            src, tgt, r = rt["row"]
            if r and r[0] is True and isinstance(r[-1], list) and len(r[-1]) > 1:
                self.graph.setdefault((src, tgt), [(p[0], p[1]) for p in r[-1]])
        self.cross = set()
        for e in trace["info"]:
            t = e["text"]
            if t.startswith("CounterAttack: the road from ") and "cross-country" in t:
                body = t[len("CounterAttack: the road from "):]
                a, rest = body.split(" to ", 1)
                b = rest.split(" is ")[0]
                self.cross.add((a, b))
        # first eta per (op id, source, package) gives the travel clock length
        self.eta0 = {}
        for r in self.rows:
            for op in r[3]:
                key = (op[0], op[4], op[3])
                if op[6] in ("TRAVEL", "SCHEDULED"):
                    self.eta0[key] = max(self.eta0.get(key, 0), op[9])
        # owner flips for the right panel
        self.flips = []
        prev = None
        for r in self.rows:
            owners = {int(z[0]): z[1] for z in r[6]}
            if prev is not None:
                for i, o in owners.items():
                    if prev.get(i) != o:
                        self.flips.append((r[0], self.zones[i][1], prev.get(i), o))
            prev = owners
        # the stability series (gmSimDiag in every UDTRACE row)
        self.series = []
        for r in self.rows:
            dg = r[11] if len(r) > 11 and isinstance(r[11], list) and len(r[11]) > 10 else None
            if dg is None:
                continue
            ops = r[3]
            live_ops = sum(1 for op in ops if op[6] == "LIVE" or op[8] > 0)
            # OpsCap: forcesOpsMaxStrong 3 while STRONG, else forcesOpsMax 2, plus one per wave record
            cap = (3 if r[2][2] == "STRONG" else 2) + sum(1 for op in ops if op[2] == "WAVE")
            far = dg[10] if isinstance(dg[10], list) and len(dg[10]) >= 6 else [0] * 6
            skipped = r[14] if len(r) > 14 else 0
            self.series.append((r[0], dg[2][0], dg[2][1], dg[2][2], dg[3][0], dg[3][1], dg[3][2], dg[3][3], far[0], live_ops, cap, dg[8][1], dg[7][1], far, dg[2][3], skipped))
        # the pool ceiling's ledger lines (5.5): banked and dropped units
        self.banked, self.dropped = [], []
        for e in trace["info"]:
            m = re.search(r"ledger: (\d+) unit\(s\) over poolStart (banked|dropped)", e["text"])
            if m:
                (self.banked if m.group(2) == "banked" else self.dropped).append((e["sim"], int(m.group(1))))
        # where an air operation flies from (the re-plan / strike lines)
        self.air_from = {}
        for e in trace["info"]:
            t = e["text"]
            m = re.search(r"\[op (\d+)\] .*(?:insertion|strike by).* from (off-map|[A-Z][A-Za-z ]+?)(?= \(|;|,| to |$)", t)
            if m:
                self.air_from[int(m.group(1))] = m.group(2)

    def index(self, t):
        lo, hi = 0, len(self.times) - 1
        if t <= self.times[0]:
            return 0, 0.0
        if t >= self.times[-1]:
            return hi, 0.0
        while hi - lo > 1:
            mid = (lo + hi) // 2
            if self.times[mid] <= t:
                lo = mid
            else:
                hi = mid
        span = self.times[hi] - self.times[lo]
        return lo, (t - self.times[lo]) / span if span > 0 else 0.0

    def route(self, src, tgt):
        zs, zt = self.zone_by_name.get(src), self.zone_by_name.get(tgt)
        if not zs or not zt:
            return None
        a = (zs[5][0], zs[5][1])
        b = (zt[5][0], zt[5][1])
        if (src, tgt) in self.cross:
            return [a, b]
        pts = self.graph.get((src, tgt))
        if pts:
            return [a] + pts + [b]
        return [a, b]


def op_progress(scene, op):
    key = (op[0], op[4], op[3])
    e0 = scene.eta0.get(key, 0)
    if e0 <= 0:
        return 0.0
    return max(0.0, min(1.0, 1.0 - op[9] / e0))


def is_air(pkg):
    return pkg in ("TRANSPORT_HELI", "ATTACK_HELI", "JET")


def build_static(scene):
    big = Image.new("RGB", (W * 2, H * 2), BG)
    d = ImageDraw.Draw(big)

    def P(x, y):
        px, py = to_px(x, y)
        return (px * 2, py * 2)

    # 1 km grid
    for gx in range(3000, 10501, 1000):
        d.line([P(gx, WY0), P(gx, WY1)], fill=(20, 28, 38), width=2)
    for gy in range(3000, 10501, 1000):
        d.line([P(WX0, gy), P(WX1, gy)], fill=(20, 28, 38), width=2)
    # the road network as the director's router sees it
    for (src, tgt), pts in scene.graph.items():
        if (src, tgt) in scene.cross:
            continue
        d.line([P(*p) for p in pts], fill=(38, 52, 66), width=5, joint="curve")
    img = big.resize((W, H), Image.LANCZOS)
    d = ImageDraw.Draw(img)
    # panels
    d.rectangle([0, 0, W, 60], fill=PANEL)
    d.rectangle([0, 60, 415, 905], fill=PANEL)
    d.rectangle([1505, 60, W, 905], fill=PANEL)
    d.rectangle([0, 905, W, H], fill=(10, 14, 20))
    d.line([(0, 905), (W, 905)], fill=LINE, width=2)
    # scale bar
    x0, y0 = to_px(3000, 2650)
    x1, _ = to_px(5000, 2650)
    d.line([(x0, y0), (x1, y0)], fill=DIM, width=2)
    d.text((x0, y0 + 6), "2 km", font=F_XS, fill=DIM)
    return img


def draw_zone(d, z, owner, row_z, t_flash, sim, hq_pos):
    name, ztype = z[1], z[2]
    x, y = to_px(z[5][0], z[5][1])
    base = ztype in ("OUTPOST", "SEAPORT", "AIRFIELD")
    camp = ztype == "CAMP"
    half = 15 if base else (12 if camp else 8)
    col = side_color(owner)
    heat = row_z[4] if row_z else 0
    if heat > 0 and owner == "GUER":
        r = half + 6 + heat / 6
        d.ellipse([x - r, y - r, x + r, y + r], outline=(min(255, 120 + int(heat * 1.3)), 90, 40), width=2)
    d.rectangle([x - half, y - half, x + half, y + half], fill=col if not camp else (40, 70, 50), outline=(230, 230, 230) if base else None, width=2)
    if camp:
        d.rectangle([x - half, y - half, x + half, y + half], outline=RES, width=3)
    # meters: occupier base-recapture meter (red), town support (green)
    if row_z:
        occc = row_z[5]
        sup = row_z[3]
        if base and owner == "GUER" and occc > 0:
            r = half + 7
            d.arc([x - r, y - r, x + r, y + r], -90, -90 + 3.6 * occc, fill=OCC, width=5)
        if ztype == "CITY" and (owner == "GUER" or row_z[8] > 0):
            r = half + 5
            d.arc([x - r, y - r, x + r, y + r], -90, -90 + 3.6 * sup, fill=RES, width=3)
    if row_z and len(row_z) > 12 and row_z[12] > sim:
        # the town-strike stamp: half income and the 2 km fear ring until then
        rr = 2000 * S
        for k in range(0, 360, 10):
            d.arc([x - rr, y - rr, x + rr, y + rr], k, k + 5, fill=ORANGE, width=2)
        d.rectangle([x - half - 3, y - half - 3, x + half + 3, y + half + 3], outline=ORANGE, width=2)
        d.text((x - half, y + half + 4), "struck until %s" % fmt_clock(row_z[12])[7:], font=F_XS, fill=ORANGE)
    if t_flash is not None:
        k = t_flash
        r = half + 10 + 50 * k
        a = int(255 * (1 - k))
        d.ellipse([x - r, y - r, x + r, y + r], outline=(255, 255, 255) if a > 0 else BG, width=3)
    label = name if name != "Village" else "Houdan"
    if base and row_z is not None:
        g = int(row_z[2])
        d.text((x, y), str(g), font=F_XS, fill=(255, 255, 255), anchor="mm")
    lx = x + half + 5
    d.text((lx, y - 9), label, font=F_S if base or camp else F_XS, fill=TXT if base or camp else DIM)
    if camp and hq_pos is not None:
        d.text((lx, y + 8), "player HQ", font=F_XS, fill=RES)


def draw_triangle(d, x, y, ang, size, fill, outline):
    pts = []
    for da in (0, 140, 220):
        a = math.radians(ang + da)
        pts.append((x + math.cos(a) * size, y + math.sin(a) * size))
    d.polygon(pts, fill=fill, outline=outline)


def spark(d, x, y, w, h, scene, sim, picks, ymax, ref=None):
    """Thin full-run sparklines: the elapsed part bright, the rest faint,
    a cursor at sim.  picks: [(index into series, colour)]."""
    ser = scene.series
    if not ser:
        return
    t0, t1 = ser[0][0], ser[-1][0]
    span = max(1.0, t1 - t0)
    d.rectangle([x, y, x + w, y + h], fill=(18, 25, 34))
    if ref is not None and ymax > 0:
        ry = y + h - h * min(1.0, ref / ymax)
        for k in range(int(x), int(x + w), 8):
            d.line([(k, ry), (k + 4, ry)], fill=(110, 110, 80), width=1)
    step = max(1, len(ser) // 600)
    for idx, col in picks:
        past, fut = [], []
        for n in range(0, len(ser), step):
            row = ser[n]
            px = x + w * (row[0] - t0) / span
            v = row[idx]
            py = y + h - h * (min(ymax, max(0.0, v)) / ymax if ymax > 0 else 0)
            (past if row[0] <= sim else fut).append((px, py))
        dim = tuple(int(c * 0.35 + 18 * 0.65) for c in col)
        if len(fut) > 1:
            d.line(([past[-1]] if past else []) + fut, fill=dim, width=1)
        if len(past) > 1:
            d.line(past, fill=col, width=2)
    cx = x + w * (min(max(sim, t0), t1) - t0) / span
    d.line([(cx, y - 2), (cx, y + h + 2)], fill=(230, 230, 230), width=1)


def series_at(scene, sim):
    ser = scene.series
    if not ser:
        return None
    lo = 0
    for n, row in enumerate(ser):
        if row[0] <= sim:
            lo = n
        else:
            break
    return ser[lo]


def draw_stability(d, scene, sim):
    x, w = 24, 366
    y = 452
    d.line([(x, y - 6), (x + w, y - 6)], fill=LINE, width=1)
    d.text((x, y), "STABILITY  (engine census every 10 s, whole run)", font=F_XS, fill=TXT)
    y += 20
    ser = scene.series
    if not ser:
        return
    cur = series_at(scene, sim)
    mx_live = max(max(r[14] for r in ser), 130) * 1.08
    mx_grp = max(r[7] for r in ser) or 1
    mx_far = max(max(r[8] for r in ser), 2)
    mx_ops = max(max(r[10] for r in ser), max(r[9] for r in ser), 1)
    mx_fp = max(max(r[11] for r in ser), 1)
    mx_sk = max(max(r[15] for r in ser), 1)
    rows = [
        ("live AI %d of 130  (occ %d, res %d, civ %d)" % (cur[14], cur[1], cur[2], cur[3]), [(14, (230, 230, 230)), (1, OCC), (2, RES), (3, NEU)], mx_live, 130),
        ("traffic skippedBudget %d (spawns held for the budget)" % cur[15], [(15, BLUE)], mx_sk * 1.1, None),
        ("far live groups %d  (arr %d, march %d, left %d, home %d, air %d)" % (cur[8], cur[13][1], cur[13][2], cur[13][3], cur[13][4], cur[13][5]), [(8, ORANGE)], mx_far * 1.1, None),
        ("live operations %d of OpsCap %d" % (cur[9], cur[10]), [(10, (150, 150, 120)), (9, (255, 120, 110))], mx_ops * 1.15, None),
        ("frame p95 %.1f ms  (gm phase p95 %.2f ms)" % (cur[11], cur[12]), [(11, (200, 200, 230))], mx_fp * 1.1, None),
        ("groups  occ %d  res %d  civ %d  (largest side %d)" % (cur[4], cur[5], cur[6], cur[7]), [(4, OCC), (5, RES), (6, NEU)], mx_grp * 1.1, None),
    ]
    for label, picks, ymax, ref in rows:
        d.text((x, y), label, font=F_XS, fill=DIM)
        y += 20
        spark(d, x, y, w, 28, scene, sim, picks, ymax, ref)
        y += 38


def render_frame(scene, static, sim, cue, cue_k, flashes, title=None):
    img = static.copy()
    d = ImageDraw.Draw(img)
    i, f = scene.index(sim)
    r0 = scene.rows[i]
    r1 = scene.rows[min(i + 1, len(scene.rows) - 1)]
    forces = r0[2]
    ops = r0[3]
    intel = r0[4]
    cas = r0[5]
    zrows = {int(z[0]): z for z in r0[6]}
    air = r0[7]
    ppos0, ppos1 = r0[8], r1[8]
    if math.dist(ppos0[:2], ppos1[:2]) > 300:
        ppos = ppos0 if f < 0.5 else ppos1
    else:
        ppos = (lerp(ppos0[0], ppos1[0], f), lerp(ppos0[1], ppos1[1], f))
    hq = r0[9]

    # ---- top bar
    d.text((20, 13), "OPERATION LONG HARVEST", font=F_TITLE, fill=TXT)
    d.text((470, 20), "a fictional campaign on Guerrilla.Abel, played by the real occupier director", font=F_S, fill=DIM)
    d.text((1180, 15), fmt_clock(sim), font=F_H, fill=GOLD)
    d.text((1370, 19), "game time %5.0f s" % sim, font=F_S, fill=DIM)
    d.rounded_rectangle([1600, 14, 1900, 46], radius=8, fill=(30, 60, 40))
    d.text((1750, 30), "ENGINE TRACE, x%d game speed" % int(r0[10]), font=F_XS, fill=(170, 230, 180), anchor="mm")

    # ---- active op routes (under the zones)
    live_pos = {}
    for ca in cas:
        live_pos[ca[0]] = ca
    for op in ops:
        src, tgt, pkg, phase = op[4], op[5], op[3], op[6]
        if is_air(pkg):
            continue
        rt = scene.route(src, tgt)
        if rt and phase in ("TRAVEL", "SCHEDULED", "LIVE", "HOLDING"):
            d.line([to_px(*p) for p in rt], fill=(120, 60, 58), width=3)

    # ---- player bubble
    px, py = to_px(ppos[0], ppos[1])
    rb = 800 * S
    for k in range(0, 360, 12):
        d.arc([px - rb, py - rb, px + rb, py + rb], k, k + 6, fill=(150, 170, 200), width=1)
    d.polygon([(px, py - 9), (px + 8, py), (px, py + 9), (px - 8, py)], fill=(250, 250, 250))
    d.text((px + 11, py - 8), "player", font=F_XS, fill=(200, 210, 230))

    # ---- zones
    for zi, z in scene.zones.items():
        zr = zrows.get(zi)
        owner = zr[1] if zr else z[3]
        fl = None
        for (fname, fsim, kind) in flashes:
            if fname == z[1] and 0 <= sim - fsim:
                pass
        draw_zone(d, z, owner, zr, None, sim, scene.hq_pos)
    # flashes (captures, strikes) in video time
    for (fname, k, kind) in flashes:
        z = scene.zone_by_name.get(fname)
        if not z:
            continue
        x, y = to_px(z[5][0], z[5][1])
        r = 20 + 60 * k
        col = ORANGE if kind == "strike" else (255, 255, 255)
        d.ellipse([x - r, y - r, x + r, y + r], outline=col, width=4 if kind == "strike" else 3)
        if kind == "strike" and k < 0.5:
            rr = 26 * (1 - k)
            d.ellipse([x - rr, y - rr, x + rr, y + rr], fill=(255, 200, 90))

    # ---- HQ meter
    if hq and hq[0] is True and len(hq) > 5 and isinstance(hq[5], list) and len(hq[5]) >= 2:
        hx, hy = to_px(hq[5][0], hq[5][1])
        if hq[3] > 0:
            d.arc([hx - 26, hy - 26, hx + 26, hy + 26], -90, -90 + 3.6 * hq[3], fill=OCC, width=6)
        if hq[1] is True:
            d.line([(hx - 16, hy - 16), (hx + 16, hy + 16)], fill=OCC, width=5)
            d.line([(hx - 16, hy + 16), (hx + 16, hy - 16)], fill=OCC, width=5)

    # ---- forces
    ops1 = {o[0]: o for o in r1[3]}
    cas1 = {c[0]: c for c in r1[5]}
    for op in ops:
        oid, kind, origin, pkg, src, tgt, phase, bodies = op[:8]
        label = "op %d %s%s %d" % (oid, pkg.replace("TRANSPORT_HELI", "HELI"), ("+" + op[12]) if len(op) > 12 and op[12] else "", bodies)
        ca = live_pos.get(oid)
        if phase == "LIVE" and ca is not None:
            lp = ca[7]
            c1 = cas1.get(oid)
            if c1 is not None and c1[7][0] != 0 and lp[0] != 0 and math.dist(lp[:2], c1[7][:2]) < 400:
                lp = (lerp(lp[0], c1[7][0], f), lerp(lp[1], c1[7][1], f))
            if lp[0] != 0:
                x, y = to_px(lp[0], lp[1])
                airborne = any(a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 3 and a[3][2] > 5 and math.dist(a[3][:2], lp[:2]) < 60 for a in air)
                if is_air(pkg) and airborne:
                    draw_triangle(d, x, y, -90, 11, OCC, (255, 255, 255))
                else:
                    d.ellipse([x - 8, y - 8, x + 8, y + 8], fill=OCC, outline=(255, 255, 255), width=2)
                for hp in (ca[8], ca[9]):
                    if hp[0] != 0:
                        hx, hy = to_px(hp[0], hp[1])
                        d.rectangle([hx - 6, hy - 4, hx + 6, hy + 4], fill=(170, 60, 55), outline=(255, 255, 255))
                d.text((x + 11, y + 4), label + " (live)", font=F_XS, fill=(255, 190, 180))
                continue
        if is_air(pkg):
            zt = scene.zone_by_name.get(tgt)
            zs = scene.zone_by_name.get(src)
            if zt is None:
                continue
            tx, ty = zt[5][0], zt[5][1]
            org = scene.air_from.get(int(oid))
            if org and org in scene.zone_by_name:
                zs = scene.zone_by_name[org]
            elif org == "off-map":
                zs = None
            if zs is not None and src != tgt:
                sx, sy = zs[5][0], zs[5][1]
            else:
                sx, sy = tx, WY0 - 1500
            p = op_progress(scene, op)
            o1 = ops1.get(oid)
            if o1 is not None and o1[3] == pkg:
                p = lerp(p, op_progress(scene, o1), f)
            x0, y0 = to_px(sx, sy)
            x1, y1 = to_px(tx, ty)
            for k in range(0, 20, 2):
                d.line([(lerp(x0, x1, k / 20), lerp(y0, y1, k / 20)), (lerp(x0, x1, (k + 1) / 20), lerp(y0, y1, (k + 1) / 20))], fill=(150, 80, 70), width=2)
            if phase == "LIVE":
                live_air = [a for a in air if a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 2]
                if live_air:
                    ax, ay = to_px(live_air[0][3][0], live_air[0][3][1])
                    d.text((ax + 12, ay + 4), label + " (live)", font=F_XS, fill=(255, 190, 180))
                    continue
            x, y = lerp(x0, x1, p), lerp(y0, y1, p)
            ang = math.degrees(math.atan2(y1 - y0, x1 - x0))
            draw_triangle(d, x, y, ang, 12, None, OCC)
            d.text((x + 12, y + 4), label, font=F_XS, fill=(255, 190, 180))
            continue
        rt = scene.route(src, tgt)
        if not rt:
            continue
        if phase == "SCHEDULED":
            pt = rt[0]
        elif phase == "HOLDING":
            pt = rt[-1]
        else:
            p = op_progress(scene, op)
            o1 = ops1.get(oid)
            if o1 is not None and o1[4] == src and o1[6] == "TRAVEL":
                p = lerp(p, op_progress(scene, o1), f)
            pt = point_at(rt, p)
        x, y = to_px(*pt)
        d.ellipse([x - 8, y - 8, x + 8, y + 8], outline=OCC, width=3)
        d.text((x + 11, y + 4), label + (" holding" if phase == "HOLDING" else ""), font=F_XS, fill=(255, 190, 180))
    # live airframes not tied to an op row (flying home)
    for a in air:
        if a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 2:
            x, y = to_px(a[3][0], a[3][1])
            draw_triangle(d, x, y, -90, 10, (200, 90, 80), (255, 255, 255))

    # ---- left panel: the pool and the director's budget
    x = 24
    yy = 74
    d.text((x, yy), "OCCUPIER DIRECTOR", font=F_H, fill=TXT)
    yy += 34
    pool = lerp(forces[0], r1[2][0], f)
    start = forces[1] or 60
    d.text((x, yy), "Pool (soldier units), ceiling poolStart", font=F_S, fill=DIM)
    d.text((390, yy), "%d / %d" % (round(pool), start), font=F_S, fill=TXT, anchor="ra")
    yy += 24
    bw = 366
    d.rectangle([x, yy, x + bw, yy + 20], fill=(34, 44, 58))
    fill = max(0.0, min(1.5, pool / start))
    d.rectangle([x, yy, x + bw * min(1.0, fill), yy + 20], fill=OCC)
    if fill > 1:
        d.rectangle([x + bw * 0.98, yy, x + bw, yy + 20], fill=GOLD)
    for m in (0.33, 0.40, 0.60, 0.66):
        mx = x + bw * m
        d.line([(mx, yy - 3), (mx, yy + 23)], fill=(230, 230, 230), width=1)
    yy += 24
    d.text((x, yy), "THIN 33/40%", font=F_XS, fill=DIM)
    d.text((x + bw * 0.60 - 20, yy), "STRONG 60/66%", font=F_XS, fill=DIM)
    yy += 24
    band = forces[2]
    bandc = {"STRONG": OCC, "STRETCHED": ORANGE, "THIN": GOLD, "SPENT": NEU}.get(band, NEU)
    d.text((x, yy + 6), "Band", font=F_S, fill=DIM)
    d.text((x + 60, yy - 2), band, font=F_BIG, fill=bandc)
    d.text((x + 250, yy + 6), "Posture", font=F_S, fill=DIM)
    d.text((x + 250, yy + 26), forces[4], font=F_H, fill=TXT if forces[4] == "AWAKE" else DIM)
    yy += 52
    d.text((x, yy), "Command tokens", font=F_S, fill=DIM)
    tok = int(forces[3])
    for k in range(2):
        cx = x + 150 + k * 34
        d.ellipse([cx, yy - 1, cx + 22, yy + 21], outline=GOLD, width=3, fill=GOLD if k < tok else None)
    if forces[4] == "AWAKE":
        d.text((x + 230, yy + 3), "next in %d min" % max(0, round(forces[14] / 60.0)), font=F_XS, fill=DIM)
    yy += 30
    ctl = lerp(forces[5], r1[2][5], f)
    d.text((x, yy), "Control  C = 0.6 M + 0.4 T", font=F_S, fill=DIM)
    d.text((390, yy), "%.3f" % ctl, font=F_S, fill=TXT, anchor="ra")
    yy += 24
    d.rectangle([x, yy, x + bw, yy + 14], fill=(34, 44, 58))
    d.rectangle([x, yy, x + bw * max(0, min(1, ctl)), yy + 14], fill=(150, 160, 200))
    for m, lab in ((0.75, "wave 1"), (0.50, "wave 2")):
        mx = x + bw * m
        d.line([(mx, yy - 4), (mx, yy + 18)], fill=GOLD, width=2)
        d.text((mx, yy + 17), lab, font=F_XS, fill=GOLD, anchor="ma")
    yy += 38
    inbound = forces[15] is True
    wtxt = "Waves fired %d, landed %d" % (int(forces[18]), int(forces[22]))
    if inbound:
        wtxt += ";  inbound %d, lands in %d s" % (int(forces[17]), int(forces[16]))
    d.text((x, yy), wtxt, font=F_S, fill=GOLD if inbound else TXT)
    yy += 26
    d.text((x, yy), "Towns ever taken: " + (", ".join(t.replace("Village", "Houdan") for t in forces[10]) or "none"), font=F_XS, fill=DIM)
    yy += 19
    cds = forces[12]
    if cds:
        d.text((x, yy), ("Resting: " + ", ".join("%s %d min" % (c[0].replace("Village", "Houdan"), round(c[1] / 60)) for c in cds))[:62], font=F_XS, fill=DIM)
    yy += 19
    if forces[11]:
        d.text((x, yy), ("Urgent: " + ", ".join(u.replace("Village", "Houdan") for u in forces[11]))[:62], font=F_XS, fill=ORANGE)
    yy += 19
    nb = sum(n for (t, n) in scene.banked if t <= sim)
    nd = sum(n for (t, n) in scene.dropped if t <= sim)
    d.text((x, yy), "Lost for good %d;  ledger: banked %d, dropped %d" % (int(forces[9]), nb, nd), font=F_XS, fill=DIM)
    # ---- stability panel: the [gmsim] census over the whole run
    draw_stability(d, scene, sim)

    # legend (map, top right: open sea)
    lx, ly = 1222, 72
    d.ellipse([lx, ly + 2, lx + 14, ly + 16], outline=OCC, width=3)
    d.text((lx + 22, ly), "abstract force (a record, no soldiers)", font=F_XS, fill=DIM)
    ly += 20
    d.ellipse([lx, ly + 2, lx + 14, ly + 16], fill=OCC, outline=(255, 255, 255))
    d.text((lx + 22, ly), "born force (live AI)", font=F_XS, fill=DIM)
    ly += 20
    draw_triangle(d, lx + 7, ly + 9, -90, 8, None, OCC)
    d.text((lx + 22, ly), "aircraft (filled: live)", font=F_XS, fill=DIM)
    ly += 20
    for k in range(0, 360, 30):
        d.arc([lx, ly + 2, lx + 14, ly + 16], k, k + 15, fill=(150, 170, 200), width=1)
    d.text((lx + 22, ly), "player's 800 m bubble", font=F_XS, fill=DIM)
    ly += 20
    for k in range(0, 360, 30):
        d.arc([lx, ly + 2, lx + 14, ly + 16], k, k + 15, fill=ORANGE, width=2)
    d.text((lx + 22, ly), "struck town: 2 km fear ring", font=F_XS, fill=DIM)

    # ---- right panel: intel, heat, operations, flips, the HQ
    x = 1522
    yy = 76
    d.text((x, yy), "DIRECTOR INTEL", font=F_H, fill=TXT)
    yy += 28
    d.text((x, yy), "only what its men saw or a local told", font=F_XS, fill=DIM)
    yy += 22
    if not intel:
        d.text((x, yy), "no sightings", font=F_S, fill=DIM)
        yy += 22
    for row in intel[:3]:
        zn = (row[0] or "outside zones").replace("Village", "Houdan")
        d.text((x, yy), "%s: %d fighter(s), %d s old" % (zn, int(row[1]), int(row[2])), font=F_S, fill=TXT)
        yy += 22
    yy += 8
    d.text((x, yy), "HEAT (resistance-held)", font=F_H, fill=TXT)
    yy += 28
    held = [(zi, zr) for zi, zr in sorted(zrows.items()) if zr[1] == "GUER" and scene.zones[zi][2] != "CAMP"]
    if not held:
        d.text((x, yy), "nothing held yet", font=F_S, fill=DIM)
        yy += 22
    urgent = set(forces[11]) if isinstance(forces[11], list) else set()
    for zi, zr in held[:5]:
        zname = scene.zones[zi][1]
        zn = zname.replace("Village", "Houdan")
        d.text((x, yy), zn, font=F_S, fill=ORANGE if zname in urgent else TXT)
        d.rectangle([x + 150, yy + 4, x + 340, yy + 18], fill=(34, 44, 58))
        d.rectangle([x + 150, yy + 4, x + 150 + 1.9 * min(100, zr[4]), yy + 18], fill=ORANGE)
        d.text((x + 348, yy), "%d" % round(zr[4]), font=F_XS, fill=DIM)
        yy += 22
    yy += 8
    d.text((x, yy), "OPERATIONS", font=F_H, fill=TXT)
    yy += 28
    if not ops:
        d.text((x, yy), "none in flight", font=F_S, fill=DIM)
        yy += 22
    for op in ops[:5]:
        oid, kind, origin, pkg, src, tgt, phase, bodies = op[:8]
        pk = pkg.replace("TRANSPORT_HELI", "HELI").replace("ATTACK_HELI", "ATK HELI")
        if len(op) > 12 and op[12]:
            pk += "+" + op[12]
        d.text((x, yy), "op %d %s %s %s" % (oid, kind, origin.lower(), pk), font=F_S, fill=(255, 190, 180))
        yy += 20
        eta = op[9]
        s3 = "%s > %s: %s, %d bodies%s" % (src.replace("Village", "Houdan"), tgt.replace("Village", "Houdan"), phase, int(bodies), (", eta %d s" % eta) if eta > 0 else "")
        d.text((x + 12, yy), s3[:58], font=F_XS, fill=DIM)
        yy += 22
    yy += 6
    d.text((x, yy), "OWNER CHANGES", font=F_H, fill=TXT)
    yy += 28
    recent = [fl for fl in scene.flips if fl[0] <= sim][-4:]
    for (ft, zn, a, b) in reversed(recent):
        zn = zn.replace("Village", "Houdan")
        d.text((x, yy), "%s  %s" % (fmt_clock(ft)[6:], zn), font=F_XS, fill=TXT)
        d.text((x + 200, yy), "to " + ("resistance" if b == "GUER" else ("occupier" if b == "EAST" else "occupier (admin.)")), font=F_XS, fill=side_color(b))
        yy += 19
    # the HQ and the campaign state
    yy = max(yy + 8, 790)
    d.text((x, yy), "HEADQUARTERS", font=F_H, fill=TXT)
    yy += 28
    if hq and len(hq) > 3:
        if hq[1] is True and hq[0] is not True:
            st, col = "DESTROYED (epoch %d)" % int(hq[2]), OCC
        elif hq[0] is True:
            st, col = "standing%s, meter %d" % (" (re-established)" if hq[2] > 0 else "", round(hq[3])), RES
        else:
            st, col = "none", DIM
        d.text((x, yy), st, font=F_S, fill=col)
        yy += 22
    ce = r0[12] if len(r0) > 12 and isinstance(r0[12], list) else None
    if ce and len(ce) > 8:
        d.text((x, yy), "campaign: %s; bases held %d of %d" % ("LOST" if ce[8] is True else ("won" if ce[0] is True else "goes on"), int(ce[4]), int(ce[5])), font=F_XS, fill=OCC if ce[8] is True else DIM)

    # ---- commentary
    if cue is not None:
        alpha = min(1.0, cue_k * 8)
        lines = wrap(d, cue["text"], F_CAP, W - 120)
        cy = 916
        if len(lines) > 2 and not cue.get("_warned"):
            cue["_warned"] = True
            print("CAPTION TOO LONG (%d lines): %s" % (len(lines), cue["text"][:60]))
        for ln in lines[:2]:
            d.text((60, cy), ln, font=F_CAP, fill=tuple(int(c * alpha + 10 * (1 - alpha)) for c in TXT))
            cy += 40
        if cue.get("line"):
            lw = wrap(d, "engine log: " + cue["line"], F_MONO, W - 120)
            if len(lw) > 2:
                lw = lw[:2]
                lw[1] = lw[1][: max(0, len(lw[1]) - 3)] + "..."
            ly = 1000 if len(lines) > 1 else 962
            for ln in lw:
                d.text((60, ly), ln, font=F_MONO, fill=(120, 200, 140))
                ly += 24
    return img


F_TAB = font("segoeui.ttf", 23)
F_TABH = font("segoeuib.ttf", 23)


def table_card(table, title, foot):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    d.text((W // 2, 34), title, font=F_CARDH, fill=TXT, anchor="ma")
    xm, xa = 90, 760
    y = 124
    for row in table:
        metric, a = row[0], str(row[1])
        la = wrap(d, a, F_TAB, W - 90 - xa)
        lm = wrap(d, metric, F_TABH, xa - xm - 30)
        n = max(len(la), len(lm))
        d.line([(xm, y - 6), (W - 90, y - 6)], fill=LINE, width=1)
        for k, ln in enumerate(lm):
            d.text((xm, y + k * 29), ln, font=F_TABH, fill=GOLD if row[0].startswith("Verdict") else TXT)
        for k, ln in enumerate(la):
            d.text((xa, y + k * 29), ln, font=F_TAB, fill=TXT)
        y += n * 29 + 12
    d.text((W // 2, 1046), foot, font=F_S, fill=DIM, anchor="ma")
    return img


def card(lines_spec, sub=None):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    y = 260
    for kind, text in lines_spec:
        if kind == "h":
            d.text((W // 2, y), text, font=F_CARDH, fill=TXT, anchor="ma")
            y += 90
        elif kind == "s":
            d.text((W // 2, y), text, font=F_H, fill=GOLD, anchor="ma")
            y += 60
        else:
            for ln in wrap(d, text, F_CARD, 1400):
                d.text((W // 2, y), ln, font=F_CARD, fill=TXT if kind == "p" else DIM, anchor="ma")
                y += 44
            y += 18
    return img


def clip_name(text):
    import hashlib
    return hashlib.md5(text.encode("utf-8")).hexdigest()[:12] + ".wav"


def wav_len(path):
    with wave.open(path) as w:
        return w.getnframes() / float(w.getframerate())


def build_timeline(cues, audio_dir, sim_end):
    """Return [(video_t0, video_t1, sim_t0, sim_t1, cue or None)]."""
    segs = []
    vt = 0.0
    prev_sim = cues[0]["sim"] if cues else 0
    for n, c in enumerate(cues):
        clip = os.path.join(audio_dir, clip_name(c["text"])) if audio_dir else None
        if clip and os.path.exists(clip):
            dur = wav_len(clip) + 0.9
        else:
            dur = max(3.5, len(c["text"].split()) / 2.6 + 1.0)
        dur = max(dur, c.get("min", 0))
        s0 = c["sim"]
        if s0 > prev_sim + 0.01:
            gap = s0 - prev_sim
            gdur = max(0.4, min(c.get("gapmax", 3.0), gap / 500.0))
            segs.append((vt, vt + gdur, prev_sim, s0, None))
            vt += gdur
        nxt = cues[n + 1]["sim"] if n + 1 < len(cues) else sim_end
        rate = c.get("rate", 6.0)
        s1 = min(nxt, s0 + dur * rate)
        s1 = max(s1, s0)
        c["_clip"] = clip if clip and os.path.exists(clip) else None
        c["_vt"] = vt
        segs.append((vt, vt + dur, s0, s1, c))
        vt += dur
        prev_sim = s1
    return segs, vt


def main():
    args = sys.argv[1:]
    trace = json.load(open(args[0]))
    spec = json.load(open(args[1], encoding="utf-8"))
    out = args[2]
    audio_dir = args[args.index("--audio-dir") + 1] if "--audio-dir" in args else None
    preview = float(args[args.index("--preview") + 1]) if "--preview" in args else None
    frames_only = args[args.index("--frames-only") + 1] if "--frames-only" in args else None
    scene = Scene(trace)
    static = build_static(scene)
    sim_end = scene.times[-1]
    cues = spec["cues"]
    segs, total = build_timeline(cues, audio_dir, sim_end)
    cards = []
    t = 0.0
    for c in spec["cards"]:
        dur = c.get("dur", 8.0)
        clip = os.path.join(audio_dir, clip_name(c["voice"])) if (audio_dir and c.get("voice")) else None
        if clip and os.path.exists(clip):
            dur = max(dur, wav_len(clip) + 1.0)
            c["_clip"] = clip
        else:
            c["_clip"] = None
        c["_vt"] = t
        cards.append((t, t + dur, card(c["lines"]), c))
        t += dur
    lead = t
    outro_voice = spec.get("outro_voice")
    outro_dur = 16.0
    oclip = os.path.join(audio_dir, clip_name(outro_voice)) if (audio_dir and outro_voice) else None
    if oclip and os.path.exists(oclip):
        outro_dur = max(outro_dur, wav_len(oclip) + 3.0)
    outro = table_card(spec["table"], spec.get("table_title", "What the run counted"), spec.get("foot", ""))
    total_all = lead + total + outro_dur
    print("video %.1f s (cards %.1f, body %.1f, outro %.1f)" % (total_all, lead, total, outro_dur))
    if preview:
        total_all = min(total_all, preview)
    events = [(fl[0], fl[1], "capture") for fl in scene.flips]
    for e in trace["info"]:
        if "] Struck " in e["text"]:
            for zn in scene.zone_by_name:
                if zn in e["text"]:
                    events.append((e["sim"], zn, "strike"))
                    break

    def frame_at(vt):
        if vt < lead:
            for (a, b, img, c) in cards:
                if a <= vt < b:
                    return img
            return cards[-1][2]
        bt = vt - lead
        if bt >= total:
            return outro
        si = 0
        while si < len(segs) - 1 and bt >= segs[si][1]:
            si += 1
        a, b, s0, s1, c = segs[si]
        k = (bt - a) / (b - a) if b > a else 0
        sim = lerp(s0, s1, k)
        if c is None:
            prev = None
            for j in range(si - 1, -1, -1):
                if segs[j][4] is not None:
                    prev = segs[j][4]
                    break
            cue, ck = prev, 1.0
        else:
            cue, ck = c, k * (b - a)
        fl = []
        rate = (s1 - s0) / (b - a) if b > a else 1
        for (es, zn, kind) in events:
            dt_sim = sim - es
            if dt_sim < 0:
                continue
            dvid = dt_sim / max(rate, 1e-3)
            if dvid < 1.6:
                fl.append((zn, dvid / 1.6, kind))
        return render_frame(scene, static, sim, cue, ck, fl)

    if frames_only:
        for n, vt in enumerate(float(x) for x in frames_only.split(",")):
            frame_at(vt).save(out.replace(".mp4", "_f%02d.png" % n))
        return
    ff = subprocess.Popen(["ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", "%dx%d" % (W, H), "-r", str(FPS), "-i", "-",
                           "-c:v", "libx264", "-pix_fmt", "yuv420p", "-crf", "18", "-preset", "medium", out + ".video.mp4"], stdin=subprocess.PIPE)
    nframes = int(total_all * FPS)
    for fr in range(nframes):
        ff.stdin.write(frame_at(fr / FPS).tobytes())
        if fr % 600 == 0:
            print("frame %d / %d" % (fr, nframes), flush=True)
    ff.stdin.close()
    ff.wait()
    if audio_dir:
        clips = [(lead + c["_vt"] + 0.3, c["_clip"]) for c in cues if c.get("_clip")]
        clips += [(c["_vt"] + 0.5, c["_clip"]) for (_, _, _, c) in cards if c.get("_clip")]
        if oclip and os.path.exists(oclip):
            clips.append((lead + total + 0.8, oclip))
        build_audio(clips, total_all, out + ".audio.wav")
        subprocess.check_call(["ffmpeg", "-y", "-loglevel", "error", "-i", out + ".video.mp4", "-i", out + ".audio.wav", "-c:v", "copy", "-af", "volume=12dB", "-c:a", "aac", "-b:a", "160k", "-shortest", out])
        os.remove(out + ".audio.wav")
        os.remove(out + ".video.mp4")
    else:
        os.replace(out + ".video.mp4", out)


def build_audio(clips, total, path):
    import array
    rate = 24000
    buf = array.array("h", [0]) * int((total + 1) * rate)
    for start_s, clip in clips:
        with wave.open(clip) as w:
            assert w.getframerate() == rate and w.getsampwidth() == 2 and w.getnchannels() == 1
            data = array.array("h")
            data.frombytes(w.readframes(w.getnframes()))
        start = int(start_s * rate)
        end = min(len(buf), start + len(data))
        buf[start:end] = data[: end - start]
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(buf.tobytes())


if __name__ == "__main__":
    main()
