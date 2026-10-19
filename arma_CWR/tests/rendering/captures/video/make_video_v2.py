"""Assemble the v2 comparison video: the vanilla game left, Uslu dur! right,
a title card before every clip, an intro and an outro card.

    python make_video_v2.py [--clip <clip> ...] [--layout sbs|split] [--montage] [--placeholder]
                            [--stills] [--out <dir>]

Frames (collect_frames.py puts them there, one near-lossless RGB video per side):
    tmp/video/v2/vanilla/<clip>/<clip>__vanilla.mkv   left, gamma applied
    tmp/video/v2/ud/<clip>/<clip>__ud.mkv             right, as shot
Both halves must have the same frame count (gen_video_v2.clip_frames);
--placeholder fills a missing right half with a grey "not shot yet" panel.

Layouts:
    sbs    3840x1080, the two full frames side by side (default; nothing cropped)
    split  1920x1080, a 960 px wide window of each frame side by side (16:9, the
           YouTube master); the window sits at the clip's "crop" (0 left edge,
           0.5 centre, 1 right edge; gen_video_v2's clip table, default 0.5)

Writes into <out> (default tmp/video/v2/out[-split]/):
    clips/<clip>.mp4          the clip with the "Vanilla" and "Uslu dur!" tags
    cards/<clip>__card.mp4    its title card (CARD_S seconds)
    cards/intro.mp4, cards/outro.mp4
    uslu_dur_v2.mp4           with --montage: intro, then card + clip per clip, outro (silent)
    timeline.json             start and end of every segment of the montage (mix_vo.py reads it)
    stills/<clip>__t<sec>.png with --stills: composed frames at 0.5 s, the middle and the end
60 fps (gen_video_v2.FPS), H.264 High (libx264, CRF 17, yuv420p, closed GOP of
half a second as YouTube asks), Pillow frames piped into ffmpeg
(C:\\ffmpeg\\bin\\ffmpeg.exe); every segment has the same settings, so the
montage is a stream copy.
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))

import gen_video_v2  # noqa: E402
import make_video  # noqa: E402

V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")
FPS = gen_video_v2.FPS
CRF = 17
CARD_S = 2.0
INTRO_S = 10.0
OUTRO_S = 10.0
LEFT_TAG = "Vanilla"
RIGHT_TAG = "Uslu dur!"
INTRO = ("Uslu dur!", "Trailer, part 1: graphics")
OUTRO = ("Uslu dur!", "Part 2: gameplay. Coming soon.")
BG = (10, 11, 13)
INK = (240, 238, 232)
SUB = (170, 170, 170)


class Pipe(make_video.Pipe):
    """make_video's encoder at this script's quality."""

    def __init__(self, ff, dst: Path, size):
        self.size = size
        self.p = subprocess.Popen([ff, "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s",
                                   f"{size[0]}x{size[1]}", "-framerate", str(FPS), "-i", "-", "-c:v", "libx264",
                                   "-profile:v", "high", "-crf", str(CRF), "-preset", "medium", "-g", str(FPS // 2),
                                   "-bf", "2", "-flags", "+cgop", "-pix_fmt", "yuv420p", "-r", str(FPS),
                                   "-movflags", "+faststart", str(dst)], stdin=subprocess.PIPE)


def frame_size(layout):
    return (3840, 1080) if layout == "sbs" else (1920, 1080)


class Reader:
    """Decodes a side's clip video (collect_frames.py) frame by frame as RGB images."""

    def __init__(self, ff, video: Path, size):
        self.size = size
        self.n = size[0] * size[1] * 3
        self.p = subprocess.Popen([ff, "-loglevel", "error", "-i", str(video), "-f", "rawvideo", "-pix_fmt", "rgb24",
                                   "-"], stdout=subprocess.PIPE)

    def next(self):
        buf = self.p.stdout.read(self.n)
        if len(buf) < self.n:
            return None
        return Image.frombytes("RGB", self.size, buf)

    def close(self):
        self.p.stdout.close()
        self.p.wait()


def source(side, clip, root):
    """(video, frames, size) of a side's clip, or None."""
    info = root / side / clip / "collect.json"
    if not info.exists():
        return None
    j = json.loads(info.read_text(encoding="utf-8"))
    video = Path(j["video"])
    return (video, j["frames"], tuple(j["size"])) if video.exists() else None


def centred(draw, cx, y, text, fnt, fill):
    box = draw.textbbox((0, 0), text, font=fnt)
    draw.text((cx - (box[2] - box[0]) // 2 - box[0], y), text, font=fnt, fill=fill)


def tag(draw, xy, text, fnt, accent=None):
    box = draw.textbbox(xy, text, font=fnt)
    draw.rectangle([box[0] - 14, box[1] - 9, box[2] + 14, box[3] + 9], fill=(0, 0, 0))
    if accent:
        draw.rectangle([box[0] - 14, box[1] - 9, box[0] - 9, box[3] + 9], fill=accent)
    draw.text(xy, text, font=fnt, fill=(255, 255, 255))


def placeholder(size):
    img = Image.new("RGB", size, (60, 60, 60))
    d = ImageDraw.Draw(img)
    centred(d, size[0] // 2, size[1] // 2 - 30, "Uslu dur! side not shot yet", make_video.font(56), (200, 200, 200))
    return img


def compose(a, b, layout, title, fonts, crop=0.5):
    """One output frame from the left and right 1920x1080 frames."""
    w, h = a.size
    if b.size != a.size:
        b = b.resize(a.size)
    if layout == "sbs":
        out = Image.new("RGB", (w * 2, h))
        out.paste(a, (0, 0))
        out.paste(b, (w, 0))
        half = w
    else:
        q = int(round(crop * (w - w // 2)))
        out = Image.new("RGB", (w, h))
        out.paste(a.crop((q, 0, q + w // 2, h)), (0, 0))
        out.paste(b.crop((q, 0, q + w // 2, h)), (w // 2, 0))
        half = w // 2
    d = ImageDraw.Draw(out)
    d.rectangle([half - 2, 0, half + 1, h], fill=(255, 255, 255))
    big, small = fonts
    # the tags sit in the top outer corners (the subtitles use the bottom centre)
    tag(d, (40, 30), LEFT_TAG, big)
    rb = d.textbbox((0, 0), RIGHT_TAG, font=big)
    tag(d, (out.size[0] - 40 - (rb[2] - rb[0]), 30), RIGHT_TAG, big, accent=(150, 165, 180))
    if title:
        box = d.textbbox((0, 0), title, font=small)
        tw = box[2] - box[0]
        tag(d, (half - tw // 2, 36), title, small)
    return out


def card_image(size, title, sub):
    img = Image.new("RGB", size, BG)
    d = ImageDraw.Draw(img)
    cx = size[0] // 2
    centred(d, cx, size[1] // 2 - 90, title, make_video.font(96), INK)
    d.rectangle([cx - 60, size[1] // 2 + 40, cx + 60, size[1] // 2 + 44], fill=(150, 165, 180))
    if sub:
        centred(d, cx, size[1] // 2 + 72, sub, make_video.font(40), SUB)
    return img


def write_card(ff, dst, size, title, sub, seconds):
    img = card_image(size, title, sub)
    pipe = Pipe(ff, dst, size)
    for _ in range(int(round(seconds * FPS))):
        pipe.save(img)
    pipe.close()
    return int(round(seconds * FPS))


def build_clip(ff, clip, layout, root, out, use_placeholder, stills, title_overlay=True):
    c = gen_video_v2.clip_by_name(clip)
    n = gen_video_v2.clip_frames(c)
    left, right = source("vanilla", clip, root), source("ud", clip, root)
    if not left or left[1] != n:
        print(f"{clip}: vanilla side missing or not {n} frames, skipped")
        return None
    if (not right or right[1] != n) and not use_placeholder:
        print(f"{clip}: ud side missing or not {n} frames, skipped (--placeholder previews it)")
        return None
    real = "ud" if right and right[1] == n else "placeholder"
    size = frame_size(layout)
    fonts = (make_video.font(46), make_video.font(34))
    dst = out / "clips" / f"{clip}.mp4"
    dst.parent.mkdir(parents=True, exist_ok=True)
    pipe = Pipe(ff, dst, size)
    ra = Reader(ff, left[0], left[2])
    rb = Reader(ff, right[0], right[2]) if real == "ud" else None
    ph = None
    still_at = {int(0.5 * FPS) + 1, n // 2 + 1, n} if stills else set()
    for f in range(1, n + 1):
        a = ra.next()
        b = rb.next() if rb else None
        if a is None or (rb and b is None):
            raise SystemExit(f"{clip}: a side's video ended at frame {f} of {n}")
        if b is None:
            ph = ph or placeholder(a.size)
            b = ph
        img = compose(a, b, layout, c["title"] if title_overlay else "", fonts, c.get("crop", 0.5))
        pipe.save(img)
        if f in still_at:
            sd = out / "stills"
            sd.mkdir(parents=True, exist_ok=True)
            img.save(sd / f"{clip}__t{(f - 1) / FPS:04.1f}.png")
    pipe.close()
    ra.close()
    if rb:
        rb.close()
    print(f"{clip}: {n} frames ({real} right) -> {dst}")
    return dst, n, real


def write_timeline_only(out: Path, layout):
    """The montage timeline the full run will produce (same order and lengths), no video."""
    out.mkdir(parents=True, exist_ok=True)
    seg, t = [], 0.0

    def add(kind, frames, clip="", title=""):
        nonlocal t
        seg.append(dict(kind=kind, clip=clip, title=title, start=round(t, 3), end=round(t + frames / FPS, 3),
                        frames=frames, file=""))
        t += frames / FPS

    add("intro", int(round(INTRO_S * FPS)))
    for c in gen_video_v2.CLIPS:
        add("card", int(round(CARD_S * FPS)), c["clip"], c["title"])
        add("clip", gen_video_v2.clip_frames(c), c["clip"], c["title"])
    add("outro", int(round(OUTRO_S * FPS)))
    timeline = dict(fps=FPS, layout=layout, size=list(frame_size(layout)), segments=seg,
                    video=str(out / "uslu_dur_v2.mp4"), duration=round(t, 3), planned=True)
    (out / "timeline.json").write_text(json.dumps(timeline, indent=1), encoding="utf-8")
    print(f"planned timeline {t:.1f} s ({len(seg)} segments) -> {out / 'timeline.json'}")
    return 0


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", action="append", default=[])
    ap.add_argument("--layout", choices=("sbs", "split"), default="sbs")
    ap.add_argument("--root", default=str(V2), help="folder holding vanilla/ and ud/")
    ap.add_argument("--out", default="")
    ap.add_argument("--montage", action="store_true")
    ap.add_argument("--placeholder", action="store_true")
    ap.add_argument("--stills", action="store_true")
    ap.add_argument("--no-title-overlay", action="store_true")
    ap.add_argument("--timeline-only", action="store_true",
                    help="write <out>/timeline.json for the whole clip table without encoding anything "
                         "(mix_vo.py --plan-only can then check the narration timing)")
    a = ap.parse_args(argv)
    root = Path(a.root)
    out = Path(a.out) if a.out else V2 / ("out" if a.layout == "sbs" else "out-split")
    if a.timeline_only:
        return write_timeline_only(out, a.layout)
    out.mkdir(parents=True, exist_ok=True)
    ff = make_video.ffmpeg_exe()
    size = frame_size(a.layout)
    clips = [c["clip"] for c in gen_video_v2.CLIPS if not a.clip or c["clip"] in a.clip]
    built = []
    for clip in clips:
        r = build_clip(ff, clip, a.layout, root, out, a.placeholder, a.stills, not a.no_title_overlay)
        if r:
            built.append((clip, r))
    cards = out / "cards"
    cards.mkdir(parents=True, exist_ok=True)
    timeline = dict(fps=FPS, layout=a.layout, size=list(size), segments=[])
    t = 0.0
    parts = []

    def add(kind, path, frames, clip="", title=""):
        nonlocal t
        dur = frames / FPS
        timeline["segments"].append(dict(kind=kind, clip=clip, title=title, start=round(t, 3),
                                          end=round(t + dur, 3), frames=frames, file=str(path)))
        parts.append(path)
        t += dur

    if a.montage:
        nf = write_card(ff, cards / "intro.mp4", size, INTRO[0], INTRO[1], INTRO_S)
        add("intro", cards / "intro.mp4", nf)
    for clip, (dst, n, real) in built:
        c = gen_video_v2.clip_by_name(clip)
        card = cards / f"{clip}__card.mp4"
        nf = write_card(ff, card, size, c["title"], f"{LEFT_TAG}   |   {RIGHT_TAG}", CARD_S)
        if a.montage:
            add("card", card, nf, clip, c["title"])
            add("clip", dst, n, clip, c["title"])
            timeline["segments"][-1]["right"] = real
    if a.montage and built:
        nf = write_card(ff, cards / "outro.mp4", size, OUTRO[0], OUTRO[1], OUTRO_S)
        add("outro", cards / "outro.mp4", nf)
        lst = out / "montage.txt"
        lst.write_text("".join(f"file '{Path(p).as_posix()}'\n" for p in parts), encoding="utf-8")
        mont = out / "uslu_dur_v2.mp4"
        subprocess.run([ff, "-y", "-loglevel", "error", "-f", "concat", "-safe", "0", "-i", str(lst), "-c", "copy",
                        "-movflags", "+faststart", str(mont)], check=True)
        timeline["video"] = str(mont)
        timeline["duration"] = round(t, 3)
        (out / "timeline.json").write_text(json.dumps(timeline, indent=1), encoding="utf-8")
        print(f"montage {t:.1f} s -> {mont}")
    return 0 if built else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
