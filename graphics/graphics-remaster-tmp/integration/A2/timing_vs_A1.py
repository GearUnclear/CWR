"""A2 timing (this folder's run) against A1's (tmp/integration/A1/20261018-014500-timing):
remastered and classic rows, GPU frame median / min, world, post, CPU frame, draws and
the GPU clock with its P-states. A row is comparable only where both runs read P0."""
import json
from pathlib import Path

A2 = Path(__file__).resolve().parent
NEW = A2 / "20261018-024623-timing" / "timing.json"
OLD = A2.parent / "A1" / "20261018-014500-timing" / "timing.json"


def rows(path):
    data = json.loads(path.read_text(encoding="utf-8"))
    groups = data["groups"] if isinstance(data, dict) and "groups" in data else data
    out = {}
    for g in groups:
        out[(g["view"], g["mode"])] = g
    return out


new, old = rows(NEW), rows(OLD)


KEYS = {"frame": "gpu_frame", "world": "gpu_world", "post": "gpu_post", "cpu_frame": "cpu_frame"}


def stat(g, key):
    k = KEYS[key]
    return g.get(f"{k}_median"), g.get(f"{k}_min")


def pstates(g):
    return "/".join(g.get("pstates") or [])


def fmt(v):
    return "" if v is None else f"{v:.2f}"


md = ["# A2 timing against A1", "",
      "GPU ms median / min over 3 interleaved rounds (`triGpuTimes`), CPU frame ms median, GPU MHz median",
      "with P-states, RTX 4060 Ti, 1920x1080, render scale 1.0, 4x MSAA. A2: 20261018-024623-timing (0 of 4",
      "tri slots held before and after, no foreign PoseidonGame in 235 sampler rows). A1:",
      "tmp/integration/A1/20261018-014500-timing. delta = A2 median minus A1 median, shown where both read",
      "P0 only.", "",
      "| view | mode | A1 frame | A2 frame | delta | A1 world | A2 world | A1 post | A2 post | A1 CPU | A2 CPU | draws A1 / A2 | MHz A1 (P) | MHz A2 (P) |",
      "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
deltas = []
for key in new:
    if key not in old:
        continue
    n, o = new[key], old[key]
    nf, nfm = stat(n, "frame")
    of, ofm = stat(o, "frame")
    p0 = pstates(n) == "P0" and pstates(o) == "P0"
    d = (nf - of) if (p0 and nf is not None and of is not None) else None
    if d is not None and key[1] == "remastered":
        deltas.append((key[0], d, of))
    md.append(f"| {key[0]} | {key[1]} | {fmt(of)} / {fmt(ofm)} | {fmt(nf)} / {fmt(nfm)} | {fmt(d)} | "
              f"{fmt(stat(o, 'world')[0])} | {fmt(stat(n, 'world')[0])} | {fmt(stat(o, 'post')[0])} | "
              f"{fmt(stat(n, 'post')[0])} | {fmt(stat(o, 'cpu_frame')[0])} | {fmt(stat(n, 'cpu_frame')[0])} | "
              f"{o.get('draws_median', '')} / {n.get('draws_median', '')} | {o.get('gpu_mhz_median') or 0:.0f} ({pstates(o)}) | "
              f"{n.get('gpu_mhz_median') or 0:.0f} ({pstates(n)}) |")
md += ["", "Remastered rows where both runs read P0 (A2 minus A1 frame median, ms):", ""]
md += [f"- {v}: {d:+.2f} ({100 * d / max(of, 1e-6):+.1f}%)" for v, d, of in deltas]
(A2 / "timing-vs-A1.md").write_text("\n".join(md) + "\n", encoding="utf-8")
print("\n".join(md[-(len(deltas) + 2):]))
