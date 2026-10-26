"""Summarise the interleaved GPU timing run (event=timing lines) into
final/timings_interleaved.md and .json: per mode, the median and minimum of
the frame, world and post sections over 3 rounds x 6 samples."""

import json
import statistics
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import taalib as T  # noqa: E402
import sheets as S  # noqa: E402

run = sys.argv[1] if len(sys.argv) > 1 else "run12-timing"
stem = sys.argv[2] if len(sys.argv) > 2 else "timings_interleaved"
rows, udcap = T.timings(S.ROOT / run / "tri")
(S.ROOT / run / "udcap.log").write_text("\n".join(udcap), encoding="utf-8")
modes = {}
for line in udcap:
    kv = T.parse_kv(line)
    if kv.get("event") != "timing":
        continue
    tag = kv["tag"].rsplit("_r", 1)[0]
    g = kv["gpu"]
    m = {}
    for f, rx in T.GPU_RES.items():
        mm = rx.search(g)
        m[f] = float(mm.group(1)) if mm else None
    modes.setdefault(tag, []).append(m)

out = {}
lines = ["| mode | n | frame median | frame min | world median | post median | ao median | composite (+TAA) median |", "|---|---|---|---|---|---|---|---|"]
for tag, ms in modes.items():
    def med(f):
        xs = [x[f] for x in ms if x[f] is not None]
        return statistics.median(xs) if xs else float("nan")
    def mn(f):
        xs = [x[f] for x in ms if x[f] is not None]
        return min(xs) if xs else float("nan")
    out[tag] = {"n": len(ms), "frame_median": med("frame"), "frame_min": mn("frame"), "world_median": med("world"),
                "post_median": med("post"), "ao_median": med("ao"), "composite_median": med("composite")}
    o = out[tag]
    lines.append(f"| {tag} | {o['n']} | {o['frame_median']:.2f} | {o['frame_min']:.2f} | {o['world_median']:.2f} | {o['post_median']:.2f} | {o['ao_median']:.2f} | {o['composite_median']:.2f} |")
(S.ROOT / "final" / f"{stem}.json").write_text(json.dumps(out, indent=1), encoding="utf-8")
(S.ROOT / "final" / f"{stem}.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print("\n".join(lines))
