# Repair 2026-10-11.4 specification (LoBo #13, #14, and the #9 runtime checks)

Status: proposed. Written 2026-10-12 by the spec writer, read-only toward
`D:/Arma_CWA/@LoBo`. Nothing in this document is implemented, applied or
verified yet; the facts in section 2 were checked against the installed bytes.

Inputs: CWR `origin/main` = `a5daeb2` (worktree
`D:/Arma_CWA/.claude/worktrees/lobo-repair-4`, detached, clean), repair tool at
`2026-10-11.3`, rule SHA-256
`75c456dcab6fbece460edd479eedc296253255e709a3c58260f28072562a0427`.
Issues: GearUnclear/lobo #9, #13, #14 (read in full; #13 and #14 have no comments,
#9 has the .3 status comment). Evidence for this spec:
`D:/Arma_CWA/tmp/lobo-repair-4/spec/` (baseline check and doctor output,
prototype outputs and hashes, texture PNGs, helper scripts in `spec/tools/`).

## 1. Scope

In scope for .4:

- #13: one fail-closed rule module `tailgun` for `LoBoIL28\script\tailgun.sqs`.
- #14: one fail-closed rule module `references` for the LoBoMig crash sound, the
  two Hilux models, the EAF pilot model, the LoBo_Pilot `wounds[]` lists and the
  AIM-9 model. `lobof16 backfuse.pac` is recorded as known missing and left
  untouched.
- Small framework extensions the two modules need (section 4).
- Runtime probes and runs for #13, #14 and the open #9 items, plus an official
  Remaster exe smoke test of the rebuilt family package (section 10).

Out of scope (recorded in 6.6, not changed): the broad unresolved-texture sweep,
`data\w_pilot_rucebok_*`, the unreferenced pilot and helmet models, the Syrian
pilot's inherited EAF wounds, IL-28 multiplayer behaviour, and #9 item 6
(preloading `lobo_airammo`).

## 2. Baseline, verified on the installed bytes (2026-10-12)

- `repair_lobo.py --mod D:/Arma_CWA/@LoBo --compatibility ud --check --asset-source <TheHKpack-1.75.pbo>`:
  exit 0, `repair_version` 2026-10-11.3, rule SHA-256 `75c456dc...`, 0 changed,
  0 added, `asset_source_sha256` `79cdb0bb...3ff5` (`spec/baseline-check.json`).
- `PoseidonTools mod doctor D:/Arma_CWA/@LoBo` (report only): exit 0, "Nothing to
  do: no known defect found in 124 pbo(s) (10 entry/entries not readable)"; the
  10 are compressed configs .4 does not touch (`spec/baseline-doctor.txt`).
- `python -B -m unittest discover -s arma_CWR/tests/ci -p "test_lobo_*.py"` in
  the worktree: 241 tests, OK.

Archives .4 will change (none is StuffPBO; none has a SHA-1 trailer; LoBoTer
has a `product` property header):

| Archive | SHA-256 before | Size |
| --- | --- | --- |
| LoBoIL28.pbo | `fb01ec28a0a8e7e8afec6d986ca5b0307a600e853366ce6f452a367135f737ec` | 13,959,245 |
| LoBoMig.pbo | `ab4980a48981f9d650c4a79356e2d3803e86c73e577f0453835d028470dd2465` | 67,538,959 |
| LoBoTer.pbo | `e6be24a8940cce6457e8ba45936d5f285f9832a5dfc8ba18eae71021d80f19a9` | 60,847,986 |
| LoBo_Pilot.pbo | `3e2309b22090cfd841b6086dbc964526c2553a7b98406b2414ec10fe653dcde4` | 21,763,539 |
| LoBo_airammo.pbo | `11afb781b3f8f2e72658ed5b8bbf5b07951349c25b1e74854df22e24d320ecf9` | 8,721,136 |

LoBo_Pilot.pbo (2007) has never been touched by a repair batch. LoBoTer and
LoBo_airammo are `@LoBo_UD` overlay archives in the family package (their
wreck `fire.sqs` differs by profile); .4 changes in them are profile-neutral.

Every reference in #13 and #14 was rechecked with the repair tool's own PBO
reader and PoseidonTools `pbo show` (MLOD face tables parsed per LOD):

| Reference | Where (installed) | Shipped target | Evidence |
| --- | --- | --- | --- |
| `_gunner = _tunits select (count(_tunits) -1)` | `loboil28/script/tailgun.sqs` line 33; guard at line 20 is `? count (crew _plane) ==0: goto "crtGun"` | n/a | member 1,872 bytes, all CRLF, SHA `9b92d408...0151` |
| `\LoBoMig\soundss\jet_crash.wss` | `lobomig/config.cpp` line 254, `soundLandCrash[]` in `LoBo_E_mig21bis_MR1` (the only `soundss` and the only `soundLandCrash` in the file) | `lobomig/sounds/jet_crash.wss` (180,640 bytes); line 253 `soundCrash[]` already names it | config 39,012 bytes SHA `599760d9...4eb7` |
| `LoBoTer\Toy\t\black_matte.pac` | `lobo_terhilux.p3d` LOD 3.0: 176 faces, LOD 5.65: 151; `lobo_terhiluxmg.p3d` LOD 3.0: 178, LOD 5.65: 151; no other LOD | `loboter/toy/t/black matte.pac` (156 bytes, 8x8 DXT1); LOD 0, 10 and 1100 of both models already name `LoBoTer\Toy\t\black matte.pac` (212/20/181 and 212/20/206 faces) | both members stored compressed; decode is stable and equals the header size |
| `LoBo_Pilot\EAF\tex\back.pac`, `legsbk.pac` | `lobo_epilot.p3d` LOD 1.0: back 21, legsbk 34; LOD 5.0: back 14, legsbk 0 | `eaf/tex/back.paa`, `eaf/tex/legsbk.paa`; LOD 0.15 and 1100 already name the `.paa` (91 and 126 faces each) | model 1,231,511 bytes, stored |
| `wounds[]` `.pac` entries | `lobo_pilot/config.cpp` lines 141, 199, 269, 329 (one long line each) | see 6.3: only 5 of the 8 `back`/`legsbk` `.pac` entries lack a file | config 22,148 bytes SHA `db7da9fb...6e28` |
| `rktf5\swtex.paa` | `lobo_aim9.p3d` LOD 1e13 (Geometry) only: 14 of 19 faces (5 have no texture) | none named so; see 6.4 | model 23,113 bytes |
| `LoBoF16\{IAF,EAF,com}\backfuse.pac` | exactly 1 face in LOD 10 of each of the 13 F-16 models (IAF_AA: IAF, EAF_AA: EAF, the other 11: com) | none anywhere | `spec/f16-backfuse.txt` |

Searches for a `backfuse` or `rktf5` file covered all 124 @LoBo pbos, the
Remaster AddOns and DTA, the 2026-10-11 package's @LoBo_Deps, @LoBo_Extras and
@LoBo_UD, the 16 LoBo release archives, the 19 dependency archives and the 26
recovered files (top-level listings): no match.

The .3 runtime logs give the baseline log counts per run (pattern
`GL33: Cannot load texture <name>`, logged once per face load):
`black_matte.pac` 327 (`glow-01`, one Toyota), `eaf\tex\back.pac` 35 and
`legsbk.pac` 34 (`mk20-03`, one pilot), `rktf5\swtex.paa` 56 to 168,
`backfuse.pac` 1 per F-16 variant (`numbers-01`), and for the sound three lines
(`FileCache: file 'lobomig\soundss\jet_crash.wss' not found`, a FileCache
ERRR and `AUDIO Load failed: 'lobomig\soundss\jet_crash.wss'`) when the MiG-21
type loads (`rbk500-01`). None of the shipped targets above was ever logged as
missing.

## 3. Decisions

| Item | Decision |
| --- | --- |
| #13 fix | Issue option 1 (skip gunner creation when the plane has no driver) as the primary rule, plus the option 2 select guard as a second line of defence (6 below explains why both). Bailout lines unchanged and pinned. |
| #14 soundss | Repoint to `\LoBoMig\sounds\jet_crash.wss` (line rule). |
| #14 Hilux | Repoint LOD 3.0 and 5.65 faces to `LoBoTer\Toy\t\black matte.pac`, the name the other three visual LODs of the same models use. Same length, fixed-width field. |
| #14 EAF pilot | Repoint LOD 1.0 and 5.0 faces to the `.paa` names LOD 0.15 and 1100 use. |
| #14 wounds | Repoint the 5 `.pac` wound sources whose file exists only as `.paa`: EAF back, EAF legsbk, RJAF back, RJAF legsbk, IAF\tex2 legsbk. Leave IAF\tex back/legsbk and IAF\tex2 back (their `.pac` files ship). |
| #14 AIM-9 | Repoint the 14 Geometry-LOD faces to `LoBo_airammo\tex\sw.pac`, the texture on all 138 faces of the model's only visual LOD. The issue's candidate (LoBoMig `swtex.paa`) is rejected. Evidence in 6.4. |
| #14 backfuse | Known missing. Leave all 13 models untouched. Evidence in 6.5. |
| Version | `REPAIR_VERSION = '2026-10-11.4'`; new modules run last: `..., 'textures', 'tailgun', 'references'`. |

## 4. Framework changes

All three are backward compatible: existing calls, PLANS and the 241 tests keep
their behaviour.

4.1 `repair/models.py` `_lods(data, trailer=None)`. When `trailer` is given and
`data[pos:] == trailer` exactly (after the last LOD), accept it. Otherwise the
current rule applies unchanged (`_oxygen_path_trailer`, which rejects a NUL-led
path; `test_non_path_32_byte_trailers_are_still_rejected` must still pass).
Why: both Hilux models end with an Oxygen default-path buffer whose path is
empty (first byte NUL, then stale memory), which the current reader rejects
("Unexpected trailing MLOD data"). Pinning the exact 32 bytes per member keeps
the reader fail-closed instead of loosening it for every model.

4.2 `repair/textures.py`:

- `face_textures(data, trailer=None)` and `repoint(data, resolutions, mapping, trailer=None)`
  pass `trailer` to `_lods`.
- In `mapping` values `(new, expected, raw32)`, `expected` may stay an int (the
  same count in every listed LOD, as today) or be a tuple of non-negative ints
  aligned by position with `resolutions`. Reject a tuple of the wrong length, a
  negative or non-int entry, and an all-zero count. A listed LOD whose entry is
  0 must hold neither original nor repaired faces of that mapping (a stray match
  there raises).
- New check: every listed resolution must match exactly one LOD of the model
  (`math.isclose`, `rel_tol=1e-6`), else raise. The installed .3 targets
  satisfy this (their `--check` is a no-op); if an existing synthetic test
  breaks on it, scope the check to tuple counts and say so in the commit.
- Why: the EAF pilot needs (21, 14) and (34, 0) and the Hilux (176, 151) or
  (178, 151); one count per mapping cannot express that, and two calls cannot
  either, because a match in an unlisted LOD raises.

4.3 `repair_lobo.py` `plan()`: optional module attribute `REQUIRED_MEMBERS`
(normalized `addon/member` keys). Collect the union; refuse keys whose addon is
not in `wanted` (`Required members outside the repaired addons`); while reading
each archive, record every entry key; after the loop raise
`Required members missing: [...]` for any absent key. The member bytes are not
read. Why: a repoint must fail closed when a reinstall lacks its target, and no
module can see archive entry tables today.

Also: `REPAIR_MODULES += ('tailgun', 'references')`, `REPAIR_VERSION = '2026-10-11.4'`.

## 5. Shared rule properties

- Both modules use `rules.apply_rules` (text) and the extended `textures.repoint`
  (models), so each accepts only the original, the repaired and a partially
  repaired state, is idempotent, preserves CRLF and every unrelated byte, and
  keeps model length.
- Profile-neutral: neither module takes `compatibility`; everything they write
  is OFP 1.x SQS (`isNull`, `driver`, `count`), a config string literal, or a
  texture name. The classic and UD plans produce identical bytes for these 7
  members, so the package's classic switch leaves them alone.
- `.sqs` results must have zero `script_findings` (enforced by `apply_rules`;
  the prototype result has none).

## 6. Rules

### 6.1 Module `repair/tailgun.py` (LoBo #13)

```python
TAILGUN = "loboil28/script/tailgun.sqs"
GUNNER = "_gunner = _tunits select (count(_tunits) -1)"
BAILOUT = ('? (!(driver _plane in _plane) || !(alive driver _plane)) and _exit == 0 '
           'and (position _plane select 2) > 20: goto "loop2"')
EJECT_TURRET = ('if (alive gunner _gun) then {_gunner = gunner _gun; _gunner action ["EJECT",_gun];'
                '_para = "ParachuteEast" createVehicle [0,0,0];_para setpos getpos _camera;_gunner moveindriver _para;}')
EJECT_PLANE = 'if (alive gunner _plane) then {gunner _plane action ["EJECT",_plane]}'
RULES = {TAILGUN: (
    # TG-1 (issue option 1): no pilot means no group to build the gunner from.
    ("line", '? count (crew _plane) ==0: goto "crtGun"', '? isNull (driver _plane): goto "crtGun"', 1),
    # TG-2 (issue option 2): never select index -1 from an empty group.
    ("sub", GUNNER, '? count _tunits == 0: goto "crtGun"\n' + GUNNER, 1),
)}
POST = {TAILGUN: _bailout_kept}   # BAILOUT, EJECT_TURRET and EJECT_PLANE each exactly once as whole lines
ADDONS = ("loboil28",)
def repair(documents): return apply_rules(documents, RULES, post=POST)
```

(The literal in `BAILOUT` is one line in the file; it is split above only for
width. Line rule anchors are whole LF-normalized lines without trailing
spaces, checked on the installed bytes.)

Why both guards. Option 1 is the safer primary: with no driver it never calls
`group objNull`, `createunit` into a null group (UD silently returns; the
official 3.05 and 1.99 exes were not checked) or `units grpNull`, and it routes
a pilotless crewed IL-28 down the existing empty-plane path, which the .3 runs
(`numbers-01`, two empty IL-28s under `--autotest`) already showed runs without
a script error. Option 2 alone would still call `createunit` with a null group.
Option 1 alone still fails when the driver seat holds a unit with no group
(`group` returns grpNull when the driver has no AIUnit, for example a dead
driver that has left its group), so TG-2 guards the failing statement itself.
TG-2 is unreachable in the normal cases (a living driver's group always contains
at least the driver).

Behaviour after the rule (unchanged unless noted):

| Crew when the script starts (0.1 s after spawn) | Before | After |
| --- | --- | --- |
| empty | unmanned turret, no error | same |
| pilot (any group) | new `LoBo_Egypt_Pilot` in the pilot's group mans the turret | same |
| gunner or cargo, no pilot | "Zero divisor" script error (fatal under `--autotest`) | unmanned turret, no error (as empty) |
| driver seat holds a groupless unit | "Zero divisor" | unmanned turret, no error |

Dead-pilot bailout kept: the main loop line (57) and the two eject lines (64,
65) are untouched and pinned by `POST`. A pilotless IL-28 above 20 m therefore
ejects the plane's gunner (`gunner _plane action ["EJECT", _plane]`), exactly as
it does for a dead pilot; the unmanned turret's eject is skipped
(`alive gunner _gun` is false). Known limitation, unchanged: the script runs once,
so a pilot who boards later never gets a turret gunner (the empty-plane
behaviour).

Expected member: 1,872 bytes `9b92d408...0151` becomes 1,908 bytes
`a2aed3ca23bb70b997e3bfdadb02e4df2547803426e110057a93304dd7bb98a0` (prototype,
`spec/proto/`). Diff: line 20 replaced; one line inserted before line 33.

### 6.2 Module `repair/references.py` (LoBo #14), text rules

```python
MIG = "lobomig/config.cpp"
PILOT = "lobo_pilot/config.cpp"
def _wound(folder, name):   # folder: path parts under LoBo_Pilot
    d = '"' + p("LoBo_Pilot", *folder, name + "_d.paa") + '"'
    return ("sub", '"' + p("LoBo_Pilot", *folder, name + ".pac") + '", ' + d,
                   '"' + p("LoBo_Pilot", *folder, name + ".paa") + '", ' + d, 1)
RULES = {
    MIG: (("line", '\t\tsoundLandCrash[]={"' + p("LoBoMig", "soundss", "jet_crash.wss") + '",0.006000,1};',
                   '\t\tsoundLandCrash[]={"' + p("LoBoMig", "sounds", "jet_crash.wss") + '",0.006000,1};', 1),),
    PILOT: (_wound(("EAF", "tex"), "back"), _wound(("EAF", "tex"), "legsbk"),
            _wound(("IAF", "tex2"), "legsbk"),
            _wound(("RJAF", "tex"), "back"), _wound(("RJAF", "tex"), "legsbk")),
}
```

Each wound anchor is the pair `"<healthy>", "<wounded _d.paa>"` (comma, one
space) so the anchor and the replacement are both unique in the file. Expected:
`lobomig/config.cpp` 39,012 bytes becomes 39,011 bytes
`1ca20ca72027a21fccc47c569184611f8e401781f39a93dd30edff0e7ab24664`;
`lobo_pilot/config.cpp` stays 22,148 bytes, becomes
`47ac0111b99b0f9d088a4fa56b7426293d75cbed8433f925f4422196069f80d6` (5 bytes).

### 6.3 Wound entries, all eight `back`/`legsbk` sources checked

| Entry | File shipped | Model faces use | Action |
| --- | --- | --- | --- |
| EAF\tex back.pac, legsbk.pac | `.paa` only | `.paa` (LOD 0.15, 1100); `.pac` in LOD 1/5 until 6.4 | repoint both |
| RJAF\tex back.pac, legsbk.pac | `.paa` only | `.paa` in every LOD of `lobo_jpilot.p3d` | repoint both |
| IAF\tex2 legsbk.pac | `.paa` only | `.paa` (`lobo_iafpilot_grey.p3d`) | repoint |
| IAF\tex2 back.pac | `.pac` ships | `.pac` | leave |
| IAF\tex back.pac, legsbk.pac | `.pac` ships | `.pac` (`lobo_iafpilot.p3d`) | leave |

Effect worth stating in the changelog: UD's `WoundInfo::Load` finds the healthy
texture by name in the shape, so these five wound pairs never matched a face
before (RJAF and IAF grey not at all, EAF only the broken LOD 1/5 faces). After
.4, wounded EAF, RJAF and 69th Sqn IAF pilots show their `_d` back and
leg-back textures for the first time. This is the authors' intent, not a
balance change. The `.pac` wound entries produced no log lines, so the wound
change is visual only.

### 6.4 Module `references`, model plans

```python
HILUX, HILUX_MG = "loboter/lobo_terhilux.p3d", "loboter/lobo_terhiluxmg.p3d"
EPILOT, AIM9 = "lobo_pilot/lobo_epilot.p3d", "lobo_airammo/lobo_aim9.p3d"
MODEL_MEMBERS = (HILUX, HILUX_MG, EPILOT, AIM9)
OLD_MATTE, NEW_MATTE = _b("LoBoTer", "Toy", "t", "black_matte.pac"), _b("LoBoTer", "Toy", "t", "black matte.pac")
PLANS = {   # member: (resolutions, mapping, pinned trailer or None)
    HILUX: ((3.0, 5.65), {OLD_MATTE: (NEW_MATTE, (176, 151), False)},
            bytes.fromhex("000000000000000000000000e03ea1000d000000f8fb1e0100b5a000b4f3c600")),
    HILUX_MG: ((3.0, 5.65), {OLD_MATTE: (NEW_MATTE, (178, 151), False)},
               bytes.fromhex("000000000000000000000000e03ea1000e000000787e710100b5a000b4f3c600")),
    EPILOT: ((1.0, 5.0), {
        _b("LoBo_Pilot", "EAF", "tex", "back.pac"): (_b("LoBo_Pilot", "EAF", "tex", "back.paa"), (21, 14), False),
        _b("LoBo_Pilot", "EAF", "tex", "legsbk.pac"): (_b("LoBo_Pilot", "EAF", "tex", "legsbk.paa"), (34, 0), False),
    }, None),
    AIM9: ((1e13,), {_b("rktf5", "swtex.paa"): (_b("LoBo_airammo", "tex", "sw.pac"), 14, False)}, None),
}
REQUIRED_MEMBERS = ("lobomig/sounds/jet_crash.wss", "loboter/toy/t/black matte.pac",
                    "lobo_pilot/eaf/tex/back.paa", "lobo_pilot/eaf/tex/legsbk.paa",
                    "lobo_pilot/iaf/tex2/legsbk.paa", "lobo_pilot/rjaf/tex/back.paa",
                    "lobo_pilot/rjaf/tex/legsbk.paa", "lobo_airammo/tex/sw.pac")
ADDONS = ("lobo_airammo", "lobo_pilot", "lobomig", "loboter")
```

`repair_models` iterates PLANS like `textures.repair_models`, wraps errors with
the member key and returns only changed members. `repair(documents)` returns the
text rules from 6.2. Names keep the case the visual LODs use; the engine
lowercases. Every new name is at most 31 characters (29, 27, 29, 23) and is
NUL-filled to 32 bytes, which is also how the authored fields are stored (the
installed old fields are NUL-filled, so exactly one byte per face changes for
the Hilux and the pilot).

Expected models (prototype; `spec/proto/proto-hashes.json`):

| Member | Before SHA-256 | After SHA-256 | Bytes changed |
| --- | --- | --- | --- |
| loboter/lobo_terhilux.p3d (3,141,285) | `731a5d77540ad069678e13d86275c78f3dc52ff62c045c13818bcdd800dc64e0` | `b96ec642ae6040067d1893bcad6e60783c50698eca9c3a9957f8e17a23e6607d` | 327 |
| loboter/lobo_terhiluxmg.p3d (4,252,849) | `6cb798ee256741971cef9b84fbaf672f0b56eeee2826f1b4f4f3f563faf423b2` | `09ec6792486e02f733579752672b3a61303e8c1d1223fe975917ee4334bab116` | 329 |
| lobo_pilot/lobo_epilot.p3d (1,231,511) | `bdb8ebbf99c7f1d16b3df6c63f32642a5ca0ef39d3d30b29bd101c9f546316e5` | `18422e9da1a384b126be0574a8b13ad58a63efc60e5e658c465344cfaed94f71` | 69 |
| lobo_airammo/lobo_aim9.p3d (23,113) | `0b294d9e18111eb549a138cdd313b8c60de20121433bf24648a25491e3e665e9` | `4e7f2e733e882174ebfbbe4b2700e0147c9a79461d84f5a7587d7773f43ee6b5` | 322 |

The AIM-9 result was also produced by the current, unmodified
`textures.repoint` (same hash). The two Hilux members are stored compressed
today and will be written back stored, as the issue anticipated: LoBoTer.pbo
grows by about 4.88 MB (2,001,511 + 2,874,191 bytes).

AIM-9 decision and evidence. The missing name sits only in the Geometry LOD
(resolution 1e13), which the engine never draws; the GL33 texture bank logs it
because shape loading touches every face texture of every LOD. So the rendered
missile cannot change, whichever loadable texture those 14 faces name. The chosen
target is the texture the same model's only visual LOD uses on all 138 faces, so
no new texture is loaded either (cache hit), and it is the pattern .3 already
applied to the MD500 collision, hitpoint and fire LODs (d1). The issue's
candidate is rejected: LoBoMig's three `swtex.paa` copies are byte-identical to
each other (SHA `8283dbe3b6f5659457cc0b67ef78a01a8bf86f21b5159e94ea630282e631feff`,
256x64 DXT1, a grey strip with yellow bands, `spec/tex/lobomig_mig21_syr_1_swtex.png`)
and differ from the AIM-9 skin `sw.pac` (SHA `a9a87e05...c8ad1`, 512x128, seeker,
fins and red and yellow bands, `spec/tex/airammo_tex_sw.png`); nothing links them
to the lost `rktf5` addon, and repointing to them would load a texture the model
never otherwise uses. If the reviewer does not accept the Geometry-LOD argument
as "verified equivalent", drop the AIM9 plan and its required member and record
`rktf5\swtex.paa` as known missing; nothing else in this spec depends on it.

### 6.5 `backfuse.pac`: known missing

One face in LOD 10 (a distant visual LOD) of each F-16 model names
`LoBoF16\IAF\backfuse.pac` (IAF AA), `LoBoF16\EAF\backfuse.pac` (EAF AA) or
`LoBoF16\com\backfuse.pac` (the other 11). No file of that name exists in any
source we hold (section 2), and that face is drawn, so a repoint would be a
guess at the art; no candidate has byte or visual evidence. Leave the models
untouched; record in `D:/LoBo-Reloaded/docs/LOST-CONTENT.txt`, the package
known issues and CONTENT-REPAIRS.md. Runtime keeps logging one line per F-16
variant load.

### 6.6 Seen while checking, not changed by .4

- All nine pilot models name `data\w_pilot_rucebok_l.pac` and `_p.pac` in LOD 1
  and 5 (7 log lines in `mk20-03`); base data ships only
  `w_pilot_rucebok_p_d.pac` and the `w_tank_rucebok_*` pair. No equivalence
  evidence; candidate for a later issue.
- `lobo_epilot2.p3d`, `helmet_lod.p3d` and `iaf_helmet.p3d` name missing
  textures (`EAF\tex\{hose,hset,mask,strap,visor}.paa`, `IAF\tex\glove.paa`,
  `scf_uspi\tex\*`, `IAF\tex\{hose,...}`); no LoBo_Pilot class names these models.
- `LoBo_Syria_Pilot` inherits the EAF `wounds[]` while its model uses
  `SyAF\tex\*.pac`, so Syrian pilots show no back or limb wounds. Separate defect.
- IL-28 in multiplayer (source reading only, unverified): the config init runs
  `tailgun.sqs` on every machine, and `createVehicle`/`createUnit` are global, so
  each machine may create its own turret and gunner; with a remote pilot group,
  `units` can return the pilot as `_gunner`. Suggest a new lobo issue.
- The other `Cannot load texture` lines in the .3 logs (for example
  `loboegypt\inf\helm_s_in.pac`, 76 in `mk20-03`) belong to the broad sweep.

## 7. Expected apply result

- Plan: 7 changed members in 5 archives, 0 added: `loboil28/script/tailgun.sqs`,
  `lobomig/config.cpp`, `lobo_pilot/config.cpp`, `lobo_pilot/lobo_epilot.p3d`,
  `loboter/lobo_terhilux.p3d`, `loboter/lobo_terhiluxmg.p3d`,
  `lobo_airammo/lobo_aim9.p3d`. Every member hash must equal sections 6.1, 6.2
  and 6.4; any difference stops the apply for review.
- Apply with `--asset-source D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/TheHKpack-1.75.pbo`
  so the HK rule stays satisfied (it adds nothing now). One new backup batch under
  `D:/Arma_CWA/@LoBo/.ud-backups/issue-69/`, `repair_version` 2026-10-11.4,
  `state` applied.
- After: `--check` with and without the asset source exits 0 with 0 changes;
  report-only doctor says Nothing to do, still 10 unreadable entries (if the now
  stored Hilux models produce any doctor finding, stop and review); a Classic
  check on a copy lists only the same 16 wreck-fire members in 14 archives as
  after .3; untouched members in the 5 archives byte-identical; the other 119
  archives unchanged.
- Composition: from the pristine originals of all applied batches, .1 to .4
  compose to the installed bytes (update the verifier's expected totals:
  .3 left 160 changed and 7 added members in 49 archives; LoBo_Pilot is new, so
  expect 50 archives; recount members rather than adding, because LoBoIL28,
  LoBoMig, LoBoTer and LoBo_airammo were already changed).
- Configs: parse `lobomig/config.cpp` and `lobo_pilot/config.cpp` with
  PoseidonTools `config tojson` before and after; the parsed difference must be
  exactly `soundLandCrash[0]` and the five wound strings.

## 8. Synthetic tests (offline)

New `tests/ci/test_lobo_tailgun.py`:
- exact change on a CRLF fixture built from the rule anchors plus the pinned
  bailout and eject lines; result has zero `script_findings`; second run returns
  `{}`;
- TG-1 only, TG-2 only and fully repaired states all converge to the same bytes;
- rejected: missing member; guard line absent or duplicated; spacing variant
  `? count (crew _plane) == 0: goto "crtGun"`; select line absent or duplicated;
  bailout line or either eject line changed, missing or duplicated (`POST`);
- no `compatibility` parameter; UD and classic `plan()` give the same bytes.

New `tests/ci/test_lobo_references.py`:
- MiG line: exact, idempotent, CRLF kept; zero or two `soundss` lines rejected;
  an already correct file is a no-op;
- wounds: the five pairs change; the three shipped `.pac` pairs (IAF\tex back,
  legsbk, IAF\tex2 back) are byte-identical; partial state completes; duplicate
  anchor rejected;
- models (synthetic MLODs as in `test_lobo_textures.py`): tuple counts exact per
  LOD; a 0 entry with a stray match raises; wrong tuple length, negative or
  all-zero counts raise; a listed resolution with no LOD raises; pinned trailer
  accepted only byte-exact and preserved; another NUL-led 32-byte tail rejected;
  without `trailer` a NUL-led tail is still rejected; only the 32-byte texture
  fields change and length is kept; fixed point;
- `REQUIRED_MEMBERS`: a synthetic mod (patched `runner.modules`, as
  `test_lobo_archives.py` does) lacking a required member fails `plan()` with
  `Required members missing`; one outside `ADDONS` fails;
- real PLANS well formed: names at most 31 characters, tuple lengths equal the
  resolutions, trailers are 32 bytes, every target appears in `REQUIRED_MEMBERS`,
  `MODEL_MEMBERS` covers PLANS, `ADDONS` as listed.

Extend `test_lobo_models.py` (exact trailer accepted, mismatch rejected, default
unchanged) and `test_lobo_textures.py` (int counts unchanged, tuple counts).
Extend `test_lobo_rule_literals.py` with hand-restated literal tables for both
modules: every anchor, replacement, count, resolution, count tuple, both
trailer hex strings and `REQUIRED_MEMBERS`. Expected total: 241 plus the new
cases; all must pass with `python -B`.

## 9. Docs and tracking (after offline verification; runtime claims only after section 10)

- `arma_CWR/tools/lobo/CONTENT-REPAIRS.md`: new "2026-10-11.4 batch" section
  above .3 (rules, decisions 6.4 and 6.5, results); fix the .3 "Problems the runs
  found" paragraph to point at .4.
- `arma_CWR/tools/lobo/runtime/README.md`: "Repair 2026-10-11.4 probes" section
  and table for `repair4/`.
- `roadmap/roadmap.yaml` item `lobo-content-repairs`: add a log line (status,
  CWR commit). Commit by path only.
- lobo repo: CHANGELOG entry using the README status words; comments on #13,
  #14 and #9; close an issue only when its runtime items are observed.
- Library: `docs/known-issues.txt`: drop the IL-28 line once R1 passes; replace
  the "A few LoBo files are missing" line with one about the F-16 `backfuse.pac`
  face only; change the official-exe line according to R5. Add backfuse and
  `rktf5\swtex.paa` (if 6.4 is dropped) to `LOST-CONTENT.txt`.

## 10. Runtime plan

Common rules for every run:

- Engine: UD `D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonGame.exe`; record
  its SHA-256. Trident: `D:/Arma_CWA/tmp/lobo-repair-3/runtime/trident-build/debug/tri.exe`
  through `arma_CWR/tools/lobo/runtime/run-local.ps1` (it closes its own game
  children in `finally`), `-Retries 2`, `-j 1` (the wrapper's default), one run
  at a time, a fresh `-OutputDir` under `D:/Arma_CWA/tmp/lobo-repair-4/runtime/<run>`.
- Data: `-DataDir 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered'`
  read only, as .3 did; record a recursive file list with sizes and mtimes of
  that folder before and after (as `remastered-tree-before.txt`) and require it
  unchanged. Never run setup or write there; never mount its own `@LoBo`.
- Mod: TOML `extra_args` `--dev --no-strict --mods-dir D:/Arma_CWA --mod @LoBo`
  (the installed, .4-applied copy). `repair_lobo.py --check` must exit 0 with 0
  changes and `repair_version` 2026-10-11.4 before and after the runs.
- New probe sources in `arma_CWR/tools/lobo/runtime/repair4/`; new mission
  `arma_CWR/tools/lobo/runtime/Repair4.Abel`: Repair3.Abel's `addOns[]` and WEST
  player, plus one EAST group whose leader is a `LoBo_Egypt_Pilot` named `r4East`
  placed far away (about [7000, 0, 9000]) to host AI crew (OFP has no
  `createGroup`, and AI put in the player's group may leave the seat to follow
  him). Tags `scripting, full_cwa, lobo, headful, exclusive`, not in a default lane.
- Every TOML runs with `--autotest` (Trident's default for these probes): a LoBo
  script error aborts the run, which counts as a failure.
- Log patterns below are case-insensitive substrings of the game stdout log.
- Close every game you start; never stop a process you did not start (other
  sessions share the machine); check for strays from your own runner only.

R0, baseline (no new run needed): the .3 logs in section 2 are the "before"
counts. If the probes are ready before the apply, running R2 once on the pre-.4
state adds before screenshots; optional.

R1, #13 IL-28 (`repair4/lobo_il28_tailgun.test.sqf`, mission Repair4.Abel):
- G, ground, player gunner, no pilot: `tgPlane = "LoBo_IL28_egy" createVehicle [8000, 9620, 0]`
  and `player moveInGunner tgPlane` in the same statement batch (the init waits
  0.1 s, so the script sees crew without a driver). Wait 5 s. Pass: no abort;
  `vehicle player == tgPlane`; a `LoBo_rearturret` exists near the plane with
  `isNull gunner` true; altitude below 20 m.
- A, above 20 m, player gunner, no pilot (player `allowDammage false`): same,
  then `setPos` the plane to 400 m in the same batch. Pass within 5 s: no abort;
  `vehicle player != tgPlane` (the authored bailout ejected the gunner); turret
  exists, unmanned.
- C1 control, empty IL-28: turret exists, unmanned, no abort.
- C2 control, pilot first: `"LoBo_Egypt_Pilot" createUnit [pos, group r4East, "tgPilot = this", 0.6, "PRIVATE"]`,
  `tgPilot assignAsDriver tgPlane; tgPilot moveInDriver tgPlane` in the spawn
  batch. Pass after 3 s: turret gunner alive, `typeOf` `LoBo_Egypt_Pilot`,
  `group` equals `group tgPilot`.
- C3 control, dead-pilot bailout kept: C2 at 400 m, then `tgPilot setDammage 1`.
  Pass within 5 s: the turret gunner left the turret and sits in a
  `ParachuteEast`.
- Log: zero `Zero divisor`, zero `Script error` lines naming `tailgun.sqs`.
  Delete each plane and turret between cases. Close-up screenshot of case G.

R2, #14 references (`repair4/lobo_missing_refs.test.sqf`, TOML adds `--log-level debug`):
- Spawn `LoBo_Ter_Toy_R`, `LoBo_Ter_ToyMG_R`, `LoBo_Egypt_Pilot`,
  `LoBo_Jordan_Pilot`, `LoBo_IAF_Pilot_grey`, `LoBo_E_mig21bis_MR1`,
  `Lobo_F16_IAF_AA`, `Lobo_F16_EAF_AA`; assert each `typeOf`.
- Screenshots: each Hilux from the side at about 8, 35, 70 and 120 m (to reach
  LOD 3.0 and 5.65; record `triGetObjectLodBias`); the EAF pilot from behind at 5
  and 40 m; the F-16 IAF AA rails close up (AIM-9s); optionally the three pilots
  from behind after `setDammage 0.6` (wound textures).
- Log pass: zero lines containing `black_matte.pac`, `eaf\tex\back.pac`,
  `eaf\tex\legsbk.pac`, `rktf5\swtex.paa`, `soundss`; zero `Cannot load texture`
  or `Load failed` lines for `black matte.pac`, `eaf\tex\back.paa`,
  `eaf\tex\legsbk.paa`, `lobo_airammo\tex\sw.pac`, `lobomig\sounds\jet_crash.wss`.
  Expected and recorded, not a failure: one `backfuse.pac` line per F-16 variant.
- Visual pass (human review of the sheets): Hilux trim and bumpers black at every
  distance with no placeholder texture; EAF pilot back and leg-back textured at
  both distances; AIM-9s look as in the .3 `numbers-01` captures.
- Not covered: the MiG-21 landing-crash sound itself (only its load is checked).

R3, #9 remaining items (`repair4/lobo_wreck_burn.test.sqf`, mission Repair3.Abel
or Repair4.Abel, TOML `--log-level debug`, `timeout` 420, wrapper
`-TimeoutSeconds 600`). Player `setCaptive true`, damage allowed (do not use
`allowDammage false`, the burn uses `setdammage`).
- S, isolated glow (before the wreck): `KioGlowFire` on open ground with
  `inflame true`, and a vanilla `Fire` 4 m away as a control. Screenshots at 4 m,
  low angle, at noon and again after `setDate` to 23:00. Pass (review): the
  vanilla control shows its campfire mesh, the glow shows none; the glow shows the
  vanilla Fire light (its `Light` class draws a small `koulesvetlo` light sprite,
  size 0.3, which is expected) and smoke.
- D1, action on the isolated glow: player on foot 3.5 m away, facing it.
  `triActionMenuText` contains "Put out fire"; `player action ["FIRE PUT DOWN", glow]`;
  after a few frames `inflamed glow` is false and the menu shows "Light fire".
  Delete both, keep night for the wreck phase.
- Wreck: empty `LoBo_Mi8_egy` at [8000, 9820, 0], player 60 m away,
  `setDammage 1`; `t0` = mission `time` when one `KioGlowFire` exists within 30 m.
- A (t0 to t0+62 s): stay away. This passes both random-explosion windows of the
  script (mortar shells while `_lifeTick` > 550, the first 12.5 s; bullets while >
  360, the first 60 s). Night screenshot at about 12 m showing the light.
- B, seated (t0+62 to t0+90): vanilla `Jeep` about 7 m to the side of the wreck,
  `player moveInDriver`. Pass: `player distance wreck` below 10 throughout;
  `damage player` and `damage jeep` unchanged over at least 25 s (two burn windows).
- C, on foot (t0+90 to t0+125): delete the jeep, player on foot at the same spot.
  Sample `damage player` with mission time every few frames. Pass: every step is
  +0.04 (within 0.001); the number of steps N satisfies N <= floor(L/10) + 2 for
  the on-foot time L; from the second step on, steps are at least 9.5 s apart
  (the reset runs on the script's own 40-tick clock, so the first two steps can
  be closer); N >= 2. Screenshot within 1 s of the first step (the
  "MEDIC! I'm getting burnt here!" line must be visible, review).
- MEDIC count: chat is logged only in `--simulate` mode, so count the scream's
  own sound instead: the `#scream` block runs `player say "eng31"` and then the
  groupchat, once per burn. Pass: the number of
  `SoundScene::OpenAndPlayOnce:` lines containing `eng_lookout3` between t0 and
  the end of D2 equals the total number of damage steps seen in C and D2.
- D2, action at the wreck (t0+125 to t0+135): player 3 to 4 m from the glow,
  facing it. Record whether "Put out fire" is offered; put it out with the action
  (fall back to `glow inflame false` if the hull blocks the position and say so).
  Pass: `inflamed` false, no script error, and the script's fire particles and
  `say` lines continue (log lines keep coming).
- E, cleanup: move the player 60 m away; wait for zero `KioGlowFire` within 30 m.
  Pass: deletion at 150 s to 200 s of mission time after t0 (600 ticks of at
  least 0.25 s plus frame overshoot); the wreck still exists.
- Sound: between t0 and the deletion, at least 100 `SoundScene::OpenAndPlayOnce:`
  lines containing `objects\fire` (the `say "fire"` every 4 ticks resolves
  `CfgSounds >> Fire` to `Objects\fire`, shipped as `sound\objects\fire.wss`) and
  zero `Load failed` lines for it. Note for the record: vanilla Fire's volume is
  0.000251, so it is barely audible; that is stock behaviour.
- "Put out fire" acceptability, for the user to decide from D1/D2: recommend
  accepting it as a documented side effect if putting it out only stops the light,
  smoke and crackle, the script's particles, burn and `say` continue, the 150 s
  cleanup still deletes the glow, and nothing errors. If not acceptable, a later
  batch needs a non-Fireplace glow (no config key hides the action; UD's
  `Fireplace::GetActions` offers it to any free soldier within 5 m facing it).

R4, package build (after R1 to R3 or in parallel once the apply is final):
`package-lobo-family.ps1` with `-SkipFetch -CacheDir D:/LoBo-Reloaded/sources/mega_nc-cache -ExtrasDir <@LoBo_Extras build> -LibraryDir D:/LoBo-Reloaded -KnownIssuesFile D:/LoBo-Reloaded/docs/known-issues.txt`
(updated per section 9) and a new version name (for example 2026-10-12). No
builder code change is needed: the builder rereads `repair_version` and the
rule SHA, the classic switch does not touch the .4 members, and LoBoTer and
LoBo_airammo still differ from their classic twins. Every builder gate must pass.

R5, official Remaster exe smoke test of the built package:
1. Fresh shadow: `pwsh -File arma_CWR/tools/runtime/new-remaster-shadow.ps1 -Source 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered' -Destination D:/Arma_CWA/tmp/lobo-repair-4/runtime/official-shadow`
   without `-LoBoSource` and without `setup-guerrilla.ps1` (the official exe must
   see stock BIN). Check that the shadow's `PoseidonGame.exe` is a real file with
   the same SHA-256 as the Steam exe (the official 3.05 build) and that BIN,
   fonts and Missions are not reparse points.
2. Copy (`Copy-Item -Recurse`; no junction or hard link) `@LoBo`, `@LoBo_Deps`
   and `@LoBo_Extras` from `D:/LoBo-Reloaded/modfolders/<version>/` into the
   shadow root and check them against the package `SHA256SUMS.txt`. Never copy
   `@LoBo_UD` (it must not be mounted on the official exe).
3. S1, menu boot: start the shadow exe detached with
   `--mod "@LoBo_Extras;@LoBo;@LoBo_Deps" --log-file <run>\official-s1.log --window --width 1280 --height 720 --timeout 90`
   and `POSEIDON_USER_DIR`, `POSEIDON_CACHE_DIR`, `POSEIDON_TEMP_DIR` set to a
   folder under the run dir (the official exe honours them, so the user's
   Documents profile stays untouched). Keep the PID; if it outlives 150 s, stop
   that PID only. Pass: it reaches the menu and exits by timeout, stderr has no
   crash or minidump line, the log shows the three mods mounted and no config
   load error.
4. S2, in mission through Trident on the official exe:
   `tri test --game-dir <shadow> --data-dir <shadow> --retries 1 -j 1 <toml>` (via
   run-local.ps1 `-GameDir` and `-DataDir` set to the shadow) with
   `repair4/lobo_official_smoke.test.sqf`, `binary = "PoseidonGame"`,
   `extra_args = ["--dev", "--no-strict", "--mod", "@LoBo_Extras;@LoBo;@LoBo_Deps", "--width", "1280", "--height", "720"]`
   (no `--mods-dir`; the folders sit next to the exe), mission Repair4.Abel.
   Content: case G of R1 (IL-28, player gunner, no pilot, ground), the R2 spawns
   and screenshots, and a `LoBo_Ter_Toy_R` killed with `setDammage 1` that must
   create exactly one `M1Smoker` (the classic-profile ground script). Pass: no
   abort, the asserts hold, the R2 log criteria hold, and the session closes.
   The official exe ships `--harness`, `--test-mission`, `--autotest`, `--mod`,
   `--log-file`, `triActionMenuText` and the other probe commands; if Trident
   still cannot drive it, fall back to copying the probe as a plain mission whose
   `init.sqs` spawns the same objects and logs markers, run with
   `--test-mission Missions\<name> --autotest --timeout 180`, and grep the log.
5. After: re-list the Steam folder (unchanged), delete nothing in Steam, close
   everything you started, keep the shadow and logs as evidence.

R6 (optional): the same S2 on the UD engine with all four package modfolders
(`@LoBo_UD` last) from the shadow, to cover the UD overlay as shipped.

Claims allowed afterwards: "runtime verified" only for what a run observed:
#13 by R1 (UD) and S2 case G (official exe); #14 by R2 logs plus the reviewed
screenshots; each #9 item by its own R3 step. The official-exe line in the known
issues may change only after S1 and S2 pass. Anything not run stays "offline
verified" or "applied".

## 11. Stop conditions

Stop and report instead of improvising when: any member hash after the plan
differs from 6.1, 6.2 or 6.4; the installed baseline differs from section 2;
`--check` after apply is not a no-op; doctor reports anything new; a test other
than the new ones changes result; R1 shows the bailout behaving differently from
6.1; R3 shows a seated burn, a burn step other than 0.04, or more than one
eng_lookout3 line per step; the official exe fails S1 (then the package must not
be reshared before review).

## 12. Commit plan

Commit by path from the worktree, rebase on `origin/main`, then
`git push origin HEAD:main`; no branch. Commits (each message ends with
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`):
1. `lobo: repair 2026-10-11.4 fixes lobo #13 and #14` (repair_lobo.py,
   repair/models.py, repair/textures.py, repair/tailgun.py,
   repair/references.py, the new and extended tests, CONTENT-REPAIRS.md).
2. `lobo: runtime probes for repair 2026-10-11.4` (runtime/repair4/,
   runtime/Repair4.Abel, runtime/README.md).
3. Roadmap log line. Never commit pbo, paa, p3d, wss, extracted members,
   screenshots or logs.
