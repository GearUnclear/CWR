# Reproducible Lobo content repairs

`repair_lobo.py` implements the static audit originally filed as CWR #69, now
tracked internally at [Lobo #1](https://github.com/GearUnclear/lobo/issues/1).
It changes installed addon members only after every repair has passed preflight.
No game is launched. Only transformation code, synthetic tests and documentation
belong in this source repository; addon data and backups stay local.

## 2026-10-11.3 batch

Implemented, applied to the installed UD profile and offline verified on
2026-10-11. In game, LoBo #4, #5 and #8 are runtime verified, #9 is partly
runtime verified, and #2 and #3 are offline verified only (see "Runtime
results" at the end of this section). Nine fail-closed rule modules run after
`followups`. Every rule accepts the original, repaired or partially repaired
state only, is idempotent and is profile-neutral (OFP 1.x SQS commands and
plain config literals; every vanilla asset it names exists in Classic 1.99 and
the Remaster).

- `loops` (LoBo #2; audit items AG-1, AG-3, AG-4, AG-10): vehicle-lifetime
  scripts that looped every frame now wait. The Sea King float plate follows at
  0.1 s and is deleted with its helicopter (a destroyed Sea King is no longer
  held on the water); the Dolphin arm, MiG-25 gear, Syrian BMP-1 doors and F-5
  and A-4 canopies poll at 0.5 to 1 s, and the A-4 canopy and Dolphin arm stop
  with their vehicle. The CoC artillery MaxWind guard in LoBoArt and CoC_Arty
  jumps to `#WindSet` like its sibling scripts instead of spinning on
  `#PrecisionSet` when the wind limit is preset. LoBo_FAB flag loops poll once
  per second (flag ownership is released up to 1 s after the driver leaves),
  CSJ airliner night lights stop for a dead or shut-down aircraft, and the CSJ
  ILS helper polls once per second.
- `mines` (LoBo #3): LoBo_Mines and JOF_Objects1 mines poll every 0.1 s (the
  flare tripwire every 0.03 s) instead of every frame and end with their mine,
  so a deleted mine can no longer fire a ghost explosion. The flare mine deletes
  its helper object on trigger, pickup and deletion.
- `clusters` (LoBo #4, #5): the A-4R, A-4RN, F-4 WW and IL-28 AS Mk-20 scripts
  call the consolidated LoBo_airammo rockeye script (the IL-28 fired script also
  gains the .1 projectile owner guard), the F-5 and IL-28 rockeye copies create
  the defined `LoBo_mk20Deployed`, and the RBK-500 script creates the defined
  `LoBoRBK500Bomblet`.
- `numbers` (LoBo #8): F-15, F-16 (IAF and EAF), IL-28 and F-4 tail numbers get
  the form .1 gave the landing craft. Digit slots follow each model's
  `hiddenSelections[]`; the F-4 afterburner flame (index 4) is no longer painted.
- `effects` (LoBo #9): `M1Smoker`, `M60Smoker`, `Smoker` and `KioGlowFire` were
  defined nowhere, so the owner-side wreck glow returned objNull (no light, fire,
  crackle or `say`). LoBo_airammo's config now defines them as
  `LoBo_WreckGlow`, an invisible vanilla `Fire` (model `empty`). KioGlowFire
  shares this mechanism instead of repointing the helicopter scripts to
  `LoBo_60fire`. Consequence: defining KioGlowFire makes the BAS helicopter
  wreck burn live for the first time. Before .3 the glow was objNull and
  `distance` to it returned 1e10, so the burn line never fired. As authored it
  would deal +0.04 damage every 0.25 s within 10 m of the glow, with no on-foot
  test (a seated player's position is the vehicle's, so a crew parked next to a
  crashed helicopter would die in about 6 s) and a warning that never reset.
  The five helicopter `fire.sqs` copies (LoBoEghelo `Engine`, `Engine2`,
  `Engine3`; LoboIDFhelo `CH53\Engine`, `UH1\Engine`) therefore get the ground
  scripts' burn policy: one scheduler on the wreck owner, an on-foot, local and
  living player only, measured to the wreck, at most once per 10 s. The authored
  0.04 per hit stays. Known side effect: the glow is a vanilla Fireplace, so a
  soldier on foot within 5 m who faces it is offered "Put out fire" (then "Light
  fire"). Putting it out stops the light and crackle; the script's particles and
  `say` go on until the 150 s cleanup. No config key suppresses that action, and
  it has not been tried in game yet. Activation caveat: the owner addon is
  `lobo_airammo`. When a mission has not activated it, UD logs one
  `Access denied ... not activated` warning per glow and still creates the
  object (run `glow-unlisted-01`). The official 3.05 exe is expected to behave
  the same but was not run, and stock 1.99 behaviour is unverified. Glow
  creation and the killed-handler path are runtime verified; the burn, the
  action, the 150 s cleanup and the `say` sound are not, so #9 stays open for
  those checks.
- `identifiers` (config audit CI-1, 2, 3, 4, 5, 7, 8, 9): undefined, misspelled
  or quoted identifiers that read as 0 become the literal their BIS define
  gives: crew perception (`CanSee*`), aircraft kind `VAir` = 2 (66 sites),
  armor kind `VArmor` = 1, crew move states (`SPEED_STATIC` = 1e10,
  `RestDuty` = -1), `WeaponHardMounted` = 65536, `LockYes` = 2, the ZU-23
  `UnitInfoType` and `hasCommander=o`. The ZU-23 token `Helicopter,` always
  evaluated to nil, read as 0 (the soldier panel, also the vanilla convention
  for static guns such as M2StaticMG), so CI-8 writes the final value
  `UnitInfoType=0;` and the gunner HUD does not change. The first applied draft
  wrote 1 (the tank panel); the rule accepts that state and corrects it.
- `content` (AG-5, AG-6, AG-11, CI-6): RKSL hangars read their authored
  `armor` (intended balance change: 1000/2000 instead of Building 150), the TMYK
  viaduct centre names its existing model and icon, LoBoIs parachutists use the
  vanilla `para` model, and the bwm_boot3 crew is `SoldierWB`, so the core mod
  no longer exits or crashes on BWMod's undefined `BWMOD_Soldier`.
- `weapons`: the R10/22 and Glock 17 icons, the M4A1 LD M203 model (its parent
  model) and the KSVK building-hit sounds (the vanilla ricochets) name existing
  files, and seven missing HK inventory icons are added to LoBoWeapons.pbo
  (below). The MSG90 picture moves from the authored `\TheHKpack\` bank to the
  LoBoWeapons copy only when that icon exists after the run. Without it the
  authored path stays, because `@LoBo_Extras` ships TheHKpack.pbo (no prefix)
  and resolves it.
- `textures`: the MD500D_R collision, fire and hitpoint LODs, the Jordanian crew
  LOD 5.0 body armor and the Syrian crew LOD 5.0 overflowed names point at
  shipped textures. Only fixed-width face texture fields change; file length
  and every other byte stay the same.

HK icons: `w_msg90`, `w_psg1`, `w_ump`, `w_mp5n`, `w_mp5k`, `w_mp5kpure` and
`m_mp5n` come from OFP: HK ("The HK Pack") 1.75 by the LoBo Team and
Vipersheart, 2002. LoBo referenced these icons but never shipped them in
LoBoWeapons. They are copied byte for byte from a user-supplied
`TheHKpack.pbo` whose seven member SHA-256 values are pinned in
`repair/weapons.py`; the icon bytes are never stored in either repository.
Both applied batches used the original 1.75 archive as the asset source
(SHA-256 `79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5`,
recorded as `asset_source_sha256`), and all seven added members equal their
pins.

Framework: `Archive.replace` can append members (only from a module's declared
`ADDED_MEMBERS`, never on StuffPBO archives) and patches StuffPBO archives in
place for same-length uncompressed members (see below). The MLOD reader accepts
any 32-byte Oxygen default-path trailer (`H:\Codemast` in LoBo_MD500D_R.p3d).
Archives with the Windows read-only attribute (JOF_Objects1, MAP_OilAddon_script,
RKSL-Hangers, TMYK_Bridges) are replaced and keep the attribute; the first apply
attempt stopped on JOF_Objects1 and rolled back cleanly (batch
`20261012T034210Z-8eb814a2`, state `rolled-back`, all 124 archives unchanged).
That folder duplicates the originals in `20261012T034354Z-0532e837` and is not a
recovery batch (see below); the family package copies only `applied` batches
into its sources. Additions are planned before the text rules, so a rule can
depend on whether its target member will exist (the MSG90 picture).

Result on the installed mod: 89 changed and 7 added members in 41 archives, in
two applied batches. `20261012T034354Z-0532e837` applied the first draft (84
changed and 7 added members; all 41 archive and 91 member hashes equal the spec
prototype). Review then changed the ZU-23 `UnitInfoType` (0 instead of the
draft's 1) and added the five helicopter burn guards; `20261012T042613Z-b68ba5de`
applied those 6 members in 3 archives (LoBo_ZU23, LoBoEghelo, LoboIDFhelo; 1,336
untouched members byte-identical, the other 121 archives unchanged). The final
rule SHA-256 is
`75c456dcab6fbece460edd479eedc296253255e709a3c58260f28072562a0427`, recorded in
the `b68ba5de` manifest and equal to the source fingerprint at verification
(the `0532e837` manifest records the draft's `81bf8fe8...`). A further check
with and without `--asset-source` exits 0 with zero changes and no skip line;
a Classic check of the installed mod lists only the 16 wreck-fire members
in the same 14 archives; report-only doctor says Nothing to do (10 unreadable
entries, down from 15 because five compressed configs are now stored
uncompressed). From the pristine originals of the applied batches, .1, .2 and .3
compose to 160 changed and 7 added members in 49 archives, each equal to the
installed bytes. Independent verification of the first batch checked the
manifest, backups, 8,013 untouched members, the pinned icons and the bwm_boot3
length, header and trailer. All 35 changed configs parse with the engine parser
before and after. 187 synthetic LoBo repair tests pass (86 before this batch;
241 with the 54 `@LoBo_Extras` tests), including literal pins of every rule
value (`test_lobo_rule_literals.py`). Evidence stays local under
`D:/Arma_CWA/tmp/lobo-repair-3/`.

Runtime results, 2026-10-11: the UD engine (`dist/x64-win-rwdi`) ran on the
official Remaster data (`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered`)
with the installed, repaired `D:/Arma_CWA/@LoBo` mounted by absolute path. The
probes are under `runtime/repair3/` (see `runtime/README.md`). Each final run
passed on its first attempt with `-j 1`, ran with `--autotest` (a LoBo script
error would have aborted it; none was logged), and closed every game it
started. The repair check before and after the runs exits 0 with zero changes,
and the Remaster folder is unchanged.

| Issue | Level | Run | What the run showed |
| --- | --- | --- | --- |
| #4 Mk-20 | runtime verified | `mk20-03` | A-4R, F-4 WW and IL-28 AS each opened one `LoBo_mk20Deployed` and released at least 194, 194 and 187 of 197 bomblets. |
| #5 RBK-500 | runtime verified | `rbk500-01` | The Syrian MiG-21 CAS1 dispenser opened at about 47.6 m (release at 50 m) and at least 100 of 105 `LoBoRBK500Bomblet` were seen. |
| #8 tail numbers | runtime verified | `numbers-01` | Two copies each of F-15C, F-16 IAF, F-16 EAF, F-4 and IL-28 show visible numbers that differ within each pair; no script error or `Unknown label` in a 1.2 million-line debug log. |
| #9 wreck glow | partly runtime verified | `glow-01`, `glow-unlisted-01` | All four classes create objects. The killed handlers of a Toyota (`M1Smoker`), a Merkava2 (`M60Smoker`) and a Mi-8 (`KioGlowFire`) each made exactly one burning glow near the wreck. With `lobo_airammo` not activated, each glow logs one `Access denied` warning and is still created. |
| #2 frame loops | offline verified only | none | |
| #3 mine polling | offline verified only | none | |

Not run yet: for #9, a player seated in a vehicle next to a burning helicopter
wreck (no burn), a player on foot within 10 m of it (one MEDIC line, 0.04
damage at most once per 10 s), the "Put out fire" action at a wreck glow, the
150 s cleanup and the `say "fire"` sound (the wreck screenshots show only
smoke). For #4 and #5 only the script path was tested: the aircraft was held
still and invulnerable at 150 m, so ballistics, spread and damage are
unchecked, and the bomblet counts are lower bounds from a check about every 10
frames. The F-5 and A-4RN Mk-20 paths were not flown. Problems the runs found
that .3 did not cause: LoBoIL28 `script\tailgun.sqs` stops with "Zero divisor"
when an IL-28 has crew aboard but no pilot, and the logs report missing
`lobomig\soundss\jet_crash.wss`, `loboter\toy\t\black_matte.pac`,
`lobo_pilot\eaf\tex\back.pac`, `rktf5\swtex.paa` and
`lobof16\iaf\backfuse.pac`.

Savegame caveat: SQS scripts already running in a save keep their old compiled
lines, and the engine does not save tail numbers or other `setObjectTexture`
results; test with new missions. A second, unrepaired LoBo copy sits at
`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered\@LoBo`; a
game that mounts that copy gets none of .1, .2 or .3.

## 2026-10-11.2 follow-up batch

Implemented, applied to the installed UD profile and offline verified on
2026-10-11 before the 13:30 PDT package cutoff:

- LoBo #6: AP mine pickup now references the existing AP_Mine1.sqs.
- LoBo #7: IDF UH-60 Rapid Insertion now references the existing FX/fastins1.sqs.
- LoBo #10: MiG-23 wing sweep clamps the local _fraction to 0..1 using digit 1.

The follow-up module rejects missing/empty destinations and unknown or duplicate
source anchors, accepts a partially repaired clamp, preserves unrelated bytes
and is idempotent. It changes three members across LoBo_Mines.pbo,
LoboIDFhelo.pbo and LoBoMig.pbo. All 86 synthetic LoBo tests pass. A second
installed repair check exits 0 with zero changes; report-only doctor says
Nothing to do (15 existing unreadable entries). Independent verification checks
all 124 archive hashes, verified backups and all 1,360 untouched members in the
three changed archives. Full evidence stays local under
`D:/Arma_CWA/tmp/lobo-runtime-acceptance-20261011/repair-2-*`.

No game was launched for this follow-up. These three source defects are corrected,
but their gameplay actions/animation are not runtime verified. Issues #2-5 and
#8-9, including wreck emitters requiring an in-game check, remained outside this
bounded batch; 2026-10-11.3 above handles them. No new Classic compatibility
behavior was added.

## Use

Run from the CWR repository root with Python 3.10 or newer:

```powershell
# Preview only; writes the optional report, not addon files.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --report tmp/lobo-preview.json

# Apply to the UD installation, with verified original backups. The asset source
# supplies the seven HK inventory icons LoBo referenced but never shipped.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --apply --asset-source 'D:/path/to/TheHKpack.pbo' --report tmp/lobo-applied.json

# A second pass must report zero changed members and exit 0.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --check

# Broad diagnostics; candidates require review, not automatic repair.
python arma_CWR/tools/lobo/audit_lobo.py --mod '@LoBo' --report tmp/lobo-audit.json

# Synthetic offline regression suite, no installed data required.
python -m unittest discover -s arma_CWR/tests/ci -p 'test_lobo_*.py'
```

Pass `--tools PATH` for another standalone PoseidonTools build. The Windows
default is `arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe`. Compressed members
are decoded through its `pbo show` command, with archive snapshot checks around
the read. The selected repair archives in the audited installation need no
compressed-member decoding.

`--asset-source PATH` names a user-supplied `TheHKpack.pbo` 1.75: the original
archive (SHA-256 `79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5`)
or the `@LoBo_Extras` copy, which differs only in its patched `config.bin`. A
source is accepted only when all seven icon members are present, stored
uncompressed and equal to their pinned SHA-256 values; anything else is
rejected, even when nothing is missing. Without the option the missing icons
are not added: the summary lists them under `skipped`, stderr prints a `WARN:`
line, the skip alone does not make `--check` fail, and the MSG90 picture keeps
its authored `\TheHKpack\` path (a repoint made earlier without the icon is put
back). Icons already present must carry the pinned bytes. The JSON summary also
reports `added_members` and, when given, `asset_source_sha256`.

The repair exits 2 on incompatible input or I/O failure. `--check` exits 1
when repairs would change data, 0 when already repaired. The default preview
exits 0 for a valid plan. The audit's `--strict` exits 1 for unresolved references
**or incomplete source-parser coverage**; ordinary candidate warnings alone do
not make it fail.

Reinstalling original Lobo data requires rerunning the repair. Run the existing
`PoseidonTools mod doctor <mod> --fix` maintenance separately for scope keywords,
malformed floats and buried wreck origins. This new repair does not replace it.

## Compatibility profiles

Use **`--compatibility ud`** (default) for Uslu dur!: wreck fires use the engine's
`nearestObjects` command to discover all nearby on-foot infantry, including
remote victims. One scheduler on the wreck owner applies damage through the
engine's network-aware `setDammage` path. The local player is excluded from that
query and handled once separately. This is verified against source behavior;
multiplayer execution has not been tested.

Use **`--compatibility classic` on a separate Classic mod copy before packaging**.
Classic fire scripts discover the player's and wreck's groups, de-duplicate them
and filter for distance, living units and on-foot state. This fixes the undefined
function and invalid glow state using older commands, but cannot discover every
unrelated group. It is a deliberately limited compatibility fallback, not full
nearby-infantry parity. Neither profile creates a trigger to perform this query.

Do not redistribute the UD profile as a Classic-compatible repair: Classic 1.99
does not provide `nearestObjects`. The newer `createTrigger`/`setTriggerArea`
commands are also unsuitable as an assumed Classic PC fallback. Official remaster
versions need their own API/runtime verification. Existing package tooling does
not automatically select this repair profile; prepare its source copy explicitly.
Switching the two profiles updates only the 16 wreck-fire queries and creates a
new backup batch; switching back is reproducible.

## Repair scope and policy

| Audit item | Implemented change |
| --- | --- |
| Missing M833 projectile | Defines the existing magazine's projectile at an explicit 400-point baseline below M900; no alias or identifier rename. |
| MG engagement | JAM-derived role profiles; separates MG and rifle 5.56 use and corrects the suppressed Negev magazine. |
| Unsupported armor | Uses the six supported tank components; repairs missing model bindings and hull/turret membership. |
| Aircraft labels | Corrects MiG-21, MiG-25 and F-5 targets; preserves pylon slots. |
| Micro Uzi | Corrects the HD magazine reference. |
| Landing craft | Repairs vehicle variable, digit labels and unused parameter access in both numbering scripts. |
| Wreck fires | Initializes the query; valid sound anchor on the reachable owner path, guarded glow lifetime and one damage scheduler. The non-owner `_glow = _object` branch is unreachable (the killed handler runs on the owner) and could not `say` on a destroyed object anyway. The glow classes themselves were undefined until 2026-10-11.3. |
| Divers | Uses the verified bundled swimmer model for both missing outfits, retaining roles/loadouts. |
| MG splash | Explicitly removes indirect damage from ordinary ball/AP rounds and preserves explosive families. |
| Blast policy | Bounds the 60 mm and vehicle-bomb plateaus while retaining contact strength and heavy-mortar baselines. |
| Mk82 family | Shares the existing Mk82 warhead values with GBU-12, retaining guidance controls. |
| Missile helpers | Deletes 41 launch terrain helpers immediately after their final read; guards projectile ownership. |
| Display text | Repairs Assault, Missile, Gauge and 122 mm strings without renaming classes. |
| Offline checks | Reference, label, local-call and custom-hitpoint diagnostics; explicit incomplete-coverage reporting. |

See [AMMUNITION-POLICY.md](AMMUNITION-POLICY.md) for all tuning choices and the
144 mathematical blast scenarios; these are not measured lethal radii or
shots-to-kill results. See [ARMOR-MODELS.md](ARMOR-MODELS.md) for actual model
selections and preserved script-facing names. The repair implements supported
component coverage, not a new directional penetration simulator.

## Backups, integrity and recovery

Every apply batch stores complete original archives and `manifest.json` under
`<mod>/.ud-backups/issue-69/<UTC-time>-<unique-id>/`. The historical directory
name remains stable after the issue transfer. The manifest records repair
version, source-rule SHA-256, profile and before/after archive/member hashes.
Reports can be stored under the checkout's ignored `tmp/` directory.

All new archives are staged and validated before originals are backed up or
replaced. Non-target member bytes, ordering, packing and metadata are preserved.
SHA-1 trailers are verified/recomputed; legacy textual signatures are retained.
Old StuffPBO archives (17-byte trailer: two bytes of an unknown legacy checksum,
then `StuffPBO V1.0b`) accept only same-length replacements of uncompressed
members, overwritten in place; the header table and trailer stay verbatim, as
with mod doctor's in-place patches. No StuffPBO reader exists in the UD or
Remaster engine source and OFP 1.99 does not validate PBO trailers, so the stale
checksum is ignored by every supported runtime; only StuffPBO's own tooling
could notice. Length changes, compressed members and added members are still
rejected there. Added members (2026-10-11.3) are appended after the existing
records and payloads with a null `before_sha256` in the manifest; restoring a
batch's original archives removes them again. Archives with the Windows
read-only attribute keep it. If that attribute cannot be put back after a
successful rename, the archive still counts as replaced (or restored), stderr
prints a `WARN:` line and the manifest lists it under `attribute_warnings`.
Unknown source values, duplicate/unsafe members, incomplete inputs and
conflicting edits fail before installation.

An interrupted multi-archive install attempts to restore every file it changed.
Rollback preserves any later concurrent edit, records conflicts/failures and
keeps the originals. Replacing a whole mod directory is not one filesystem
transaction; inspect the manifest if power loss interrupts the process.

For deliberate recovery, stop using that mod copy, verify each current archive
matches the manifest's `after_sha256` and each backup matches `before_sha256`,
then restore that batch's originals. If a hash differs, review the later change
instead of overwriting it. Restore batches in reverse order when reverting
multiple repairs, and restore only batches whose manifest `state` is `applied`:
a `rolled-back` batch never changed the installed archives (its `after_sha256`
values were never installed) and its originals duplicate those of a later
applied batch. Keep backups out of both GitHub repositories.

## Validation limits

The lightweight source linter intentionally fails closed on malformed legacy
syntax. The audited pack has five configs it cannot fully parse (`LoBo_FAB`,
`LoBo_IAk47Iraqi`, `LoBoChallen`, `LoBoEgypt`, `W_CombatStatics`). It reports them
and downgrades unresolved references to uncertain candidates because skipped
files may contain those definitions. PoseidonTools' legacy config exports and
the mounted static probe are separate evidence; a successful export can recover
old syntax and is not proof the source was well formed. No braces are invented
by the repair to silence this report.

Label/local-variable scans also cannot prove reachability or general dataflow.
The broad candidate list is not a list of confirmed gameplay defects. The
targeted repair validates all changed script labels and known variable state,
model selection structure, expected ammunition values and repeatability. The
35 configs the 2026-10-11.3 batch changes were also parsed with the engine's
own preprocessor and parser (PoseidonTools `config tojson`) before and after;
the parsed difference contains only the intended keys.

The repair tool itself launches no game. Runtime evidence comes from separate
probes: the issue #1 acceptance runs and the 2026-10-11.3 probes, both recorded
in [runtime/README.md](runtime/README.md). Runtime combat, all missile shots, AI
weapon selection, animated hit locations, saves and multiplayer still need a
separately authorized play test. All peers should use the same repaired content.
