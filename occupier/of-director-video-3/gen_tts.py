"""Generate one narration clip per spoken line in cues.json with the repo's TTS
script (arma_CWR/voice-lines/generate_voice.py, OpenRouter, key from the
gitignored .env).  Clips are named by the md5 of the line, so
render_compare.py finds them.

Usage: python gen_tts.py cues.json
"""
import hashlib
import json
import os
import subprocess
import sys

GEN = r"D:\Arma_CWA\arma_CWR\voice-lines\generate_voice.py"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tts")
DIRECTION = "calm, clear documentary narrator explaining a strategy game, measured pace, neutral tone"


def main(path):
    spec = json.load(open(path, encoding="utf-8"))
    lines = [c["voice"] for c in spec["cards"] if c.get("voice")]
    lines += [c["text"] for c in spec["cues"]]
    if spec.get("outro_voice"):
        lines.append(spec["outro_voice"])
    os.makedirs(OUT, exist_ok=True)
    words = sum(len(t.split()) for t in lines)
    print("%d lines, %d words" % (len(lines), words))
    for text in lines:
        name = hashlib.md5(text.encode("utf-8")).hexdigest()[:12] + ".wav"
        p = os.path.join(OUT, name)
        if os.path.exists(p):
            continue
        for attempt in range(3):
            r = subprocess.run([sys.executable, GEN, "--line", text, "--direction", DIRECTION, "--voice", "sage", "--out", p], capture_output=True, text=True, timeout=240)
            if os.path.exists(p):
                break
        print(name, (r.stdout + r.stderr).strip()[-200:])


if __name__ == "__main__":
    main(sys.argv[1])
