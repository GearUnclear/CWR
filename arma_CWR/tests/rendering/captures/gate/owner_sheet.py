"""owner_sheet.py: the owner's review sheet (PERF-PLAN P0-12, 3.4 "Escalation", 3.6). One page
per batch: per item, what changed in player words, the same spot and time as a 320 px pair and a
960 px pair, for temporal items a three-way clip side by side at real speed (original | today |
new), and ONE plain question. No numbers, no tuning values, no metric tables, ever.

    python -I owner_sheet.py <spec.json> --out <tmp\\perf-squad\\owner\\<date>> [--no-video]

Encoding is heavy work: run it through squad-heavy.ps1 -Kind encode.

spec.json:
{
  "title": "Changes to look at",                       (optional)
  "items": [
    {"id": "W1-1",
     "says": "Distant ground textures are filtered more cheaply.",
     "question": "Is the right side as good as the left? If not, what looks wrong?",
     "stills": [{"spot": "the valley at dawn", "left": "<golden png>", "right": "<candidate png>"}],
     "clip": {"original": "<clip>", "today": "<clip>", "new": "<clip>", "from": 0, "frames": 600,
              "original_label": "Classic lighting on today's engine"}         (T items only)
    }]
}
Left is always today's game and right the change; the clip runs original | today | new. Until
vanilla clips exist (P0-10 old-sha builds), pass the tip's Classic lighting row as "original"
with original_label "Classic lighting on today's engine"; never call that the original game.
The script refuses text with digits (numbers and tuning values do not belong on the sheet) or
em dashes; --allow-digits exists for place names such as a hill number, used sparingly.

Writes: index.html (open it locally), sheet.png (every item's 320 px pairs on one page),
pairs/<id>_<n>_960.png, clips/<id>.mp4 (H.264 preset slow, 60 fps, each clip scaled to 960 px wide,
labels burnt in; crf 14 first, then re-encoded at a higher crf, at most 24, until the file is under
--max-mb, 19 MiB by default, because the owner views clips over remote control with a 25 MB limit;
never judge motion from a crf 18 encode, A3, so a clip that needed more says its crf in the log).

Frame-time graphs (OVERLAY-1, the owner asked for them on 2026-11-03): a clip with
"timing": {"original": <a.timing.json>, "today": ..., "new": ...} (frametime.py extract, from the
motion job's unrecorded timing pass; a missing panel may be null) gets a 150 px graph strip under
the three panels (frametime.StripRenderer: CPU work and GPU frame per frame, 16.7 and 33.3 ms lines,
4 s window with the current frame at three quarters, a live readout). The strip's numbers are the
one place digits appear on the sheet, at the owner's request; spec text is still checked.
"""
import argparse
import html
import json
import math
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402
import frametime  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

BG = (24, 24, 26)
CRF = 14       # first try; never judge motion from a crf 18 encode (A3) unless the size forces it
CRF_MAX = 24
MAX_MB = 19.0  # MiB: the owner views the clips over remote control, which refuses files over 25 MB
FONT_FILE = "C\\:/Windows/Fonts/arial.ttf"


def check_text(s, allow_digits, where):
    if not s:
        raise SystemExit(f"{where}: empty text")
    if "\u2014" in s:
        raise SystemExit(f"{where}: em dash in '{s}' (no em dashes in owner-facing prose)")
    if not allow_digits and re.search(r"\d", s):
        raise SystemExit(f"{where}: digits in '{s}' (no numbers or tuning values on the owner sheet)")


def fit(path, width):
    im = Image.open(path).convert("RGB")
    h = round(im.height * width / im.width)
    return im.resize((width, h), Image.LANCZOS)


def pair_image(left, right, width, spot, f_lab, f_spot, gap=8):
    a, b = fit(left, width), fit(right, width)
    h = max(a.height, b.height)
    top = 26
    out = Image.new("RGB", (2 * width + gap, h + top + 20), BG)
    d = ImageDraw.Draw(out)
    d.text((0, 4), spot, fill=(230, 210, 120), font=f_spot)
    out.paste(a, (0, top))
    out.paste(b, (width + gap, top))
    d.text((2, top + h + 2), "Today", fill=(205, 205, 205), font=f_lab)
    d.text((width + gap + 2, top + h + 2), "New", fill=(205, 205, 205), font=f_lab)
    return out


def _x264(cmd_in, filt, out_path, crf):
    cmd = cmd_in + ["-filter_complex", filt, "-map", "[v]", "-r", "60", "-c:v", "libx264", "-preset", "slow",
                    "-crf", str(crf), "-movflags", "+faststart", out_path]
    subprocess.run(cmd, check=True)
    return os.path.getsize(out_path)


def encode_threeway(clip, out_path, width=960, crf=CRF, max_mb=MAX_MB):
    """The three-way clip; with clip["timing"] (OVERLAY-1) a frame-time graph strip under each panel.
    Encoded at `crf`; when the file is over max_mb MiB, encoded again at a crf estimated from the size
    (about +6 per halving) until it fits (at most CRF_MAX). Returns (path, crf, bytes)."""
    f = int(clip.get("from", 0))
    n = int(clip.get("frames", 600))
    labs = [clip.get("original_label", "Original game"), "Today", "New"]
    srcs = [clip["original"], clip["today"], clip["new"]]
    parts = []
    text = (f"fontfile='{FONT_FILE}':fontsize=26:fontcolor=white:x=14:y=12:shadowcolor=black@0.8:shadowx=1:shadowy=1")
    for i, lab in enumerate(labs):
        parts.append(f"[{i}:v]trim=start_frame={f}:end_frame={f + n},setpts=PTS-STARTPTS,scale={width}:-2:flags=lanczos,"
                     f"setsar=1,drawtext={text}:text='{lab.replace(':', chr(92) + ':').replace(chr(39), '')}'[p{i}]")
    cmd = [gl.ffmpeg(), "-hide_banner", "-loglevel", "error", "-y"]
    for s in srcs:
        cmd += ["-i", s]
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    strip = None
    tm = clip.get("timing")
    if tm:
        # frame i of the strip is video frame f + i, as the panels after their trim
        ts = [frametime.load_timing(tm.get(k)) for k in ("original", "today", "new")]
        strip = os.path.splitext(out_path)[0] + ".strip.mkv"
        frametime.render_strip(ts, strip, n, f)
        cmd += ["-i", strip]
        parts.append("[p0][p1][p2]hstack=inputs=3,format=yuv444p[top]")
        parts.append("[3:v]setsar=1,format=yuv444p[g]")
        parts.append("[top][g]vstack=inputs=2,format=yuv420p[v]")
    else:
        parts.append("[p0][p1][p2]hstack=inputs=3,format=yuv420p[v]")
    filt = ";".join(parts)
    limit = int(max_mb * 1024 * 1024)
    try:
        size = _x264(cmd, filt, out_path, crf)
        tries = 1
        while size > limit and crf < CRF_MAX and tries < 4:
            # x264 size roughly halves per +6 crf; aim at 95 % of the limit
            crf = min(CRF_MAX, crf + max(1, math.ceil(6 * math.log2(size / (0.95 * limit)))))
            size = _x264(cmd, filt, out_path, crf)
            tries += 1
    finally:
        if strip and os.path.exists(strip):
            os.remove(strip)
    print(f"clip {os.path.basename(out_path)}: crf {crf}, {size / 1e6:.1f} MB{' (over the limit)' if size > limit else ''}")
    return out_path, crf, size


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("spec")
    ap.add_argument("--out", required=True)
    ap.add_argument("--no-video", action="store_true")
    ap.add_argument("--allow-digits", action="store_true")
    ap.add_argument("--crf", type=int, default=CRF, help="first crf tried (default %(default)s)")
    ap.add_argument("--max-mb", type=float, default=MAX_MB, help="re-encode at a higher crf above this many MiB")
    o = ap.parse_args()
    spec = json.load(open(o.spec, encoding="utf-8-sig"))
    title = spec.get("title", "Changes to look at")
    check_text(title, o.allow_digits, "title")
    items = spec["items"]
    for it in items:
        check_text(it["says"], o.allow_digits, f"{it['id']} says")
        check_text(it["question"], o.allow_digits, f"{it['id']} question")
        if it["question"].count("?") < 1:
            raise SystemExit(f"{it['id']}: the question must be a question")
        for s in it.get("stills", []):
            check_text(s["spot"], o.allow_digits, f"{it['id']} spot")
            for k in ("left", "right"):
                if not os.path.exists(s[k]):
                    raise SystemExit(f"{it['id']}: missing {k} image {s[k]}")
        if it.get("clip"):
            check_text(it["clip"].get("original_label", "Original game"), o.allow_digits, f"{it['id']} clip label")
            for k, tp in (it["clip"].get("timing") or {}).items():
                if tp and not os.path.exists(tp):
                    raise SystemExit(f"{it['id']}: missing {k} timing {tp}")
    os.makedirs(o.out, exist_ok=True)
    f_lab, f_spot, f_head, f_title = gl.font(14), gl.font(15), gl.font(18), gl.font(24)
    # sheet.png: every item's 320 px pairs
    blocks = []
    for it in items:
        rows = [pair_image(s["left"], s["right"], 320, s["spot"], f_lab, f_spot) for s in it.get("stills", [])]
        if not rows:
            continue
        w = max(r.width for r in rows)
        hh = 30 + sum(r.height + 10 for r in rows)
        blk = Image.new("RGB", (w, hh), BG)
        d = ImageDraw.Draw(blk)
        d.text((0, 4), it["says"], fill=(235, 235, 235), font=f_head)
        y = 30
        for r in rows:
            blk.paste(r, (0, y))
            y += r.height + 10
        blocks.append(blk)
    if blocks:
        W = max(b.width for b in blocks) + 32
        H = 56 + sum(b.height + 24 for b in blocks)
        sheet = Image.new("RGB", (W, H), BG)
        ImageDraw.Draw(sheet).text((16, 12), title, fill=(240, 240, 240), font=f_title)
        y = 56
        for b in blocks:
            sheet.paste(b, (16, y))
            y += b.height + 24
        sheet.save(os.path.join(o.out, "sheet.png"))
    # 960 px pairs and clips
    secs = []
    for it in items:
        pid = re.sub(r"[^A-Za-z0-9_-]", "_", it["id"])
        figs = []
        for n, s in enumerate(it.get("stills", [])):
            p = os.path.join(o.out, "pairs", f"{pid}_{n}_960.png")
            os.makedirs(os.path.dirname(p), exist_ok=True)
            pair_image(s["left"], s["right"], 960, s["spot"], gl.font(18), gl.font(20), gap=12).save(p)
            figs.append(f'<figure><img src="pairs/{pid}_{n}_960.png" alt="{html.escape(s["spot"])}"></figure>')
        vid = ""
        if it.get("clip"):
            mp4 = os.path.join(o.out, "clips", f"{pid}.mp4")
            if not o.no_video:
                encode_threeway(it["clip"], mp4, crf=o.crf, max_mb=o.max_mb)
            graph = ""
            if it["clip"].get("timing"):
                graph = (" Under each picture, a running graph of how long each frame took to draw on this machine"
                         " (orange: processor, blue: graphics card; lower is better); the white dashed line is the"
                         " budget of a smooth game, the grey one half of that, and the white mark is the frame on screen.")
            vid = (f'<video src="clips/{pid}.mp4" controls loop muted playsinline preload="auto"></video>'
                   f'<p class="hint">{html.escape(it["clip"].get("original_label", "Original game"))} on the left, '
                   f'today in the middle, the change on the right. Same camera path, real speed.{graph}</p>')
        secs.append(f'<section><h2>{html.escape(it["says"])}</h2>{vid}{"".join(figs)}'
                    f'<p class="q">{html.escape(it["question"])}</p></section>')
    page = f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)}</title>
<style>
:root {{ --bg:#18181a; --fg:#e8e8e8; --dim:#a8a8a8; --accent:#e6d278; }}
body {{ background:var(--bg); color:var(--fg); font:16px/1.5 Segoe UI, Arial, sans-serif; margin:0 16px 48px; }}
h1 {{ font-size:24px; margin:20px 0; }} h2 {{ font-size:19px; margin:28px 0 10px; }}
section {{ border-top:1px solid #333; padding-top:8px; }}
img, video {{ max-width:100%; height:auto; display:block; margin:8px 0; }}
.q {{ color:var(--accent); font-size:18px; }} .hint {{ color:var(--dim); font-size:14px; }}
</style></head><body><h1>{html.escape(title)}</h1>
<p class="hint">Left is today's game, right is the change, same spot and time.</p>
{"".join(secs)}
</body></html>
"""
    gl.write_text(os.path.join(o.out, "index.html"), page)
    print("owner sheet:", os.path.join(o.out, "index.html"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
