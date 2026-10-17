"""Median GPU timings over the final passes: python cost.py <runs dir> <out.md>"""
import json
import statistics
import sys
from pathlib import Path

VIEWS = ["coastline_sunset", "sea_noon", "sea_away_from_sun", "coast_shore_midsun"]
IDS = ["stock", "A", "B", "C"]


def main(runs, out):
    runs = Path(runs)
    data = {}
    for rid in IDS:
        for d in sorted(runs.glob(f"{rid}-final*")):
            p = d / "timings.json"
            if not p.exists():
                continue
            for r in json.loads(p.read_text(encoding="utf-8")):
                key = (r["view"], rid if r["mode"] == "remastered" else "classic")
                data.setdefault(key, []).append((r.get("gpu_frame_ms"), r.get("gpu_world_ms")))
    lines = ["# GPU cost of the water variants", "",
             "Median (and spread) over the final passes of `triGpuTimes` (smoothed GPU timestamps, ~90 frames per",
             "shot). Other research agents were running games on the same GPU at the same time, so single values",
             "move by a millisecond or more; read the medians, and only large differences.", "",
             "| view | shader | n | frame ms median | frame min-max | world ms median | world min-max | world delta vs now |",
             "|---|---|---|---|---|---|---|---|"]
    for v in VIEWS:
        base = None
        for rid in ["classic"] + IDS:
            vals = [x for x in data.get((v, rid), []) if x[0] is not None]
            if not vals:
                continue
            fr = [x[0] for x in vals]
            wo = [x[1] for x in vals if x[1] is not None]
            fm, wm = statistics.median(fr), (statistics.median(wo) if wo else None)
            if rid == "stock":
                base = wm
            delta = "" if (base is None or wm is None or rid in ("classic", "stock")) else f"{wm - base:+.2f}"
            name = {"classic": "Classic", "stock": "Remastered now"}.get(rid, f"Variant {rid}")
            lines.append(f"| {v} | {name} | {len(fr)} | {fm:.2f} | {min(fr):.2f}-{max(fr):.2f} | "
                         f"{'' if wm is None else f'{wm:.2f}'} | {'' if not wo else f'{min(wo):.2f}-{max(wo):.2f}'} | {delta} |")
    Path(out).write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
