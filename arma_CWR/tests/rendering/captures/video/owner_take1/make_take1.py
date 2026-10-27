"""The owner's free take 1 (2026-10-23): one continuous read over the whole trailer, already in sync
with the picture. Clean it lightly, level it to -16 LUFS, mux it onto both montages, write subtitles from
his own words, burn a subtitled copy and a 720p phone preview.

    python make_take1.py

Light clean only: low cut at 80 Hz, gentle noise reduction (floor about -58 dB), a soft gate between
phrases, de-ess, 2:1 compression; no words cut, no timing changed, no pitch or tempo change.
"""
import json
import subprocess
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(TOOLS))
import udpaths  # noqa: E402  (where things live; see udpaths.py)
FF = udpaths.FFMPEG
V2 = udpaths.WORK
import mix_vo  # noqa: E402  (subtitle look and burn helpers shared with the trailer mixer)

HERE = Path(__file__).resolve().parent
SRC = V2 / "reading" / "recordings" / "owner_take1_2026-10-23.wav"
SPLIT = V2 / "out-split" / "uslu_dur_v2.mp4"
SBS = V2 / "out-sbs" / "uslu_dur_v2.mp4"
FINAL = V2 / "final"
NAME = "uslu_dur_part1_graphics_owner_take1"
VIDEO_END = 133.4

# His words, as read (fillers and one restart left out of the text only), with the times he said them.
ROWS = [
    (0.00, 11.34, "Hey y'all, Uslu dur! is a total overhaul for Arma: Cold War Assault. Today we're going to do a little video on the graphics and rendering improvements."),
    (12.84, 19.30, "The models are all unchanged, but as you can see, there's a couple of nice changes here."),
    (19.60, 29.16, "We got some volumetric fog. You'll be able to see light shafts come through the trees, very impressively."),
    (30.00, 35.10, "The water now shows reflections of the islands and sky above it."),
    (35.20, 47.40, "Color palette is a bit more reminiscent of Arma 3 as a whole, you might find, for better or for worse, going for a little bit more of a realistic vibe."),
    (47.50, 56.60, "It works with mods. We got, you know, cloud shadows, those can be pretty freaky."),
    (57.90, 64.40, "Lost Brothers Reloaded mod, we got that in."),
    (64.90, 72.90, "You know, golden hour, ground textures, everything's looking a lot nicer. The shadows, they're beautiful."),
    (78.20, 85.24, "The color palette is not the same. I will admit, it's a bit less red."),
    (85.88, 89.08, "Not for everyone, but this is for Uslu dur!"),
    (90.32, 94.14, "A lot of improvements have been made to nighttime."),
    (96.20, 102.78, "The buildings, they look beautiful at night. The street lights, they look amazing."),
    (104.14, 110.00, "Nighttime as a whole is just a lot darker. More impressive, more realistic lighting."),
    (110.10, 122.00, "I think that this is going to really impress y'all, and I hope that you're excited to see more of the Uslu dur! mod."),
    (122.10, 131.60, "Next time we'll be discussing a little bit about some gameplay and enemy AI systems."),
]

CLEAN = ("aresample=48000:resampler=soxr,highpass=f=80:poles=2,afftdn=nr=8:nf=-58:tn=1,"
         "agate=threshold=0.004:ratio=2:range=0.3:attack=5:release=300,deesser=i=0.3,"
         "acompressor=threshold=0.05:ratio=2:attack=15:release=250")


def run(cmd, **kw):
    print(" ".join(str(c) for c in cmd))
    return subprocess.run([str(c) for c in cmd], check=True, **kw)


def main():
    cleaned = HERE / "take1_clean.wav"
    norm = HERE / "take1_norm.wav"
    run([FF, "-y", "-loglevel", "error", "-i", SRC, "-af", CLEAN, "-c:a", "pcm_s24le", cleaned])
    # The voice centred in stereo (both channels; R128 counts both, so level the stereo signal), brought up
    # near the target with a gentle peak limiter so the final loudnorm can stay linear.
    staged = HERE / "take1_stereo.wav"
    run([FF, "-y", "-loglevel", "error", "-i", cleaned, "-af",
         "pan=stereo|c0=c0|c1=c0,volume=16dB,alimiter=limit=0.63:attack=5:release=60:level=false",
         "-c:a", "pcm_s24le", staged])
    cleaned = staged
    # two-pass loudnorm, linear, to the trailer's spec
    p = run([FF, "-hide_banner", "-i", cleaned, "-af", "loudnorm=I=-16:TP=-1.5:LRA=11:print_format=json",
             "-f", "null", "-"], capture_output=True, text=True)
    a = p.stderr.rindex("{")
    m = json.loads(p.stderr[a:p.stderr.index("}", a) + 1])
    ln = ("loudnorm=I=-16:TP=-1.5:LRA=11:linear=true:print_format=summary:"
          f"measured_I={m['input_i']}:measured_TP={m['input_tp']}:measured_LRA={m['input_lra']}:"
          f"measured_thresh={m['input_thresh']}:offset={m['target_offset']}")
    print("loudnorm pass 1:", m)
    run([FF, "-y", "-loglevel", "error", "-i", cleaned, "-af", ln + ",aresample=48000", "-c:a", "pcm_s24le", norm])

    FINAL.mkdir(exist_ok=True)
    master = FINAL / f"{NAME}_youtube_1080p60.mp4"
    sbs = FINAL / f"{NAME}_side_by_side_3840x1080p60.mp4"
    for vid, dst in ((SPLIT, master), (SBS, sbs)):
        run([FF, "-y", "-loglevel", "error", "-i", vid, "-i", norm, "-map", "0:v", "-map", "1:a", "-c:v", "copy",
             "-c:a", "aac", "-b:a", "320k", "-ar", "48000", "-t", str(VIDEO_END),
             "-movflags", "+faststart", dst])

    rows = [dict(start=a, end=b, text=t) for a, b, t in ROWS]
    mix_vo.write_srt(rows, FINAL / f"{NAME}.srt", VIDEO_END)
    subbed = FINAL / f"{NAME}_subtitled.mp4"
    mix_vo.burn(FF, master, rows, VIDEO_END, subbed,
                ["-c:v", "libx264", "-crf", "18", "-preset", "medium", "-pix_fmt", "yuv420p", "-c:a", "copy"])
    preview = FINAL / f"{NAME}_preview_720p60.mp4"
    run([FF, "-y", "-loglevel", "error", "-i", subbed, "-vf", "scale=1280:720:flags=lanczos", "-c:v", "libx264",
         "-preset", "medium", "-crf", "25", "-r", "60", "-c:a", "aac", "-b:a", "160k", "-movflags", "+faststart",
         preview])
    print("done")


if __name__ == "__main__":
    main()
