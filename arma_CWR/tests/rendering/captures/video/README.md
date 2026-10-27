# Trailer and voice tooling (branch `video-production`)

This folder holds the tools that made "Uslu dur! trailer, part 1: graphics": the clip staging,
the side-by-side montage, the narration (owner read, ElevenLabs through OpenRouter, local TTS),
music, Shorts captions and the thumbnail. They are too heavy and too tied to one production for
`main`, so they live only on the `video-production` branch.

## What the branch is

`video-production` = the fork's `main` + the `graphics-remaster` branch + this tooling. Nothing
else. The engine, the capture rig (`../capture_lib.sqf`, `../run-captures.ps1`, `capture_motion.sqf`
here, the vanilla rig in `../vanilla/`), the fog metrics in `../fogcheck/` and the docs come from
`graphics-remaster`; the branch only adds:

- `*.py`, `run_v2_lane.ps1`, `udpaths.py`, `owner_take1/`, `thumbnail_bay/` in this folder
- `../fogcheck/gen_probe_fog1.py`, `../fogcheck/gen_rt_probe.py` (probes cut from the trailer's dawn shot)
- `arma_CWR/docs/graphics-remaster/research/HANDOFF-2026-10-19.md` and `HANDOFF-2026-10-23.md`
  (the production handoffs; they used to sit in the engine tree)

## Keeping it current

Merge, never rebase (the branch is shared history):

    git fetch origin
    git merge origin/main
    git merge origin/graphics-remaster     # while graphics-remaster is not yet merged into main

Once `graphics-remaster` is in `main`, the first merge alone is enough. Conflicts, if any, are
usually in `roadmap/roadmap.yaml`: keep both sides' items. Never commit media (frames, clips,
audio, thumbnails): the game-data guard in `.githooks/` blocks them, and they live in the work
folder below.

## Where things live

`udpaths.py` resolves every location; set an environment variable to move one:

| Variable | What | Default |
|---|---|---|
| `UD_VIDEO_WORK` | work folder: clips, frames, montages, voice, `final/` | `<repo>/tmp/video/v2` |
| `UD_VANILLA_ROOT` | checkout of Bohemia's upstream engine for the vanilla side | `<repo>/../vanilla-upstream` |
| `UD_VANILLA_LOBO` | @LoBo addons + dta (no Bin) for the vanilla side | `<UD_VANILLA_ROOT>/tmp/lobo-mods` |
| `UD_FONT_DIR` | the game's OFL fonts | `arma_CWR/thirdparty/guerrilla-fonts` |
| `UD_WHISPER_PYTHON` | a Python with faster-whisper (split, listen-check) | the running Python |
| `UD_TTS_OUT` | WAVs of a TTS take | `<UD_VIDEO_WORK>/tts` |
| `UD_ENV_FILE` | `.env` with `OPENROUTER_API_KEY` | the main checkout's `.env` |
| `FFMPEG`, `FFPROBE` | ffmpeg tools | found on PATH |
| `UD_MUSIC_DIR` | the CC BY 4.0 album (only `owner_take1/run_all.sh`) | none, required |

`run_v2_lane.ps1` takes `-V2` and `-VanillaRoot` (same defaults). The OpenRouter key stays in the
`.env`, which is never committed; `tts_openrouter.py` reads it and never prints it.

## The pipeline, in order

1. `gen_video_v2.py --side both` writes the clip tests (generated, ignored by `.gitignore`).
2. `run_v2_lane.ps1 -Side vanilla` and `-Side ud -GameDir <dist>` shoot them, one game at a time,
   and `collect_frames.py` gathers the frames, checking which build and lighting mode each side came
   from (the ud side must be the overhaul build, the vanilla side the upstream build).
3. `trailer_v2.py` (and `--layout sbs`, `--layout short`) renders the montages.
4. Voice: `owner_vo.py all` then `finish` for a human read; `tts_openrouter.py` or the local TTS for
   synthetic voices, then `mix_vo.py` and `check_vo.py`; `short_captions.py` for Shorts.
5. `owner_take1/` (the take that shipped, with music) and `thumbnail_bay/` (the thumbnail sheets).

Older v1 tools (`gen_video_tests.py`, `make_video.py`, `make_video_v2.py`, `vo_v2.py`) are kept
because the v2 tools import them.

## Cleanup warning

Game-running worktrees hold directory junctions in `arma_CWR/packages` that point into the shared
Remaster shadow. Remove each junction on its own (`cmd /c rmdir <junction>`) before deleting a
worktree folder; a recursive delete follows the junction and wipes the shadow's game data.
