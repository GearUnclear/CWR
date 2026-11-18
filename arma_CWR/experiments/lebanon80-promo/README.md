# Lebanon80 promo: shoot, check and edit tools (branch `video-production`)

These tools shot and cut the Uslu dur! Guerrilla Mode promo on the Lebanon80 template (IDF occupier,
Hizballah resistance, the @LoBo Lebanon80 island, Dedushka the kaiju on the occupier's side). The owner
treats that shoot as test footage: what it left behind is this tool set and the hard rules below, each one
learned from a take that had to be shot again. Use them for the next promo, on any island.

The production itself (clips, run logs, sheets, stems, the cut, and the shoot, pop-in, night and edit
notes) stays outside git in the production folder. Comments that cite `shoot-notes.md`,
`popin-diagnosis.md`, `night-diagnosis.md` or `edit-notes.md` mean the notes in that folder; the rules
they led to are written out here.

## Where things live

Every location resolves through `promopaths.py`, which reads `udpaths.py` in
`arma_CWR/tests/rendering/captures/video/` (see the README there). Set a variable to move one:

| Variable | What | Default |
|---|---|---|
| `UD_PROMO_DIR` | the production folder: `clips/`, `work/` (runs, logs, gates, judder, post, sheets, edit), the final cut | `<worktree>/tmp/video/lebanon80-promo` |
| `UD_PROMO_AUDIO` | the edit's music, a WAV muxed untouched | `<UD_PROMO_DIR>/ud-lebanon80-promo-instrumental.wav` |
| `UD_SHOOT_SHADOW` | the shoot's Remaster data dir (`run-promo.ps1 -DataDir`) | `<main checkout>/tmp/remaster-shadow-video` |
| `UD_SHOOT_GAMEDIR` | the game build (`-GameDir`) | `<worktree>/arma_CWR/dist/video-rwdi` |
| `UD_TRI_HIDDEN_DESKTOP` | the hidden-desktop wrapper for tri (`-Wrapper`) | `<main checkout>/tmp/tri-hidden-desktop.ps1` |
| `UD_OWNER_HOLD_FILE` | a file whose presence pauses the queue (`-HoldFile`) | the perf squad's `OWNER-HOLD.txt` in the sibling `graphics-remaster` worktree |
| `UD_FONT_DIR`, `FFMPEG`, `FFPROBE` | fonts and ffmpeg, as in `udpaths.py` | repo fonts, ffmpeg on PATH |

Python 3 with numpy and Pillow runs the shoot and edit tools; the audio tools also need librosa, scipy,
soundfile, torch, torchaudio, demucs and faster-whisper (a separate venv is fine).

## What is here

Shoot:

- `gen_promo.py` writes the takes: `promo_<take>.test.sqf` (the tri script), `.test.toml` (mission, mods,
  window size) and `.shots.json` (each clip's length, note and thirds gate). All three are generated and
  gitignored; edit the generator. `promolib.py` holds the take header, the camera (`cam`), the settle and
  the recorder (`record`).
- `PromoShoot.Lebanon80/`: the shoot mission. `description.ext` is a verbatim copy of
  `guerrilla-mode/mission/Guerrilla.Lebanon80` and `init.sqs` is the template's with a header comment
  (copy them again when the template changes); `mission.sqm` adds the @LoBo addons the staging needs and
  names the briefing "Guerrilla". `promo_lib.sqf` is the staging library (`ps*`: units, vehicles, props,
  road headings, kjSteer patrols) and the camera (`pc*`: moves, aim, the PCF log); the `.sqs` files are the
  per-clip helpers (camera step, civilian clear, ambush, roadblock, convoy, helicopter paths).
- `sync-kaiju-mission.sh` + `sync_kaiju_patch.py` build `PromoKaiju.Lebanon80` (gitignored) from the shoot
  mission and `experiments/kaiju/integration/kj`, adding the kaiju addons and the tremor guard.
- `promo_probe`, `promo_scout`, `promo_scout2` (`.test.sqf/.toml`): the hand-written probes that scouted
  the island (classes, road points and houses, AI movement); `plot_scout.py` maps the scout's output.
- `run-promo.ps1` runs one take, `run-batch.sh` several, `check-branches.sh` the pre-shoot branch check.

Checks (each reads a run folder `work/runs/<take>-<stamp>` and the clips):

- `check_run.py`: pin, finest detail and settle before every clip, every dump written, script errors.
- `gate.py`: the thirds gate from the PCF log, and the take's thirds sheet `clips/thirds-sheet-<take>.jpg`.
- `judder.py`: camera steps and kaiju tremor from the log, whole-picture jerk from the clip;
  `judder_plot.py` draws a tremor on/off proof.
- `popscan.py`: one-frame steps (pop-in, detail snaps, judder) per clip.
- `post_take.py`: waits for the encodes, then all four above; `post_watch.sh` runs it on every finished run.
- `nightcheck.py`: levels and the moon feature gate of every `_night_` clip. `sun.py`: the engine's sun and
  moon for any date and time.
- `encode_promo.py`: dumps to clips (head frames cut, raw dumps deleted once the mp4 reads back).
- `final_report.py`, `final_sheets.py`, `make_shotlist.py` (with the hand-written ratings in
  `review_table.py`), `review.py`: the per-clip report, the delivered thirds sheets and the shot list.
  `rename_dusk.py` renames clips that turned out to be dusk and carries the rename through every result.

Edit (`edit/`) and music (`audio/`), kept beside the shoot tools because they are one pipeline: the cut
list names this shoot's clips, the edit's night gate re-reads the shoot's night rule, and one README and
one `promopaths.py` cover shoot to delivery. Their general variables are in `udpaths.py` all the same.

- `edit/build.py`: the cut (CUTS), text cards, grade and lock points at the top of one script; renders an
  EDL to exact frame counts, muxes the music untouched, runs the night luma gate, writes the plain and
  thirds sheets of the cut and a 720p phone copy. `--dry-run` checks every shot against its clip and writes
  nothing. `make_notes.py` writes `edit-notes.md`; `frames_at.py`, `scout_sheets.py` are stills for
  checking; `luma.py` prints per-shot levels of the cut; `grade.py` measures the grade on a grey ramp and
  fails if it lifts black.
- `audio/separate.py` (Demucs stems from a source WAV), `mkinst.py` (drums + bass + other into the
  instrumental), `whisper_scan.py` (word counts and times only, never text), `envstats.py`, `hot.py` (voice
  left in the instrumental), `beats.py` (first look), `beats_final.py` (the beat map the cut uses).

## Setup: data, shadow, build

- **Branch check first.** `bash check-branches.sh` lists every commit on the engine branch tips
  (`main`, `graphics-remaster`, `occupier-graphics`, or the ones you name) that the shot branch lacks.
  Merge what it lists (never rebase), rebuild, then shoot.
- **Build**: a RelWithDebInfo build of this branch staged as `arma_CWR/dist/video-rwdi` (or pass
  `-GameDir`), and `tri` at `arma_CWR/target/debug/tri.exe`. See `arma_CWR/CLAUDE.md` for the build.
- **Data**: a Remaster shadow kept for the shoot alone (default `<main checkout>/tmp/remaster-shadow-video`),
  never the Steam install and never a shadow other sessions test on. `@LoBo` sits inside the data dir (the
  takes mount it with `--mods-dir . --mod @LoBo`); install and repair it as `setup-guerrilla.ps1` does.
  Kaiju takes also need `@kj_dedushka` in the same folder: build it with
  `experiments/kaiju/integration/addon/build-kj-dedushka.ps1 -ModDir <shadow>/@kj_dedushka`, then run
  `bash sync-kaiju-mission.sh` here. Do not change the shadow during a shoot.
- **Disk**: a 2560 x 1440 dump is about 11 MB a frame (a 10 s clip about 3.5 GB). `run-promo.ps1` stops
  below 15 GB free on the production drive; `run-batch.sh` waits for earlier encodes until 50 GB are free.

## Running a take

    python gen_promo.py land                                   # writes promo_land.test.sqf/.toml/.shots.json
    powershell -NoProfile -File run-promo.ps1 -Name promo_land # prints OUT=<run folder> at the end
    python encode_promo.py <run folder>                        # clips/<clip>.mp4, sheets, dumps deleted
    python post_take.py <run folder>                           # check_run, gate, judder, popscan

A retake records only the named clips and lets the others' time pass, so the staging keeps its timing:

    python gen_promo.py land --only c02_dawn_olive_plain_push,c03_day_ghajar_village_dolly --tag r2
    powershell -NoProfile -File run-promo.ps1 -Name promo_land_r2

## Running a batch

    python gen_promo.py land occ1 occ2 guer1                   # or: all (probes excluded)
    bash post_watch.sh &                                       # checks each run as it finishes
    bash run-batch.sh land occ1 occ2 guer1                     # one game at a time, encodes in between

For the kaiju takes run `bash sync-kaiju-mission.sh` after any change to `PromoShoot.Lebanon80` or `kj/`.
After the batch: `python nightcheck.py` (night clips), `python final_report.py`, `python final_sheets.py`,
`python make_shotlist.py`.

## Shoot checklist (hard rules)

Each rule came from footage that failed it. A take that breaks one is shot again, not fixed in the edit.

1. **Check every engine branch tip against video-production before any shoot** (`check-branches.sh`). The
   first shoot lacked 11 graphics-remaster commits, the pop-in fix among them.
2. **Pin and settle before every clip.** The take header calls `triPinObjectDetail 1` right after
   `triSetLighting "remastered"`; every clip is preceded by `triSettleScene [300,30,2]` on its first pose
   with the world paused, and each recording holds that pose for 15 head frames that `encode_promo.py`
   drops. Without the pin, one slow frame coarsens object detail up to about 9x for about 5 s (issue #219).
   `check_run.py` fails a clip whose settle does not read the finest detail with `pin 1` (`lod 0.0078125`
   at 2560 wide, `0.010416667` at 1920). A view with many lights or thousands of trees may never report
   settled although it is still; that is a known swing in the draw count, not loading, and its pop scan
   decides.
3. **Kaiju takes carry the tremor guard.** The kaiju's camera tremor (about 7 Hz) reads as single-frame
   judder at 30 fps. `sync_kaiju_patch.py` puts a `PROMO_NOSHAKE` guard in the shoot copy of
   `kj/roar/shake.sqs`, and `kaiju_spawn` sets `PROMO_NOSHAKE`. A kaiju take whose PCF log shows a tremor
   gain above 0 at the camera is a failed take.
4. **Popscan and the judder check after every take** (`post_take.py` runs both). A clip with a frame of 40
   or more flagged blocks is looked at frame by frame and explained (an explosion, a house passing the
   lens) or shot again. A picture jerk spike over 1 px (640-wide) that the camera log does not explain is a
   fault.
5. **Gate the framing on thirds from the PCF log, and keep the thirds sheets.** Every clip logs its
   subject's screen position each frame; `gate.py` fails a clip whose subject is off its third line (0.08,
   up to 0.12 for fast or large subjects) or off screen for more than 1 s, or whose level-lens horizon sits
   in the middle band (0.42 to 0.58) for more than 1 s. Only declared symmetric shots (`sym=True`) sit on
   the centre. The sheets (`clips/thirds-sheet-<take>.jpg`, and the cut's `final_thirds_sheet.jpg`) are
   read shot by shot.
6. **Use the engine's lens model.** At 16:9 the tangent of the half view angle is camSetFov x 4/3 across
   and camSetFov x 0.75 up (Hor+, `UI/Settings/AspectRatio.cpp`), not camSetFov across. Aim with the eye
   height read back from the camera after `camCommit 0`, and take a vehicle's height from `getPos` plus the
   ground: `getPosASL` of the @LoBo vehicles reads several metres above their wheels.
7. **Head props from the road, not by eye.** `psRoadHeading` takes the two nearest `nearestRoads`
   segment centres and logs the heading between them. A barrier's long axis is its local x (east at
   `setDir 0`), so `setDir` of the road heading puts the bar across the carriageway. Check it with an
   overhead still in the same run.
8. **True night only.** A night clip is shot on a new-moon date with the sun at least 12 degrees down
   (`NIGHT_DATE` is 16 June 1985, 20:00 on Lebanon80; check other dates and islands with `sun.py`).
   Anything between sunset and 12 degrees down is dusk and is named and cut as dusk. Lighting is
   `triSetLighting "remastered"` and nothing else: no brightness, exposure, camera, moon, light or sky
   knob, no gamma, no fog. Every night shot is lit from inside the story (a lamp, a fire, headlights,
   muzzle flashes within 30 m, the furnace) with the subject in the pool. `nightcheck.py` must read
   `night=1.00` and a `moonShadows` gate that is off for every night clip.
9. **No lift in the grade.** Night shots get gamma 1.000, contrast 1.00, no grain and stay out of the
   exposure match; other shots may only be darkened by it (gamma 0.88 to 1.00); the curve starts at 0/0;
   no normalize, histeq or auto levels. `build.py`'s night luma gate fails the build when a night shot's
   10th percentile luma comes out more than 1 code above its source; `grade.py` checks a changed grade.
10. **Record at 1440p.** Takes record at 2560 x 1440 so a shot can be reframed by up to 1.33x for a 1080p
    delivery without dropping below 1080 source pixels.
11. **One game at a time, on the hidden desktop.** `run-promo.ps1` runs tri with `-j1 --global-slots 1`
    through the hidden-desktop wrapper, so no take shares the machine with another game and no window lands
    on the owner's desktop. Never stop games globally; a leftover is stopped with
    `tools/stop-owned-games.ps1 -DataDir <shadow>`.

## Edit and music

    python audio/separate.py <source.wav> <stems dir>          # stems, 32-bit float at the source rate
    python audio/mkinst.py <stems dir> <instrumental.wav>
    python audio/whisper_scan.py <instrumental.wav> --vad      # must find no words
    python audio/beats.py <instrumental.wav> <stems dir>/drums.wav <first.json>
    python audio/beats_final.py <instrumental.wav> <stems dir>/drums.wav <beats.json>
    python edit/build.py --dry-run                             # every shot fits its clip
    python edit/build.py                                       # or --reuse to re-render changed shots only
    python edit/make_notes.py

Keep the music, the stems and every analysis of them in the production folder. Name the source
generically: its title and any of its words stay out of file names, comments, notes and commits, and
`whisper_scan.py` prints timestamps and word counts only.

## Not committed

Media of any kind (clips, dumps, sheets, stills, audio), the production notes and the generated takes and
kaiju mission (see `.gitignore`). The game-data guard in `.githooks/` blocks media, game data and secrets
at commit and push.
