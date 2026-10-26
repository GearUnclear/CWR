# Look lane "light": photoreal light and camera (switch `photoLight`)

Scratch copy `D:\Arma_CWA_worktrees\look-light`, detached at **7b7e3e6** (graphics-remaster
"every tree casts a long golden-hour shadow"). Nothing committed, no branch. Patch:
`lane.patch` (25 files, +1951 / -60; `git apply --check --reverse` passes on the scratch copy).
Build `build/look-light-rwdi`, game `dist/look-light-rwdi`: both are junctions to
`C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-...\scratchpad\ll\{build,dist}`
because D: had 0 bytes free when this lane started (the configure failed with "No space left
on device"; only this lane's own half-made build dir was deleted). That scratchpad may be
cleaned with the session.

## What it looks like

`contact_sheet.png` (480 px panels): Original (Classic) | Overhaul, the eight hero views plus
two of the lane's own (`woods_pine2_evening`, `blue_hour_coast`). `contact_sheet_3col.png` adds
the branch tip (`remastered,photoLight=0`) in the middle. `final\<view>__{classic,off,on}.png`
are the 1920x1080 shots (one game per world, same camera, clock, weather, props, clouds),
`final\<view>__pair.png` the pairs at 960 px a panel. `extra_tank_interior_optics.png`: the
interior and gunner optic views (checked after the meter change below).

Mean absolute difference from Classic at 480 px (0..255 per channel), Overhaul against the
branch tip's: lowsun_fp 17.3 (tip 15.8), noon_fields 19.9 (7.9), forest_edge_golden 24.4 (9.1),
sunrise_ridge 48.4 (13.7), coastline_sunset 25.8 (23.9), overcast_broken 16.0 (13.2), town_night
29.0 (6.8), sinai_desert_noon 32.0 (8.0), woods_pine2_evening 14.1 (17.6), blue_hour_coast 25.5
(18.4). What I see on the sheet:

- noon_fields, sinai_desert_noon: a deep blue sky graded to a pale horizon instead of a pale
  wash; sunlit ground a little darker and warmer, short hard shadows, vehicles dark olive.
- lowsun_fp: blue shade and long blue shadows, a bright hazy sky around the sun, the ground
  brighter, the shaded hulls dark.
- forest_edge_golden, woods_pine2_evening: warm low sun on grass and crowns, deep shadows in
  the trees, saturated greens; the clearest win.
- sunrise_ridge: the backlit sky is bright with a white-gold glow at the sun over a pale blue
  sky (Classic: dim pink), the foreground lifted from near black to readable dark grass.
- coastline_sunset: a golden band on the horizon under a blue sky, a bright sun with its glint
  path on the sea, a tan beach (Classic: pink wash, salmon beach).
- blue_hour_coast (sun 4.9 deg under): a purple-blue sky over an orange horizon band, the sea
  visible (Classic: flat red).
- town_night: about 1 EV darker than Classic, slightly cool, lamps and the fire glow.
- overcast_broken: the smallest change: a little brighter and more contrasty, slightly cooler.

## What was built

1. **Physical sun and sky** (`LightRig.cpp` `ComputePhysicalRig`, `PhotoCamera.cpp`
   `ComputePhysicalSky`). The sun is the sky table's transmittance toward it at the eye
   (Bruneton atmosphere of A7): (0.93, 0.85, 0.74) at 60 deg, (0.72, 0.47, 0.23) at 10 deg, 0
   below the horizon. The sky fill is the table's clear sky (Rayleigh + multiple scattering +
   Mie with its phase, `SkyViewTexel`) integrated over the hemisphere on a 12 x 11 grid of the
   sky-view parametrisation, for an up-facing surface and for walls facing toward and away
   from the sun's azimuth (0.7 ms on the CPU, redone when the sun has moved 0.25 deg). Measured
   in the unit test: noon sun on the ground 0.746 vs sky 0.074 (10:1), sky light very blue
   (r/b 0.24); 10 deg sun 0.088 vs 0.038. Because the Bruneton air is very clean, the rig takes
   the sky light 35 % toward the sun's colour and 1.4 x stronger (`PhysicalSkyFill`, haze
   scattering sunlight), so the clear noon ratio is about 6.5:1 and the shade blue but not
   cyan. Overcast: the clear sky dims by half the cover and 55 % of the sunlight the deck takes
   out comes down as an even grey light (walls get half). Ground bounce: albedo 0.3 in the
   colour of the light on the ground. Moon and the classic night floor as before. Scale: a
   white surface under the 60 deg clear sun and sky shows at the classic noon brightness;
   every other time of day keeps its physical ratio to that (a 10 deg sun is about 7 x less
   light on the ground). The old exposure meter ("sunlit ground at Classic brightness") is not
   used while the camera runs (`rig.exposure` = the `exposure` knob).
2. **World pass writes scene light** (`camEncode`, `litDisplay` in the fragment prelude). With
   the camera on, lit fragments skip the Neutral tonemap and the display saturation, the sky
   dome and fog skip theirs, and `finishFragment` stores linear light: as is up to 0.6, the
   peak channel above folded into the codes left (knee + (1 - knee) t / (1 + t)), hue kept,
   decodable to 64. The frame target stays RGBA8 and every pass keeps its blending space.
   Pre-lit draws are encoded the same way. The sun disk's core (sky mode 3) is 3 x its sprite.
3. **The camera** (post composite, `psPostComposite` camera path; `EngineGL33_Camera.cpp`):
   - meter pass `psPostMeter`: the resolved frame into 64 x 36 RGBA16F texels (mean log2
     luminance of 16 taps, weight, brightest and darkest tap; sky taps count 0.6, centre
     weighted 0.3..1, black-through blocks 0 so an optic's mask or bars do not count), read
     back through a ring of 3 pixel-pack buffers with fences (`glGetBufferSubData` after the
     fence: the backend may not map buffers, `test_buffer_usage_audit`);
   - CPU (`Meter`, `TargetExposureLog2`): weighted percentiles, the mean between the darkest 5
     % and the brightest 3 %; target = middle grey 0.18 for the mean, plus `camEv` (0.3), a
     scene darker than a sunny day (mean log2 -2.3) keeps 0.4 of its darkness, a brighter one
     0.4 of its brightness; a highlight guard pulls 60 % toward keeping the 97th percentile of
     the blocks' brightest taps under 2.2 (curve input); at night (`CameraNight`: the sun from
     -2 to -10 deg, not Classic's night effect, which is 1 at any sun under the horizon and
     held the blue hour as dark as night) `camNight` -2 EV more and never brighter than 1:1;
     clamp -3..+2.5 EV;
   - eye adaptation (`AdaptExposureLog2`): exponential in log2 with `camAdapt` 1 s, 1.5 s when
     opening up to the dark; wall-clock dt capped at 0.25 s; a cut (50 m or 60 deg in a frame),
     the camera turning on and any `triSetLighting` snap to the first meter of a frame after it;
   - white balance (`WhiteBalance`, Bradford): set for daylight (the 60 deg clear sun plus sky
     light is white) and following the light it is in by 15 % at a low sun to 50 % at 35 deg
     (`WhiteBalanceShare`), at strength `camWb` 0.45: noon near neutral and slightly warm,
     golden hour warm, shade and the blue hour blue, overcast grey; at night it fades out and a
     cool night white (camera balanced for (1, 0.9, 0.75), 0.45 at full night) takes over;
   - scene saturation `camSat` 1.15 about luminance, in the same 3x3 as the white balance;
   - filmic curve (`ToneCurve`, Lottes 2016): middle grey 0.18 to 0.18, toe `camContrast` 1.3,
     shoulder `camShoulder` 0.97, white at `camWhite` 16 middle greys, applied to the peak
     channel with the channel ratios kept and sent to white by y^4 near the top (hue-keeping;
     a bright sky stays blue, the low sun orange until it is truly bright). `camCurve` 1 = the
     same curve per channel, 2 = ACES fit, 3 = Khronos Neutral with its toe;
   - glare: bloom only from light past the curve's white (threshold 3, knee 1, capped at 8,
     gain 0.1 x the bloom knob x `camBloom`), the shafts gather the decoded sky capped at 1.5
     and add at 0.06 x the shafts knob; vignette `camVignette` 0.18; triangular dither.
4. **Sky brightness and hue under the camera.** The clear sky dome, the fog colour (all fog of
   lit and pre-lit draws, so the far ground fades into the same sky) and the sky the sea
   reflects take `skyGain` = the physical horizon radiance under the rig's scale over the
   classic fog colour's luminance, times `camSky` 0.85, clamped 0.3..1 (0.6..0.75 by day in the
   final run, 0.47 in the blue hour, 0.98..1 at sunrise and sunset), going to 1 under an overcast deck and as the sky table
   fades at night. The dome's clouds keep their light. The dome's hue is `camSkyHue` (1 =
   physical) in place of A7's `atmoHue` (0.5): the golden band under a blue sky at sunset and
   the blue dusk come from that.
5. **Registry, gate, plan, slot.** `Feature::PhotoLight` (key `photoLight`, step "L1", track
   "look-light", GPU section `composite`), 13 sub-knobs under it (`camEv`, `camAdapt`,
   `camCurve`, `camContrast`, `camShoulder`, `camWhite`, `camSat`, `camWb`, `camSky`,
   `camSkyHue`, `camNight`, `camVignette`, `camBloom`). Gate: off in Classic, with the switch
   off, under night vision, with the post effects off or a debug view on (the world then keeps
   its old tonemap and rig). `PostFrameInputs::camera` asks for the post pass by itself. The
   sky pass hook also opens the remastered programs for the dome and sun disk when only the
   camera wants them. LitConstants slot 70 (`litCamera`: x on, y sun disk gain, z sky gain);
   `litPlanned` shrank to 71..73. `triFeatureState` appends `| cam ev= target= mean= high= wb=
   sky= sun= skyup= reads=` (logged with every capture shot as `feat=`).
6. **Tests**: `test_photo_camera.cpp` (9 cases: encoding round trip, meter cuts and the peak
   percentile, exposure keeps/guards/night, `CameraNight`, adaptation, curve through grey and
   to white with hue kept, white balance, the physical sky's ratios and colours and its CPU
   time, the physical rig's scale and overcast and the switch-off identity, registry, gate and
   plan). `test_feature_switches.cpp`, `test_remaster_lighting.cpp` updated (feature count 27,
   landed 16/27, 14 to 27 sub-knobs, slot 70, 16 post shaders). The whole PoseidonTests suite
   passes except 6 localisation cases (UI/Map legend and equipment, the MP version message) that
   read game data through a `packages\Remaster` junction this scratch copy does not have.
7. **Capture tooling**: `tests/rendering/captures/gen_look_light.py` (hero views on Abel, Eden,
   Sinai with @LoBo, any `label=setting` modes in one game per world, plus the lane's
   `blue_hour_coast`). Here: `sheet.py` (thumbnail sheets), `collect.py` (copies every mode's
   shot and prints the camera readout per shot), `make_deliverables.py`.

## Tried and dropped (every item from real captures, `variants_sheet_t*.png`)

- t1: the per-channel curve against the hue-keeping one: hardly different at thumbnail; the
  hue-keeping one keeps a slightly more orange sun. Kept hue-keeping. ACES and Neutral are
  selectable but were not captured.
- t1/t2: sun disk x8 and bloom from 1.6 (scene light after exposure): a white ball a tenth of
  the frame wide at sunset; x4 and threshold 2 still large. t4: with bloom and shafts off the
  sunrise blow-out went away, so the shafts (gathering the HDR sky near the sun) were the
  white flood: capped at 1.5 and gain 0.35 to 0.06; bloom threshold 3, cap 8; the sun's gain
  only on the sprite's core.
- t2: white balance adapting fully to the current light (sun plus sky): golden hour went grey
  and the coast's beach olive. Replaced by the daylight-anchored balance with a small share.
- t2/t3: the raw table sky light (r/b 0.24, a tenth of the sun): blue shade and near-black
  vehicles. Whitened 35 % toward the sun's colour and x1.4.
- t3/t4: sky gain up to 1.25 at a low sun (the physical horizon is brighter than Classic's
  next to dim ground): blown sunrise sky. Capped at 1. t3 `camSky` 0.75: deeper, kept 0.85.
- t5/t6: contrast 1.35 to 1.45 and 1.25 (1.3 kept), white balance 1 / 0.6 / 0.4 (0.45 kept),
  saturation 1.12 / 1.2 (1.15 kept), `camEv` +0.3 (kept: sunny scenes are bright pictures).
- final run 1: the blue hour exposed as night (Classic's night effect): replaced by
  `CameraNight`.
- t8: A7's hue 0.5 against the physical hue: the physical one gives the real sunset band and
  blue dusk; taken as `camSkyHue` 1.
- Tank optics: holding the exposure while an optics overlay is drawn kept the interior's
  +1.1 EV on the outside view (washed sky). Replaced by black-through meter blocks weighing
  nothing; the optic now meters its own image (ev 0.58).
- Not built: an RGBA16F frame target (the exposure research found no gain and blown pre-lit
  smoke; the folded RGBA8 encoding keeps every pass's blending space and memory), a
  pre-exposure inside the world pass (the classic-program draws in the world pass, stars,
  moon, a sun disk outside the sky pass, could not take it), local tone mapping.

## Cost

`timing.md` (RTX 4060 Ti, 1080p, 4x MSAA, TAA; off and on interleaved three times per view on
five Abel views; one or two other agents' games on the GPU the whole run, `gpu_timing.csv`).
The camera's work is the post pass: the composite section goes from 0.04 to 0.29..0.31 ms
(minimum; 0.61 at sunrise, where the off frame already had 0.17), the whole post pass by
+0.06..+0.18 ms (minimum of three). Frame and world times swing by more than that with the
other games and are not a measurement of this lane. CPU: the physical sky integral 0.7 ms each
time the sun moves 0.25 deg (and once per haze for the reference), the meter's percentiles on
2304 samples, a 3x3 matrix per frame. Memory: one 64 x 36 RGBA16F target and three 36 KB
pixel-pack buffers. On a 3060 expect roughly 1.3 to 1.5 x the post numbers.

## Problems and integration notes

- **The patch is on 7b7e3e6 and does not apply on the moved branch tip 06c7e4d** (B8
  7c21bc7 and B3 06c7e4d landed during this lane; `git apply --check` fails on FeatureSwitches,
  LightRig.hpp, PostStages, EngineGL33.hpp, EngineGL33_Lighting.{hpp,cpp}, the shader files,
  PostFX). Slot 70 is now B3's `litSkySun` and 71 `litNight`: the camera slot needs a new home
  (79, raising `Count` to 80, or one of B4/B7's planned 72/73 if they do not land there).
- Any post stage that reads or writes the frame's colour must take the camera's encoding into
  account: B8's air pass (beams and haze) and B6's SSR, if they add light into the frame, must
  `camDecode` what they read and add scene light the camera then exposes (or add before the
  camera in the composite). B3's moon key (`ComputeKeyLight`) moves moon light into the sun
  slots after the rig: it has to run on the physical rig's output too.
- The look lane "fog" (`look-fog`) works on haze and fog and shares this session's scratchpad;
  the camera scales the fog colour by `skyGain`, so the two must agree on who owns the fog
  colour's brightness.
- Classic-program draws inside the world pass (stars, the moon, the classic dome when the sky
  table and clouds are off) are not encoded: their top codes decode as very bright light (a
  white moon with glare). Intended for the sun and moon, not checked for the stars.
- Vehicles' shaded sides at noon are dark (dark olive in shade under a 72 deg sun is 2 to 3
  stops under the ground); at 480 px the noon Jeep reads almost black. A local exposure (the
  exposure research's bilateral grid) would be the fix; not built.
- Faint banding in the blue-hour sky shows only under a 4x contrast stretch.
- The adaptation runs on wall-clock time: frames under `triSetFixedStep` or paused still adapt
  in real time. Captures converge within the settle frames (a mode change snaps).
- Gunner optics meter the optic's dark vignette ring too (0.5 EV brighter than outdoors).
- Night levels will move with B3's lamp model; `camNight` (-2) was tuned on the 7b7e3e6 night.
- The overcast view changes least of the set.
