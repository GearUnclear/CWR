"""Summarise an interleaved off/on timing run (gen_look_light.py modes
off1..on3) into a markdown table: GPU sections and the render thread's CPU.

    python timing_summary.py <run dir> <out.md> [note]
"""
import json
import re
import statistics
import sys
from pathlib import Path

run, out = Path(sys.argv[1]), Path(sys.argv[2])
note = sys.argv[3] if len(sys.argv) > 3 else ""
rows = json.loads((run / "timings.json").read_text(encoding="utf-8"))


def perf(r, key):
    m = re.search(re.escape(key) + r"=([0-9.]+)/([0-9.]+)", r.get("perf_raw") or "")
    return float(m.group(1)) if m else None


def gpu_section(r, name):
    m = re.search(r"\b" + re.escape(name) + r" ([0-9.]+)", r.get("gpu_raw") or "")
    return float(m.group(1)) if m else None


def cam(r, key):
    m = re.search(r"\| cam [^|]*\b" + key + r"=([0-9.]+)/([0-9.]+)", r.get("feat") or "")
    return (float(m.group(1)), float(m.group(2))) if m else None


by = {}
for r in rows:
    kind = "on" if r["mode"].startswith("on") else "off"
    by.setdefault((r["view"], kind), []).append(r)


def med(vals):
    vals = [v for v in vals if v is not None]
    return f"{statistics.median(vals):.2f}" if vals else "-"


def mn(vals):
    vals = [v for v in vals if v is not None]
    return f"{min(vals):.2f}" if vals else "-"


lines = ["# photoLight off and on, interleaved three times per view", "", note, "",
         "GPU (ms; `triGpuTimes`, smoothed over the 150 frames before each shot): frame, post pass, the air "
         "pass, the reflections and the composite (where the camera's meter and composite run). CPU (ms; "
         "`triPerfStats` over the same frames): the render thread's whole frame, `drw:post` (the post pass's "
         "CPU side: the camera's readback collection and meter queue are in it) and the camera's own timers "
         "(`cam cpu=collect/queue`, smoothed). Medians of three, the GPU frame also as the minimum.", "",
         "| view | mode | GPU frame min | GPU frame med | post med | air med | ssr med | composite med | CPU frame med | "
         "FPS med | drw:post med | cam collect | cam queue |",
         "|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
for (v, k), rs in sorted(by.items()):
    camc = [cam(r, "cpu") for r in rs]
    lines.append(
        f"| {v} | {k} | {mn([r['gpu_frame_ms'] for r in rs])} | {med([r['gpu_frame_ms'] for r in rs])} | "
        f"{med([r['gpu_post_ms'] for r in rs])} | {med([gpu_section(r, 'air') for r in rs])} | "
        f"{med([gpu_section(r, 'ssr') for r in rs])} | {med([r['gpu_composite_ms'] for r in rs])} | "
        f"{med([r['cpu_frame_ms'] for r in rs])} | {med([r['cpu_fps'] for r in rs])} | "
        f"{med([perf(r, 'drw:post') for r in rs])} | {med([c[0] if c else None for c in camc])} | "
        f"{med([c[1] if c else None for c in camc])} |")
out.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(out.read_text(encoding="utf-8"))
