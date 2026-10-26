"""Render the side-by-side branch comparison video.

Usage:
  python render_compare.py <traceA.json> <traceB.json> <cues.json> <out.mp4>
         [--audio-dir DIR] [--preview SECONDS] [--frames-only N]

traceA/traceB come from parse_trace.py (engine output of one tri run each).
cues.json comes from make_cues.py: {"cards": [...], "cues": [...],
"table": [...]}.  A cue is {"sim", "text", "lineA", "lineB", "rate",
optional "inset": "births" | "base"}.  Both halves are drawn at the same
game time (the shared time axis); a cue's video duration is its voice clip
plus padding, or a reading-time estimate.
"""
import hashlib
import json
import math
import os
import re
import subprocess
import sys
import wave

from clock import Clock

from PIL import Image, ImageDraw, ImageFont

W, H, FPS = 1920, 1080, 30
FONT_DIR = r"C:\Windows\Fonts"


def font(name, size):
    return ImageFont.truetype(os.path.join(FONT_DIR, name), size)


F_TITLE = font("segoeuib.ttf", 26)
F_H = font("segoeuib.ttf", 19)
F_B = font("segoeui.ttf", 17)
F_S = font("segoeui.ttf", 15)
F_XS = font("segoeui.ttf", 13)
F_BIG = font("segoeuib.ttf", 26)
F_CAP = font("segoeui.ttf", 28)
F_MONO = font("consola.ttf", 15)
F_CARD = font("segoeui.ttf", 28)
F_CARDH = font("segoeuib.ttf", 50)
F_TAB = font("segoeui.ttf", 23)
F_TABH = font("segoeuib.ttf", 24)

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
SIDE_COL = {"A": (120, 190, 250), "B": (240, 190, 70)}

WX0, WX1, WY0, WY1 = 2700.0, 10300.0, 2450.0, 10250.0
TOP = 56
HDR_H = 34
MAP_Y0 = TOP + HDR_H + 4
MAP_Y1 = 808
AXIS_Y0, AXIS_Y1 = 812, 862
CAP_Y0 = 866
LEG_Y = 1068


class TF:
    """World to pixel transform for one half's map box."""

    def __init__(self, x0, y0, x1, y1):
        sx = (x1 - x0) / (WX1 - WX0)
        sy = (y1 - y0) / (WY1 - WY0)
        self.s = min(sx, sy)
        self.ox = x0 + ((x1 - x0) - (WX1 - WX0) * self.s) / 2
        self.oy = y0 + ((y1 - y0) - (WY1 - WY0) * self.s) / 2

    def __call__(self, x, y):
        return (self.ox + (x - WX0) * self.s, self.oy + (WY1 - y) * self.s)


def lerp(a, b, f):
    return a + (b - a) * f


def side_color(owner):
    if owner == "EAST":
        return OCC
    if owner == "GUER":
        return RES
    return NEU


def fmt_clock(sim):
    total = 4 * 3600 + 50 * 60 + sim
    day = 1 + int(total // 86400)
    total %= 86400
    return "Day %d  %02d:%02d" % (day, int(total // 3600), int((total % 3600) // 60))


def zname(n):
    return n.replace("Village", "Houdan") if isinstance(n, str) else n


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


def fit(draw, text, fnt, width):
    if draw.textlength(text, font=fnt) <= width:
        return text
    while text and draw.textlength(text + "...", font=fnt) > width:
        text = text[:-1]
    return text + "..."


class Scene:
    def __init__(self, trace, name):
        self.name = name
        self.trace = trace
        self.rows = trace["trace"]
        self.times = [r[0] for r in self.rows]
        self.zones = {int(z[0]): z for z in trace["zones"]}
        self.zone_by_name = {z[1]: z for z in trace["zones"]}
        hq = trace.get("hq")
        self.hq_pos = hq[1] if hq else None
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
        self.eta0 = {}
        for r in self.rows:
            for op in r[3]:
                key = (op[0], op[4], op[3])
                if op[6] in ("TRAVEL", "SCHEDULED"):
                    self.eta0[key] = max(self.eta0.get(key, 0), op[9])
        self.flips = []
        prev = None
        for r in self.rows:
            owners = {int(z[0]): z[1] for z in r[6]}
            if prev is not None:
                for i, o in owners.items():
                    if prev.get(i) != o:
                        self.flips.append((r[0], self.zones[i][1], prev.get(i), o))
            prev = owners
        self.air_from = {}
        for e in trace["info"]:
            t = e["text"]
            m = re.search(r"\[op (\d+)\] .*(?:insertion|strike by).* from (off-map|[A-Z][A-Za-z ]+?)(?= \(|;|,| to |$)", t)
            if m:
                self.air_from[int(m.group(1))] = m.group(2)
        info = sorted(trace["info"], key=lambda e: e["sim"])
        self.info = info
        # time-axis events
        self.ev = []
        for e in info:
            t = e["text"]
            m = re.search(r"OccupierForces: \[op (\d+)\] (.*?) (from|over|on) ", t)
            if m and ("squad" in t or "insertion" in t or "strike" in t.lower() or "retake" in t.lower() or "assault" in t.lower()) and " arrived" not in t and "born" not in t:
                kind = "heli" if "insertion" in t else ("strike" if "strike" in t.lower() and "squad" not in t else ("truck" if ("Motorized" in t or "truck" in t.lower()) else ("armour" if ("APC" in t or "Tank" in t or "Armed car" in t or "Mechanized" in t) else "foot")))
                self.ev.append((e["sim"], "launch", kind, t))
            elif re.search(r"\[op \d+\] born for ", t) or "born airborne" in t:
                self.ev.append((e["sim"], "birth", "", t))
        for a in trace.get("anomalies", []):
            if "deferred (live squads" in a["text"]:
                self.ev.append((a["sim"], "defer", "", a["text"]))
            elif "[gmsim] budget" in a["text"]:
                self.ev.append((a["sim"], "budget", "", a["text"]))
        for fl in self.flips:
            self.ev.append((fl[0], "flip", fl[3], fl[1]))
        self.k_lines = [(e["sim"], e["text"]) for e in info if "town(s) taken before" in e["text"]]
        self.fine = trace.get("fine", [])
        self.notes = [e for e in info if e["kind"] == "note"]
        self.supp = []

    def fine_state(self, sim, since):
        """The latest per-frame row in (since, sim]: op phases and the live census."""
        best = None
        for r in self.fine:
            if since < r[0] <= sim and r[1] in ("births", "base"):
                best = r
            elif r[0] > sim:
                break
        if best is None:
            return {}, None
        if best[1] == "births":
            return {int(o[0]): o[3] for o in best[2]}, best[3][3]
        return {int(o[0]): o[2] for o in best[5]}, None

    def note_sim(self, pat):
        for e in self.notes:
            if re.search(pat, e["text"]):
                return e["sim"]
        return None

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


def draw_triangle(d, x, y, ang, size, fill, outline):
    pts = []
    for da in (0, 140, 220):
        a = math.radians(ang + da)
        pts.append((x + math.cos(a) * size, y + math.sin(a) * size))
    d.polygon(pts, fill=fill, outline=outline)


HALVES = {"A": 0, "B": 960}


def map_tf(side):
    x0 = HALVES[side]
    return TF(x0 + 6, MAP_Y0, x0 + 600, MAP_Y1)


def build_static(scenes, sub):
    img = Image.new("RGB", (W, H), BG)
    for side, sc in scenes.items():
        x0 = HALVES[side]
        tf = map_tf(side)
        big = Image.new("RGB", (960 * 2, (MAP_Y1 - MAP_Y0) * 2), BG)
        d = ImageDraw.Draw(big)

        def P(x, y):
            px, py = tf(x, y)
            return ((px - x0) * 2, (py - MAP_Y0) * 2)

        for gx in range(3000, 10301, 1000):
            d.line([P(gx, WY0), P(gx, WY1)], fill=(20, 28, 38), width=2)
        for gy in range(3000, 10251, 1000):
            d.line([P(WX0, gy), P(WX1, gy)], fill=(20, 28, 38), width=2)
        for (src, tgt), pts in sc.graph.items():
            if (src, tgt) in sc.cross:
                continue
            d.line([P(*p) for p in pts], fill=(38, 52, 66), width=4, joint="curve")
        small = big.resize((960, MAP_Y1 - MAP_Y0), Image.LANCZOS)
        img.paste(small, (x0, MAP_Y0))
        d = ImageDraw.Draw(img)
        d.rectangle([x0 + 604, MAP_Y0, x0 + 958, MAP_Y1], fill=PANEL)
        col = SIDE_COL[side]
        d.rectangle([x0, TOP, x0 + 958, TOP + HDR_H], fill=(26, 36, 50))
        d.rectangle([x0, TOP, x0 + 6, TOP + HDR_H], fill=col)
        d.text((x0 + 16, TOP + 4), side, font=F_TITLE, fill=col)
        d.text((x0 + 46, TOP + 8), sub[side], font=F_S, fill=TXT)
        x1, y1 = tf(3000, 2700)
        x2, _ = tf(5000, 2700)
        d.line([(x1, y1), (x2, y1)], fill=DIM, width=2)
        d.text((x1, y1 + 4), "2 km", font=F_XS, fill=DIM)
    d = ImageDraw.Draw(img)
    d.line([(959, TOP), (959, MAP_Y1)], fill=LINE, width=2)
    d.rectangle([0, 0, W, TOP - 2], fill=PANEL)
    d.rectangle([0, AXIS_Y0 - 2, W, AXIS_Y1], fill=(16, 23, 32))
    d.rectangle([0, AXIS_Y1, W, H], fill=(10, 14, 20))
    d.line([(0, AXIS_Y1), (W, AXIS_Y1)], fill=LINE, width=2)
    return img


def draw_zone(d, tf, z, owner, row_z, sim):
    name, ztype = z[1], z[2]
    x, y = tf(z[5][0], z[5][1])
    base = ztype in ("OUTPOST", "SEAPORT", "AIRFIELD")
    camp = ztype == "CAMP"
    half = 11 if base else (9 if camp else 6)
    col = side_color(owner)
    heat = row_z[4] if row_z else 0
    if heat > 0 and owner == "GUER":
        r = half + 5 + heat / 8
        d.ellipse([x - r, y - r, x + r, y + r], outline=(min(255, 120 + int(heat * 1.3)), 90, 40), width=2)
    d.rectangle([x - half, y - half, x + half, y + half], fill=col if not camp else (40, 70, 50), outline=(230, 230, 230) if base else None, width=2)
    if camp:
        d.rectangle([x - half, y - half, x + half, y + half], outline=RES, width=2)
    if row_z:
        occc = row_z[5]
        sup = row_z[3]
        if base and owner == "GUER" and occc > 0:
            r = half + 6
            d.arc([x - r, y - r, x + r, y + r], -90, -90 + 3.6 * occc, fill=OCC, width=4)
        if ztype == "CITY" and (owner == "GUER" or row_z[8] > 0):
            r = half + 4
            d.arc([x - r, y - r, x + r, y + r], -90, -90 + 3.6 * sup, fill=RES, width=2)
    if row_z and len(row_z) > 12 and row_z[12] > sim:
        rr = 2000 * tf.s
        for k in range(0, 360, 12):
            d.arc([x - rr, y - rr, x + rr, y + rr], k, k + 6, fill=ORANGE, width=1)
        d.rectangle([x - half - 3, y - half - 3, x + half + 3, y + half + 3], outline=ORANGE, width=2)
    if base and row_z is not None:
        d.text((x, y), str(int(row_z[2])), font=F_XS, fill=(255, 255, 255), anchor="mm")
    lab = zname(name).replace("Le Port Harbour", "Harbour").replace("Southeast Compound", "SE Compound")
    if z[5][0] > 9000:
        d.text((x - half - 4, y - 8), lab, font=F_XS, fill=TXT if base or camp else DIM, anchor="ra")
    else:
        d.text((x + half + 4, y - 8), lab, font=F_XS, fill=TXT if base or camp else DIM)


def draw_forces(d, sc, tf, r0, r1, f, over):
    ops = r0[3]
    cas = r0[5]
    air = r0[7]
    live_pos = {ca[0]: ca for ca in cas}
    ops1 = {o[0]: o for o in r1[3]}
    cas1 = {c[0]: c for c in r1[5]}
    for op in ops:
        src, tgt, pkg, phase = op[4], op[5], op[3], op[6]
        if is_air(pkg):
            continue
        rt = sc.route(src, tgt)
        if rt and phase in ("TRAVEL", "SCHEDULED", "LIVE", "HOLDING"):
            d.line([tf(*p) for p in rt], fill=(110, 56, 54), width=2)
    for op in ops:
        oid, kind, origin, pkg, src, tgt, phase, bodies = op[:8]
        pk = pkg.replace("TRANSPORT_HELI", "HELI").replace("ATTACK_HELI", "ATK HELI")
        label = "op %d %s %d" % (oid, pk, bodies)
        ca = live_pos.get(oid)
        if phase == "LIVE" and ca is not None:
            lp = ca[7]
            c1 = cas1.get(oid)
            if c1 is not None and c1[7][0] != 0 and lp[0] != 0 and math.dist(lp[:2], c1[7][:2]) < 400:
                lp = (lerp(lp[0], c1[7][0], f), lerp(lp[1], c1[7][1], f))
            if lp[0] != 0:
                x, y = tf(lp[0], lp[1])
                airborne = any(a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 3 and a[3][2] > 5 and math.dist(a[3][:2], lp[:2]) < 60 for a in air)
                if is_air(pkg) and airborne:
                    draw_triangle(d, x, y, -90, 9, OCC, (255, 255, 255))
                else:
                    d.ellipse([x - 7, y - 7, x + 7, y + 7], fill=OCC, outline=(255, 255, 255), width=2)
                for hp in (ca[8], ca[9]):
                    if hp[0] != 0:
                        hx, hy = tf(hp[0], hp[1])
                        d.rectangle([hx - 5, hy - 3, hx + 5, hy + 3], fill=(170, 60, 55), outline=(255, 255, 255))
                d.text((x + 9, y + 3), label + " live", font=F_XS, fill=(255, 190, 180))
                continue
        if is_air(pkg):
            zt = sc.zone_by_name.get(tgt)
            zs = sc.zone_by_name.get(src)
            if zt is None:
                continue
            tx, ty = zt[5][0], zt[5][1]
            org = sc.air_from.get(int(oid))
            if org and org in sc.zone_by_name:
                zs = sc.zone_by_name[org]
            elif org == "off-map":
                zs = None
            if zs is not None and src != tgt:
                sx, sy = zs[5][0], zs[5][1]
            else:
                sx, sy = tx, WY0 - 1500
            p = op_progress(sc, op)
            o1 = ops1.get(oid)
            if o1 is not None and o1[3] == pkg:
                p = lerp(p, op_progress(sc, o1), f)
            x0, y0 = tf(sx, sy)
            x1, y1 = tf(tx, ty)
            for k in range(0, 20, 2):
                d.line([(lerp(x0, x1, k / 20), lerp(y0, y1, k / 20)), (lerp(x0, x1, (k + 1) / 20), lerp(y0, y1, (k + 1) / 20))], fill=(150, 80, 70), width=2)
            if phase == "LIVE":
                live_air = [a for a in air if a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 2]
                if live_air:
                    ax, ay = tf(live_air[0][3][0], live_air[0][3][1])
                    d.text((ax + 10, ay + 3), label + " live", font=F_XS, fill=(255, 190, 180))
                    continue
            x, y = lerp(x0, x1, p), lerp(y0, y1, p)
            ang = math.degrees(math.atan2(y1 - y0, x1 - x0))
            draw_triangle(d, x, y, ang, 10, None, OCC)
            d.text((x + 10, y + 3), label, font=F_XS, fill=(255, 190, 180))
            continue
        rt = sc.route(src, tgt)
        if not rt:
            continue
        if over.get(int(oid)) == "LIVE" and phase != "LIVE":
            p = op_progress(sc, op)
            x, y = tf(*point_at(rt, p))
            d.ellipse([x - 7, y - 7, x + 7, y + 7], fill=OCC, outline=(255, 255, 255), width=2)
            d.text((x + 9, y + 3), label + " live", font=F_XS, fill=(255, 190, 180))
            continue
        if phase == "SCHEDULED":
            pt = rt[0]
        elif phase == "HOLDING":
            pt = rt[-1]
        else:
            p = op_progress(sc, op)
            o1 = ops1.get(oid)
            if o1 is not None and o1[4] == src and o1[6] == "TRAVEL":
                p = lerp(p, op_progress(sc, o1), f)
            pt = point_at(rt, p)
        x, y = tf(*pt)
        d.ellipse([x - 7, y - 7, x + 7, y + 7], outline=OCC, width=3)
        d.text((x + 9, y + 3), label + (" holding" if phase == "HOLDING" else ""), font=F_XS, fill=(255, 190, 180))
    for a in air:
        if a[5] == 1 and isinstance(a[3], list) and len(a[3]) >= 2:
            x, y = tf(a[3][0], a[3][1])
            draw_triangle(d, x, y, -90, 8, (200, 90, 80), (255, 255, 255))


def bar(d, x, y, w, h, frac, col, marks=(), mark_col=(230, 230, 230)):
    d.rectangle([x, y, x + w, y + h], fill=(34, 44, 58))
    d.rectangle([x, y, x + w * max(0.0, min(1.0, frac)), y + h], fill=col)
    for m in marks:
        mx = x + w * m
        d.line([(mx, y - 3), (mx, y + h + 3)], fill=mark_col, width=1)


def draw_column(d, sc, side, sim, r0, r1, f, floor_on, over, live_now):
    x0 = HALVES[side] + 616
    w = 330
    forces = r0[2]
    ops = r0[3]
    intel = r0[4]
    y = MAP_Y0 + 8
    pool = lerp(forces[0], r1[2][0], f)
    start = forces[1] or 50
    d.text((x0, y), "POOL", font=F_H, fill=TXT)
    d.text((x0 + w, y + 2), "%d / %d" % (round(pool), start), font=F_B, fill=TXT, anchor="ra")
    y += 28
    marks = [0.33, 0.40, 0.60, 0.66]
    bar(d, x0, y, w, 18, pool / start, OCC, marks)
    if floor_on:
        mx = x0 + w * 0.30
        d.line([(mx, y - 6), (mx, y + 24)], fill=GOLD, width=3)
        d.text((mx, y + 22), "floor 0.3", font=F_XS, fill=GOLD, anchor="ma")
    y += 40
    band = forces[2]
    bandc = {"STRONG": OCC, "STRETCHED": ORANGE, "THIN": GOLD, "SPENT": NEU}.get(band, NEU)
    d.text((x0, y + 4), "band", font=F_S, fill=DIM)
    d.text((x0 + 60, y - 2), band, font=F_BIG, fill=bandc)
    d.text((x0 + 210, y + 4), forces[4].lower() if isinstance(forces[4], str) else "", font=F_B, fill=TXT if forces[4] == "AWAKE" else DIM)
    y += 36
    d.text((x0, y + 2), "tokens", font=F_S, fill=DIM)
    tok = int(forces[3])
    for k in range(2):
        cx = x0 + 70 + k * 30
        d.ellipse([cx, y, cx + 20, y + 20], outline=GOLD, width=3, fill=GOLD if k < tok else None)
    if forces[4] == "AWAKE" and tok < 2:
        d.text((x0 + 140, y + 2), "next in %d min" % max(0, round(forces[14] / 60.0)) if forces[2] != "SPENT" else "", font=F_S, fill=DIM)
    y += 32
    # the live census against the budget
    dg = r0[11] if len(r0) > 11 and isinstance(r0[11], list) and len(r0[11]) > 3 else None
    if dg:
        tot = live_now if live_now is not None else dg[2][3]
        gocc = dg[3][0]
        d.text((x0, y), "live AI %d of the 130 budget" % tot, font=F_S, fill=OCC if tot > 130 else TXT)
        y += 20
        bar(d, x0, y, w, 12, tot / 160.0, (150, 160, 200) if tot <= 130 else OCC, [130 / 160.0], GOLD)
        y += 18
        d.text((x0, y), "occupier groups %d of 40" % gocc, font=F_XS, fill=DIM)
        y += 22
    # k
    kl = [k for k in sc.k_lines if k[0] <= sim]
    if kl:
        m = re.search(r"(\w[\w ]*?) rose; .*?(\d+) town\(s\) taken before", kl[-1][1])
        if m:
            d.text((x0, y), "last flip: %s, k = %s" % (zname(m.group(1)), m.group(2)), font=F_S, fill=TXT)
            y += 22
    y += 4
    d.text((x0, y), "OPERATIONS", font=F_H, fill=TXT)
    y += 24
    if not ops:
        d.text((x0, y), "none in flight", font=F_S, fill=DIM)
        y += 20
    for op in ops[:6]:
        oid, kind, origin, pkg, src, tgt, phase, bodies = op[:8]
        pk = pkg.replace("TRANSPORT_HELI", "HELI").replace("ATTACK_HELI", "ATK HELI").lower()
        if len(op) > 12 and op[12]:
            pk += "+" + op[12]
        d.text((x0, y), fit(d, "op %d %s %s %s" % (oid, kind.lower(), pk, origin.lower()), F_S, w), font=F_S, fill=(255, 190, 180))
        y += 18
        eta = op[9]
        phase = over.get(int(oid), phase)
        s3 = "%s > %s %s %d%s" % (zname(src).replace("Le Port Harbour", "Harbour"), zname(tgt).replace("Le Port Harbour", "Harbour"), phase.lower(), int(bodies), (" eta %ds" % eta) if eta > 0 else "")
        d.text((x0 + 10, y), fit(d, s3, F_XS, w - 10), font=F_XS, fill=DIM)
        y += 19
    y = max(y + 4, 600)
    d.text((x0, y), "INTEL", font=F_H, fill=TXT)
    y += 24
    if not intel:
        d.text((x0, y), "no sightings", font=F_S, fill=DIM)
        y += 20
    for row in intel[:2]:
        zn = zname(row[0] or "outside zones")
        d.text((x0, y), fit(d, "%s: %d fighter(s), %d s old" % (zn, int(row[1]), int(row[2])), F_S, w), font=F_S, fill=TXT)
        y += 20
    y += 6
    recent = [fl for fl in sc.flips if fl[0] <= sim][-3:]
    if recent:
        d.text((x0, y), "OWNER CHANGES", font=F_H, fill=TXT)
        y += 24
        for (ft, zn, a, b) in reversed(recent):
            d.text((x0, y), "%s  %s" % (fmt_clock(ft)[7:], zname(zn)), font=F_XS, fill=TXT)
            d.text((x0 + 170, y), "to " + ("resistance" if b == "GUER" else "occupier"), font=F_XS, fill=side_color(b))
            y += 18


def draw_half(img, sc, side, sim, flashes, floor_on, inset):
    d = ImageDraw.Draw(img)
    tf = map_tf(side)
    i, f = sc.index(sim)
    r0 = sc.rows[i]
    r1 = sc.rows[min(i + 1, len(sc.rows) - 1)]
    d.text((HALVES[side] + 940, TOP + 9), "%s   game time %d s" % (fmt_clock(sim), sim), font=F_S, fill=GOLD, anchor="ra")
    zrows = {int(z[0]): z for z in r0[6]}
    ppos0, ppos1 = r0[8], r1[8]
    if math.dist(ppos0[:2], ppos1[:2]) > 300:
        ppos = ppos0 if f < 0.5 else ppos1
    else:
        ppos = (lerp(ppos0[0], ppos1[0], f), lerp(ppos0[1], ppos1[1], f))
    # player bubble
    px, py = tf(ppos[0], ppos[1])
    rb = 800 * tf.s
    for k in range(0, 360, 12):
        d.arc([px - rb, py - rb, px + rb, py + rb], k, k + 6, fill=(150, 170, 200), width=1)
    d.polygon([(px, py - 7), (px + 6, py), (px, py + 7), (px - 6, py)], fill=(250, 250, 250))
    for zi, z in sc.zones.items():
        zr = zrows.get(zi)
        owner = zr[1] if zr else z[3]
        draw_zone(d, tf, z, owner, zr, sim)
    for (fname, k, kind) in flashes:
        z = sc.zone_by_name.get(fname)
        if not z:
            continue
        x, y = tf(z[5][0], z[5][1])
        r = 14 + 46 * k
        col = ORANGE if kind == "strike" else (255, 255, 255)
        d.ellipse([x - r, y - r, x + r, y + r], outline=col, width=3)
        if kind == "strike" and k < 0.5:
            rr = 20 * (1 - k)
            d.ellipse([x - rr, y - rr, x + rr, y + rr], fill=(255, 200, 90))
    hq = r0[9]
    if hq and hq[0] is True and len(hq) > 5 and isinstance(hq[5], list) and len(hq[5]) >= 2:
        hx, hy = tf(hq[5][0], hq[5][1])
        if hq[3] > 0:
            d.arc([hx - 20, hy - 20, hx + 20, hy + 20], -90, -90 + 3.6 * hq[3], fill=OCC, width=5)
    over, live_now = sc.fine_state(sim, r0[0])
    draw_forces(d, sc, tf, r0, r1, f, over)
    draw_column(d, sc, side, sim, r0, r1, f, floor_on, over, live_now)
    if inset:
        draw_inset(d, sc, side, inset)


def draw_inset(d, sc, side, kind):
    x0 = HALVES[side] + 8
    y0 = 556
    w, h = 360, 248
    d.rectangle([x0, y0, x0 + w, y0 + h], fill=(12, 17, 24), outline=LINE, width=2)
    if kind == "births":
        t0 = sc.note_sim(r"the player steps between")
        rows = [r for r in sc.fine if len(r) > 1 and r[1] == "births"]
        d.text((x0 + 10, y0 + 6), "per frame: live AI after the step into the bubble", font=F_S, fill=TXT)
        if t0 is None or not rows:
            d.text((x0 + 10, y0 + 40), "no sample", font=F_S, fill=DIM)
            return
        span = (-3.0, 45.0)
        gx0, gx1, gy0, gy1 = x0 + 50, x0 + w - 16, y0 + 36, y0 + h - 70
        lo, hi = 100, 150
        def X(t):
            return gx0 + (gx1 - gx0) * (t - t0 - span[0]) / (span[1] - span[0])
        def Y(v):
            return gy1 - (gy1 - gy0) * (min(hi, max(lo, v)) - lo) / (hi - lo)
        d.line([(gx0, Y(130)), (gx1, Y(130))], fill=GOLD, width=1)
        d.text((gx0 - 6, Y(130)), "130", font=F_XS, fill=GOLD, anchor="rm")
        d.line([(X(t0), gy0), (X(t0), gy1)], fill=(150, 170, 200), width=1)
        d.text((X(t0) + 3, gy0), "step in", font=F_XS, fill=(150, 170, 200))
        pts = []
        prev = None
        born = []
        for r in rows:
            t = r[0]
            if t < t0 + span[0] or t > t0 + span[1]:
                continue
            v = r[3][3]
            if prev is not None:
                pts.append((X(t), Y(prev)))
            pts.append((X(t), Y(v)))
            prev = v
            for op in r[2]:
                if op[3] == "LIVE" and op[0] not in [b[0] for b in born]:
                    born.append((op[0], t, op[1]))
        if pts:
            pts.append((gx1, pts[-1][1]))
            d.line(pts, fill=OCC, width=2)
        mx = max((r[3][3] for r in rows if t0 + span[0] <= r[0] <= t0 + span[1]), default=0)
        d.text((x0 + w - 10, y0 + 28), "peak %d" % mx, font=F_S, fill=OCC if mx > 130 else TXT, anchor="ra")
        yy = gy1 + 8
        for (oid, t, tgt) in born[:3]:
            d.text((x0 + 12, yy), "op %d (%s) born at +%.1f s" % (oid, zname(tgt), t - t0), font=F_S, fill=(255, 190, 180))
            yy += 19
        if not born:
            d.text((x0 + 12, yy), "no birth in the window", font=F_S, fill=DIM)
    elif kind in ("base", "entry"):
        src = getattr(sc, "supp", []) if kind == "entry" else sc.fine
        rows = [r for r in src if len(r) > 1 and r[1] in ("base", "enter")]
        exits = [r for r in src if len(r) > 1 and r[1] == "exit"]
        ent = [r for r in rows if r[1] == "enter"]
        d.text((x0 + 10, y0 + 6), "per frame: Outpost capture meter at the entry", font=F_S, fill=TXT)
        if not ent:
            d.text((x0 + 10, y0 + 40), "no entry sampled", font=F_S, fill=DIM)
            return
        t0 = ent[0][0]
        span = (-8.0, 14.0) if kind == "entry" else (-12.0, 30.0)
        # a retake (owner back to the occupier) is drawn as the meter at 100
        def mv(r):
            return 100.0 if r[3] == "EAST" else r[2]
        win = [r for r in rows if t0 + span[0] <= r[0] <= t0 + span[1]]
        vals = [mv(r) for r in win]
        lo = max(0.0, min(vals) - 4) if vals else 0
        hi = min(104.0, max(vals) + 4) if vals else 100
        gx0, gx1, gy0, gy1 = x0 + 50, x0 + w - 16, y0 + 50, y0 + h - 62
        def X(t):
            return gx0 + (gx1 - gx0) * (t - t0 - span[0]) / (span[1] - span[0])
        def Y(v):
            return gy1 - (gy1 - gy0) * (v - lo) / max(1e-3, hi - lo)
        d.line([(X(t0), gy0), (X(t0), gy1)], fill=(150, 170, 200), width=1)
        d.text((X(t0) + 3, gy0 - 16), "player enters (720 m)", font=F_XS, fill=(150, 170, 200))
        ex = [r[0] for r in exits if r[0] > t0]
        if ex and ex[0] < t0 + span[1]:
            d.line([(X(ex[0]), gy0), (X(ex[0]), gy1)], fill=(110, 120, 140), width=1)
            d.text((X(ex[0]) + 3, gy1 - 14), "leaves", font=F_XS, fill=(110, 120, 140))
        prev = None
        pts = []
        for r in win:
            t = r[0]
            v = mv(r)
            if prev is not None:
                pts.append((X(t), Y(prev)))
            pts.append((X(t), Y(v)))
            prev = v
            d.ellipse([X(t) - 3, Y(v) - 3, X(t) + 3, Y(v) + 3], fill=OCC)
        if pts:
            pts.append((gx1, pts[-1][1]))
            d.line(pts, fill=OCC, width=2)
        d.text((gx0 - 6, gy0), "%.0f" % hi, font=F_XS, fill=DIM, anchor="rm")
        d.text((gx0 - 6, gy1), "%.0f" % lo, font=F_XS, fill=DIM, anchor="rm")
        after = [r for r in rows if t0 < r[0] <= t0 + 6.5]
        v_in = ent[0][2]
        flip = [r for r in after if r[3] == "EAST"]
        rises = [r for r in after if r[3] != "EAST" and r[2] > v_in]
        if flip:
            msg, col = "%.0f at entry; next tick (+%.1f s): RETAKEN, player inside" % (v_in, flip[0][0] - t0), OCC
        elif rises:
            msg, col = "%.0f at entry; next tick %.0f, player inside" % (v_in, rises[0][2]), OCC
        else:
            nxt = [r for r in after if r[2] != v_in]
            msg, col = ("%.0f at entry; next tick %.0f: no drain" % (v_in, nxt[0][2])) if nxt else ("%.0f at entry; no drain" % v_in), RES
        d.text((x0 + 12, gy1 + 12), msg, font=F_S, fill=col)


def draw_axis(d, scenes, sim, t_end, clock):
    x0, x1 = 60, W - 30
    def X(t):
        return x0 + (x1 - x0) * max(0, min(1, t / t_end))
    d.text((8, AXIS_Y0 + 2), "A", font=F_H, fill=SIDE_COL["A"])
    d.text((8, AXIS_Y0 + 24), "B", font=F_H, fill=SIDE_COL["B"])
    for k in range(0, int(t_end) + 1, 1800):
        d.line([(X(k), AXIS_Y0), (X(k), AXIS_Y1 - 4)], fill=LINE, width=1)
        d.text((X(k) + 2, AXIS_Y1 - 16), "%d min" % (k // 60), font=F_XS, fill=DIM)
    for k, (c, lab) in enumerate(zip(clock.c[1:], ["floor", "births", "step in", "", "Outpost", "entry", "", "free hour", "end"])):
        d.line([(X(c), AXIS_Y0 - 2), (X(c), AXIS_Y0 + 4)], fill=GOLD, width=2)
    for row, side in ((AXIS_Y0 + 12, "A"), (AXIS_Y0 + 32, "B")):
        sc = scenes[side]
        d.line([(x0, row), (X(clock.to_c(side, sc.times[-1])), row)], fill=(40, 52, 66), width=2)
        for (t, kind, sub, text) in sc.ev:
            tc = clock.to_c(side, t)
            if tc > sim:
                continue
            x = X(tc)
            if kind == "launch":
                col = {"foot": OCC, "truck": ORANGE, "heli": BLUE, "strike": GOLD, "armour": ORANGE}.get(sub, OCC)
                if sub == "heli":
                    draw_triangle(d, x, row, -90, 6, col, None)
                else:
                    d.ellipse([x - 4, row - 4, x + 4, row + 4], fill=col)
            elif kind == "birth":
                d.ellipse([x - 3, row - 3, x + 3, row + 3], outline=(255, 255, 255), width=1)
            elif kind == "defer":
                d.line([(x - 4, row - 4), (x + 4, row + 4)], fill=GOLD, width=2)
                d.line([(x - 4, row + 4), (x + 4, row - 4)], fill=GOLD, width=2)
            elif kind == "budget":
                d.rectangle([x - 3, row - 7, x + 3, row - 4], fill=OCC)
            elif kind == "flip":
                d.line([(x, row - 6), (x, row + 6)], fill=side_color(sub), width=1)
    d.line([(X(sim), AXIS_Y0), (X(sim), AXIS_Y1 - 2)], fill=(240, 240, 240), width=2)
    lx = 1240
    AXIS_Y1_ = AXIS_Y1
    d.text((40, LEG_Y - 18), "time axis: scenario clock, milestones aligned (gold ticks); one row per branch", font=F_XS, fill=DIM)
    for lab, col, shape in (("foot", OCC, "o"), ("truck / APC", ORANGE, "o"), ("heli", BLUE, "t"), ("birth", (255, 255, 255), "c"), ("birth refused (budget)", GOLD, "x"), ("budget WARN", OCC, "r")):
        if shape == "t":
            draw_triangle(d, lx + 4, LEG_Y - 10, -90, 5, col, None)
        elif shape == "x":
            d.line([(lx, LEG_Y - 14), (lx + 8, LEG_Y - 6)], fill=col, width=2)
            d.line([(lx, LEG_Y - 6), (lx + 8, LEG_Y - 14)], fill=col, width=2)
        elif shape == "c":
            d.ellipse([lx, LEG_Y - 14, lx + 8, LEG_Y - 6], outline=col, width=1)
        elif shape == "r":
            d.rectangle([lx + 1, LEG_Y - 12, lx + 7, LEG_Y - 9], fill=col)
        else:
            d.ellipse([lx, LEG_Y - 14, lx + 8, LEG_Y - 6], fill=col)
        d.text((lx + 12, LEG_Y - 18), lab, font=F_XS, fill=DIM)
        lx += 40 + d.textlength(lab, font=F_XS)


def render_frame(scenes, static, sim, cue, cue_k, flashes, t_end, floor_on, title, clock):
    img = static.copy()
    d = ImageDraw.Draw(img)
    d.text((16, 12), title, font=F_TITLE, fill=TXT)
    d.text((1080, 17), "scenario clock %3d min (beats aligned)" % (sim // 60), font=F_S, fill=DIM)
    d.rounded_rectangle([1600, 12, 1904, 42], radius=8, fill=(30, 60, 40))
    d.text((1752, 27), "ENGINE TRACES (two tri runs)", font=F_XS, fill=(170, 230, 180), anchor="mm")
    inset = cue.get("inset") if cue else None
    for side, sc in scenes.items():
        draw_half(img, sc, side, clock.sim(side, sim), flashes.get(side, []), floor_on.get(side, False), inset)
    d = ImageDraw.Draw(img)
    draw_axis(d, scenes, sim, t_end, clock)
    if cue is not None:
        alpha = min(1.0, cue_k * 8)
        lines = wrap(d, cue["text"], F_CAP, W - 100)
        cy = CAP_Y0 + 2
        for ln in lines[:3]:
            d.text((40, cy), ln, font=F_CAP, fill=tuple(int(c * alpha + 10 * (1 - alpha)) for c in TXT))
            cy += 36
        ly = max(cy + 6, CAP_Y0 + 86)
        for side in ("A", "B"):
            ln = cue.get("line" + side) or ""
            if not ln:
                continue
            col = SIDE_COL[side]
            d.text((40, ly), side, font=F_MONO, fill=col)
            for k, part in enumerate(wrap(d, ln, F_MONO, W - 110)[:2]):
                d.text((62, ly), part, font=F_MONO, fill=(130, 205, 150))
                ly += 19
            ly += 3
    return img


def card(spec):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    y = spec.get("y", 120)
    for kind, text in spec["lines"]:
        if kind == "h":
            d.text((W // 2, y), text, font=F_CARDH, fill=TXT, anchor="ma")
            y += 80
        elif kind == "s":
            d.text((W // 2, y), text, font=F_H, fill=GOLD, anchor="ma")
            y += 48
        elif kind in ("A", "B"):
            d.rectangle([200, y + 4, 212, y + 30], fill=SIDE_COL[kind])
            d.text((228, y), kind, font=F_CARDH, fill=SIDE_COL[kind])
            yy = y + 4
            for ln in wrap(d, text, F_CARD, 1450):
                d.text((300, yy), ln, font=F_CARD, fill=TXT)
                yy += 40
            y = max(yy, y + 64) + 18
        else:
            for ln in wrap(d, text, F_CARD, 1500):
                d.text((W // 2, y), ln, font=F_CARD, fill=TXT if kind == "p" else DIM, anchor="ma")
                y += 40
            y += 14
    return img


def table_card(table, title, foot):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    d.text((W // 2, 40), title, font=F_CARDH, fill=TXT, anchor="ma")
    xm, xa, xb = 90, 820, 1370
    y = 130
    d.text((xa, y), "A  reviewable_forces", font=F_TABH, fill=SIDE_COL["A"])
    d.text((xb, y), "B  occupier-forces", font=F_TABH, fill=SIDE_COL["B"])
    y += 44
    for row in table:
        metric, a, b = row[0], str(row[1]), str(row[2])
        la = wrap(d, a, F_TAB, 520)
        lb = wrap(d, b, F_TAB, 500)
        lm = wrap(d, metric, F_TABH, 700)
        n = max(len(la), len(lb), len(lm))
        d.line([(xm, y - 6), (W - 90, y - 6)], fill=LINE, width=1)
        for k, ln in enumerate(lm):
            d.text((xm, y + k * 30), ln, font=F_TABH, fill=TXT)
        for k, ln in enumerate(la):
            d.text((xa, y + k * 30), ln, font=F_TAB, fill=TXT)
        for k, ln in enumerate(lb):
            d.text((xb, y + k * 30), ln, font=F_TAB, fill=TXT)
        y += n * 30 + 14
    d.text((W // 2, 1040), foot, font=F_S, fill=DIM, anchor="ma")
    return img


def clip_name(text):
    return hashlib.md5(text.encode("utf-8")).hexdigest()[:12] + ".wav"


def wav_len(path):
    with wave.open(path) as w:
        return w.getnframes() / float(w.getframerate())


def build_timeline(cues, audio_dir, sim_end):
    segs = []
    vt = 0.0
    prev_sim = cues[0]["sim"] if cues else 0
    for n, c in enumerate(cues):
        clip = os.path.join(audio_dir, clip_name(c["text"])) if audio_dir else None
        if clip and os.path.exists(clip):
            dur = wav_len(clip) + 0.8
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
    ta = json.load(open(args[0]))
    tb = json.load(open(args[1]))
    spec = json.load(open(args[2]))
    out = args[3]
    audio_dir = args[args.index("--audio-dir") + 1] if "--audio-dir" in args else None
    preview = float(args[args.index("--preview") + 1]) if "--preview" in args else None
    frames_only = args[args.index("--frames-only") + 1] if "--frames-only" in args else None
    scenes = {"A": Scene(ta, "A"), "B": Scene(tb, "B")}
    sub = spec.get("sub", {"A": "origin/reviewable_forces", "B": "occupier-forces"})
    static = build_static(scenes, sub)
    clock = Clock(scenes["A"].notes, scenes["A"].times[-1], scenes["B"].notes, scenes["B"].times[-1])
    t_end = clock.end
    for side, path in (("A", "--suppA"), ("B", "--suppB")):
        if path in args:
            scenes[side].supp = json.load(open(args[args.index(path) + 1])).get("fine", [])
    cues = spec["cues"]
    segs, total = build_timeline(cues, audio_dir, t_end)
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
        cards.append((t, t + dur, card(c), c))
        t += dur
    lead = t
    outro_voice = spec.get("outro_voice")
    outro_dur = 14.0
    oclip = os.path.join(audio_dir, clip_name(outro_voice)) if (audio_dir and outro_voice) else None
    if oclip and os.path.exists(oclip):
        outro_dur = max(outro_dur, wav_len(oclip) + 2.0)
    outro = table_card(spec["table"], spec.get("table_title", "What the two runs counted"), spec.get("foot", ""))
    total_all = lead + total + outro_dur
    print("video %.1f s (cards %.1f, body %.1f, outro %.1f)" % (total_all, lead, total, outro_dur))
    if preview:
        total_all = min(total_all, preview)
    events = {}
    for side, sc in scenes.items():
        ev = [(fl[0], fl[1], "capture") for fl in sc.flips]
        for e in sc.info:
            if "] Struck " in e["text"]:
                for zn in sc.zone_by_name:
                    if zn in e["text"]:
                        ev.append((e["sim"], zn, "strike"))
                        break
        events[side] = ev
    floor_win = {}
    for side, sc in scenes.items():
        floor_win[side] = (sc.note_sim(r"lever: poolFloor 0.3"), sc.note_sim(r"lever: pool restored"))
    title = spec.get("title", "Branch comparison: the occupier director")

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
            if cue is not None and cue.get("inset"):
                cue = dict(cue)
                cue.pop("inset")
        else:
            cue, ck = c, k * (b - a)
        fl = {}
        rate = (s1 - s0) / (b - a) if b > a else 1
        for side, ev in events.items():
            lst = []
            for (es, zn, kind) in ev:
                dt_sim = sim - clock.to_c(side, es)
                if dt_sim < 0:
                    continue
                dvid = dt_sim / max(rate, 1e-3)
                if dvid < 1.6:
                    lst.append((zn, dvid / 1.6, kind))
            fl[side] = lst
        fon = {s: (w[0] is not None and w[1] is not None and w[0] <= clock.sim(s, sim) < w[1]) for s, w in floor_win.items()}
        return render_frame(scenes, static, sim, cue, ck, fl, t_end, fon, title, clock)

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
