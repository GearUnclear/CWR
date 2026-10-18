"""Assemble the side-by-side feature videos from a motion-capture run
(INTEGRATION-PLAN.md section 7.4). Skeleton from step A1.

    python make_video.py <run folder> [--out <dir>] [--clip <clip> ...] [--montage] [--keep-frames]

<run folder> is a run-captures.ps1 output folder of gen_video_tests.py tests.
For every clip found in its tri tree (NNN_<clip>__off__f####.png and
NNN_<clip>__on__f####.png; a retry's newer file wins) this script checks both
passes have the same frames, then writes into <out>/<clip>/ (default
<worktree>/tmp/video/<clip>/):

    <clip>__sbs.mp4    3840x1080: the "off" pass left, "on" right, labelled,
                       with the GPU frame time of every 10th frame when the
                       frame log has it
    <clip>__wipe.mp4   1920x1080: the left half off, the right half on, a divider
    frames.txt         which frames went in, and the frame log's feat= line of
                       the first frame of each pass (the switches and gates)

--montage also writes <out>/montage.mp4: every clip in the order of the clip
table (plan section 4 order), each after a two-second title card naming its
feature and switch keys. 30 fps, H.264 (libx264, CRF 18, yuv420p), through
ffmpeg (C:\\ffmpeg\\bin\\ffmpeg.exe or ffmpeg on PATH); frames are composed with
Pillow first.
"""
import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))

import gen_video_tests  # noqa: E402

FRAME_RE = re.compile(r"^(\d{3,})_(.+)__(off|on)__f(\d{4})\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
GPU_FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
FPS = 30


def ffmpeg_exe():
    p = Path(r"C:\ffmpeg\bin\ffmpeg.exe")
    if p.exists():
        return str(p)
    found = shutil.which("ffmpeg")
    if not found:
        raise SystemExit("ffmpeg not found (C:\\ffmpeg\\bin\\ffmpeg.exe or PATH)")
    return found


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf", "DejaVuSans-Bold.ttf"):
        for base in (Path("C:/Windows/Fonts"), Path("/usr/share/fonts/truetype/dejavu")):
            if (base / name).exists():
                return ImageFont.truetype(str(base / name), size)
    return ImageFont.load_default()


def label(draw, xy, text, fnt):
    box = draw.textbbox(xy, text, font=fnt)
    draw.rectangle([box[0] - 8, box[1] - 5, box[2] + 8, box[3] + 5], fill=(0, 0, 0))
    draw.text(xy, text, font=fnt, fill=(255, 255, 255))


def collect(tri_dir: Path):
    """clip -> pass -> frame number -> newest path."""
    best = {}
    for p in tri_dir.rglob("*.png"):
        m = FRAME_RE.match(p.name)
        if not m:
            continue
        clip, pas, f = m.group(2), m.group(3), int(m.group(4))
        slot = best.setdefault(clip, {}).setdefault(pas, {})
        if f not in slot or p.stat().st_mtime > slot[f].stat().st_mtime:
            slot[f] = p
    return best


def frame_log(tri_dir: Path):
    """label -> {gpu_ms, feat, mode} from the event=frame lines."""
    out = {}
    for p in sorted(tri_dir.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m or "event=frame" not in m.group(1):
                continue
            body = m.group(1).strip().rstrip('"')
            gpu = ""
            if ";gpu=" in body:
                body, gpu = body.split(";gpu=", 1)
            kv = dict(part.split("=", 1) for part in body.split(";") if "=" in part)
            g = GPU_FRAME_RE.search(gpu)
            out[kv.get("label", "")] = {"gpu_ms": float(g.group(1)) if g else None, "feat": kv.get("feat", ""),
                                        "mode": kv.get("mode", "")}
    return out


def clip_info(clip):
    for c in gen_video_tests.CLIPS:
        if c["clip"] == clip:
            return c
    return {"clip": clip, "feature": clip, "keys": []}


def compose(clip, frames, log, work: Path):
    """Pillow frames for the side-by-side and the wipe; returns the two folders."""
    sbs_dir, wipe_dir = work / "sbs", work / "wipe"
    sbs_dir.mkdir(parents=True, exist_ok=True)
    wipe_dir.mkdir(parents=True, exist_ok=True)
    info = clip_info(clip)
    big, small = font(34), font(24)
    off_title = f"{info['feature']} off ({', '.join(info['keys'])} = 0)" if info.get("keys") else "off"
    gpu_last = {"off": None, "on": None}
    for i, f in enumerate(sorted(frames["off"])):
        a = Image.open(frames["off"][f]).convert("RGB")
        b = Image.open(frames["on"][f]).convert("RGB")
        w, h = a.size
        sbs = Image.new("RGB", (w * 2, h))
        sbs.paste(a, (0, 0))
        sbs.paste(b.resize((w, h)) if b.size != a.size else b, (w, 0))
        d = ImageDraw.Draw(sbs)
        for pas, x0, title in (("off", 0, off_title), ("on", w, "on")):
            label(d, (x0 + 24, 20), title, big)
            row = log.get(f"{clip}__{pas}__f{f:04d}", {})
            if row.get("gpu_ms") is not None:
                gpu_last[pas] = row["gpu_ms"]
            if gpu_last[pas] is not None:
                label(d, (x0 + 24, h - 52), f"GPU frame {gpu_last[pas]:.2f} ms", small)
        sbs.save(sbs_dir / f"{i + 1:05d}.png")
        wipe = a.copy()
        wipe.paste(b.crop((w // 2, 0, w, h)), (w // 2, 0))
        d = ImageDraw.Draw(wipe)
        d.rectangle([w // 2 - 2, 0, w // 2 + 1, h], fill=(255, 255, 255))
        label(d, (24, 20), off_title, big)
        label(d, (w // 2 + 24, 20), "on", big)
        wipe.save(wipe_dir / f"{i + 1:05d}.png")
    return sbs_dir, wipe_dir


def encode(ff, src_dir: Path, dst: Path):
    cmd = [ff, "-y", "-loglevel", "error", "-framerate", str(FPS), "-i", str(src_dir / "%05d.png"), "-c:v", "libx264",
           "-crf", "18", "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(dst)]
    subprocess.run(cmd, check=True)


def title_card(ff, clip, out: Path, size=(1920, 1080), seconds=2):
    info = clip_info(clip)
    work = Path(tempfile.mkdtemp(prefix="card_"))
    img = Image.new("RGB", size, (12, 12, 12))
    d = ImageDraw.Draw(img)
    d.text((120, 420), info["feature"], font=font(72), fill=(240, 240, 240))
    d.text((120, 540), f"switch: {', '.join(info.get('keys', [])) or '-'}", font=font(40), fill=(200, 200, 200))
    d.text((120, 610), f"clip {clip}: left off, right on", font=font(32), fill=(160, 160, 160))
    for i in range(FPS * seconds):
        img.save(work / f"{i + 1:05d}.png")
    encode(ff, work, out)
    shutil.rmtree(work, ignore_errors=True)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("run")
    ap.add_argument("--out", default=None)
    ap.add_argument("--clip", action="append", default=[])
    ap.add_argument("--montage", action="store_true")
    ap.add_argument("--keep-frames", action="store_true")
    a = ap.parse_args(argv)
    run = Path(a.run).resolve()
    tri = run / "tri" if (run / "tri").exists() else run
    out = Path(a.out).resolve() if a.out else HERE.parents[4] / "tmp" / "video"
    ff = ffmpeg_exe()
    found = collect(tri)
    log = frame_log(tri)
    clips = [c for c in found if not a.clip or c in a.clip]
    if not clips:
        print(f"no clip frames under {tri}")
        return 1
    order = [c["clip"] for c in gen_video_tests.CLIPS]
    clips.sort(key=lambda c: order.index(c) if c in order else len(order))
    made = []
    bad = 0
    for clip in clips:
        passes = found[clip]
        off, on = set(passes.get("off", {})), set(passes.get("on", {}))
        if not off or off != on:
            print(f"{clip}: frames do not match (off {len(off)}, on {len(on)}; missing in on: "
                  f"{sorted(off - on)[:10]}, missing in off: {sorted(on - off)[:10]})")
            bad += 1
            continue
        cdir = out / clip
        cdir.mkdir(parents=True, exist_ok=True)
        work = cdir / "_frames"
        sbs_dir, wipe_dir = compose(clip, passes, log, work)
        encode(ff, sbs_dir, cdir / f"{clip}__sbs.mp4")
        encode(ff, wipe_dir, cdir / f"{clip}__wipe.mp4")
        first = {p: log.get(f"{clip}__{p}__f{min(off):04d}", {}) for p in ("off", "on")}
        lines = [f"clip {clip}: {len(off)} frames, f{min(off):04d} to f{max(off):04d}"]
        for p in ("off", "on"):
            lines.append(f"{p} setting: {first[p].get('mode', '')}")
            lines.append(f"{p} feat: {first[p].get('feat', '')}")
        lines += [f"{f:04d} off={passes['off'][f]} on={passes['on'][f]}" for f in sorted(off)]
        (cdir / "frames.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
        if not a.keep_frames:
            shutil.rmtree(work, ignore_errors=True)
        made.append(clip)
        print(f"{clip}: {len(off)} frames -> {cdir}")
    if a.montage and made:
        parts = []
        for clip in made:
            card = out / clip / f"{clip}__card.mp4"
            title_card(ff, clip, card)
            parts += [card, out / clip / f"{clip}__wipe.mp4"]
        lst = out / "montage.txt"
        lst.write_text("".join(f"file '{p.as_posix()}'\n" for p in parts), encoding="utf-8")
        subprocess.run([ff, "-y", "-loglevel", "error", "-f", "concat", "-safe", "0", "-i", str(lst), "-c", "copy",
                        str(out / "montage.mp4")], check=True)
        print(f"montage: {out / 'montage.mp4'}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
