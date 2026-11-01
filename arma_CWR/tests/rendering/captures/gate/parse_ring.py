"""Per-frame ring dumps: p50/p95/p99/max of the CPU frame (and of each GPU section when a GPU
ring is present), spike count and spacing. Writes <out>.md, <out>.json and <out>_frames.csv.

    python -I parse_ring.py <run dir> [...] --out <prefix> [--csv-glob "**/*ring*.csv"]

Moved from tmp/perf-audit/reports/measure/parse_ring.py on 2026-10-31 and extended.
Two input forms:

1. Log form (the round-1 / M2 scratch verb): a UDCAP `event=ring;label=<L>` line followed by
   a `[tri] triPerfRing a,b,c,d,e;a,b,c,d,e;...` line. Per frame: total, drw:obj, swap,
   land:obj, drw:land (ms).
2. CSV form (INFRA-C1's triPerfRing / triGpuRing, which write CSV under TRI_OUTPUT_DIR; the
   exact file names and columns are INFRA-C1's to fix): any CSV matching --csv-glob with a
   header row. The CPU frame column is the first of total, frame, cpu, cpu_frame; GPU
   columns are any of frame, world, shadow, ground, pass1, rest, post (prefixed gpu_ when the
   file name contains "gpu"). The label is a `label` column if present, else the file stem.

All rows carry source=ring (per-frame medians). Never compare them with parse_meas.py's
source=ema rows (PERF-PLAN 3.1 P0-3).
"""
import argparse
import csv
import json
import re
import statistics as st
from pathlib import Path

CPU_COLS = ("total", "frame", "cpu", "cpu_frame", "cpu_ms")
GPU_COLS = ("frame", "world", "shadow", "ground", "pass1", "rest", "post")


def pct(v, p):
    s = sorted(v)
    if not s:
        return float("nan")
    k = (len(s) - 1) * p / 100.0
    lo = int(k)
    hi = min(lo + 1, len(s) - 1)
    return s[lo] + (s[hi] - s[lo]) * (k - lo)


def summarize(tot):
    """Distribution summary of one ring (list of ms). Spike = frame > 1.5 x median + 2 ms."""
    if not tot:
        return {}
    med = st.median(tot)
    spikes = [i for i, t in enumerate(tot) if t > med * 1.5 + 2]
    gaps = [b - c for b, c in zip(spikes[1:], spikes[:-1])]
    normal = [t for t in tot if t <= med * 1.5 + 2] or tot
    return dict(n=len(tot), mean=st.mean(tot), p50=med, p95=pct(tot, 95), p99=pct(tot, 99), max=max(tot),
                nspikes=len(spikes), spike_mean=st.mean([tot[i] for i in spikes]) if spikes else 0.0,
                gap=st.median(gaps) if gaps else 0, normal_mean=st.mean(normal))


def from_logs(rd):
    rows, frames = [], []
    for lp in Path(rd).rglob("game_stdout.log"):
        label = None
        for line in lp.read_text(encoding="utf-8", errors="replace").splitlines():
            m = re.search(r"event=ring;label=([^\";]+)", line)
            if m:
                label = m.group(1)
                continue
            m = re.search(r"triPerfRing (.*)$", line)
            if m and label:
                fr = [tuple(float(x) for x in t.split(",")) for t in m.group(1).strip().split(";") if t.strip()]
                tot = [f[0] for f in fr]
                s = summarize(tot)
                med = s["p50"]
                ok = [f for f in fr if f[0] <= med * 1.5 + 2] or fr
                sp = [f for f in fr if f[0] > med * 1.5 + 2]
                row = dict(run=Path(rd).name, label=label, kind="cpu", source="ring", **s)
                if fr and len(fr[0]) >= 3:
                    row.update(drwobj_normal=st.mean([f[1] for f in ok]),
                               drwobj_spike=st.mean([f[1] for f in sp]) if sp else 0.0,
                               swap_normal=st.mean([f[2] for f in ok]),
                               swap_p50=st.median([f[2] for f in fr]),
                               work_p50=st.median([f[0] - f[2] for f in fr]),
                               work_p95=pct([f[0] - f[2] for f in fr], 95))
                rows.append(row)
                for i, f in enumerate(fr):
                    frames.append((Path(rd).name, label, i) + f)
                label = None
    return rows, frames


def from_csv(rd, pattern):
    rows = []
    for cp in sorted(Path(rd).glob(pattern)):
        try:
            with open(cp, encoding="utf-8", errors="replace") as f:
                recs = list(csv.DictReader(f))
        except OSError:
            continue
        if not recs:
            continue
        cols = {c.strip().lower(): c for c in recs[0].keys() if c}
        is_gpu = "gpu" in cp.name.lower()
        labels = {}
        for r in recs:
            labels.setdefault(r.get(cols.get("label", ""), "") or cp.stem, []).append(r)
        for lab, rs in labels.items():
            def series(col):
                out = []
                for r in rs:
                    try:
                        out.append(float(r[cols[col]]))
                    except (KeyError, ValueError, TypeError):
                        pass
                return out
            if is_gpu:
                row = dict(run=Path(rd).name, label=lab, kind="gpu", source="ring", file=str(cp))
                for c in GPU_COLS:
                    if c in cols:
                        v = series(c)
                        if v:
                            row[f"gpu_{c}_p50"] = st.median(v)
                            row[f"gpu_{c}_p95"] = pct(v, 95)
                            row["n"] = len(v)
                rows.append(row)
            else:
                cpu = next((c for c in CPU_COLS if c in cols), None)
                if not cpu:
                    continue
                tot = series(cpu)
                row = dict(run=Path(rd).name, label=lab, kind="cpu", source="ring", file=str(cp), **summarize(tot))
                if "swap" in cols:
                    sw = series("swap")
                    if len(sw) == len(tot):
                        row["swap_p50"] = st.median(sw)
                        row["work_p50"] = st.median([a - b for a, b in zip(tot, sw)])
                        row["work_p95"] = pct([a - b for a, b in zip(tot, sw)], 95)
                rows.append(row)
    return rows


def collect(run_dirs, pattern="**/*ring*.csv"):
    rows, frames = [], []
    for rd in run_dirs:
        r, f = from_logs(rd)
        rows += r
        frames += f
        rows += from_csv(rd, pattern)
    return rows, frames


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--out", required=True)
    ap.add_argument("--csv-glob", default="**/*ring*.csv")
    a = ap.parse_args()
    rows, frames = collect(a.runs, a.csv_glob)
    keys = ["run", "label", "kind", "n", "mean", "p50", "p95", "p99", "max", "nspikes", "gap", "spike_mean",
            "normal_mean", "drwobj_normal", "drwobj_spike", "swap_normal", "work_p50", "work_p95"]
    with open(a.out + ".md", "w", encoding="utf-8", newline="\n") as f:
        f.write("| " + " | ".join(keys) + " |\n|" + "---|" * len(keys) + "\n")
        for r in rows:
            f.write("| " + " | ".join(f"{r[k]:.2f}" if isinstance(r.get(k), float) else str(r.get(k, ""))
                                      for k in keys) + " |\n")
    with open(a.out + ".json", "w", encoding="utf-8", newline="\n") as f:
        json.dump(rows, f, indent=1)
    with open(a.out + "_frames.csv", "w", encoding="utf-8", newline="\n") as f:
        f.write("run,label,i,total,drwobj,swap,landobj,drwland\n")
        for fr in frames:
            f.write(",".join(str(x) for x in fr) + "\n")
    print(open(a.out + ".md", encoding="utf-8").read())


if __name__ == "__main__":
    main()
