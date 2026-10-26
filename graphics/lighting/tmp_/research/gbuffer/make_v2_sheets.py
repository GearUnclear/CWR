"""Sheets and timing summary for the resumed run (run06_v2).

    python make_v2_sheets.py <run06_dir> <run05_dir>

Uses collect_gb.py to copy the shots and parse UDCAP lines, then writes into
<run06_dir>:
    cmp_<view>__v2.png         Classic | Remastered | + late VBO | + GI v2
    str_<view>.png             + late VBO | GI v2 x1 | x2 | x3 whole-frame source
    noise_<view>.png           GI v2 accumulated | single frame (what a moving camera sees)
    wedge_<view>.png           v1 vs v2 sky visibility + bounce debug (v1 from run05)
    timing_rounds.md           interleaved timing rounds: per mode median / min / max
"""
import json
import statistics
import sys
from pathlib import Path

from PIL import Image, ImageDraw

import collect_gb as cg

cg.PRETTY.update({
    "rm_vbo": "Remastered + late VBO",
    "v2gi": "+ G-buffer GI v2 (x1, r 6 m)",
    "v2gi_x2": "+ GI v2 x2",
    "v2gi_x3s": "+ GI v2 x3, whole-frame source",
    "v2gi_1f": "+ GI v2, single frame (no accumulation)",
    "v2dbg_bounce": "GI v2 bounce irradiance",
    "v2dbg_vis": "GI v2 sky visibility",
    "dbg_bounce": "GI v1 bounce irradiance",
    "dbg_vis": "GI v1 sky visibility",
})


def sheet(view, specs, dst, rows, title):
    """specs: list of (dir, label). Full frames side by side, labelled, GPU ms."""
    imgs = []
    for d, l in specs:
        p = Path(d) / f"{view}__{l}.png"
        imgs.append(Image.open(p).convert("RGB") if p.exists() else None)
    w, h = 960, 640
    gap, top = 8, 40
    out = Image.new("RGB", (len(specs) * w + (len(specs) - 1) * gap, h + top), (24, 24, 24))
    dr = ImageDraw.Draw(out)
    dr.text((10, 8), title, font=cg.font(22), fill=(235, 235, 235))
    for i, ((d, l), im) in enumerate(zip(specs, imgs)):
        x0 = i * (w + gap)
        if im is None:
            dr.rectangle([x0, top, x0 + w, top + h], fill=(70, 0, 0))
            cg.label_box(dr, (x0 + 12, top + 10), "MISSING " + l, cg.font(22))
            continue
        out.paste(im, (x0, top))
        cg.label_box(dr, (x0 + 12, top + 10), cg.PRETTY.get(l, l), cg.font(22))
        r = rows.get((Path(d).name, view, l))
        if r and r.get("frame") is not None:
            txt = f"GPU frame {r['frame']:.2f} ms | world {r['world'] or 0:.2f} | post {r['post'] or 0:.2f} (ao/gi {r['ao'] or 0:.2f})"
            cg.label_box(dr, (x0 + 12, top + h - 30), txt, cg.font(15))
    out.save(dst)
    return dst


def main():
    run6, run5 = Path(sys.argv[1]), Path(sys.argv[2])
    keys = cg.collect(run6 / "tri", run6)
    rows6, other = cg.read_rows(run6 / "tri")
    rows5, _ = cg.read_rows(run5 / "tri")
    rows = {}
    for (v, m), r in rows6.items():
        rows[(run6.name, v, m)] = r
    for (v, m), r in rows5.items():
        rows[(run5.name, v, m)] = r
    views = sorted({v for v, l in keys if not l.startswith("t")})
    made = []
    for v in views:
        r6 = str(run6)
        made.append(sheet(v, [(r6, "classic"), (r6, "remastered"), (r6, "rm_vbo"), (r6, "v2gi")],
                          run6 / f"cmp_{v}__v2.png", rows, f"{v}: Classic | Remastered | + late VBO | + G-buffer GI v2"))
        made.append(sheet(v, [(r6, "rm_vbo"), (r6, "v2gi"), (r6, "v2gi_x2"), (r6, "v2gi_x3s")],
                          run6 / f"str_{v}.png", rows, f"{v}: GI v2 strength"))
        made.append(sheet(v, [(r6, "v2gi"), (r6, "v2gi_1f")],
                          run6 / f"noise_{v}.png", rows, f"{v}: GI v2 accumulated over still frames vs one frame"))
        made.append(sheet(v, [(str(run5), "dbg_vis"), (r6, "v2dbg_vis"), (str(run5), "dbg_bounce"), (r6, "v2dbg_bounce")],
                          run6 / f"wedge_{v}.png", rows, f"{v}: GI v1 (run05, built in) vs v2 (override) debug targets"))
        for l in ("v2gi", "v2gi_x2", "v2gi_x3s", "v2gi_1f"):
            f = cg.diff(v, l, run6, ref="rm_vbo", gain=4)
            if f:
                made.append(f)
    # timing rounds: labels <view>__t<r>_<mode>
    tr = {}
    for (v, m), r in rows6.items():
        if not (m.startswith("t") and "_" in m and m[1].isdigit()):
            continue
        mode = m.split("_", 1)[1]
        tr.setdefault((v, mode), []).append(r)
    md = ["| view | mode | n | frame median | frame min | frame max | world median | post median | ao/gi median |",
          "|---|---|---|---|---|---|---|---|---|"]
    summary = []
    for (v, mode), rs in sorted(tr.items()):
        fr = [r["frame"] for r in rs if r.get("frame") is not None]
        wo = [r["world"] for r in rs if r.get("world") is not None]
        po = [r["post"] for r in rs if r.get("post") is not None]
        ao = [r["ao"] for r in rs if r.get("ao") is not None]
        med = lambda a: f"{statistics.median(a):.2f}" if a else ""
        md.append(f"| {v} | {mode} | {len(fr)} | {med(fr)} | {min(fr) if fr else 0:.2f} | {max(fr) if fr else 0:.2f} | {med(wo)} | {med(po)} | {med(ao)} |")
        summary.append({"view": v, "mode": mode, "frames": fr, "world": wo, "post": po, "ao": ao})
    (run6 / "timing_rounds.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    (run6 / "timing_rounds.json").write_text(json.dumps(summary, indent=1), encoding="utf-8")
    # plain per-shot table too
    md2 = ["| view | shot | GPU frame | world | shadow | post | ao/gi |", "|---|---|---|---|---|---|---|"]
    for (v, m), r in sorted(rows6.items()):
        if m.startswith("t") and m[1:2].isdigit():
            continue
        f = lambda k: "" if r.get(k) is None else f"{r[k]:.2f}"
        md2.append(f"| {v} | {m} | {f('frame')} | {f('world')} | {f('shadow')} | {f('post')} | {f('ao')} |")
    (run6 / "timings_gb.md").write_text("\n".join(md2) + "\n\nengine log lines:\n\n" + "\n".join(other[:40]) + "\n", encoding="utf-8")
    (run6 / "timings_gb.json").write_text(json.dumps([r for r in rows6.values()], indent=1), encoding="utf-8")
    print("made", len(made))
    print("\n".join(md))
    print("\n".join(md2))


if __name__ == "__main__":
    main()
