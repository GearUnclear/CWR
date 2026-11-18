"""Writes <UD_PROMO_DIR>/edit-notes.md from work/edit/edl.json, night_gate.json and thirds_audit.json (run after
build.py). thirds_audit.json is written by hand while reading final_thirds_sheet.jpg: {"summary": "...",
"notes": {"<clip> (shot n)": "..."}, "weak": ["..."]}.
"""
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from promopaths import AUDIO, FP, PROMO  # noqa: E402

ROOT = PROMO
ED = ROOT / "work" / "edit"
E = json.loads((ED / "edl.json").read_text())
G = json.loads((ED / "night_gate.json").read_text()) if (ED / "night_gate.json").exists() else []
T = json.loads((ED / "thirds_audit.json").read_text()) if (ED / "thirds_audit.json").exists() else {}


def probe(p):
    if not p.exists():
        return None
    return json.loads(subprocess.run([FP, "-v", "error", "-show_entries",
                                      "stream=codec_type,duration:format=size", "-of", "json", str(p)],
                                     capture_output=True, text=True).stdout)


main, ph = probe(ROOT / "ud-lebanon80-promo-1080p.mp4"), probe(ROOT / "ud-lebanon80-promo-720p-phone.mp4")
vd = next(s["duration"] for s in main["streams"] if s["codec_type"] == "video")
ad = next(s["duration"] for s in main["streams"] if s["codec_type"] == "audio")
size = int(main["format"]["size"])
psize = int(ph["format"]["size"]) if ph else 0
shots = E["shots"]
beam = next(s for s in shots if s["clip"] == "c65" and 100 < s["t"] < 110)
dz = next(s for s in shots if s["clip"] == "c98")
lk = E["locks"]

out = f"""# Lebanon80 promo: edit notes (reshoot cut)

Output:
- `ud-lebanon80-promo-1080p.mp4`: 1920x1080, 30 fps, H.264 High CRF 18, yuv420p tagged bt709 / tv,
  AAC 320k 48 kHz, faststart, metadata stripped, title "{E['title']}". Video {vd} s ({E['frames']}
  frames), audio {ad} s, {size:,} bytes. {len(shots)} shots ({len(shots) - 1} cuts).
- `ud-lebanon80-promo-720p-phone.mp4`: 1280x720, two-pass x264 at 1250k, AAC 160k, {psize:,} bytes
  ({psize / 2**20:.2f} MiB).

The sources are the reshoot (88 clips, 2560x1440, `clips\\shotlist.md`). Every shot is downscaled to
1080p with lanczos; c24, the journal (c72) and c06n are reframed with a 1.33x crop that keeps
1920x1080 source pixels.

## How it is built

The tools are in the repo at `arma_CWR/experiments/lebanon80-promo/edit/` (see the README there); their
output is in `work\\edit\\`:

- `build.py`: the cut list (CUTS), the text cards (TEXT), the grade and the lock points live at the top
  of this one script. It writes `edl.json`, renders each shot to an exact frame count (temporary CRF 12
  segments in `work\\edit\\seg`), concatenates them, overlays the cards, muxes the untouched WAV, checks
  the stream durations, runs the night luma gate, writes `final_sheet.jpg` and `final_thirds_sheet.jpg`
  (the middle frame of every shot, the second with the thirds grid) and the 720p phone copy.
- `make_notes.py`: writes this file. `scout_sheets.py`, `frames_at.py`: scouting and checking stills.

Render command (UD_PROMO_DIR set to this folder, ffmpeg on PATH or FFMPEG / FFPROBE set, Python 3 with
numpy and Pillow):

    python build.py              # full build; deletes the segments afterwards
    python build.py --reuse      # keeps the segments and re-renders only the shots that changed
    python make_notes.py

The final encode inside build.py, in short:

    ffmpeg -f concat -safe 0 -i seg\\concat.txt -i {AUDIO.name}
           -loop 1 -framerate 30 -t <dur> -i cards\\<card>.png ...   (one per text card)
           -filter_complex "<fade in/out on each card's alpha, overlay at its time>"
           -map [vout] -map 1:a -frames:v {E['frames']} -c:v libx264 -preset slow -profile:v high -crf 18
           -pix_fmt yuv420p -r 30 -colorspace bt709 -color_primaries bt709 -color_trc bt709 -color_range tv
           -c:a aac -b:a 320k -ar 48000 -map_metadata -1 -map_chapters -1
           -metadata title="{E['title']}" -movflags +faststart ud-lebanon80-promo-1080p.mp4

The audio is the whole instrumental, unaltered (no trim, fade, gain or ducking). The video is
{E['frames']} frames ({E['frames'] / 30:.4f} s) against {E['audio_duration']} s of audio: the streams differ
by {abs(float(vd) - float(ad)) * 1000:.1f} ms, under one frame.

## Grading rules (night-diagnosis.md, Edit checklist)

- True-night shots (the moon gate passed in every take: c06n, c13n, c23n, c35n, c36n, c61, c62, c63,
  c67, c85, c85b, c86, c87, c88) get gamma 1.000, contrast 1.00, no colour balance and no grain, and
  are left out of the exposure match. Chain: `{E['grade_night']}`.
- Every other shot: `{E['grade_day']}`. G is an exposure match that may only darken: the shot's mean
  luma is moved 35 % of the way toward 0.40 and G is clamped to 0.88 .. 1.00, so a dark dusk shot keeps
  G 1.000 and a bright day shot comes down a little.
- The curve's first point is 0/0 (black stays black), grain c0s=1 at most and none at night, the
  vignette stays. No normalize, histeq or auto levels anywhere. Fades to and from black only.
- The sources are bt601 limited range (encode_promo.py converts rgb24 with the default matrix); every
  segment decodes them as bt601 / tv and is converted to bt709 / tv with
  `scale=out_color_matrix=bt709:out_range=tv`. Every x264 call tags `-colorspace bt709 -color_primaries
  bt709 -color_trc bt709 -color_range tv`.
- Dusk shots (c27, c82, c43b, c44, c96, c95) sit between golden hour and night, never in a night slot.

## Night luma gate

After the final encode build.py runs signalstats over every night shot, in the output (the shot's frames)
and in its source (the same frames, cropped and scaled the same way, no grade). The build fails when the
output's 10th percentile of luma (YLOW, the mean over the shot) is above the source's plus 1.
Y codes are limited range (black is 16).

| # | Time | Clip | Source YLOW | Output YLOW | Diff | Output YLOW max | Result |
|---|---|---|---|---|---|---|---|
""" + "".join(f"| {r['n']} | {r['t']:.3f} | {r['clip']} | {r['src_ylow']:.2f} | {r['out_ylow']:.2f} | {r['diff']:+.2f} | {r['out_ylow_max']:.1f} | {'PASS' if r['pass_'] else 'FAIL'} |\n" for r in G) + f"""
Every night shot comes out at or under its source: the grade only darkens the dark end (the curve,
the vignette, the end fade), it never lifts it.

## Lock points

- **Beam (c65)**: the listed fire time is 4.10 s; the measured first frame of the flash and the beam is
  clip frame {lk['c65_fire_frame']} ({lk['c65_fire_frame'] / 30:.3f} s; mean luma 68.6 to 77.2 in one frame). Shot
  {beam['n']} starts at the 103.61 cut (output frame {round(beam['t'] * 30)}) on clip frame
  {beam['clip_in_frame']} and runs unbroken, so the fire frame plays at output frame
  {round(beam['t'] * 30) + lk['c65_fire_frame'] - beam['clip_in_frame']} = {(round(beam['t'] * 30) + lk['c65_fire_frame'] - beam['clip_in_frame']) / 30:.3f} s,
  on the 107.40 drop. The drop then cuts on half bars to c64 (fire 4.33 s), c67 (fire 4.37 s, true night)
  and c66 (fire 4.27 s, impact about 4.8 s).
- **Dolly zoom (c98)**: the listed run time is 4.35 s; by frames the fighter breaks into his run on clip
  frame {lk['c98_run_frame']} ({lk['c98_run_frame'] / 30:.3f} s). Shot {dz['n']} starts at the 124.46 cut (output frame
  {round(dz['t'] * 30)}) on clip frame {dz['clip_in_frame']}, so the zoom plays across the riser and the run lands
  on output frame {round(dz['t'] * 30) + lk['c98_run_frame'] - dz['clip_in_frame']} = {(round(dz['t'] * 30) + lk['c98_run_frame'] - dz['clip_in_frame']) / 30:.3f} s, the frame
  nearest the 128.247 drop. He is out of frame by about 129.6 s and the shot cuts on the 129.67 beat.
- Helicopter shots: c93 and c91 in the escort break, c96 and c94 in drop 3, c95 in the final montage.

## Thirds audit

`final_thirds_sheet.jpg` (one frame per cut with the thirds grid) was read shot by shot.
""" + T.get("summary", "") + "\n\n" + "".join(f"- {k}: {v}\n" for k, v in T.get("notes", {}).items()) + """
## Text

Oswald (`cwr_title.ttf`, the game's OFL title font), off-white with a soft blurred shadow, fades of
0.4 s (the end card fades in over 0.3 s and holds to the last frame). Lines used:

| In | Out | Card | Lines |
|---|---|---|---|
""" + "".join(f"| {t['t_in']:.2f} | {t['t_out']:.2f} | {t['id']} | {' / '.join(t['lines'])} |\n" for t in E["text"]) + """
## Cut list

Times are output seconds (frame / 30); In and Out are clip seconds. N = true night (gamma 1.000).

| # | Time | Clip | In | Out | Frames | Gamma | Why |
|---|---|---|---|---|---|---|---|
"""
for s in shots:
    if s["clip"] == "black":
        out += f"| {s['n']} | {s['t']:.3f} | black |  |  | {s['frames']} |  | {s.get('why', '')} |\n"
        continue
    extra = []
    if s.get("crop"):
        extra.append("crop {}x{}+{}+{}".format(*s["crop"]))
    if s.get("speed"):
        extra.append(f"speed {s['speed']}")
    if s.get("fade_in"):
        extra.append(f"fade in {s['fade_in']} s")
    if s.get("fade_out"):
        extra.append(f"fade out {s['fade_out']} s")
    g = f"{s.get('gamma_used', 1.0):.3f}" + (" N" if s.get("night") else "")
    out += (f"| {s['n']} | {s['t']:.3f} | {s['file'].replace('.mp4', '')} | {s['clip_in']:.2f} | {s['clip_out']:.2f} | "
            f"{s['frames']} | {g} | {s.get('why', '')}{' (' + ', '.join(extra) + ')' if extra else ''} |\n")

out += "\n## Choices and weak spots\n\n" + "".join(f"- {w}\n" for w in T.get("weak", []))
(ROOT / "edit-notes.md").write_text(out, encoding="utf-8")
print(ROOT / "edit-notes.md")
