"""Pool the timing attempts of the final check: keep only the shots whose
sampler window (WINDOW s before the shot up to the shot) saw no foreign game,
then median / min per view and mode over those clean shots.

python pool_quiet.py <out.md> <run_dir> [<run_dir> ...]
"""
import csv, datetime as dt, json, re, statistics as st, sys
from pathlib import Path

WINDOW = 10.0
SHOT_RE = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\].*UDCAP;view=[^;]*;event=shot;label=([^;\"]+)")
ROUND_RE = re.compile(r"^(.+)_r(\d+)$")
SECTIONS = ["frame", "world", "shadow", "ground", "pass1", "rest", "sky", "canopy", "post", "frame_min"]


def shots_clean(d):
    shots = {}
    for line in (d / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
        m = SHOT_RE.match(line)
        if m:
            shots[m.group(2)] = dt.datetime.strptime(m.group(1)[:23], "%Y-%m-%d %H:%M:%S.%f")
    day = min(shots.values()).date()
    samples = []
    with (d / "gpu_samples.csv").open(encoding="ascii", errors="replace") as f:
        for r in csv.DictReader(f):
            try:
                t = dt.datetime.combine(day, dt.datetime.strptime(r["time"], "%H:%M:%S").time())
            except (ValueError, KeyError):
                continue
            samples.append((t, float(r["clock_gr"]), r["pstate"], r.get("poseidon_games", "")))
    out = {}
    for label, t in shots.items():
        win = [s for s in samples if t - dt.timedelta(seconds=WINDOW) <= s[0] <= t + dt.timedelta(seconds=1)]
        foreign = any(re.search(r"foreign([1-9])", s[3]) for s in win)
        near = [s for s in win if s[0] >= t - dt.timedelta(seconds=3)]
        mhz = st.median(s[1] for s in near) if near else None
        out[label] = (bool(win) and not foreign, mhz, sorted({s[2] for s in near}))
    return out


def main():
    outp = Path(sys.argv[1])
    groups = {}
    used = {}
    for dname in sys.argv[2:]:
        d = Path(dname)
        clean = shots_clean(d)
        for r in json.loads((d / "timings.json").read_text(encoding="utf-8")):
            m = ROUND_RE.match(r["view"])
            if not m:
                continue
            label = f"{r['view']}__{r['mode']}"
            ok, mhz, ps = clean.get(label, (False, None, []))
            key = (m.group(1), r["mode"])
            used.setdefault(key, [0, 0])
            used[key][1] += 1
            if not ok:
                continue
            used[key][0] += 1
            r = dict(r, mhz=mhz, ps=ps)
            groups.setdefault(key, []).append(r)
    lines = ["| view | mode | clean / all shots | " + " | ".join(SECTIONS) + " | CPU frame | draws | GPU MHz | P-states |",
             "|---|---|---|" + "---|" * (len(SECTIONS) + 4)]
    res = {}
    for key in sorted(used, key=lambda k: list(used).index(k)):
        g = groups.get(key, [])
        cells = []
        rec = {}
        for s in SECTIONS:
            vals = [x.get(f"gpu_{s}_ms") for x in g if x.get(f"gpu_{s}_ms") is not None]
            rec[s] = (st.median(vals), min(vals)) if vals else None
            cells.append(f"{rec[s][0]:.2f} / {rec[s][1]:.2f}" if rec[s] else "")
        cpu = [x.get("cpu_frame_ms") for x in g if x.get("cpu_frame_ms") is not None]
        draws = [x.get("draws") for x in g if x.get("draws") is not None]
        mhz = [x["mhz"] for x in g if x["mhz"] is not None]
        ps = sorted({p for x in g for p in x["ps"]})
        lines.append("| " + " | ".join([key[0], key[1], f"{used[key][0]} / {used[key][1]}"] + cells + [
            f"{st.median(cpu):.2f}" if cpu else "", f"{st.median(draws):,.0f}" if draws else "",
            f"{st.median(mhz):,.0f}" if mhz else "", "/".join(ps)]) + " |")
        res["|".join(key)] = {"clean": used[key][0], "all": used[key][1], **{s: rec[s] for s in SECTIONS},
                              "cpu": st.median(cpu) if cpu else None, "draws": st.median(draws) if draws else None,
                              "mhz": st.median(mhz) if mhz else None, "ps": ps}
    outp.write_text("\n".join(lines) + "\n", encoding="utf-8")
    outp.with_suffix(".json").write_text(json.dumps(res, indent=1), encoding="utf-8")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
