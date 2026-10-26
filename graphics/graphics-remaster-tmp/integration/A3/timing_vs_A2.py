"""A3 timing (this folder's run) against A2's (tmp/integration/A2/20261018-024623-timing):
remastered and classic rows, GPU frame median / min, world, post, CPU frame, draws and
the GPU clock with its P-states. A row is comparable only where both runs read P0."""
import json
from pathlib import Path

A3 = Path(__file__).resolve().parent
NEW = A3 / "20261018-035421-timing" / "timing.json"
OLD = A3.parent / "A2" / "20261018-024623-timing" / "timing.json"


def rows(path):
    data = json.loads(path.read_text(encoding="utf-8"))
    groups = data["groups"] if isinstance(data, dict) and "groups" in data else data
    out = {}
    for g in groups:
        out[(g["view"], g["mode"])] = g
    return out


new, old = rows(NEW), rows(OLD)


KEYS = {"frame": "gpu_frame", "world": "gpu_world", "post": "gpu_post", "cpu_frame": "cpu_frame",
        "ao": "gpu_ao", "resolve": "gpu_resolve"}


def stat(g, key):
    k = KEYS[key]
    return g.get(f"{k}_median"), g.get(f"{k}_min")


def pstates(g):
    return "/".join(g.get("pstates") or [])


def fmt(v):
    return "" if v is None else f"{v:.2f}"


md = ["# A3 timing against A2", "",
      "GPU ms median / min over 3 interleaved rounds (`triGpuTimes`), CPU frame ms median, GPU MHz median",
      "with P-states, RTX 4060 Ti, 1920x1080, render scale 1.0, 4x MSAA. A3: 20261018-035421-timing (0 of 4",
      "tri slots held before and after, no foreign PoseidonGame in the sampler rows). A2:",
      "tmp/integration/A2/20261018-024623-timing. delta = A3 median minus A2 median, shown where both read",
      "P0 only. A2's `ao` section ran from WorldEnd, so it held the resolve blits: compare it with A3's",
      "resolve + ao.", "",
      "| view | mode | A2 frame | A3 frame | delta | A2 world | A3 world | A2 post | A3 post | A2 ao | A3 resolve + ao | A2 CPU | A3 CPU | draws A2 / A3 | MHz A2 (P) | MHz A3 (P) |",
      "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
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
    nr, na = stat(n, "resolve")[0], stat(n, "ao")[0]
    ra = None if (nr is None and na is None) else (nr or 0.0) + (na or 0.0)
    md.append(f"| {key[0]} | {key[1]} | {fmt(of)} / {fmt(ofm)} | {fmt(nf)} / {fmt(nfm)} | {fmt(d)} | "
              f"{fmt(stat(o, 'world')[0])} | {fmt(stat(n, 'world')[0])} | {fmt(stat(o, 'post')[0])} | "
              f"{fmt(stat(n, 'post')[0])} | {fmt(stat(o, 'ao')[0])} | {fmt(ra)} | {fmt(stat(o, 'cpu_frame')[0])} | "
              f"{fmt(stat(n, 'cpu_frame')[0])} | {o.get('draws_median', '')} / {n.get('draws_median', '')} | "
              f"{o.get('gpu_mhz_median') or 0:.0f} ({pstates(o)}) | {n.get('gpu_mhz_median') or 0:.0f} ({pstates(n)}) |")
md += ["", "Remastered rows where both runs read P0 (A3 minus A2 frame median, ms):", ""]
md += [f"- {v}: {d:+.2f} ({100 * d / max(of, 1e-6):+.1f}%)" for v, d, of in deltas]
(A3 / "timing-vs-A2.md").write_text("\n".join(md) + "\n", encoding="utf-8")
print("\n".join(md[-(len(deltas) + 2):]))
