"""Reads one run of an anim tri test and says pass or fail.

  python analyze_anim.py <run dir> --kind clips|walk|families --timing <ded_anim_timing.json> [--tri-exit N] [--keep]

Writes <run dir>\\analysis.txt (and prints it), a contact sheet, and for the
walk test the foot strip: the bottom of every second burst frame stacked in
time order, so a planted foot shows as a column that does not move.

clips: every contract move was entered, each one-shot lasted as long as its
clip, the held states held, no animation or skeleton error in the log, every
screenshot exists and is not blank.

walk: measured from the game, not from the clip.
- speed: a straight-line fit of his position against time while he walks;
- foot slide: the fixed side camera sees only his feet in the half metre
  above the ground. Each frame is compared with an empty background frame;
  the toe-side edge of each foot on the ground is followed from frame to
  frame, and a planted foot is an edge that stays. Its drift over the time
  it is down is the slide. Metres per pixel come from two calibration frames
  with him standing 40 m apart;
- the same for the turn on the spot, and his turn rate from his heading;
- the script footfall events (kj\\anim\\footfalls.sqs) against the moments
  the camera sees a foot arrive.
"""
import argparse
import glob
import json
import math
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw

SCRIPT = re.compile(r'\[\S*SCRIPT\S*\]\s+"(.*)"\s*$')
ANSI = re.compile(r"\x1b\[[0-9;]*m")
ARR = r"\[([^\]]*)\]"
ERRORS = re.compile(r"bad animation|bad bone|broken animation|missing bone|no entry .*cfgmovesdedushka|"
                    r"\.rtm.*(error|not found|cannot|empty)|uddedushka.*(not found|cannot load)|"
                    r"bad (ipol|simple) transition|no edge from", re.I)
CONTRACT = ["DedIdle", "DedWalk", "DedTurnL", "DedTurnR", "DedRoar", "DedBeamRise", "DedBeamHold", "DedBeamLower",
            "DedStomp", "DedSwat", "DedFlinch", "DedDormant"]


def find_log(run):
    hits = glob.glob(os.path.join(run, "**", "game_stdout.log"), recursive=True)
    return hits[0] if hits else None


def script_lines(log):
    out, raw = [], []
    with open(log, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            line = ANSI.sub("", line.rstrip("\n"))
            raw.append(line)
            m = SCRIPT.search(line)
            if m:
                out.append(m.group(1))
    return out, raw


def nums(text):
    return [float(v) for v in text.replace("[", " ").replace("]", " ").replace(",", " ").split() if re.match(r"^-?[\d.]+(e-?\d+)?$", v)]


def shots_in(run):
    out = {}
    for p in glob.glob(os.path.join(run, "**", "*.png"), recursive=True):
        m = re.match(r"^\d+_(.+)\.png$", os.path.basename(p))
        if m:
            out[m.group(1)] = p
    return out


def sheet(paths, out, cols=6, w=320, caption=None):
    if not paths:
        return
    thumbs = []
    for label, p in paths:
        im = Image.open(p).convert("RGB")
        im = im.resize((w, int(im.height * w / im.width)))
        ImageDraw.Draw(im).text((4, 4), label, fill=(255, 255, 0))
        thumbs.append(im)
    rows = (len(thumbs) + cols - 1) // cols
    h = thumbs[0].height
    img = Image.new("RGB", (cols * w, rows * h), (0, 0, 0))
    for i, im in enumerate(thumbs):
        img.paste(im, ((i % cols) * w, (i // cols) * h))
    img.save(out)


# ---------------------------------------------------------------------------
def analyse_clips(run, timing, lines, raw, rep):
    ok = True
    by = {c["move"]: c for c in timing["clips"]}
    anim = []          # (time, move)
    plays = []         # (time, move)
    for s in lines:
        m = re.match(r"DEDA anim ([\d.]+) (\S+)", s)
        if m:
            anim.append((float(m.group(1)), m.group(2).lower()))
        m = re.match(r"DEDA play (\S+) ([\d.]+)", s)
        if m:
            plays.append((float(m.group(2)), m.group(1)))
    seen = {m for _, m in anim}
    for move in CONTRACT:
        got = move.lower() in seen or move == "DedIdle"
        if not got:
            ok = False
        rep.append("%-13s %s" % (move, "entered" if got else "NEVER ENTERED"))
    rep.append("")
    # Each one-shot: from the playMove to the next change away from it.
    for t0, move in plays:
        c = by[move]
        later = [(t, m) for t, m in anim if t > t0 + 0.2 and m != move.lower()]
        if not later:
            continue
        t1, nxt = later[0]
        if c["looped"]:
            rep.append("%-13s loop, played from %.2f, left for %s after %.2f s" % (move, t0, nxt, t1 - t0))
            continue
        want = c["secs"]
        good = abs((t1 - t0) - want) < 0.6
        if not good:
            ok = False
        rep.append("%-13s one shot: %.2f s in game, clip %.2f s, then %s  %s" % (move, t1 - t0, want, nxt, "ok" if good else "WRONG LENGTH"))
    for s in lines:
        if s.startswith(("DEDA walkcycle", "DEDA hold kept", "DEDA dormant kept", "DEDA settled", "DEDA done")):
            rep.append(s)
    bad = [l for l in raw if ERRORS.search(l)]
    rep.append("")
    rep.append("animation or skeleton errors in the log: %d" % len(bad))
    for l in bad[:10]:
        rep.append("    " + l[-200:])
    if bad:
        ok = False
    shots = shots_in(run)
    want = [m.group(1) for s in lines for m in [re.match(r"DEDS (\S+) ", s)] if m]
    missing = [w for w in want if w not in shots]
    blank = []
    for w in want:
        if w in shots:
            a = np.asarray(Image.open(shots[w]).convert("L"), dtype=np.float32)
            if a.std() < 4.0:
                blank.append(w)
    rep.append("screenshots: %d logged, %d missing, %d blank" % (len(want), len(missing), len(blank)))
    if missing or blank or len(want) < 30:
        ok = False
    sheet([(w, shots[w]) for w in want if w in shots], os.path.join(run, "sheet.png"))
    grid = [s for s in lines if s.startswith("DEDG")]
    if grid:
        with open(os.path.join(run, "ground_grid.txt"), "w", encoding="utf-8") as fh:
            fh.write("\n".join(grid) + "\n")
    return ok


# ---------------------------------------------------------------------------
def analyse_families(run, timing, lines, raw, rep):
    """The stance families test: tri's own asserts carry it; here the log is read back for the record."""
    ok = True
    for s in lines:
        if s.startswith("DEDA ") and not s.startswith("DEDA anim"):
            rep.append(s[:200])
    anim = [m.group(2).lower() for s in lines for m in [re.match(r"DEDA anim ([\d.]+) (\S+)", s)] if m]
    rep.append("")
    rep.append("move changes: %d; moves entered: %s" % (len(anim), ", ".join(sorted(set(anim)))))
    for need in ("dedidlelow", "dedwalklow", "dedwalk"):
        if need not in anim:
            rep.append("FAIL: never entered %s" % need)
            ok = False
    if not any(m in anim for m in ("dedturnllow", "dedturnrlow")):
        rep.append("FAIL: never turned in the low family")
        ok = False
    bad = [l for l in raw if ERRORS.search(l)]
    rep.append("animation or skeleton errors in the log: %d" % len(bad))
    if bad:
        ok = False
    shots = shots_in(run)
    want = [m.group(1) for s in lines for m in [re.match(r"DEDS (\S+) ", s)] if m]
    missing = [w for w in want if w not in shots]
    rep.append("screenshots: %d logged, %d missing" % (len(want), len(missing)))
    if missing or len(want) < 4:
        ok = False
    sheet([(w, shots[w]) for w in want if w in shots], os.path.join(run, "sheet.png"), cols=4)
    return ok


def load(path):
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.int16)


def runs_of(flags, min_len, close):
    """[(first, last)] runs of True, gaps up to ``close`` closed, runs under ``min_len`` dropped."""
    idx = np.flatnonzero(flags)
    if idx.size == 0:
        return []
    out = [[int(idx[0]), int(idx[0])]]
    for i in idx[1:]:
        if i - out[-1][1] <= close + 1:
            out[-1][1] = int(i)
        else:
            out.append([int(i), int(i)])
    return [(a, b) for a, b in out if b - a + 1 >= min_len]


def follow(frames, tol, which):
    """Follow edges through the burst. frames: [(time, [(left, right), ...])] in metres.
    Returns tracks [[(time, x), ...]] of edges that stay within ``tol`` from one frame to the next."""
    tracks, live = [], []
    for fi, (t, spans) in enumerate(frames):
        edges = [(s[1] if which == "R" else s[0]) for s in spans]
        used = set()
        still = []
        for tr in live:
            best, bd = None, tol
            for j, e in enumerate(edges):
                d = abs(e - tr[-1][1])
                if j not in used and d <= bd:
                    best, bd = j, d
            if best is not None:
                used.add(best)
                tr.append((t, edges[best], fi))
                still.append(tr)
            elif fi - tr[-1][2] <= 1:          # one frame may miss it
                still.append(tr)
            else:
                tracks.append(tr)
        for j, e in enumerate(edges):
            if j not in used:
                still.append([(t, e, fi)])
        live = still
    return tracks + live


def analyse_walk(run, timing, lines, raw, rep, keep):
    ok = True
    walk = next(c for c in timing["clips"] if c["move"] == "DedWalk")
    shots = shots_in(run)
    logged = {}
    for s in lines:
        m = re.match(r"DEDS (\S+) ([\d.]+) (\S+) " + ARR + r" ([\d.\-]+)", s)
        if m:
            logged[m.group(1)] = {"t": float(m.group(2)), "move": m.group(3).lower(), "pos": nums(m.group(4)), "dir": float(m.group(5))}
    watch = []
    for s in lines:
        m = re.match(r"DEDW ([\d.]+) (\S*) " + ARR + r" ([\d.\-e]+) ([\d.\-]+)", s)
        if m:
            p = nums(m.group(3))
            watch.append((float(m.group(1)), m.group(2).lower(), p[0], p[1], p[2], float(m.group(4)), float(m.group(5))))
    marks = {}
    for s in lines:
        m = re.match(r"DEDA (walk start|walk stop|walk end|turn start|turn end|back start|back end) ([\d.]+)", s)
        if m:
            marks[m.group(1)] = float(m.group(2))
    anim = [(float(m.group(1)), m.group(2).lower()) for s in lines for m in [re.match(r"DEDA anim ([\d.]+) (\S+)", s)] if m]
    falls = []
    for s in lines:
        if s.startswith("DEDF "):
            v = nums(s)
            kind = re.search(r'"(walk|turnl|turnr|stomp)"', s)
            foot = re.search(r'"(L|R)"', s)
            if len(v) >= 4:     # [unit, foot, [x, y, 0], time, kind]: the last four numbers
                falls.append({"t": v[-1], "x": v[-4], "y": v[-3], "foot": foot.group(1) if foot else "?", "kind": kind.group(1) if kind else "?"})
    for s in lines:
        if s.startswith(("DEDC", "DEDG track", "DEDA walk", "DEDA turn", "DEDA back")):
            rep.append(s[:260])
    rep.append("")
    if not watch or "walk start" not in marks:
        rep.append("no DEDW samples or no walk start in the log")
        return False

    # ---- the camera ----------------------------------------------------------
    need = [n for n in ("bg", "cal_a", "cal_b") if n not in shots]
    if need:
        rep.append("missing frames: %s" % need)
        return False
    bg = load(shots["bg"])
    h, w_px = bg.shape[0], bg.shape[1]

    def mask_of(name):
        return (np.abs(load(shots[name]) - bg).max(axis=2) > 26)

    cal = {}
    for n in ("cal_a", "cal_b"):
        m = mask_of(n)
        rows = np.flatnonzero(m.sum(axis=1) >= 6)
        if rows.size == 0:
            rep.append("calibration frame %s shows nothing" % n)
            return False
        ground = int(rows.max())
        band = m[max(0, ground - 14):ground - 2]
        cols = np.flatnonzero(band.mean(axis=0) > 0.5)
        cal[n] = {"ground": ground, "left": int(cols.min()), "right": int(cols.max()), "pos": logged.get(n, {}).get("pos")}
    if not (cal["cal_a"]["pos"] and cal["cal_b"]["pos"]):
        rep.append("the calibration frames were not logged")
        return False
    apart = abs(cal["cal_b"]["pos"][0] - cal["cal_a"]["pos"][0])
    ppm = 0.5 * ((cal["cal_b"]["left"] - cal["cal_a"]["left"]) + (cal["cal_b"]["right"] - cal["cal_a"]["right"])) / apart
    ground = int(round(0.5 * (cal["cal_a"]["ground"] + cal["cal_b"]["ground"])))
    foot_px = cal["cal_a"]["right"] - cal["cal_a"]["left"]
    rep.append("CAMERA      %.2f px per metre (%.1f cm per pixel), ground line at row %d of %d (rows %d and %d in the two calibration frames),"
               % (ppm, 100.0 / ppm, ground, h, cal["cal_a"]["ground"], cal["cal_b"]["ground"]))
    rep.append("            his two feet side by side are %.2f m long in the picture; the frame is %.1f m wide" % (foot_px / ppm, w_px / ppm))
    if not 4.0 < ppm < 60.0:
        rep.append("            FAIL: calibration is off")
        return False
    r0 = int(round(ground - 1.0 * ppm))
    r1 = int(round(ground - 0.35 * ppm))
    mid_px = 0.5 * (0.5 * (cal["cal_a"]["left"] + cal["cal_a"]["right"]) + 0.5 * (cal["cal_b"]["left"] + cal["cal_b"]["right"]))
    mid_x = 0.5 * (cal["cal_a"]["pos"][0] + cal["cal_b"]["pos"][0])

    def spans(name):
        band = mask_of(name)[r0:r1 + 1]
        flags = band.mean(axis=0) >= 0.5
        return [(mid_x + (a - mid_px) / ppm, mid_x + (b + 1 - mid_px) / ppm) for a, b in runs_of(flags, max(3, int(0.6 * ppm)), 2)]

    def burst(prefix):
        names = sorted(n for n in shots if re.match(r"^%s_\d+$" % prefix, n) and n in logged)
        return [(logged[n]["t"], spans(n), n) for n in names]

    def strip(frames, out, every=2):
        rows = []
        top, bot = max(0, int(ground - 4.0 * ppm)), min(h, int(ground + 0.8 * ppm))
        for i, (t, _, n) in enumerate(frames):
            if i % every:
                continue
            im = Image.open(shots[n]).convert("RGB").crop((0, top, w_px, bot))
            ImageDraw.Draw(im).text((4, 2), "%.2f s" % (t - frames[0][0]), fill=(255, 255, 0))
            rows.append(im)
        if rows:
            img = Image.new("RGB", (w_px, sum(r.height for r in rows)), (0, 0, 0))
            y = 0
            for r in rows:
                img.paste(r, (0, y))
                y += r.height
            img.save(out)

    tol = 0.30

    def walk_segment(title, prefix, t_a, t_b, t_stop):
        """One stretch of walking: speed from his logged positions, planted feet from the camera."""
        good = True
        seg = [w for w in watch if t_a <= w[0] <= t_b and w[1] == "dedwalk"]
        rep.append(title)
        if len(seg) < 30:
            rep.append("    FAIL: he hardly walked (%d samples in dedwalk)" % len(seg))
            return False
        t_first, t_last = seg[0][0], seg[-1][0]
        steady = [w for w in seg if t_first + 3.0 <= w[0] <= t_last - 1.0]
        ts = np.array([w[0] for w in steady])
        xs = np.array([w[2] for w in steady])
        zs = np.array([w[3] for w in steady])
        vx = float(np.polyfit(ts, xs, 1)[0])
        vz = float(np.polyfit(ts, zs, 1)[0])
        speed = math.hypot(vx, vz)
        east = 1.0 if vx > 0 else -1.0
        line = np.polyfit(ts, xs, 1)
        resid = xs - (line[0] * ts + line[1])
        rate = speed / walk["speed_m_s"]
        rep.append("    SPEED   %.3f m/s (%.2f km/h) over %.1f s of steady walking, heading %s. The clip is %.3f m/s at full rate: the AI ran it at %.2f."
                   % (speed, speed * 3.6, ts[-1] - ts[0], "east" if east > 0 else "west", walk["speed_m_s"], rate))
        rep.append("            engine speed readout %.2f km/h; sideways drift %.2f m; surge about the straight line %.2f m (the body hanging back at each footfall)"
                   % (float(np.median([w[5] for w in steady])), abs(float(zs[-1] - zs[0])), float(resid.max() - resid.min())))
        if not (walk["slowest"] - 0.03 <= rate <= 1.03):
            rep.append("            FAIL: the rate is outside the walk's band %.2f to 1" % walk["slowest"])
            good = False
        frames = burst(prefix)
        fr = [(t, s) for t, s, _ in frames]
        toe_side, heel_side = ("R", "L") if east > 0 else ("L", "R")
        min_stay = 2.0 * min(1.0, 0.8 / rate)
        toe = [tr for tr in follow(fr, tol, toe_side) if tr[-1][0] - tr[0][0] >= min_stay and tr[0][0] > t_first + 0.5]
        toe.sort(key=lambda tr: tr[0][0])
        planted = []
        for tr in toe:
            xs_ = np.array([p[1] for p in tr])
            drift = float(xs_.max() - xs_.min())
            late = t_stop is not None and tr[-1][0] > t_stop - 0.2
            planted.append((drift, float(np.median(xs_)), tr, late))
            rep.append("    toe edge at %8.2f m east, on the ground from %6.2f to %6.2f s (%.2f s): drift %.3f m%s"
                       % (float(np.median(xs_)), tr[0][0] - t_a, tr[-1][0] - t_a, tr[-1][0] - tr[0][0], drift, "  [the stop]" if late else ""))
        steady_feet = [p for p in planted if not p[3]]
        if len(steady_feet) < 3:
            rep.append("    FAIL: only %d planted feet found (expected one per step)" % len(steady_feet))
            good = False
        else:
            worst = max(p[0] for p in steady_feet)
            med = float(np.median([p[0] for p in steady_feet]))
            pos = [p[1] for p in steady_feet]
            strides = [abs(pos[i + 2] - pos[i]) for i in range(len(pos) - 2)]
            rep.append("    SLIDE   %d planted feet: worst drift %.3f m, median %.3f m (one pixel is %.3f m)"
                       % (len(steady_feet), worst, med, 1.0 / ppm))
            if strides:
                rep.append("            ground covered by two steps, seen by the camera: %s m (the clip: %.2f m)"
                           % (", ".join("%.2f" % g for g in strides), walk["step_m"]))
                if abs(float(np.median(strides)) - walk["step_m"]) > 0.04 * walk["step_m"]:
                    rep.append("            FAIL: the steps on the ground are not the clip's steps")
                    good = False
            if worst > 0.25:
                rep.append("            FAIL: a planted foot drifts more than 0.25 m")
                good = False
        # The start: the foot he stands on while the other lifts. Its heel is the hind edge from the first frame.
        # (Seen a little from the side, the near foot hides the far heel until it lifts: take the edge that
        # appears within the first three frames and lasts longest.)
        first = [tr for tr in follow(fr, 0.65, heel_side) if tr[0][2] <= 2]
        if first:
            tr = max(first, key=lambda tr: tr[-1][0] - tr[0][0])
            tr = [p for p in tr if p[0] <= t_a + 1.6]          # before its own heel lifts
            xs_ = np.array([p[1] for p in tr])
            rep.append("    START   the standing foot's heel, seen from %.2f to %.2f s after the order, moves %+.2f m (minus is backward) while the walk blends in"
                       % (tr[0][0] - t_a, tr[-1][0] - t_a, float(xs_[-1] - xs_[0]) * east))
        if t_stop is not None:
            after = [(t, s) for t, s in fr if t >= t_stop - 0.3]
            moves = []
            for which in ("L", "R"):
                for tr in follow(after, tol, which):
                    if tr[0][2] == 0 and tr[-1][0] - tr[0][0] >= 0.75:
                        xs_ = np.array([p[1] for p in tr])
                        moves.append((float(xs_[-1] - xs_[0]) * east, tr[-1][0] - tr[0][0]))
            if moves:
                rep.append("    STOP    foot edges on the ground when he is told to stop, and how far each is then dragged: %s"
                           % ", ".join("%+.2f m in %.1f s" % mv for mv in moves))
        # Script footfalls against the camera. The camera first sees a foot "down" when its sole is in the
        # lower half of the band, about 0.2 s (at full rate) before it touches, and a frame is up to
        # 0.25 s late, half that on average.
        births = [tr[0][0] + 0.2 / rate - 0.125 for tr in toe]
        mine = [f for f in falls if f["kind"] == "walk" and t_first + 0.5 <= f["t"] <= t_last + 0.5]
        if mine and births:
            gaps = [min(abs(f["t"] - b) for b in births) for f in mine]
            signed = [min((f["t"] - b for b in births), key=abs) for f in mine]
            hit = sum(1 for g in gaps if g <= 0.4)
            rep.append("    FOOTFALLS  script events: %d; within 0.4 s of a foot the camera saw land: %d; mean offset %+.2f s, worst %.2f s"
                       % (len(mine), hit, float(np.mean(signed)), float(np.max(gaps))))
            if hit < 0.75 * len(mine):
                rep.append("            FAIL: the script footfalls do not match the feet")
                good = False
        else:
            rep.append("    FOOTFALLS  no script footfall events in this stretch: FAIL")
            good = False
        strip(frames, os.path.join(run, "%s_feet_strip.png" % prefix))
        return good

    ok = walk_segment("WALK EAST, far goal (the AI is free to use the full rate)", "w", marks["walk start"],
                      marks.get("walk end", watch[-1][0]), marks.get("walk stop")) and ok
    rep.append("")

    # ---- the turn ---------------------------------------------------------------
    tf = burst("t")
    if tf and "turn start" in marks:
        t_ts = marks["turn start"]
        t_te = marks.get("turn end", tf[-1][0])
        for s in lines:
            if s.startswith("DEDA turn by"):
                rep.append(s)
        turning = [w for w in watch if t_ts <= w[0] <= t_te and w[1] in ("dedturnl", "dedturnr")]
        flips = [a for a in anim if t_ts <= a[0] <= t_te]
        if len(turning) > 20:
            tt = np.array([w[0] for w in turning])
            dd = np.unwrap(np.radians([w[6] for w in turning]))
            core = (tt >= tt[0] + 2.0) & (tt <= tt[0] + 9.0)
            rate_t = abs(float(np.polyfit(tt[core], dd[core], 1)[0])) if core.sum() > 10 else float("nan")
            total = abs(math.degrees(dd[-1] - dd[0]))
            rep.append("TURN        %s for %.1f s, %.0f degrees; %.3f rad/s once it is under way (the clip is built for %.3f); %d move changes"
                       % (turning[0][1], tt[-1] - tt[0], total, rate_t, timing["turn_rate_rad_s"], len(flips)))
            if not (abs(rate_t - timing["turn_rate_rad_s"]) < 0.012):
                rep.append("            FAIL: he does not turn at the rate the turn clips are built for")
                ok = False
            tfr = [(t, s) for t, s, _ in tf if tt[0] + 1.5 <= t <= tt[-1] - 1.0]
            planted = []
            for which in ("L", "R"):
                for tr in follow(tfr, tol, which):
                    if tr[-1][0] - tr[0][0] >= 1.5:
                        xs_ = np.array([p[1] for p in tr])
                        planted.append((float(xs_.max() - xs_.min()), tr[-1][0] - tr[0][0]))
            if planted:
                rep.append("TURN FEET   %d foot edges stayed 1.5 s or more; worst drift %.3f m, median %.3f m"
                           % (len(planted), max(p[0] for p in planted), float(np.median([p[0] for p in planted]))))
                if max(p[0] for p in planted) > 0.5:
                    rep.append("            FAIL: a planted foot drifts more than 0.5 m in the turn")
                    ok = False
            else:
                rep.append("TURN FEET   no foot edge stayed 1.5 s in the turn: FAIL")
                ok = False
            tfalls = [f for f in falls if f["kind"] in ("turnl", "turnr") and t_ts <= f["t"] <= t_te]
            rep.append("            script footfall events in the turn: %d (the clip lands a foot every %.1f s)"
                       % (len(tfalls), next(c for c in timing["clips"] if c["move"] == "DedTurnL")["secs"] / 2.0))
        else:
            rep.append("TURN        he did not go into a turn move (%d samples): FAIL" % len(turning))
            ok = False
        strip(tf, os.path.join(run, "t_feet_strip.png"))
        rep.append("")
    if "back start" in marks:
        ok = walk_segment("WALK WEST, near goal (the AI holds him in the slow band)", "b", marks["back start"],
                          marks.get("back end", watch[-1][0]), None) and ok
        rep.append("")
    bad = [l for l in raw if ERRORS.search(l)]
    rep.append("animation or skeleton errors in the log: %d" % len(bad))
    if bad:
        ok = False
    # A few whole frames for the record, then the burst goes.
    allb = {p: burst(p) for p in ("w", "t", "b")}
    keepers = [n for p in allb for i, (_, _, n) in enumerate(allb[p]) if i % 14 == 0]
    sheet([(n, shots[n]) for n in ["cal_a", "cal_b"] + keepers], os.path.join(run, "sheet.png"), cols=4)
    if not keep:
        gone = 0
        for n, p in shots.items():
            if re.match(r"^[wtb]_\d+$", n) and n not in keepers:
                os.remove(p)
                gone += 1
        rep.append("burst frames deleted after the analysis: %d (kept %d, the strips and the sheet)" % (gone, len(keepers)))
    return ok


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--kind", required=True, choices=("clips", "walk", "families"))
    ap.add_argument("--timing", required=True)
    ap.add_argument("--tri-exit", type=int, default=0)
    ap.add_argument("--keep", action="store_true")
    args = ap.parse_args()
    timing = json.load(open(args.timing, encoding="utf-8"))
    rep = ["%s test, run %s" % (args.kind, os.path.abspath(args.run)), "tri exit code: %d" % args.tri_exit, ""]
    log = find_log(args.run)
    ok = False
    if not log:
        rep.append("no game_stdout.log under the run folder")
    else:
        lines, raw = script_lines(log)
        fails = [l for l in raw if re.search(r"FAIL:|assert.*fail", l, re.I)]
        for l in fails[:8]:
            rep.append("tri: " + l[-220:])
        if args.kind == "clips":
            ok = analyse_clips(args.run, timing, lines, raw, rep)
        elif args.kind == "families":
            ok = analyse_families(args.run, timing, lines, raw, rep)
        else:
            ok = analyse_walk(args.run, timing, lines, raw, rep, args.keep)
    rep.append("")
    rep.append("ANALYSIS %s" % ("PASS" if ok else "FAIL"))
    text = "\n".join(rep) + "\n"
    with open(os.path.join(args.run, "analysis.txt"), "w", encoding="utf-8") as fh:
        fh.write(text)
    print(text)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
