"""The v2 narration: one line for the intro, one per clip, one for the outro.
Writes tmp/video/v2/VO_SCRIPT.txt (the lines with their timing in the montage)
and tmp/video/v2/lines.json (the shape the local TTS's uslu_dur_vo.py reads:
[{"id": ..., "text": ...}]).

    python vo_v2.py

Tone: part 1 of an Uslu dur! trailer in the register of a calm
gameplay documentary (graphics only; part 2 shows the gameplay), read
by a pleasant, warm, confident female narrator: calm, measured, plain words,
each line naming what the player sees, dry understatement at most once or twice.
Original copy only: never Rockstar's or GTA's lines, characters or brands
(style notes: tmp/video/v2/reference/STYLE_NOTES.txt). "Uslu dur" is written without
the exclamation mark so the voice does not shout it.

Timing: each clip's line starts LEAD s into its title card and should end at
least BREATH s before the next card; 2.3 words per second is the planning
rate (the real lengths come from the rendered WAVs, which mix_vo.py checks).
"""
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import gen_video_v2  # noqa: E402
import make_video_v2  # noqa: E402

WPS = 2.3
LEAD = 0.35
BREATH = 0.6
V2 = make_video_v2.V2

INTRO = "Uslu dur is a total overhaul of Arma: Cold War Assault. This first video is about the light."
LINES = {
    "opening": "Malden, under occupation. The models are the originals. The light is new.",
    "valley_mist": "A resistance squad moves out at first light. The sun is still burning off the valley mist.",
    "pine_shafts": "In the pine woods, morning sun falls through the haze in shafts. The guerrillas keep to cover.",
    "bay_reflections": "On the bay, an occupier patrol boat crosses calm water that mirrors the hills and the sky.",
    "outpost": "A low winter sun lights the occupier outpost from the side. The shade stays cool and deep.",
    "cloud_shadows": "Over Everon, the clouds catch the sun, and their shadows move slowly across the hills.",
    "sinai_camp": "Modded islands get it too. In Sinai, a camp sits under netting in the late sun.",
    "ambush": "Golden hour. Guerrillas wait at the forest edge; every tree throws a long shadow downhill.",
    "convoy": "At sunset, an occupier convoy drives into the low sun.",
    "checkpoint": "At dusk, the sentries at the checkpoint gather around a fire.",
    "town_patrol": "At night, street lamps and the fire light the occupied town. A patrol comes down the street.",
    "rain_town": "Then the rain. Puddles catch the lamps, and the resistance slips past the patrol.",
    "supply_run": "Out in the country, the nights are dark. A supply run drives by its headlights alone.",
    "firefight": "When a firefight breaks out, every flare, fire and shell burst lights the field.",
}
OUTRO = "Uslu dur. Part two covers the gameplay: Guerrilla Mode and the AI."


def words(text):
    return len(text.replace(",", " ").replace(".", " ").split())


def plan():
    """[(id, text, window start, window end, segment label)] in montage order. The windows
    come from trailer_v2.plan(): each scene from the middle of the wipe that brings it in
    to the middle of the one that takes it away; the intro rides on the opening scene's
    first INTRO_UNTIL seconds, the outro on the closing card."""
    import trailer_v2
    _, total, wins = trailer_v2.plan()
    rows = [("v2_00_intro", INTRO, *wins["intro"], "intro, over the opening scene")]
    for i, c in enumerate(gen_video_v2.CLIPS, 1):
        rows.append((f"v2_{i:02d}_{c['clip']}", LINES[c["clip"]], *wins[c["clip"]],
                     f"scene {c['clip']} ({c['title']})"))
    rows.append(("v2_99_outro", OUTRO, *wins["outro"], "closing card"))
    return rows


def fmt(s):
    return f"{int(s // 60)}:{s % 60:05.2f}"


def main():
    rows = plan()
    out = ["Uslu dur! comparison video v2: narration draft",
           "==============================================",
           "",
           "Voice: a pleasant, warm, confident woman, narrating a trailer, calm and measured",
           "(the instruction in the local TTS's uslu_dur_vo.py). Original copy; nothing from Rockstar or GTA. Part 1 of 2: graphics only.",
           f"Planning rate {WPS} words a second. Each line starts {LEAD} s into its scene's window and",
           f"should end {BREATH} s or more before the window ends. A window runs from the middle of the",
           "transition that brings the scene in to the middle of the one that takes it away (titles are",
           "overlays now, no title cards). Times are positions in the montage (trailer_v2.py);",
           "mix_vo.py places the rendered lines by the same rule.",
           "The spoken lines write the brand as \"Uslu dur\" (no exclamation mark) so the voice does not",
           "shout it; the subtitles (mix_vo.py) put the mark back.",
           "Owner approved recording on 2026-10-19; rendered after the overhaul clips are shot, with no games on the GPU.",
           "",
           f"{'id':22s} {'window':>17s}  {'words':>5s}  {'est. s':>6s}  {'room s':>6s}",
           ]
    worst = 99.0
    for rid, text, a, b, label in rows:
        est = words(text) / WPS
        room = (b - a) - LEAD - est
        worst = min(worst, room)
        out.append(f"{rid:22s} {fmt(a):>8s}-{fmt(b):>8s}  {words(text):5d}  {est:6.1f}  {room:6.1f}   {label}")
        out.append(f"    \"{text}\"")
        out.append("")
    total = rows[-1][3]
    out.append(f"Montage length {fmt(total)}; smallest room after a line {worst:.1f} s.")
    (V2 / "VO_SCRIPT.txt").write_text("\n".join(out) + "\n", encoding="utf-8")
    (V2 / "lines.json").write_text(json.dumps([{"id": r[0], "text": r[1]} for r in rows], indent=1) + "\n",
                                   encoding="utf-8")
    print("\n".join(out))
    if worst < BREATH:
        print(f"WARNING: a line leaves only {worst:.1f} s before the next card")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
