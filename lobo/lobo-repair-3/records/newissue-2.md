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
