# LoBo repair 2026-10-11.3: implementation spec

Status: ready for implementation. Written 2026-10-11 by the spec writer from the
seven investigations (issue-2-frame-loops, issue-3-mines, issues-4-5-cluster,
issue-8-numbers, issue-9-smokers, textures-icons, config-identifiers,
audit-gaps) plus the orchestrator decisions. Every rule below was re-run
read-only against the installed `D:/Arma_CWA/@LoBo` (state 2026-10-11.2) by a
prototype harness; nothing in @LoBo, the worktree or the Remaster tree was
written.

Prototype and evidence (local only, contains extracted APL-SA bytes under
`dump/`, never commit): `D:/Arma_CWA/tmp/lobo-repair-3/spec-proto/`

- `proto/*.py`: working prototypes of every new module (`loops`, `mines`,
  `clusters`, `numbers`, `effects`, `identifiers`, `content`, `weapons`,
  `textures`), the shared helpers (`common.py`) and the two Archive extensions
  (`archive_ext.py`). They import the shipped `repair.*` package and are the
  byte-exact reference for this spec; port them, do not reinvent them.
- `run.py`: composes `ammo, scripts, models, followups` + the nine new modules
  exactly like `repair_lobo.plan()`, checks the fixed point, stages every
  archive in memory. Options: `ud|classic`, `--asset-source PATH`,
  `--pristine` (reinstall scenario from the .1/.2 backups), `--dump DIR`.
- `report-ud-asset.json`, `report-ud-noasset.json`, `report-classic-noasset.json`,
  `report-ud-pristine.json`: member and archive hashes.
- `diffs.txt`: unified diff of every changed text member. `cfgdiff.py`: engine
  parser (PoseidonTools `config tojson`) diff of every changed config.
- `baseline.py`, `mlodinfo.py`, `texcheck.py`, `hkcompare.py`: anchor counts,
  MLOD texture census, HK icon pins.

Run any of them with `python -B` (no `__pycache__`).

## 0. Summary

Repair 2026-10-11.3 adds nine fail-closed rule modules after `followups`, two
Archive capabilities (member addition from a hash-pinned external source;
same-length in-place replacement for StuffPBO archives) and one relaxation in
the MLOD reader. Verified prototype result on the installed mod:

| Profile | Asset source | Archives | Members changed | Members added | Skipped |
| --- | --- | ---: | ---: | ---: | --- |
| ud | `TheHKpack.pbo` 1.75 | 41 | 84 | 7 | none |
| ud | none | 41 | 84 | 0 | 1 report line (7 icons) |
| classic | none | 45 | 100 | 0 | the same 84 plus exactly the 16 wreck-fire queries |

- Fixed point holds for all 13 modules in both profiles, and every new rule
  produces identical bytes under `ud` and `classic` (only the 16 known
  `fire.sqs` queries differ, as today).
- Reinstall scenario (`--pristine`: originals from the .1/.2 backups): .1 + .2
  + .3 compose to 155 changed members in 49 archives, and every member the .3
  rules touch ends byte-identical to the installed-path result.
- All 35 changed `config.cpp` members parse with the engine parser before and
  after; the parsed diff contains only the intended keys
  (`spec-proto/cfgdiff.txt`; the four empty wreck-glow subclasses do not show in
  the flattened diff because they have no own keys).
- Every staged archive keeps all untouched members byte-identical; SHA-1
  trailers recomputed (BDE_mir3, LoboF5), textual trailers kept (JOF_Objects1),
  the StuffPBO trailer kept verbatim (bwm_boot3), compressed members rewritten
  uncompressed (CSJairlines x2, CSJcitation, CSJils, CoC_Torpedoes,
  RKSL-Hangers, TMYK_Bridges).

Lobo issues closed by this batch: #2, #3, #4, #5, #8, #9 (#9 pending the
runtime confirmation required by orchestrator decision 6). The remaining items
come from the textures-icons, config-identifiers and audit-gaps passes and
have no lobo issue number.

## 1. Ground rules for the implementer

- Source of truth: `D:/Arma_CWA/.claude/worktrees/lobo-repair-3` (detached at
  origin/main). No branch, no commit, no push. Prose (docs, comments) without
  em dashes.
- Write into `D:/Arma_CWA/@LoBo` only through
  `repair_lobo.py --apply`. Evidence goes to `D:/Arma_CWA/tmp/lobo-repair-3/<label>/`.
- No LoBo, HK pack or base-game bytes in the repository: rules contain anchors
  and hashes only, tests use synthetic fixtures (existing practice).
- Every rule accepts the original, the repaired and a partially repaired state,
  rejects anything else, and is idempotent. Unknown or duplicate anchors,
  missing members and unexpected static findings raise `ValueError` before any
  archive is written (existing transaction model).
- All rules are profile-neutral: they take no `compatibility` option, use only
  OFP 1.x SQS commands (`~`, `?:`, `goto`, `exit`, `alive`, `not`, `isNull`,
  `local`, `deleteVehicle`, `isEngineOn`, `&&`) and plain config literals, and
  every referenced vanilla asset exists in both Classic 1.99 and Remaster
  (`para.p3d`, `empty.p3d` in Data3D.pbo; `00000mc_tankistae2.paa`,
  `00002mc_tankistae2.paa` in Data.pbo; `Weapons\bulletricochet4.wss`,
  `bulletricochet5.wss` in Sound.pbo; vanilla `CfgVehicles >> Fire` with an
  identical body in both CONFIG.BIN).
- Run Python with `-B` or `PYTHONDONTWRITEBYTECODE=1`; another session also runs
  Python in this worktree, so do not delete `repair/__pycache__` it creates.

## 2. Conflicts between investigators and how they were resolved

All re-checked against installed bytes.

1. **Newlines.** Every member touched by .3 is CRLF (audit-gaps reported LF for
   the LoBoArt/CoC_Arty start scripts, helicopter fire scripts and RKSL/TMYK
   configs; the installed bytes are CRLF throughout). Rules work on
   LF-normalized text and restore the member's own newline, as `scripts.py`
   does, so this does not change any rule.
2. **KioGlowFire (helicopter wreck glow).** issue-9 proposes defining the class
   next to the Smokers; audit-gaps (AG-2) proposes repointing three scripts to
   `LoBo_60fire` (LoboIDFhelo). Chosen: define
   `class KioGlowFire: LoBo_WreckGlow {};` in the same LoBo_airammo block.
   Reasons: one mechanism for both wreck-fire families, one runtime check, zero
   script edits, and it covers all five copies (LoBoEghelo `Engine`,
   `Engine2`, `Engine3`; LoboIDFhelo `CH53\Engine`, `UH1\Engine`) regardless of
   which are reachable. `LoBo_60fire` would make Egyptian and Jordanian
   helicopter wrecks depend on the LoBo_IDF_Helo addon instead, with the same
   activation caveat. Consequence to verify at runtime: the BAS script's
   authored player burn (+0.04 damage every 0.25 s within 10 m of the glow, no
   once-only guard, unlike the ground scripts) becomes live; fallback if judged
   too harsh: drop only the `KioGlowFire` line.
3. **JOF_Objects1 mines.** issue-3 (0.1 s / 0.03 s with isNull guard and helper
   cleanup) wins over AG-9 (0.25 s): 0.25 s breaks the flare tripwire and gives
   about 5 m detonation lag for AT mines at 70 km/h. `JOF_Claymore` is live
   (`JOF_Objects1/config.cpp:301` init EH) and is included, which AG-9 missed.
4. **AG-3 evidence correction.** `camdestroy _flag` in the four `setflags.sqs`
   is a no-op on a `FlagCarrier` (CamDestroy only accepts CameraHolder,
   `GameStateExtUi.cpp:1562`), so the flag lives and the `#repeat` loop does
   matter: it releases flag ownership when the driver leaves. The fix (wait 1 s
   instead of a frame) is still correct; release is up to 1 s later.
5. **AG-7 = CI-2 (CSJ `type=VAir`) and AG-8 = CI-7 (CoC `canLock=LockYes`)**:
   implemented once, in `identifiers`.
6. **HK icon source.** The textures-icons investigator hashed the HK pack from
   `OFPHKpack.zip` (archive sha256 `99a020e2...`). As stored members it carries
   none of the seven pinned byte streams (0 of 7 match). The orchestrator's
   1.75 archive (`79cdb0bb...`, both copies under `D:/LoBo-Reloaded/_work/inspect/hk/`)
   carries all seven, uncompressed. Pin 1.75 only.
7. **`\LoBoWeapons\Tex\w_mp5k.paa`.** Not byte-identical to either HK MP5K icon
   (30154 B vs 31391 B `w_mp5k`, 26262 B `w_mp5kpure`); the investigator only
   claimed visual similarity. Not used. The HK icons are added at the paths the
   config already names, so the MP5/UMP/PSG1 lines need no config edit.
8. **MD500D_R MLOD trailer.** `LoBo_MD500D_R.p3d` ends with a 32-byte Oxygen
   path trailer `H:\Codemast` + NUL + 20 bytes, not `data\` + NUL; `models._lods()` rejects
   it today ("Unexpected trailing MLOD data"). The texture rule needs the
   reader relaxation in 3.3. Not noticed by the investigator.
9. **Pre-existing correct lines.** Exact-line rules need a baseline where the
   replacement line already exists in another class: LoboIDFhelo `\t\ttype=2;`
   x2, LoBoJorHelo `\t\ttype=2;` x2, LoBo_FAB `\t\ttype=1;` x4 and
   `\t\thasCommander=0` x2, LoBoWeapons parent model line x1 (section 4.0).
10. **AG-1 static finding.** Both `m252start.sqs` copies carry a harmless
    duplicate label (`#precisionset` and `#PrecisionSet`); the rule asserts
    exactly that one pre-existing finding instead of failing.
11. **Interplay with the concurrent `@LoBo_Extras` work.** Another session has
    uncommitted changes in this same worktree (`package-lobo-family.ps1`
    modified; new `EXTRAS.md`, `build-lobo-extras.py`, `lobo_cfgpatches.py`,
    `tests/ci/test_lobo_extras.py`). Do not touch, revert or commit them. Its
    `ExpectedClassicSwitch` list is unchanged, so section 5 still holds. The
    optional `@LoBo_Extras` modfolder (mounted first) ships TheHKpack.pbo, BAS
    Littlebirds (`bas_lbs.pbo`), BWMod Basic Pack 1.56, HYK, FAMAS and the Iraqi
    Shilka. Checked against those packs (raw scan of every config/script member,
    binarized included): none defines `M1Smoker`, `M60Smoker`, `Smoker`,
    `KioGlowFire` or `LoBo_WreckGlow`, so the effects block cannot collide.
    BWMod defines `class BWMOD_Soldier: SoldierWB` (`infantry_nato.hpp:37`):
    with Extras mounted the authored Boot3 crew would resolve. CI-6 still
    repoints Boot3 to `SoldierWB`, the base class of `BWMOD_Soldier`, because
    the core @LoBo must not exit or crash without the optional pack; with
    Extras the Boot3 crew becomes the generic West soldier (cosmetic). The HK
    icons go into LoBoWeapons, so the core no longer depends on Extras for
    them; the MD500D_R repoint makes that model self-contained whether or not
    `bas_lbs.pbo` is mounted.

## 3. Framework changes

### 3.1 `repair_lobo.py`

- `REPAIR_MODULES = ('ammo','scripts','models','followups','loops','mines','clusters','numbers','effects','identifiers','content','weapons','textures')`
- `REPAIR_VERSION = '2026-10-11.3'`.
- New option `--asset-source PATH` (type Path, optional). Forwarded to plan().
- `plan(mod, tool, compatibility='ud', asset_source=None)`:
  - `added_keys = union of module.ADDED_MEMBERS` (attribute optional, default
    empty). While reading each wanted archive, read the bytes of any
    `stem/key` in `added_keys` that already exists (`existing_added`).
  - After the existing text/model passes, for each module with `add_members`:
    `additions, skipped = module.add_members(existing_for_module, asset_source)`.
    Validate: additions keys are a subset of the module's `ADDED_MEMBERS`, none
    already exists. Fixed point: calling again with
    `existing + additions` must return `({}, [])`; when nothing was added the
    second call must return the same skip lines.
  - `changed[key] = bytes` for additions too; record
    `{'member', 'added': True, 'stored_name', 'before_sha256': None, 'before_bytes': 0, 'after_sha256', 'after_bytes'}`.
  - Return a 6-tuple `(paths, hashes, changed, records, docs, skipped)`;
    update every 5-name unpacking of `self.plan()` in
    `tests/ci/test_lobo_archives.py` (index-based uses such as `plan[2]` and
    `*plan[:4]` keep working). Read `ADDED_MEMBERS` with `getattr(..., ())` so
    the synthetic `SimpleNamespace` modules in that file still work.
- `apply(...)`: per stem split edits into replacements (key exists in the
  archive) and additions (record has `added`), call
  `Archive(raw).replace(replacements, additions={stored_name: bytes})`. Manifest
  `members` already carries the records (added members with null before hash).
  Restoring the backup removes added members; say so in the docs.
- Summary JSON gains `'skipped': [...]` (always present), `'added_members': n`
  and `'asset_source_sha256'` (when given). Print each skip line to stderr as
  `WARN: ...`.
- Exit codes unchanged: `--check` exits 1 when `changed` is non-empty (this
  includes missing icons only when `--asset-source` is given). A skip alone does
  not make `--check` fail (orchestrator decision 1); it is reported. The family
  package's ud preflight therefore passes on a source that has the icons, and
  on one that lacks them it passes with the WARN line.

### 3.2 `repair/pbo.py`: two Archive capabilities

`Archive.replace(self, replacements, additions=None)`.

**Member addition** (`additions`: `{stored_name: bytes}`, insertion order kept):

- Refuse on StuffPBO archives. Refuse names that fail `normalize()`, that
  normalize to an existing member or to each other, or non-bytes payloads.
- Layout (must match exactly, the expected hashes in section 9 depend on it):
  existing header records and payloads exactly as `replace()` writes them
  today, then one header record per addition appended after the last existing
  record and before the 21-byte terminator, `fields = (0, 0, 0, 0, len(payload))`
  (stored, no reserved value, timestamp 0), payloads appended after the
  existing payloads in the same order; SHA-1 tail recomputed when
  `checksummed`, any textual trailer kept.
- Verify on the result: member order is the original keys followed by the added
  keys, untouched members have identical fields and bytes, replaced and added
  members read back exactly.

**StuffPBO same-length policy** (needed only for `bwm_boot3.pbo`, CI-6):

- Today `replace()` raises `Legacy StuffPBO checksum rewriting is unsupported`
  for any change. New behaviour: allowed only when every replacement targets an
  uncompressed member (`fields[0] == 0`) and has exactly `fields[4]` bytes; no
  additions. The archive is copied and only those payload byte ranges are
  overwritten in place; header table and the 17-byte trailer
  (`05 21 01 'StuffPBO V1.0b'`) stay verbatim. Anything else still raises
  (keep the old message for the length/compression/addition cases).
- Verify: re-parse, trailer unchanged, member order and every untouched member
  identical.
- Rationale to document: the trailer's two leading bytes are an unknown legacy
  checksum. No StuffPBO reader exists in the UD/Remaster engine source, and OFP
  1.99 does not validate PBO trailers, so a stale value is ignored by every
  supported runtime. Only StuffPBO's own tooling could notice. It is the same
  same-length, header-preserving technique `mod doctor` uses for its in-place
  patches.

### 3.3 `repair/models.py`: MLOD trailer

`_lods()` currently accepts a trailer only when it is exactly 32 bytes starting
with `b'data\\\0'`. Relax to: exactly 32 bytes, containing a NUL, whose bytes
before the first NUL are a non-empty run of printable ASCII (32..126). This is
Oxygen's default texture path field (`data\` in the M1/Merkava/crew models,
`H:\Codemast` in `LoBo_MD500D_R.p3d`); preserve it verbatim as today. Any other
trailing data still raises.

### 3.4 New helper module `repair/rules.py`

Port `spec-proto/proto/common.py`:

- `split_newline(source) -> (newline, lf_text)`.
- `line_rule(text, old, new, count, baseline=0)`: whole-line substitution on
  LF text. `old` may be a tuple of alternative original lines. Requires
  `lines equal to any old + lines equal to new == count + baseline` and
  `lines equal to new >= baseline`; replaces every old line. Exact line equality
  cannot hit a longer line that merely contains the anchor.
- `apply_rules(documents, rules, expected_findings=None, post=None)`: rules
  `{member: ((kind, before, after, count[, baseline]), ...)}` with kind `sub`
  (existing `scripts._replace` semantics: original, repaired or partial state,
  `count` occurrences) or `line`. Missing member raises. After the rules, the
  optional `post[member]` (used for `scripts._explicit_exit`). Every `.sqs`
  result must have `script_findings(updated) == expected_findings.get(member, [])`
  exactly. Restore the member's newline; return only changed members.
- Pitfall (from issue-2): never write a `sub` rule whose `after` is a substring
  of `before`; `_replace` counts it as already repaired and does nothing. All
  rules below were checked against this.

`textures.py` has no text rules: give it `def repair(documents): return {}`
because `plan()` calls `repair()` on every module.

## 4. Rule modules

Notation. Script anchors are Python string literals on LF-normalized text
(`\n` newline, `\t` TAB, `\\` one backslash). In config tables the indent column
gives the leading whitespace (T2 = two TABs, T3 = three, T4 = four, S8 = eight
spaces, S16 = sixteen spaces) and the text columns show the rest of the line
verbatim with single backslashes. "Sites" is `count`; "+n" is the `baseline`.
Member keys are `stem/member` lowercase with `/`, as `plan()` builds them.

### 4.0 Baselines (exact-line rules whose replacement already exists)

| Member | Replacement line | Pre-existing |
| --- | --- | ---: |
| loboidfhelo/config.cpp | T2 `type=2;` | 2 |
| lobojorhelo/config.cpp | T2 `type=2;` | 2 |
| lobo_fab/config.cpp | T2 `type=1;` | 4 |
| lobo_fab/config.cpp | T2 `hasCommander=0` | 2 |
| loboweapons/config.cpp | T2 `model="\LoBoWeapons\LoBo_M4A1_m203.p3d";` | 1 |

### 4.1 `repair/loops.py` (lobo #2 plus AG-1, AG-3, AG-4, AG-10)

ADDONS: `coc_arty, csjairlines, csjils, lobo_a4, lobo_fab, loboart,
loboeghelo, lobof5, lobomig, lobosyrapc`. New to the tool: LoBoEghelo,
LoBoSyrAPC, CoC_Arty, CSJairlines, CSJils (CSJ members are compressed; read via
PoseidonTools, written uncompressed). 17 members. `post`: `_explicit_exit` for
the BMP script. Expected findings: exactly `["duplicate label: precisionset"]`
for both `m252start.sqs` copies, none elsewhere.

**L1 Sea King float plate (lobo #2).** LoBoEghelo.pbo `mk2\float.sqs`
(210 B, CRLF, no trailing newline; callers config.cpp:1305, :1354). Spins up to
50 passes per frame forever and leaks the `LoBo_floater` plate.

```python
('sub', '#loop\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\ngoto "loop"',
        '#loop\n?(not alive _Hfloat): deleteVehicle _floater; exit\n'
        '_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\n~0.1\ngoto "loop"', 1)
```
Result 268 B. `alive objNull` is false, so destruction and deletion both exit
and delete the local plate. Intended change: a destroyed Sea King is no longer
held on the water.

**L2 Dolphin arm (lobo #2).** LoBo_FAB.pbo `dolp\init.sqs` (431 B; caller
config.cpp:1907). Two rules:

```python
('sub', '#loop\n?(Behaviour _sub == "STEALTH")',
        '#loop\n?(not alive _sub): goto "exit"\n?(Behaviour _sub == "STEALTH")', 1)
('sub', '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n\ngoto "loop"',
        '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n~1\ngoto "loop"', 1)
```
Result 465 B. `#exit` exists. Arm follows behaviour within 1 s of a 50 s sweep.

**L3 MiG-25 gear (lobo #2).** LoBoMig.pbo `mig25\sqs\mig25gear.sqs` (2136 B;
callers config.cpp:1082, :1130).

```python
('sub', '#CheckPosLoop\n\n;********** check pilot status',
        '#CheckPosLoop\n~0.5\n\n;********** check pilot status', 1)
('sub', '\ngoto "CheckPosition"\n', '\n;goto "CheckPosition"\n', 1)
```
Result 2143 B. One wait covers all four back-edges; the commented jump was
unreachable and targeted a missing label.

**L4 Syrian BMP-1 doors (lobo #2).** LoBoSyrAPC.pbo `scripts\bmp11anim.sqs`
(717 B; callers config.cpp:165, :257). Rule plus `_explicit_exit`:

```python
('sub', '?(not alive _unit ): goto "exit"\n\ngoto "GO"\t',
        '?(not alive _unit ): goto "exit"\n~0.5\ngoto "GO"\t', 1)
```
Result 738 B (`Exit;` tail becomes `Exit;\n\n#Exit\nexit\n`, CRLF). The trailing
TAB keeps the anchor distinct from the second `goto "GO"`.

**L5 F-5 canopy (lobo #2).** LoboF5.pbo `scripts\autocanopy.sqs` (661 B;
callers config.cpp:223, :255, :286; SHA-1 trailer recomputed).

```python
('sub', 'goto "CanopyDown"\ngoto "PollLoop"', 'goto "CanopyDown"\n~0.5\ngoto "PollLoop"', 1)
```
Result 667 B. The loop must keep running (the Open Canopy action can reopen).

**L6 A-4 canopy (lobo #2).** LoBo_A4.pbo `scripts\canopy.sqs` (408 B; callers
config.cpp:234, :261, :288).

```python
('sub', '#Mainloop\n_pos = GetPos _plane1;', '#Mainloop\n?(not alive _plane1): exit\n_pos = GetPos _plane1;', 1)
('sub', '~0.0001\n', '~0.5\n', 1)
```
Result 433 B. Wrecks no longer get the canopy forced open every frame.

**L7 CoC artillery MaxWind self-jump (AG-1, no issue).** The MaxWind guard sits
directly under `#PrecisionSet` and jumps back to it, so a preset
`Co*MaxWind` (CoC_IFZeroPrecision patch logic, or mission init) spins the
script with no wait for the session and the asset never registers. The five
sibling scripts use `Goto "WindSet"`; `#WindSet` is three lines below in every
copy. Exact whole-line rule, count 1 each:

| Member | Current line | Replacement |
| --- | --- | --- |
| loboart/scripts/m252start.sqs | `?CoCM252MaxWind == CoCM252MaxWind:Goto "PrecisionSet"` | `?CoCM252MaxWind == CoCM252MaxWind:Goto "WindSet"` |
| loboart/scripts/m109start.sqs | `?CoCM109MaxWind == CoCM109MaxWind:Goto "PrecisionSet"` | `?CoCM109MaxWind == CoCM109MaxWind:Goto "WindSet"` |
| loboart/scripts/m1064a3start.sqs | `?CoCm252MaxWind == CoCm252MaxWind:Goto "PrecisionSet"` | `?CoCm252MaxWind == CoCm252MaxWind:Goto "WindSet"` |
| coc_arty/scripts/m252start.sqs | as LoBoArt m252start | as LoBoArt m252start |
| coc_arty/scripts/m109start.sqs | as LoBoArt m109start | as LoBoArt m109start |

Callers: LoBoArt config.cpp init EH 604, 816 (m109), 942 (M1064A3), 1115,
1134 (m252); CoC_Arty 1441 (m109), 1568 (m252). Clean first runs are unchanged
(the guard is false while the variable is undefined).

**L8 LoBo_FAB flag loops (AG-3).** `lcm\scripts\setflags.sqs`,
`lcm\scripts\setflags2.sqs`, `osa\scripts\setflags.sqs`,
`saar5\scripts\setflags.sqs` (446/447 B). Each polls `_driver in _boat` every
frame while an AI-crewed ship keeps its driver.

```python
('sub', '#repeat\n~0.001\n', '#repeat\n~1\n', 1)
```
Flag ownership is released up to 1 s after the driver leaves (see section 2.4).

**L9 CSJ airliner night lights (AG-4).** CSJairlines.pbo `scripts\AIdLights.sqs`
(943 B, compressed). At night the `#dlightsOn` branch loops back every 2 s with
no alive or engine test; each engine on/off event leaves another perpetual
script, surviving destruction and deletion (CSJ7376/CSJNI/CSJEUA/CSJAM and
LoBo_airline's LoBo_737_egyptair/LoBo_737_ELAL).

```python
('sub', '_plane setobjecttexture [16,"\\csjairlines\\pristroje.pac"]\ngoto "ckL"\n',
        '_plane setobjecttexture [16,"\\csjairlines\\pristroje.pac"]\n'
        '?(alive _plane) && (isengineon _plane): goto "ckL"\n', 1)
```
Result 983 B. A dead or shut-down aircraft now falls into `#dlightsOff`, which
clears the textures and exits. `isEngineOn` is already used by this script.

**L10 CSJ ILS helper (AG-10).** CSJils.pbo `ILSact.sqs` (270 B, compressed;
`CSJilsStart` init EH, config.cpp:81). `_distObjs` is computed once, so a
player starting more than 250 m away gets two knowsAbout and two reveal calls
every 0.02 s forever.

```python
('line', '_wait = .02', '_wait = 1', 1)
```
Result 268 B.

Tests (`tests/ci/test_lobo_loops.py`): synthetic fixtures built from the rule
anchors (like `test_lobo_followups.fixture()`), with CRLF and a trailing
`\r\n\r\n ` tail; exact output and fixed point; every `goto` back-edge to an
earlier label in each repaired fixture has a `~` line between label and jump
(port `tools/loops.py` from the issue-2 evidence); BMP explicit-exit append;
`m252start` keeps exactly its one duplicate-label finding and a second extra
finding fails; unknown, duplicate and missing-member rejection; partial repair
completion for the two-rule scripts.

### 4.2 `repair/mines.py` (lobo #3)

ADDONS: `jof_objects1, lobo_mines`. JOF_Objects1 is new to the tool (101
members, textual trailer ` ** PboTool (c)2002 JaZzBrE ** ` kept). 17 members.
No expected findings.

| Rule set | Before | After |
| --- | --- | --- |
| PLAIN | `'#Zut\n~0.001\n_J = nearestobject [_X,_Y,_Z]\n'` | `'#Zut\n~0.1\n? isNull _L1 : exit\n_J = nearestobject [_X,_Y,_Z]\n'` |
| LADUNG | `'#Zut\n~0.001\n_W = _D + 45\n'` | `'#Zut\n~0.1\n? isNull _L1 : exit\n_W = _D + 45\n'` |
| CLAYMORE | `'#Zut\n~0.01\n_M = _L1\n'` | `'#Zut\n~0.1\n? isNull _L1 : exit\n_M = _L1\n'` |
| LEUCHT 1 | `'#Zut\n~0.0001\n'` | `'#Zut\n~0.03\n? isNull _L1 : deleteVehicle _Mess; exit\n'` |
| LEUCHT 2 | `'#Ende\n_L1 animate ["aus", 1]\n'` | `'#Ende\ndeleteVehicle _Mess\n_L1 animate ["aus", 1]\n'` |
| LEUCHT 3 | `'? not alive _L1:exit\n'` | `'? not alive _L1:deleteVehicle _Mess; exit\n'` |

All `sub`, count 1. Members: PLAIN on `lobo_mines/scripts/{ap_mine, at1_mine,
at2_mine, at3_mine, at4_mine, at5_mine, at6_mine, egyptnavmine, m08navmine,
mk118_mine}.sqs` and `jof_objects1/ap_mine.sqs`; LADUNG on
`lobo_mines/scripts/ladung.sqs`, `jof_objects1/ladung.sqs`; CLAYMORE on
`lobo_mines/scripts/claymore.sqs`, `jof_objects1/claymore.sqs`; LEUCHT 1 to 3
on `lobo_mines/scripts/leucht_mine.sqs`, `jof_objects1/leucht_mine.sqs`.
Callers: LoBo_Mines config.cpp init EH 226, 281, 371, 418, 480, 526, 572, 618,
664, 710, 758, 776, 786; JOF_Objects1 114, 167, 256, 301.

Why: today each placed mine runs an untyped 50 m `nearestObject` scan every
frame for the whole session, and a deleted mine (every non-triggering MP client
after a detonation, or any script/editor deletion) keeps polling with
`_J != _L1` true, so the next forward-moving object near the old spot fires a
ghost explosion. `isNull` retires the script with its mine. 0.1 s keeps AP
detection at about 0.99 for maximum-speed crossings and AT detonation lag under
2 m at 70 km/h; the flare tripwire is only about 1 m wide, so 0.03 s (about the
frame rate the author tuned for) keeps it reliable while capping it at 33
passes per second. LEUCHT 2/3 delete the leaked global `HeliHEmpty` helper on
the trigger and pickup exits. The rule for `? not alive _L1:exit` must touch
only leucht_mine (the same text exists in ap/at/ladung/claymore and stays). The
.2 AP pickup path fix on `ap_mine.sqs` line 24 is a disjoint anchor
(composition verified from pristine).

Tests (`test_lobo_mines.py`): exact output and fixed point for each rule set;
CRLF and trailing bytes preserved; guard line present exactly once per polled
script; `? not alive _L1:exit` unchanged in non-flare fixtures; partial leucht
repair completed; unknown, duplicate, mixed-duplicate anchors and missing member
rejected.

### 4.3 `repair/clusters.py` (lobo #4, #5)

ADDONS: `lobo_a4, lobo_airammo, lobo_f4, lobof5, loboil28`. LoBoIL28 is new to
the tool (no prefix, no trailer, uncompressed; its extra root member
`copy of config.cpp` is never read). 7 members. Preconditions (fail closed):
`lobo_airammo/scripts/cbu/rockeye.sqs` exists and is non-empty;
`lobo_airammo/config.cpp` declares `LoBo_mk20Deployed`, `LoBoMk20Bomblet` and
`LoBoRBK500Bomblet` (regex `^\s*class\s+NAME\b`, case-insensitive). No expected
findings. `post`: `_explicit_exit` for the IL-28 fired script.

| Id | Member | Before | After |
| --- | --- | --- | --- |
| 4a | lobo_a4/scripts/firemissiles_a2.sqs (line 103) | `'exec "\\LoBoF16\\scr\\rockeye.sqs"'` | `'exec "\\LoBo_airammo\\Scripts\\cbu\\rockeye.sqs"'` |
| 4b | lobo_a4/scripts/firemissiles_a3.sqs (line 103) | same | same |
| 4c | lobo_f4/scripts/firemissiles_ww.sqs (line 130) | same | same |
| 4d | loboil28/script/fire_mk20.sqs | `'exec "\\LoBoF15\\scr\\rockeye.sqs"'` | `'exec "\\LoBo_airammo\\Scripts\\cbu\\rockeye.sqs"'` |
| 4d | loboil28/script/fire_mk20.sqs | `'_missobj = nearestobject[_plane, _ammoname]'` | that line + `'\n? isNull _missobj: goto "Exit"\n? !(local _missobj): goto "Exit"'` |
| 4e | lobof5/scripts/rockeye.sqs (line 28) | `'"LoBoMk20Deployed" createvehicle'` | `'"LoBo_mk20Deployed" createvehicle'` |
| 4f | loboil28/script/rockeye.sqs (line 28) | same | same |
| 5 | lobo_airammo/scripts/cbu/rbk500.sqs (line 49) | `'"LoBoRBKBomblet" createvehicle'` | `'"LoBoRBK500Bomblet" createvehicle'` |

All `sub`, count 1. Sizes: 4a 4399 to 4412, 4b 4395 to 4408, 4c 3091 to 3104,
4d 454 to 548, 4e 1524 to 1525, 4f 1526 to 1527, 5 1635 to 1638.

Why: neither `\LoBoF16\scr\` nor `\LoBoF15\scr\` contains `rockeye.sqs`
(removed in the Dec 2006 consolidation; all nine post-consolidation callers use
the airammo path), so A-4R/A-4RN/F-4 WW/IL-28 AS Mk-20s land as one point HE
bomb. IL-28 uses the airammo script (investigator recommendation; the
unreferenced `LoBoIL28\script\rockeye.sqs` is a 2005 snapshot); 4f fixes that
dormant copy anyway since the archive is rewritten. 4d adds the .1 owner guard,
because the fired event runs on every machine and a working exec would
otherwise spawn bomblets on every client. 4e/4f/5 fix undefined class literals
(dispenser model, and the 105 RBK-500 bomblets that today all fail). Renames, not
aliases (no conflicting dependents; matches the no-alias precedent).
4a to 4c share members with `scripts.MISSILE_PATHS`; anchors are disjoint and
the fixed point holds from both the installed and the pristine state.

Tests (`test_lobo_clusters.py`): exact changes and fixed point; missing or
empty destination script rejected; each missing config class rejected; unknown
and duplicate sources rejected; CRLF preserved; IL-28 guard and explicit exit
idempotent; composition with `scripts._missile` on a synthetic A-4 fixture.

### 4.4 `repair/numbers.py` (lobo #8)

ADDONS: `lobo_f4, lobof15, lobof16, loboil28`. 5 members. Port
`proto/numbers.py` (the issue-8 draft). For each member apply the LCM form of
.1 (`scripts._number` steps) with a per-ladder slot map:

| Member | num1 slot | `_number = _this select 1` | Live callers |
| --- | ---: | --- | --- |
| lobof15/scr/number.sqs | 2 (unchanged) | replaced by the LCM comment | 9 classes, config.cpp 277..1238 |
| lobof16/scr/number.sqs | 2 | replaced | 9 IAF classes |
| lobof16/scr/number2.sqs | 2 | replaced | 4 EAF classes |
| loboil28/script/number.sqs | 0 (2,3,4 become 0,1,2) | replaced | 5 classes via common_init(2).sqs |
| lobo_f4/scripts/number.sqs | 1 (2,3,4 become 1,2,3) | kept (init passes `714`) | 5 classes |

Steps on LF text: (1) `_replace('_number = _this select 1', '; Init provides only the vehicle; an unused second argument is unnecessary.')`
unless kept; (2) `_unit setObjectTexture` to `_plane setObjectTexture` x30;
(3) `goto "n2"` to `goto "Num2"` x10, `goto "n3"` to `goto "Num3"` x10;
(4) `], goto "` to `]; goto "` x30; (5) slot remap with
`^(\?_num([123]) >=? \d+: _plane setObjectTexture \[)(\d+)(?=, ")`, accepting
only the original slot N+1 or the target `base+N-1`, exactly 10 lines per
ladder; (6) `_explicit_exit`. Script findings must be empty. Example F-4 result
line: `?_num1 > 90: _plane setObjectTexture [1, "\LoBo_F4\nums\w9.paa"]; goto "Num2"`.
Results: F-15/F-16 x2 2581 B, IL-28 2611 B, F-4 2530 B.

Why: `_unit` is never assigned (silent nil) and the first `goto "n2"` exits
the script (unknown label), so no digit is ever drawn. The IL-28 and F-4 slot
maps are a second defect the issue missed: IL-28 `hiddenSelections[]` is
`{num1,num2,num3}` (index 0..2), F-4 is `{roundl,num1,num2,num3,burn,...}`
(index 4 is the afterburner flame that burner.sqs/ab.sqs drive); the LCM slots
would hide two IL-28 digits and paint a digit on every F-4 flame.

Tests (`test_lobo_numbers.py`): synthetic ladder fixture parametrized by base
slot 2, 1, 0 with exact output; F-4 keeps `_number`, others get the comment;
idempotency and partial completion; unexpected slot, wrong ladder count,
foreign variable and missing member rejected; CRLF preserved; the .1 LCM paths
are untouched (module never lists them).

### 4.5 `repair/effects.py` (lobo #9 plus the KioGlowFire sibling)

ADDONS: `lobo_airammo`. Member `lobo_airammo/config.cpp` (55287 B, CRLF, no
trailing newline, no `CfgVehicles` today; byte-identical in installed @LoBo,
the staged Classic copy and @LoBo_UD). Insert before the unique anchor
`'};\nclass CfgNonAIVehicles\n'` (replace with `'};\n' + BLOCK + 'class CfgNonAIVehicles\n'`),
CRLF restored. BLOCK (each line followed by a newline):

```
class CfgVehicles
{
	class All {};
	class Static: All {};
	class Building: Static {};
	class NonStrategic: Building {};
	class Fire: NonStrategic {};
	// UD repair, LoBo issue 9: invisible fireplace that the wreck fire.sqs
	// scripts create for inflame and say. Vanilla Fire supplies simulation,
	// sound, Light and Smoke; only the campfire mesh is removed.
	class LoBo_WreckGlow: Fire
	{
		scope=1;
		model="empty";
		displayName="Wreck fire";
	};
	class M1Smoker: LoBo_WreckGlow {};
	class M60Smoker: LoBo_WreckGlow {};
	class Smoker: LoBo_WreckGlow {};
	class KioGlowFire: LoBo_WreckGlow {};
};
```

(indentation is TABs). Result 55908 B. Idempotency and fail-closed: if the full
block occurs once, exactly one `class CfgVehicles` exists and all five names are
declared, no change; otherwise any block fragment, any `class CfgVehicles`, or
any of the five names declared raises (partial or foreign state); the anchor
must occur exactly once. Composes with `ammo._airammo` (disjoint regions).

Why: `M1Smoker` (11 scripts), `M60Smoker` (4), `Smoker` (1) and `KioGlowFire`
(5 helicopter scripts) are defined nowhere (all @LoBo configs incl. compressed
and binarized, @LoBo_UD, 40 @LoBo_Deps pbos, Classic and Remaster base configs).
`createVehicle` returns objNull, so on the owner path (the only reachable one)
there is no light, no Fireplace, no crackle loop and no `say`. Vanilla `Fire` is
the only `simulation="fire"` class in CWA (FireplaceBurning does not exist).
The base chain must be declared exactly as above with empty bodies (a merge into
Remaster CONFIG.BIN only adds classes). `model="empty"` removes the campfire mesh
and its geometry LOD; with no memory point the light sits at the object origin,
which the scripts place on the wreck (1 m above it for the ground scripts; the
helicopter scripts start at 1.5 m and then move it to the wreck position).

Profile behaviour: config-only, identical in both profiles; LoBo_airammo is
already in both archive sets, so `ExpectedClassicSwitch` does not change.
Activation caveat: owner addon `lobo_airammo`; when a mission has not activated
it, UD and the official 3.05 exe log one `Access denied ... not activated` WARN
and still create the object; stock 1.99 behaviour is unverified (legacy).

Tests (`test_lobo_effects.py`): insertion on a synthetic CRLF config with the
anchor; already-applied is a no-op; partial block, foreign `CfgVehicles`, a
pre-declared `Smoker` and a missing or duplicated anchor are rejected; CRLF and
all other bytes preserved; the block's class chain is exactly All, Static,
Building, NonStrategic, Fire.

### 4.6 `repair/identifiers.py` (CI-1, 2, 3, 4, 5, 7, 8, 9; no issue)

Undefined identifiers in config values read as 0 at runtime (no `#define` in
the file, not in the CONFIG.BIN variable table, or quoted so macros never
expand). Each value token becomes the literal the standard BIS define gives.
All `line` rules. 29 members; ADDONS = their stems. New to the tool: BDE_mir3,
CSJairlines and CSJcitation (compressed configs), CoC_Torpedoes (compressed
config), LoBoC130, LoBoIL28, LoBoJorHelo, LoBoChallen, MAP_OilAddon_script,
LoBoHizballCar, LoBo_110, CoC_Arty, LoBoEghelo. Already in the tool: LoBoArt
(ammo), LoBoMerkava2/4 (models, scripts), LoBoMig and LoboIDFhelo (scripts,
followups), LoBo_A4, LoBo_F4, LoBo_Kfir, LoboF5, LoBoF15, LoBoF16, LoBo_FAB,
LoBoCen, LoBoEgAPC, LoBo_ZU23, LoBo_BTR60 (scripts).

**CI-1 crew perception** (CanSeeRadar 1, Eye 2, Optics 4, Ear 8, Compass 16,
All 31). 0 means no optics, a 17 degree eye cone, deaf, no radar/compass.

| Member | Ind | Current | Replacement | Sites |
| --- | --- | --- | --- | ---: |
| coc_arty/config.cpp | T2 | `driverCanSee = CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;` | `driverCanSee = 27;` | 1 |
| coc_arty/config.cpp | T2 | `drivercansee = CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;` | `drivercansee = 27;` | 1 |
| loboart/config.cpp | T2 | `driverCanSee = CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;` | `driverCanSee = 27;` | 1 |
| loboart/config.cpp | T2 | `drivercansee = CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;` | `drivercansee = 27;` | 2 |
| lobohizballcar/config.cpp | T2 | `gunnerCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;` | `gunnerCanSee = 29;` | 1 |
| lobohizballcar/config.cpp | T2 | `driverCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;` | `driverCanSee = 29;` | 1 |
| lobo_110/config.cpp | T2 | `driverCanSee=CanSeeEye+CanSeeEar;` | `driverCanSee=10;` | 1 |
| lobo_btr60/config.cpp | S16 | `driverCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;` | `driverCanSee=22;` | 1 |
| lobo_btr60/config.cpp | S16 | `gunnerCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;` | `gunnerCanSee=22;` | 1 |
| loboidfhelo/config.cpp | T2 | `gunnerCanSee = CanSeeAll;` | `gunnerCanSee = 31;` | 1 |
| loboidfhelo/config.cpp | T2 | `gunnerCanSee = CanSeeEye+CanSeeCompass+CanSeeOptics;` | `gunnerCanSee = 22;` | 1 |
| loboidfhelo/config.cpp | T2 | `driverCanSee = CanSeeEye+CanSeeCompass;` | `driverCanSee = 18;` | 1 |
| lobojorhelo/config.cpp | T2 | `gunnerCanSee = CanSeeAll;` | `gunnerCanSee = 31;` | 1 |
| lobomerkava2/config.cpp | T2 | `gunnerCanSee="CanSeeAll";` | `gunnerCanSee=31;` | 1 |
| lobomerkava4/config.cpp | T2 | `gunnerCanSee="CanSeeAll";` | `gunnerCanSee=31;` | 1 |

Root causes: `CanseeCompass` case typo for `CanSeeCompass` (CoC_Arty, LoBoArt), no defines
(LoBoHizballCar, LoBo_110, LoboIDFhelo/LoBoJorHelo `CanSeeAll`), `#define CanSeeRye 2`
(LoBo_BTR60; the define stays, it becomes unused), quoted `"CanSeeAll"`
(Merkavas). Classes: CoC_M270MLRS, CoC_M109PALADIN, LoBo_M109A6_IDF(+Egy),
LoBo_M270MLRS_IDF(+Egy), LoBo_M1064A3_120_IDF, LoBo_ural_Hizbal_BM21,
LoBo_ATV_IDF(+3), LoBo_BTR60(+Syria, BRDM-2), LoBo_AH1(+F_IAF), LoBo_Panther_M_IAF,
LoBo_AH1S_Jor(+4), LoBoMerkava1 family, LoBoMerkava4DT family.
`PilotCanSee` lines are never read and stay.

**CI-2 aircraft kind** (`VAir` = 2, the value Plane/Helicopter would give;
today 0 = VSoft: AI rifles rated 20x too effective, AA undervalued, formation
points on the ground, kills booked as soft).

| Member | Ind | Current (all sites) | Replacement | Sites (+base) |
| --- | --- | --- | --- | ---: |
| bde_mir3/config.cpp | T2 | `type ` + 4 TAB + `= VAir;` | `type ` + 4 TAB + `= 2;` | 1 |
| csjairlines/config.cpp | T2 | `type=VAir;` | `type=2;` | 1 |
| csjcitation/config.cpp | T2 | `type=VAir;` | `type=2;` | 1 |
| loboc130/config.cpp | T2 | `type=VAir;` | `type=2;` | 1 |
| loboeghelo/config.cpp | T2 | `type=VAir;` or `type="VAir";` | `type=2;` | 6 |
| lobof15/config.cpp | T2 | `type="VAir";` | `type=2;` | 9 |
| lobof16/config.cpp | T2 | `type=VAir;` | `type=2;` | 13 |
| loboil28/config.cpp | T2 | `type=VAir;` | `type=2;` | 5 |
| lobomig/config.cpp | T2 | `type=VAir;` or `type="VAir";` | `type=2;` | 10 |
| lobo_a4/config.cpp | T2 | `type=VAir;` | `type=2;` | 1 |
| lobo_f4/config.cpp | T2 | `type=VAir;` | `type=2;` | 5 |
| lobo_kfir/config.cpp | T2 | `type ` + TAB + `= VAir;` | `type ` + TAB + `= 2;` | 1 |
| lobof5/config.cpp | T2 | `type=VAir;` | `type=2;` | 3 |
| loboidfhelo/config.cpp | T2 | `type=VAir;` or `type="VAir";` | `type=2;` | 8 (+2) |
| lobojorhelo/config.cpp | T2 | `type=VAir;` | `type=2;` | 1 (+2) |

66 sites, 100 classes including LoBo_airline's 737s (inherit CSJ7376) and the
Plane-derived LoBo_K36EjectionSeat. Quoted sites must become unquoted literals
(a header would not help).

**CI-3 armor kind** (`VArmor` = 1, the value Tank/M1Abrams/Ship give).

| Member | Ind | Current | Replacement | Sites (+base) |
| --- | --- | --- | --- | ---: |
| coc_arty/config.cpp | T2 | `type=Varmor;` | `type=1;` | 1 |
| loboart/config.cpp | T2 | `type=Varmor;` | `type=1;` | 2 |
| lobocen/config.cpp | T2 | `type=VArmor;` | `type=1;` | 1 |
| lobochallen/config.cpp | T2 | `type=VArmor;` | `type=1;` | 1 |
| lobo_fab/config.cpp | T2 | `type=VArmor;` | `type=1;` | 1 (+4) |
| map_oiladdon_script/config.cpp | T2 | `type="VArmor";` | `type=1;` | 2 |

Exact-line matching leaves the commented `//\t\ttype=VArmor;` at CoC_Arty:841
alone. Car-derived `VArmor`/`Armor` sites are CI-11 (not fixed).

**CI-4 crew move states** (`SPEED_STATIC` 1e10, `RestDuty` -1; today speed 0
means a static pose never "finishes", so connectTo transitions such as the
MH-60 and MLRS crew death moves never fire; duty 0 means no fatigue recovery).

| Member | Ind | Current | Replacement | Sites |
| --- | --- | --- | --- | ---: |
| loboegapc/config.cpp | T3 | `speed="SPEED_STATIC";` | `speed=1e10;` | 33 |
| loboegapc/config.cpp | T3 | `duty="RestDuty";` | `duty=-1;` | 32 |
| loboart/config.cpp | T3 | `speed="SPEED_STATIC";` | `speed=1e10;` | 3 |
| loboart/config.cpp | T3 | `duty="RestDuty";` | `duty=-1;` | 2 |
| loboidfhelo/config.cpp | T4 | `speed=SPEED_STATIC; \` | `speed=1e10; \` | 5 |
| loboidfhelo/config.cpp | T3 | `duty = RestDuty;` | `duty = -1;` | 1 |
| coc_arty/config.cpp | T4 | `speed=SPEED_STATIC; \` | `speed=1e10; \` | 2 |

The T4 lines are macro-body continuations: the line must still end in
`space backslash`. LoBo_ZU23 defines `SPEED_STATIC 1000` itself and is not
touched.

**CI-5** loboart/config.cpp, T2 `weaponType=WeaponHardMounted;` to
`weaponType=65536;`, 2 sites (LoBo_120Mortar, LoBo_M160Mortar; today 0 hides
the mortar from `weapons` and offers a needless Reload action).

**CI-7** coc_torpedoes/config.cpp (compressed), T2 `canLock=LockYes;` to
`canLock=2;`, 1 site (CoC_Mk48LauncherBoat and the whole CoC launcher family,
including LoBo_FAB's LoBo_Mk48LauncherSUB on the Dolphin; enables player
lock-on, AI unaffected).

**CI-8** lobo_zu23/config.cpp, T2 `UnitInfoType=Helicopter,` to
`UnitInfoType=1;`, 1 site (LoBo_ZU23 and 3 children; gunner HUD shows the tank
panel the class would inherit).

**CI-9** lobo_fab/config.cpp, T2 `hasCommander=o` to `hasCommander=0`
(no semicolon, as authored), 1 site (+2 baseline); LoBo_Mubarak_egypt; removes
the `'o'` warning, same effective value.

Engine-parsed diff (cfgdiff) after the rules: only the listed keys change, for
example `CfgVehicles/LoBo_Panther_M_IAF/gunnerCanSee: "CanSeeEye+CanSeeCompass+CanSeeOptics" -> 22`,
`CfgMovesMC/States/Lobo_mh60cargo10Dead/speed: "SPEED_STATIC" -> 1e10`.

Tests (`test_lobo_identifiers.py`): `line_rule` unit tests (tuple originals,
baseline accounting, a longer line containing the anchor is ignored,
mixed/partial states, wrong totals rejected, CRLF kept); one synthetic config per
rule shape (plain, quoted, aligned `type \t= VAir;`, macro line ending in
`space backslash`, newline-terminated `hasCommander=o`); commented
`//\t\ttype=VArmor;` untouched; fixed point.

### 4.7 `repair/content.py` (AG-5, AG-6, AG-11, CI-6; no issue)

ADDONS: `bwm_boot3, lobois, rksl-hangers, tmyk_bridges` (all new to the tool;
RKSL-Hangers and TMYK_Bridges configs are compressed). 4 members.

| Id | Member | Ind | Current | Replacement | Sites |
| --- | --- | --- | --- | --- | ---: |
| AG-5 | rksl-hangers/config.cpp | T2 | `armour=1000;` | `armor=1000;` | 3 |
| AG-5 | rksl-hangers/config.cpp | T2 | `armour=2000;` | `armor=2000;` | 6 |
| AG-6 | tmyk_bridges/config.cpp | S8 | `model = "\TMYK_Bridges\TMYK_ViaductCenter.p3d"` | `model = "\TMYK_Bridges\TMYK_Viaduct.p3d";` | 1 |
| AG-6 | tmyk_bridges/config.cpp | S8 | `icon = "\TMYK_Bridges\icons\TMYK_Viaduct.pac";` | `icon = "\TMYK_Bridges\icons\TMYK_ViaductCenter.pac";` | 1 |
| AG-11 | lobois/config.cpp | T2 | `model="\LoBoIs\LoBo_para.p3d";` | `model="para";` | 1 |
| CI-6 | bwm_boot3/config.cpp | T2 | `crew="BWMOD_Soldier";` | `crew="SoldierWB";` + 4 spaces | 1 |

- AG-5: the engine reads `armor`, never `armour`; all nine RKSL hangar classes
  fall back to Building armor 150. They are live island objects bound by model
  name (Sinai 27, Lebanon80 16, s_iran_iraq 19, Gaza 5). Intended balance change:
  hangars take the authored 1000/2000.
- AG-6: neither `TMYK_ViaductCenter.p3d` nor `icons\TMYK_Viaduct.pac` exists;
  the pbo has `TMYK_Viaduct.p3d` and `Icons\TMYK_ViaductCenter.pac` (basenames
  swapped), and the model line lacks `;`. A placed "Viaduct (center)" is an empty
  shape today. Side effect: the 12 s_iran_iraq centre viaducts start binding to
  TMYK_Viaduct (armor 25000, DestructBuilding), matching the already-bound left
  and right pieces.
- AG-11: `LoBo_para.p3d` has MLOD version field `0x3F8CCCCD` (rejected by
  LODShape::Load; also unreadable by PoseidonTools), so LoBo_paraW/E/G/C spawn
  invisible. `"para"` is the vanilla ParachuteBase model the class would inherit
  (present in Classic and Remaster Data3D.pbo). Editor-only units; the HALO
  chutes override the model and are unaffected.
- CI-6: `BWMOD_Soldier` is defined nowhere in @LoBo, @LoBo_Deps or the base
  game; a manned Boot3 hits `Invalid crew` in `AICenter::CreateSoldier` and
  then dereferences null (exit under `--strict`, crash otherwise). `SoldierWB`
  is BoatW's inherited crew and the base of BWMod's own `BWMOD_Soldier`; side
  West matches `side=TWest` (defined 1 in the file). The archive is StuffPBO:
  the module asserts the changed member keeps its length (1169 B) and the
  archive path uses 3.2's in-place policy. CI-6 plus the StuffPBO policy is
  self-contained: if the orchestrator drops it, remove only this rule, the
  policy and their tests (see section 2.11 for the BWMod interplay).

Tests (`test_lobo_content.py`): each rule exact and idempotent; CI-6 output
length equals input length and a non-padded variant is rejected by the module;
missing member rejected. Archive-level StuffPBO tests in 3.2.

### 4.8 `repair/weapons.py` (orchestrator decisions 1 and 2; no issue)

ADDONS: `loboweapons` (already used by `ammo`). Text member
`loboweapons/config.cpp` (103063 B, CRLF):

| Id | Ind | Current | Replacement | Sites (+base) |
| --- | --- | --- | --- | ---: |
| R10/22 (line 2123) | T2 | `picture="\LoBoWeapons\LoBoWeapons\Inventory\w_1022sd.paa";` | `picture="\LoBoWeapons\Inventory\w_1022sd.paa";` | 1 |
| R10/22 mag (line 2145) | T2 | `picture="\LoBoWeapons\LoBoWeapons\Inventory\M_1022sd.paa";` | `picture="\LoBoWeapons\Inventory\M_1022sd.paa";` | 1 |
| Glock 17 (line 2239) | T2 | `picture="\LoBoWeapons\GearPics\glock18gear.paa";` | `picture="\LoBoWeapons\GearPics\glockgear.paa";` | 1 |
| MSG90 (line 3456) | T2 | `picture="\TheHKpack\inventory\w_msg90.paa";` | `picture="\LoBoWeapons\inventory\w_msg90.paa";` | 1 |
| M4A1 LD M203 (line 1362) | T2 | `model="\LoBoWeapons\LoBo_M4A1_LD_m203.p3d";` | `model="\LoBoWeapons\LoBo_M4A1_m203.p3d";` | 1 (+1) |
| KSVK ammo (line 374) | T2 | `soundHitBuilding1[]={"\LoBoWeapons\Sounds\KsvkHitBuilding1.wav",0.031623,1};` | `soundHitBuilding1[]={"weapons\bulletricochet4",0.031623,1};` | 1 |
| KSVK ammo (line 375) | T2 | `soundHitBuilding2[]={"\LoBoWeapons\Sounds\KsvkHitBuilding22.wav",0.031623,1};` | `soundHitBuilding2[]={"weapons\bulletricochet5",0.031623,1};` | 1 |

Result 103001 B. Decisions:

- R10/22 doubled bank prefix and Glock 17 icon: exact existing targets
  (`Inventory\w_1022sd.paa`, `Inventory\M_1022sd.paa`, `GearPics\glockgear.paa`).
- MSG90: no `\TheHKpack` bank exists anywhere; repointed to the LoBoWeapons
  path the added icon occupies. Unconditional: without the icon the picture is
  missing either way, so the edit never makes things worse.
- M4A1 LD M203: no `LoBo_M4A1_LD_m203.p3d` exists in any pbo. The parent class
  `LoBo_M4A1_m203` uses `LoBo_M4A1_m203.p3d` with the same iron-sight optics
  (`optic_m16.p3d`) and M203 muzzle; the class's distinguishing laser is the
  `LoBo_M4A1_LD` muzzle (cursor), not the model. This is the model inheritance
  would give, so it is the clear equivalent; only the LD accessory box is
  missing visually. The alternative `LoBo_M4A1_LD_Acog_m203.p3d` would show an
  ACOG that contradicts the iron-sight optics. The class has no unit users
  (editor/script only).
- KSVK building hits: no KSVK building-impact sound exists in any pbo (only
  HitArmor1/2, HitGround1..3, HitMan1/2). The equivalents present are the
  vanilla BulletSingle building sounds the class would inherit without the
  override, at the same volume; they exist in Classic and Remaster
  `Sound.pbo`. KSVK armor sounds were rejected (vehicle metal impacts).

**Member additions** (decision 1). `ADDED_MEMBERS` and
`add_members(existing, asset_source)` as in `proto/weapons.py`:

| Added stored name in LoBoWeapons.pbo | Source member in TheHKpack.pbo 1.75 | Bytes | sha256 |
| --- | --- | ---: | --- |
| `Inventory\w_msg90.paa` | `inventory\w_msg90.paa` | 26629 | `df3c75bbf03feac8ae6852664561aa351fb6e0941975e5f62ef0a816776bc507` |
| `Inventory\w_psg1.paa` | `inventory\w_psg1.paa` | 19051 | `7565c8428f8b80590446844fca9d9f410542aab2a94582cd93eb9fea7557c4f6` |
| `Inventory\w_ump.paa` | `inventory\w_ump.paa` | 32606 | `672af23564079246c0cc078d8a3dfcf9a002b6e4d15801159d87e890121cc1fd` |
| `Inventory\w_mp5n.paa` | `inventory\w_mp5n.paa` | 41102 | `3dd51b7a66e18425370e32c3c8b18dd0d469f961234b2b2db35c4433f04f4546` |
| `Inventory\w_mp5k.paa` | `inventory\w_mp5k.paa` | 31391 | `3a7d928fb1be5dbdc6637c581689f3d8af32ed6811fe4359f820a46e43b01fa7` |
| `Inventory\w_mp5kpure.paa` | `inventory\w_mp5kpure.paa` | 26262 | `a9d382e29592a68eeae361c20f4ca2e5069c7b62dfc5856bc40a9e5ff6e923ca` |
| `Inventory\m_mp5n.paa` | `inventory\m_mp5n.paa` | 4381 | `f8a0590906680bbbda19f8d90f0a9db7751ffdb0c6350d6a7a6342af1b0e0e06` |

(list re-verified: these are exactly the seven LoBoWeapons picture paths that
resolve to no member: PSG1 3437, UMP 3475, MP5N 3493, MP5A3 3511, MP5Kpdw 3529,
MP5K 3549, MP5Kmag 3603, plus MSG90 after its repoint. `Inventory\` matches the
existing folder spelling; lookups are case-insensitive, QBStream lowercases.)

- Pinned archive: sha256 `79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5`
  (31104109 B; extracted with 7-Zip from the recovered `ofphk-175.exe`, sha256
  `f8921148ad24eaa2cae50dd5495df91a14b5af5af6b2e9219b5a69444ead7a5a`, never
  run). Reference path:
  `D:/LoBo-Reloaded/_work/inspect/hk/x/ofphk-175/TheHKpack.pbo`.
- `add_members` contract: every present target must already equal its pin
  (else raise); when `asset_source` is given its archive hash must equal the pin
  (else raise, even if nothing is missing); each source member must be
  uncompressed and match its pin; with no source and missing icons, return no
  additions and one report line naming the missing files and the expected
  source. Additions are appended in the table's order (layout in 3.2).
- Credit (docs): OFP: HK ("The HK Pack") 1.75 by the LoBo Team and Vipersheart,
  2002. LoBo referenced these icons but never shipped them in LoBoWeapons.

Tests (`test_lobo_weapons.py`): config rules exact and idempotent (with the
M4A1 baseline); add_members with a synthetic source PBO and patched pins
(`unittest.mock.patch.dict` / `patch.object`): adds all when missing, nothing
when present, skip line without source, wrong source hash raises, present but
wrong bytes raises, compressed or mismatching source member raises, fixed point.

### 4.9 `repair/textures.py` (textures-icons d1, e1, e2; no issue)

ADDONS: `loboidfhelo, lobojordan, lobosyria`; `MODEL_MEMBERS` as below;
`repair()` returns `{}`; `repair_models()` uses the relaxed `models._lods`
(3.3). MLOD 257 / SP3X 153: face table at
`lod.start + header + points*16 + normals*12`, 104 bytes per face, bytes 0..31
are the texture field (read to the first NUL; UD copies 32 bytes into a 33-byte
buffer, Classic relies on a NUL). New names are at most 31 characters, written
followed by NUL fill to 32 bytes. Match the NUL-terminated name
case-insensitively, or all 32 raw bytes for overflowed names (no NUL). Only the
listed LOD resolutions may contain a match (raise otherwise). Per listed LOD,
original matches plus faces already equal to `new + NUL fill` must equal the
expected count. File length and every other byte unchanged.

| Id | Member | LODs (resolution) | Old field | New name | Per LOD |
| --- | --- | --- | --- | --- | ---: |
| d1 | loboidfhelo/lobo_md500d_r.p3d | 1e13, 5e15, 7e15 | `bas_lbs\m\uh1ffarpod.pac` | `LoboIDFhelo\MD\uh1ffarpod.pac` | 10 |
| d1 | same | same | `bas_lbs\m\uh1podend.pac` | `LoboIDFhelo\MD\uh1podend.pac` | 8 |
| e1 | lobojordan/lobo_crew_jor.p3d | 5.0 | `LoBoJordan\wl\rba_frt.paa` | `LoBoJordan\wl\ba_frt.paa` | 26 |
| e1 | same | 5.0 | `LoBoJordan\wl\rba_bk.paa` | `LoBoJordan\wl\ba_bk.paa` | 26 |
| e2 | lobosyria/lobo_crew_syria.p3d | 5.0 | raw32 `LoBoSyria\syr_crew\e_tank_hrud_p` | `LoBoSyria\crew\hrud_p.pac` | 23 |
| e2 | same | 5.0 | raw32 `LoBoSyria\syr_crew\e_tank_nohy_p` | `LoBoSyria\crew\nohy_p.pac` | 15 |
| e2 | same | 5.0 | raw32 `LoBoSyria\syr_crew\e_tank_nohy_z` | `LoBoSyria\crew\nohy_z.pac` | 6 |
| e2 | same | 5.0 | raw32 `LoBoSyria\syr_crew\e_tank_zada_z` | `LoBoSyria\crew\zada_z.pac` | 18 |
| e2 | same | 5.0 | raw32 `LoBoSyria\syr_crew\00000mc_tanki` | `data\00000mc_tankistae2.paa` | 38 |
| e2 | same | 5.0 | raw32 `LoBoSyria\syr_crew\00002mc_tanki` | `data\00002mc_tankistae2.paa` | 24 |

- d1: the MD500D_R collision, hitpoint and fire LODs still name the BAS Little
  Birds source textures; each load logs 54 `GL33: Cannot load texture` WARNs and
  pod contacts get a null surface. The visual LODs already use the LoboIDFhelo
  copies. Model is 3547020 B, trailer `H:\Codemast` (needs 3.3); 1572 bytes change.
- e1: LOD 5.0 (visible distance LOD) of the Jordanian crew (Challenger, APCs,
  M60) names `rba_*` textures that never shipped; LOD 0/1 use `ba_*` on the same
  `bodyarmor` faces. 494 bytes change.
- e2: LOD 5.0 of the Syrian crew (T-72, BTR-60, Syrian APCs): names overflowed
  the 32-byte field (folder `syr_crew` does not exist). The four `e_tank_*`
  targets are the recolours LOD 0..2 use; the two merged atlases match the
  truncated names and the same LOD already uses `data\00004mc_tankistae2.paa`
  (123 faces), with `00006mc`/`00001mc` in LOD 5.23, so base atlases are part of
  this model's look. Six NUL-led residues of these names in LODs 2/5/6 are left
  alone (a NUL-led field never matches a raw32 pattern). 3348 bytes change.

Tests (`test_lobo_textures.py`, synthetic MLOD built like
`test_lobo_models.py`): repoint exact bytes and fixed point; raw32 matching;
NUL-led residue untouched; a match in an unlisted LOD raises; wrong count raises;
name longer than 31 raises; `H:\Codemast`-style trailer accepted and preserved,
non-path 32-byte trailer still rejected (add the trailer cases to
`test_lobo_models.py` too).

## 5. Profile behaviour

Every rule in section 4 is profile-neutral and produces identical bytes under
`--compatibility ud` and `classic` (prototype: the classic plan differs from the
ud plan only in the 16 `fire.sqs` wreck queries the .1 profile switch already
owns). Consequences for packaging:

- `package-lobo-family.ps1` `ExpectedClassicSwitch` needs no change (the new
  edits land in the source in both profiles, so the staged classic switch still
  rewrites only the 16 fire scripts in the same 14 archives).
- `ExpectedRepairVersion` (still `2026-10-11.1`) and `ExpectedRuleSha256` only
  warn; the orchestrator may refresh them to `2026-10-11.3` and the new rule
  sha after this lands.
- The source @LoBo must have .3 applied (with `--asset-source`) before
  packaging, otherwise the preflight ud `--check` exits 1.
- Classic 1.99 exe: all new SQS commands and config literals exist there; the
  only unverified Classic behaviour is a possible missing-addon notice for
  `lobo_airammo` when a mission creates a wreck glow without activating it
  (legacy, accepted).

## 6. Documentation changes (worktree, no em dashes)

`arma_CWR/tools/lobo/CONTENT-REPAIRS.md`:

- New section `## 2026-10-11.3 batch` above the .2 section: one bullet per
  module with the issues it closes (#2, #3, #4, #5, #8, #9) and the audit items,
  the KioGlowFire choice and its burn consequence, the activation caveat for
  `lobo_airammo`, the HK icon credit (OFP: HK 1.75 by the LoBo Team and
  Vipersheart, 2002; copied byte for byte from a user-supplied, hash-pinned
  `TheHKpack.pbo`; never stored in either repository), "no game launched", and
  the savegame caveat (SQS scripts already running in a save keep their old
  compiled lines; tail numbers and other `setObjectTexture` results are not
  saved by the engine).
- `## Use`: add the `--asset-source` example and the skip behaviour.
- `## Backups, integrity and recovery`: replace "Old StuffPBO trailers can be
  inspected, but rewriting their unsupported checksum format is rejected" with
  the same-length in-place policy (3.2); add that added members are removed by
  restoring a batch's originals.
- Repair-scope table, Wreck fires row: the non-owner `_glow = _object` branch
  cannot play `say` on a destroyed object (ObjSay returns early) and is
  unreachable anyway (the killed handler runs on the owner); say "valid sound
  anchor on the reachable owner path" and point to .3 for the missing classes.
- `## Validation limits`: mention that the .3 configs were checked with the
  engine parser (PoseidonTools `config tojson`) before and after.

`AMMUNITION-POLICY.md`: one short subsection "Cluster delivery (2026-10-11.3)":
no CfgAmmo value changes; the A-4/F-4/IL-28 Mk-20 and MiG-21 RBK-500 now run
the existing airammo bomblet scripts (197 and 105 bomblets), and the KSVK
building-hit sounds point to the vanilla ricochets.
`ARMOR-MODELS.md`: no change (vehicle kind and crew perception are AI inputs,
not component armor).

Orchestrator follow-ups outside the tool (not for the implementer): update the
roadmap item that tracks LoBo repairs; refresh the package script's expected
version and rule sha; optionally preload `LoBo_airammo` on UD via
`guerrilla-mode/config/lobo-factions.hpp` (`class CfgAddons { class PreloadAddons { class LoBoWreckFx { list[] = {"LoBo_airammo"}; }; }; };`)
if the runtime phase finds the activation WARN noisy.

## 7. Implementation and offline verification procedure

1. Implement 3.1 to 3.4 and the nine modules by porting `spec-proto/proto/`.
2. Tests: `python -B -m unittest discover -s arma_CWR/tests/ci -p "test_lobo_*.py"`;
   all existing 86 plus the new tests pass.
3. Preview on the installed mod (no writes):
   `python -B arma_CWR/tools/lobo/repair_lobo.py --mod D:/Arma_CWA/@LoBo --compatibility ud --check --report D:/Arma_CWA/tmp/lobo-repair-3/<label>/check-before.json`
   must exit 1 with 84 changed members in 41 archives and one skip line; with
   `--asset-source D:/LoBo-Reloaded/_work/inspect/hk/x/ofphk-175/TheHKpack.pbo`
   it must report 91 (84 + 7 added) and no skip line. Compare every
   `after_sha256` with section 9 and `spec-proto/report-ud-asset.json`; any
   difference must be explained before applying.
4. Apply: same command with `--apply --asset-source ...`. Then
   `--check` (with and without `--asset-source`) exits 0 with zero changes and
   no skip line.
5. `PoseidonTools.exe mod doctor D:/Arma_CWA/@LoBo` (report only, no `--fix`)
   still says Nothing to do (15 unreadable entries).
6. Independent verification: every archive hash in the new manifest; backups
   equal `before_sha256`; all untouched members of the 41 changed archives
   byte-identical; each added member equals its pin; bwm_boot3 length and
   trailer unchanged.
7. Classic: run the tool `--compatibility classic --check` on a scratch copy
   (not the installed mod) or rely on the family package run; expect exactly the
   16 fire scripts.
8. Engine parse: every changed root `config.cpp` re-extracted from the applied
   archives parses with `PoseidonTools config tojson` (use `spec-proto/cfgdiff.py`
   against a before/after dump).

## 8. Runtime phase checklist (UD on the Remaster data; nothing here was run)

- #9 (mandatory, decision 6): kill a LoBoM60 (M60Smoker), a LoBoTer vehicle
  (M1Smoker) and an empty LoBo_M35 truck or an A-4 (Smoker); confirm
  `count nearestObjects [wreck, ["Fire"], 3] > 0`, `inflamed` true, crackle
  audible, light visible, `say "fire"` heard, nothing visible at [0,0,0] or at
  the glow (no campfire mesh), the object disappears after 150 s; note the
  `Access denied ... lobo_airammo` WARN in a mission that does not activate it.
  Same for a LoBoEghelo helicopter wreck (KioGlowFire), including the burn
  message and damage within 10 m.
- #2: Sea King rests on water and the plate follows; destroyed or deleted Sea
  King removes its plate; BMP-1 doors, F-5 and A-4 canopies, MiG-25 gear and
  Dolphin arm still animate; script count stays flat over time.
- #3: AP mine walk/run/sprint, AT mine drive-over at speed, naval mine, flare
  tripwire walk/run/sprint on both sides, claymore; deleted mine produces no
  ghost explosion; flare helper gone after trigger and pickup.
- #4/#5: Mk-20 from A-4R, A-4RN, F-4 WW, IL-28 AS opens into bomblets; F-5
  dispenser model appears; Syrian MiG-21bis CAS1 RBK-500 releases 105 bomblets;
  no `Cannot create non-ai vehicle` or `Script ... not found` lines.
- #8: tail digits on F-15, F-16 (IAF and EAF), IL-28 (three digits), F-4 (three
  digits, afterburner flame not painted).
- Identifiers: an IDF Merkava gunner and an AH-1/AH-64 spot and engage at normal
  ranges; AA units prioritise LoBo aircraft; MH-60 and MLRS seated crew play the
  death move; the 'does not evaluate to a number' list no longer contains VAir,
  Varmor, CanSee*, SPEED_STATIC, RestDuty, WeaponHardMounted, LockYes,
  `Helicopter,` or `o` (VSoft, VArmor, Armor, ManActCoC_LAR7_DiverGetIn,
  ReadOnlyVerified, ReadAndCreate, LockCadet, flase are expected to remain).
- AG-1: preset `CoCM109MaxWind = 3` in init, then spawn a LoBo_M109A6_IDF:
  `CoCM109MinRange` is published and the script ends.
- Content: manned Boot3 placement no longer exits/crashes; hangar durability on
  Sinai; placed TMYK viaduct centre visible; LoBo_paraW visible.
- Weapons/textures: gear screen icons for MSG90, PSG1, UMP, MP5N/A3/K/Kpdw/K mag,
  R10/22, Glock 17; M4A1 LD M203 visible in hand; KSVK building impact sound; no
  `Cannot load texture bas_lbs` for the MD500D_R; Jordanian and Syrian crew
  textured at mid distance (visual check of the Syrian atlas tint).
- Savegames made before .3 keep the old compiled SQS lines; test with new
  missions.

## 9. Expected results (prototype, ud with asset source)

Archive hashes (first 16 hex digits; full values in
`spec-proto/report-ud-asset.json`). These hold only if the implementation is
byte-identical to the prototype, including the addition layout in 3.2.

| Archive | Before | After | Bytes | Changed + added |
| --- | --- | --- | --- | --- |
| BDE_mir3.pbo | 12dfee706a98cad5 | d21401b62ab0a6b9 | 10024301 -> 10024298 | 1 + 0 |
| bwm_boot3.pbo | 985b9297cf56ff60 | 78f93996eb307ae1 | 634853 -> 634853 | 1 + 0 |
| CoC_Arty.pbo | aba0604e1cedfc3b | 9fe6d11d5f3cc98c | 11648564 -> 11648447 | 3 + 0 |
| CoC_Torpedoes.pbo | 712a81ae705f7af9 | e05b6e4a5a5eb5a2 | 3169579 -> 3178617 | 1 + 0 |
| CSJairlines.pbo | 20fddcbd619341c9 | 0afebb84733fb09f | 6366974 -> 6380399 | 2 + 0 |
| CSJcitation.pbo | ddf6a8c930a79823 | 357923ad409c9dd9 | 1932081 -> 1941295 | 1 + 0 |
| CSJils.pbo | b11ce4856c201885 | 3924d8ec9cf86a2e | 737809 -> 737910 | 1 + 0 |
| JOF_Objects1.pbo | 38101029303ab3a2 | fd2e40af86db1171 | 3702071 -> 3702211 | 4 + 0 |
| LoBo_110.pbo | d6d64f11d9db9913 | c087fc0ccdc306de | 14075741 -> 14075724 | 1 + 0 |
| LoBo_A4.pbo | 1fc34e6e14266dbb | 25f03895dc7f66fb | 17064628 -> 17064676 | 4 + 0 |
| LoBo_airammo.pbo | e0881ae6f836f04b | 11afb781b3f8f2e7 | 8720512 -> 8721136 | 2 + 0 |
| LoBo_BTR60.pbo | 4481398236f11b9b | c2daf92e718ac717 | 40047457 -> 40047389 | 1 + 0 |
| LoBo_F4.pbo | 11f8649bb880500d | d266d9a125c2998f | 21130478 -> 21130561 | 3 + 0 |
| LoBo_FAB.pbo | ad6a0b9d0b6c7d15 | ab8b9f52abcde230 | 78726446 -> 78726459 | 6 + 0 |
| LoBo_Kfir.pbo | 8ec2b31451513668 | 472988e12161a35c | 10972601 -> 10972598 | 1 + 0 |
| LoBo_Mines.pbo | fcd1709e46b15d9a | df16a682a62fa52c | 6129358 -> 6129669 | 13 + 0 |
| LoBo_ZU23.pbo | 10c68d94e53a8343 | b5f15e88ced1b127 | 17732557 -> 17732548 | 1 + 0 |
| LoBoArt.pbo | 395ad5a03b1678ec | 00b2ef31849e8914 | 35953465 -> 35953241 | 4 + 0 |
| LoBoC130.pbo | 5a25748dee2669ce | 5b88bf344940c79a | 39880846 -> 39880843 | 1 + 0 |
| LoBoCen.pbo | 1513b1c07c6ebe3a | 94b17f998689c43d | 20161819 -> 20161814 | 1 + 0 |
| LoBoChallen.pbo | dd00c36758756d65 | 03a062a3ea6f3cd7 | 7303351 -> 7303346 | 1 + 0 |
| LoBoEgAPC.pbo | 28f9876b285db969 | 45c21ba836a70894 | 54744865 -> 54744279 | 1 + 0 |
| LoBoEghelo.pbo | a1f03a39c10e04d1 | cfee5139d89e332a | 62321366 -> 62321400 | 2 + 0 |
| LoBoF15.pbo | e10270d42d9dcc63 | a357669e730e75e1 | 46570978 -> 46571069 | 2 + 0 |
| LoBoF16.pbo | 1b943e8020573e46 | f6a6de457b2e1ad4 | 35889957 -> 35890190 | 3 + 0 |
| LoboF5.pbo | f655ff8fa541ab83 | 9d1c1b5ed3d7aa5e | 7267928 -> 7267926 | 3 + 0 |
| LoBoHizballCar.pbo | 5b4d08a2057213fb | 80b09f60de9cfbc7 | 15656565 -> 15656473 | 1 + 0 |
| LoboIDFhelo.pbo | 8f29c2ef147004a5 | 9d242dce75e3f5f9 | 96764743 -> 96764601 | 2 + 0 |
| LoBoIL28.pbo | f7a51c267a9cfef8 | fb01ec28a0a8e7e8 | 13959029 -> 13959245 | 4 + 0 |
| LoBoIs.pbo | 839d3c027304da50 | 796affe61b08bc20 | 50104134 -> 50104117 | 1 + 0 |
| LoBoJordan.pbo | bf7b6985b236f80f | 8afa5888f69f7c0f | 28826748 -> 28826748 | 1 + 0 |
| LoBoJorHelo.pbo | 09f9a4ed14700f3d | 13925c6e592e9511 | 51513351 -> 51513341 | 1 + 0 |
| LoBoMerkava2.pbo | 0341271e5b217519 | 6075c05276e57742 | 22012155 -> 22012146 | 1 + 0 |
| LoBoMerkava4.pbo | 3dcbf81919e73bb5 | 1b0516726697db3e | 10935912 -> 10935903 | 1 + 0 |
| LoBoMig.pbo | 4b2de152278af3e9 | ab4980a48981f9d6 | 67538984 -> 67538959 | 2 + 0 |
| LoBoSyrAPC.pbo | 607f0abbae6cd2eb | 331a1109371a8248 | 16054598 -> 16054619 | 1 + 0 |
| LoBoSyria.pbo | 12fc2a3ab14d311a | a5261e32be947138 | 28635473 -> 28635473 | 1 + 0 |
| LoBoWeapons.pbo | 3858b562d9373084 | 35f544c74e0a57cd | 162524376 -> 162706027 | 1 + 7 |
| MAP_OilAddon_script.pbo | e5dc6d8602e01823 | da709907895e2b3c | 113843 -> 113829 | 1 + 0 |
| RKSL-Hangers.pbo | 06d0c98d3398c822 | c89aa79520d1de44 | 1425806 -> 1429166 | 1 + 0 |
| TMYK_Bridges.pbo | 5d72d362912f4f75 | 7d118ca43f926e02 | 4130585 -> 4139255 | 1 + 0 |

Member hashes (first 16 hex digits): `spec-proto/member-table.md` (91 rows)
and the full values in `report-ud-asset.json`. Cross-checks with the
investigators: float `040259d5`, Dolphin `bc39564d`, MiG-25 `b490a9c4`, BMP
`8d07ebe6`, F-5 canopy `890e1ed8`, A-4 canopy `6feb2556`, F-15 number
`21760948`, F-16 `f0c64cb8` / `76433500`, IL-28 number `2fdb71e5`, F-4 number
`efdd0bfa` all match the values the investigators computed independently.

## 10. NOT being fixed in 2026-10-11.3, and why

Decided by the orchestrator:

- yk_ussol / yk_opfor / hyk_opfor model strings (LoBoJordan, LoBoSyria,
  LoBoHizballah): 0 of 1,182 raw hits are live (NUL-led fields, proxies,
  collision LODs); `syriangear.p3d` is an orphan. Not a gap (decision 3).
- @LoBo_Deps: OPGWC_EE11 `EE11.paa` icon and the BAS MH-60
  `optika_flir_gunner.p3d` `bas_lbs\m\optikatl.paa` (ODOL): third-party deps,
  handled by the orchestrator separately (decision 5). The EE11 copies inside
  LoBoJorAPC/LoBo_BTR60 are nested configs the engine never parses.

Needs a design call or runtime evidence first:

- CI-11 Car-derived `type=VArmor` / `type=Armor` (LoBoHMWV, LoBo_110, LoBoSufa,
  LoBoToy, LoBoTer, LoBoCars, LoBoRollers, LoBo_BTR60; 17 sites, 30 classes):
  authored value contradicts the soft base class and 2005 behaviour; making
  technicals and patrol jeeps "armor" makes riflemen ignore them. If forced
  later: BTR-60/BRDM-2 to 1, S1203 `Armor` to 0. Needs an AI ambush test.
- LoBoIs/LoBoEgypt `chkhalo.sqs:28` / `chkhaloplayer.sqs:17`
  `__LoBonewpara setvelocity` typo: fixing hands the canopy free-fall velocity
  at 150 m; hard-landing risk without an in-game test. File a new issue.
- Dolphin later-boarding player override (`?(player in _sub): goto "idle"`): AI
  behaviour change beyond #2.
- MP mine locality (every client runs every mine and creates its own global
  ammo/helper): needs a locality redesign and an MP test; .3 only removes the
  later ghost detonations. New issue.
- Optional UD preload of `LoBo_airammo` (section 6): decide after the runtime
  phase reports the WARN volume.

Warning-only or not a defect:

- CI-10 `type=VSoft` (107 sites in JAM_Magazines, LoBoIs, LoBoEgypt, LoBoJordan,
  LoBo_ZU23): nil already reads as 0 = VSoft; the engine warns once per spelling,
  so fixing a subset removes nothing. Deferred entirely.
- CI-12 `destrType` Destruct* spellings (89 sites, 20 pbos): resolved through
  the CONFIG.BIN variable table in every runtime; no warnings.
- CI-13 `access = ReadOnlyVerified/ReadAndCreate`: reads 0 (most permissive);
  restoring 3/1 could block later merges including the @LoBo_UD overlay.
- CI-14 world `cutscenes[]` (lost, lobo_iran, lobo_gaza, LoBo_Leb): no data value
  helps; engine side (UD 11655b4, CE PR #330).
- CI-15 inert references (commented f3wx markers, `setflagtexture` on a plane,
  unused LoBo_737Egyptair MLOD, non-root LoBoSA6\tex3 and LoBo_Zelda\scripts
  configs, `PilotCanSee`, JAM magazine/LoBoWeapat ammo `canLock`, coc_diver
  `ManActCoC_LAR7_DiverGetIn` which no data value can fix, ICPrpg7
  `#define WeaponHardMounted65536`), plus stock `LockCadet`/`flase` warnings
  from base-game addons.
- LoBo_KSVK CfgWeapons `hitGround/hitMan/hitArmor/hitBuilding[]` unquoted paths
  to missing files: weapons never read hit sounds (the ammo class does, fixed).
- Base-class redeclarations that disagree with vanilla (GWbuild1, RKSL
  `House: Strategic`), duplicate CfgPatches names, undefined `units[]` entries
  and CfgTextureToMaterial entries: log lines only.
- Mod-doctor model census (315 compressed + 880 MLOD + 1 empty ODOL not read by
  the doctor): no buried static prop; rope/cable models hang below origin by
  design.

Real but out of this batch's scope (each needs its own triage or issue):

- Broad unresolved-reference sweep: about 8,700 visual-LOD faces with
  unresolved live texture names in config-referenced MLODs (largest:
  LoBoWeapons `m16\m4_details.pac` x658, LoBo_Ruger_1022 `\datasupressor.pac`
  x162, LoboIDFhelo `Cable\recorde.pac` x280, LoboF5, LoBoHMWV, LoBoTer, LoBoF16,
  LoBo_A4, LoBo_M1) and about 70 other unresolved root-config
  picture/icon/model paths (LoBoMig MiG-29 model, canopy and seat; LoBo_airammo
  Mk81..84 LDGB and R-27 models; LoBoEgAPC, CoC_Arty, LoBoUSEmbassy, LoBo_Ob
  pictures). Candidates only, not individually verified; needs a scoped ledger
  (`textures-icons/sweep-cfgref-visual.txt`, `cfgrefs-lobo.txt`).
- Hizballah LOD0 single face `LoBoHizballah\wl\headband2.paa`: target ambiguous.
  Four tiny `pack_mic` faces with blanked names: invisible.
- `feuer.sqs` (LoBo_Mines and JOF_Objects1): flare-mine fire exits after one
  burst and `_lifeTick2` is never assigned. New issue.
- A-4 `firemissiles_a2/a3` `_weapon` dispatch compares weapon names against ammo
  names (cosmetic pylon texture on the wrong slot). New issue.
- LoBo_airammo `autosmoke1.sqs` (`~0.01` poll on every parked A-4, has an alive
  exit): a slow branch would help; outside #2.
- LoBo_M1 `minAngleX=--5;` (commander optics), F3WX_O1 `radius = 2,0;`,
  LoBoWeapNad `simulationStep=0,100000;`, CoC_Torpedoes `initSpeed=1` without
  `;`: parse-tolerated oddities whose evaluated values need checking. New issue.
- CoC_Torpedoes `coctorpreg.sqf` registers `ShkvalNucPB` (not `CoC_ShkvalNucPB`)
  and `init.sqs` leaks one EmptyDetector per carrier; CSJ Citation `500hit.sqs`
  (4 s forever), `csj500eng.sqs` contrails at altitude, `500GearInt.sqs`
  undefined `_aircraft`; CSJils actions with radius 80000: real but negligible
  or unused by LoBo.
- Undefined created classes on rare or dead paths: `BAS_60fire`/`BAS_60smoke`
  (LoboIDFhelo `uh60\MPFX\mpfire.sqs`, after a sling-load drop),
  `EffetTerre`/`EffetEau25` (unreferenced duster.sqs), LoBoF15 `scr\cluster.sqs`
  (`RKTF15CBUBomblet`, orphan), LoBo_airammo `duranfired.sqs` (`.sqs.sqs`,
  uncalled), LoBoJorHelo `uh60\fx\fastins.sqs` (the #7 typo, nested config never
  loaded).
- Dormant frame loops: LoBo_FAB `waves.sqs`/`wind.sqs` and LoBoC130
  `turbosound.sqs` (unreferenced), LoBo_A4 `damage.sqs` (unreferenced),
  LoBo_FAB `dolp\iscapt.sqs` (unreachable); LoBo_FAB `place*.sqs` per-frame
  attach loops are the pre-attachTo mechanism and must stay.
- Other dead-path number scripts: LoBo_F4 `number2.sqs`, LoBo_FAB
  `saar5\scripts\number*.sqs` and `scripts\number*.sqs`, LoBoCen
  `scripts\number.sqs` (never started); `ohp\scr\Onumber.sqs` is correct.
- Deploy TOW-2 actions call a missing `deployTOWB.sqs` but their condition
  needs an undefined `LoBo_TOWWpnB`, so they never show.

Engine-side, not content:

- `EntityAI::_hiddenSelectionsTextures` is not serialized: tail numbers and all
  LoBo pylon/afterburner textures vanish after loading a save. Candidate UD
  engine issue.
- Savegames keep compiled SQS lines (`Script::Serialize`): scripts already
  running in an old save keep the pre-.3 behaviour. No migration.
- MP determinism of random tail numbers (local `setObjectTexture`): design
  change.

Rejected alternatives (recorded so they are not re-proposed):

- Same-length byte patches from the issue texts (mines `~0.25 `, number
  `_unit  =`, rockeye comment trimming): unnecessary, `Archive.replace` repacks,
  and several would leave defects (0.25 s breaks the flare tripwire; the number
  patch leaves the IL-28/F-4 slots wrong).
- Alias classes (`LoBoMk20Deployed`, `LoBoRBKBomblet`) and rockeye copies in
  LoBoF15/LoBoF16: renames and the consolidated path are cleaner.
- Shipping the whole TheHKpack.pbo inside the core @LoBo (243 classes,
  re-bodies HandGunBase and G36aBase shared with JAM_Magazines/LoBoWeapons; it
  ships separately in the optional @LoBo_Extras) or using
  `\LoBoWeapons\Tex\w_mp5k.paa` / `GearPics\mp5gear.paa` substitutes.
- Repointing the helicopter scripts to `LoBo_60fire` (section 2.2); switching
  wreck scripts to vanilla `"Fire"` (visible campfire mesh with collision);
  defining the classes in `@LoBo/Bin/config.cpp` (UD-only and regenerated).
- PosDistance-based flare tripwire (UD-only command, breaks Classic).
- `D:/SteamLibrary/.../Remastered/@LoBo`: a separate, unrepaired copy in the
  read-only Remaster tree; never touched.
