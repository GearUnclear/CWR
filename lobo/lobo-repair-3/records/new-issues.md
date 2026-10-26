# New issue drafts, found by the repair 2026-10-11.3 runtime probes

Drafts only, not posted. Suggested labels: `bug`, `lobo`.

---

## Title

IL-28 tail gun script stops with "Zero divisor" when the plane has crew but no pilot

## Body

The IL-28 tail gun script builds its gunner from the pilot's group. When the plane has someone aboard but no pilot at the moment the script runs, the script stops on a script error and the tail gun setup never finishes.

**Where**
- `LoBoIL28\script\tailgun.sqs`, started by `script\common_init.sqs:10` 0.1 s after spawn. `common_init.sqs` runs from the init handlers of four IL-28 classes (`LoBoIL28\config.cpp:295`, `:405`, `:458`, `:510`).
- The script skips gunner creation only when `count (crew _plane) == 0`. With crew but no driver, `_pilot = driver _plane` is objNull and its group is null, so `createunit` adds nobody and `_tunits = units _pilot_grp` is empty. The next line, `_gunner = _tunits select (count(_tunits) -1)`, then selects index -1.

**Consequence**
- The engine reports `Script error at '_gunner = _tunits select (count(_tunits) -1)|#|': Zero divisor`. Under `--autotest` that aborts the game (run `mk20-02`, `game_stdout.log` line 450). What the script does after the error in normal play (whether the turret is still created and manned) was not checked.
- The probe only hit this because it seated the player as gunner with no pilot. Whether a normal mission reaches it is unverified: an editor-placed IL-28 with a gunner but no pilot, or a script that seats the gunner before the pilot within the first 0.1 s, would.
- An empty IL-28 spawns without errors, and with an AI pilot seated first the final probe (`mk20-03`) ran clean.
- Context for a fix: by design the main loop ejects the turret gunner and the plane's gunner once the plane is above 20 m without a living pilot (a dead-pilot bailout). A pilotless IL-28 that survives the error would take that path too.

**Fix options**
- Skip gunner creation when there is no pilot, the way the empty-plane path already does: replace `? count (crew _plane) ==0: goto "crtGun"` with `? isNull (driver _plane): goto "crtGun"`.
- Or guard the select only: add `? count _tunits == 0: goto "crtGun"` before the `_gunner = ...` line.
- Either is plain OFP 1.x SQS, the same under the ud and classic profiles, and needs a new fail-closed rule in `repair_lobo.py` (the member is repacked, as in .3).
- Add a runtime case: IL-28 with a gunner and no pilot, on the ground and above 20 m.

Found during the 2026-10-11 runtime probes for repair 2026-10-11.3 (UD engine, official Remaster data, installed @LoBo). Not caused by .3: no repair batch touches `tailgun.sqs` or `common_init.sqs`.

---

## Title

Five LoBo file references point at files the mod does not ship (MiG crash sound, Hilux, EAF pilot, AIM-9 and F-16 textures)

## Body

The 2026-10-11 runtime logs report five LoBo file references the engine could not load. A read-only look at the installed pbos shows that three are spelling or extension slips next to a file the mod does ship, and two have no shipped file at all.

**Where**

| Reference as logged | Named in | What the mod ships | Logged in run |
|---|---|---|---|
| `lobomig\soundss\jet_crash.wss` | `LoBoMig\config.cpp:254` (`soundLandCrash[]` in class `LoBo_E_mig21bis_MR1`) | `LoBoMig\sounds\jet_crash.wss` (`sounds`, one `s`) | `rbk500-01` |
| `loboter\toy\t\black_matte.pac` | `LoBoTer\lobo_terhilux.p3d` and `lobo_terhiluxmg.p3d` (MLOD, stored compressed; about 330 faces each) | `LoBoTer\toy\t\black matte.pac` (a space, not an underscore) | `glow-01`, `glow-unlisted-01` (654 warnings) |
| `lobo_pilot\eaf\tex\back.pac` (and `legsbk.pac`) | `LoBo_Pilot\lobo_epilot.p3d` (MLOD, 69 faces) and the EAF `wounds[]` list in `LoBo_Pilot\config.cpp` | `eaf\tex\back.paa` and `eaf\tex\legsbk.paa` (`.paa`, not `.pac`) | `mk20-03` |
| `rktf5\swtex.paa` | `LoBo_airammo\lobo_aim9.p3d` (MLOD, 14 faces) | Nothing: no archive has the `rktf5` prefix. LoBoMig ships files of the same name under `mig21\syr\1\`, `egy21\1\` and `egy21\3\` | `mk20-01` to `mk20-03`, `numbers-01` (224 warnings) |
| `lobof16\iaf\backfuse.pac` (and `eaf\backfuse.pac`) | the 13 F-16 models in `LoBoF16` (MLOD), which also name `com\backfuse.pac` | Nothing: LoBoF16 has no `backfuse` file | `numbers-01` |

The same `.pac` versus `.paa` slip appears in `LoBo_Pilot\config.cpp` `wounds[]` for `RJAF\tex\back`, `RJAF\tex\legsbk` and `IAF\tex2\legsbk`; those were not logged in these runs.

**Consequence**
- `LoBo_E_mig21bis_MR1` and the classes that inherit its value: no crash sound on a landing crash (`soundLandCrash`).
- Hilux, EAF pilot, AIM-9 and F-16: the engine logs a load failure and draws those faces without their intended texture.
- Log noise: hundreds of `Cannot load texture` lines per run for the Hilux and the AIM-9 alone.

**Fix options**
- `soundss` to `sounds` in the LoBoMig config line (one byte shorter; the repair tool repacks the member).
- Hilux and EAF pilot: same-length repoints of the fixed-width MLOD face texture names (`black_matte.pac` to `black matte.pac`, `.pac` to `.paa`), the method the .3 `textures` module already uses. The two Hilux models are stored compressed and would be written back uncompressed. Also repoint the matching `wounds[]` entries, or every `.pac` entry whose file only exists as `.paa`.
- `rktf5\swtex.paa` and `backfuse.pac`: no shipped file to point at. Either repoint to a shipped texture after a visual check (for example one of LoBoMig's `swtex.paa` copies for the AIM-9, if it is the same art), recover the files from the original source addons, or record them as known missing.
- Each change needs a new fail-closed rule with tests, and a visual check in game afterwards.

Found during the 2026-10-11 runtime probes for repair 2026-10-11.3 (UD engine, official Remaster data, installed @LoBo). Not caused by .3: every reference above is as the authors shipped it (the `soundss` line is already in the pre-repair LoBoMig backup), and no repair batch touches these lines or model files. The same logs carry other `Cannot load texture` lines; those overlap the broad unresolved-texture sweep that the .3 spec left out of scope and are not triaged here.
