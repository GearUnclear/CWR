"""Generate one narration clip per commentary line with the repo's TTS script
(arma_CWR/voice-lines/generate_voice.py, OpenRouter, key from the gitignored
.env). Clips are named by the md5 of the line, so render.py finds them."""
import hashlib, os, subprocess, sys
sys.path.insert(0, os.path.dirname(__file__))
import make_cues
GEN = r"D:\Arma_CWA\arma_CWR\voice-lines\generate_voice.py"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tts")
os.makedirs(OUT, exist_ok=True)
DIRECTION = "calm, clear documentary narrator explaining a strategy game, measured pace, neutral tone"
for pat, text, linepat, rate in make_cues.SPEC:
    name = hashlib.md5(text.encode("utf-8")).hexdigest()[:12] + ".wav"
    path = os.path.join(OUT, name)
    if os.path.exists(path):
        continue
    r = subprocess.run([sys.executable, GEN, "--line", text, "--direction", DIRECTION, "--voice", "sage", "--out", path], capture_output=True, text=True, timeout=180)
    print(name, (r.stdout + r.stderr).strip()[-300:])
