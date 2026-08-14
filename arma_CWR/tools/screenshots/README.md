# Screenshot shoots

Scripted, repeatable in-game photography. These are **screenshot generators, not
regression tests** — they live outside `tests/integration/` on purpose so a full
`tri test tests/integration` sweep never picks them up. Run them explicitly.

The output feeds the public site (`site/`), so the frames need to look like
photographs, not like a debug view: no HUD, deliberate framing, decent light.

## Running

From `arma_CWR/`:

```bash
./engine/Trident/target/debug/tri.exe test --retries 0 --output-dir tmp/showcase/sinai tools/screenshots/lobo_sinai_showcase.test.sqf
```

| Shoot | What it covers |
| --- | --- |
| `lobo_sinai_showcase` | @LoBo roster catalogue: IDF / Egyptian / Syrian / Jordanian armour, aircraft, infantry, the Eilat port, the Suways street |
| `lobo_sinai_stories` | @LoBo narrative frames: ambush, firefight, checkpoint, aftermath, night contact |
| `vanilla_malden_stories` | Stock game on Malden, no mods — same rig, same discipline |
| `noe_stories` | Stock game on **Nogova**, no mods: forest-road ambush, farm-village firefight, villagers, checkpoint and armour among the concrete blocks. Boots `missions/Stories.Noe`, a two-object mission that exists only to hang a camera off |
| `probe_props` | Diagnostic: spawns each static scenery prop alone to find the ones that crash the renderer |
| `probe_aim` | Diagnostic: proves the captive `reveal`+`doFire` aimed-fire mechanism the firing scenes use, and measures which men actually shoot (ammo deltas come back through a deliberately-failing assert) |

> **Spawning an unresolvable class no longer kills the process.** It used to: an
> addon-denied entry resolves `scope` through to the inherited 0, so
> `VehicleTypeBank::Load` fell back to a bare abstract `EntityAIType`, and
> `NewVehicle` then `static_cast` it to whatever its simulation implied.
> `Building::Building` read `NPos()` off garbage and `_locks.Resize()` memmove'd
> that length — 0xC0000005 inside VCRUNTIME, reproducible with @LoBo's
> `LoBo_uralwreck01`. `NewVehicle` now refuses to build an abstract type and logs
> `Cannot create '<class>': type is abstract` at WARN
> (`World/Entities/Vehicles/VehicleTypes.cpp`). A shoot that names a class it
> cannot have now just gets a no-show. Still worth keeping the props out of the
> critical path: never aim a camera *at* a prop, and never index into a prop list
> the scene might not have filled.

> **The wreck props are usable now (2026-08-08).** `LoBo_uralwreck01` and the
> other nine `LoBoWreck` classes were not "never meant to be spawnable": that pbo
> and `LoBoPalObj.pbo` are the only two @LoBo addons that omit the
> `#define private 0 / protected 1 / public 2` header every other @LoBo config
> carries, so `scope = public;` resolved to nothing and read back as 0. @LoBo is
> ours to modify now, so this is repaired at source by
> `tools/lobo/fix-lobo-scope.ps1` rather than shimmed in `@udshowcase`. Run it
> once per @LoBo install. Two caveats for framing: `LoBo_M60A1_wreck` and
> `LoBo_M60A1_wreck2` build and draw but sit about 4 m below ground at
> `createVehicle` height (their p3d lacks the seating LOD its siblings have) —
> `setPos [x, y, 4]` puts them on the sand; and a real vehicle with
> `setDammage 1` + `inflame true` still reads better on camera than any static
> wreck.

Each writes a PNG **and** a BMP per shot into `--output-dir`, numbered in
capture order. Delete the BMPs afterwards (`rm *.bmp`) — they are ~11 MB each
and carry no extra information.

To review a whole run at once, build a contact sheet rather than opening 40
files:

```bash
python tools/screenshots/contact_sheet.py tmp/showcase/sinai tmp/showcase/sheet.png
```

## Preconditions

The @LoBo shoots need the same setup as the `lobo` test lane:

- the full CWA 1.99 install as `OFPR_DATA_DIR` (see `.trident.env`)
- `@LoBo` next to it at `D:\Arma_CWA\@LoBo`
- one-time, idempotent, rerun after any @LoBo reinstall:
  `tools/lobo/fix-lobo-scope.ps1` and
  `tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1`

They also mount `@udshowcase` (in this folder) last. That mod exists because the
engine only builds a vehicle type whose owner addon is **active**, and a mission
activates only what its `addOns[]` lists. The shoots spawn most of @LoBo's
roster, far beyond what `Guerrilla.Sinai` declares, so `@udshowcase` pre-activates
the rest via `CfgAddons >> PreloadAddons`. It is deliberately separate from
`@lobofixup`: that fixture is shared by the `lobo` test lane, and widening a
shared fixture to suit a screenshot shoot moves every one of those tests onto an
addon set no shipped mission has. Keep the shoot's roster in the shoot's own mod.

The vanilla shoot needs no mods and no fixture.

## How the camera works

This is the part worth knowing before editing a shoot. All of it was arrived at
the hard way and the details are load-bearing.

**Framing is `triSetView [east, UP, north, dirE, dirUp, dirN]`** — absolute world
coordinates in engine axis order, where Y is elevation. The obvious approach,
the cutscene camera (`camCreate` + `camSetTarget` + `camSetRelPos`), aimed at the
sky; `camSetDir` could not rescue it, because at the time `camSetDir` and
`camSetBank` were both mis-wired to `CamSetDive` in the command table
(`GameStateExt.cpp` ~1374) and pitched the camera instead of turning it. That is
fixed (see `tests/integration/rendering/cam_set_dir_heading`), but `triSetView`
remains the right primitive for a shoot: it takes an absolute eye point and an
absolute look direction in one call, both computed from `getPosASL` arithmetic,
with no commit step and no angle conversion in between.

**Coordinates are computed in-script from `getPosASL obj`**, which returns
`[east, north, ASL]`. Nothing hardcodes a terrain height, so the same scene block
works on a 7 m quay or a 280 m hilltop. Do not trust authored elevations: the
`Guerrilla.Sinai` zone config claims 160 m at the Camp where the ground is 64.7 m.

**A cutscene camera is still created, purely to kill the HUD.** The gameplay HUD
(ammo counter, action menu, crosshair) is gated on `!GWorld->GetCameraEffect()`
(`DisplayUIMenus.cpp:974`). Without an active camera effect every frame is stamped
with UI. Its own framing is irrelevant — `triSetView` overrides it.

**`triScreenshot` must be given a string literal.** The runner parses the literal
out of the statement to maintain its own sequence counter, so a `format [...]`
label desyncs it and the run dies on `FAIL:screenshot_not_written`.

## Harness notes

- Trident splits a `.test.sqf` on newlines and semicolons at brace depth 0 and
  sends each statement as a **separate** eval, so `_local` variables do not
  survive from one line to the next. A multi-line `{ ... }` block is sent as one
  eval. Everything shared between statements here is a global (`ss*`).
- `createGroup` returns `grpNull` for a side with no center, and the units then
  silently never appear. Sinai has no RESISTANCE or CIVILIAN center, so the
  shoots call `createCenter` first (it is idempotent).
- Everything spawned is `setCaptive true`, so nothing engages on its own — the
  live campaign ignores the sets and the sets ignore each other. Captivity only
  changes how *observers* classify a unit (`GetTargetSide` → civilian); a
  captive can still be *ordered* to shoot, which is how the firing scenes work
  (next section) while staying invisible to every AI not in on the shot.
- Clock on Sinai: sunset is around 16:30. 15:00 is clean daylight, 15:40 is
  golden hour, 17:30 is already night.

## Finding somewhere worth photographing

Do not guess at coordinates, and do not trust a mission's zone positions — "El
Tor" in the Sinai template is bare dune. Dump the world's objects and cluster
them:

```bash
PoseidonTools terrain objects <world>.wrp > objects.txt
```

The columns are `[east, ELEVATION, north]`, **not** the order the header
suggests. Cluster the building-ish model names on a 150-250 m grid and the towns,
airbases, ports and firebases fall out. That is where every anchor in these
shoots came from.

Keep the camera at least 6 m up and 14 m out for anything staged near buildings.
Earlier passes shot the inside of a hangar wall, a shanty roof and a monastery
wall.

## Framing that works

| Subject | Spacing | Camera |
| --- | --- | --- |
| Group of vehicles | 5-6 m | ~12 m out, 3.5 m up, aim 2 m |
| Hero vehicle | — | ~8 m out, 1.5 m up, aim 1.7 m |
| Aircraft line | 17 m | ~17 m out, 4 m up, aim 1.8 m |
| Infantry line | 1.9 m | ~9 m out, 2 m up, aim 1.5 m |
| Single figure | — | ~4 m out, 1.6 m up, aim 1.5 m |
| Establishing | — | 70-150 m out, 55-110 m up, aim 0 |

Wider spacing plus a longer throw makes everything tiny in a sea of sand, which
is how the first three passes failed.

## Making a frame tell a story

A parked tank is a catalogue entry. What reads as a photograph:

- **Firing — give the man a target, never force the shot.** `unit fire
  (primaryWeapon unit)` on a target-less soldier goes through
  `Man::AimWeaponForceFire`, which aims at `Direction()` with an elevation
  component of **10** — near-vertical, clamped to the stance's max gun
  elevation — and pass 2 shipped whole squads volleying at the sky at 45-60°.
  What works instead: spawn a captive foe line 15-30 m out **on the
  subject→background line**, `reveal` it to the squad, then give each man his
  own silent per-man `doFire` (AssignTarget + EnableFireTarget; the
  enable-fire target is exempt from the is-it-an-enemy gate in
  `SelectFireWeapon`, so a captive/"civilian" target is shot anyway). The
  combat AI then aims **level at a human** and pours real bursts on its own
  cadence — so instead of one synchronized volley frame, take two or three
  frames a few sim-frames apart and keep the ones where a flash landed.
  Re-issue the `doFire` before every frame (a repeat of the same order is a
  no-op) to re-point any man whose target just died. Two rules proved by
  `probe_aim.test.sqf`: captive men never trip the friendly-in-the-lane fire
  block (they read as civilians to it), and a firing lane must never cross
  another staged squad — incoming rounds suppress the men and scramble the
  poses.
- **Wreckage.** A real vehicle with `setDammage 1` + `inflame true` gives the
  destroyed model, fire and smoke. Prefer this to the static wreck props. The
  flame phase is finite: a scene that stages a long engage warm-up after the
  burn must call the burn again just before its frames, or the "firelight"
  scene shoots in the dark.
- **Casualties.** The bullets are real now, so foes engaged at 15-30 m
  genuinely drop — bodies land where the fire found them. Snapshot the foe
  list *at spawn* (a dead man is no longer in `units group`) and `setDammage 1`
  any survivor afterwards when a later scene depends on the corpses.
- **Posture.** Mixing `setUnitPos "UP"/"MIDDLE"/"DOWN"` across a group reads as a
  firefight; all-standing reads as a parade. Use all-standing for patrols and
  checkpoint guards, where a prone man just disappears into the ground.
- **A vehicle weapon needs a crew, and a target.** An uncrewed hull has no
  gunner AI: forced `fire` does nothing at all — no flash, no report, no
  error — and even a crewed tank force-fired with no target hoists the gun to
  max elevation first (`EntityAI::AimWeaponForceFire`, elevation component 3).
  `moveInDriver`/`moveInGunner` a spawned unit, then `doFire` the tank at an
  **empty enemy hull** parked 45-60 m downrange: empty + engine-off classifies
  as civilian, so the campaign ignores it — and the fire-value math never
  spends a main-gun shell on a civilian-classified target, so the gunner works
  it with the coax: level, continuous, and incapable of destroying the target
  out from under the scene.

## Getting vanilla civilians to stand up

Base-game civilians are the one thing in these shoots that fights back, and two
whole passes went into it, so:

1. **List `BIS_Resistance` in the mission's `addOns[]`.** `Civilian4`..`Civilian11`
   and `Woman1`..`Woman5` live in `AddOns/O.pbo`, whose CfgPatches class is
   `BIS_Resistance`. Without it the engine logs `Access denied: … owner addon
   'bis_resistance' is not activated` and the class resolves only *partially* —
   the unit spawns but has no usable stance and lies in the road at an angle, no
   matter what `setUnitPos` says. Only `Civilian`, `Civilian2` and `Civilian3`
   come from `bin/config.bin` and are always visible. Every `Soldier*` and every
   vehicle used in these shoots is base-game, which is why *only* the civilians
   misbehaved.
2. **Order the group, not the units.** `setBehaviour` and `setCombatMode` live on
   the group here (`GrpSetBehaviour`/`GrpSetCombatMode`), so
   `grp setBehaviour "CARELESS"; grp setCombatMode "BLUE"`.
3. **Finish with `switchMove "Civil"`** immediately before the frame. Even with
   1 and 2 done, a civilian who can see a burning car or an armed man goes to
   hands and knees within a second or two. `Civil` is the standing civilian state
   in `CfgMovesMC` (`CivilBase` → `Civil`) and setting it directly on the model
   overrides the AI's choice. Pair it with `disableAI "MOVE"` so they do not crawl
   out of frame instead.

Do **not** reach for `disableAI "ANIM"`: with the animation selector gone the
unit freezes mid-transition and hangs half-sunk in the terrain. It is worse than
the problem.

@LoBo's civilians (`LoBo_Civ_*`, `LoBo_CivF_*`) need none of this — they are
reskinned soldier classes and stand where they are put.
