# Reproducible Lobo content repairs

`repair_lobo.py` implements the static audit originally filed as CWR #69, now
tracked internally at [Lobo #1](https://github.com/GearUnclear/lobo/issues/1).
It changes installed addon members only after every repair has passed preflight.
No game is launched. Only transformation code, synthetic tests and documentation
belong in this source repository; addon data and backups stay local.

## 2026-10-11.4 batch

Implemented, applied to the installed UD profile in three batches, offline
verified and runtime verified on 2026-10-12 for
[LoBo #13](https://github.com/GearUnclear/lobo/issues/13),
[LoBo #14](https://github.com/GearUnclear/lobo/issues/14), the pilot wounds
below, the four LoBo #9 items the .3 runs left open, and the restored and
repointed files of the third and fourth parts, all on the UD engine (see
"Runtime verification" at the end of this section). The official Remaster exe
(3.05) passed a menu boot and an in-mission smoke test with the LoBo Reloaded
2026-10-12 package, built from the final .4 rule (see "The official Remaster
exe and the 2026-10-12 package"). Its first attempt, on an earlier, unpublished
2026-10-12 build, found two package defects that the 2026-10-11 package also
carries: a Classic wreck-fire query that only UD accepts (fixed by the third
part below) and a malformed float in `@LoBo_Deps` (fixed by the packager's new
doctor pass). Five fail-closed rule modules are new and run last, after
`textures`, in this order: `tailgun`, `references`, `repoints`, `wounds` and
`restore`; `scripts` changes its Classic query. The new modules are
profile-neutral: they write OFP 1.x SQS (`isNull`, `driver`, `count`), config
string literals, texture names, plain DXT1 textures or unchanged members of
older LoBo releases, so the UD and Classic plans give the same bytes and the
package's Classic switch leaves these members alone. Only the `scripts` change
differs by profile, and its UD output is unchanged.

- `tailgun` (LoBo #13): LoBoIL28 `script\tailgun.sqs` runs 0.1 s after an
  IL-28 spawns. It skipped gunner creation only for an empty plane, so a plane
  with a gunner or cargo but no pilot built the turret gunner from
  `group driver`, got an empty group and stopped with "Zero divisor" (fatal
  under `--autotest`). TG-1, the issue's option 1, replaces
  `? count (crew _plane) ==0: goto "crtGun"` with
  `? isNull (driver _plane): goto "crtGun"`. Without a pilot the script now
  takes the existing empty-plane path (unmanned turret) and never calls
  `createunit` into a null group. TG-2, option 2, inserts
  `? count _tunits == 0: goto "crtGun"` before the failing `select`, for a
  driver seat that holds a unit without a group; a living driver's group always
  contains the driver, so it is unreachable in normal play. The authored
  dead-pilot bailout line and both eject lines are pinned unchanged, so a
  pilotless IL-28 above 20 m still ejects the plane's gunner, as it does for a
  dead pilot. Unchanged limitation: the script runs once, so a pilot who boards
  later never gets a turret gunner.
- `references` (LoBo #14), text rules: `soundLandCrash[]` of the LoBoMig
  `LoBo_E_mig21bis_MR1` names `\LoBoMig\sounds\jet_crash.wss`. The folder was
  misspelt `soundss`; `soundCrash[]` on the line above already named the
  shipped file. In LoBo_Pilot `wounds[]`, five healthy-texture entries whose
  `.pac` never shipped name the shipped `.paa`: EAF `back` and `legsbk`, RJAF
  `back` and `legsbk`, and IAF `tex2\legsbk`. The three IAF pairs whose `.pac`
  ships stay as authored. UD's `WoundInfo::Load` finds the healthy texture by
  name in the shape, so these five pairs never matched a face. Effect, runtime
  verified in `refs-01` and `wounds-01` (below): wounded EAF and RJAF pilots
  show back and leg-back wounds for the first time, and 69th Squadron IAF
  pilots the leg-back one (their back pair already matched, because
  `lobo_iafpilot_grey.p3d` names the shipped `IAF\tex2\back.pac` on 217
  faces). The wounded half of the EAF and IAF `tex2` pairs names the
  suit-coloured textures of `wounds` (below); RJAF keeps the shipped olive
  `_d` art, which matches its olive suit. This rule's match stops before the
  wounded name's extension (`...\back_d`), so it accepts the shipped `_d.paa`
  and the `_dr.paa` that `wounds` writes later in the same plan. Each of the five pairs is counted
  inside the class that authored it (`LoBo_Egypt_Pilot`,
  `LoBo_IAF_Pilot_grey`, `LoBo_Jordan_Pilot`), because `wounds` also writes
  the repaired `IAF\tex2\legsbk` pair into the `grey2` and `grey3` classes;
  an unrepaired `.pac` pair anywhere else in the config still fails closed.
- `references`, model repoints (fixed-width face texture fields; file length
  and every other byte unchanged). LODs 3.0 and 5.65 of the Hilux and Hilux MG
  models name `LoBoTer\Toy\t\black matte.pac`, the name their LODs 0, 10 and
  1100 already use (`black_matte.pac` never shipped). LODs 1.0 and 5.0 of the
  EAF pilot (`lobo_epilot.p3d`) name the EAF `back.paa` and `legsbk.paa` that
  its LODs 0.15 and 1100 use. The Geometry LOD of the AIM-9 (`lobo_aim9.p3d`)
  names `LoBo_airammo\tex\sw.pac` instead of `rktf5\swtex.paa` from a lost
  addon.
- `wounds` (user decisions 2026-10-12: suit-coloured blood, then wounds for
  every pilot class; applied in the second batch and runtime verified in
  `wounds-01`): every LoBo pilot shows body wounds in its own suit with red
  blood. A wounded soldier swaps each body
  texture named in its class's `wounds[]` for the paired `_d` texture; UD's
  `WoundInfo::Load` keeps only pairs whose healthy name (extension included)
  is a face texture of the model. LoBo ships one olive wounded set: `back_d`,
  `legsbk_d`, `front_d`, `armle_d`, `armri_d`, `legsfr_d`, `legslft_d` and
  `legsrgt_d` are byte-identical in every folder that has them, so a wounded
  pilot in a non-olive suit turned olive. Olive suits keep that shipped set.
  Each non-olive suit gets new members next to the shipped files, named
  `<part>_dr.paa` (no shipped texture is overwritten). Inventory (each class
  includes its downed `_cap` subclass, which inherits its model and
  `wounds[]`; `LoBo_Syria_Pilot` has none; no other LoBo addon defines a class
  on these models). Body textures are the eight parts with a `_d` texture:

  | Class | Model, body textures | Suit | Before `wounds` | After |
  | --- | --- | --- | --- | --- |
  | `LoBo_Egypt_Pilot` | `lobo_epilot`, `EAF\tex` | reddish brown | 8/8 paired, olive `_d` | 8 new `EAF\tex\*_dr.paa`, 8 pairs repointed |
  | `LoBo_IAF_Pilot_grey` (69th Sqn) | `lobo_iafpilot_grey`, `IAF\tex2` | grey | 8/8 paired, olive `_d` | 8 new `IAF\tex2\*_dr.paa`, 8 pairs repointed |
  | `LoBo_IAF_Pilot_grey2` (106th Sqn) | `lobo_iafpilot_grey2`, `IAF\tex2`, left arm `IAF\tex3` | grey | 1/8 paired (left arm, olive `_d`), no face entries | own full array: faces, 7 `tex2` pairs on `tex2\*_dr.paa`, `tex3\armle` on new `tex3\armle_dr.paa` |
  | `LoBo_IAF_Pilot_grey3` (133th Sqn) | `lobo_iafpilot_grey3`, `IAF\tex2`, left arm `IAF\tex4` | grey | 0/8: its one pair named `tex3\armle`, no face entries | own full array: faces, 7 `tex2` pairs on `tex2\*_dr.paa`, `tex4\armle` on new `tex4\armle_dr.paa` |
  | `LoBo_IAF_Pilot` (101st Sqn) | `lobo_iafpilot`, `IAF\tex` | olive | 8/8 paired, shipped `_d` | unchanged |
  | `LoBo_IAF_Pilot2` (253rd Sqn) | `lobo_iafpilot2`, `IAF\tex`, left arm `IAF\tex5` (same bytes as `tex\armle`) | olive | 7/8: inherited `IAF\tex` pairs, no left-arm pair | own array: faces, 7 `IAF\tex` pairs, `tex5\armle` on the shipped `tex5\armle_d.paa` |
  | `LoBo_Jordan_Pilot` | `lobo_jpilot`, `RJAF\tex` | olive | 8/8 paired, shipped `_d` | unchanged |
  | `LoBo_Syria_Pilot` | `lobo_spilot`, `SyAF\tex` (`back`, `legsbk` are `.pac`) | olive | 0/8: inherited EAF pairs match no face | own array: faces, 8 `SyAF\tex` pairs on the shipped `SyAF\tex\*_d.paa` |

  Faces were read from each p3d (every LOD). No model uses the root `tex\`,
  `RJAF\lod\tex` or `SyAF\lod\tex` folders (olive copies of the SyAF art), and
  `lobo_epilot2.p3d` has no class. Every new array keeps the 52 face (xicht)
  pairs every authored full array carries, byte for byte (the rule checks
  that the four authored arrays carry exactly that list), sits where the
  parent's array sits (after `class Animations`) or replaces the authored
  two-entry one, lists its pairs in the authored order (`armle`, `armri`,
  `back`, `front`, `legsbk`, `legsfr`, `legslft`, `legsrgt`), and has no pair
  that matches no face: `grey2` and `grey3` get no `tex2\armle` pair and
  `LoBo_IAF_Pilot2` no `IAF\tex\armle` pair, because their models draw that
  arm from `tex3`, `tex4` or `tex5`. No new texture is generated: the `grey2`
  and `grey3` bodies reuse the `tex2` `_dr` set and the olive classes name
  shipped `_d` files, which the module lists as `REQUIRED_MEMBERS` (16), so a
  reinstall that lacks one fails closed. Left out on purpose: LOD 9.0 draws
  `EAF\lod` or `IAF\lod` `back`, `front`, `side` and `top`, and no `lod`
  folder has `_d` art; helmet, glove, mask, hose, strap, visor, `top`,
  `leiste`, `veth` and `pou` have no `_d` art either, so they stay unpaired as
  in every authored array.
  Method: the shipped olive set is the RJAF healthy suit plus painted blood
  (DXT noise apart). Per part, the RJAF healthy texture, box-downscaled to the
  `_d` size (half size for front, arms and legs), is the base; RJAF-only
  patches (name tag, arm flag, knee patch: pixels that are not plain olive,
  dilated 2 px) are filled from IAF `tex` where that is plain olive, else from
  the part's mean olive. alpha is a smoothstep of the colour distance between
  `_d` and base, gated on "red" or "darker and not green", quantised to 8 bits
  and blurred as Pillow's GaussianBlur(0.6) does. The blood layer is unmixed as
  `_d - base * (1 - alpha)`; the olive left in it (G above 0.30 R, B above
  0.25 R) moves into R (the brightest of the candidate variants) and the layer
  is capped at 255 * alpha per channel. The result is
  `healthy * (1 - alpha) + layer`, so outside the blood each texture equals
  its suit's downscaled healthy art exactly. Compared with the prototype
  (`wound_recolor.py`), the cap removes its pink fringe on the IAF `tex2`
  shoulder fold highlight (front, `armle`): the fringe came from base pixels
  filled with the mean olive under a highlight, where the unmixed layer kept
  `_d - base` at alpha 0, not from the mask gate, and the same leak had added
  faint DXT noise to every pixel. Three grey leg textures (`tex2\legsbk`,
  `legslft`, `legsrgt`) are AI88 with LZSS levels, which the prototype read
  through PoseidonTools at 4 bits; the rule decodes the 8 bits the GL33
  backend uploads. Inside the blood (alpha above 0.05) the pre-DXT pixels of
  the DXT1-sourced parts match the prototype to a mean of 0.01 to 0.22 summed
  RGB levels. Known cosmetic limits: blood is less contrasted on the dark EAF
  brown than on grey, and the IAF `tex2` back art has a red block in its
  top-right corner (in the healthy texture too).
  Format and determinism: the code is stdlib only (`repair/paa.py` reads DXT1
  and AI88 PAA and writes DXT1 in Python; `repair/wounds.py` holds the pixel
  math), uses integer arithmetic and IEEE double `+ - * /` and `sqrt` in a
  fixed order, and pins the SHA-256 of all 42 source textures it reads from
  LoBo_Pilot.pbo (no repair changes them) and of all 18 outputs, so another
  LoBo install, or a change of the generator, fails closed instead of writing
  different bytes. A present member is checked against its pin without being
  regenerated. Each output has the shipped `_d` size, level count and layout
  (AVGC and OFFS tags, empty palette, 256x256 with 7 levels or 64x256 with 5,
  six-byte terminator), so it is exactly 43,839 or 11,049 bytes like the file
  it replaces in the pair; the AVGC value is the rounded mean of the decoded
  top level (the shipped values run about 1 lower per channel, by an unknown
  formula). PoseidonTools was not used to encode: its writer adds no tags and
  writes 8 levels. The encoder (principal axis and inset box endpoints, two
  least-squares refinements, four-colour blocks only) is within 1.7 to 3.0 RMS
  levels of its source, against 1.2 to 3.1 for PoseidonTools on the same
  pixels. Generating all 18 takes about 6 s; a full UD check took about 11 s.

AIM-9 decision: the missing name sat only in the Geometry LOD (resolution
1e13), which the engine never draws; the GL33 texture bank logged it only
because shape loading touches every face texture of every LOD. The target is
the texture on all 138 faces of the model's only visual LOD, so neither the
rendered missile nor the set of loaded textures changes. This is the pattern .3
applied to the MD500 collision, hitpoint and fire LODs. The issue's candidate,
LoBoMig `swtex.paa`, was rejected: its three copies are byte-identical to each
other but are a 256x64 grey strip with yellow bands, unlike the 512x128 AIM-9
skin, and nothing links them to the lost `rktf5` addon.

Known missing, not changed: one face in LOD 10 of each of the 13 F-16 models
names `LoBoF16\IAF\backfuse.pac` (IAF AA), `LoBoF16\EAF\backfuse.pac` (EAF AA)
or `LoBoF16\com\backfuse.pac` (the other 11). No file of that name exists in any
of the 124 LoBo pbos, the Remaster data, the 2026-10-11 package or the LoBo
release, dependency and recovered archives we hold. That face is drawn, so a
repoint would be a guess at the art; the game keeps logging one line per F-16
variant. Seen while checking and left for later issues: the nine pilot models'
`data\w_pilot_rucebok_l.pac` and `_p.pac` faces, missing textures of the
unreferenced `lobo_epilot2.p3d`, `helmet_lod.p3d` and `iaf_helmet.p3d`, and the
IL-28 tail turret in multiplayer (unverified, from reading the source). The
Syrian pilot's inherited EAF `wounds[]`, seen here first, is now repaired by
`wounds`.

Framework: `textures.repoint` accepts a tuple of expected counts aligned with
the listed resolutions. One int still means the same count in every listed LOD,
and a 0 entry means that LOD holds neither original nor repaired faces. Every
listed resolution must now match exactly one LOD. `models._lods` takes an
optional pinned trailer: the two Hilux models end with an Oxygen default-path
buffer whose path is empty (a NUL, then stale memory). It is accepted only when
all 32 bytes equal that member's pin; any other tail still has to be a
printable path. A module can declare `REQUIRED_MEMBERS`. `plan()` refuses one
outside the repaired addons and fails with `Required members missing` when an
archive entry table lacks it, so a reinstall without a repoint target fails
closed. The two Hilux models were stored compressed; they are decoded through
PoseidonTools and written back stored, and LoBoTer.pbo grows by 4,875,702
bytes. A module can also declare `SOURCE_MEMBERS` (`wounds`: the textures it
generates from). `plan()` reads their original bytes in the same archive pass,
refuses one that is outside the repaired addons, not normalized, a text member
or a model or added member of any rule, fails with `Source members missing`
when an archive lacks one, and passes them to that module's `add_members` as
`sources` (both calls, including the idempotence check).

Result on the installed mod, first two applied batches (the third is under
"Third batch" below): together 7 changed and 18
added members in 5 archives (`lobo_pilot/config.cpp` changed in both). The
first, `20261012T085301Z-fdbc36a3` (state `applied`, rule SHA-256
`f6e16ee80ec1c0698394d17ede66506c18a31bb443c9ef63f8cb12373374d1f9`, before
`wounds`), changed 7 members in 5 archives and added none. Every member hash
equals the spec prototype:

| Member | Bytes before, after | SHA-256 after |
| --- | --- | --- |
| `loboil28/script/tailgun.sqs` | 1,872, 1,908 | `a2aed3ca23bb70b997e3bfdadb02e4df2547803426e110057a93304dd7bb98a0` |
| `lobomig/config.cpp` | 39,012, 39,011 | `1ca20ca72027a21fccc47c569184611f8e401781f39a93dd30edff0e7ab24664` |
| `lobo_pilot/config.cpp` | 22,148, 22,148 | `47ac0111b99b0f9d088a4fa56b7426293d75cbed8433f925f4422196069f80d6` |
| `lobo_pilot/lobo_epilot.p3d` | 1,231,511, same | `18422e9da1a384b126be0574a8b13ad58a63efc60e5e658c465344cfaed94f71` |
| `loboter/lobo_terhilux.p3d` | 3,141,285, same | `b96ec642ae6040067d1893bcad6e60783c50698eca9c3a9957f8e17a23e6607d` |
| `loboter/lobo_terhiluxmg.p3d` | 4,252,849, same | `09ec6792486e02f733579752672b3a61303e8c1d1223fe975917ee4334bab116` |
| `lobo_airammo/lobo_aim9.p3d` | 23,113, same | `4e7f2e733e882174ebfbbe4b2700e0147c9a79461d84f5a7587d7773f43ee6b5` |

The 1,660 untouched members of those archives are byte-identical, and the other
119 archives are unchanged. A further check with and without `--asset-source`
exits 0 with zero changes; a Classic check of the installed mod lists the same
16 wreck-fire members in 14 archives as after .3; report-only doctor says
Nothing to do, with the same 10 unreadable entries. PoseidonTools
`config tojson` shows exactly six parsed differences: `soundLandCrash[0]` and
the five wound strings. From the pristine originals of the applied batches up
to this one, .1 to .4 composed to 166 changed and 7 added members in 50
archives, each equal to the installed bytes (LoBo_Pilot is new;
`lobomig/config.cpp` was already changed by .3). At that point 211 synthetic
repair tests passed (265 with the 54 `@LoBo_Extras` tests), including literal
pins of both new modules. Evidence stays local under
`D:/Arma_CWA/tmp/lobo-repair-4/`.

The second batch, `20261012T185029Z-2990ea75` (state `applied`, rule SHA-256
`de63a7e05bb1973cc8cc1899ad148ca1a0d1473c2515b3c5318c8a3d51dcf3c5`, the .4
rule at that time; the final rule is the third batch's; the first, four-class
draft of `wounds` was `42a829bf...`), applied
`wounds` for every pilot class: 1 changed and 18 added members, all in
LoBo_Pilot.pbo. `lobo_pilot/config.cpp` went from 22,148 to 31,348 bytes
(SHA-256 `e0f0ee8e7cdcc86010b0a65abbc829ba778119320de20515ea63c9d2a9017fae`),
and every member hash equals the UD check made before the apply. The
installed LoBo_Pilot.pbo equals the manifest's after hash. That check of the
first batch's state listed exactly these 19 members, and the 18 added
textures are byte-identical to the four-class draft's. Two check runs gave
identical reports and member hashes. PoseidonTools `config tojson` shows 496
parsed leaf differences, all in the `wounds[]` arrays of six classes: 8
wounded names each in Egypt and grey, and 120 each in grey2 and grey3 (their
2 authored entries become a 120-entry array) and in the new arrays of
`LoBo_IAF_Pilot2` and `LoBo_Syria_Pilot`.
On the engine-parsed rebuilt config, with bases from the class declarations,
all 15 pilot classes (the 8 above and their 7 `_cap` subclasses) pair every
body face of their model (8 of 8; before `wounds`: 7 for 253rd Sqn, 1 for
106th Sqn, 0 for 133th Sqn and SyAF, with the `_cap` classes), no pair names a
texture its model lacks, and every wounded target is an archive member
(shipped or added). The `references` wound rules changed only in where their
match stops and in counting per class (same bytes out). Before the apply, a
LoBo_Pilot.pbo rebuilt in tmp from that plan re-planned `references` and
`wounds` with zero changes and without regenerating, and a Classic check
listed the 16 wreck-fire members plus the same 19. 240 synthetic repair tests
pass (294 with the 54 `@LoBo_Extras` tests; the opt-in real-archive test is
skipped unless `LOBO_PILOT_PBO` names a LoBo_Pilot.pbo, read-only, and it
passed on the then installed and on the rebuilt archive). A preview decoded
from the plan's output bytes (healthy, shipped olive `_d`, generated `_dr`,
per suit and part) is at `D:/Arma_CWA/tmp/lobo-wound-recolor/final/preview.html`;
evidence for the all-class check is under
`D:/Arma_CWA/tmp/lobo-repair-4/allwounds/`. In game, `wounds-01` (below)
draws every class's wounds.

### Runtime verification

Run on 2026-10-12 with the UD engine (`dist/x64-win-rwdi`,
`PoseidonGame.exe` SHA-256
`F0BCDA0852C3AAF84F00F23E23B0FF81275689DE0D937B9E9F46176384A85A0A`) through
Trident (`tri.exe` from the .3 runs, SHA-256
`43D48C10D73AB8925880908FE4CBB4C704EF0B451236157D151E7DF5574A09CF`) on the
official Remaster data
(`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered`, read
only), with the installed `D:/Arma_CWA/@LoBo` mounted by absolute path:
after the first two .4 batches for `tailgun-02`, `refs-01`, `wounds-01` and
`burn-01` (the five changed archives equal their manifests' after hashes),
and after the third batch for `restored-02` (its eight archives equal the
third batch's after hashes, recorded at the start of the run). The probes are
under `runtime/repair4/` (see `runtime/README.md`). All five runs passed on
their first attempt with retries 0 and `-j 1`, ran with `--autotest` (no
script error, `Zero divisor` or `Access denied` line), passed their
`repair4/check_logs.py` criteria and closed every game they started; both
executables had the same hash after each run. Around the first four runs the
repair check exits 0 with 0 changed and 0 added members, `repair_version`
2026-10-11.4 and the then current rule `de63a7e0...`, and the recursive listing
of the Steam Remastered folder (7,971 files with sizes and write times) is
unchanged. After `restored-02` the package build's UD check of the installed
mod (14:47 local, final rule `98f07a96...`) again reports 0 changed and 0
added members, and the listing of the whole Steam CWA app folder (8,782
files) is the same at 12:19, 12:47, 14:37 and 14:50 local, a window that
covers `restored-02` and the official-exe runs below.

| Item | Level | Run | What the run showed |
| --- | --- | --- | --- |
| #13 IL-28 tail turret | runtime verified (UD engine) | `tailgun-02` | No `Zero divisor`, script error or error naming `tailgun.sqs`. C1, empty IL-28: unmanned turret. G, player gunner and no pilot on the ground: he stays in the gunner seat, the driver seat is empty and the turret is unmanned (screenshot). C2, AI pilot first: the turret gunner is a living `LoBo_Egypt_Pilot` in the pilot's group. C3, C2 at 400 m with the pilot killed: the turret gunner bails out into a `ParachuteEast`. A, player gunner and no pilot at 400 m: the authored bailout ejects him (`ParachuteWest`) and the turret stays unmanned. Both bailouts match the spec. |
| #14 references | runtime verified (UD engine) | `refs-01` | Debug log: zero lines naming `black_matte.pac`, `eaf\tex\back.pac`, `eaf\tex\legsbk.pac`, `rktf5\swtex.paa` or `soundss` (the .3 runs logged 327, 35, 34, 56 and 3), and no load failure for any repointed target. `backfuse.pac`: 2 lines, one per F-16 variant, as expected. Reviewed screenshots: Hilux and Hilux MG textured at 8, 35, 70 and 120 m (LOD bias 1), the EAF pilot's back and leg-backs textured at 5 and 40 m, and the AIM-9s, F-16s and MiG-21bis MR1 textured. Only the load of the MiG-21 landing-crash sound is checked, not the sound itself. |
| Pilot wounds (`wounds`, `references`) | runtime verified (UD engine) | `wounds-01` | All eight pilot classes spawned and stayed alive at total damage 0.5 with hits `[hlava, telo, ruce, nohy]` = `[0.85, 0.8, 0.8, 0.8]`. Every class shows wounds front and back in its own suit colour: EAF reddish brown and IAF grey (69th, 106th and 133rd Sqn) with red blood, and the olive classes (IAF 101st and 253rd Sqn, RJAF, SyAF) with the shipped olive art. No texture under `lobo_pilot` failed to load. |
| #9 burn, action, cleanup, sound | runtime verified (UD engine) | `burn-01` | Each item by its own step, below. |
| Restored and repointed files (`restore`, `repoints`) | runtime verified (UD engine) | `restored-02` | 83 log checks pass, 0 fail. The five bomb ammo classes (`LoBo_MK81`, `LoBo_MK81_LDGB` to `LoBo_MK84_LDGB`), the five carriers (MiG-21bis MR1 and MR2, A-4, F-15I Mk-84, F-16I B), the four Egyptian infantry classes, both Egyptian Guard M1s, the F-4 Wild Weasel, the three F-5s, the MDA medic barrack and the Snunit boat are created with their own type. The ten restored members and the nine repointed targets never fail to load, and none of the nine old names is logged. The A-4 dropped one Mk-81 at 427 m (ammo 12 to 11). Screenshots (55) of the bombs as shots, the carriers, the infantry at 4 to 850 m, the M1s at 12 to 1,420 m, the F-4, the F-5 profiles and pilot views, the barrack, the boat's group-bar picture and the AN/PSC-5D gear picture are kept; the bomb shots show the slick Mk-81 and the retarded LDGB with its fins open. Other missing files these models name, none a .4 target: `lobo_ob\gw\has\has.paa` (2 lines), `lobo_ob\emb\icon.paa` (1), `lobo_a4\tex2\tailwing.paa` (4), `lobof16\com\backfuse.pac` (1). An earlier run of the probe, `restored-01` (without the cockpit camera position line), also passed. |
| Official Remaster exe (R5) | runtime verified (official 3.05 exe, smoke test only) | `official-s1-03`, `official-s2-04` | See "The official Remaster exe and the 2026-10-12 package" below. |

The engine draws a body-part wound only at a part damage of 0.7 or more (0.8
for the head; `Man::WoundsAnimation`), so `setDammage 0.5` alone shows none;
the probes raise the hits with `setSelectionDamage`.

`burn-01` used the LoBoEghelo Mi-8 at night; t0 is the mission time at which
its `KioGlowFire` existed, 0.12 s after the kill.

- S: an isolated `KioGlowFire` beside a vanilla `Fire`, both burning, at noon
  and 23:00: the glow shows no campfire mesh, only the Fire light sprite and
  smoke (reviewed).
- D1: on the isolated glow at 3.5 m, "Put out fire" was offered and worked:
  `inflamed` false, then "Light fire" offered. The menu sampled 30 frames
  after the player was placed did not list the action yet; the runner's
  retried assert found it.
- A: 60 m away for the first 70 s (past the script's mortar and bullet
  windows): no damage; the light shows at night (screenshot).
- B: seated in a Jeep 6.86 m from the wreck for 28.1 s: no burn to the player
  or the Jeep, and he never left the seat.
- C: on foot at most 7.17 m from the wreck for 33.3 s: five steps of +0.04
  (at most floor(L/10)+2), from the second step on at least 10.24 s apart.
  "MEDIC! I'm getting burnt here!" shows in the chat 0.19 s after the first
  step (screenshot).
- Screams: 6 `eng_lookout3` plays between t0 and the end of D2 for the 6
  damage steps in that window, one per step.
- D2: at the wreck glow, 3.5 m away, "Put out fire" was offered and the action
  alone put it out (no `inflame false` fallback). No script error; the
  script's particles go on (screenshot) and `say "fire"` played 29 more
  times.
- Sound: 150 `objects\fire` plays between t0 and the cleanup, no load
  failure.
- E: with the player 60 m away, the glow was deleted 163.7 s after t0 (window
  150 to 200 s); the wreck still exists.

Whether "Put out fire" on a wreck glow is acceptable is the user's call: the
run shows the script's particles, `say` lines and cleanup going on after it.

Side observation: `refs-01` logged `lobo_airammo\lobo_mk81.p3d` as not found
(four lookups) when the MiG-21bis MR1 was created. The third part (below)
restores it, and `restored-02` loads it without a failure.

Not run: the optional UD run with all four package modfolders, and any
multiplayer or save case. The official Remaster exe ran only the short smoke
test below.

### The official Remaster exe and the 2026-10-12 package

R5 runs the official 3.05 `PoseidonGame.exe` (SHA-256
`b501e6e6e617b65570ff313cbf3ec5a3ac758a2897710c0491417d830972e7e5`, the same
bytes as the Steam copy) in a throwaway shadow of the Steam Remastered folder
(`tools/runtime/new-remaster-shadow.ps1`: AddOns, DTA, Worlds and the other
data folders are junctions, BIN and the exe are copies), with `@LoBo`,
`@LoBo_Deps` and `@LoBo_Extras` of a package copied in and mounted as
`--mod "@LoBo_Extras;@LoBo;@LoBo_Deps"` (never `@LoBo_UD`). Probe and
procedure are in `runtime/README.md`, "The official Remaster exe (R5)".

First attempt (`official-s1`, `official-s1-02`, `official-s2-01` to
`official-s2-03-diag`, `official-s2-fallback`, `official-s2-fallback-diag`),
on a first 2026-10-12 build (12:30 local, builder `8677533`, rule
`de63a7e0...`; unpublished, moved to
`D:\LoBo-Reloaded\_work\failed-2026-10-12\`). It found two package defects,
both also present in the published 2026-10-11 package:

- `IRAR_EE9.pbo` in `@LoBo_Deps` carries `tracerColor[]={0.8,0.5,0.0.1,1}`
  (`config.cpp` line 55). UD's config reader coerces the token; the official
  exe keeps it as a string and the script evaluator rejects it with
  `Script error at '0.0|#|.1': Unknown operator`: a WARN on both menu boots
  and fatal in two of the four `--autotest` boots (`official-s2-02`,
  `official-s2-fallback`). The 2026-10-11 and the failed build ship the same
  `IRAR_EE9.pbo` (SHA-256 `882562d6...`).
- The Classic wreck-fire query measured `distance` from a position, which
  only UD accepts: `official-s2-fallback-diag` stopped with
  `distance: Type Array, expected Object` as soon as the killed Hilux burned.
  The 2026-10-11 package's Classic `@LoBo` carries the same query (its Classic
  switch wrote the 4,840-byte `fire.sqs`, 9 bytes longer than the fixed one).

The rest were probe matters: `--dev` is refused by the release exe
(`official-s2-01`), and it has no `nearestObjects` (`official-s2-03-diag`).
The fallback run showed case G on the official exe before it stopped: player
gunner, no pilot, no `Zero divisor`.

Fixes:

- Content: the third part's `scripts` change (below); the Classic query now
  measures from the wreck object.
- Package (`package-lobo-family.ps1`): the builder now runs
  `PoseidonTools mod doctor --fix` on the staged `@LoBo_Deps` and
  `@LoBo_Extras` only (never on the cache, the `-ExtrasDir` folder or the
  library), moves the doctor's originals out of the package to
  `<OutDir>\_work\<Version>\doctor-orig`, lists every patch in
  `UD-CHANGES.txt` and the build report, and before writing
  `SHA256SUMS.txt` requires a report-only doctor to say "Nothing to do" for
  every staged modfolder. In the 2026-10-12 build it patched
  `IRAR_EE9.pbo` (MALFORMED_FLOAT, `'0.0.1' -> '0.000'`, SHA-256
  `882562d6...` to `3c80c209...`) and `bas_mah60.pbo` (BURIED_MODEL_ORIGIN
  on `BAS_ROPER.p3d` and `BAS_TJSvetlo.p3d`, buried 8.43 m and 4.53 m,
  SHA-256 `da597670...` to `4d3fb885...`); `@LoBo_Extras` needed nothing.
  The 2026-10-11 package ships the unpatched `bas_mah60.pbo` too. A guard
  refuses `--fix` on, under or above any build input, which includes
  `-LibraryDir`: `-OutDir` must therefore lie outside the library (a first
  run with `-OutDir D:\LoBo-Reloaded\_work\build-2026-10-12-r2\out` was
  refused when it reached the staged `@LoBo_Deps`, after staging `@LoBo`).
- Probe: no `--dev`, `NearestObjectDistance` instead of `nearestObjects`,
  and bool asserts through `format` strings (the 3.05 `triAssert` reads a
  bool as an empty string).

Second attempt, on the package built by the fixed builder from the final .4
rule (`D:\Arma_CWA\tmp\lobo-repair-4\r4-build-02`, 14:41 local). Its
`SHA256SUMS.txt` differs from the published 2026-10-12 package's only in
`README-FIRST.txt` and `@LoBo\UD-CHANGES.txt` (the known-issues text was
updated after these runs); every pbo is the same.

- `official-s1-03`, menu boot: version 3.05 with the three mods loaded,
  `--auto-screenshot` at 60 s, then a clean exit with code 0 after 69 s. No
  `Script error` line at all, no crash or minidump, empty stderr. The
  capture shows the Lost Brothers main menu.
- `official-s2-04`, in mission through Trident with `--autotest`: passed in
  28.0 s on the first attempt, `check_logs.py r5` RESULT PASS. Case G: player
  in the IL-28 gunner seat, no pilot, unmanned turret, no `Zero divisor`. The
  R2 spawns (both Hiluxes, the EAF, RJAF and 69th Sqn pilots wounded at total
  damage 0.5, the MiG-21bis MR1, both F-16 AA variants): none of the five old
  names logged, no load failure for a repointed target, no missing wound
  texture, two expected `backfuse.pac` lines. A killed Hilux made exactly one
  `M1Smoker`, and the wreck burned for about 3 s before the clean shutdown
  without the `distance` error (the failed run hit it right at the kill). No
  `0.0|#|.1` line.

This is a smoke test: longer missions, saves and multiplayer on the official
exe are untested. The published package is
`D:\LoBo-Reloaded\dist\LoBo-Reloaded-2026-10-12.7z` (885,057,397 bytes,
SHA-256 `9dd1e854f748c489ad0524a3a9370074cf234b700c9442f556c59de49bcaca0e`,
built 14:49 local by builder `69fe8b6` with rule `98f07a96...`, 473
`SHA256SUMS.txt` entries; record in `D:\LoBo-Reloaded\docs\build-2026-10-12.txt`).
Its Classic switch rewrote the 16 wreck-fire scripts in 14 archives, and the
source UD check, the Classic check of the staged copy and the composite UD
check all report 0 changes.

### Third part: Classic wreck-fire query and restored members

Implemented and offline verified on 2026-10-12, then applied to the installed
UD profile with the fourth part in the third batch (see "Third batch" below).
Runtime verified: the restored members on the UD engine in `restored-02`, the
Classic query on the official Remaster exe in `official-s2-04` (above). The
UD query is unchanged, so the third batch changed no `fire.sqs`; the Classic
query ships in the package's Classic `@LoBo`. Rule SHA-256 when this part was
checked:
`038742923721bf1b5e46832bb8cabfdee655603aef494441bf0f123b13f9db3d`. Evidence
stays local under `D:/Arma_CWA/tmp/lobo-repair-4/restore/` and
`D:/Arma_CWA/tmp/lobo-missing-models/`.

**Classic wreck-fire query (`scripts`).** The official Remaster exe smoke test
(`runtime/official-s2-fallback-diag`) stopped with `distance: Type Array,
expected Object` when a wreck burned. The Classic `_AI` query measured
`_x distance (_this select 0)`, and its caller passes a position
(`[getPos _object, 10] call _AI`). The position forms of `distance` are a UD
addition (`GameStateExt.cpp`, 3ef5cd5); OFP 1.99 and the official exe accept
Object distance Object only, so all 16 Classic `fire.sqs` failed there. The query
now measures from the wreck object, as the player check
(`player distance _object`) does:

```
before: ... && _x != player && _x distance (_this select 0) < (_this select 1) && !(_x in _units)) then ...
after:  ... && _x != player && _x distance _object < (_this select 1) && !(_x in _units)) then ...
```

`_object` is the caller's wreck: `call` evaluates the code in a child of the
script's variable space (`StringCall`), which the query's
`units group _object` already relied on. The policy is unchanged: only the
wreck owner runs the burn, the local player is handled by its own check
(alive, on foot, within 10 m), the query keeps the living on-foot members of
the player's and the wreck's groups other than the player, de-duplicated and
within range, and the damage cadence (at most 0.04 per 10 s) is untouched.
The literal names only OFP 1.85+ core commands (`private`, `units`, `group`,
`player`, `alive`, `vehicle`, `distance` on two objects, `in` on an array,
`forEach`, `if`/`then`, `select`; a test pins this word set) and no UD-only
command. The earlier literal, the Classic query since 2026-10-11.1, stays
known as `AI_QUERY_CLASSIC_LEGACY`: a copy switched with it (the Classic
`@LoBo` of the 2026-10-11 package and of the failed first 2026-10-12 build)
is upgraded in place instead of getting a second query. The UD query is
byte-identical: the UD check of the installed mod without sources reports 0
changed members. The Classic check of the installed (UD) mod lists the same 16
members in 14 archives as before, each 49 bytes longer, all with the same
one-line diff (the `_AI` line). The official exe smoke test was rerun with a
package built from the final rule: `official-s2-04` burned a Hilux wreck on
the Classic query without the `distance` error.

**Restored members (`restore`).** Ten files that the installed data names,
that are missing from the archive that should hold them, and that an earlier
LoBo release ships under the same name in the same addon. They are copied
unchanged (bytes and stored name); nothing is generated or repointed.

| Archive: stored name | Source (release, pbo date) | Bytes | SHA-256 |
| --- | --- | --- | --- |
| LoBo_airammo.pbo: `LoBo_Mk81.p3d` | LoBoAddonPack3, 2007-01-06 | 23,291 | `02e83da4b977cc78bfe351357423dab55064dd65ac450d07a97f0092737a5336` |
| LoBo_airammo.pbo: `LoBo_Mk81_LDGB.p3d` | same | 25,562 | `fd59a0bfdfaa93e6d690efe835f9d39a360ceb0eaaa7f7817769f648c34980d9` |
| LoBo_airammo.pbo: `LoBo_Mk81_LDGB_d.p3d` | same | 30,592 | `54a6e76b488551b83bbe8bc0c2ce0c82bad97b80d498010d62792998c12813cd` |
| LoBo_airammo.pbo: `LoBo_Mk82_LDGB.p3d` | same | 25,968 | `51254074addff3a70adac198d51bfb9a7f19bf026d7655cc901e46b7e98e42bc` |
| LoBo_airammo.pbo: `LoBo_Mk82_LDGB_d.p3d` | same | 30,592 | `3ce1aad6ec76a4ee35831015e64f060a1767878a7183f9ddc2bfb33ec39e5adc` |
| LoBo_airammo.pbo: `LoBo_Mk83_LDGB.p3d` | same | 26,050 | `65dc83bef7944e27bd2f9311eb74c3f9806e05d1dee9f88850e7dca95d686806` |
| LoBo_airammo.pbo: `LoBo_Mk83_LDGB_d.p3d` | same | 30,674 | `18b13a48e160f4b605f282c671042b6fa500c6488536ec80392e1b13a7a55567` |
| LoBo_airammo.pbo: `LoBo_Mk84_LDGB.p3d` | same | 26,050 | `5a4436dc254c8b8d4943e14bb2da6413d057e93198e15ef3e4bf392d28b5adc5` |
| LoBo_airammo.pbo: `LoBo_Mk84_LDGB_d.p3d` | same | 30,592 | `09db3129eb6bc21d2495488af9c9a6c18f4c8755f090d66032bf020d38da38f5` |
| LoBoEgypt.pbo: `inf\helm_s_in.pac` | LoBoAddonPack1, 2004-06-11 (LoBoDemo2 has the same bytes) | 43,839 | `e21bd718fa96dd5b6e51c29af5f1a8ec1332d55461f4505ae10d9a00be874669` |

- Bombs: the installed LoBo_airammo.pbo (Jordanian/Syrian pack, 2007-05-26)
  defines CfgAmmo `LoBo_MK81` and `LoBo_MK8x_LDGB` and CfgNonAIVehicles
  `ProxyLoBo_Mk81` and `ProxyLoBo_Mk8x_LDGB` on `\LoBo_airammo\LoBo_Mk81`,
  `LoBo_Mk8x_LDGB` and `LoBo_Mk8x_LDGB_d` (`_D` for Mk-83 and Mk-84; the engine
  matches names case-insensitively), but lacks the nine models. The MiG-21bis
  MR1 and MR2 (`LoBo_Mk81Rail2`) and the A-4 (`LoBo_Mk81Rail12`) carry the
  Mk-81, the F-15 and F-16I the Mk-82 and Mk-84 LDGB. LoBoAddonPack3's
  LoBo_airammo.pbo has all nine at the pbo root under exactly these names.
  PoseidonTools `model inspect` loads each (exit 0): MLOD 257, two LODs
  (visual, Geometry with one `Component01`), no proxies, and the only texture
  `LoBo_airammo\tex\bomb.pac`, which the installed archive ships with the
  same bytes (opaque DXT). Wireframe renders show the slick and the
  retarded (`_d`, fins open) bombs.
- Helmet texture: LOD 10 of `loboegyhi`, `loboegygrd`, `loboegypara` and
  `loboegyamphi.p3d` in the installed LoBoEgypt.pbo (LoBoAddonPack3,
  2006-12-16) names `LoBoEgypt\inf\helm_s_in.pac` on 1, 19, 1 and 1 faces;
  that release dropped the file, and the runtime logs carry 1,216 load
  failures for it in 16 runs. In LoBoAddonPack1 and LoBoDemo2 it was the
  helmet texture of LoBoEgyHI's first LOD (126 faces); 19 and 37 other members
  of those pbos are byte-identical to the installed ones. DXT1 256x256, 7
  levels, opaque.

Sources are pbos given with `--asset-source`, now repeatable. Each is matched
to its rule by content and accepted only when the whole archive equals a
pinned SHA-256 and every member it supplies equals its pin (stored, not
compressed); a present member must already carry its pin. The LoBo Reloaded
library keeps byte-identical 7-Zip extracts of the release pbos next to
TheHKpack-1.75.pbo in `sources/recovered-2026-10-11/derived/` (with a NOTE):

| Library file | SHA-256 | From |
| --- | --- | --- |
| `LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo` (7,597,104 bytes) | `de7f4728cc41bd98e93a40e8d01d4816632a39805f191dc05ea6c023b56489fd` | `LoBoAddonPack3.rar` (`b36101e2...`), `LoBoAddonPack3\addons\LoBo_airammo.pbo` |
| `LoBoEgypt-LoBoAddonPack1-2004-06-11.pbo` (14,432,312 bytes) | `992d304a27fefae4105da52cfc58fd9ae6ae0c33d9a963d0ee20161ce77340e9` | `LoBoAddonPack1.rar` (`76a87969...`), `LoBoEgypt.pbo` |

LoBoDemo2.rar's LoBoEgypt.pbo (`f7fe8ca9...`) is accepted as well. The tool
does not call 7-Zip. Without a source its members are left out with one
`WARN:` line per source (the skip alone does not fail `--check`).

Offline verification: `LoBo_airammo.pbo` and `LoBoEgypt.pbo` rebuilt in tmp
from the UD plan (8,721,136 to 8,970,861 bytes, SHA-256
`d54dc5c4d33c54a581b8c602a5b4c37f24057625a484bc056b0f895731427d99`; 57,468,834
to 57,512,711 bytes, SHA-256
`5693df3b80cfda5e59186a25da7fa3f4166425537c31ccc12c50f32cecea5eb0`): the
added records follow the existing ones, PoseidonTools `pbo list` reads 213
members in LoBo_airammo and `pbo show` returns the added bytes unchanged,
report-only `mod doctor` finds nothing, and a UD check of a mod copy holding
them reports 0 changed and 0 added members with and without sources (Classic:
the 16 fire scripts). Checks of the installed mod: UD with the three sources
lists exactly the 10 added members in 2 archives and nothing else; UD without
sources 0 changes, exit 0, two `WARN:` lines; Classic without sources the 16
fire scripts; Classic with sources 26 members in 15 archives. 314 synthetic
tests pass (two opt-in real-archive tests skipped by default; the restore one,
`LOBO_RESTORE_SOURCES`, passed on the library pbos and on LoBoDemo2's
LoBoEgypt.pbo).

Framework: `--asset-source` may be repeated. `plan()` hands each source to
the one rule whose `claims_source(archive)` recognizes it (`weapons`: any of
the seven icon names; `restore`: a pinned archive hash or a restored member
name); a source no rule claims, that two rules claim, that is not a PBO or
that is given twice is an error, and an error message lists each rule's
`ASSET_SOURCE_NAME`. A claiming rule's `add_members` receives the tuple of
its paths (empty when none was given), any other rule `None`; `weapons`
accepts at most one. The idempotence check now requires the second pass to
add nothing and report the same skips (`restore` may add one source's
members and still skip the other's). The summary's `asset_source_sha256` is
replaced by `asset_sources` (path, SHA-256 and rule of each).

R-27 verdict, not changed: CfgAmmo `LoBo_R27R`, `LoBo_R27RE` and
`LoBo_R27TE` name `LoBo_r27r`, `LoBo_r27re` and `LoBo_r27te.p3d`, which no
LoBo release we hold ships (AddonPack1 and Demo2 2004, AddonPack3 2007-01,
Jordanian/Syrian 2007-05), nor any other archive in the library. No magazine,
weapon, script or other config uses those three classes (only `LoBo_R27T`
fires, through `LoBo_R27TRail2`, and `LoBo_r27t.p3d` ships), so the models
are never loaded in play and no runtime log names them. The shipped
`aces_r27r`, `aces_r27re`, `aces_r27te` and `aces_r27t_r.p3d` are
unreferenced ACES originals, not equivalents: they texture `ACES\tex\r27.paa`
and `ACES\tex\rocketflame.paa`, which no LoBo pbo ships, carry a flame
section, and `aces_r27re`/`te` have an empty Geometry LOD. `LoBo_r27t.p3d` is
LoBo's conversion of `aces_r27t_r` (the 466-point missile without the
14-point flame, retextured to `LoBo_airammo\tex\r27.paa`), as `LoBo_r60` and
`LoBo_r73` are of their ACES `_R` models; the r27r, re and te conversions were
never released. Left missing.

Sweep: every LoBo path that 51 game logs (the 2026-10-11 acceptance runs, the
.3 and .4 runs, `official-s2`) report as not found and that the installed mod
still lacks, looked up in a name index of every pbo inside the 16 release, 19
dependency and 26 recovered archives (7-Zip streamed; the 11 pbos of
`CoC_UA110_Setup.exe` were unreadable), the library modfolders and the repair
backups:

- Restored here: `lobo_airammo\lobo_mk81.p3d`, `lobo_mk82_ldgb(_d).p3d` (the
  other six LDGB models were not logged), `loboegypt\inf\helm_s_in.pac`.
- Already handled by `references` (repoints): `loboter\toy\t\black_matte.pac`,
  `lobo_pilot\eaf\tex\back.pac` and `legsbk.pac`, `rktf5\swtex.paa`,
  `lobomig\soundss\jet_crash.wss`.
- A file of that name exists elsewhere in the same installed addon:
  `lobo_fab\snunit.paa`, `loboradios\icon\basirto.paa`, `lobo_m1\tex1\kolo.pac`,
  `lobo_m1\tex1\main_side.paa`, `lobo_f4\idf\mer.pac`,
  `lobo_f4\idf\dark_grey.pac`. All repointed by the fourth part (below).
- Only an older release has a file of that name, at another path:
  `lobo_a4\tex2\tailwing.paa`. Left; see the fourth part.
- No source found in that sweep: `lobo_ob\bar\b4.jpg`, `lobo_ob\emb\icon.paa`,
  `lobo_ob\gw\has\has.paa`, `loboegapc\bmp\ebmp.paa`, `bmpg\ebmp.paa`,
  `m113\em113.paa`, `zsu\ezsu.paa`, `lobof5\com\white.paa`,
  `lobojorapc\cvr\icon.paa`, `lobolebobject\tex\usemicon.paa`,
  `lobousembassy\tex\usemicon.paa`, `lobosherm\im4.paa`, and the known
  `lobof16\{eaf,iaf}\backfuse.pac` and `data\w_pilot_rucebok_l.pac`,
  `_p.pac`. The fourth part repoints `b4.jpg` and `white.paa` and names the
  sources found for three more.
- Not LoBo files (dependencies or other mods, out of scope): `coc_arty`,
  `coc_mlrs` and `apcupd` animations and textures, `biscamel`, `bizmk_1`,
  `dkmm_arty`, `dkmm_ukpack`, `opgwc_ee11`, `editorupdate102`, `tmyk_bridges`,
  `data\periskop2.paa`, `sound\vehicles\gun_elevate2.wss`, and UD's own fonts.

### Fourth part: references to files LoBo ships under another name

Implemented and offline verified on 2026-10-12, applied to the installed UD
profile in the third batch (below) and runtime verified on the UD engine in
`restored-02` (none of the nine old names logged, every target loads; see
"Runtime verification"). Module `repoints` (after `references`, same techniques: one config
string per rule, fixed-width model texture fields with exact per-LOD counts,
`REQUIRED_MEMBERS` failing closed). It adds nothing. Evidence stays local
under `D:/Arma_CWA/tmp/lobo-missing-2/`; `preview.html` there shows, per item,
what the model or config wants and what now fills it.

Every candidate of the sweep above was checked by hand: who names the file
(config class, or model, LOD and face count), the candidate's bytes, both
decoded to PNG through PoseidonTools and looked at, and for model faces the
faces' UVs drawn over the candidate. A final search indexed every pbo inside
every archive under `D:/LoBo-Reloaded/sources` (releases, dependencies,
recovered, arma-era, the mega_nc cache, the repair originals; 7-Zip streamed,
nested archives included, 76,148 entries; the 11 pbos of
`CoC_UA110_Setup.exe`, `HKRealismPatch.exe` and the ACE archive stay
unreadable) and the Steam Remaster folder (read only).

Repointed, 9 members in 6 archives:

| Missing name | Named by | Now names |
| --- | --- | --- |
| `\LoBo_FAB\Snunit.paa` | `picture` of `loBo_Snunit` | `\LoBo_FAB\SN\Snunit.paa` |
| `\LoBoRadios\icon\BASirto.paa`, `icon\BAS_satcom.paa` | `uiPicture` and `picture` of `BAS_Satcom` | `icons\` (same names) |
| `\LoBo_Ob\bar\b4.jpg` | `icon` of `LoBo_B_Medic_MDA` (6 subclasses inherit it) | `\Baracken\b4.jpg` |
| `LoBo_M1\tex1\kolo.pac`, `tex1\main_side.paa` | `lobo_m1a1e_r` and `lobo_m1a2e_r` (Egyptian Guard M1A1/M1A2): LOD 15 84 and 117 faces, LOD 20 20 `main_side` faces | `tex1\kolo.paa`, `tex1\main_side.pac` |
| `LoBo_F4\idf\mer.pac`, `idf\dark_grey.pac` | `lobo_f4_ww_iaf` (IAF Wild Weasel), LOD 5: 6 and 3 faces of the ECM pod selections | `idf\1\mer.pac`, `com\dark_grey.pac` |
| `LoBoF5\com\white.paa` | the three F-5 models, LODs 500 and 1100 (pilot view): 10 and 347 faces | `LoBo_Kfir\kfir1\white.paa` |

- Snunit: `SN\Snunit.paa` (128x64, alpha) is a picture of the camouflaged
  patrol boat, stored with its hull textures and painted like them; no model
  uses it, and the other pictures of the addon have the same size.
- Radios: `icons\basirto.paa` has the bytes of `icons\sebirto.paa`, which the
  three other radios use (a crouching soldier with a radio pack);
  `BAS_satcom.paa` is a picture of the AN/PSC-5D set the class is named. The
  picture never reached the logs (gear pictures load on demand) but has the
  same `icon` typo.
- Medic buildings: Baracken's `Brown_Baracke04` is the medic barrack
  (`ATTENDANT=1`, brown, green and grey variants); LoBo's six medic classes
  repeat that pattern with MDA and Red Crescent signs from `bar\`, and copied
  its icon path into their own folder without the file. `\Baracken\b4.jpg` is
  that class's icon, a barrack drawn with a cross. Baracken.pbo ships in
  @LoBo since the Sinai island release, so `baracken` is a repaired addon (read
  only) and `baracken/b4.jpg` a required member.
- M1 Guard: the `_r` models are the three-colour `tex1` set. LODs 5, 7.5 and
  20 use `tex1\kolo.paa`, LODs 5 and 7.5 `tex1\main_side.pac` (with
  `main_side2`); only the extension differs. Drawn over the targets, the LOD
  15 side faces cover `main_side.pac`'s atlas as LOD 5 does, and the LOD 15
  wheel faces map a wheel disk as LOD 20's do. LOD 20's 58 `kolo.paa` wheel
  faces are the author's own repoint of the same wheels (the sand model's
  LOD 20 names `tex2\kolo.pac` on 56 of them); they count with the target,
  so the LOD 20 count is 58. The sweep's candidate, the sand `tex2\kolo.pac` of the
  other M1 models, was rejected for its colour.
- F-4 Wild Weasel: the faces sit on the borrowed ECM and LANTIRN pods
  (`ecm_1` to `ecm_3`); the `mer` faces are the centre pylon strip, mapped
  across the whole MER texture. Both F-4 `mer.pac` files and the F-16 IAF one
  are byte-identical, and so are the `dark_grey.pac` of the F-4, F-15 and
  F-16. The counts (232, 266) include the 226 and 263 authored faces.
- F-5: LoboF5 ships no white. Every `white.paa` in @LoBo (Kfir, IL-28,
  MiG-21, MiG-25 and the BDE Mirage) is the same 32x32 opaque white (SHA-256
  prefix `b70b6af3`), and the F-5's `red` and `silver` have the bytes of the
  Kfir's and MiG-21's. The Kfir's, which its own model draws on 1,282 faces,
  is the target. The three models end with an empty Oxygen default-path
  buffer, pinned per member as for the Hilux.

Left as authored:

- `LoBo_A4\Tex2\tailwing.paa` (one degenerate face at the fin cap, LODs 0.5
  and 2 of the three A-4 models). The 2004 releases (AddonPack1, Demo2) ship
  `tailwing.paa` in the addon root, byte-identical to their
  `tailwingtop2.paa`, painted in the earlier bright camouflage; the face
  keeps exactly the 2004 UVs. The 2007 release re-made the tail textures in
  `Tex2` (its own `tailwgtop2.paa` is a different, grey texture), so the
  missing `Tex2` file was a re-skin that never shipped, not the 2004 art.
- Found only outside @LoBo: `LoBo_Ob\gw\has\HAS.paa`, the editor icon of
  the hardened aircraft shelter, is OPGWC's `HAS.paa` marker (LoBo_Ob's
  `gw\` textures are byte-identical to OPGWC `ds_obj.pbo`'s, from
  `opgwcbeta.zip`); `LoBoJorAPC\CVR\icon.paa`, the Scorpion picture: the
  `CVR` folder is GW_Scimitar's (byte-identical textures), whose own config
  takes its picture from `\MLV_scimitar\icon.paa` (MLV_MOD_BETA3). Both
  addons are dependencies (`@LoBo_Deps`), so a repoint would tie @LoBo to
  them and could not be checked by `--mod`; restoring would copy third-party
  bytes under a new name. `\LoBoSherm\iM4` (LoBoRollers' picture): the only
  LoBoSherm is Project73's ArmA 1 conversion (2009, an "M4-A3" picture); the
  OFP addon is lost and @LoBo has no LoBoSherm prefix.
- No source anywhere: the LoBoEgAPC pictures `BMP\ebmp.paa`, `BMPG\ebmp.paa`,
  `M113\em113.paa` and `ZSU\ezsu.paa`; the editor icons
  `LoBoUSEmbassy\tex\USemicon.paa`, `LoBolebObject\tex\USemicon.paa` and
  `LoBo_Ob\emb\icon.paa`; `LoBoF16\{com,EAF,IAF}\backfuse.pac`; the pilots'
  `data\w_pilot_rucebok_l.pac` and `_p.pac` (the Remaster `Data.pbo` has only
  `w_pilot_rucebok_p_d.pac` and the tank crew's `w_tank_rucebok_l/_p.pac`).

Offline verification (rule SHA-256
`98f07a96676496b149528e96450c63c27755341e9e47c87aa6f977049385aa48`, the final
.4 rule):

- Checks of the installed mod. UD with the three sources: 10 added members
  (unchanged from the third part) and the 9 changed members above, nothing
  else, in 8 archives. UD without sources: the 9, two `WARN:` lines. Classic
  without sources: the 16 fire scripts and the 9; with sources, also the 10.
  Every model diff changes only the listed texture fields (file length and
  all other bytes equal); PoseidonTools `config tojson` shows exactly four
  parsed differences: the Snunit `picture`, both BAS_Satcom paths and the
  medic `icon`.
- The six archives rebuilt in tmp from the UD plan (LoBo_FAB 78,726,459 to
  78,726,462 bytes, LoBo_Ob 71,028,973 to 71,028,970, LoBoRadios 930,069 to
  930,071, LoBo_F4, LoBo_M1 and LoboF5 same length; LoBo_airammo and
  LoBoEgypt with the third part's after hashes) re-plan to 0 changes with
  and without sources (Classic: the 16 fire scripts); `pbo list` and
  `model inspect` read them; report-only `mod doctor` on the copy finds
  nothing.
- From the pristine originals (each archive's earliest applied backup), .1
  to .4 compose to 163 changed and 35 added members; every one of the 163
  equals the installed bytes, and the 9 repoints equal the installed-mod
  plan.
- 324 synthetic tests pass (two opt-in real-archive tests skipped; the
  restore one passes on the library pbos). `test_lobo_repoints.py` covers
  the text rules, the model plans with authored target faces, the pinned
  trailers and the end-to-end plan with each required member missing. At
  `69fe8b6` the same discover run gives 324 tests, which include the 54
  `@LoBo_Extras` tests and the 3 packager doctor tests
  (`test_lobo_package_doctor.py`), with the same two skips.

### Third batch (third and fourth parts applied)

Applied on 2026-10-12 as batch `20261012T205817Z-7a0580a4` (state `applied`,
`repair_version` 2026-10-11.4, rule SHA-256
`98f07a96676496b149528e96450c63c27755341e9e47c87aa6f977049385aa48`, the final
.4 rule): 9 changed and 10 added members in 8 archives, the repair report
counting 19 members. The installed LoBo_airammo, LoBoEgypt, LoBo_FAB,
LoBoRadios, LoBo_Ob, LoBo_M1, LoBo_F4 and LoboF5 pbos equal the manifest's
after hashes (`d54dc5c4...`, `5693df3b...`, `6f44009f...`, `b44f3a01...`,
`a9e772c7...`, `97f9100b...`, `b6f1c711...`, `990917ae...`), the LoBo_airammo
and LoBoEgypt ones equal the archives rebuilt in tmp for the third part, and
every member hash equals the plan. The UD check of the installed mod during the
2026-10-12 package build reports 0 changed and 0 added members with this rule,
and the report-only doctor says Nothing to do in 124 pbos (10 entries not
readable). In game: `restored-02` (UD engine) and, through the package,
`official-s2-04` (official exe).

The command, from the repository root:

```powershell
python arma_CWR/tools/lobo/repair_lobo.py --mod 'D:\Arma_CWA\@LoBo' --compatibility ud --apply `
  --tools 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe' `
  --asset-source 'D:\LoBo-Reloaded\sources\recovered-2026-10-11\derived\TheHKpack-1.75.pbo' `
  --asset-source 'D:\LoBo-Reloaded\sources\recovered-2026-10-11\derived\LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo' `
  --asset-source 'D:\LoBo-Reloaded\sources\recovered-2026-10-11\derived\LoBoEgypt-LoBoAddonPack1-2004-06-11.pbo' `
  --report 'D:\Arma_CWA\tmp\lobo-missing-2\applied-ud.json'
```

After a reinstall of @LoBo the same command applies every rule from .1 to .4
in one batch (the asset sources supply the seven HK icons of .3 and the ten
restored members); a second check with or without the sources must then
report 0 changes and exit 0.

## 2026-10-11.3 batch

Implemented, applied to the installed UD profile and offline verified on
2026-10-11. In game, LoBo #4, #5 and #8 are runtime verified, #9 was partly
runtime verified by these runs and the rest by the 2026-10-11.4 runs, and #2
and #3 are offline verified only (see "Runtime results" at the end of this
section). Nine fail-closed rule modules run after `followups`. Every rule
accepts the original, repaired or partially repaired state only, is
idempotent and is profile-neutral (OFP 1.x SQS commands and
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
  `say` go on until the 150 s cleanup. No config key suppresses that action;
  the .4 run `burn-01` tried it in game. Activation caveat: the owner addon is
  `lobo_airammo`. When a mission has not activated it, UD logs one
  `Access denied ... not activated` warning per glow and still creates the
  object (run `glow-unlisted-01`). The official 3.05 exe is expected to behave
  the same but was not run, and stock 1.99 behaviour is unverified. The .3
  runs verified glow creation and the killed-handler path; the burn, the
  action, the 150 s cleanup and the `say` sound were runtime verified on the
  UD engine by the .4 run `burn-01` (see the 2026-10-11.4 section).
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

Not run by these .3 runs: for #9, a player seated in a vehicle next to a
burning helicopter wreck (no burn), a player on foot within 10 m of it (one
MEDIC line, 0.04 damage at most once per 10 s), the "Put out fire" action at a
wreck glow, the 150 s cleanup and the `say "fire"` sound (the wreck
screenshots show only smoke). The .4 run `burn-01` covered all of these on
2026-10-12 ("Runtime verification" in the 2026-10-11.4 section). For #4 and
#5 only the script path was tested: the aircraft was held
still and invulnerable at 150 m, so ballistics, spread and damage are
unchecked, and the bomblet counts are lower bounds from a check about every 10
frames. The F-5 and A-4RN Mk-20 paths were not flown. Problems the runs found
that .3 did not cause: LoBoIL28 `script\tailgun.sqs` stops with "Zero divisor"
when an IL-28 has crew aboard but no pilot, and the logs report missing
`lobomig\soundss\jet_crash.wss`, `loboter\toy\t\black_matte.pac`,
`lobo_pilot\eaf\tex\back.pac`, `rktf5\swtex.paa` and
`lobof16\iaf\backfuse.pac`. The 2026-10-11.4 batch above repairs the IL-28
script (LoBo #13) and the first four references (LoBo #14), runtime verified
on the UD engine in its runs `tailgun-02` and `refs-01`; `backfuse.pac` stays
known missing (one line per F-16 variant in `refs-01`).

Savegame caveat: SQS scripts already running in a save keep their old compiled
lines, and the engine does not save tail numbers or other `setObjectTexture`
results; test with new missions. A second, unrepaired LoBo copy sits at
`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered\@LoBo`; a
game that mounts that copy gets none of .1 to .4.

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

# Apply to the UD installation, with verified original backups. The asset
# sources supply members LoBo references but its release lacks: the seven HK
# inventory icons and, since 2026-10-11.4, nine bomb models and a helmet texture.
python arma_CWR/tools/lobo/repair_lobo.py --mod '@LoBo' --compatibility ud --apply `
  --asset-source 'D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/TheHKpack-1.75.pbo' `
  --asset-source 'D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo' `
  --asset-source 'D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/LoBoEgypt-LoBoAddonPack1-2004-06-11.pbo' `
  --report tmp/lobo-applied.json

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
the read. Since 2026-10-11.4 this also decodes the two compressed Hilux
models, which are written back stored.

`--asset-source PATH` names a user-supplied pbo and may be repeated (since
2026-10-11.4). Each source goes to the one rule that recognizes it by content;
one that no rule recognizes, that two rules claim, that is not a PBO or that
is given twice is an error. The sources:

- `TheHKpack.pbo` 1.75 (`weapons`, seven HK inventory icons): the original
  archive (SHA-256 `79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5`)
  or the `@LoBo_Extras` copy, which differs only in its patched `config.bin`.
  Accepted only when all seven icon members are present, stored uncompressed
  and equal to their pinned SHA-256 values.
- LoBoAddonPack3's `LoBo_airammo.pbo` (`restore`, nine bomb models; SHA-256
  `de7f4728cc41bd98e93a40e8d01d4816632a39805f191dc05ea6c023b56489fd`) and
  LoBoAddonPack1's or LoBoDemo2's `LoBoEgypt.pbo` (`restore`,
  `inf\helm_s_in.pac`; `992d304a...` or `f7fe8ca9...`). Accepted only when the
  whole archive equals a pinned hash and each supplied member its pin. The
  LoBo Reloaded library keeps these as
  `sources/recovered-2026-10-11/derived/LoBo_airammo-LoBoAddonPack3-2007-01-06.pbo`
  and `LoBoEgypt-LoBoAddonPack1-2004-06-11.pbo`.

A wrong source is rejected even when nothing is missing. Without a source its
members are not added: the summary lists them under `skipped`, stderr prints a
`WARN:` line per source, the skip alone does not make `--check` fail, and the
MSG90 picture keeps its authored `\TheHKpack\` path (a repoint made earlier
without the icon is put back). Members already present must carry their
pinned bytes. The JSON summary also reports `added_members` and, when sources
are given, `asset_sources` (path, SHA-256 and rule of each; before .4 a single
`asset_source_sha256`).

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
Since 2026-10-11.4 the Classic query measures range from the wreck object
(`_x distance _object`): OFP 1.99 and the official Remaster exe accept only
Object distance Object, and the earlier `_x distance (_this select 0)` (a
position) stopped the official exe with "distance: Type Array, expected
Object". A copy switched with the earlier query is upgraded by the next Classic
run.

Do not redistribute the UD profile as a Classic-compatible repair: Classic 1.99
does not provide `nearestObjects`. The newer `createTrigger`/`setTriggerArea`
commands are also unsuitable as an assumed Classic PC fallback. On the official
Remaster exe (3.05) only a smoke test has run (`official-s2-04`, one wreck
burn with the Classic query, no script error); legacy 1.99 has not been run.
`package-lobo-family.ps1` switches its staged copy of @LoBo to this profile
itself and ships the UD-profile pbos separately in `@LoBo_UD`; outside that
builder, prepare a Classic source copy explicitly.
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
the parsed difference contains only the intended keys. The same holds for the
two configs the 2026-10-11.4 batch changes.

The repair tool itself launches no game. Runtime evidence comes from separate
probes: the issue #1 acceptance runs and the 2026-10-11.3 and 2026-10-11.4
probes, all recorded in [runtime/README.md](runtime/README.md). Runtime combat, all missile shots, AI
weapon selection, animated hit locations, saves and multiplayer still need a
separately authorized play test. All peers should use the same repaired content.
