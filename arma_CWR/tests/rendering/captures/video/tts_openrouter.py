"""Render the trailer narration with an ElevenLabs voice through OpenRouter's speech endpoint.

Same technique as arma_CWR/voice-lines/generate_voice.py: the OpenRouter key is read from the
repo-root .env (OPENROUTER_API_KEY), never printed. One named voice for every line, so the voice
cannot drift between lines.

    python tts_openrouter.py --voice lily [--model elevenlabs/eleven-v4] [--only id,id]
                             [--lines tmp/video/v2/lines.json] [--out <wav dir>] [--env D:/Arma_CWA/.env]

For each line writes <out>/<id>.mp3 (as received) and <out>/<id>.wav (24 kHz mono 16-bit, what
mix_vo.py --wav-dir reads).
"""
import argparse
import json
import subprocess
import sys
import urllib.error
import urllib.request
from pathlib import Path

API_URL = "https://openrouter.ai/api/v1/audio/speech"
FF = "C:/ffmpeg/bin/ffmpeg.exe"
V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")


def load_api_key(env_file: Path) -> str:
    for raw in env_file.read_text().splitlines():
        if raw.startswith("OPENROUTER_API_KEY="):
            return raw.split("=", 1)[1].strip()
    sys.exit(f"OPENROUTER_API_KEY not found in {env_file}")


def speak(key: str, model: str, voice: str, text: str, mp3: Path) -> None:
    body = {"model": model, "input": text, "voice": voice, "response_format": "mp3"}
    req = urllib.request.Request(API_URL, data=json.dumps(body).encode(),
                                 headers={"Authorization": f"Bearer {key}", "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=120) as resp:
            data = resp.read()
    except urllib.error.HTTPError as e:
        sys.exit(f"HTTP {e.code} for {mp3.stem}: {e.read()[:300]!r}")
    if not data:
        sys.exit(f"no audio for {mp3.stem}")
    mp3.write_bytes(data)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--voice", required=True)
    ap.add_argument("--model", default="elevenlabs/eleven-v4")
    ap.add_argument("--lines", default=str(V2 / "lines.json"))
    ap.add_argument("--out", default=str(V2 / "vo_elevenlabs"))
    ap.add_argument("--only", default="")
    ap.add_argument("--env", default="D:/Arma_CWA/.env")
    a = ap.parse_args()
    key = load_api_key(Path(a.env))
    lines = json.loads(Path(a.lines).read_text(encoding="utf-8"))
    only = {x for x in a.only.split(",") if x}
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    for ln in lines:
        if only and ln["id"] not in only:
            continue
        mp3, wav = out / f"{ln['id']}.mp3", out / f"{ln['id']}.wav"
        speak(key, a.model, a.voice, ln["text"], mp3)
        subprocess.run([FF, "-v", "error", "-y", "-i", str(mp3), "-ar", "24000", "-ac", "1", "-c:a", "pcm_s16le", str(wav)],
                       check=True)
        print(f"{ln['id']}: {wav}")


if __name__ == "__main__":
    main()
