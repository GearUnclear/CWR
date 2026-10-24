"""Uslu dur! trailer, part 1 (graphics), YouTube Shorts: captions, the music mixes and the finished files.

    python short_captions.py [--sets owner,sarah,lily] [--montage <dir>] [--ass-only] [--no-preview]

Reads <montage>/timeline.json and <montage>/uslu_dur_v2.mp4 (trailer_v2.py --layout short, default
tmp/video/v2/out-short). For each voice set it writes a vertical ASS (PlayResX 1080, PlayResY 1920) and
burns it into the montage with the set's music mix (tmp/video/v2/vo_owner/music/<name>/mix.wav, voice
and background music, already -16 LUFS; used as it is, AAC 320k 48 kHz stereo), into tmp/video/v2/final/:
    uslu_dur_part1_graphics_<set>_short_1080x1920p60.mp4           (H.264 High, CRF 17, yuv420p, 60 fps)
    uslu_dur_part1_graphics_<set>_short_preview_720x1280p60.mp4    (a phone preview, two-pass, under 30 MB)

The captions: Roboto (the trailer's body font, cwr_body.ttf), off-white with a soft dark halo and
shadow, no box, 46 px, at most two rows of about 32 characters, centred in the black band between the
two frames. While a card is up (the intro and the closing card are centred on the whole canvas, over
the band) the caption sits just under the card instead and glides into the band as the card fades
(or out of it as the closing card comes up), so text never lies over the card.
    owner        his own words and times: ROWS in tmp/video/v2/vo_owner/take1/make_take1.py
    sarah, lily  tmp/video/v2/final/uslu_dur_part1_graphics_<set>.srt, re-wrapped for the narrow
                 width (their times kept; a cue too long for two rows is split, its time shared)
Work files go to tmp/video/v2/vo_owner/short/<set>/. Never commit the video or audio files.
"""
import argparse
import importlib.util
import re
import shutil
import subprocess
import sys
from pathlib import Path

from PIL import ImageFont

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import make_video  # noqa: E402
import mix_vo  # noqa: E402
import trailer_v2 as T  # noqa: E402

V2 = T.V2
FINAL = V2 / "final"
WORK = V2 / "vo_owner" / "short"
MUSIC = V2 / "vo_owner" / "music"
STEM = "uslu_dur_part1_graphics"
SETS = {
    "owner": dict(mix=MUSIC / f"{STEM}_owner_take1_music" / "mix.wav"),
    "sarah": dict(mix=MUSIC / f"{STEM}_sarah_music" / "mix.wav", srt=FINAL / f"{STEM}_sarah.srt"),
    "lily": dict(mix=MUSIC / f"{STEM}_lily_music" / "mix.wav", srt=FINAL / f"{STEM}_lily.srt"),
}
OWNER_ROWS = V2 / "vo_owner" / "take1" / "make_take1.py"

ROW = 33                 # characters per caption row (about 32)
SIZE = 46                # px on the 1080x1920 canvas
SPACING = 0.6
MARGIN = 54              # the canvas' 5% side margin; no row may come nearer the edge
SLOT_GAP = 36            # under a card: the caption block's top this far below the card's ink
NBSP = " "
KEEP_TOGETHER = ("Uslu dur!", "Arma: Cold War Assault", "Guerrilla Mode", "Arma 3", "Lost Brothers Reloaded mod",
                 "shell burst", "light shafts", "breaks out", "come through")
# a break right after one of these leaves it hanging at the end of a row or cue
LEADING = {"a", "an", "the", "and", "or", "but", "of", "to", "for", "in", "on", "at", "with", "by", "that", "is",
           "are", "be", "been", "have", "has", "had", "will", "can", "i", "we", "we're", "you", "it's", "this", "some",
           "every", "its", "their", "no", "might", "would", "could", "should", "must", "we'll", "you'll",
           "they're", "you're", "we've", "i'll", "it'll"}
# a break right before one of these falls between two phrases
JOINING = {"and", "but", "or", "so", "of", "to", "for", "in", "on", "at", "with", "by", "that", "through", "into",
           "across", "under", "from", "about", "as"}
ASS_HEAD = f"""[Script Info]
ScriptType: v4.00+
PlayResX: {T.SW}
PlayResY: {T.SH}
WrapStyle: 2
ScaledBorderAndShadow: yes

[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Sub,Roboto,{SIZE},&H00F3F1F0,&H000000FF,&H60000000,&H80000000,0,0,0,0,100,100,{SPACING},0,1,3,1.5,5,{MARGIN},{MARGIN},0,1

[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
"""


# ---------------------------------------------------------------- cue text
def sentence_end(prev, nxt):
    """"Uslu dur! is ..." does not end a sentence; "Uslu dur! Part two ..." does."""
    return nxt is None or prev[-1] in ".?" or (prev[-1] == "!" and nxt[0].isupper())


def brk_cost(prev, nxt):
    """How good a break after word prev (before word nxt) is: 0 a sentence end, 3 a clause end, 6 before
    a joining word, 10 inside a phrase, 14 after a word that should not end a row."""
    if sentence_end(prev, nxt):
        return 0
    if prev[-1] in ",;:":
        return 3
    if prev.lower() in LEADING:
        return 14
    if nxt.lower() in JOINING:
        return 6
    return 10


def rows_of(words):
    """The best one- or two-row layout of these words (each row at most ROW characters) and the cost
    of its row break, or None."""
    one = " ".join(words)
    if len(one) <= ROW:
        return [one], 0.0
    best = None
    for k in range(1, len(words)):
        r1, r2 = " ".join(words[:k]), " ".join(words[k:])
        if len(r1) > ROW or len(r2) > ROW:
            continue
        b = brk_cost(words[k - 1], words[k])
        cost = max(len(r1), len(r2)) + (b - 10)
        if best is None or cost < best[0]:
            best = (cost, [r1, r2], b)
    return (best[1], {0: 0.0, 3: 0.0, 6: 1.2, 10: 2.8}.get(best[2], 8.0)) if best else None


def split_text(text):
    """Split text into few, well-broken cues of at most two rows (a small dynamic programme): breaks at
    sentence and clause ends are cheap, breaks inside a phrase dear, a cue that runs on from one
    sentence into half of the next dearer still."""
    for p in KEEP_TOGETHER:
        text = text.replace(p, p.replace(" ", NBSP))
    words = [w for w in text.split(" ") if w]          # str.split() would also split at the no-break spaces
    n = len(words)
    best = [None] * (n + 1)
    best[0] = (0, 0, None)
    for j in range(1, n + 1):
        for i in range(j):
            if best[i] is None:
                continue
            fit = rows_of(words[i:j])
            if fit is None:
                continue
            rows, row_cost = fit
            nxt = words[j] if j < n else None
            c = best[i][0] + 4 + row_cost + (1.5 * brk_cost(words[j - 1], nxt) if j < n else 0)
            chars = len(" ".join(words[i:j]))
            c += abs(chars - 44) * 0.05
            if chars < 12 and (i > 0 or j < n):
                c += 10
            inner = any(sentence_end(words[k], words[k + 1]) for k in range(i, j - 1))
            if inner and j < n and brk_cost(words[j - 1], nxt) >= 6:   # into half a phrase of the next sentence
                c += 6
            if best[j] is None or c < best[j][0]:
                best[j] = (c, i, rows)
    if best[n] is None:
        raise SystemExit(f"cannot fit {text!r} into rows of {ROW}")
    out, j = [], n
    while j > 0:
        _, i, rows = best[j]
        out.append([r.replace(NBSP, " ") for r in rows])
        j = i
    return out[::-1]


def timed(a, b, text):
    """(start, end, rows) cues for text shown from a to b, the time shared by characters."""
    parts = split_text(mix_vo.brand(text))
    total = sum(len(" ".join(r)) for r in parts)
    out, t = [], a
    for r in parts:
        d = (b - a) * len(" ".join(r)) / total
        out.append((t, t + d, r))
        t += d
    return out


def owner_cues(video_end):
    spec = importlib.util.spec_from_file_location("make_take1", OWNER_ROWS)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    rows = mod.ROWS
    out = []
    for i, (a, b, text) in enumerate(rows):
        nxt = rows[i + 1][0] if i + 1 < len(rows) else video_end
        out += timed(a, min(b + mix_vo.HOLD, nxt - 0.05), text)
    return out


def srt_seconds(s):
    h, m, rest = s.strip().split(":")
    sec, ms = rest.split(",")
    return int(h) * 3600 + int(m) * 60 + int(sec) + int(ms) / 1000.0


def srt_cues(path):
    out = []
    for block in re.split(r"\n\s*\n", path.read_text(encoding="utf-8").strip()):
        lines = [x for x in block.splitlines() if x.strip()]
        k = next(i for i, x in enumerate(lines) if "-->" in x)
        a, b = (srt_seconds(x) for x in lines[k].split("-->"))
        out += timed(a, b, " ".join(x.strip() for x in lines[k + 1:]))
    return out


# ---------------------------------------------------------------- placement
def place(cues, timeline):
    """[(a, b, rows, where)], where ("pos", y) or ("move", y0, y1, t0, t1), t0 and t1 in seconds.
    Captions sit in the middle of the band, except around the cards (centred on the canvas, over the
    band): under the intro card until it has faded completely (a cue change that falls inside its
    fade waits for its end, so no caption meets the fading card); on the closing card a caption that
    is up while the last scene dips out glides from the band to under the card in the dark of the dip,
    and later ones sit under the card."""
    g = timeline["short"]
    band = round((g["band"][0] + g["band"][1]) / 2.0)
    lh = SIZE * 1.172                                     # Roboto's line height at this size (libass)
    intro = g["cards"]["intro"]
    i0, i1 = intro["fade_out"]
    slot_i = round(intro["ink"][1] + SLOT_GAP + lh)
    cues = [[round(a, 2), round(b, 2), rows] for a, b, rows in cues]
    for k in range(len(cues) - 1):
        x = cues[k][1]
        if i0 < x < i1 and abs(cues[k + 1][0] - x) < 0.006:
            cues[k][1] = cues[k + 1][0] = i1
    closing = next((s for s in timeline["segments"] if s["kind"] == "closing"), None)
    dark = None
    if closing is not None and "outro" in g["cards"]:
        tin = closing.get("transition_in")
        dark = closing["start"] + (tin[1] / timeline["fps"] / 2 if tin else 0.0)   # the dip's black
        g0, g1 = round(dark - 0.35, 2), round(dark + 0.25, 2)
        slot_o = round(g["cards"]["outro"]["ink"][1] + SLOT_GAP + lh)
    out = []
    for a, b, rows in cues:
        if a < i1 - 0.006:
            if b <= i1 + 0.006:
                out.append((a, b, rows, ("pos", slot_i)))
            else:   # one cue across the end of the fade: under the card, then in the band
                out.append((a, i1, rows, ("pos", slot_i)))
                out.append((i1, b, rows, ("pos", band)))
        elif dark is not None and b > g0:
            if a >= g0:
                out.append((a, b, rows, ("pos", slot_o)))
            elif b > g1:
                out.append((a, b, rows, ("move", band, slot_o, g0, g1)))
            else:
                out.append((a, b, rows, ("pos", band)))
        else:
            out.append((a, b, rows, ("pos", band)))
    return out


def ass_time(t):
    cs = int(round(t * 100))
    return f"{cs // 360000}:{cs // 6000 % 60:02d}:{cs // 100 % 60:02d}.{cs % 100:02d}"


def write_ass(cues, timeline, path):
    g = timeline["short"]
    band = round((g["band"][0] + g["band"][1]) / 2.0)
    x = T.SW // 2
    lines = [ASS_HEAD.rstrip("\n")]
    report = []
    for a_r, b_r, rows, p in place(cues, timeline):
        if p[0] == "move":
            _, y0, y1, t0, t1 = p
            pos = f"\\move({x},{y0},{x},{y1},{int(round((t0 - a_r) * 1000))},{int(round((t1 - a_r) * 1000))})"
            where = f"glide {y0}->{y1}"
        else:
            pos = f"\\pos({x},{p[1]})"
            where = "band" if p[1] == band else f"under card y={p[1]}"
        body = "\\N".join(rows)
        lines.append(f"Dialogue: 0,{ass_time(a_r)},{ass_time(b_r)},Sub,,0,0,0,,{{\\an5{pos}\\blur7\\fad(250,250)}}{body}")
        report.append(f"{a_r:7.2f} {b_r:7.2f}  {where:22s} {' / '.join(rows)}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return report


def check_widths(cues):
    """Every row must fit between the side margins at the caption size (letter spacing included)."""
    f = ImageFont.truetype(str(T.BODY_TTF), SIZE)
    widest = max((f.getlength(r) + SPACING * (len(r) - 1), r) for _a, _b, rows in cues for r in rows)
    room = T.SW - 2 * MARGIN
    if widest[0] > room:
        raise SystemExit(f"caption row too wide ({widest[0]:.0f} > {room} px): {widest[1]!r}")
    return widest


# ---------------------------------------------------------------- encode
def run(cmd, **kw):
    print("  " + " ".join(str(c) for c in cmd)[:500], flush=True)
    return subprocess.run([str(c) for c in cmd], check=True, **kw)


def finish(ff, name, montage, mix, ass, work, duration, fps, preview):
    fonts = work / "fonts"
    fonts.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(T.BODY_TTF, fonts / "cwr_body.ttf")
    final = FINAL / f"{STEM}_{name}_short_1080x1920p60.mp4"
    run([ff, "-y", "-loglevel", "error", "-i", montage.resolve(), "-i", mix.resolve(), "-map", "0:v", "-map", "1:a",
         "-vf", f"ass={ass.name}:fontsdir=fonts", "-c:v", "libx264", "-profile:v", "high", "-crf", "17",
         "-preset", "medium", "-g", str(fps // 2), "-bf", "2", "-flags", "+cgop", "-pix_fmt", "yuv420p",
         "-r", str(fps), "-c:a", "aac", "-b:a", "320k", "-ar", "48000", "-ac", "2", "-t", f"{duration}",
         "-movflags", "+faststart", final.resolve()], cwd=str(work))
    if not preview:
        return final, None
    prev = FINAL / f"{STEM}_{name}_short_preview_720x1280p60.mp4"
    common = ["-vf", "scale=720:1280:flags=lanczos", "-c:v", "libx264", "-profile:v", "high", "-preset", "medium",
              "-b:v", "1500k", "-g", str(fps), "-pix_fmt", "yuv420p", "-r", str(fps), "-passlogfile",
              str(work / "x264pass")]
    run([ff, "-y", "-loglevel", "error", "-i", final, "-map", "0:v"] + common + ["-pass", "1", "-an", "-f", "null", "-"])
    run([ff, "-y", "-loglevel", "error", "-i", final, "-i", mix, "-map", "0:v", "-map", "1:a"] + common
        + ["-pass", "2", "-c:a", "aac", "-b:a", "160k", "-ar", "48000", "-ac", "2", "-t", f"{duration}",
           "-movflags", "+faststart", prev])
    for p in work.glob("x264pass*"):
        p.unlink()
    return final, prev


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sets", default="owner,sarah,lily")
    ap.add_argument("--montage", default=str(V2 / "out-short"))
    ap.add_argument("--ass-only", action="store_true", help="write and report the captions only")
    ap.add_argument("--no-preview", action="store_true")
    a = ap.parse_args(argv)
    montage_dir = Path(a.montage)
    import json
    timeline = json.loads((montage_dir / "timeline.json").read_text(encoding="utf-8"))
    if timeline.get("layout") != "short" or "short" not in timeline:
        raise SystemExit(f"{montage_dir}: not a --layout short montage")
    duration, fps = timeline["duration"], int(timeline["fps"])
    montage = Path(timeline["video"])
    ff = make_video.ffmpeg_exe()
    for name in [s for s in a.sets.split(",") if s]:
        if name not in SETS:
            raise SystemExit(f"no voice set {name!r} ({', '.join(SETS)})")
        spec = SETS[name]
        cues = owner_cues(duration) if name == "owner" else srt_cues(spec["srt"])
        widest = check_widths(cues)
        work = WORK / name
        work.mkdir(parents=True, exist_ok=True)
        ass = work / "subs.ass"
        report = write_ass(cues, timeline, ass)
        print(f"{name}: {len(cues)} cues, widest row {widest[0]:.0f} px {widest[1]!r} -> {ass}")
        print("\n".join("  " + r for r in report))
        if a.ass_only:
            continue
        if not spec["mix"].exists():
            raise SystemExit(f"{name}: no music mix at {spec['mix']}")
        if not montage.exists():
            raise SystemExit(f"no montage at {montage}")
        final, prev = finish(ff, name, montage, spec["mix"], ass, work, duration, fps, not a.no_preview)
        print(f"{name}: -> {final}" + (f"\n{name}: -> {prev}" if prev else ""), flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
