"""Check the rendered narration: transcribe every line's WAV with faster-whisper
large-v3 (the cached model only, nothing is downloaded) and compare it with its
text, and check that each line fits its slot.

    <resolve-edit venv python> check_vo.py [--wav-dir <dir>] [--lines <lines.json>] [--device cuda|cpu]
                                           [--only id,id] [--out <json>]

A line passes when the transcript matches the text word for word after
normalising case and punctuation (similarity >= 0.9, no missing words; the
brand "Uslu dur" may come back in any spelling), and when the line ends at least
BREATH s before its window does (it starts LEAD s in; vo_v2.py's rule).
Run it with C:/dev/resolve-edit/.venv/Scripts/python.exe (faster-whisper 1.2).
"""
import argparse
import difflib
import json
import re
import sys
import wave
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

# vo_v2 and mix_vo need Pillow, which the whisper venv lacks: the same
# constants here, and the windows from the montage's timeline.json.
V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")
LEAD = 0.35
BREATH = 0.6
WAV_DIR = Path("C:/dev/local-tts/outputs/uslu-dur-video")
BRAND = "usludur"
# Whisper's vocabulary hint: the script's proper nouns and game words, so it spells what it
# hears the way the script does (it otherwise writes "Averon" for Everon).
PROMPT = "Uslu dur, Arma: Cold War Assault. Malden, Everon, Sinai. Guerrillas, Guerrilla Mode, the AI."
# Words the ear cannot tell apart; the transcript's spelling is mapped to the script's.
HOMOPHONES = {"gorillas": "guerrillas", "gorilla": "guerrilla"}
DIGITS = {"1": "one", "2": "two", "3": "three", "4": "four", "5": "five"}


def norm(text):
    t = text.lower().replace("’", "'")
    t = re.sub(r"[^a-z0-9' ]+", " ", t)
    words = t.split()
    out, i = [], 0
    while i < len(words):
        w = words[i]
        # the brand in any spelling whisper picks ("Uslu dur", "Usludur", "Oslo door", ...)
        if i + 1 < len(words) and re.fullmatch(r"(u|o)s(l|t)?(u|o)", w) and re.fullmatch(r"d(u|o)+(r|re)?", words[i + 1]):
            out.append(BRAND)
            i += 2
            continue
        # spelt out in three pieces ("Us lu dur")
        if (i + 2 < len(words) and re.fullmatch(r"(u|o)s", w) and re.fullmatch(r"l(u|o)", words[i + 1])
                and re.fullmatch(r"d(u|o)+r", words[i + 2])):
            out.append(BRAND)
            i += 3
            continue
        if re.fullmatch(r"(u|o)sl?u?d(u|o)+r", w):
            out.append(BRAND)
            i += 1
            continue
        out.append(HOMOPHONES.get(w, DIGITS.get(w, w)))
        i += 1
    return out


def windows(timeline):
    """line id -> (start, end) of its window (mix_vo.windows, keyed by line id)."""
    if "windows" in timeline:
        return {k: tuple(v) for k, v in timeline["windows"].items()}
    seg = timeline["segments"]
    out = {}
    for i, x in enumerate(seg):
        if x["kind"] == "intro":
            out["intro"] = (x["start"], x["end"])
        elif x["kind"] == "outro":
            out["outro"] = (x["start"], x["end"])
        elif x["kind"] == "card":
            end = next((y["end"] for y in seg[i + 1:] if y["kind"] == "clip" and y["clip"] == x["clip"]), x["end"])
            out[x["clip"]] = (x["start"], end)
    return out


def wav_seconds(p):
    with wave.open(str(p), "rb") as w:
        return w.getnframes() / float(w.getframerate())


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--wav-dir", default=str(WAV_DIR))
    ap.add_argument("--lines", default=str(V2 / "lines.json"))
    ap.add_argument("--timeline", default=str(V2 / "out-split" / "timeline.json"))
    ap.add_argument("--device", default="cuda")
    ap.add_argument("--only", default="")
    ap.add_argument("--out", default=str(V2 / "vo_check.json"))
    a = ap.parse_args(argv)
    from faster_whisper import WhisperModel

    lines = json.loads(Path(a.lines).read_text(encoding="utf-8"))
    want = {s for s in a.only.split(",") if s}
    wins = windows(json.loads(Path(a.timeline).read_text(encoding="utf-8")))
    ctype = "float16" if a.device == "cuda" else "int8"
    model = WhisperModel("large-v3", device=a.device, compute_type=ctype, local_files_only=True)
    results, bad = [], 0
    for ln in lines:
        if want and ln["id"] not in want:
            continue
        wav = Path(a.wav_dir) / f"{ln['id']}.wav"
        if not wav.exists():
            print(f"{ln['id']}: MISSING {wav}")
            bad += 1
            continue
        segs, _ = model.transcribe(str(wav), language="en", beam_size=5, vad_filter=False,
                                   initial_prompt=PROMPT)
        heard = " ".join(s.text.strip() for s in segs).strip()
        ref, got = norm(ln["text"]), norm(heard)
        sm = difflib.SequenceMatcher(a=ref, b=got)
        missing, extra = [], []
        for op, i1, i2, j1, j2 in sm.get_opcodes():
            if op in ("delete", "replace"):
                missing += ref[i1:i2]
            if op in ("insert", "replace"):
                extra += got[j1:j2]
        dur = wav_seconds(wav)
        w0, w1 = wins.get(re.sub(r"^v2_\d\d_", "", ln["id"]), (0, 1e9))
        room = (w1 - w0) - LEAD - dur
        ok_words = sm.ratio() >= 0.9 and not missing
        ok_fit = room >= BREATH
        ok = ok_words and ok_fit
        bad += 0 if ok else 1
        results.append(dict(id=ln["id"], ok=ok, words_ok=ok_words, fits=ok_fit, ratio=round(sm.ratio(), 3),
                            seconds=round(dur, 2), room=round(room, 2), missing=missing, extra=extra,
                            text=ln["text"], heard=heard))
        print(f"{ln['id']:22s} {'OK  ' if ok else 'FAIL'} ratio {sm.ratio():.3f} {dur:5.2f} s room {room:5.2f}"
              f"{'  missing ' + ' '.join(missing) if missing else ''}{'  extra ' + ' '.join(extra) if extra else ''}")
        print(f"    heard: {heard}")
    Path(a.out).write_text(json.dumps(results, indent=1), encoding="utf-8")
    print(f"{len(results) - bad if bad <= len(results) else 0}/{len(results)} pass -> {a.out}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
