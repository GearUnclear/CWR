"""Lay a background music bed under a narrated trailer run.

    python add_music.py --voice <wav or mp4 with the voice> --music <track> --name <stem> [--subs owner] [--sbs]

The music is background: levelled to BED_LUFS on its own, faded in and out, and ducked further under the
voice (sidechain); the voice is untouched. The mix is levelled to -16 LUFS integrated, true peak under
-1.5 dBTP, and muxed onto the fog-fixed montage (video copied). Writes to tmp/video/v2/final/:
<stem>_youtube_1080p60.mp4, <stem>_preview_720p60.mp4, and with --subs owner also <stem>_subtitled.mp4
and <stem>.srt (the owner's own words), with --sbs <stem>_side_by_side_3840x1080p60.mp4.
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

FF = "C:/ffmpeg/bin/ffmpeg.exe"
V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")
SPLIT = V2 / "out-split" / "uslu_dur_v2.mp4"
SBS = V2 / "out-sbs" / "uslu_dur_v2.mp4"
FINAL = V2 / "final"
DUR = 133.4
BED_LUFS = -26.0          # the music alone (intro, pauses, closing card)
FADE_IN, FADE_OUT = 2.0, 4.0
# ducking under the voice: the voice sits near -19 dBFS RMS, so about 8 dB more off the bed while it talks
DUCK = "sidechaincompress=threshold=0.03:ratio=4:attack=60:release=700:knee=4"


def run(cmd, **kw):
    print(" ".join(str(c) for c in cmd), flush=True)
    return subprocess.run([str(c) for c in cmd], check=True, **kw)


def loudness(src, af=""):
    chain = (af + "," if af else "") + "loudnorm=I=-16:TP=-1.5:LRA=11:print_format=json"
    p = run([FF, "-hide_banner", "-i", src, "-af", chain, "-f", "null", "-"], capture_output=True, text=True)
    a = p.stderr.rindex("{")
    return json.loads(p.stderr[a:p.stderr.index("}", a) + 1])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--voice", required=True)
    ap.add_argument("--music", required=True)
    ap.add_argument("--name", required=True)
    ap.add_argument("--subs", default="")
    ap.add_argument("--sbs", action="store_true")
    o = ap.parse_args()
    work = Path(__file__).resolve().parent / o.name
    work.mkdir(exist_ok=True)

    voice = work / "voice.wav"
    run([FF, "-y", "-loglevel", "error", "-i", o.voice, "-vn", "-ac", "2", "-ar", "48000", "-t", str(DUR),
         "-c:a", "pcm_s24le", voice])
    trim = f"atrim=0:{DUR},asetpts=PTS-STARTPTS,aresample=48000"
    mi = float(loudness(o.music, trim)["input_i"])
    bed = work / "bed.wav"
    run([FF, "-y", "-loglevel", "error", "-i", o.music, "-af",
         f"{trim},volume={BED_LUFS - mi:.2f}dB,afade=t=in:d={FADE_IN},afade=t=out:st={DUR - FADE_OUT}:d={FADE_OUT}",
         "-c:a", "pcm_s24le", bed])
    raw = work / "mix_raw.wav"
    run([FF, "-y", "-loglevel", "error", "-i", bed, "-i", voice, "-filter_complex",
         f"[1:a]asplit=2[sc][v];[0:a][sc]{DUCK}[ducked];[ducked][v]amix=inputs=2:normalize=0:duration=longest[m]",
         "-map", "[m]", "-c:a", "pcm_s24le", raw])
    m = loudness(raw, "alimiter=limit=0.8:attack=5:release=60:level=false")
    ln = ("alimiter=limit=0.8:attack=5:release=60:level=false,loudnorm=I=-16:TP=-1.5:LRA=11:linear=true:"
          f"measured_I={m['input_i']}:measured_TP={m['input_tp']}:measured_LRA={m['input_lra']}:"
          f"measured_thresh={m['input_thresh']}:offset={m['target_offset']}")
    mix = work / "mix.wav"
    run([FF, "-y", "-loglevel", "error", "-i", raw, "-af", ln + ",aresample=48000", "-c:a", "pcm_s24le", mix])
    print("music alone", mi, "LUFS; mix pass 1", m, flush=True)

    FINAL.mkdir(exist_ok=True)
    outs = [(SPLIT, FINAL / f"{o.name}_youtube_1080p60.mp4")]
    if o.sbs:
        outs.append((SBS, FINAL / f"{o.name}_side_by_side_3840x1080p60.mp4"))
    for vid, dst in outs:
        run([FF, "-y", "-loglevel", "error", "-i", vid, "-i", mix, "-map", "0:v", "-map", "1:a", "-c:v", "copy",
             "-c:a", "aac", "-b:a", "320k", "-ar", "48000", "-t", str(DUR), "-movflags", "+faststart", dst])
    master = outs[0][1]
    src_for_preview = master
    if o.subs == "owner":
        sys.path.insert(0, str(V2 / "vo_owner" / "take1"))
        sys.path.insert(0, "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
        import mix_vo
        from make_take1 import ROWS
        rows = [dict(start=a, end=b, text=t) for a, b, t in ROWS]
        mix_vo.write_srt(rows, FINAL / f"{o.name}.srt", DUR)
        subbed = FINAL / f"{o.name}_subtitled.mp4"
        mix_vo.burn(FF, master, rows, DUR, subbed,
                    ["-c:v", "libx264", "-crf", "18", "-preset", "medium", "-pix_fmt", "yuv420p", "-c:a", "copy"])
        src_for_preview = subbed
    run([FF, "-y", "-loglevel", "error", "-i", src_for_preview, "-vf", "scale=1280:720:flags=lanczos",
         "-c:v", "libx264", "-preset", "medium", "-crf", "25", "-r", "60", "-c:a", "aac", "-b:a", "160k",
         "-movflags", "+faststart", FINAL / f"{o.name}_preview_720p60.mp4"])
    print("done", flush=True)


if __name__ == "__main__":
    main()
