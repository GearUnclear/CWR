"""Uslu dur! Lebanon80 promo: reproducible edit (reshoot footage, 2560x1440 sources).

python build.py                 write edl.json, render segments, final mp4, gates, sheets, phone copy
python build.py --edl-only      only write edl.json (and the text card PNGs)
python build.py --reuse         reuse segments already rendered (same EDL entry hash), keep them
python build.py --keep          keep the intermediate segments afterwards
python build.py --no-phone      skip the 720p phone copy
python build.py --sheets-only   only the night gate and the two sheets, on the existing final mp4
python build.py --dry-run       build the EDL and check every shot against its clip; write nothing

Paths (promopaths.py): the clips, the work folder and the outputs are in UD_PROMO_DIR; the music is
UD_PROMO_AUDIO; the fonts are UD_FONT_DIR; ffmpeg and ffprobe are FFMPEG and FFPROBE or found on PATH.

Everything is ffmpeg + PIL. The music is the untouched instrumental WAV, muxed as is.
Cut times are music times from beats.json; each is snapped to the nearest 1/30 s frame.

Grading rules (the edit rules in ../README.md):
- True-night shots (moon gate passed) get gamma 1.000, contrast 1.00 and no grain, and stay out of
  the exposure match.
- Every other shot may only be darkened by the match: gamma clamped to 0.88 .. 1.00.
- The curve's first point is 0/0. Grain c0s=1 at most, never on night shots. Vignette kept.
- No normalize, histeq or auto levels anywhere.
- Sources are bt601 limited range (rgb24 -> yuv420p default in encode_promo.py); every encode here
  converts to and is tagged bt709 / tv.
- Gate: the build fails when a night shot's 10th-percentile luma (signalstats YLOW, mean over the
  shot) in the output is above its source value plus 1.
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from promopaths import AUDIO, FF, FONT_DIR, FP, PROMO  # noqa: E402

ROOT = PROMO                                        # the production folder
EDIT = ROOT / "work" / "edit"
CLIPS = ROOT / "clips"
SEG = EDIT / "seg"
CARDS = EDIT / "cards"
WAV = AUDIO
OUT = ROOT / "ud-lebanon80-promo-1080p.mp4"
PHONE = ROOT / "ud-lebanon80-promo-720p-phone.mp4"
FONTS = FONT_DIR
FPS = 30
SW, SH = 2560, 1440                                  # source size
AUDIO_DUR = 160.759979
NFRAMES = round(AUDIO_DUR * FPS)                     # 4823 frames = 160.767 s
TITLE = "Uslu dur! Guerrilla Mode: Lebanon80"
TAGS = ["-colorspace", "bt709", "-color_primaries", "bt709", "-color_trc", "bt709", "-color_range", "tv"]

# True-night clips (16 June 20:00, new moon; moonShadows gate off in every take log).
NIGHT = {"c06n", "c13n", "c23n", "c35n", "c36n", "c61", "c62", "c63", "c67", "c85", "c85b", "c86", "c87", "c88"}

# Lock points measured on the sources (frame = clip frame number at 30 fps):
C65_FIRE_FRAME = 122      # first frame of the flash and the beam (mean luma 68.6 -> 77.2); listed 4.10 s
C98_RUN_FRAME = 130       # the fighter breaks into his run (4.333 s; listed 4.35 s)

# ---------------------------------------------------------------------------------------------
# The cut list. (start time in music seconds, clip prefix or "black", clip in-point s, options)
# A shot runs until the next shot's start. Options: speed, crop=(w,h,x,y) in source pixels
# (w >= 1920, so a reframe is at most 1.33x and never below 1080p source pixels), fade_in,
# fade_out, gamma (overrides the exposure match), in_frame (exact clip frame), why.
# ---------------------------------------------------------------------------------------------
CUTS = [
    # INTRO 0-13.50: landscape, fade up from black, black in the 12.4-13.4 gap
    (0.000, "c01", 1.0, dict(fade_in=1.35, why="fade up from black: dawn orbit of Tyre harbour; first hit 1.44")),
    (4.762, "c03", 2.0, dict(why="downbeat: descending dolly onto Ghajar")),
    (8.570, "c07", 3.0, dict(why="BIG HIT 8.57: the Litani valley in low sun, Guerrilla Mode card")),
    (10.473, "c05", 4.0, dict(why="downbeat: Tyre against the sun setting over the sea")),
    (12.377, "black", 0.0, dict(why="hit 12.38, then the near-silent gap: black while the card holds")),
    # BUILD 13.50-19.99: the occupation arrives
    (13.500, "c14", 0.3, dict(fade_in=0.20, why="build: behind the checkpoint soldiers, the stopped van")),
    (16.183, "c15", 2.0, dict(why="downbeat: long lens down the road onto the checkpoint")),
    (18.087, "c22", 3.5, dict(why="last bar: low in front of the lead Merkava at the village edge")),
    # DROP 1 19.99-31.41: IDF occupation, one shot a bar
    (19.990, "c20", 1.6, dict(why="DROP: the Merkava passes the roadside camera close")),
    (21.893, "c10", 6.5, dict(why="the patrol walks into the long lens")),
    (23.796, "c16", 3.8, dict(why="orbit round the checkpoint on the paved road, the bar across it")),
    (25.699, "c24", 0.4, dict(crop=(1920, 1080, 0, 173), why="the convoy comes down the road past the roadside camera (reframed 1.33x: the Humvee from the centre onto the right third)")),
    (27.602, "c12", 3.5, dict(why="side dolly with the walking patrol")),
    (29.505, "c21", 2.0, dict(why="aerial follow of the armour into Ghajar")),
    # BREAK 31.41-35.21
    (31.408, "c13", 6.2, dict(why="break: the checkpoint on the paved road, bar closed; the van rolls in under the lens at the end, Patrols card")),
    # DROP 35.21-46.62: the resistance, golden hour
    (35.213, "c39", 0.0, dict(why="DROP: the file walks into the sun, over the last fighter's shoulder")),
    (37.115, "c38", 4.5, dict(why="the squad leader walks at the camera, the file behind him")),
    (39.017, "c37", 3.0, dict(why="side dolly alongside the file")),
    (40.920, "c33", 4.2, dict(why="the technical on the Ghajar road (after the walls, 3 s on)")),
    (42.822, "c34", 5.3, dict(why="half bar: chasing the technical out of the village")),
    (43.773, "c31", 1.5, dict(why="half bar: behind the squad, the leader ahead")),
    (44.724, "c35b", 3.0, dict(why="the cache orbit at the edge of Ghajar, last of the golden light")),
    # BREAK 46.62-65.64: dusk into true night, the camp, the journal
    (46.625, "c27", 4.0, dict(why="break: dusk over Tyre, lamps coming on")),
    (48.527, "c82", 2.5, dict(why="dusk: the squad crosses the road")),
    (50.429, "c23n", 3.55, dict(why="true night: the convoy's headlights fill the road (ends 5.93 s, before the Humvee drives over the lens)")),
    (52.806, "c35n", 2.0, dict(why="accent 52.80: fighters round the camp fire at the cache, Ambush card")),
    (56.133, "c72", 0.3, dict(crop=(1920, 1080, 0, 150), why="the field journal, Contents page titled Guerrilla (crop 1.33x to the journal)")),
    (58.034, "c13n", 2.0, dict(why="true night: the checkpoint soldiers in their Humvee's headlights, the bar across the road")),
    (59.935, "c36n", 1.0, dict(why="true night: push on the camp fire, new-game card")),
    (63.736, "c06n", 7.5, dict(crop=(1920, 1080, 0, 180), why="the lift: the lamp-lit street at the village edge through the trees")),
    # DROP 2 65.64-86.53: the ambush
    (65.637, "c40", 4.5, dict(why="DROP: wide over the fighters in the grass, the convoy rolls in")),
    (67.537, "c47", 2.8, dict(why="half bar: along the line as the fighters rise and open fire (3.2 s)")),
    (68.487, "c42", 4.0, dict(why="half bar: the RPG gunner rises")),
    (69.437, "c41", 4.4, dict(why="the truck passes 10 m from the lens; the blast (clip 6.3 s) breaks on the 71.34 downbeat")),
    (72.287, "c48", 6.3, dict(why="half bar: the same blast from high across the road")),
    (73.237, "c42", 7.2, dict(why="smoke over the wreck behind the gunner")),
    (75.137, "c46", 3.0, dict(why="pitch dive: the IDF dismount and return fire")),
    (77.036, "c45", 1.0, dict(why="push in on the burning wreck")),
    (78.935, "c40", 9.0, dict(why="the smoke column over the ambush, wide")),
    (80.835, "c43b", 5.7, dict(why="dusk: behind the fighters, the truck blows (6.3 s lands on 81.4)")),
    (82.734, "c44", 8.3, dict(why="dusk, low on the road: the lead Humvee is hit (9.2 s)")),
    (84.632, "c88", 3.3, dict(why="the one night flash: the burning truck, a second burst at 4.0 s")),
    # BREAK 86.53-103.61: the beast teased
    (86.531, "c62", 2.0, dict(why="break: his furnace glow above the lamp-lit roofs, long lens")),
    (90.328, "c63", 2.0, dict(why="his silhouette and chest glow from a village street")),
    (94.123, "c53", 1.0, dict(why="thin part: his legs and footfalls")),
    (96.021, "c54", 3.4, dict(why="hit 96.2: he comes out of the low sun; the soldiers swing the bar up (4.1 to 7.1 s) and step aside")),
    (99.816, "c52", 1.0, dict(why="behind IDF soldiers as he walks past them")),
    # BUILD 103.61 + DROP 107.40: the beam
    (103.610, "c65", 0.0, dict(in_frame=C65_FIRE_FRAME - (round(107.403 * FPS) - round(103.610 * FPS)),
                              why="one shot: chest glow builds, the BEAM FIRES ON 107.40")),
    (108.351, "c64", 4.6, dict(why="half bar: the beam across the valley (fire 4.33 s)")),
    (109.299, "c67", 4.5, dict(why="half bar: true night, beside his legs, the beam into the dark")),
    (110.247, "c66", 4.6, dict(why="half bar: up the beam as it comes down on the fighters")),
    # BREAK 111.19-124.46: the beast and its escort, helicopters for scale
    (111.195, "c50", 0.5, dict(why="break: he advances on the low camera (symmetric, deliberate)")),
    (114.986, "c93", 2.5, dict(why="HELI: orbit at helicopter height, a Cobra circling in the foreground")),
    (118.776, "c91", 3.0, dict(why="HELI: the escort pair at his shoulders")),
    (120.671, "c55", 10.3, dict(why="accent 120.67: he roars over the open IDF roadblock (roar from 10.5 s)")),
    (123.039, "black", 0.0, dict(why="dropout 123.0-123.7: hard cut to black")),
    (123.739, "c50", 7.0, dict(why="back in on the hit 123.74")),
    # BUILD 124.46 + DROP 3 128.25: the dolly zoom runs across the riser into the drop
    (124.460, "c98", 0.0, dict(in_frame=C98_RUN_FRAME - (round(128.247 * FPS) - round(124.460 * FPS)),
                              why="DOLLY ZOOM while he roars; the fighter runs (clip 4.33 s) on the 128.25 drop")),
    (129.667, "c61", 1.0, dict(why="beat after the fighter leaves frame: true night, he walks through the burning position")),
    (132.034, "c96", 6.0, dict(why="HELI: dusk wide, three helicopters circle his glow")),
    (133.927, "c64", 8.5, dict(why="the beam sweeps the valley, fires")),
    (135.820, "c94", 3.0, dict(why="HELI: a Cobra flies past his head")),
    (137.712, "c66", 7.0, dict(why="drums and bass only from 138: calmer, up the beam through the smoke")),
    (139.604, "c51", 3.0, dict(why="his scale over the escorting tanks from the air")),
    (141.496, "c25", 3.5, dict(why="a Bell and a Cobra fly low over the plain toward the sun")),
    # BREAK 143.39-147.17
    (143.388, "c09", 3.0, dict(why="quiet held shot: Ghajar sunset orbit")),
    # FINAL DROP 147.17-154.73: half-bar montage of the best moments, new frames
    (147.170, "c65", 6.5, dict(why="DROP: the beam sweep under him")),
    (148.116, "c41", 9.4, dict(why="the truck burning close")),
    (149.061, "c95", 1.0, dict(why="HELI: looking up at the glowing chest, helicopters circling")),
    (150.006, "c39", 5.0, dict(why="the file into the sun")),
    (150.951, "c54", 8.0, dict(why="the roadblock open, the soldiers aside, as he comes on")),
    (151.897, "c61", 8.0, dict(why="through the fires at night")),
    (152.842, "c67", 7.0, dict(why="the night beam")),
    (153.787, "c52", 6.0, dict(why="he passes the soldiers")),
    # OUTRO 154.73-160.76
    (154.732, "c62", 5.7, dict(fade_out=1.2, why="dark held frame: his glow over the village, sinks to black by the stop")),
    (158.983, "black", 0.0, dict(why="HARD STOP 159.0: title card on black to the end")),
]

# Text cards: (id, t_in, t_out, kind, lines). Times are when the fade in starts / fade out ends.
TEXT = [
    ("t1_overhaul", 2.86, 6.55, "line", ["A total overhaul of Arma: Cold War Assault"]),
    ("t2_mode", 8.57, 13.35, "mode", ["Uslu dur!", "Guerrilla Mode"]),
    ("t3_occupation", 13.95, 17.95, "line", ["A persistent occupation"]),
    ("t4_patrols", 31.70, 34.95, "line", ["Patrols, roadblocks, convoys"]),
    ("t5_ambush", 53.05, 55.95, "line", ["Ambush. Hide. Resupply."]),
    ("t6_newgame", 60.20, 63.55, "line", ["Choose the island and the factions at new game"]),
    ("t7_end", 158.93, AUDIO_DUR, "end", ["Uslu dur!", "Guerrilla Mode"]),
]
FADE = 0.4

CURVE = "curves=master='0/0 0.25/0.24 0.75/0.765 1/0.975'"
TARGET_LUMA, MATCH, GMIN, GMAX = 0.40, 0.35, 0.88, 1.00


def grade_chain(night, gamma, fade_in, fade_out, dur):
    """crop/scale happen before; this works on 1920x1080 yuv420p (bt601 tv) and ends in bt709 tv."""
    f = [f"eq=contrast={1.00 if night else 1.05}:saturation=0.93:gamma={gamma:.3f}",
         "scale=in_color_matrix=bt601:in_range=tv", "format=gbrp", CURVE]
    if not night:
        f.append("colorbalance=rh=0.012:bh=-0.01:bs=0.008")
    f.append("vignette=angle=0.42")
    if fade_in:
        f.append(f"fade=in:st=0:d={fade_in}")
    if fade_out:
        f.append(f"fade=out:st={dur - fade_out:.4f}:d={fade_out}")
    f += ["scale=out_color_matrix=bt709:out_range=tv", "format=yuv420p"]
    if not night:
        f.append("noise=c0s=1:c0f=t+u")
    return ",".join(f)


GRADE_DOC = grade_chain(False, 1.0, 0, 0, 1).replace("gamma=1.000", "gamma=G")
GRADE_NIGHT_DOC = grade_chain(True, 1.0, 0, 0, 1)


# ---------------------------------------------------------------------------------------------
# Text cards
# ---------------------------------------------------------------------------------------------
def font(name, size):
    return ImageFont.truetype(str(FONTS / f"{name}.ttf"), size)


def draw_tracked(d, xy, text, f, tracking, fill):
    x, y = xy
    for ch in text:
        d.text((x, y), ch, font=f, fill=fill)
        x += f.getlength(ch) + tracking


def tracked_width(text, f, tracking):
    return sum(f.getlength(c) for c in text) + tracking * (len(text) - 1)


def make_card(cid, kind, lines):
    W, H = 1920, 1080
    txt = Image.new("L", (W, H), 0)
    d = ImageDraw.Draw(txt)
    if kind == "line":
        f = font("cwr_title", 56)
        tr = 2
        w = tracked_width(lines[0], f, tr)
        x, y = 128, 880
        # thin rule above the line
        d.rectangle([x, y - 26, x + 64, y - 22], fill=255)
        draw_tracked(d, (x, y), lines[0], f, tr, 255)
    elif kind == "mode":
        fs, fb = font("cwr_title", 52), font("cwr_title", 132)
        ws = tracked_width(lines[0], fs, 4)
        wb = tracked_width(lines[1].upper(), fb, 10)
        draw_tracked(d, ((W - ws) / 2, 400), lines[0], fs, 4, 255)
        draw_tracked(d, ((W - wb) / 2, 470), lines[1].upper(), fb, 10, 255)
    elif kind == "end":
        fb, fs = font("cwr_title", 190), font("cwr_title", 54)
        wb = tracked_width(lines[0], fb, 4)
        ws = tracked_width(lines[1].upper(), fs, 14)
        draw_tracked(d, ((W - wb) / 2, 345), lines[0], fb, 4, 255)
        d.rectangle([(W - 120) / 2, 652, (W + 120) / 2, 655], fill=255)
        draw_tracked(d, ((W - ws) / 2, 690), lines[1].upper(), fs, 14, 255)
    shadow = txt.filter(ImageFilter.GaussianBlur(9)).point(lambda v: min(255, int(v * 1.6)))
    shadow2 = txt.filter(ImageFilter.GaussianBlur(2)).point(lambda v: int(v * 0.8))
    sa = np.maximum(np.asarray(shadow), np.asarray(shadow2)).astype(np.float32) / 255 * 0.62
    ta = np.asarray(txt).astype(np.float32) / 255
    alpha = ta + sa * (1 - ta)                      # white text over a soft black shadow
    col = np.divide(ta, alpha, out=np.zeros_like(ta), where=alpha > 1e-4)
    arr = np.zeros((H, W, 4), np.uint8)
    for c, v in enumerate((246, 244, 238)):
        arr[..., c] = np.clip(col * v, 0, 255).astype(np.uint8)
    arr[..., 3] = np.clip(alpha * 255, 0, 255).astype(np.uint8)
    CARDS.mkdir(parents=True, exist_ok=True)
    p = CARDS / f"{cid}.png"
    Image.fromarray(arr, "RGBA").save(p)
    return p


# ---------------------------------------------------------------------------------------------
# EDL
# ---------------------------------------------------------------------------------------------
def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        print(" ".join(map(str, cmd)))
        print(r.stderr[-3000:])
        raise SystemExit(1)
    return r.stdout + r.stderr


def clip_path(prefix):
    m = sorted(CLIPS.glob(prefix + "_*.mp4"))
    if not m:
        raise SystemExit(f"no clip for {prefix}")
    return m[0]


def clip_frames(path):
    return int(run([FP, "-v", "error", "-select_streams", "v", "-count_packets", "-show_entries",
                    "stream=nb_read_packets", "-of", "csv=p=0", str(path)]).strip())


def make_edl(write=True):
    shots = []
    for i, (t, clip, cin, opt) in enumerate(CUTS):
        t1 = CUTS[i + 1][0] if i + 1 < len(CUTS) else AUDIO_DUR
        f0 = round(t * FPS)
        f1 = round(t1 * FPS) if i + 1 < len(CUTS) else NFRAMES
        e = dict(n=i + 1, t=round(f0 / FPS, 4), t_music=t, frames=f1 - f0, clip=clip, **opt)
        if clip != "black":
            p = clip_path(clip)
            e["file"] = p.name
            e["night"] = clip in NIGHT
            sp = e.get("speed", 1.0)
            in_f = e.pop("in_frame", None)
            in_f = round(cin * FPS) if in_f is None else in_f
            e["clip_in_frame"] = in_f
            e["clip_in"] = round(in_f / FPS, 4)
            need = int(np.ceil(e["frames"] * sp))
            e["clip_out"] = round((in_f + need) / FPS, 3)
            nf = clip_frames(p)
            if in_f < 0 or in_f + need > nf:
                raise SystemExit(f"shot {i+1} {p.name}: needs frames {in_f}..{in_f+need} of {nf}")
            if e.get("crop"):
                w, h, x, y = e["crop"]
                assert w >= 1920 and h >= 1080 and abs(w / h - 16 / 9) < 1e-3 and x + w <= SW and y + h <= SH, e["crop"]
        shots.append(e)
    assert sum(s["frames"] for s in shots) == NFRAMES
    edl = dict(title=TITLE, fps=FPS, frames=NFRAMES, audio=WAV.name, audio_duration=AUDIO_DUR,
               shots=shots, text=[dict(id=a, t_in=b, t_out=c, kind=d, lines=e) for a, b, c, d, e in TEXT],
               grade_day=GRADE_DOC, grade_night=GRADE_NIGHT_DOC,
               exposure_match=dict(target=TARGET_LUMA, strength=MATCH, gamma_range=[GMIN, GMAX], night="excluded, gamma 1.000"),
               locks=dict(c65_fire_frame=C65_FIRE_FRAME, c98_run_frame=C98_RUN_FRAME))
    if write:
        EDIT.mkdir(parents=True, exist_ok=True)
        (EDIT / "edl.json").write_text(json.dumps(edl, indent=1))
    return edl


# ---------------------------------------------------------------------------------------------
# Segments
# ---------------------------------------------------------------------------------------------
def frame_scale(s):
    """The crop + downscale to 1920x1080 (lanczos), as one filter string, no grade."""
    f = []
    if s.get("crop"):
        w, h, x, y = s["crop"]
        f.append(f"crop={w}:{h}:{x}:{y}")
    f.append("scale=1920:1080:flags=lanczos")
    return ",".join(f)


def mean_luma(s):
    path = CLIPS / s["file"]
    need = int(np.ceil(s["frames"] * s.get("speed", 1.0)))
    vals = []
    for k in (0.15, 0.5, 0.85):
        f = s["clip_in_frame"] + int(need * k)
        vf = rf"select=eq(n\,{f})," + frame_scale(s).replace("1920:1080", "192:108")
        raw = subprocess.run([FF, "-v", "error", "-i", str(path), "-vf", vf, "-frames:v", "1",
                              "-f", "rawvideo", "-pix_fmt", "gray", "-"], capture_output=True).stdout
        if raw:
            vals.append(np.frombuffer(raw, np.uint8).mean() / 255)
    return float(np.mean(vals))


def auto_gamma(m):
    new = m + MATCH * (TARGET_LUMA - m)
    g = np.log(m) / np.log(new)
    return float(np.clip(g, GMIN, GMAX))


def seg_hash(s):
    keys = {k: s.get(k) for k in ("clip", "file", "clip_in_frame", "frames", "speed", "crop", "fade_in", "fade_out", "gamma_used", "night")}
    keys["chain"] = grade_chain(s.get("night", False), s.get("gamma_used", 1.0), s.get("fade_in"), s.get("fade_out"), s["frames"] / FPS)
    return hashlib.sha1(json.dumps(keys, sort_keys=True).encode()).hexdigest()[:10]


ENC_SEG = ["-c:v", "libx264", "-preset", "medium", "-crf", "12", "-pix_fmt", "yuv420p", "-r", str(FPS),
           "-video_track_timescale", "15360", "-an", *TAGS]


def render_segment(s, reuse):
    SEG.mkdir(parents=True, exist_ok=True)
    n = s["frames"]
    out = SEG / f"s{s['n']:03d}_{seg_hash(s)}.mp4"
    s["segment"] = out.name
    if reuse and out.exists():
        return out
    for old in SEG.glob(f"s{s['n']:03d}_*.mp4"):
        old.unlink()
    if s["clip"] == "black":
        run([FF, "-v", "error", "-y", "-f", "lavfi", "-i", f"color=c=black:s=1920x1080:r={FPS}",
             "-vf", "format=yuv420p", "-frames:v", str(n), *ENC_SEG, str(out)])
        return out
    sp = s.get("speed", 1.0)
    in_f = s["clip_in_frame"]
    need = int(np.ceil(n * sp))
    vf = [f"trim=start_frame={in_f}:end_frame={in_f + need}", "setpts=PTS-STARTPTS"]
    if sp != 1.0:
        vf += [f"setpts=PTS/{sp}", f"fps={FPS}"]
    vf.append(frame_scale(s))
    vf.append(grade_chain(s["night"], s["gamma_used"], s.get("fade_in"), s.get("fade_out"), n / FPS))
    run([FF, "-v", "error", "-y", "-i", str(CLIPS / s["file"]), "-vf", ",".join(vf), "-frames:v", str(n),
         *ENC_SEG, str(out)])
    got = clip_frames(out)
    if got != n:
        raise SystemExit(f"segment {out.name}: {got} frames, wanted {n}")
    return out


def render(edl, reuse):
    shots = edl["shots"]
    for s in shots:
        if s["clip"] == "black":
            continue
        if s["night"]:
            s["gamma_used"] = 1.0
        elif "gamma" in s:
            s["gamma_used"] = min(1.0, s["gamma"])
        else:
            m = mean_luma(s)
            s["mean_luma"] = round(m, 3)
            s["gamma_used"] = round(auto_gamma(m), 3)
    for s in shots:
        render_segment(s, reuse)
        print(f"  seg {s['n']:3d} {s['t']:8.3f} {s['clip']:6s} {s['frames']:4d}f g={s.get('gamma_used', '-')}"
              + (" NIGHT" if s.get("night") else ""), flush=True)
    (EDIT / "edl.json").write_text(json.dumps(edl, indent=1))

    lst = SEG / "concat.txt"
    lst.write_text("".join(f"file '{(SEG / s['segment']).as_posix()}'\n" for s in shots))
    cards = [(t, make_card(t["id"], t["kind"], t["lines"])) for t in edl["text"]]

    cmd = [FF, "-v", "error", "-y", "-f", "concat", "-safe", "0", "-i", str(lst), "-i", str(WAV)]
    fc = ["[0:v]setpts=PTS-STARTPTS[v0]"]
    last = "v0"
    for k, (t, p) in enumerate(cards):
        dur = t["t_out"] - t["t_in"]
        cmd += ["-loop", "1", "-framerate", str(FPS), "-t", f"{dur:.3f}", "-i", str(p)]
        idx = k + 2
        fo = "" if t["kind"] == "end" else f",fade=out:st={dur - FADE:.3f}:d={FADE}:alpha=1"
        fc.append(f"[{idx}:v]format=rgba,fade=in:st=0:d={FADE if t['kind'] != 'end' else 0.3}:alpha=1{fo},"
                  f"setpts=PTS-STARTPTS+{t['t_in']:.3f}/TB[c{k}]")
        fc.append(f"[{last}][c{k}]overlay=0:0:eof_action=pass:format=auto[v{k + 1}]")
        last = f"v{k + 1}"
    fc.append(f"[{last}]format=yuv420p[vout]")
    cmd += ["-filter_complex", ";".join(fc), "-map", "[vout]", "-map", "1:a",
            "-frames:v", str(NFRAMES),
            "-c:v", "libx264", "-preset", "slow", "-profile:v", "high", "-crf", "18", "-pix_fmt", "yuv420p",
            "-r", str(FPS), *TAGS, "-c:a", "aac", "-b:a", "320k", "-ar", "48000",
            "-map_metadata", "-1", "-map_chapters", "-1", "-metadata", f"title={TITLE}",
            "-movflags", "+faststart", str(OUT)]
    print("final encode ...", flush=True)
    run(cmd)


# ---------------------------------------------------------------------------------------------
# Gates and checks
# ---------------------------------------------------------------------------------------------
def ylow_series(path, vf_pre):
    """Per-frame signalstats YLOW (10th percentile of Y) after the given filters."""
    out = run([FF, "-v", "error", "-i", str(path), "-vf",
               vf_pre + ",signalstats,metadata=print:key=lavfi.signalstats.YLOW:file=-", "-f", "null", "-"])
    return [float(v) for v in re.findall(r"lavfi\.signalstats\.YLOW=([\d.]+)", out)]


def night_gate(edl):
    rows, fail = [], False
    for s in edl["shots"]:
        if not s.get("night"):
            continue
        f0 = round(s["t"] * FPS)
        out = ylow_series(OUT, f"trim=start_frame={f0}:end_frame={f0 + s['frames']}")
        need = int(np.ceil(s["frames"] * s.get("speed", 1.0)))
        src = ylow_series(CLIPS / s["file"],
                          f"trim=start_frame={s['clip_in_frame']}:end_frame={s['clip_in_frame'] + need},setpts=PTS-STARTPTS,"
                          + frame_scale(s))
        o, r = float(np.mean(out)), float(np.mean(src))
        ok = o <= r + 1.0
        fail |= not ok
        rows.append(dict(n=s["n"], t=s["t"], clip=s["clip"], src_ylow=round(r, 2), out_ylow=round(o, 2),
                         diff=round(o - r, 2), out_ylow_max=round(max(out), 1), pass_=ok))
        print(f"  night gate shot {s['n']:2d} {s['clip']:5s} src YLOW {r:6.2f}  out {o:6.2f}  diff {o - r:+5.2f}  {'PASS' if ok else 'FAIL'}")
    (EDIT / "night_gate.json").write_text(json.dumps(rows, indent=1))
    if fail:
        raise SystemExit("NIGHT GATE FAILED: a night shot came out lighter than its source (see night_gate.json)")
    return rows


def check():
    js = json.loads(run([FP, "-v", "error", "-count_frames", "-show_entries",
                         "stream=codec_type,duration,nb_read_frames,profile,pix_fmt,width,height,sample_rate,"
                         "color_range,color_space,color_primaries,color_transfer:format=duration,size:format_tags",
                         "-of", "json", str(OUT)]))
    print(json.dumps(js, indent=1))
    v = next(x for x in js["streams"] if x["codec_type"] == "video")
    a = next(x for x in js["streams"] if x["codec_type"] == "audio")
    assert abs(float(v["duration"]) - float(a["duration"])) < 1 / FPS, "stream durations differ by a frame or more"
    return js


def sheet(edl, name, grid):
    tw, th, cols = 384, 216, 5
    shots = edl["shots"]
    rows = (len(shots) + cols - 1) // cols
    f = ImageFont.truetype(str(FONTS / "cwr_mono.ttf"), 17)
    im = Image.new("RGB", (cols * tw, rows * (th + 24)), (16, 16, 16))
    d = ImageDraw.Draw(im)
    for i, s in enumerate(shots):
        fm = round(s["t"] * FPS) + s["frames"] // 2
        raw = subprocess.run([FF, "-v", "error", "-ss", f"{fm / FPS:.3f}", "-i", str(OUT), "-frames:v", "1",
                              "-vf", f"scale={tw}:{th}", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                             capture_output=True).stdout
        x, y = (i % cols) * tw, (i // cols) * (th + 24)
        if len(raw) == tw * th * 3:
            im.paste(Image.frombytes("RGB", (tw, th), raw), (x, y + 24))
        if grid:
            for k in (1, 2):
                d.line([x + tw * k // 3, y + 24, x + tw * k // 3, y + 24 + th], fill=(255, 255, 255), width=1)
                d.line([x, y + 24 + th * k // 3, x + tw, y + 24 + th * k // 3], fill=(255, 255, 255), width=1)
        d.text((x + 4, y + 3), f"{s['n']:02d} {s['t']:.2f} {s['clip']} @{s.get('clip_in', 0):.2f}"
               + (" N" if s.get("night") else "") + (" crop" if s.get("crop") else ""), fill=(255, 220, 120), font=f)
    im.save(EDIT / name, quality=85)


def phone():
    """720p copy for phones, two-pass x264, under 29 MiB."""
    log = EDIT / "phone2pass"
    common = ["-vf", "scale=1280:720:flags=lanczos", "-c:v", "libx264", "-preset", "slow", "-profile:v", "high",
              "-b:v", "1250k", "-maxrate", "2500k", "-bufsize", "5000k", "-pix_fmt", "yuv420p", *TAGS,
              "-passlogfile", str(log)]
    run([FF, "-v", "error", "-y", "-i", str(OUT), *common, "-pass", "1", "-an", "-f", "null", os.devnull])
    run([FF, "-v", "error", "-y", "-i", str(OUT), *common, "-pass", "2", "-c:a", "aac", "-b:a", "160k",
         "-ar", "48000", "-map_metadata", "-1", "-map_chapters", "-1", "-metadata", f"title={TITLE}",
         "-movflags", "+faststart", str(PHONE)])
    for p in EDIT.glob("phone2pass*"):
        p.unlink()
    size = PHONE.stat().st_size
    print(f"phone copy {size} bytes = {size / 2**20:.2f} MiB")
    assert size < 29 * 2**20, "phone copy is 29 MiB or more"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--edl-only", action="store_true")
    ap.add_argument("--reuse", action="store_true")
    ap.add_argument("--keep", action="store_true")
    ap.add_argument("--no-phone", action="store_true")
    ap.add_argument("--sheets-only", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    edl = make_edl(write=not a.dry_run)
    print(f"{len(edl['shots'])} shots, {NFRAMES} frames", flush=True)
    if a.dry_run:
        night = sum(1 for s in edl["shots"] if s.get("night"))
        print(f"dry run: every shot fits its clip ({night} night shots, {len(edl['text'])} cards); music {WAV}"
              f" ({'found' if WAV.exists() else 'MISSING'}), output {OUT}")
        return
    if a.edl_only:
        for t in edl["text"]:
            make_card(t["id"], t["kind"], t["lines"])
        return
    if not a.sheets_only:
        render(edl, a.reuse)
        check()
    edl = json.loads((EDIT / "edl.json").read_text())
    night_gate(edl)
    sheet(edl, "final_sheet.jpg", grid=False)
    sheet(edl, "final_thirds_sheet.jpg", grid=True)
    if not a.no_phone and not a.sheets_only:
        phone()
    if not (a.keep or a.reuse):
        shutil.rmtree(SEG, ignore_errors=True)
    print("done", flush=True)


if __name__ == "__main__":
    sys.exit(main())
