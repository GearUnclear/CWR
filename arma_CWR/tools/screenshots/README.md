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
| `portraits/portrait_vanilla` | Dossier portraits, stock roster: the nine Legend bodies the two vanilla faction descriptors can produce, at four faces each. Generated from `portrait_roster.py`; see `guerrilla-mode/core/portraits/README.md` |
| `portraits/portrait_lobo` | The same for @LoBo's 23 Legend bodies. Must run against the real Classic install, not the shadow: `--mods-dir ".."` resolves against the game's own working directory and the shadow has no `@LoBo` sibling |
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
> once per @LoBo install.

> **The M60A1 wrecks seat correctly now (2026-08-08).** `LoBo_M60A1_wreck` and
> `LoBo_M60A1_wreck2` used to need a `setPos [x, y, 4]` workaround: at plain
> `createVehicle` height the whole tank was underground. That was **not** a
> missing LOD (no LoBoWreck model has a LandContact LOD, including the eight that
> always seated fine). A static prop is seated at
> `terrainY + shape->BoundingCenter().Y` (`Entity::PlaceOnSurface`, Static
> branch, `World/Simulation/Simul.cpp`), which puts the model's *authored
> origin* on the ground, and both M60A1 models were authored with that origin
> above the mesh (roof 0.45 m / 0.92 m below it). Two layers fix it, and they
> agree on the seat: `tools/lobo/fix-lobo-model-origin.ps1` repairs
> `boundingCenter.Y` in the p3d at source (run it once per @LoBo install,
> alongside `fix-lobo-scope.ps1`), and the engine seats any model whose whole
> mesh sits at or below its own origin on its lowest vertex, with a WARN
> (`StaticSeatOffsetY`). Spawn them at `[x, y, 0]` like any other prop now; an
> empty desert frame at `p03_m60a1_wreck` / `p03b_m60a1_wreck2` means both
> layers failed, which a current build should make impossible.
> One framing note stands regardless: a real vehicle with `setDammage 1` +
> `inflame true` still reads better on camera than any static wreck.

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
  `PoseidonTools mod doctor <@LoBo> --fix` (the older `tools/lobo/fix-lobo-*.ps1`
  scripts now just wrap it; the `@lobofixup` patched pbos are no longer needed)

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
(ammo counter, action menu, crosshair) is gated on `_ui && IsUIEnabled() && !_cameraEffect`
in `World::Draw` (`engine/Poseidon/World/World.cpp:1603`). Without an active camera
effect every frame is stamped with UI. Its own framing is irrelevant, because
`triSetView` overrides it.

**A long lens needs a camera target.** `camSetFov` on its own does nothing:
`CameraVehicle::CamEffectFOV` (`World/Scene/Camera/CameraHold.cpp`) derives the
FOV from the camera's target and keeps the old value (0.7) while there is none.
Give the camera a far-away target, then `camSetFov`, then `camCommit 0`; the
FOV holds and `triSetView` still sets the framing. 0.3 to 0.45 reads far more
like a photograph than the default wide angle.

**`triScreenshot` must be given a string literal.** The runner parses the literal
out of the statement to maintain its own sequence counter, so a `format [...]`
label desyncs it and the run dies on `FAIL:screenshot_not_written`.

### Where in the frame the capture is taken

`triScreenshot` has two capture points and the shoot chooses which one it wants.

**Default (mid-frame, pre-gamma).** `TriScreenshot` calls `Screenshot(path)` and
then `FlushPendingScreenshot()` in the same statement, so the readback happens
where the script runs: whatever is in the default framebuffer at that instant.
That is *before* this frame's `ApplyGammaPass` and before `DebugOverlay::Render`
(`EngineGL33::BackToFront`). The default gamma is 1.2 and the gamma pass only
short-circuits between 0.999 and 1.001, so a default capture is genuinely darker
and more contrasty than the frame the player is shown. Every shoot in this
directory, and every other integration lane that screenshots, is on this mode and
is calibrated on those pixels.

**`triCaptureAtPresent true` (presented frame).** `TriScreenshot` then only
enqueues the path, and `BackToFront` writes the file at the next swap through
`CaptureScreenshotIfPending`: after the gamma pass, after the overlay, the exact
pixels about to be presented. Only the Guerrilla journal portrait shoot uses this,
because it calibrates a tone recipe on the capture and has to calibrate it on what
a player would see. Turn it back off (`triCaptureAtPresent false`) if the same
lane later takes an ordinary capture. The runner needs no change: it already polls
for the file after the statement and holds the next statement until it appears, so
two deferred captures cannot race for the single pending path.

**Either way, a scene change and its capture must not share a statement.** The
deferred mode moves the capture to the end of the frame, not to a later frame: the
sim still has to run for a pose, a mimic or a camera commit to settle. Keep a
`triSimFrames` between the change and the `triScreenshot`, exactly as before.

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
- **Posture.** Mixing `setUnitPos "UP"` and `"DOWN"` across a group reads as a
  firefight; all-standing reads as a parade. Use all-standing for patrols and
  checkpoint guards, where a prone man just disappears into the ground.
  `"MIDDLE"` does not exist in this engine: the only names are Up, Down and
  Auto (`GetEnumNames(UnitPosition)`, `AI/AIRadioImpl.cpp`), so `"MIDDLE"` is
  silently ignored and the man stays on Auto. `lobo_sinai_stories` still asks
  for it. For a crouch, play a kneeling animation instead.
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

@LoBo's civilians (`LoBo_Civ_*`, `LoBo_CivF_*`) are reskinned soldier classes
and do not need step 1. They are not immune to the rest, though: on the
2026-10-12 Lebanon80 shoot, villagers near armed men dropped into a weaponless
crouched combat pose, and `switchMove "Civil"` right before each frame was what
stood them back up.

## Notes from the 2026-10-12 Remaster shoot

Three shoots (Malden stock, Sinai and Lebanon80 on @LoBo) ran on the full
Remaster shadow (`packages/RemasterUD`) with `triCaptureAtPresent true`. Their
scripts lived outside the repo; these are the lessons they paid for.

### Running a one-off shoot from outside the repo

- A toml outside the repo works if every path in it is absolute:
  `data_dir`, `mission`, and any `--mod` folder (forward slashes). tri copies
  the mission to a temp folder, so a mission folder in scratch is never written
  into the data dir. Pass the binary with `--game-dir`.
- The shadow carries its own `@LoBo`, so `--mods-dir . --mod
  "@LoBo;<repo>/arma_CWR/tools/screenshots/@udshowcase"` loads both, with
  `@udshowcase` referenced in place.
- A `dist/` folder built by an agent can lack `PoseidonTools.exe`; the copy
  under `build/<dir>/apps/tools/Tools/` works for `terrain objects` and
  `pbo show`.
- Lower `assert_timeout` to about 90 for shoots. A statement that never writes
  its screenshot otherwise hangs the run for the full timeout.

### Staging

- **Snapshot every group at spawn, not just the foes.** A man who dies leaves
  his group, so `units grp select 2` can come back empty and the frame after it
  is never written. Pick men from a list saved when they were created.
- **`createVehicle` moves things off occupied spots.** It silently shifts the
  object by metres, which also shifts any camera anchored to it. Follow every
  spawn with a `setPos` to the intended point.
- **Burning vehicles explode.** The blast kills or flattens men within about
  10 to 15 m, so stage the fire first and keep people out of that ring.
  `inflame` on a vehicle that is already dead does not relight it.
- **Vanilla wrecks on the Remaster may only smoke.** On Malden a destroyed Ural
  gave smoke and no flame, and kept its paint. The flames in those frames came
  from `Fire` campfire objects placed against the hull, plus the static
  `UralWreck` model. @LoBo hulls burned normally.
- **Walking is hard to get.** `doMove` near wrecks or corpses turns into a
  combat run even under CARELESS and LIMITED, and a group `move` under SAFE and
  LIMITED barely moves anyone. What worked: `disableAI "MOVE"` on marks plus a
  walk animation played in place. The animation drifts the man about 0.6 m per
  sim frame, so set the marks to allow for it.
- **Moving armour drifts.** Trailing vehicles leave the road, and tank exhaust
  renders as a blue haze. A parked column with crews turned out reads better.
- **Checkpoint guards aim at the queue.** Golani guards raised their rifles at
  civilians even in SAFE. Pose them, or frame the guard from behind.
- **Rooftops work.** `setPos [east, north, h]` with `h` a little above the roof
  drops a man onto it; a point about 2 m off the footprint falls to the street.
- **Keep launchers out of close cameras.** Men given a target swing AT tubes
  across a lens a few metres away.
- @LoBo's `LoBo_Terror_01E` (the Hizballah civilian-outfit body) renders as a
  white thobe and red keffiyeh; strip its weapons for an undercover frame.

### Camera placement

- **Set the eye height from the ground under the lens**, not from the subject.
  Fixed offsets put cameras inside a hillside, a wall or a willow on Malden, and
  inside the terrain on the Lebanon80 Hermon road and in the Aley olive grove.
- Ground texture goes soft very close to a low lens; a longer FOV from further
  back hides it.
- Lebanon80's heightmap (4WVR) stores int16 heights scaled by 1/22.222; parsing
  it is enough to plan eye points and lines of sight offline.

### Light

| World | Usable window | Notes |
| --- | --- | --- |
| Sinai | 15:00 daylight, 15:40 golden hour | East-facing slopes go dull and purple after about 15:45. Fire-lit "night" only works 17:05 to 17:20; after 17:30 even a post-gamma capture is near black. |
| Lebanon80 | 13:30 to 15:40 | Pink from 16:00, night by about 17:00. The sun stands in the north at midday, so the world's latitude puts it in the southern hemisphere. |
| Malden | Dawn and dusk both shoot well | Night frames need a fire in shot. |

Night needs a light source in frame. A vehicle's ammunition cook-off lights a
whole street, but flat grey; one frame at 20:45 on Lebanon80 also needed in-game
brightness 1.4.
