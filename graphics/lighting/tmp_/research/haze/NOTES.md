# Haze track: aerial perspective, height haze, morning valley mist

Research prototype, not production code. Two sessions worked on it: the first
built the C++ sky backdrop, the haze shader and rounds 1-5 (stopped when the
research was paused, round 5 Eden half shot); the resumed session (this file)
shot rounds 6-8, added variants 37/38 and the sky-edge seam fix (39-49), ran
the interleaved timing and the time-of-day series, and wrote the comparisons.

## The idea

The current Remastered fog is the classic linear fog (start to end of the view
distance, display space) with the sky's sun glow added toward the sun
(`fogColorToward`). Distance only fades toward one flat colour, so far hills
do not sit in the sky behind them, there is no colour shift with distance, and
the air has no structure (no valleys, no morning, no shadowed air).

The prototype replaces that per fragment with single-scattering aerial
perspective:

1. **Wavelength-dependent extinction** with height: optical depth
   `sigma * chroma * integral exp(-y / H)` along the view ray, computed in
   closed form (`hzExpLength`), scale height 350 m, so low ground is hazier
   than ridges. `chroma` = (0.85, 1.0, 1.2) in the chosen family: blue thins out
   first, so distance shifts toward blue-grey only as far as the sky behind
   does.
2. **The limit colour is the real sky behind the pixel**, not a fog colour.
   C++ part: `Landscape::Draw` calls a new `Engine::CaptureSkyBackdrop()` after
   sky + clouds and before the ground; the GL33 backend blits the SSAA colour
   target into an RGBA8 mipmapped texture (unit 6, sampler `skyBackdrop`).
   The vertex shader projects the view direction, lifted to just above the
   horizon, to the screen (`vSkyClip`); the fragment samples the backdrop at
   mip 3-6 there. Far hills therefore dissolve into the clouds, the sunrise
   glow or the overcast grey that is actually behind them. This is what
   removes the "hills cut out against the sky" look.
3. **In-scatter split into sky light and sun light.** Sky-lit air takes the
   backdrop colour; sunlit air adds a Henyey-Greenstein forward lobe (g 0.72)
   of the sun colour, so the haze glows warm toward a low sun and stays cool
   away from it. Shadowed air keeps only a dimmer zenith-coloured sky term.
4. **Volumetric sun visibility** (variants with `vol`): 12 dithered samples
   along the ray read the existing terrain horizon map (lifted for height
   above ground) and the drifting cloud-shadow noise, so air in a ridge's
   shadow or under a cloud shadow scatters less sun.
5. **Morning valley mist**: an extra layer of flat-topped exponential density
   whose top sits 40 m (noise-varied 18-54 m, plus 1/520 m and 1/1700 m noise
   banks) above the **lowest ground within ~850 m** (14 height-map taps), so it
   pools on valley floors and water and leaves ridges clear. Its weight follows
   the sun: from just before sunrise, full until 22 degrees elevation, gone by
   38 degrees, only with an eastern sun (`hzMorning`). Thick mist is lit as a
   multiple-scattering medium (near-white sky term plus a broad forward lobe).
6. **Near field untouched by construction**: nothing changes inside 150 m
   (only the moved classic edge fog, see below); the effect grows with the
   optical depth beyond that.
7. **Classic edge fog kept for the view-distance edge**: the classic linear
   fog start is moved out to 55% of the fog end in clear weather and blends
   the last stretch into the sky limit colour, so the terrain edge and
   weather fog stay hidden as before.

Everything runs only in the Remastered path. The variant number rides on the
`saturation` knob (`triSetLighting "saturation=1.12+0.001k"`); 1.12 (the
default) is variant 0, which is the current Remastered output bit for bit
(the shader takes the `else` branch with the original fog line).

### Variant map (src/haze_block.glsl)

| id | what |
|---|---|
| 0 | current Remastered (reference) |
| 1-6 | first family: sigma 1.6e-4, chroma (0.72, 1, 1.5), fog-colour limit. 1 air, 2 air+mist, 3 air+mist+vol, 4 thick mist, 5 earlier edge fog, 6 light mist+vol |
| 11-16 | 1-6 with the sky backdrop as limit colour (needs the C++ build) |
| 21-26 | 11-16 with gentler chroma (0.85, 1, 1.2), sigma 1.1e-4, classic edge fog blended in display space |
| 31-36 | 11-16 with the gentler chroma/sigma and a linear edge blend (the chosen family). **31** = air only, **32** = air+mist, **33** = air+mist+vol, **36** = lighter mist+vol |
| 37 | round 7: 33 plus humid morning air (air density x2 with the mist weight), darker shadowed air (0.30) and mie 0.28 |
| 38 | round 7: 31 with grey extinction (chroma 1,1,1) |
| 39-49 | round 8: 29-39 plus the sky-edge seam fix (43 = 33, 47 = 37) |

## Rounds and what is on disk

| run | shaders | views / variants | state |
|---|---|---|---|
| runs/check1 | shaders (shared build, no backdrop) | valley v0/v3 | done |
| runs/cpp1, cpp2chk, cpp3chk | shaders_r1 | first C++ build iterations | done |
| runs/cpp_r2 | shaders_r2 | 4 views, v1/v3/v11/v13/v14/v16 | done |
| runs/cpp_r3 | shaders_r3 | 4 views, v11/v13/v21/v23/v26 + 3-round timing | done |
| runs/cpp_r4 | shaders_r3 | dusk_valley v11-v23 | images only (cut off by the pause, no sheet) |
| runs/cpp_r5 | shaders_r5 | sunrise/valley/dusk/noon, v31/v33/v36 | Abel done, Eden cut off by the pause |
| runs/cpp_r6 | shaders_r5 | sunrise/valley/noon/overcast, classic/v0/v31/v33(/v36); time-of-day 05:15-08:00; 4-round timing | done, 4/4 passed |
| runs/cpp_r7 | shaders_r7 | sunrise/valley/overcast v0/v32/v33/v37/v38; 8-round timing | done, 3/3 passed |
| runs/cpp_r8 | shaders_r8 | valley 05:15 + 06:30, sunrise; classic/v0/v43/v47 | done, 1/1 passed |
| runs/cpp_r9_timing | shaders_r8 | timing only, 3 x 16 readings per mode | done, 1/1 passed |

Every run's game_stdout.log shows the override loading
(`GL33: shader override loaded`) and `GL33 haze: sky backdrop 1440x960, 11 levels`.

## What the real data showed

Images to open first (all three-way, Classic | current Remastered | prototype,
same build, same camera, same sim clock, 960x640 presented frame, 150% render
scale, 4x MSAA). The GPU ms printed on each panel is a single reading taken
while other games shared the GPU; it is not a cost measure (see GPU cost).

| file | view | verdict |
|---|---|---|
| `BEST_valley_sunrise_mist.png` | Abel valley 05:15, sun 6 deg | **The hero result.** Pink sunrise mist lies on the lake and the low ground, islands and ridges stand out of it, and the hard line where the sea met the flat horizon band in current Remastered is gone. Near slope unchanged. Reads as a different game and still as Malden. |
| `BEST_valley_morning.png` | Abel valley 06:30, sun 20 deg | Mist banks on the lake and lowland with clear ridges; the far island dissolves into the sky. Strong and plausible. The lake under the mist goes white-grey (Classic's warm glint on the water is hidden by the mist). |
| `BEST_valley_mist_timeofday.png` | same view at 05:15 / 06:00 / 06:30 / 07:15 / 08:00 | The mist forms with the sun, is thickest around 06:00, thins by 07:15 and is gone at 08:00 (sun 37 deg), as designed. |
| `BEST_sunrise_ridge.png` | Abel 05:08, looking into a 5 deg sun | The 2 km massif now sits in the dawn sky instead of being cut out against it; mist in the far valley glows where the sun reaches it and stays grey in the ridge shadow. Some will read the massif as too washed out. |
| `cmp_vol_sunrise.png` | sunrise_ridge, v32 vs v33 | What the 12-sample sun-visibility march buys: without it the whole far-valley mist glows pink (flat); with it the part in the ridge's shadow is grey and only the sunlit part glows. A clear win at low sun, near invisible at 06:30 and later (`view/r7_valley_morning.png`). |
| `BEST_overcast_broken.png`, `zoom_overcast_broken.png` | Eden 14:30, overcast 0.5 | Hills at 1-3 km go lighter and blue-grey, so the valley gains depth layers; the near field is identical (mean RGB of the bottom third equal). This is the closest thing to "blue haze" in the set, see risks. |
| `cmp_overcast_chroma.png` | same, v33 vs v38 | Far-hill blue minus red: current 11.8, v33 20.2, v38 (grey extinction) 15.6 levels. Most of the cool shift comes from the real sky colour behind the hills, not from the wavelength chroma. |
| `cmp_dusk_valley.png` | Abel 18:55 into the sun (round 5) | Far ridges sink into the warm sky glow; no evening mist by design. Modest. |
| `cmp_noon_fields.png` | Abel 12:30, VD 2500 | Almost nothing changes: the far tree line is under 1 km away and the air is thin. An honest null result at noon. |

Variant judgements:

- **v31 (air only)**: depth layering on long views (overcast, sunrise), no
  change at noon. On its own a refinement, not a transformation.
- **v32 (air + mist, no march)**: the mist is there, but at sunrise it is lit
  uniformly and the far valley turns into a pink wash.
- **v33 / v43 (air + mist + march)**: the recommended look. The seam fix in v43
  removes a vertical seam at the right edge where the "sky behind" direction
  left the screen (`view/seamfix_zoom.png`; rounds 1-7 had it).
- **v36 (lighter mist)**: same character, thinner; the fallback if v33 is too much.
- **v37 / v47 (dawn air)**: denser morning air and darker shadowed air; warmer
  and hazier at sunrise, otherwise close to v33. Not clearly better.
- **v38 (grey extinction)**: reduces the cool shift on the overcast hills by
  about half; the option to offer if the owner calls v31 "blue haze".
- **v1-v16 (first family, chroma 0.72/1/1.5, sigma 1.6e-4)**: far hills went
  visibly blue-teal on overcast (`view/r2_overcast_family.png`); dropped by the
  first session as the "blue haze" the owner dislikes.

Near-field check (pixel diff against current Remastered, round 6): in the
bottom third of sunrise_ridge and noon_fields the largest difference is 0.7
levels and the mean is 0.0; in overcast_broken the bottom-third mean RGB is
identical. In valley_morning the bottom third does change (mean +4/+5/+10 RGB,
`view/diff_valley_v31.png`) because that hillside is 150-600 m below the
camera, and looking toward the sun the forward-scatter lobe brightens it a little.

Known defects in the prototype:

- **Dither of the march** shows as a fine crosshatch in thick mist at 3x zoom
  (`view/t0515_zoom.png`): 12 samples with interleaved gradient noise and no
  temporal accumulation. Hardly visible at 1x.
- **Horizon band**: the sky dome's flat band at the horizon (present in every
  mode) still meets the far edge of the mist in a straight line at 05:15.
- **The mist top is evaluated at the end point**, not along the ray (one layer
  height per pixel). Good enough in these views; a ray crossing several
  valleys gets the last valley's layer.
- **Forest LOD state differs between shots**: the large forest blocks on the
  left of the valley view sometimes render as flat dark blocks and sometimes
  as canopies, in any mode (compare the Classic panels of rounds 5 and 6). Not
  caused by the haze.
- The haze also covers water (`psWaterRemasterMain`), so mist hides lakes and
  the sun path on them.

## GPU cost (honest: not resolved)

Every run shared the one GPU with up to three other games (tri slot holders
from other research tracks were always present), and the GPU timer measures
elapsed GPU time, which includes the other games' work. Three attempts:

- round 6 (`timings_r6.md`): 4 interleaved rounds, 60 frames each;
- round 7 (`timings_r7.md`): 8 interleaved rounds of six modes, 45 frames each;
- round 9 (`timings_r9.md`): 3 rounds x 16 readings per mode, 4 frames apart.

None is consistent: the medians of every Remastered variant sit at 35-50 ms,
and the paired deltas against v0 change sign between rounds (round 9 has v33
cheaper than v0 on valley_morning, which cannot be true). What can be said:

- Classic is the cheapest in every reading, and current Remastered is clearly
  above it (round 7 minima: sunrise 1.3 vs 5.3 ms, valley 5.2 vs 18.1 ms).
- Round 7 paired medians put the haze at +3 to +10 ms frame over v0
  (sunrise: v31 +4.2, v33 +4.9, v37 +8.0; valley: v31 +6.7, v33 +6.8).
- The quietest single readings (round 8, valley 06:30): v0 13.7, v43 22.6,
  v47 23.8 ms frame, so about +9 ms.

Treat "+5 to +10 ms at 1440x960 internal with 4x MSAA" as an order of
magnitude, not a measurement. The structure explains why it is expensive:
the haze runs **per fragment in every remastered pixel shader**, so alpha-
tested forest overdraw multiplies it. Per fragment beyond 150 m:

| part | texture fetches | when |
|---|---|---|
| air (closed-form optical depth, HG phase, sky-backdrop limit colour) | 1 | always (v31 and up) |
| valley mist (lowest ground nearby + two noise banks) | 14 height + 2 noise | morning only |
| sun-visibility march | 12 x (1 height + 2 horizon array + 1 cloud noise) = 48 | vol variants, sun up |

Plus, per frame, one 1440x960 blit and an 11-level mip chain for the sky backdrop.

## What a production version needs

1. **Move it out of the forward shaders into one full-screen pass** after the
   opaque world, reading the resolved depth the post pass already has (AO,
   shafts): `C' = C * T + S` once per pixel, no overdraw. Blended surfaces
   (smoke, particles, glass, water) keep a cheap per-fragment analytic version
   (no march, no mist taps) or read a low-resolution transmittance/inscatter texture.
2. **March at half or quarter resolution** with blue noise, temporal
   accumulation and a depth-aware upsample. That removes the crosshatch and
   most of the cost.
3. **Precompute the mist floor**: a per-island texture of the lowest ground
   within ~850 m (CPU, once at load, beside the horizon map) with the noise
   banks baked in, so 16 taps become 1.
4. **Sky backdrop**: render the sky pass into its own target (or keep the
   copy) and mip it; free the hard-coded unit 6 and handle device reset,
   portrait targets and night vision (the prototype skips those cases).
5. **Drive the mist from weather and time**, not only from an eastern sun:
   humidity or `setFog`, season, wind (bank drift), a mission script command,
   and a fade under heavy overcast. Keep weather fog on the classic path for
   gameplay visibility parity.
6. Keep Classic bit-identical: everything above sits on the Remastered path
   only (the prototype already does; v0 is the original fog line).

## Risks against the owner's taste list

- **Blue haze**: v31/v33 make far hills under a blue sky cooler and lighter
  (far-hill B-R +8 levels on overcast). It grows with distance only and the
  near field is untouched, but it points in the direction the owner rejected.
  Ready mitigations: v38 grey extinction, lower sigma, or tint the limit
  colour toward the horizon grey instead of the sky behind.
- **Wash over the frame**: thick sunrise mist (05:08-06:00) covers much of the
  far half of valley views. It is a time-limited weather state, not a grade,
  but the owner may find the sunrise_ridge massif too washed out; v36 is thinner.
- **Warm evenings**: the haze takes the colour of the sky behind, so at dusk it
  is as warm as the sky (no neutral cast at 18:55), and it never touches hull lighting.
- **Only darker**: no. The haze brightens distance and never darkens the near field.
- No grade, vignette, grain or lens effects are involved.

## Verdict

Promising. The sky-matched aerial perspective is a quiet but real depth
improvement on long views; the terrain-aware morning mist with the sun march
is the "different game" moment (the 05:15 to 06:30 valley shots and
sunrise_ridge). In this per-fragment form the cost is too high and could not be
measured cleanly; a production version must be a deferred, low-resolution,
temporally accumulated pass with a precomputed mist floor.

## Files

- `src/haze_block.glsl`: the haze code (all variants). `scripts/build_override.py
  glsl_dump_remaster <out>` splices it into the dumped engine GLSL.
- `shaders_r8/`: the final override dir (every variant; 0 = current
  Remastered). `shaders_r5/` and `shaders_r7/` are what rounds 5-6 and 7 ran.
- `prototype.patch`: the C++ part (sky backdrop capture, unit 6 sampler),
  built as `D:\Arma_CWA_worktrees\gfx-haze\arma_CWR\dist\gfx-haze-rwdi`. The
  scratch worktree is left in place, detached, nothing committed.
- Research tests: `D:\Arma_CWA_worktrees\gfx-haze\arma_CWR\tests\rendering\research\haze\`
  (`haze_*_r6` to `r9`), generated by `scripts/gen_tests.py`,
  `gen_extra_r6.py`, `gen_r8.py` and `gen_timing_r9.py`.
- `scripts/compare.py`, `scripts/tod_strip.py`: labelled sheets;
  `scripts/timing_table.py`: the timing tables `timings_r6/r7/r9.md`.
- `view/`: working sheets and zooms used for the judgements above.

To reproduce a variant on the scratch build: from `gfx-haze\arma_CWR`,
`run-captures.ps1 -GameDir dist/gfx-haze-rwdi -DataJunction <own junction>
-ShaderOverrideDir <research>\haze\shaders_r8 -Tests <research test>`; in a
test, `43 call capVar` selects v43 (`triSetLighting "saturation=1.163"`).
