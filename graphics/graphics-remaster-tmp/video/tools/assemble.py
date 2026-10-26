"""Assemble the Uslu dur! graphics comparison video from the per-feature
side-by-side clips (make_video.py output), with title cards, the narration,
an SRT and a subtitled copy.

    python assemble.py lines           write the TTS lines JSON (for uslu_dur_vo.py)
    python assemble.py video           title cards + silent combined MP4 + timeline.json
    python assemble.py mix             narration mix, SRT, final MP4s (needs vo/*.wav)

Everything lands in D:\\Arma_CWA_worktrees\\graphics-remaster\\tmp\\video\\.
"""
import json
import shutil
import subprocess
import sys
import wave
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

VIDEO = Path(r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\video")
FF = r"C:\ffmpeg\bin\ffmpeg.exe"
FPS = 30
W, H = 2560, 720          # combined video size (each panel 1280x720)
CRF = 21
CARD_S = 2.5
TITLE_S = 3.0
CLOSE_S = 6.0
LEAD = 0.4                # narration starts this long after its segment

# (segment id, clip folder or None for a card-only segment, card title, card
# subtitle (what to look for), narration line, delivery note)
SEGMENTS = [
    ("opening", "opening_pine_eve", "Uslu dur!", "Remastered lighting: the original look on the left, the new lighting on the right",
     "Your war used to look like a postcard left in the rain. Meet Uslu dur. Same island, same soldiers, brand new light.",
     "Bright and inviting, a smile on 'brand new light'."),
    ("temporal", "temporal_forest_edge", "Temporal anti-aliasing", "Look for: steadier edges on grass, branches and the passing Jeep",
     "Jagged edges? Retired. Temporal anti-aliasing keeps every branch calm, even when the traffic isn't.",
     "Brisk; a dry beat after 'Retired'."),
    ("lighttiles", "lighttiles_town_night", "Tiled light list", "Look for: each lamp and the fire lighting the street near it",
     "A smarter light list means every lamp lights the street it actually stands on. Civic pride, at last.",
     "Matter-of-fact, then a wink on 'Civic pride'."),
    ("atmosphere", "atmosphere_coastline_sunset", "Sky and fog colour", "Look for: the colour of the sky and the horizon at sunset",
     "A new sky table paints every dusk in honest colour. Sunsets this good should be illegal.",
     "Warm, slightly dreamy; playful on 'illegal'."),
    ("shadows", "shadows_woods_pine2_evening", "Tree shadows and soft sun shadows", "Look for: trees shading the ground, softer shadow edges",
     "Trees cast real shadows now, with soft, natural edges. Perfect for hiding. Or for posing.",
     "Smooth; pause before 'Or for posing'."),
    ("shade", "shade_lowsun_fp_orbit", "Low-sun light and tree crowns", "Look for: morning light and shade on hulls, men and tents",
     "Low morning sun wraps around every hull and helmet. Even the tent looks expensive.",
     "Glossy, a little smug on 'expensive'."),
    ("night", "night_road_flat_drive", "Night: low beams, lamps and stars", "Look for: the Jeep's low beams on the road, a darker night sky",
     "Night is dark again. Low beams, honest lamps, and a sky full of stars. Romantic. Mostly.",
     "Hushed and intimate; deadpan 'Mostly'."),
    ("combat", "combat_ffh", "Combat light", "Look for: fire, flare and shell light on the men, and their shadows",
     "Fires, flares and shell bursts now light up everyone nearby, shadows and all. Subtle, it isn't.",
     "Punchy, a grin on 'Subtle, it isn't'."),
    ("water", "water_coastline_sunset", "Sea waves", "Look for: moving waves and the sun's glitter on the sea",
     "The sea finally moves like a sea. Rolling waves, golden glitter. Seasickness sold separately.",
     "Breezy; quick aside on the last sentence."),
    ("ssr", "ssr_bay_offshore", "Reflections on water", "Look for: the hills and the boat mirrored in the calm bay",
     "Calm water now reflects the hills and boats above it. Vanity, rendered in real time.",
     "Smooth, amused on 'Vanity'."),
    ("materials", "materials_town_night_rain", "Wet weather and lamp streaks", "Look for: wet stone, shining puddles, lamp light streaking on the road",
     "When it rains, the streets get wet, puddles shine, and lamp light streaks across the stones. Bring a coat.",
     "Cosy and rain-soaked; light tag 'Bring a coat'."),
    ("airbeams", "airbeams_pinewood_morn", "Sun beams", "Look for: shafts of morning light between the pines",
     "Morning sun pours through the pines in visible beams. Very spiritual. Helmets stay on, please.",
     "Soft and reverent, then a crisp aside."),
    ("airhaze", "airhaze_valley_lapse", "Aerial haze and valley mist", "Look for: distance fading into haze, mist lying in the valley at dawn",
     "Far hills melt into soft haze, and mist settles in the valleys at dawn. Lovely weather for an ambush.",
     "Slow and scenic; sly on 'ambush'."),
    ("hfrt", "hfrt_hill_1730", "Long tree shadows", "Look for: long evening shadows of the woods across the slopes",
     "Whole forests throw long evening shadows across the hills. The trees have clearly been working out.",
     "Confident; tease on the last line."),
    ("clouds", "clouds_coast_oc_up", "Lit clouds and cloud shadows", "Look for: clouds lit by the low sun, cloud shadows that match the sky",
     "Clouds glow with the evening light, and their shadows finally match the sky. Weather you can trust.",
     "Warm, reassuring."),
    ("texsharp", "texsharp_forest_edge_walk", "Terrain detail and sharpening", "Look for: crisper ground texture up close",
     "And up close, the ground is sharper than ever. Every blade of grass, a personal achievement.",
     "Building to the finish; proud, a little silly."),
    ("closing", None, "Uslu dur!", "Remastered lighting. Same war, better light.",
     "Uslu dur. Same war. Better light. See you out there.",
     "Sign-off: warm, confident, a smile."),
]


def font(size, bold=True):
    for name in (("arialbd.ttf",) if bold else ()) + ("arial.ttf", "segoeui.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def card_image(title, sub, small=""):
    img = Image.new("RGB", (W, H), (14, 15, 17))
    d = ImageDraw.Draw(img)
    f1, f2, f3 = font(int(H * 0.10)), font(int(H * 0.042), False), font(int(H * 0.032), False)
    y = int(H * 0.36)
    d.text((W // 2, y), title, font=f1, fill=(244, 240, 230), anchor="mm")
    d.line([(W // 2 - 220, y + int(H * 0.085)), (W // 2 + 220, y + int(H * 0.085))], fill=(200, 150, 70), width=4)
    d.text((W // 2, y + int(H * 0.16)), sub, font=f2, fill=(205, 205, 205), anchor="mm")
    if small:
        d.text((W // 2, y + int(H * 0.25)), small, font=f3, fill=(150, 150, 150), anchor="mm")
    return img


def run(cmd):
    subprocess.run(cmd, check=True)


def card_mp4(img, seconds, dst):
    png = dst.with_suffix(".png")
    img.save(png)
    run([FF, "-y", "-loglevel", "error", "-loop", "1", "-framerate", str(FPS), "-i", str(png), "-t", f"{seconds}",
         "-c:v", "libx264", "-crf", str(CRF), "-preset", "slow", "-pix_fmt", "yuv420p", "-r", str(FPS), str(dst)])


def probe_dur(p):
    out = subprocess.run([FF.replace("ffmpeg.exe", "ffprobe.exe"), "-v", "error", "-show_entries", "format=duration",
                          "-of", "default=nw=1:nk=1", str(p)], capture_output=True, text=True, check=True).stdout
    return float(out.strip())


def clip_mp4(clip):
    return VIDEO / clip / f"{clip}__sbs.mp4"


def cmd_lines():
    lines = [{"id": f"{i:02d}_{s[0]}", "text": s[4]} for i, s in enumerate(SEGMENTS)]
    dst = Path(r"C:\dev\local-tts\outputs\uslu-dur-video\lines.json")
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(json.dumps(lines, indent=1), encoding="utf-8")
    for line in lines:
        n = len(line["text"].split())
        print(f"{line['id']:16s} {n:3d} words ~{n / 2.5:4.1f}s")
    print(dst)


def cmd_video():
    work = VIDEO / "_assembly"
    work.mkdir(exist_ok=True)
    parts, timeline, t = [], [], 0.0
    for i, (sid, clip, title, sub, text, note) in enumerate(SEGMENTS):
        seg_start = t
        if sid == "opening":
            card = work / f"{i:02d}_{sid}_card.mp4"
            card_mp4(card_image(title, sub, "Abel (Malden), 1920x1080, 4x MSAA, render scale 1.0"), TITLE_S, card)
            parts.append(card)
            t += TITLE_S
        elif clip is None:
            card = work / f"{i:02d}_{sid}_card.mp4"
            card_mp4(card_image(title, sub), CLOSE_S, card)
            parts.append(card)
            t += CLOSE_S
        else:
            card = work / f"{i:02d}_{sid}_card.mp4"
            card_mp4(card_image(title, sub, "left: off   |   right: on"), CARD_S, card)
            parts.append(card)
            t += CARD_S
        clip_start = None
        if clip:
            src = clip_mp4(clip)
            if not src.exists():
                raise SystemExit(f"missing clip {src}")
            clip_start = t
            parts.append(src)
            t += probe_dur(src)
        timeline.append(dict(id=f"{i:02d}_{sid}", segment=sid, clip=clip, title=title, look_for=sub, text=text,
                             note=note, start=round(seg_start, 3), clip_start=clip_start and round(clip_start, 3),
                             end=round(t, 3)))
    # one re-encode through the concat filter: every part scaled to W x H
    cmd = [FF, "-y", "-loglevel", "error"]
    for p in parts:
        cmd += ["-i", str(p)]
    fl = "".join(f"[{k}:v]scale={W}:{H}:flags=lanczos,setsar=1,fps={FPS},format=yuv420p[v{k}];" for k in range(len(parts)))
    fl += "".join(f"[v{k}]" for k in range(len(parts))) + f"concat=n={len(parts)}:v=1:a=0[out]"
    silent = work / "combined_silent.mp4"
    cmd += ["-filter_complex", fl, "-map", "[out]", "-c:v", "libx264", "-crf", str(CRF), "-preset", "slow",
            "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(silent)]
    run(cmd)
    (VIDEO / "timeline.json").write_text(json.dumps(timeline, indent=1), encoding="utf-8")
    print(f"{silent}: {probe_dur(silent):.2f}s, {silent.stat().st_size / 1e6:.1f} MB")
    for s in timeline:
        print(f"{s['start']:7.2f} {s['end']:7.2f} {s['id']}")


def wav_dur(p):
    with wave.open(str(p)) as w:
        return w.getnframes() / w.getframerate()


def srt_time(x):
    ms = int(round(x * 1000))
    return f"{ms // 3600000:02d}:{ms // 60000 % 60:02d}:{ms // 1000 % 60:02d},{ms % 1000:03d}"


def cmd_mix():
    timeline = json.loads((VIDEO / "timeline.json").read_text(encoding="utf-8"))
    silent = VIDEO / "_assembly" / "combined_silent.mp4"
    total = probe_dur(silent)
    vo = VIDEO / "vo"
    cues, inputs, bad = [], [], []
    for k, s in enumerate(timeline):
        wav = vo / f"{s['id']}.wav"
        if not wav.exists():
            raise SystemExit(f"missing {wav}")
        d = wav_dur(wav)
        start = s["start"] + LEAD
        nxt = timeline[k + 1]["start"] if k + 1 < len(timeline) else total
        end = start + d
        s.update(vo_start=round(start, 3), vo_end=round(end, 3), vo_dur=round(d, 3))
        if end > nxt - 0.1:
            bad.append(f"{s['id']}: ends {end:.2f}s, next segment at {nxt:.2f}s")
        cues.append((start, min(end + 0.3, nxt - 0.05), s["text"]))
        inputs.append((wav, start))
    if bad:
        print("OVERRUN:\n  " + "\n  ".join(bad))
        raise SystemExit(1)
    (VIDEO / "timeline.json").write_text(json.dumps(timeline, indent=1), encoding="utf-8")
    srt = VIDEO / "uslu-dur-graphics-remaster.srt"
    srt.write_text("".join(f"{i + 1}\n{srt_time(a)} --> {srt_time(b)}\n{t}\n\n" for i, (a, b, t) in enumerate(cues)),
                   encoding="utf-8")
    # narration track: each line delayed to its start, mixed, loudness-normalised
    cmd = [FF, "-y", "-loglevel", "error"]
    for wav, _ in inputs:
        cmd += ["-i", str(wav)]
    fl = "".join(f"[{k}:a]aresample=48000,adelay={int(st * 1000)}:all=1[a{k}];" for k, (_, st) in enumerate(inputs))
    fl += "".join(f"[a{k}]" for k in range(len(inputs)))
    fl += f"amix=inputs={len(inputs)}:normalize=0:duration=longest,apad,atrim=0:{total:.3f}," \
          f"loudnorm=I=-16:TP=-1.5:LRA=11,aresample=48000[out]"
    track = VIDEO / "_assembly" / "narration.wav"
    cmd += ["-filter_complex", fl, "-map", "[out]", "-ac", "2", "-c:a", "pcm_s16le", str(track)]
    run(cmd)
    final = VIDEO / "uslu-dur-graphics-remaster.mp4"
    run([FF, "-y", "-loglevel", "error", "-i", str(silent), "-i", str(track), "-map", "0:v", "-map", "1:a",
         "-c:v", "copy", "-c:a", "aac", "-b:a", "192k", "-shortest", "-movflags", "+faststart", str(final)])
    # burned-in subtitles (run from VIDEO so the filter sees a plain relative path)
    sub = VIDEO / "uslu-dur-graphics-remaster-subtitled.mp4"
    style = "FontName=Arial,FontSize=15,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,BackColour=&H80000000," \
            "BorderStyle=4,Outline=1,Shadow=0,MarginV=34"
    subprocess.run([FF, "-y", "-loglevel", "error", "-i", final.name, "-vf",
                    f"subtitles=uslu-dur-graphics-remaster.srt:force_style='{style}'", "-c:v", "libx264", "-crf",
                    str(CRF), "-preset", "slow", "-pix_fmt", "yuv420p", "-c:a", "copy", "-movflags", "+faststart",
                    sub.name], check=True, cwd=str(VIDEO))
    for p in (final, sub, srt):
        print(p, f"{p.stat().st_size / 1e6:.1f} MB")
    for s in timeline:
        print(f"{s['id']:16s} seg {s['start']:7.2f}-{s['end']:7.2f}  vo {s['vo_start']:7.2f}-{s['vo_end']:7.2f}")


if __name__ == "__main__":
    {"lines": cmd_lines, "video": cmd_video, "mix": cmd_mix}[sys.argv[1]]()
