"""Numbers side of the perf gate (PERF-PLAN 3.3 "Perf gate", P0-3): turns the boots of one
perf-bench.ps1 job into per-view medians, A/B verdicts, budget distances and the contention
verdict. Text only (no image work), so the runner may call it inside its timed job.

    python -I bench_report.py --plan <out>\\bench-plan.json            (what perf-bench.ps1 runs)
    python -I bench_report.py --adhoc A:<run dir>[;<foreign.csv>] B:<run dir>[;<foreign.csv>] ...
                              --set quick --read meas --out <prefix>   (re-analyse old runs)

Per window (one event=meas line, or one ring dump in ring mode):
  * dropped when the boot's sampler (gpu_sampler.ps1 `reasons`, or M2's foreign.csv) saw
    foreign load inside it, 3 s before to 1 s after (M2 analyze.py's rule); a `gpu:<name>=<SM>`
    reason under that process's override (SM_OVERRIDES, --sm-override; default Discord 40%)
    is tolerated, not foreign: Discord-only windows measured within the clean spread
    (STEWARD-1 2026-10-31, perf-squad RESUME.md 6). Tolerated windows are counted per row;
  * invalid when lit is not OK or the view's settle said UNSETTLED.
Per side and row (view x variant): medians over the clean windows of every boot of that side.
  meas (legacy) source: CPU = triPerfStats window mean and window p95, work = mean - swap;
         GPU = triGpuTimes EMA (weight 0.1). Labelled ema; never compared with ring numbers.
  ring source: CPU p50/p95/p99 and work p50/p95 per window from the per-frame ring, GPU
         sections from the GPU ring median.
A/B (when the job has two sides; A = baseline, B = candidate): delta of medians, per-pair
deltas in boot order, the gain and regression rules of 3.3, provisional noise bands of M2 4.
Budget: distance of each side's p95 to 15 ms CPU and 9 ms GPU (3.3 last line; in meas mode
the CPU p95 is the frame p95 including the swap wait and the GPU number is the EMA frame:
both labelled so).
Contended: a row with fewer clean windows than two thirds of what the set ran -> exit 3.
Writes <out>.json, <out>.md and <out>.summary.txt (one line). Exit 0, 3 contended, 4 a boot
failed, 5 sources mixed (refused).
"""
import argparse
import csv
import json
import math
import os
import re
import statistics as st
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib  # noqa: E402
import parse_meas  # noqa: E402
import parse_ring  # noqa: E402

LABEL_RE = re.compile(r"^(?P<view>.+?)_r(?P<round>\d+)__(?P<variant>.+?)(?:_w(?P<w>\d+))?$")
TIME_RE = re.compile(r"(\d+):(\d+):(\d+)(?:\.(\d+))?")

# M2 4 provisional A-vs-A spreads (ms), until the A-vs-A ABBA of EXP-X covers all 8 views.
NOISE = {"valley_0515": {"cpu_work": 0.18, "cpu_avg": 0.18, "gpu_frame": 0.22, "gpu_pass1": 0.15},
         "sunrise_ridge": {"cpu_work": 0.24, "cpu_avg": 0.24}}
NOISY_FRAME_VIEWS = {"town_night_rain"}  # gate on pass1 / ground (3.3)
# Views whose particles keep the draw count moving, so triSettleScene may run out of frames: UNSETTLED is
# counted there, not fatal. combat_fires added 2026-10-31 (BENCH-FIX): the fire and smoke cloudlets left 8
# of its 10 BATCH1 boots UNSETTLED:300 (2 settled at 99 and 154 frames), so every abba/abba-scoped/full job
# was contended on it; its frame numbers stay gated like any other view.
SETTLE_EXEMPT_VIEWS = NOISY_FRAME_VIEWS | {"combat_fires"}
CPU_BUDGET, GPU_BUDGET = 15.0, 9.0
KEEP_DIRTY = False  # --keep-dirty: count windows with foreign samples (only to reproduce old tables)
# --ring-cpu-only: a ring window without its GPU ring is valid (GPU columns stay empty). Only to re-analyse
# jobs whose GPU rings were never written (BATCH1 abba-scoped 2026-10-31: triGpuRing read after capMeas's
# triGpuTimes true had emptied the ring); a live job needs both rings.
RING_CPU_ONLY = False
# Per-process SM limits (name prefix, case-insensitive, .exe ignored): a gpu:<name>=<SM> reason below its
# limit is tolerated. Same rule as the runner (squad-queue.config.json sm_threshold_by_process) and
# gpu_sampler.ps1 / gate-lib.ps1 (-SmOverrides). --sm-override none restores the plain 15% rule.
SM_OVERRIDES = {"discord": 40}
GPU_TOKEN_RE = re.compile(r"^gpu:(.+?)=(\d+)$")
TOLERATED = {}  # path -> [(secs, token)] of tolerated samples, filled by foreign_times
METRICS_EMA = ["cpu_avg", "cpu_p95", "cpu_work", "swap", "land:obj", "gpu_frame", "gmin_frame", "gpu_world",
               "gpu_ground", "gpu_pass1", "gpu_rest", "gpu_post", "draws", "objects"]
METRICS_RING = ["cpu_p50", "cpu_p95", "cpu_p99", "cpu_max", "cpu_work", "cpu_work_p95", "gpu_frame", "gpu_frame_p95",
                "gpu_world", "gpu_ground", "gpu_pass1", "gpu_rest", "gpu_post", "draws", "objects"]


def secs(t):
    m = TIME_RE.search(t or "")
    if not m:
        return None
    s = int(m.group(1)) * 3600 + int(m.group(2)) * 60 + int(m.group(3))
    if m.group(4):
        s += int(m.group(4)) / (10 ** len(m.group(4)))
    return s


def tolerated_token(tok):
    m = GPU_TOKEN_RE.match(tok)
    if not m:
        return False
    name = re.sub(r"\.exe$", "", m.group(1), flags=re.I).lower()
    for k, lim in SM_OVERRIDES.items():
        if name.startswith(k) and int(m.group(2)) < lim:
            return True
    return False


def foreign_times(path):
    out = []
    tol = TOLERATED.setdefault(path, []) if path else []
    if not path or not os.path.exists(path):
        return out, False
    del tol[:]
    with open(path, encoding="utf-8", errors="replace") as f:
        for r in csv.DictReader(f):
            s = secs(r.get("time", ""))
            if s is None:
                continue
            toks = (r.get("reasons") or "").split()
            keep = [t for t in toks if not tolerated_token(t)]
            tol += [(s, t) for t in toks if t not in keep] + [(s, t) for t in (r.get("tolerated") or "").split()]
            if keep:
                out.append((s, " ".join(keep)))
    return out, True


def fnum(x):
    try:
        v = float(x)
        return v if math.isfinite(v) else None
    except (TypeError, ValueError):
        return None


def windows_of_boot(boot, read, runner_foreign=None):
    """All windows of one boot: list of dicts with view, variant, round, w, values, flags. Foreign
    samples come from the boot's own sampler and from the squad runner's monitor (foreign.csv)."""
    run = boot["run_dir"]
    ft, have_sampler = foreign_times(boot.get("foreign"))
    tt = list(TOLERATED.get(boot.get("foreign"), []))
    if runner_foreign:
        ft2, have2 = foreign_times(runner_foreign)
        ft += ft2
        tt += TOLERATED.get(runner_foreign, [])
        have_sampler = have_sampler or have2
    rows = []
    settle = {}
    rings = {}
    if read == "ring":
        for r in parse_ring.collect([run])[0]:
            rings.setdefault(r["label"], {}).update({("gpu_" if r["kind"] == "gpu" else "cpu_"): r})
    for lp in parse_meas.logs_of(run):
        with open(lp, encoding="utf-8", errors="replace") as f:
            for line in f:
                if "UDCAP;" not in line:
                    continue
                if "event=mode" in line:
                    m = re.search(r"view=([^;]+);.*?settle=([^;\"]+)", line)
                    if m:
                        settle[m.group(1)] = m.group(2)
                    continue
                d = parse_meas.parse_line(line)
                if not d:
                    continue
                m = LABEL_RE.match(d.get("label", ""))
                if not m:
                    continue
                w = dict(view=m.group("view"), variant=m.group("variant"), round=int(m.group("round")),
                         w=int(m.group("w") or 1), label=d["label"], time=d.get("time", ""), boot=boot["idx"],
                         side=boot["side"], lit=d.get("lit", ""), settle=settle.get(d.get("view", ""), ""),
                         source="ema" if read != "ring" else "ring")
                v = {}
                for k in ("cpu_avg", "cpu_p95", "cpu_max", "swap", "land:obj", "gpu_frame", "gmin_frame", "gpu_world",
                          "gpu_ground", "gpu_pass1", "gpu_rest", "gpu_post", "draws", "objects", "frames"):
                    v[k] = fnum(d.get(k))
                if v["cpu_avg"] is not None and v["swap"] is not None:
                    v["cpu_work"] = v["cpu_avg"] - v["swap"]
                if read == "ring":
                    rr = rings.get(d["label"], {})
                    c, g = rr.get("cpu_"), rr.get("gpu_")
                    ring = {k: v[k] for k in ("draws", "objects", "frames")}
                    if c:
                        ring.update(cpu_p50=c.get("p50"), cpu_p95=c.get("p95"), cpu_p99=c.get("p99"), cpu_max=c.get("max"),
                                    cpu_work=c.get("work_p50"), cpu_work_p95=c.get("work_p95"))
                    if g:
                        for s in ("frame", "world", "ground", "pass1", "rest", "post"):
                            if f"gpu_{s}_p50" in g:
                                ring[f"gpu_{s}"] = g[f"gpu_{s}_p50"]
                        if "gpu_frame_p95" in g:
                            ring["gpu_frame_p95"] = g["gpu_frame_p95"]
                    w["missing_ring"] = not c or (not g and not RING_CPU_ONLY)
                    w["missing_cpu_ring"] = not c
                    w["missing_gpu_ring"] = not g
                    v = ring
                w["values"] = v
                # dirty: a foreign sample inside the window (3 s margin before, 1 s after)
                end = secs(w["time"])
                dur = (v.get("frames") or 256) * max(v.get("cpu_avg") or v.get("cpu_p50") or 0,
                                                     v.get("gpu_frame") or 0, 1.0) / 1000.0
                hits = [r for (s, r) in ft if end is not None and end - dur - 3.0 <= s <= end + 1.0]
                w["dirty"] = bool(hits)
                w["tolerated"] = sorted({t for (s, t) in tt if end is not None and end - dur - 3.0 <= s <= end + 1.0})
                w["dirty_reasons"] = sorted({h.split(":")[0] for h in " ".join(hits).split()}) if hits else []
                w["sampler"] = have_sampler
                # UNSETTLED voids a window except on rain and fire views, whose particles keep the draw
                # count moving (M2 quick runs: the wet town ran out of settle frames in 3 of 6 quiet views;
                # SETTLE_EXEMPT_VIEWS); there it is counted in `unsettled` and the row says so.
                uns = w["settle"].startswith("UNSETTLED")
                w["unsettled"] = uns
                w["invalid"] = (w["lit"] not in ("OK", "")) or (uns and w["view"] not in SETTLE_EXEMPT_VIEWS) or bool(
                    w.get("missing_ring"))
                if KEEP_DIRTY:
                    w["dirty"] = False
                rows.append(w)
    return rows


def med(vals):
    vals = [x for x in vals if x is not None]
    return st.median(vals) if vals else None


def side_stats(ws, metrics):
    out = {"n": len(ws)}
    for k in metrics:
        vals = [w["values"].get(k) for w in ws]
        vals = [x for x in vals if x is not None]
        if vals:
            out[k] = st.median(vals)
            out[k + "_min"] = min(vals)
            out[k + "_max"] = max(vals)
    return out


def per_boot(ws, k):
    by = {}
    for w in ws:
        x = w["values"].get(k)
        if x is not None:
            by.setdefault(w["boot"], []).append(x)
    return {b: st.median(v) for b, v in by.items()}


def compare(view, a_ws, b_ws, metrics, read, noise):
    sa, sb = side_stats(a_ws, metrics), side_stats(b_ws, metrics)
    res = {"delta": {}, "pairs": {}, "gain": [], "regression": [], "notes": []}
    for k in metrics:
        if sa.get(k) is not None and sb.get(k) is not None:
            res["delta"][k] = sb[k] - sa[k]
            pa, pb = per_boot(a_ws, k), per_boot(b_ws, k)
            ka, kb = sorted(pa), sorted(pb)
            res["pairs"][k] = [pb[j] - pa[i] for i, j in zip(ka, kb)]
    nb = dict(noise.get(view, {}))
    cpu_key = "cpu_work"
    gpu_keys = ["gpu_pass1", "gpu_ground"] if view in NOISY_FRAME_VIEWS else ["gpu_frame", "gpu_pass1"]
    if view in NOISY_FRAME_VIEWS:
        res["notes"].append("wet town: GPU frame and rest are rain-noisy, gated on pass1 and ground (3.3)")
    # gain claimed: beyond max(2x A-vs-A spread, 0.3 CPU / 0.5 GPU), same sign in every pair
    for k, floor in [(cpu_key, 0.3)] + [(g, 0.5) for g in gpu_keys]:
        d = res["delta"].get(k)
        if d is None:
            continue
        bar = max(2 * nb.get(k, 0.0), floor)
        pairs = res["pairs"].get(k, [])
        if d < -bar and pairs and all(p < 0 for p in pairs):
            res["gain"].append(f"{k} {d:+.2f} ms (bar {bar:.2f}, {len(pairs)} pairs all faster)")
    # regression: GPU median frame +0.5 ms or +3%; CPU work +0.3 ms or +2%; p99 +2 ms (ring only).
    # In a CPU-bound row the GPU idles between frames and its clock drops, so every GPU section
    # inflates (3.3, M2 Setup): a GPU rise there is reported, not decided.
    cpu_frame = sa.get("cpu_avg") or sa.get("cpu_p50")
    cpu_bound = bool(cpu_frame and sa.get("gpu_frame") and sa["gpu_frame"] < 0.9 * cpu_frame)
    if cpu_bound:
        res["notes"].append("CPU-bound row: GPU sections are upper bounds (idle clock); GPU deltas not decided here")
    for k in gpu_keys:
        d, a = res["delta"].get(k), sa.get(k)
        if d is not None and a is not None and d > max(0.5, 0.03 * a):
            (res["notes"] if cpu_bound else res["regression"]).append(f"{k} {d:+.2f} ms (> max(0.5, 3% of {a:.2f}))")
    d, a = res["delta"].get(cpu_key), sa.get(cpu_key)
    if d is not None and a is not None and d > max(0.3, 0.02 * a):
        res["regression"].append(f"{cpu_key} {d:+.2f} ms (> max(0.3, 2% of {a:.2f}))")
    if read == "ring":
        d = res["delta"].get("cpu_p99")
        if d is not None and d > 2.0:
            res["regression"].append(f"cpu_p99 {d:+.2f} ms (> 2)")
    else:
        res["notes"].append("p99 rule needs ring mode (meas mode has window p95 only)")
    oa, ob = sa.get("objects"), sb.get("objects")
    if oa and ob and abs(ob - oa) > 0.05 * oa:
        res["notes"].append(f"objects= differs by more than 5% ({oa:.0f} -> {ob:.0f}): a faster number is void unless declared")
    return sa, sb, res


def budget(s, read):
    if not s:
        return {}
    if read == "ring":
        cpu, cl = s.get("cpu_work_p95"), "CPU work p95 (ring)"
        gpu, gl = s.get("gpu_frame_p95") or s.get("gpu_frame"), "GPU frame p95 (ring)"
    else:
        cpu, cl = s.get("cpu_p95"), "CPU frame p95 incl. swap wait (window p95, meas)"
        gpu, gl = s.get("gpu_frame"), "GPU frame (EMA, meas)"
    out = {"cpu_label": cl, "gpu_label": gl}
    if cpu is not None:
        out.update(cpu_p95=cpu, cpu_to_budget=CPU_BUDGET - cpu, cpu_median=s.get("cpu_work") if read == "ring" else s.get("cpu_avg"))
    if gpu is not None:
        out.update(gpu_p95=gpu, gpu_to_budget=GPU_BUDGET - gpu, gpu_median=s.get("gpu_frame"))
    return out


def legacy_check(rows, ref_path):
    ref = json.load(open(ref_path, encoding="utf-8"))
    tol = ref["tolerance"]
    out = []
    for rr in ref["rows"]:
        hit = next((r for r in rows if r["view"] == rr["view"] and r["variant"] == rr["variant"]), None)
        if not hit:
            out.append(dict(view=rr["view"], variant=rr["variant"], status="missing"))
            continue
        s = hit["sides"].get("B") or next(iter(hit["sides"].values()))
        checks = [("cpu_avg", tol["cpu_avg"])]
        checks += [("gpu_pass1", tol["gpu_pass1"]), ("gpu_ground", tol["gpu_pass1"])] if rr.get("noisy_gpu_frame") \
            else [("gpu_frame", tol["gpu_frame"])]
        res = dict(view=rr["view"], variant=rr["variant"], status="ok", checks=[])
        for k, t in checks:
            got, want = s.get(k), rr.get(k)
            if got is None or want is None:
                res["checks"].append(f"{k}: n/a")
                continue
            ok = abs(got - want) <= t
            res["checks"].append(f"{k} {got:.2f} vs {want:.2f} ({got - want:+.2f}, tol {t})" + ("" if ok else " OUTSIDE"))
            if not ok:
                res["status"] = "outside"
        out.append(res)
    return out


def analyse(plan):
    read = plan.get("read", "meas")
    boots = plan["boots"]
    all_ws = []
    failed = []
    for b in boots:
        if b.get("exit") not in (0, None):
            failed.append(f"boot {b['idx']} ({b['side']}) exit {b.get('exit')}")
        all_ws += windows_of_boot(b, read, plan.get("foreign_runner"))
    sides = sorted({b["side"] for b in boots})
    srcs = {w["source"] for w in all_ws}
    out = {"set": plan.get("set"), "read": read, "config": plan.get("config"), "sides": plan.get("sides", {}),
           "failed_boots": failed, "rows": [], "contended": False, "mixed": len(srcs) > 1}
    if read == "ring":
        out["ring_cpu_only"] = RING_CPU_ONLY
        out["no_cpu_ring"] = sum(1 for w in all_ws if w.get("missing_cpu_ring"))
        out["no_gpu_ring"] = sum(1 for w in all_ws if w.get("missing_gpu_ring"))
    metrics = METRICS_RING if read == "ring" else METRICS_EMA
    keys = []
    for w in all_ws:
        k = (w["view"], w["variant"])
        if k not in keys:
            keys.append(k)
    rounds_windows = plan.get("expected_windows", {})
    # expected_windows counts the main boots only (perf-bench.ps1); an abba-scoped row of the other standard
    # views comes from the one A and one B `rest` boot, so its share is scaled to the boots of its own part
    # (before 2026-10-31 BENCH-FIX every rest row asked for 8 of 12 windows from 3 and was always contended).
    boot_part = {b.get("idx"): (b.get("part") or "main") for b in plan.get("boots", [])}

    def part_boots(side, parts):
        return sum(1 for b in plan.get("boots", []) if b.get("side") == side and (b.get("part") or "main") in parts)
    for view, variant in keys:
        row = {"view": view, "variant": variant, "sides": {}, "clean": {}, "dropped": {}, "invalid": {}}
        per_side = {}
        for s in sides:
            ws = [w for w in all_ws if (w["view"], w["variant"], w["side"]) == (view, variant, s)]
            clean = [w for w in ws if not w["dirty"] and not w["invalid"]]
            per_side[s] = clean
            row["clean"][s] = len(clean)
            row["dropped"][s] = sum(1 for w in ws if w["dirty"])
            row.setdefault("tolerated", {})[s] = sum(1 for w in clean if w.get("tolerated"))
            row["invalid"][s] = sum(1 for w in ws if w["invalid"])
            if any(w.get("unsettled") for w in ws):
                row.setdefault("unsettled", {})[s] = sum(1 for w in ws if w.get("unsettled"))
            row["sides"][s] = side_stats(clean, metrics)
            row.setdefault("budget", {})[s] = budget(row["sides"][s], read)
            exp = rounds_windows.get(s) or len(ws)
            parts = {boot_part.get(w["boot"], "main") for w in ws}
            if rounds_windows.get(s) and parts and "main" not in parts and part_boots(s, {"main"}):
                exp = round(rounds_windows[s] * part_boots(s, parts) / part_boots(s, {"main"}))
            need = max(1, math.ceil(exp * 2 / 3))
            if len(clean) < need:
                row.setdefault("short", []).append(f"{s}: {len(clean)} clean of {exp} (need {need})")
                out["contended"] = True
        if "A" in per_side and "B" in per_side:
            _, _, cmp_ = compare(view, per_side["A"], per_side["B"], metrics, read, NOISE)
            row["compare"] = cmp_
        out["rows"].append(row)
    if plan.get("set") == "legacy":
        out["legacy_reconciliation"] = legacy_check(out["rows"], os.path.join(gatelib.HERE, "legacy_reference.json"))
    return out


# ----------------------------------------------------------------------------- logs, counts, boot cache

GREP = {  # always collected per boot; items read the ones they need
    "program_link_error": r"Program link error",
    "shaders_failed": r"shaders failed to build",
    "vbo_map_failed": r"VBO map failed",
    "shader_cache": r"GL33: shader cache (hit|miss|saved)",
    "world_programs": r"world programs:",
    "initgl": r"GL33: InitGL",
    "unsettled": r"UNSETTLED",
    "script_error": r"Error in expression|Script error|FAIL:",
}
LOGTS = re.compile(r"^\[(\d{4}-\d\d-\d\d) (\d\d):(\d\d):(\d\d)\.(\d{3})\]")


def log_ms(line):
    m = LOGTS.match(line)
    if not m:
        return None
    return ((int(m.group(2)) * 60 + int(m.group(3))) * 60 + int(m.group(4))) * 1000 + int(m.group(5))


def grep_boot(run):
    out = {k: [] for k in GREP}
    for lp in parse_meas.logs_of(run):
        with open(lp, encoding="utf-8", errors="replace") as f:
            for line in f:
                for k, pat in GREP.items():
                    if len(out[k]) < 40 and re.search(pat, line):
                        out[k].append(line.strip()[:400])
    return out


def stat_lines(run):
    """Every UDCAP event that is not part of the capture rig (e.g. event=landfit from an env-gated
    stats switch): (event, view) -> list of {key: number}."""
    rig = {"meas", "mode", "shot", "begin", "view", "clock", "clouds", "spawn", "setup", "window_boot", "ring",
           "ringres", "done", "config", "det", "reset", "skip_spawn", "spawn_failed", "jeep", "shell", "step"}
    res = {}
    for lp in parse_meas.logs_of(run):
        for _, d in gatelib.udcap_lines(lp):
            ev = d.get("event", "")
            if not ev or ev in rig:
                continue
            nums = {k: fnum(v) for k, v in d.items() if k not in ("event", "view") and fnum(v) is not None}
            res.setdefault((ev, re.sub(r"_r\d+$", "", d.get("view", ""))), []).append(nums)
    return res


def counts_report(plan, out):
    L = [f"# counts: {plan.get('set')} (untimed; numbers are counts, not timings)", ""]
    sides = {}
    for b in plan["boots"]:
        run = b["run_dir"]
        rows = parse_meas.collect([run])
        shots = {}
        for lp in parse_meas.logs_of(run):
            for _, d in gatelib.udcap_lines(lp):
                if d.get("event") == "shot":
                    shots[d.get("label")] = (d.get("objects"), d.get("draws"), d.get("mode"))
        sides.setdefault(b["side"], []).append(dict(boot=b["idx"], meas=rows, shots=shots, stats=stat_lines(run)))
    rep = {"set": plan.get("set"), "sides": {}}
    for s, boots in sides.items():
        rep["sides"][s] = []
        L += [f"## side {s}: {plan.get('sides', {}).get(s, {}).get('label', '')}", "",
              "| boot | window | draws | objects | counters |", "|---|---|---|---|---|"]
        for bt in boots:
            for r in bt["meas"]:
                L.append(f"| {bt['boot']} | {r.get('label')} | {r.get('draws')} | {r.get('objects')} | {r.get('counters', '')} |")
            stats = {}
            for (ev, view), recs in bt["stats"].items():
                keys = sorted({k for r in recs for k in r})
                stats[f"{ev}@{view}"] = {k: st.median([r[k] for r in recs if k in r]) for k in keys}
                stats[f"{ev}@{view}"]["lines"] = len(recs)
            rep["sides"][s].append(dict(boot=bt["boot"], shots=bt["shots"], stats=stats,
                                        meas=[{k: r.get(k) for k in ("label", "draws", "objects", "counters")} for r in bt["meas"]]))
            if stats:
                L += ["", f"stats lines of boot {bt['boot']} (median per key over the lines):", ""]
                for k, v in stats.items():
                    L.append(f"- {k}: " + ", ".join(f"{kk}={vv:g}" for kk, vv in v.items()))
            L.append("")
        L += ["shots (objects / draws / mode):", ""]
        for bt in boots:
            for lab, (ob, dr, mo) in sorted(bt["shots"].items()):
                L.append(f"- boot {bt['boot']} {lab}: {ob} / {dr} / {mo}")
        L.append("")
    if "A" in sides and "B" in sides:
        sa = sides["A"][0]["shots"]
        sb = sides["B"][0]["shots"]
        diff = [f"{k}: A {sa[k][:2]} B {sb[k][:2]}" for k in sorted(set(sa) & set(sb)) if sa[k][:2] != sb[k][:2]]
        rep["shot_counts_differ"] = diff
        L += ["## A vs B shot counts", "", ("every shot's objects and draws match" if not diff else "DIFFER:\n- " + "\n- ".join(diff)), ""]
    gatelib.write_json(out + ".counts.json", rep)
    gatelib.write_text(out + ".counts.md", "\n".join(L) + "\n")
    return rep


def bootcache_report(plan, out):
    """W1-4 / M2 8b: per build a cold boot and a warm boot sharing one user dir (program cache)."""
    rows = []
    for b in plan["boots"]:
        r = dict(side=b["side"], role=b.get("role", ""), boot=b["idx"])
        lines = []
        for lp in parse_meas.logs_of(b["run_dir"]):
            with open(lp, encoding="utf-8", errors="replace") as f:
                lines += [ln.rstrip() for ln in f]
        t_init = t_after = t_miss = t_saved = t_hit = None
        for i, ln in enumerate(lines):
            if "GL33: InitGL" in ln and t_init is None:
                t_init = log_ms(ln)
                for ln2 in lines[i + 1:]:
                    if not re.search(r"shader cache|world programs|Program link|InitGL|compil|link", ln2, re.I) and log_ms(ln2) is not None:
                        t_after = log_ms(ln2)
                        r["after_line"] = ln2[ln2.find("]  ") + 3:][:120] if "]  " in ln2 else ln2[:120]
                        break
            m = re.search(r"shader cache hit \((\d+) entries\)", ln)
            if m:
                t_hit, r["hit_entries"] = log_ms(ln), int(m.group(1))
            if "shader cache miss" in ln:
                t_miss = log_ms(ln)
            m = re.search(r"shader cache saved \((\d+) entries\)", ln)
            if m:
                t_saved, r["saved_entries"] = log_ms(ln), int(m.group(1))
            m = re.search(r"world programs: (\d+) restored, (\d+) linked, ([\d.]+) ms", ln)
            if m:
                r["restored"], r["linked"], r["world_programs_ms"] = int(m.group(1)), int(m.group(2)), float(m.group(3))
            if "Program link error" in ln:
                r["link_errors"] = r.get("link_errors", 0) + 1
        if t_init is not None and t_after is not None:
            r["initgl_ms"] = t_after - t_init
        if t_miss is not None and t_saved is not None:
            r["miss_to_saved_ms"] = t_saved - t_miss
        cache = os.path.join(b.get("user_dir") or "", "gl33_shaders.bin")
        if os.path.exists(cache):
            r["cache_bytes"] = os.path.getsize(cache)
        r["warm_linked_something"] = r.get("role") == "warm" and (r.get("linked", 0) > 0 or "miss_to_saved_ms" in r)
        rows.append(r)
    L = ["# boot cache: cold then warm boot per build, one shared user dir per build", "",
         "| side | role | InitGL to next step ms | miss to saved ms | saved entries | hit entries | restored / linked / ms | cache MB | link errors | warm linked |",
         "|---|---|---|---|---|---|---|---|---|---|"]
    for r in rows:
        L.append(f"| {r['side']} | {r['role']} | {r.get('initgl_ms', '')} | {r.get('miss_to_saved_ms', '')} | {r.get('saved_entries', '')} | "
                 f"{r.get('hit_entries', '')} | {r.get('restored', '')}/{r.get('linked', '')}/{r.get('world_programs_ms', '')} | "
                 f"{round(r['cache_bytes'] / 1e6, 2) if r.get('cache_bytes') else ''} | {r.get('link_errors', 0)} | {r['warm_linked_something']} |")
    L += ["", "InitGL to next step: the 'GL33: InitGL' line to the first later line that is not a shader cache, program or "
          "link line (M2 8b read about 383 ms cold, 356 ms warm on the tip)."]
    gatelib.write_json(out + ".bootcache.json", rows)
    gatelib.write_text(out + ".bootcache.md", "\n".join(L) + "\n")
    return rows


def fmt(x, nd=2):
    return "" if x is None else (f"{x:.{nd}f}" if isinstance(x, float) else str(x))


def markdown(rep):
    read = rep["read"]
    L = [f"# perf bench: set {rep['set']}, read {read} ({'ring medians' if read == 'ring' else 'EMA / window means, legacy'}), config {rep['config']}", ""]
    for s, info in (rep.get("sides") or {}).items():
        L.append(f"- side {s}: {info.get('label', '')} `{info.get('dist', '')}` build {info.get('build_id', '')}")
    if rep["failed_boots"]:
        L.append(f"- FAILED BOOTS: {', '.join(rep['failed_boots'])}")
    if rep["contended"]:
        L.append("- CONTENDED: some rows have too few clean windows (see `short`); re-run the job")
    if rep["mixed"]:
        L.append("- REFUSED: EMA and ring numbers mixed in one job")
    if rep.get("no_cpu_ring") or rep.get("no_gpu_ring"):
        L.append(f"- ring dumps missing: {rep.get('no_gpu_ring', 0)} window(s) without a GPU ring, "
                 f"{rep.get('no_cpu_ring', 0)} without a CPU ring (no perfring-/gpuring-<label>.csv and no "
                 "[tri] ring log line under the window's label); "
                 + ("GPU-less windows kept, GPU columns empty (--ring-cpu-only, re-analysis)" if rep.get("ring_cpu_only")
                    else "those windows count as invalid"))
    L.append("")
    cols = ["cpu_avg", "cpu_p95", "cpu_work", "swap", "gpu_frame", "gmin_frame", "gpu_ground", "gpu_pass1", "gpu_rest",
            "gpu_post", "draws", "objects"] if read != "ring" else \
        ["cpu_p50", "cpu_p95", "cpu_p99", "cpu_work", "cpu_work_p95", "gpu_frame", "gpu_frame_p95", "gpu_ground",
         "gpu_pass1", "gpu_rest", "gpu_post", "draws", "objects"]
    ntol = sum(sum((r.get("tolerated") or {}).values()) for r in rep["rows"])
    if ntol:
        L.append(f"- {ntol} clean window(s) kept with a tolerated sample inside (under its per-process SM override, "
                 f"{', '.join(f'{k} {v}%' for k, v in SM_OVERRIDES.items())}); bench.json rows[].tolerated")
        L.append("")
    L.append("| view | variant | side | clean/dropped/invalid | " + " | ".join(cols) + " | CPU to 15 | GPU to 9 |")
    L.append("|" + "---|" * (len(cols) + 6))
    for r in rep["rows"]:
        for s, st_ in r["sides"].items():
            b = r["budget"].get(s, {})
            L.append(f"| {r['view']} | {r['variant']} | {s} | {r['clean'][s]}/{r['dropped'][s]}/{r['invalid'][s]} | "
                     + " | ".join(fmt(st_.get(c), 0 if c in ("draws", "objects") else 2) for c in cols)
                     + f" | {fmt(b.get('cpu_to_budget'))} | {fmt(b.get('gpu_to_budget'))} |")
    if any("compare" in r for r in rep["rows"]):
        L += ["", "## A/B (B minus A; A = baseline, B = candidate)", "",
              "| view | variant | d cpu_work | d gpu_frame | d pass1 | pairs (cpu_work) | gain | regression | notes |",
              "|---|---|---|---|---|---|---|---|---|"]
        for r in rep["rows"]:
            c = r.get("compare")
            if not c:
                continue
            d = c["delta"]
            L.append(f"| {r['view']} | {r['variant']} | {fmt(d.get('cpu_work'))} | {fmt(d.get('gpu_frame'))} | "
                     f"{fmt(d.get('gpu_pass1'))} | {' '.join(f'{p:+.2f}' for p in c['pairs'].get('cpu_work', []))} | "
                     f"{'; '.join(c['gain'])} | {'; '.join(c['regression'])} | {'; '.join(c['notes'])} |")
    if rep.get("legacy_reconciliation"):
        L += ["", "## Legacy reconciliation against round 1 (measurements.md section 2)", ""]
        for x in rep["legacy_reconciliation"]:
            L.append(f"- {x['view']} {x['variant']}: {x['status']}" + (": " + "; ".join(x.get("checks", [])) if x.get("checks") else ""))
    L += ["", "Budget columns: distance of p95 to 15 ms CPU and 9 ms GPU. " + (
        "Meas mode: CPU p95 is the frame p95 including the swap wait, GPU is the EMA frame (not a p95)."
        if read != "ring" else "Ring mode: CPU work p95 and GPU frame p95 from the per-frame rings.")]
    return "\n".join(L) + "\n"


def summary_line(rep):
    regs = [(r["view"], r["variant"], g) for r in rep["rows"] for g in r.get("compare", {}).get("regression", [])]
    gains = [(r["view"], r["variant"], g) for r in rep["rows"] for g in r.get("compare", {}).get("gain", [])]
    s = f"set={rep['set']} read={rep['read']} rows={len(rep['rows'])}"
    s += " CONTENDED" if rep["contended"] else ""
    s += f" FAILED={len(rep['failed_boots'])}" if rep["failed_boots"] else ""
    s += " MIXED-REFUSED" if rep["mixed"] else ""
    if regs:
        s += f" regressions={len(regs)} first={regs[0][0]}/{regs[0][1]}: {regs[0][2]}"
    if gains:
        s += f" gains={len(gains)} first={gains[0][0]}/{gains[0][1]}: {gains[0][2]}"
    if rep.get("legacy_reconciliation"):
        out_ = [x for x in rep["legacy_reconciliation"] if x["status"] == "outside"]
        miss = [x for x in rep["legacy_reconciliation"] if x["status"] == "missing"]
        s += f" legacy={'OK' if not out_ else 'OUTSIDE ' + ','.join(x['view'] + '/' + x['variant'] for x in out_)}"
        if miss:
            s += f" (no row for {len(miss)} reference rows)"
    return s


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default="")
    ap.add_argument("--adhoc", nargs="*", default=[], help="SIDE:<run dir>[;<foreign csv>]")
    ap.add_argument("--set", default="adhoc")
    ap.add_argument("--read", default="meas", choices=["meas", "ring"])
    ap.add_argument("--out", default="")
    ap.add_argument("--keep-dirty", action="store_true", help="keep windows with foreign load (reproductions only)")
    ap.add_argument("--sm-override", default="", help="per-process SM limits, e.g. Discord=40;Foo=20, or none")
    ap.add_argument("--ring-cpu-only", action="store_true",
                    help="ring mode: windows without a GPU ring stay valid, GPU columns empty (re-analysis only)")
    a = ap.parse_args()
    global KEEP_DIRTY, RING_CPU_ONLY
    KEEP_DIRTY = a.keep_dirty
    RING_CPU_ONLY = a.ring_cpu_only
    if a.sm_override:
        SM_OVERRIDES.clear()
        for kv in re.split(r"[;,]", a.sm_override):
            if "=" in kv:
                k, v = kv.split("=", 1)
                SM_OVERRIDES[k.strip().lower()] = int(v)
    if a.plan:
        plan = json.load(open(a.plan, encoding="utf-8-sig"))
        base = os.path.dirname(os.path.abspath(a.plan))
        for b in plan["boots"]:
            if "boot_dir" in b:
                b.setdefault("run_dir", os.path.join(b["boot_dir"], "tri"))
                b.setdefault("foreign", os.path.join(b["boot_dir"], "gpu_samples.csv"))
        out = a.out or os.path.join(base, "bench")
    else:
        plan = {"set": a.set, "read": a.read, "config": "unknown", "boots": [], "sides": {}}
        for i, spec in enumerate(a.adhoc):
            side, _, rest = spec.partition(":")
            run, _, fcsv = rest.partition(";")
            plan["boots"].append(dict(idx=i, side=side, run_dir=run, foreign=fcsv or None, exit=0))
            plan["sides"].setdefault(side, {"label": side, "dist": "", "build_id": ""})
        out = a.out or "bench"
    greps = {str(b["idx"]): grep_boot(b["run_dir"]) for b in plan["boots"] if os.path.isdir(b.get("run_dir", ""))}
    if plan.get("set") in ("counts", "bootcache"):
        if plan["set"] == "counts":
            r = counts_report(plan, out)
            line = f"set=counts boots={len(plan['boots'])} shot-counts-differ={len(r.get('shot_counts_differ', []))}"
        else:
            rows = bootcache_report(plan, out)
            bad = [x for x in rows if x.get("link_errors") or x["warm_linked_something"]]
            line = "set=hitch/boot-cache " + " | ".join(
                f"{x['side']}-{x['role']}: initgl {x.get('initgl_ms', '?')} ms, entries {x.get('saved_entries', x.get('hit_entries', '?'))}"
                for x in rows) + (" PROBLEM: link error or warm boot linked" if bad else "")
        gatelib.write_json(out + ".grep.json", greps)
        gatelib.write_text(out + ".summary.txt", line + "\n")
        print(line)
        failed = [b for b in plan["boots"] if b.get("exit") not in (0, None)]
        return 4 if failed else 0
    rep = analyse(plan)
    rep["grep"] = greps
    gatelib.write_json(out + ".json", rep)
    gatelib.write_text(out + ".md", markdown(rep))
    line = summary_line(rep)
    gatelib.write_text(out + ".summary.txt", line + "\n")
    print(line)
    if plan.get("timed", plan.get("set") in ("quick", "abba", "abba-scoped", "full", "obs", "hitch", "legacy")) and not rep["rows"]:
        gatelib.write_text(out + ".summary.txt", line + " NO WINDOWS PARSED\n")
        return 4
    if rep["mixed"]:
        return 5
    if rep["failed_boots"]:
        return 4
    if rep["contended"]:
        return 3
    return 0


if __name__ == "__main__":
    sys.exit(main())
