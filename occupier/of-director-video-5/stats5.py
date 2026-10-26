"""Video 5 extras for stats.py (round 3.5): re-arms, idle while STRONG,
strike refusals, arrival births in hulls, the HQ settled line, the escort in
the launch line, push-out packages, empty groups and releases, director and
engine WARN / ERRR lines.  Called from stats.main with the parsed trace."""
import re

HQ_ZONE = "Camp"


def extras(t, rows, info, series, out, findings):
    an = t.get("anomalies", [])
    # ---- re-arm (6.3) ---------------------------------------------------------
    rearm = [[round(e["sim"]), e["text"]] for e in info if "re-armed, heat held at" in e["text"]]
    out["rearm_lines"] = rearm
    launches = out["launch_lines"]
    follow = []
    for r in rearm:
        zn = re.search(r"OccupierForces: (.+?) rested with", r[1]).group(1)
        nxt = None
        for l in launches:
            if l[0] >= r[0] - 1 and re.search(r" (to|on) %s \(" % re.escape(zn), l[1]):
                nxt = l
                break
        follow.append([r[0], zn, nxt[0] if nxt else None, (nxt[0] - r[0]) if nxt else None, nxt[1] if nxt else ""])
        if nxt is None or nxt[0] - r[0] > 600 + 60:
            findings.append(["re-arm without a launch inside one token interval", r[0], r[1] + " / next launch " + (nxt[1] if nxt else "none")])
    out["rearm_follow"] = follow
    # ---- operations per zone and their ends (repels) ---------------------------
    rep = [[round(e["sim"]), e["text"]] for e in info if re.search(r"\[op \d+\] Repelled at ", e["text"])]
    out["repels"] = rep
    out["arudy_ops"] = [l for l in launches if re.search(r" (to|on) Arudy \(", l[1])]
    out["arudy_repels"] = [r for r in rep if "Repelled at Arudy" in r[1]]
    out["arudy_notes"] = [[round(e["sim"]), e["text"]] for e in info if e["kind"] == "note" and e["text"].startswith("Arudy:")]
    # ---- idle while STRONG with a reported, rested held zone and no op on it -----
    idle = []
    cur = None
    for r in rows:
        f = r[2]
        if len(f) < 13:
            continue
        cds = {c[0] for c in f[12]} if isinstance(f[12], list) else set()
        targets = {op[5] for op in r[3]}
        intel = {row[0]: row[1] for row in r[4]}
        born = sum(1 for op in r[3] if op[2] != "WAVE" and op[1] != "STRIKE")
        zones = {}
        for z in r[6]:
            zones[int(z[0])] = z
        held = []
        for zt in t["zones"]:
            zr = zones.get(int(zt[0]))
            if zr and zr[1] == "GUER" and zt[1] != HQ_ZONE:
                held.append(zt[1])
        cands = [z for z in held if intel.get(z, 0) > 0 and z not in cds and z not in targets]
        cond = f[4] == "AWAKE" and f[2] == "STRONG" and f[3] >= 1 and born < 3 and cands
        if cond:
            if cur is None:
                cur = [r[0], r[0], cands]
            cur[1] = r[0]
        elif cur is not None:
            idle.append(cur)
            cur = None
    if cur is not None:
        idle.append(cur)
    # one director tick (60 s) plus a sample of grace is not idling
    idle_long = [[round(a), round(b), round(b - a), c] for a, b, c in idle if b - a > 120]
    out["idle_strong"] = idle_long
    out["idle_strong_min"] = round(sum(x[2] for x in idle_long) / 60.0, 1)
    for x in idle_long:
        findings.append(["idle while STRONG with a reported rested held zone", x[0], "%d s, candidates %s" % (x[2], x[3])])
    # ---- strikes and the band rule (7.3) -------------------------------------------
    out["strike_refusals"] = [[round(e["sim"]), e["text"]] for e in info if "no strike with the operation on" in e["text"]]
    out["forced_strike"] = [[round(e["sim"]), e["text"]] for e in info if e["kind"] == "note" and "FALLBACK, no director-bought strike" in e["text"]]
    # ---- births in a hull (6.5, 06414a4) -------------------------------------------
    out["hull_births"] = [[round(e["sim"]), e["text"]] for e in info if " in a hull)" in e["text"] or " in a hull;" in e["text"] or "in a hull" in e["text"]]
    out["edge_lines"] = [[round(e["sim"]), e["text"]] for e in info if "reached the town edge" in e["text"]]
    out["escort_launch"] = [l for l in launches if re.search(r"\) with [A-Za-z0-9_]+ from ", l[1])]
    out["hull_home"] = [[round(e["sim"]), e["text"]] for e in info if re.search(r"hull is home unseen|drives home to|escort .* is lost|\] the \w+ of the force on .* is lost", e["text"])]
    # ---- HQ settled line (5.5) -------------------------------------------------------
    out["hq_done"] = [[round(e["sim"]), e["text"]] for e in info if "HQ assault done" in e["text"]]
    out["hq_retaken_line"] = [[round(e["sim"]), e["text"]] for e in info if "@HQ retaken" in e["text"]]
    if out["hq_retaken_line"]:
        findings.append(["HQ fall still logs the retake line", out["hq_retaken_line"][0][0], out["hq_retaken_line"][0][1]])
    # ---- push-outs (6.4) ----------------------------------------------------------------
    po = []
    for l in launches:
        if "Push-out" in l[1]:
            m = re.search(r"Push-out: (.+?) from ", l[1])
            q = re.search(r"(road|line x1\.5|line) (\d+) s\)", l[1])
            po.append([l[0], m.group(1) if m else "?", (q.group(1) + " " + q.group(2) + " s") if q else "", l[1]])
    out["pushouts"] = po
    out["arrival_rule_pushout"] = [a for a in out["arrival_rule"] if "Push-out" in a[1] or "push-out" in a[1]]
    # ---- empty groups (contract 6, 0c43877) -------------------------------------------
    emp = [(s["sim"], s.get("empty", 0), s.get("released", 0)) for s in series]
    out["empty_peak"] = max(emp, key=lambda x: x[1]) if emp else [0, 0, 0]
    out["released_total"] = emp[-1][2] if emp else 0
    out["release_lines"] = [[round(e["sim"]), e["text"]] for e in info if "[gmsim] released" in e["text"]]
    out["empty_end"] = emp[-1][1] if emp else 0
    # ---- WARN / ERRR from the director and the engine checks ----------------------------
    dir_an = [[round(a["sim"]), a["level"], a["text"]] for a in an if re.search(r"OccupierForces|CounterAttack|OccupierAir|GuerrillaBase|CampaignEnd|ZoneRegistry|\[gmsim\]|check failed|Assert|assert", a["text"])]
    out["director_warn_error"] = dir_an
    out["errr_lines"] = [[round(a["sim"]), a["text"]] for a in an if a["level"] == "ERRR"]
