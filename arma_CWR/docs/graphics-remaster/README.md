# Remastered lighting

The Remastered lighting is a new look for the GL 3.3 renderer of Uslu dur!. It
uses only the models and textures the game already ships. The stock renderer
adds sun and ambient light per vertex and clips the sum before texturing, so
most surfaces show bare texture colour with little shading. Remastered lights
every pixel in linear light and runs the frame through a metered camera.

What changes on screen:

- **Light and shadow.** A physically based sun and sky with an exposure meter
  and a filmic curve; soft sun shadows, long tree and building shadows at a low
  sun, hill shadows at any range, drifting cloud shadows taken from the sky's
  own clouds; ambient occlusion that darkens only sky light.
- **Sky and air.** A physically based clear sky under the shipped clouds;
  volumetric fog (haze, dust, humid ground air, weather and night fog, valley
  mist at dawn) with sun beams through it.
- **Water and shine.** Waves and a sun path on the sea, screen-space
  reflections on calm water, wet weather in which only puddles reflect.
- **Night and combat.** Moonlight, lamps, headlights and fires lit per pixel;
  muzzle flashes, flares and fires light the ground; explosions scale with
  their charge.
- **Image quality.** Temporal anti-aliasing over 4x MSAA, terrain detail from
  each square's own texture.

## The Lighting row

Options > Graphics > **Lighting** chooses **Classic** (the stock look) or
**Remastered** (the default). The setting is stored in `graphics.cfg`.
A few changes are not tied to the row and also apply with Classic: 4x MSAA by
default, three-entry config colours read correctly (street lamps), and
explosions that scale with their charge.

## AI sight follows the drawn fog

AI never sees through fog the player cannot see through.

- **What cuts AI sight.** With **Remastered**, the AI's line of sight is cut
  by all the fog the renderer draws, in place of the classic fog: haze, dust,
  humid ground air, weather fog (denser at night, as drawn) and valley mist,
  up to the view-distance edge where the drawn world ends. For the valley
  mist the AI takes the banks at their thickest wherever they drift, so it is
  never in clear air where mist is drawn; it can be a little blinder than the
  average drawn mist.
- **Classic weather floor.** Heavy fog and rain are never drawn thinner than
  the original game's fog at the same distance, and AI sight follows that. A
  clear sky is unchanged: there the drawn air is clearer than the classic fog
  was, and so is AI sight. In light or medium weather (for example fog 0.3 or
  rain 0.5) the drawn air past about a third of the view distance can stay up
  to about a tenth clearer than the classic fog.
- **Both ends.** Haze, dust and mist thin out near an eye. That thinning is
  taken at both ends of a line and the lesser view wins, so an AI inside a
  mist bank sees a player on a ridge no better than that player sees the AI.
- **Cockpits and night vision.** The fog is drawn in vehicle cockpit and
  interior views, including the world seen through their glass, and with
  night vision, so the player sees what the AI sees.
- **Classic.** A machine that draws the classic fog gives its AI the classic
  fog: the Lighting row is on Classic, the remastered shaders are not active
  (they failed to build, or the engine's lighting switch is on Classic), or
  the post effects are off.
- **Who decides.** The fog the AI uses comes from simulation state (the synced
  weather and time with default tuning), never from per-client fog settings,
  so machines in a multiplayer game agree. The machine that simulates an AI
  decides; a dedicated server or headless client applies no graphics options
  and uses the default, Remastered.

Code: `engine/Poseidon/Graphics/Remaster/VolumetricFog.{hpp,cpp}` (the fog
medium, the classic weather floor `FogClassicFloor`, the mist bound
`FogSightMistBound` and the sight query `FogSightTransmittance`),
`engine/Poseidon/World/Terrain/TerrainLighting.cpp` (`FogSightClassic`,
`FogSightDrawsClassic`, the snapshot published each simulation step) and
`engine/Poseidon/World/Detection/Target.cpp` (where visibility takes the fog).
Tests: `tests/unit/engine/Poseidon/World/test_fog_sight_link.cpp` and
`tests/unit/engine/Poseidon/Graphics/test_volumetric_fog.cpp` (tag
`[aisight]`); in game, `tests/rendering/captures/cockpit_fog_abel` checks the
fog through a helicopter canopy and a jeep windscreen.

## Build and test

From `arma_CWR` (see `arma_CWR/CLAUDE.md` for the toolchain):

```powershell
cmake --preset win-x64-clang-rwdi            # Linux: linux-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi
ctest --test-dir build/win-x64-clang-rwdi --output-on-failure
```

The lighting's own unit tests carry the `[remaster]` tag:

```powershell
build/win-x64-clang-rwdi/tests/unit/engine/Poseidon/PoseidonTests.exe "[remaster]"
```

Run the tests from a drive that has a `\tmp` folder (see the `/tmp` note in
`arma_CWR/CLAUDE.md`).

With game data:

- `tests/rendering/captures/` shoots matched Classic and Remastered pictures on
  Abel, Eden and Sinai with `@LoBo` through Trident, with GPU timings
  (`README.md` there).
- `tests/rendering/captures/fogcheck/` measures flicker and motion in fog
  recordings. Judge any change to the fog or other temporal effects in motion,
  not by a flicker number alone.

Without game data, `tests/rendering/lighting_lab/` renders a synthetic scene
through the engine's own shaders.

## Known issues

- **Cost (issue #216).** The long views (the dawn valley, the Eden long view)
  run at 27 to 34 ms per frame on an RTX 4060 Ti, against a target of 60 fps
  at 1080p on an RTX 3060 class card. Most of the extra cost is CPU time per
  draw call in the new draw path, and it shows with every feature switched
  off. About 30 post shader programs also compile on the first world frame.

## More

- `DESIGN.md`: how each piece works, the calibration and the tuning knobs.
- `ITERATIONS.md`: the change log, with captures, costs and verdicts.
- `research/`: the October 2026 research run and the integration plan
  (dated snapshots).
