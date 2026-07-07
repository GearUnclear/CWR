# Custom voice lines

How we generate new spoken voice lines with TTS and play them in-game. This covers the whole pipeline: generation via OpenRouter, the audio formats the engine accepts, and the config/script hookup.

## TL;DR

1. Generate a WAV with `generate_voice.py` (needs `OPENROUTER_API_KEY` in the repo-root `.env`).
2. Drop the file into the mission's `sound\` folder (OGG or WAV both work, no `.wss` conversion needed).
3. Declare it in `CfgSounds` in the mission's `description.ext`.
4. Trigger with `unit say "lineName"` (positional 3D speech) or `unit sideRadio "lineName"` (radio traffic with mic clicks).

No engine changes are required for any of this.

## 1. Generating audio

```
python generate_voice.py --line "Ahhh, I'm taking fire!" \
    --direction "panicked soldier under enemy fire, distressed, shouting" \
    --voice ash \
    --out samples/taking_fire.wav
```

- Model: `openai/gpt-audio-mini` via OpenRouter (chat completions with audio output).
- Key comes from `OPENROUTER_API_KEY` in the repo-root `.env` (gitignored, never commit it).
- Voices: `alloy`, `ash`, `ballad`, `coral`, `echo`, `sage`, `shimmer`, `verse`. `ash` is our default male-soldier voice.
- `--direction` is free-text acting direction ("wary, hushed urgency over the radio", "shouting over gunfire", etc.).
- Output is 24 kHz mono 16-bit PCM WAV, which the engine plays directly.

The script prints the transcript of what was actually spoken. **Check it matches your line**; see gotchas below.

### API gotchas (learned the hard way)

- **Audio output requires `stream: true`.** A plain non-streaming request returns HTTP 400 ("Audio output requires stream: true"). The script parses the SSE stream and concatenates the base64 `delta.audio.data` chunks.
- **Streaming audio arrives as raw `pcm16`**, not a WAV container. The script wraps it in a RIFF/WAV header itself (24 kHz, mono, 16-bit).
- **The model improvises if you let it.** A polite "say this line" system prompt got back ad-libbed squad chatter ("Stay down! Get to cover now, move!") instead of the requested line. The fix is framing it as a verbatim line-reading tool and quoting the script line in the user message. Always verify the printed transcript.

### Sample lines

`samples/` holds the first two generated lines:

| File | Line | Direction |
|---|---|---|
| `taking_fire.wav` | "Ahhh, I'm taking fire!" | panicked soldier under enemy fire, shouting |
| `roadblock_ahead.wav` | "Looks like there's a roadblock up ahead." | wary soldier on patrol, tense, hushed urgency |

The delivery lands surprisingly close to the vanilla 2001 voice acting (flat, single-take radio reads), so new lines blend in with the stock voice banks.

## 2. What the engine can play

Format dispatch is by file extension in `SoundLoadFile` (`engine/Poseidon/Audio/Streaming/WaveStream.cpp:146`):

- `.wav` (PCM) via `WaveLoaders.cpp`
- `.ogg` (Vorbis, real libvorbis decoder) via `WaveStreamOGG.cpp`
- anything else falls through to the `.wss` loader (BI's own container)

So TTS output needs **no conversion to `.wss`**. Use WAV as generated, or transcode to OGG Vorbis if size matters (e.g. `ffmpeg -i line.wav -c:a libvorbis -q:a 4 line.ogg`).

## 3. Hooking lines into the game

There are two speech systems in the engine. Use the second one.

### The radio word bank (do NOT use for custom lines)

AI radio chatter ("2, move to...", contact reports) is assembled word-by-word from `.wss` clips in `CfgVoice >> Words` (`DTA\Voice.pbo`), resolved in `BasicSpeaker::Say` (`Audio/Speaker.cpp:50`), which hard-codes the `.wss` extension. New sentence *types* are C++ `RadioMessage` subclasses (`AI/AIRadio.cpp:1249-1310`, `AIRadioImpl.cpp`). Adding a line here means per-word clips plus engine work. Only worth it if a line must flow through the AI radio queue with priority handling.

### CfgSounds + `say` (the path we use)

Whole-clip playback, fully config-driven. `FindSound` (`UI/OptionsUI.cpp:419`) resolves the class name searching mission `description.ext`, then campaign, then global config, so this works from a plain mission or a distributable addon.

Mission layout:

```
MyMission.Abel\
  description.ext
  sound\
    taking_fire.ogg
    roadblock_ahead.ogg
```

`description.ext`:

```c
class CfgSounds {
    sounds[] = { taking_fire, roadblock_ahead };
    class taking_fire {
        name = "taking_fire";
        sound[] = { "sound\taking_fire.ogg", db+5, 1.0 };   // {file, volume(dB), pitch}
        titles[] = { 0, "Ahhh, I'm taking fire!" };          // subtitle at t=0s (optional)
    };
    class roadblock_ahead {
        name = "roadblock_ahead";
        sound[] = { "sound\roadblock_ahead.ogg", db+5, 1.0 };
        titles[] = { 0, "Looks like there's a roadblock up ahead." };
    };
};
```

Triggering from SQF/SQS or a trigger:

```sqf
soldier1 say "taking_fire";                  // 3D positional, from the soldier's mouth
soldier1 say ["roadblock_ahead", 200, 1.0];  // explicit audible distance + pitch
playSound "taking_fire";                     // 2D, non-positional UI cue
~ (soundLength "roadblock_ahead") + 0.3      // SQS: pace follow-up dialogue to clip length
```

Command implementations: `say` at `Game/Commands/GameStateExtUi.cpp:993`, `playSound` at `:1054`, `soundLength` at `:1067`.

### Radio-flavored variant: CfgRadio + `sideRadio`

If a line should sound like radio traffic (mic click, radio channel display), declare it in `class CfgRadio` (same structure, `sentence = "..."` instead of `titles[]`) and trigger with `unit sideRadio "msg"` / `groupRadio` / `vehicleRadio` (`GameStateExtUi.cpp:1153`, resolved by `FindRadio` at `OptionsUI.cpp:517`). Still whole-clip OGG/WAV, still config-only.

### Bonus: per-language variants

`FindSound` applies a language suffix override (`Audio/VoiceLangPath.hpp`): if `taking_fire.<Lang>.ogg` exists next to the base file, it is used for that voice language. Free localization hook for faction-specific voices.

## 4. Working examples in the repo

- `tests/integration/missions/sound_preload.Demo/description.ext` - minimal WAV CfgSounds + `player say` test
- `tests/fixtures/audio/voice-lang-mission/voice_test.demo/description.ext` - OGG + subtitles + language override
