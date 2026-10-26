r"""Before/after comparison for the object-shadows fix.

Usage: python compare.py <before_dir> <after_dir> <out_dir>
Each run dir is a run-captures.ps1 output folder (tri tree under tri\).
Writes <label>__ba.png (before | after, labelled) and timings.md.
"""
import re
import sys
from pathlib import Path
from statistics import median

from PIL import Image, ImageDraw, ImageFont

UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "shadow": re.compile(r"\(shadow ([\d.]+)\)"),
}
PERF_RE = re.compile(r"\bfps=([\d.]+) frame=([\d.]+)/([\d.]+)/([\d.]+)")
SHOT_RE = re.compile(r"^\d{3}_(.+)\.png$")


def shots(run: Path):
    found = {}
    for p in sorted((run / "tri").rglob("*.png")):
        m = SHOT_RE.match(p.name)
        if m:
            found[m.group(1)] = p  # newest (highest seq) wins
    return found


def records(run: Path):
    out = []
    for log in (run / "tri").rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(line)
            if not m or "event=shot" not in m.group(1):
                continue
            body = m.group(1)
            gpu = body.split("gpu=", 1)[1] if "gpu=" in body else ""
            label = re.search(r"label=([^;]+)", body).group(1)
            rec = {"label": label, "gpu": gpu}
            for k, rx in GPU.items():
                g = rx.search(gpu)
                rec[k] = float(g.group(1)) if g else None
            p = PERF_RE.search(body)
            rec["cpu_frame"] = float(p.group(2)) if p else None
            rec["fps"] = float(p.group(1)) if p else None
            out.append(rec)
    return out


def font(size):
    for name in ("arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def side_by_side(a: Path, b: Path, out: Path, title: str, ta: str, tb: str):
    ia, ib = Image.open(a).convert("RGB"), Image.open(b).convert("RGB")
    w, h = ia.size
    canvas = Image.new("RGB", (w * 2 + 8, h + 34), (24, 24, 24))
    canvas.paste(ia, (0, 34))
    canvas.paste(ib.resize((w, h)), (w + 8, 34))
    d = ImageDraw.Draw(canvas)
    f = font(18)
    d.text((8, 7), f"BEFORE (branch tip): {ta}", fill=(255, 255, 255), font=f)
    d.text((w + 16, 7), f"AFTER (fix): {tb}", fill=(255, 255, 255), font=f)
    canvas.save(out)


def main():
    before, after, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    out.mkdir(parents=True, exist_ok=True)
    sb, sa = shots(before), shots(after)
    rb, ra = records(before), records(after)
    byb = {r["label"]: r for r in rb}
    bya = {r["label"]: r for r in ra}

    def tag(r):
        if not r:
            return ""
        parts = []
        if r.get("frame") is not None:
            parts.append(f"GPU frame {r['frame']:.2f} ms")
        if r.get("shadow") is not None:
            parts.append(f"shadow pass {r['shadow']:.2f} ms")
        return ", ".join(parts)

    for label in sorted(set(sb) & set(sa)):
        side_by_side(sb[label], sa[label], out / f"{label}__ba.png", label, tag(byb.get(label)), tag(bya.get(label)))

    lines = ["| label | before GPU frame | before world | before shadow | before CPU frame | after GPU frame | after world | after shadow | after CPU frame |", "|---|---|---|---|---|---|---|---|---|"]
    for label in sorted(set(byb) | set(bya)):
        b, a = byb.get(label, {}), bya.get(label, {})

        def f(r, k):
            v = r.get(k)
            return "" if v is None else f"{v:.2f}"
        lines.append(f"| {label} | {f(b,'frame')} | {f(b,'world')} | {f(b,'shadow')} | {f(b,'cpu_frame')} | {f(a,'frame')} | {f(a,'world')} | {f(a,'shadow')} | {f(a,'cpu_frame')} |")

    # Shadow pass medians over the remastered reads of each view (shot + t2 + t3).
    lines.append("")
    lines.append("| view | before shadow pass median (n) | after shadow pass median (n) | before GPU frame median | after GPU frame median |")
    lines.append("|---|---|---|---|---|")
    views = sorted({l.split("__")[0] for l in set(byb) | set(bya)})
    for v in views:
        def pick(recs, key):
            vals = [r[key] for l, r in recs.items() if l.split("__")[0] == v and l.split("__")[1] in ("remastered", "t2", "t3") and r.get(key) is not None]
            return vals
        sbv, sav = pick(byb, "shadow"), pick(bya, "shadow")
        fbv, fav = pick(byb, "frame"), pick(bya, "frame")
        def m(x):
            return f"{median(x):.2f} ({len(x)})" if x else ""
        lines.append(f"| {v} | {m(sbv)} | {m(sav)} | {m(fbv)} | {m(fav)} |")

    lines.append("")
    lines.append("Raw triGpuTimes summaries:")
    lines.append("")
    for name, recs in (("before", rb), ("after", ra)):
        for r in recs:
            lines.append(f"- {name} `{r['label']}`: `{r['gpu']}` cpu frame {r['cpu_frame']}")
    (out / "timings.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines[:60]))


if __name__ == "__main__":
    main()
