"""Mix the v2 narration onto the montage: per-line placement, -16 LUFS, an SRT
and a subtitled copy.

    python mix_vo.py [--out <montage folder>] [--wav-dir <dir>] [--lufs -16] [--plan-only] [--burn-draft]

Reads <out>/timeline.json and <out>/uslu_dur_v2.mp4 (make_video_v2.py
--montage) and tmp/video/v2/lines.json (vo_v2.py). Line ids name their
segment: v2_00_intro -> the intro card, v2_NN_<clip> -> that clip's title card
(its window runs to the end of the clip), v2_99_outro -> the outro card.

Placement: each line starts LEAD s into its window (vo_v2.py uses the same
rule) and should end BREATH s before the window ends; a line that runs past
that is reported, and with --fit the following lines are pushed back so no two
overlap (the video is not stretched).

--plan-only    no audio: line lengths are estimated (2.3 words a second);
               writes vo_plan.txt and uslu_dur_v2_vo.draft.srt and prints the
               ffmpeg steps of a full run. --burn-draft also burns the draft
               SRT into uslu_dur_v2_draftsubs.mp4 (a check of the subtitle
               look, no sound).
full run       after the TTS rendered every line (C:/dev/local-tts/uslu_dur_vo.py
               --lines <this lines.json>, which writes <wav-dir>/<id>.wav, 24 kHz
               mono): reads each WAV's length, mixes all lines into one track
               (adelay + amix; each line levelled to -18 LUFS, a gentle peak
               limiter, the voice centred in stereo, padded to the video),
               normalises it to -16 LUFS integrated under -1.5 dBTP (two-pass
               loudnorm, linear), muxes it (video stream copied) and writes, in
               tmp/video/v2/final/:
                 uslu_dur_part1_graphics_youtube_1080p60.mp4   the master with the narration
                 uslu_dur_part1_graphics.srt                   subtitles from the real line timings
                 uslu_dur_part1_graphics_subtitled.mp4         the same with the subtitles burnt in
                 uslu_dur_part1_graphics_side_by_side_3840x1080p60.mp4   the --also montage, same mix
               and vo_plan.txt (where every line went, measured loudness) next to the montage.
Never commit the video or audio files.
"""
import argparse
import json
import re
import subprocess
import sys
import textwrap
import wave
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import make_video  # noqa: E402
import vo_v2  # noqa: E402

V2 = vo_v2.V2
WAV_DIR = Path("C:/dev/local-tts/outputs/uslu-dur-video")
LEAD = vo_v2.LEAD
BREATH = vo_v2.BREATH
HOLD = 0.25         # a subtitle stays this long after its line ends
# Every line is brought to the same loudness before the mix (the TTS takes came out between
# -26 and -12 LUFS); the whole track is then normalised to the target.
LINE_LUFS = -18.0
ROW = 46            # subtitle characters per row
# Burnt-in subtitles: an ASS file in Roboto (the game's own OFL body font, cwr_body.ttf), off-white,
# bottom-centred on the 5% safe margin, two lines at most, with a soft, wide, low-opacity dark halo
# (libass \\bord + \\blur on a translucent outline) instead of a box; libass loads the font from fontsdir.
FONT_DIR = Path("D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts")
ASS_HEAD = """[Script Info]
ScriptType: v4.00+
PlayResX: 1920
PlayResY: 1080
WrapStyle: 2
ScaledBorderAndShadow: yes

[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Sub,Roboto,46,&H00F3F1F0,&H000000FF,&H60000000,&H00000000,0,0,0,0,100,100,0.6,0,1,3,0,2,96,96,54,1

[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
"""


def windows(timeline):
    """id -> (start, end) of the line's window in the montage. trailer_v2.py writes them
    ("windows": intro, one per scene, outro); older timelines are derived from the cards."""
    if "windows" in timeline:
        return {k: tuple(v) for k, v in timeline["windows"].items()}
    seg = timeline["segments"]
    out = {}
    for i, s in enumerate(seg):
        if s["kind"] == "intro":
            out["intro"] = (s["start"], s["end"])
        elif s["kind"] == "outro":
            out["outro"] = (s["start"], s["end"])
        elif s["kind"] == "card":
            clip_end = next((x["end"] for x in seg[i + 1:] if x["kind"] == "clip" and x["clip"] == s["clip"]), s["end"])
            out[s["clip"]] = (s["start"], clip_end)
    return out


def key_of(rid):
    m = re.match(r"^v2_(\d\d)_(.+)$", rid)
    if not m:
        raise SystemExit(f"line id {rid!r} is not v2_NN_<name>")
    return m.group(2)


def wav_seconds(p: Path):
    with wave.open(str(p), "rb") as w:
        return w.getnframes() / float(w.getframerate())


def srt_time(t):
    ms = int(round(t * 1000))
    return f"{ms // 3600000:02d}:{ms // 60000 % 60:02d}:{ms // 1000 % 60:02d},{ms % 1000:03d}"


KEEP_TOGETHER = ("Cold War Assault", "Guerrilla Mode")   # never broken across subtitle rows


def cues_for(text, start, end):
    """Split a line into cues of at most two rows, time shared by words."""
    for phrase in KEEP_TOGETHER:
        text = text.replace(phrase, phrase.replace(" ", " "))   # textwrap does not break at a no-break space
    sentences = re.split(r"(?<=[.!?])\s+", text.strip())
    chunks, cur = [], ""
    for s in sentences:
        cand = (cur + " " + s).strip()
        if len(textwrap.wrap(cand, ROW)) <= 2:
            cur = cand
        else:
            if cur:
                chunks.append(cur)
            cur = s
    if cur:
        chunks.append(cur)
    total = sum(len(c.split()) for c in chunks)
    out, t = [], start
    for c in chunks:
        d = (end - start) * len(c.split()) / total
        out.append((t, t + d, "\n".join(textwrap.wrap(c, ROW))))
        t += d
    return out


def place(lines, wins, lengths, fit):
    rows, prev_end = [], 0.0
    for ln in lines:
        k = key_of(ln["id"])
        if k not in wins:
            raise SystemExit(f"{ln['id']}: no segment {k!r} in the timeline")
        a, b = wins[k]
        start = a + LEAD
        if fit and start < prev_end + 0.3:
            start = prev_end + 0.3
        d = lengths[ln["id"]]
        end = start + d
        rows.append(dict(id=ln["id"], text=ln["text"], start=round(start, 3), end=round(end, 3), dur=round(d, 3),
                         window=[a, b], over=round(end - (b - BREATH), 3)))
        prev_end = end
    return rows


def brand(text):
    """The narration spells the brand without "!" so the voice does not shout it; the subtitles keep it."""
    return text.replace("Uslu dur.", "Uslu dur!").replace("Uslu dur ", "Uslu dur! ")


def write_srt(rows, path, video_end):
    n, out = 0, []
    for i, r in enumerate(rows):
        nxt = rows[i + 1]["start"] if i + 1 < len(rows) else video_end
        end = min(r["end"] + HOLD, nxt - 0.05)
        # the narration spells the brand without "!" so the voice does not shout it; the subtitles keep it
        for a, b, txt in cues_for(brand(r["text"]), r["start"], end):
            n += 1
            out.append(f"{n}\n{srt_time(a)} --> {srt_time(b)}\n{txt}\n")
    path.write_text("\n".join(out), encoding="utf-8")


def ass_time(t):
    cs = int(round(t * 100))
    return f"{cs // 360000}:{cs // 6000 % 60:02d}:{cs // 100 % 60:02d}.{cs % 100:02d}"


def cues(rows, video_end):
    out = []
    for i, r in enumerate(rows):
        nxt = rows[i + 1]["start"] if i + 1 < len(rows) else video_end
        end = min(r["end"] + HOLD, nxt - 0.05)
        out += cues_for(brand(r["text"]), r["start"], end)
    return out


def write_ass(rows, path, video_end):
    lines = [ASS_HEAD.rstrip("\n")]
    for a, b, txt in cues(rows, video_end):
        body = txt.replace("\n", "\\N")
        lines.append(f"Dialogue: 0,{ass_time(a)},{ass_time(b)},Sub,,0,0,0,,{{\\blur9\\fad(250,250)}}{body}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def burn(ff, src: Path, rows, video_end, dst: Path, extra):
    """Burn the subtitles into a copy of src with libass, the font loaded from the guerrilla-fonts folder."""
    import shutil
    work = dst.parent / "_subs"
    shutil.rmtree(work, ignore_errors=True)
    (work / "fonts").mkdir(parents=True)
    shutil.copyfile(FONT_DIR / "cwr_body.ttf", work / "fonts" / "cwr_body.ttf")
    write_ass(rows, work / "subs.ass", video_end)
    run([ff, "-y", "-loglevel", "error", "-i", str(src.resolve()), "-vf", "ass=subs.ass:fontsdir=fonts"] + extra
        + [str(dst.resolve())], cwd=str(work))
    shutil.rmtree(work, ignore_errors=True)


def plan_text(rows, note):
    out = [note, "", f"{'id':22s} {'start':>7s} {'end':>7s} {'len':>5s}  window          over"]
    for r in rows:
        flag = "  RUNS LONG" if r["over"] > 0 else ""
        gain = f"  gain {r['gain_db']:+.1f} dB" if "gain_db" in r else ""
        out.append(f"{r['id']:22s} {r['start']:7.2f} {r['end']:7.2f} {r['dur']:5.2f}  "
                   f"{r['window'][0]:6.2f}-{r['window'][1]:6.2f} {r['over']:+5.2f}{flag}{gain}")
    return "\n".join(out) + "\n"


def run(cmd, **kw):
    print("  " + " ".join(str(c) for c in cmd)[:400])
    return subprocess.run(cmd, check=True, **kw)


def line_lufs(ff, wav):
    """Integrated loudness of one line (ffmpeg ebur128 summary)."""
    r = subprocess.run([ff, "-hide_banner", "-nostats", "-i", str(wav), "-af", "ebur128", "-f", "null", "-"],
                       capture_output=True, text=True, check=True)
    tail = r.stderr[r.stderr.rindex("Summary:"):]
    return float(re.search(r"I:\s+(-?[\d.]+) LUFS", tail).group(1))


def loudnorm_measure(ff, src, lufs):
    r = subprocess.run([ff, "-hide_banner", "-nostats", "-i", str(src), "-af",
                        f"loudnorm=I={lufs}:TP=-1.5:LRA=11:print_format=json", "-f", "null", "-"],
                       capture_output=True, text=True, check=True)
    blob = r.stderr[r.stderr.rindex("{"):r.stderr.rindex("}") + 1]
    return json.loads(blob)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(V2 / "out-split"))
    ap.add_argument("--final-dir", default=str(V2 / "final"))
    ap.add_argument("--name", default="uslu_dur_part1_graphics")
    ap.add_argument("--also", default=str(V2 / "out-sbs" / "uslu_dur_v2.mp4"),
                    help="a second montage on the same timeline (the 3840x1080 side by side) that gets the same mix")
    ap.add_argument("--lines", default=str(V2 / "lines.json"))
    ap.add_argument("--wav-dir", default=str(WAV_DIR))
    ap.add_argument("--lufs", type=float, default=-16.0)
    ap.add_argument("--plan-only", action="store_true")
    ap.add_argument("--burn-draft", action="store_true")
    ap.add_argument("--fit", action="store_true", help="push lines back so none overlap the one before")
    a = ap.parse_args(argv)
    out = Path(a.out)
    timeline = json.loads((out / "timeline.json").read_text(encoding="utf-8"))
    video = Path(timeline["video"])
    dur = timeline["duration"]
    lines = json.loads(Path(a.lines).read_text(encoding="utf-8"))
    wins = windows(timeline)
    lines = [ln for ln in lines if key_of(ln["id"]) in wins]
    ff = make_video.ffmpeg_exe()

    if a.plan_only:
        lengths = {ln["id"]: vo_v2.words(ln["text"]) / vo_v2.WPS for ln in lines}
        rows = place(lines, wins, lengths, a.fit)
        srt = out / "uslu_dur_v2_vo.draft.srt"
        write_srt(rows, srt, dur)
        note = (f"PLAN ONLY (no audio): line lengths estimated at {vo_v2.WPS} words a second. "
                f"Montage {dur:.2f} s, {len(rows)} lines, target {a.lufs} LUFS.")
        (out / "vo_plan.txt").write_text(plan_text(rows, note), encoding="utf-8")
        print(plan_text(rows, note))
        print(f"draft SRT: {srt}")
        print("a full run will: adelay+amix the WAVs -> vo_raw.wav; loudnorm pass 1 (measure) and pass 2 "
              f"(I={a.lufs}, TP=-1.5, LRA=11, linear) -> vo_norm.wav (48 kHz); mux with {video.name} (video copied, "
              "AAC 320k 48 kHz stereo); burn the SRT into a subtitled copy")
        if a.burn_draft:
            burn(ff, video, rows, dur, out / "uslu_dur_v2_draftsubs.mp4",
                 ["-c:v", "libx264", "-crf", "18", "-preset", "medium", "-pix_fmt", "yuv420p", "-an"])
            print(f"draft subtitles burnt in: {out / 'uslu_dur_v2_draftsubs.mp4'}")
        return 0

    wav_dir = Path(a.wav_dir)
    missing = [ln["id"] for ln in lines if not (wav_dir / f"{ln['id']}.wav").exists()]
    if missing:
        raise SystemExit(f"no WAV for {missing} in {wav_dir} (render them first; see the docstring)")
    lengths = {ln["id"]: wav_seconds(wav_dir / f"{ln['id']}.wav") for ln in lines}
    rows = place(lines, wins, lengths, a.fit)
    long_ = [r["id"] for r in rows if r["over"] > 0]
    if long_:
        print(f"lines running past their window less {BREATH} s: {long_} (shorten them or use --fit)")
    # 1. one VO track: every line delayed to its start, mixed, padded to the video
    raw = out / "vo_raw.wav"
    cmd = [ff, "-y", "-loglevel", "error"]
    for r in rows:
        cmd += ["-i", str(wav_dir / f"{r['id']}.wav")]
    gains = {r["id"]: LINE_LUFS - line_lufs(ff, wav_dir / f"{r['id']}.wav") for r in rows}
    for r in rows:
        r["gain_db"] = round(gains[r["id"]], 2)
    parts = [f"[{i}:a]aresample=48000,volume={gains[r['id']]:.2f}dB,"
             f"adelay=delays={int(round(r['start'] * 1000))}:all=1[d{i}]"
             for i, r in enumerate(rows)]
    mix = "".join(f"[d{i}]" for i in range(len(rows)))
    # The mono voice goes to both channels at -3 dB (the usual pan law: the loudness stays, each
    # channel peaks 3 dB lower). A gentle peak limiter on the levelled mono track then leaves room
    # for the final gain (about target - LINE_LUFS), so the loudnorm pass below can stay linear (one
    # fixed gain) under its -1.5 dBTP ceiling instead of falling back to its dynamic mode, which
    # rides the level inside the lines. With the defaults the limiter sits at -1.8 dBFS and only
    # touches the hottest syllables; the result peaks near -2.5 dBTP.
    limit_db = 0.2 - (a.lufs - LINE_LUFS)
    graph = (";".join(parts) + f";{mix}amix=inputs={len(rows)}:normalize=0:dropout_transition=0,"
             f"alimiter=limit={10 ** (limit_db / 20):.4f}:attack=5:release=50:level=false,"
             f"pan=stereo|c0=0.7071*c0|c1=0.7071*c0,apad,atrim=0:{dur}[vo]")
    run(cmd + ["-filter_complex", graph, "-map", "[vo]", "-ac", "2", "-ar", "48000", str(raw)])
    # 2. loudness: two-pass loudnorm to the target
    m = loudnorm_measure(ff, raw, a.lufs)
    norm = out / "vo_norm.wav"
    r2 = subprocess.run([ff, "-y", "-hide_banner", "-nostats", "-i", str(raw), "-af",
                         f"loudnorm=I={a.lufs}:TP=-1.5:LRA=11:measured_I={m['input_i']}:measured_TP={m['input_tp']}:"
                         f"measured_LRA={m['input_lra']}:measured_thresh={m['input_thresh']}:"
                         f"offset={m['target_offset']}:linear=true:print_format=json",
                         "-ar", "48000", "-ac", "2", str(norm)], capture_output=True, text=True, check=True)
    mode = json.loads(r2.stderr[r2.stderr.rindex("{"):r2.stderr.rindex("}") + 1]).get("normalization_type", "?")
    print(f"  loudnorm pass 2: {mode} (limiter at {limit_db:.1f} dBFS before it)")
    after = loudnorm_measure(ff, norm, a.lufs)
    # 3. mux, SRT, subtitled copy
    fin = Path(a.final_dir)
    fin.mkdir(parents=True, exist_ok=True)
    final = fin / f"{a.name}_youtube_1080p60.mp4"
    aac = ["-c:a", "aac", "-b:a", "320k", "-ar", "48000", "-ac", "2"]
    run([ff, "-y", "-loglevel", "error", "-i", str(video), "-i", str(norm), "-map", "0:v", "-map", "1:a", "-c:v",
         "copy"] + aac + ["-shortest", "-movflags", "+faststart", str(final)])
    srt = fin / f"{a.name}.srt"
    write_srt(rows, srt, dur)
    fps = str(timeline.get("fps", 60))
    burn(ff, final, rows, dur, fin / f"{a.name}_subtitled.mp4",
         ["-c:v", "libx264", "-profile:v", "high", "-crf", "17", "-preset", "medium", "-g", str(int(fps) // 2),
          "-bf", "2", "-flags", "+cgop", "-pix_fmt", "yuv420p", "-r", fps, "-c:a", "copy", "-movflags", "+faststart"])
    also = Path(a.also) if a.also else None
    if also and also.exists():
        run([ff, "-y", "-loglevel", "error", "-i", str(also), "-i", str(norm), "-map", "0:v", "-map", "1:a", "-c:v",
             "copy"] + aac + ["-shortest", "-movflags", "+faststart", str(fin / f"{a.name}_side_by_side_3840x1080p60.mp4")])
    note = (f"Montage {dur:.2f} s, {len(rows)} lines, each levelled to {LINE_LUFS} LUFS, peak limiter at "
            f"{limit_db:.1f} dBFS. Loudness before {m['input_i']} LUFS, after {after['input_i']} LUFS (target "
            f"{a.lufs}), true peak after {after['input_tp']} dBTP, loudnorm {mode}.")
    (out / "vo_plan.txt").write_text(plan_text(rows, note), encoding="utf-8")
    print(plan_text(rows, note))
    print(f"-> {final}\n-> {srt}\n-> {fin / (a.name + '_subtitled.mp4')}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
