"""Count what the run did and scan it for anomalies (trace.json from
parse_trace.py).  Writes stats.json (read by render.py for the outro card and
the stability panel) and prints a human report.

Usage: python stats.py <trace.json> <stats.json>
"""
import json
import re
import sys
from collections import Counter, defaultdict

LIVE_TIMEOUT = 1200.0  # counterAttackLiveTimeout default


def main(src, dst):
    t = json.load(open(src))
    rows = t["trace"]
    info = sorted(t["info"], key=lambda e: e["sim"])
    out = {}

    # ---- operations: first sight of every director id ------------------------
    first = {}
    pkgs = defaultdict(list)
    phases = defaultdict(list)  # id -> [(sim, phase, eta, bodies, hulls, src, tgt, pkg)]
    for r in rows:
        for op in r[3]:
            oid = int(op[0])
            if oid not in first:
                first[oid] = {"sim": r[0], "kind": op[1], "origin": op[2], "package": op[3], "source": op[4], "target": op[5]}
            if not pkgs[oid] or pkgs[oid][-1] != op[3]:
                pkgs[oid].append(op[3])
            phases[oid].append((r[0], op[6], op[9], op[7], op[8], op[4], op[5], op[3], op[11] if len(op) > 11 else "", op[12] if len(op) > 12 else ""))
    by_kind = Counter()
    by_pkg = Counter()
    for oid, f in first.items():
        by_kind["%s/%s" % (f["kind"], f["origin"])] += 1
        by_pkg[f["package"]] += 1
    replans = {oid: p for oid, p in pkgs.items() if len(p) > 1}
    escorts = sorted({(oid, ph[-1]) for oid, lst in phases.items() for ph in lst if ph[-1]})
    out["ops"] = {"count": len(first), "by_kind": dict(by_kind), "by_package": dict(by_pkg), "replans": {str(k): v for k, v in replans.items()},
                  "escorts": [[o, e] for o, e in escorts], "first": {str(k): v for k, v in first.items()}}

    # launch lines (the director's own words)
    launches = [e for e in info if re.search(r"OccupierForces: \[op \d+\] ", e["text"]) and re.search(r" from .* (to|on) ", e["text"])]
    out["launch_lines"] = [[round(e["sim"]), e["text"]] for e in launches]

    # ---- arrivals, births, cancels ---------------------------------------------
    arr = Counter()
    arr_lines = []
    for e in info:
        m = re.search(r"\[op (\d+)\] arrived (abstract|live) at", e["text"])
        if m:
            arr[m.group(2)] += 1
            arr_lines.append([round(e["sim"]), e["text"]])
    births = Counter()
    birth_lines = []
    for e in info:
        m = re.search(r"\[op (\d+)\] born for .*\(spawnReason ([a-z]+)\)", e["text"])
        if m:
            births[m.group(2)] += 1
            birth_lines.append([round(e["sim"]), e["text"]])
    cancels = [[round(e["sim"]), e["text"]] for e in info if re.search(r"cancelled unborn|failed twice; cancelled unseen|fell before the force left; cancelled|the force is cancelled", e["text"])]
    out["arrivals"] = dict(arr)
    out["arrival_lines"] = arr_lines
    out["births"] = dict(births)
    out["birth_lines"] = birth_lines
    out["cancels"] = cancels

    # ---- census series ----------------------------------------------------------
    series = []
    for r in rows:
        d = r[11] if len(r) > 11 else None
        if not d or not isinstance(d, list) or len(d) < 11:
            continue
        ops = r[3]
        live_ops = sum(1 for op in ops if op[6] == "LIVE" or (len(op) > 8 and op[8] > 0))
        wave_recs = sum(1 for op in ops if op[2] == "WAVE")
        series.append({
            "sim": r[0],
            "occ": d[2][0], "res": d[2][1], "civ": d[2][2], "total": d[2][3],
            "gocc": d[3][0], "gres": d[3][1], "gciv": d[3][2], "gmax": d[3][3],
            "far": d[10][0] if isinstance(d[10], list) else 0,
            "far_parts": d[10] if isinstance(d[10], list) else [],
            "live_ops": live_ops, "ops_cap": 2 + wave_recs, "ops": len(ops),
            "gm_p95": d[7][1], "frame_p95": d[8][1], "frame_max": d[8][2],
            "pool": r[2][0], "band": r[2][2],
        })
    out["series"] = series

    def mx(k):
        return max((s[k] for s in series), default=0)

    out["max_groups"] = {"occ": mx("gocc"), "res": mx("gres"), "civ": mx("gciv"), "maxSide": mx("gmax")}
    out["max_live"] = {"occ": mx("occ"), "res": mx("res"), "civ": mx("civ"), "total": mx("total")}
    out["max_far"] = mx("far")
    out["max_live_ops"] = mx("live_ops")
    fp = sorted(s["frame_p95"] for s in series)
    out["frame_p95"] = {"median": fp[len(fp) // 2] if fp else 0, "max": fp[-1] if fp else 0}
    out["sim_end"] = rows[-1][0] if rows else 0

    # ---- anomaly scan -----------------------------------------------------------
    findings = []
    # band edges and flicker
    bands = [e for e in info if re.search(r"OccupierForces: band \w+ -> \w+", e["text"])]
    out["band_lines"] = [[round(e["sim"], 1), e["text"]] for e in bands]
    for a, b in zip(bands, bands[1:]):
        if b["sim"] - a["sim"] < 15:
            findings.append(["band flicker", round(a["sim"]), a["text"] + " / " + b["text"]])
    # live phase too long, eta stuck
    for oid, lst in phases.items():
        live = [p for p in lst if p[1] == "LIVE"]
        if live and live[-1][0] - live[0][0] > LIVE_TIMEOUT + 120:
            findings.append(["live too long", round(live[0][0]), "op %d LIVE %d s (%s -> %s)" % (oid, live[-1][0] - live[0][0], lst[0][5], lst[0][6])])
        hold = [p for p in lst if p[1] == "HOLDING"]
        if hold and hold[-1][0] - hold[0][0] > 1800:
            findings.append(["holding long", round(hold[0][0]), "op %d HOLDING %d s at %s" % (oid, hold[-1][0] - hold[0][0], lst[0][6])])
        trav = [p for p in lst if p[1] in ("TRAVEL", "SCHEDULED")]
        # eta that does not move for 600 s while travelling
        for i in range(len(trav)):
            j = i
            while j + 1 < len(trav) and trav[j + 1][2] == trav[i][2] and trav[i][2] > 0:
                j += 1
            if trav[j][0] - trav[i][0] > 600:
                findings.append(["eta stalled", round(trav[i][0]), "op %d %s eta %d s unchanged for %d s (%s -> %s)" % (oid, trav[i][1], trav[i][2], trav[j][0] - trav[i][0], trav[i][5], trav[i][6])])
                break
        out.setdefault("op_phases", {})[str(oid)] = [[round(p[0]), p[1], p[2], p[3], p[4], p[7]] for k, p in enumerate(lst) if k == 0 or p[1] != lst[k - 1][1] or p[7] != lst[k - 1][7]]
    # landings
    for e in info:
        m = re.search(r"down (\d+) m from its LZ", e["text"])
        if m and int(m.group(1)) > 100:
            findings.append(["landing off", round(e["sim"]), e["text"]])
    # leftover growth
    lo = [(s["sim"], s["far_parts"][3] if len(s["far_parts"]) > 3 else 0) for s in series]
    if lo:
        peak = max(lo, key=lambda x: x[1])
        tail = lo[-1][1]
        out["leftover"] = {"peak": peak[1], "peak_sim": peak[0], "end": tail}
    # warn / error lines from the engine
    an = Counter()
    first_an = {}
    for a in t.get("anomalies", []):
        key = re.sub(r"\d+(\.\d+)?", "N", a["text"])[:160]
        an[(a["level"], key)] += 1
        first_an.setdefault((a["level"], key), (round(a["sim"]), a["text"]))
    out["warn_error"] = [[lvl, n, first_an[(lvl, k)][0], first_an[(lvl, k)][1]] for (lvl, k), n in an.most_common()]

    # ---- comparison extras (branch comparison video) ----------------------
    notes = [e for e in info if e["kind"] == "note"]

    def note_sim(pat):
        for e in notes:
            if re.search(pat, e["text"]):
                return e["sim"]
        return None

    # k: the "N town(s) taken before" in every flip answer line
    out["k_lines"] = [[round(e["sim"], 1), e["text"]] for e in info if re.search(r"rose; .*\((?:chance [0-9.]+, )?\d+ town\(s\) taken before|rose; the occupier director takes it", e["text"])]
    # births refused by a budget (the deferral WARN names the census)
    defer = []
    for a in t.get("anomalies", []):
        m = re.search(r"force on (.+?) deferred \(live squads (\d+)/(\d+), live AI (\d+)\+(\d+)/(\d+), occupier groups (\d+)\+(\d+)/(\d+), born ops (\d+)/(\d+), live hulls (\d+)/(\d+)\)", a["text"])
        if m:
            g = [int(x) for x in m.groups()[1:]]
            why = []
            if g[0] >= g[1]:
                why.append("live squads")
            if g[5] > 0 and g[2] + g[3] > g[4]:
                why.append("bodies")
            if g[8] > 0 and g[6] + g[7] > g[8]:
                why.append("groups")
            if g[9] >= g[10]:
                why.append("born ops")
            defer.append([round(a["sim"]), m.group(1), ",".join(why) or "other", a["text"]])
    out["deferred"] = defer
    out["deferred_budget"] = [d for d in defer if "bodies" in d[2] or "groups" in d[2]]
    out["budget_warns"] = [[round(a["sim"]), a["text"]] for a in t.get("anomalies", []) if "[gmsim] budget" in a["text"]]
    # pool floor beat
    out["floor_notes"] = [[round(e["sim"]), e["text"]] for e in notes if re.search(r"at the floor|pool restored|pool forced to 15", e["text"])]
    t_floor = note_sim(r"lever: poolFloor 0.3")
    t_rest = note_sim(r"lever: pool restored")
    out["floor_launches"] = [l for l in out["launch_lines"] if t_floor is not None and t_rest is not None and t_floor <= l[0] <= t_rest]
    out["refused_lines"] = [[round(e["sim"]), e["text"]] for e in info if "refused" in e["text"] and "OccupierForces" in e["text"]]
    # the births beat: fine rows
    fine = t.get("fine", [])
    out["fine_births"] = [r for r in fine if len(r) > 1 and r[1] == "births"]
    out["fine_base"] = [r for r in fine if len(r) > 1 and r[1] in ("base", "enter")]
    # the free-running hour: launches after the stretch starts
    t_str = note_sim(r"the cells in the empty towns move on")
    out["stretch_start"] = t_str
    if t_str is not None:
        t_rise = note_sim(r'lever: \["La Trinite"') or t_str
        sl = [l for l in out["launch_lines"] if t_rise - 1 <= l[0] <= t_str + 3600]
        out["stretch_launches"] = sl
        sp = Counter()
        for l in sl:
            m = re.search(r"\] (Push-out: \w+|Foot|Motorized|APC|Tank|Armed car|Helicopter insertion|Mechanized|Column|Base retake \(\w+)", l[1])
            sp[m.group(1).replace("Push-out:", "push-out").replace("Motorized", "truck").replace("Helicopter insertion", "heli insertion") if m else "other"] += 1
        out["stretch_packages"] = dict(sp)
    # ops by package with origin, from first sight
    pk = Counter()
    for oid, f in first.items():
        pk["%s %s" % (f["origin"].lower(), f["package"].lower())] += 1
    out["ops_origin_pkg"] = dict(pk)
    out["findings"] = findings
    json.dump(out, open(dst, "w"), indent=1)

    print("sim end %.0f s, %d rows" % (out["sim_end"], len(rows)))
    print("ops", out["ops"]["count"], out["ops"]["by_kind"], out["ops"]["by_package"], "replans", out["ops"]["replans"], "escorts", out["ops"]["escorts"])
    print("arrivals", out["arrivals"], "births", out["births"], "cancels", len(cancels))
    print("max groups", out["max_groups"], "max live", out["max_live"], "max far", out["max_far"], "max live ops", out["max_live_ops"], "frame p95", out["frame_p95"])
    print("leftover", out.get("leftover"))
    for b in out["band_lines"]:
        print("band", b)
    for f in findings:
        print("FINDING", f)
    for w in out["warn_error"]:
        print("LOG", w[0], w[1], "x, first at", w[2], ":", w[3][:200])


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
