"""Video 6 extras for stats.py: the audit's eight playtest numbers
(tmp/of-director-levers-audit.md, "What to measure in the next playtest")
and the round 4a surfaces: the repel ladder per town, massing, break-offs
and withdrawals (hulls after the infantry), the band on strength, the wave
surge and the stored spare hulls.  Called from stats.main with the parsed
trace; adds findings for anything that reads against the docs."""
import re
from collections import Counter, defaultdict

HELD = ("Arudy", "Sainte Marie")


def lines(info, pat):
    return [[round(e["sim"]), e["text"]] for e in info if re.search(pat, e["text"])]


def extras6(t, rows, info, series, out, findings):
    an = t.get("anomalies", [])
    end = out["sim_end"]
    launches = out["launch_lines"]
    notes = [e for e in info if e["kind"] == "note"]

    def note_sim(pat):
        for e in notes:
            if re.search(pat, e["text"]):
                return e["sim"]
        return None

    t_hold = note_sim(r"^the hold begins")
    t_rise = note_sim(r"^before the rising")
    t_free = note_sim(r"^free running from here")
    out["phases"] = {"hold": t_hold, "rising": t_rise, "free": t_free, "end": end}

    # ---- 1. time outside STRONG vs kills per hour ---------------------------------
    samp = [(r[0], r[2][2]) for r in rows if r[2][2] != "OFF"]
    tot = Counter()
    for (a, b), (a2, _) in zip(samp, samp[1:]):
        tot[b] += a2 - a
    span = sum(tot.values()) or 1
    out["band_time"] = {k: round(v) for k, v in tot.items()}
    out["outside_strong_share"] = round(1 - tot.get("STRONG", 0) / span, 4)
    lost = [(r[0], r[2][9]) for r in rows if r[2][2] != "OFF"]
    out["lost_end"] = lost[-1][1] if lost else 0
    hours = (lost[-1][0] - lost[0][0]) / 3600.0 if len(lost) > 1 else 1
    out["lost_per_hour"] = round((lost[-1][1] - lost[0][1]) / hours, 1) if lost else 0
    # the busiest hour of losses (sliding, the band's loss window)
    best = (0, 0)
    j = 0
    for i in range(len(lost)):
        while lost[i][0] - lost[j][0] > 3600:
            j += 1
        d = lost[i][1] - lost[j][1]
        if d > best[0]:
            best = (d, lost[i][0])
    out["lost_peak_hour"] = {"lost": best[0], "ending": round(best[1])}
    rep_lost = []
    for e in info:
        m = re.search(r"\[op (\d+)\] Repelled at (.+?): (\d+) lost, (\d+) going home", e["text"])
        if m:
            rep_lost.append([round(e["sim"]), int(m.group(1)), m.group(2), int(m.group(3)), int(m.group(4))])
    out["repel_losses"] = rep_lost
    out["repel_lost_total"] = sum(r[3] for r in rep_lost)
    boot = [e["text"] for e in info if "OccupierForces: active, poolStart" in e["text"]]
    out["boot_line"] = boot[0] if boot else ""
    m = re.search(r"refill ([0-9.]+)/h", out["boot_line"])
    out["refill_per_hour"] = float(m.group(1)) if m else None
    # strength vs pool (gmOccupierLedger slot 0), committed, losses, repaid
    led = []
    for r in rows:
        if len(r) > 15 and isinstance(r[15], list) and len(r[15]) >= 11:
            L = r[15]
            surge = sum(x[1] for x in L[7]) if isinstance(L[7], list) else 0
            stored = len(L[8]) if isinstance(L[8], list) else 0
            led.append([r[0], r[2][0], L[0], L[1], L[2], L[3], surge, stored, L[4], L[5], L[6], L[9], L[10]])
    out["ledger_series"] = led[:: max(1, len(led) // 400)]
    if led:
        out["strength_min"] = min(led, key=lambda x: x[2])[:3]
        out["committed_max"] = max(led, key=lambda x: x[3])[:4]
        out["unrepaid_max"] = max(led, key=lambda x: x[4] - x[5])[:6]
        out["surge_max"] = max(led, key=lambda x: x[6])[:7]
        out["stored_max"] = max(led, key=lambda x: x[7])[:8]
        out["ledger_end"] = led[-1]
        # the band must follow the strength (5.6): STRONG above 66 %, leave below 60 %
        ps = rows[0][2][1] or 1
        for x, r in zip(led, [r for r in rows if len(r) > 15 and isinstance(r[15], list) and len(r[15]) >= 11]):
            share = x[2] / float(ps) if x[1] <= ps else x[2] / float(max(ps, x[1]))
            if r[2][2] == "STRONG" and x[2] < 0.60 * ps - 1:
                findings.append(["band STRONG below 60 % strength", round(x[0]), "strength %d of %d, pool %d" % (x[2], ps, x[1])])
                break
    out["band_lines6"] = lines(info, r"OccupierForces: band \w+ -> \w+")

    # ---- 2. launch spacing --------------------------------------------------------
    lt = []
    for l in launches:
        m = re.search(r"\[op (\d+)\] (.*?) (?:to|on) (.+?) \(", l[1])
        tgt = None
        for z in [z[1] for z in t["zones"]]:
            if re.search(r" (to|on) %s \(" % re.escape(z), l[1]):
                tgt = z
        lt.append([l[0], tgt, "Wave" in l[1], "Push-out" in l[1], l[1]])
    gaps = [b[0] - a[0] for a, b in zip(lt, lt[1:])]
    out["launch_gaps"] = gaps
    out["launch_gap_stats"] = {"n": len(gaps), "min": min(gaps) if gaps else None, "median": sorted(gaps)[len(gaps) // 2] if gaps else None, "max": max(gaps) if gaps else None}
    per = defaultdict(list)
    for x in lt:
        if x[1]:
            per[x[1]].append(x[0])
    out["launch_per_town"] = {k: v for k, v in per.items()}
    out["town_gaps"] = {k: [b - a for a, b in zip(v, v[1:])] for k, v in per.items()}

    # ---- 3. urgent share -------------------------------------------------------------
    urg = [x for x in launches if ", urgent" in x[1]]
    out["urgent_launches"] = len(urg)
    out["urgent_share"] = round(len(urg) / float(len(launches)), 3) if launches else 0
    out["urgent_lines"] = urg

    # ---- 4. slow legs (the arrival rule) ------------------------------------------------
    sl = []
    for a in out["arrival_rule"]:
        m = re.search(r"needs (?:road |line x1\.5 |line )?(\d+) s", a[1])
        sl.append([a[0], int(m.group(1)) if m else None, a[1]])
    out["slow_legs"] = sl
    out["slow_legs_900_1500"] = [x for x in sl if x[1] is not None and 900 <= x[1] <= 1500]
    out["helicopter_launches"] = [l for l in launches if "Helicopter insertion" in l[1] or "helicopter insertion" in l[1] or "Mi17" in l[1]]

    # ---- 5. lethality vs reported fighters -----------------------------------------------
    lv = []
    for l in launches:
        m = re.search(r"lethality ([0-9.]+) of ([0-9.]+), (\d+) fighter\(s\) reported", l[1])
        if m:
            lv.append([l[0], float(m.group(1)), float(m.group(2)), int(m.group(3)), l[1]])
    out["lethality"] = lv
    under = [x for x in lv if x[1] + 1e-6 < x[2] and "Wave" not in x[4]]
    out["lethality_under"] = under

    # ---- 6. born ops in flight vs the cap; spare cap ----------------------------------------
    born = []
    for r in rows:
        n = sum(1 for op in r[3] if op[2] in ("DIRECTOR", "PUSHOUT") and op[1] != "STRIKE")
        born.append((r[0], n, r[2][2]))
    out["born_max"] = max(born, key=lambda x: x[1]) if born else [0, 0, ""]
    out["born_at3_samples"] = sum(1 for b in born if b[1] >= 3)
    out["born_over_cap"] = [[round(b[0]), b[1], b[2]] for b in born if b[1] > (3 if b[2] == "STRONG" else 2)]
    out["spare_cap_targets"] = sorted({re.search(r"spare cap: .*? for (.+?)[;,( ]", s[1]).group(1) if re.search(r"spare cap: .*? for (.+?)[;,( ]", s[1]) else s[1][:60] for s in out["spare_cap"]})

    # ---- 7. first wave -------------------------------------------------------------------------
    w = [x for x in out["waves"] if re.search(r"Wave 1 \(\d+\) inbound", x[1])]
    out["wave1"] = w[0] if w else None
    out["wave_lines6"] = out["waves"]

    # ---- 8. peaks ------------------------------------------------------------------------------
    out["peak_live"] = out["max_live"]
    out["peak_groups"] = out["max_groups"]

    # ---- the repel ladder per town -----------------------------------------------------------
    lad = defaultdict(list)
    for e in info:
        tx = e["text"]
        m = re.search(r"OccupierForces: (.+?): (first|second|third|\w+) repel \(", tx)
        if m:
            lad[m.group(1)].append([round(e["sim"]), m.group(2), tx])
        m = re.search(r"OccupierForces: (.+?): containment lifted", tx)
        if m:
            lad[m.group(1)].append([round(e["sim"]), "lifted", tx])
        m = re.search(r"OccupierForces: (.+?) changed hands: the repel ladder resets", tx)
        if m:
            lad[m.group(1)].append([round(e["sim"]), "reset", tx])
        m = re.search(r"OccupierForces: \[op \d+\] the repel ladder on (.+?) \(", tx)
        if m:
            lad[m.group(1)].append([round(e["sim"]), "applied", tx])
        m = re.search(r"OccupierForces: \[op \d+\] escalation strike", tx)
        if m:
            lad["(escalation strikes)"].append([round(e["sim"]), "strike", tx])
    out["ladder"] = {k: v for k, v in lad.items()}
    out["repels_rows_end"] = rows[-1][16] if rows and len(rows[-1]) > 16 else []
    out["massing"] = lines(info, r"OccupierForces: massing for ")
    out["breakoffs"] = lines(info, r"CounterAttack: \[op \d+\] Broken off at ")
    out["withdrawals"] = lines(info, r"CounterAttack: \[op \d+\] Withdrew from ")
    out["hull_after_infantry"] = lines(info, r"leaves after its infantry|holds for the infantry until")
    out["withdraw_home"] = lines(info, r"withdrawing soldier\(s\) home unseen")
    out["surge_lines"] = lines(info, r"OccupierForces: surge: ")
    out["stored_lines"] = lines(info, r"stored as a spare hull|rejoins the pool")
    out["dropped_hull_lines"] = lines(info, r"hull unit\(s\) over poolStart dropped")
    out["contained_lines"] = lines(info, r"contained: no more attacks")
    out["ends"] = lines(info, r"CounterAttack: \[op \d+\] (Broken off|Withdrew)")
    # operations on a contained town (6.3: the director spends nothing on it)
    cont = {}
    for z, ev in lad.items():
        state = False
        spans = []
        t0 = None
        for x in sorted(ev):
            if "contained" in x[2] and x[1] in ("third", "fourth", "fifth", "sixth"):
                if not state:
                    state, t0 = True, x[0]
            elif x[1] in ("lifted", "reset") and state:
                spans.append([t0, x[0]])
                state = False
        if state:
            spans.append([t0, end])
        cont[z] = spans
    out["contained_spans"] = cont
    for x in lt:
        if x[1] in cont and not x[2]:
            for a, b in cont[x[1]]:
                if a < x[0] < b:
                    findings.append(["an operation on a contained town", x[0], x[4]])
    # defenders
    df = [(r[0], r[18]) for r in rows if len(r) > 18 and isinstance(r[18], list)]
    out["defenders_end"] = df[-1][1] if df else []
    out["defenders_min"] = {}
    for _, rr in df:
        for d in rr:
            out["defenders_min"][d[0]] = min(out["defenders_min"].get(d[0], 99), d[1])
    out["director_warn_error6"] = [[round(a["sim"]), a["level"], a["text"]] for a in an if re.search(r"OccupierForces|CounterAttack|OccupierAir|ZoneRegistry|GuerrillaBase|check failed", a["text"])]
