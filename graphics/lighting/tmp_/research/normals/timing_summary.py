"""Summarise an interleaved timing run (labels <variant><round>, e.g. cura, v8b).

    python timing_summary.py <run_dir>

Reads the UDCAP shot lines (GPU frame / world ms per shot) and prints, per
view and variant, every sample plus the min and median, and the delta of the
medians against the late-VB base ('vb'). Writes <run_dir>/timing_summary.md.
"""
import re
import statistics
import sys
from pathlib import Path

sys.path.insert(0, r"D:\Arma_CWA_worktrees\lighting\arma_CWR\tests\rendering\captures")
from make_pairs import read_udcap, parse_kv, GPU_RES  # noqa: E402

NAMES = {"cur": "Remastered as shipped (late shapes on the CPU path)", "vb": "+ late-VB fix, no bump",
         "v5": "+ load-time normal map", "v8": "+ map, micro shadows, cavities", "v9": "+ all x2",
         "v2": "+ shader-tap bump", "v7": "+ micro shadows"}


def main():
    run = Path(sys.argv[1])
    data = {}
    for _p, _line, body in read_udcap(run / "tri"):
        if body is None:
            continue
        kv = parse_kv(body)
        if kv.get("event") != "shot" or "__" not in kv.get("label", ""):
            continue
        view, label = kv["label"].rsplit("__", 1)
        m = re.match(r"^(cur|vb|v\d)([a-h])$", label)
        if not m:
            continue
        row = {}
        for f, rx in GPU_RES.items():
            mm = rx.search(kv.get("gpu", ""))
            row[f] = float(mm.group(1)) if mm else None
        data.setdefault(view, {}).setdefault(m.group(1), []).append(row)
    out = ["| view | variant | GPU frame samples (ms) | min | min - vb min | median | median - vb | world median |", "|---|---|---|---|---|---|---|---|"]
    for view, var in sorted(data.items()):
        base = statistics.median(r["frame"] for r in var.get("vb", [])) if "vb" in var else None
        baseMin = min(r["frame"] for r in var.get("vb", [])) if "vb" in var else None
        for v in ["cur", "vb", "v2", "v5", "v7", "v8", "v9"]:
            if v not in var:
                continue
            fr = [r["frame"] for r in var[v] if r["frame"] is not None]
            wo = [r["world"] for r in var[v] if r.get("world") is not None]
            med = statistics.median(fr)
            delta = f"{med - base:+.2f}" if base is not None else ""
            out.append(f"| {view} | {NAMES.get(v, v)} | {', '.join(f'{x:.2f}' for x in fr)} | {min(fr):.2f} | {(min(fr) - baseMin) if baseMin is not None else 0:+.2f} | {med:.2f} | {delta} | {statistics.median(wo):.2f} |")
    text = "\n".join(out) + "\n"
    (run / "timing_summary.md").write_text(text)
    print(text)


if __name__ == "__main__":
    main()
