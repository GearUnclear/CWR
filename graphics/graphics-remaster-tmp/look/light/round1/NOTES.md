# Look lane "light": photoreal light and camera (switch `photoLight`), critic round 1

Scratch copy `D:\Arma_CWA_worktrees\look-light`, detached, now at **c36ee4b** (the graphics-remaster
tip when this round ended: B6 "calm water and wet streets reflect what is on screen", on top of the
06c7e4d the critics named). Rebased twice: 7b7e3e6 to 06c7e4d (the round-0 diff saved as
`round0/lane.patch`, the detached HEAD moved with `git reset --hard`, the diff re-applied with
`git apply -3`, 14 files merged by hand), then, when B6 landed during the round, 06c7e4d to c36ee4b
the same way (12 files) with the reflections made camera-aware. Nothing committed, no branch.
Patch: `lane.patch` (`git diff --binary`, new files added with `git add -N`; `git apply --check`
passes on a c36ee4b tree). `lane_06c7e4d.patch` is the same lane on 06c7e4d (applies there). Build `arma_CWR/build/look-light-rwdi`, game
`arma_CWR/dist/look-light-rwdi` (junctions into this session's scratchpad, as in round 0).

## What it looks like

`contact_sheet.png`: Original (Classic) | Overhaul at 480 px, the eight hero views plus
`woods_pine2_evening` and `blue_hour_coast`. `contact_sheet_3col.png` puts the branch tip
(`remastered,photoLight=0`) in the middle. `final/<view>__{classic,off,on}.png` are the 1920 x 1080
shots (one game per world, same camera, clock, weather, props, clouds), `final/<view>__pair.png` the
pairs, `final/metrics.txt` the numbers below, `final/udcap.log` the shot log with the camera readout.
`emitters_2x.png`: the sunset sun, the street lamp, the fire and the sunrise sun at 2x, rows Classic,
tip, Overhaul.

Numbers from `tools/metrics.py` on the final shots (Overhaul; Classic in brackets; luma or RGB 0..255):

| critic point | round 0 | now |
|---|---|---|
| noon_fields p99 / ground p99 | 172 / 179 | 200 / 203 (195 / 194) |
| sinai p99 / sand p99 | 161 / 168 | 189 / 193 (203 / 209) |
| shaded vehicles p10 (noon Jeep, Sinai Hummer) | 14, 10 | 20, 14 (36, 26) |
| noon zenith / horizon band | (120,152,199) / (118,154,194) | (103,137,182) / (114,152,193) |
| sunrise rows by the sun / clipped / white area | (225,221,212) / 1.5 % / 428 x 182 | (229,201,174) / 0.11 % / 84 x 76 (the disc behind the cypresses) |
| coastline sun white area / olive band | 167 x 307 (with glare) / 28 % | 19 x 19 (the disc) / 11 % of the band's pixels, band mean (150,142,122) |
| lowsun_fp sun | peak 238 | 25 x 22 px at 255 |
| woods canopy g/b, sky gap clipped | 2.53, clipped | 1.71 (1.29), 0 % |
| blue hour zenith / beach luma | (96,64,84) / 15 | (82,80,100) / 31 (34) |
| town_night ground | (90,90,85) | (57,59,66): cool, darker |
| overcast MAD to Classic at 480 px | 16 | 12 (the tip 13) |

What I see on the sheet: noon and the desert have a deep blue zenith over a paler horizon, bright
ground with real whites and hard short shadows; the golden-hour forest is warm grass under a blue
sky with deep shade; lowsun_fp has a small white sun in a warm hazy glow that fades to blue, long
blue shadows; the sunrise is a warm gold glow round a small sun over a blue-grey sky and a dark
foreground; the sunset a small sun, a golden horizon band and a golden path on a dark sea under a
grey-blue sky; the blue hour a blue-violet sky over an orange band with the sea readable; the town
night is cool blue-grey with a warm fire and smoke; the overcast view is the least changed (a
brighter, greener foreground and a bluer sky).

## What changed this round (every critic point)

1. **HDR encoding** (`PhotoCamera.cpp` `EncodePeakCode`/`DecodePeakCode`, shader `kCameraCodecGLSL`
   `camFold`/`camDecode`). The `t / (1 + t)` fold to 64 is gone. The world now stores light at the
   frame's **pre-exposure** (the camera's exposure when the frame began, `litCamera.w`), so the frame
   is near display level; linear to 0.6, then the peak channel's code on
   `cK + (1 - cK) (1 - (0.6 / m)^0.7) / span` up to 16 (16 codes for 1 to 2, 10 for 2 to 4, 10 for 4
   to 16; the old fold had 5 codes for 4.9 to 64). The composite applies only what the exposure
   moved since (`exposureLog2 - preExposureLog2`); the meter divides the pre-exposure back out. One
   GLSL text (`kCameraCodecGLSL`) is pasted by the lit prelude, the composite, bloom, shafts, meter
   and the air apply (a unit test checks the text and its constants against PhotoCamera.hpp).
2. **Pre-lit and emissive draws keep their hue.** Pre-lit draws (particles, fire, smoke, the far
   horizon) store the original's display colour without exposure; only their fog colour takes the
   pre-exposure. The tone curve goes to white late: `max(y^8, excess)` with `excess` the stops past
   the curve's white over 2.5 (it was `y^4`). The night white balance (camera balanced for a warm
   light, which neutralised lamps and fire) is gone; the camera stays on daylight at night. Additive
   draws are flagged (`litFlags.x` -1) and scaled by `camAdditive` (default 1: with the new fold and
   curve the fire is orange at 1; `variants_r4x_fire_additive.png` has 0.7 / 0.9 / 1).
3. **The sun.** Both quads of the sun sprite draw an analytic disc of 0.45 degrees in the
   sunlight's colour at a fixed pre-exposed level of 12 (`camSunDisc` in `finishFragment`, dimmed
   behind a dome cloud as before): the frame's one clipped white point, round and anti-aliased. The
   sky's light rolls off softly from 1.2 toward 2.4 (curve input; `camSkyRolloff` on the dome, the
   fog colour and the sea's sky), so the aureole round a low sun is a smooth golden gradient under
   white. Light flares (`Scene::DrawFlare`: IsLight decals drawn without the depth test) keep 0.15 of
   their alpha under the camera: added over the bright sky they ran the frame into the top codes, the
   white starburst ball the critics cropped. The vignette now darkens the light before the curve, so
   the sun at a frame corner still clips (it was 238).
4. **Sky.** Under the camera the table's air holds a summer haze (`camHaze` 6 x the reference Mie
   density; `HazeOf`), and the dome, the fog colour and the sea's sky are the table's own radiance at
   the rig's scale (`LightRig::skyScale`, `skyClear`) instead of rows normalised to the original
   dome's brightness times `skyGain` (which flattened the noon sky and darkened the fog). The
   horizon is therefore brighter and paler than the zenith (unit test: 3 x at 0.5 degrees in the
   hazy table) and the haze brighter than the hills it veils. The twilight fallback dome is the
   original's brightness in a night blue (its dusk red made the blue hour mauve). The sky clouds take
   the physical light: the sun as the deck at 2.5 km sees it (so they glow at dawn and dusk) times
   1.3, the sky above, both at the rig's scale.
5. **Exposure.** Daylight reference -3.0 (a sunlit plain meters 0.9 above it) and `kBrightKeep` 0.85:
   noon +0.6 EV. Guards: the 97th percentile of the blocks' brightest taps (now every block counts
   alike, the sky through a canopy included) at 1.3; the sky's own bright end (new fourth meter
   channel: the brightest sky tap per block, the 99.5th percentile, the sun's own neighbourhood left
   out) at 2.2; the sky 0.07 to 0.16 frame heights from a low sun in frame (`SunRingPeak`, full under
   15 degrees of sun, none above 30) at 2.0. The limits sit under the sky's rolloff cap, so the meter
   still sees the sky it guards. Exposure range -3..+5 EV (the blue hour opens to +4.6); `CameraNight`
   starts at -4.5 degrees (was -2) and is full at -12.
6. **Colour.** White balance share 0.35 at a low sun to 0.55 high (was 0.15 to 0.5): the golden hour
   stays warm, green stays green. Scene saturation 1.15 falling by 0.2 toward a low sun. The sky fill
   is the table's own (no whitening toward the sun's colour, which made the shade golden and the
   pines mustard), x 1.15. Ground bounce albedo 0.4 (vehicles' shaded sides). At night a rod shift
   (dim light toward a cool grey, `camScotopic`), the ambient floor at 0.6 of the classic one,
   `camNight` -1.5 EV and at most +0.7 EV of lift: the moonlit ground reads cool, lamp and fire pools
   keep their colour.
7. **Overcast.** Under the camera the sky's own clouds cast nearly opaque shadows (0.08 of the sun,
   `kPhysicalCloudTransmit`) over the share they cover and the gaps get the full sun (no classic
   mean sun is kept). In `overcast_broken` the camera stands in a cloud shadow and the Jeep and the
   hills behind are in sun (`variants_r4x_cloud_shadows.png`: with `cloudShadows=0` the foreground is
   16 to 28 luma brighter).
8. **Rebase and integration.** On 06c7e4d and c36ee4b: `litCamera` moved to slot 79, `Count` 80
   (`LitConstPlanned::End` == `LitConst::Camera`, static_assert; INTEGRATION-PLAN 3.3 updated). B3's
   `ComputeKeyLight` runs on the physical rig's output (the moon key and the night lights work under
   the camera). B8's air apply decodes the frame, adds its haze and beams in pre-exposed light (its
   sky and sun times `litCamera.w`) and stores with `camFold`; the meter reads the frame the composite
   reads (after the reflections and the air). B6's reflections: the trace reads the frame through
   `camDecode` and the sky at the pre-exposure (`frameLin`, `skyLin`), the composite decodes, adds and
   stores with `camFold` (uniform `ssrCamera`); the coastline shot has `ssr=1:on` and no clipped
   pixel on the sea.

Knobs (all under `photoLight`, 16): `camEv` 0.3, `camAdapt` 1, `camCurve` 0, `camContrast` 1.3,
`camShoulder` 0.95, `camWhite` 32 (was 16: a longer shoulder), `camSat` 1.15, `camWb` 0.45, `camSky` 1,
`camSkyHue` 1, `camNight` -1.5, `camVignette` 0.18, `camBloom` 1, new `camHaze` 6, `camScotopic` 1,
`camAdditive` 1. `triFeatureState` shows `| cam ev= target= pre= mean= high= wb= sky= sun= skyup= reads=`.

## Tried and dropped (captures in `variants_*.png`, round 0's in `round0/`)

- `camHaze` 3 / 6 / 10 (`variants_r2_haze_*`): 10 gave a broad white haze dome round a low sun and an
  over-red low sun; 3 little horizon brightening; 6 kept.
- A two-lobe Mie phase (20 % in a g 0.985 lobe) for a sharper aureole (`variants_r4_mie_core_lowsun`):
  a 120 px white ball with a ragged rim at the low sun; reverted.
- Sun-ring guard limit 2.2 and 4.0 with the ring 0.04..0.12: sunrise and sunset ground near black
  (luma 12 to 16) while the flare blob was still in the ring; superseded by the flare fix, the sky
  rolloff and the ring at 0.07..0.16 with limit 2.0.
- `camAdditive` 0.7 / 0.8 / 0.9: dimmer, browner fire; 1 kept.
- `camSat` 1.05 against 1.15 at a low sun (`variants_r3_saturation`): the pines less yellow but the
  golden forest flatter; replaced by the saturation falling toward a low sun.
- White balance share 0.2 at a low sun (to keep the sunset sky blue): made pines more mustard and
  did not make the sky bluer; 0.35 kept.

## Cost

See `timing.md` and `timing.csv` (RTX 4060 Ti, 1080p, 4x MSAA, TAA; off and on interleaved three
times on five Abel views, `triGpuTimes` smoothed over 150 frames; measured on 06c7e4d with this
round's build before the flare and reflection edits, which add no pass). The look-fog lane's game
shared the GPU during the run, so frame and world times swing by more than the lane costs (frame
minimum off / on: lowsun 10.85 / 8.20, noon 11.32 / 11.02, forest 9.24 / 9.34, sunrise 5.69 / 6.12,
night 10.57 / 9.99 ms). The composite section, where the camera works, goes from 0.04 to 0.30 to
0.66 ms (minimum of three; the highest at the forest edge); the whole post pass moves by -0.3 to
+0.5 ms within that noise. On a 3060 expect about 1.3 to 1.5 x the post numbers. GPU work added by the lane is in the post pass (the
meter pass, its readback and the camera's composite) plus a few ALU in every lit fragment's encode,
the dome's rolloff and the air apply's decode and encode. CPU: the physical sky integral (about 0.7
ms when the sun moves 0.25 degrees), the meter's percentiles and the two new guards on 2304
samples, a 3x3 matrix per frame. Memory: one 64 x 36 RGBA16F target and three 36 KB pixel-pack
buffers.

## Problems and integration notes

- Combat light (B4) and wet materials (B7) are not on c36ee4b. Any pass of theirs that reads or
  writes the frame's colour while `litCamera.x` is 1 must `camDecode` what it reads and `camFold`
  what it writes (as the air apply and the reflections now do); one that writes `toDisplay` of light
  above 1 would land on the top code, which the camera decodes as 16 (white sparkles). Below the knee
  the stored frame is display-like, so passes that only blend in the usual range look right
  unchanged. B4/B7 take slots 72/73; the camera is at 79.
- The look lane "fog" (`look-fog`) works on haze: the camera now owns the sky table's haze under the
  camera (`camHaze`) and the sky's brightness (`skyScale`); the two must agree on who sets them.
- The fire's look varies a lot from shot to shot (particle timing): r-b>60 pixel counts in the fire
  box ranged 810 to 2580 between Overhaul shots of this round (Classic 4524 to 5351). The flame is
  orange and the smoke warm in every shot; its size is the particle system's.
- `overcast_broken` changes least (MAD 12 at 480 px; the tip is 13 from Classic): the camera stands
  in a cloud shadow, so the frame is the diffuse light; the sunlit Jeep and hills show the shadows.
- The sunrise and sunset foregrounds are darker than the tip's (exposure set by the sky by the sun,
  as a photograph into the sun); the sunset's upper sky is a grey-blue rather than blue.
- Shaded sides of dark vehicles at noon stay dark (p10 20 and 14 against Classic's 36 and 26); a
  local exposure would be the real fix (not built).
- The street lamp's flare is now faint (0.15); its glow is the bulb's own light and the camera's
  bloom.
- Classic-program draws in the world pass (the moon, stars) are not encoded; at pre-exposure their
  display values decode as about the same display light, so they look as in the tip (the moon's top
  codes still decode bright).
- The adaptation runs on wall-clock time (frames under `triSetFixedStep` still adapt in real time).
- Unit tests on c36ee4b: the full PoseidonTests run (3316 cases) fails only the 6 localisation cases that read game data through a
  `packages\Remaster` junction this scratch copy does not have (as in round 0); the hidden fixture
  test `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` fails on the graphics-remaster
  tip's own build too.
