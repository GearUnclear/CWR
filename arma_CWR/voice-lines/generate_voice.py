#!/usr/bin/env python3
"""Generate a spoken voice line as a WAV file via OpenRouter (openai/gpt-audio-mini).

Reads OPENROUTER_API_KEY from the repo-root .env. OpenRouter requires
stream=true for audio output, so this parses the SSE stream and
concatenates the base64 audio deltas.

Usage:
  python generate_voice.py --line "Ahhh, I'm taking fire!" \
      --direction "panicked soldier under fire, shouting" \
      --out samples/taking_fire.wav
"""
import argparse
import base64
import json
import sys
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
API_URL = "https://openrouter.ai/api/v1/chat/completions"


def load_api_key() -> str:
    env_file = REPO_ROOT / ".env"
    for raw in env_file.read_text().splitlines():
        if raw.startswith("OPENROUTER_API_KEY="):
            return raw.split("=", 1)[1].strip()
    sys.exit(f"OPENROUTER_API_KEY not found in {env_file}")


def generate(line: str, direction: str, voice: str, out_path: Path) -> None:
    body = {
        "model": "openai/gpt-audio-mini",
        "modalities": ["text", "audio"],
        "audio": {"voice": voice, "format": "pcm16"},  # pcm16 required when streaming
        "stream": True,
        "messages": [
            {
                "role": "system",
                "content": (
                    "You are a verbatim line-reading tool, not a conversational assistant. "
                    "The user gives you a script line in quotes; your entire spoken output "
                    "must be that line word for word - no additions, no improvisation, no "
                    f"responses to its content. Delivery/acting direction: {direction}"
                ),
            },
            {"role": "user", "content": f'Read this line: "{line}"'},
        ],
    }
    req = urllib.request.Request(
        API_URL,
        data=json.dumps(body).encode(),
        headers={
            "Authorization": f"Bearer {load_api_key()}",
            "Content-Type": "application/json",
        },
    )

    audio = bytearray()
    transcript = []
    with urllib.request.urlopen(req) as resp:
        for raw in resp:
            raw = raw.decode("utf-8").strip()
            if not raw.startswith("data: ") or raw == "data: [DONE]":
                continue
            chunk = json.loads(raw[len("data: "):])
            delta = (chunk.get("choices") or [{}])[0].get("delta") or {}
            a = delta.get("audio") or {}
            if a.get("data"):
                audio.extend(base64.b64decode(a["data"]))
            if a.get("transcript"):
                transcript.append(a["transcript"])

    if not audio:
        sys.exit("No audio received from API")

    # pcm16 stream is raw 24 kHz mono 16-bit; wrap it in a WAV header.
    import struct
    sample_rate, channels, bits = 24000, 1, 16
    byte_rate = sample_rate * channels * bits // 8
    header = (
        b"RIFF" + struct.pack("<I", 36 + len(audio)) + b"WAVEfmt "
        + struct.pack("<IHHIIHH", 16, 1, channels, sample_rate, byte_rate,
                      channels * bits // 8, bits)
        + b"data" + struct.pack("<I", len(audio))
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(header + bytes(audio))
    print(f"Wrote {out_path} ({len(audio) // 1024} KB pcm, "
          f"{len(audio) / (sample_rate * 2):.1f}s) transcript: {''.join(transcript)!r}")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--line", required=True, help="Exact line to speak")
    p.add_argument("--direction", default="distressed soldier on a military radio",
                   help="Acting direction for delivery")
    p.add_argument("--voice", default="ash",
                   help="TTS voice (alloy, ash, ballad, coral, echo, sage, shimmer, verse)")
    p.add_argument("--out", required=True, type=Path, help="Output .wav path")
    args = p.parse_args()
    generate(args.line, args.direction, args.voice, args.out)
