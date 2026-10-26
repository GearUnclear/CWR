# Remastered water: research notes

Owner's complaint: "the water is so much less defined, looks so smooth" (Remastered vs Classic).
Scope: research prototype only. Every change here is a shader override loaded through
`CWR_SHADER_OVERRIDE_DIR`; no engine source was edited and the shared `dist/lighting-rwdi`
build was not rebuilt.

## 1. What actually draws the sea (the brief's diagnosis was about the wrong shader)

The brief pointed at `s_psWaterRemasterMain` (flat-normal sky reflection) and Classic's
`s_psWaterGLSL` bump glint. Neither draws the sea.

- Landscape water faces carry only `SpecularTexture`: `LandscapeRender.cpp:320-322`
  (`// const int WaterFlags = IsWater|SpecularTexture;` then `const int WaterFlags = SpecularTexture;`).
  `BuildRenderPassDescriptor` therefore routes them to `ShaderFamily::Detail`, i.e. **PSDetail**
  with the `SpecularTex` multitexture format and `TexGenMode::Detail`.
- So the sea is shaded by `psDetail` (Classic) and `psDetailRemasterMain` (Remastered), with
  - tex0 = the animated sea texture (`more_anim.01..07.pac`, 512 px, one repeat per 50 m),
  - tex1 = `data\specular_dx.paa` (AI88, luminance 255 everywhere, alpha 51-170: a ripple
    pattern) at 32 repeats per 50 m (`vUV1 = 32 x vUV0`).
- Proven in game, not only by reading: `runs/debug` painted diagnostic bands into the
  `psWater` and `psWaterRemasterMain` overrides and none appeared on the sea; `runs/debug2`
  painted them into `psDetail` / `psDetailRemasterMain` and they covered the sea. The same
  run read back the sea's normal (straight up), its material specular (colour about 0.6-0.7,
  power about 5-14) and the UV mapping (1 repeat per 50 m).
- Side findings about the dead PSWater path: `misc\waveBumpMap.paa`, named by
  `remaster.cpp` `CfgDetailTextures >> waterBump`, does not exist in the Remaster data (all DTA
  pbos, the shadow and both Steam installs searched) and `GetWaterBumpMap()` has no callers.
  PSWater binds the detail texture (`detail_dx.paa`, luminance 255) as tex1, so its "bump
  normal" is the constant `-(1,1,1)`: Classic's PSWater glint and the Remastered "sparkle" are a
  per-frame constant over the whole surface. Harmless today because nothing selects PSWater,
  but if anyone restores `IsWater`, the Remastered version adds `6 x sun` to all water for some
  sun directions.

### Why Classic looks defined and Remastered looks smooth

- Classic `psDetail`: `vColor * t0 * (t1.a * 2) + vSpecColor`. By day `vColor` clamps to 1, so the
  sea texture is shown at full contrast; the sun path is the per-vertex Blinn-Phong specular
  (`vSpecColor`) added on top, so the texture stays visible inside the path.
- Remastered `psDetailRemasterMain` -> `shadeRemastered` + `glossy()` with the flat normal:
  - the material's specular power is about 5-14, so the normalised Blinn-Phong lobe is very
    broad: a smooth blurred column at a low sun, and at noon a near-uniform pale sheen over the
    whole sea even with the sun behind the camera;
  - the sky reflection `skyIrradiance(reflect(-v, up)) * gloss * (0.04 + 0.96 fv) * 0.6` is a
    smooth gradient that brightens toward the horizon, a pale veil;
  - both are additive and smooth, so they lift the mean and flatten the texture.
  Measured over the water region (wave-scale luminance sd, 0-255): sea_noon Classic 14.0 vs
  Remastered 6.9; coast_shore_midsun 24.6 vs 15.4.

## 2. Technique (variants A, B, C)

All three replace only the sea branch of `psDetailRemasterMain` (sea = a VSTransform draw whose
material specular luminance is above 0.3; the terrain's is about 0.08). Everything else in that
shader is stock. Source: `src/water_template.glsl`, constants per variant in `src/make_variants.py`
and `shaders_<id>/params.txt`.

**Wave normal (all variants).** No bump map exists, so the normal is derived from textures the
game already has: the sea texture's luminance is treated as a height field. The UV-to-world
Jacobian is solved per pixel from screen derivatives (exact on the flat sea, so no tangent frame
or C++ is needed), and the finite-difference step follows the pixel footprint (minor axis, or
major/16 for the 16x anisotropic filter), so the gradient is band-limited by the same mip chain
that filters the colour. 2 extra texture taps.

- **A, "stock + wave normal"**: the stock remastered shading (`shadeRemastered` body plus the
  stock `glossy()` sun and sky terms) evaluated with the wave normal. Minimal change.
- **B, "Fresnel sky + glint"**: A's normal (strength 0.8 m per luminance unit) with a new water
  lighting model:
  - body = the lit sea texture (`shadeRemastered` without `glossy`);
  - sky reflection = the visible sky in the reflected direction (`fogColorToward(r)`, the same
    sky colour with sun glow the remastered fog uses), weighted by Schlick Fresnel of the wave
    normal. The mean weight is kept low (0.3 x the flat-water Fresnel) so the sea never turns
    into a pale mirror at grazing range, while the wave-driven part of the Fresnel term goes in
    at full weight: facets tilted away from the viewer show sky, facets tilted toward show water.
    That variation is where most of the definition comes from;
  - sun = the material's own broad lobe at 0.35 (keeps some of Classic's sheen), plus a broad
    normalised Blinn-Phong lobe (power 32, x2.5) and a tight sparkle (power 1500), all on the
    wave normal, times Schlick(v.h), shadowed by the existing sun visibility (shadow maps, hill
    shadow, cloud shadow);
  - wave strength scaled down where the footprint is longer than the 16:1 the anisotropic filter
    covers (floor 0.3).
- **C, "multi-scale"**: B plus a swell layer (the same sea texture at 0.27x frequency, rotated
  37 degrees, 3 taps) and a ripple layer (`specular_dx` alpha at its 32x tiling, 2 taps), the
  whole normal faded from 400 m to 3 km (floor 0.35), and the sparkle lobe widened with the pixel
  footprint (Toksvig-style) so distant glitter turns into sheen instead of sub-pixel sparks.

## 3. Views and how they were shot

Research-only test `arma_CWR/tests/rendering/research/water/abel_water.test.sqf` (git-excluded),
run through `tests/rendering/captures/run-captures.ps1` with `-DataJunction RemasterUD-water`
(cleanup stops only games started from that junction) and `-ShaderOverrideDir shaders_<id>`.
Abel west coast, beach at [1550, 5150] (~3.5 m ASL, waterline ~20 m west, open sea along 278):

| view | eye / heading / pitch | time (1985-07-05) | sun |
|---|---|---|---|
| coastline_sunset | 3 m / 278 / -4 | 18:35 | 8.4 deg, az 293, 15 deg right |
| sea_noon | 12 m / 278 / -12 | 12:30 | 71.9 deg, az 202, left and behind |
| sea_away_from_sun | 3 m / 278 / -5 | 09:00 | 48.4 deg, az 100, behind |
| coast_shore_midsun | 15 m / 278 / -22 | 16:30 | 31.6 deg, az 275, ahead (glint ~24 m out) |

The 12 m and 15 m eyes stand in for a bluff over the beach. "Remastered now" is the stock
shader shot in the same pass. Final images come from pass 1 of three identical passes
(`runs/<id>-final1..3`); the pass-1 metrics and those of tuning round r4 (same B and C shaders)
agree closely (fine sd within 1.3, means within 1.5).

## 4. Results on real data

Look at `images/overview_all.png` first (one row per view: Classic | now | A | B | C), then the
three-way images `images/triptych_<view>__<id>.png` (Classic | Remastered now | Remastered +
variant, full 960x640 each).

Water-region luminance (0-255, `cmp/final1/metrics.md`). "fine sd" = sd of a 9x9 high-pass, the
wave detail; "wave sd" = sd of the lightly blurred image (includes the sun path, so compare it
within a view only).

| view | Classic mean / fine sd / wave sd | now | A | B | C |
|---|---|---|---|---|---|
| coastline_sunset | 70 / 3.7 / 29.4 | 100 / 3.0 / 29.1 | 96 / 12.3 / 24.7 | 112 / 22.2 / 29.5 | 113 / 23.7 / 29.3 |
| sea_noon | 145 / 6.1 / 14.0 | 129 / 4.4 / 6.9 | 130 / 7.7 / 7.3 | 125 / 10.1 / 8.3 | 128 / 12.7 / 8.9 |
| sea_away_from_sun | 112 / 7.4 / 6.5 | 114 / 5.7 / 7.4 | 115 / 6.1 / 6.5 | 124 / 14.4 / 6.3 | 124 / 15.1 / 6.7 |
| coast_shore_midsun | 176 / 7.0 / 24.6 | 154 / 3.5 / 15.4 | 151 / 8.6 / 14.6 | 138 / 12.4 / 17.7 | 139 / 13.3 / 18.7 |

What the images show:

- **coastline_sunset.** Now: a smooth, blurred copper column on flat water. B and C: wave rows
  across the whole sea and a broken, glittering warm path that stays textured to the horizon.
  The path is narrower than Classic's broad per-vertex column. The far water is lighter and
  greyer than Classic (mean 112 vs 70; now is 100) because it picks up the sky; that is where I
  would expect a "wash" objection. C's rows are less regular than B's thanks to the swell layer.
- **sea_noon (sun behind).** Now: an even grey-blue sheet. B and C: visible wave relief over the
  whole frame from the Fresnel variation, no glare. Fine detail is above Classic; Classic's large
  blotchy variation (whitecaps on its brighter, greener sea) is still stronger (wave sd 14 vs 9).
  The water reads cooler and bluer than Classic, as now already does.
- **sea_away_from_sun.** B and C give the near-shore water clear wave relief with sky-lit crests;
  the far sea is like now. Mean rises 114 -> 124 (crests reflect sky); it does not read as a haze.
- **coast_shore_midsun.** Now: a pale, smooth sheen patch on a washed-out sea. B and C: a
  glittering glint patch and column, wave relief everywhere, darker troughs. Darker on average
  than now (138 vs 154) and much darker than Classic's yellow-green sheen (176), but with brighter
  highlights, so not "only darker"; the colour is the owner's call.
- **A** adds fine detail near the camera and glitter at the near end of the sunset path, but at
  range and at noon it is close to now. On its own it does not fix the complaint.
- Classic is untouched: Classic never loads the override, and the Classic shot of each pass-1
  variant run matches the stock run's (mean abs difference 0.01-0.16 where the sea animation frame
  lined up, up to 4.7 where it did not). Non-water pixels: the beach is identical; the sky near the
  sun differs by at most 28/255 at single pixels (bloom from the brighter glints).

## 5. GPU cost

RTX 4060 Ti, 1440x960 internal (960x640 at 150% render scale), 4x MSAA. `cost.md` has the medians
over three passes. **The cost could not be measured reliably**: other research agents' games
shared the GPU during every run (all four global tri slots were often taken), and the spread
swamps any shader difference. Classic's own frame time on one view ranged 0.55-4.69 ms over 12
shots, and variant A, which adds only two texture taps, came out anywhere from -0.14 to +1.61 ms
of world time against now depending on the view. By instruction count the sea branch adds 2
texture taps (A, B) or 7 (C) and roughly 60-100 ALU per water pixel, so I expect well under
0.1 ms at this resolution; that is an estimate, not a measurement. A production change needs a
timing run on an idle machine.

## 6. Verdict

- The complaint is real, and the cause is the remastered sea shading (flat normal, a very broad
  material lobe and a smooth additive sky term), not the dead PSWater shader.
- B and C fix it on all four views with textures the game already ships: the sea has wave relief
  facing toward and away from the sun, and the sun lays a broken glittering path instead of a
  blurred column. Fine detail beats Classic everywhere; Classic keeps more large-scale colour
  variation at noon. **C is my pick** (more natural, less regular rows at range); B is the cheaper
  fallback; A is not enough.
- Taste risks for the owner: the sunset far sea is lighter and greyer than Classic; noon and
  mid-sun water is cooler and bluer than Classic's yellow-green.
- Not verified (stills only): temporal stability. The sea texture switches frame every ~0.14 s
  (7 frames, ping-pong, hard switch), so the derived normals will pop with it, more visibly than
  the colour did; and the pixel-high rows at grazing range may crawl when the camera moves.
  Someone has to watch it in motion before anything ships.

## 7. What a production version needs

- **Route the sea explicitly (C++).** Give landscape water faces their own render-flag bit (or
  restore an `IsWater`-style family) that maps to a dedicated remastered water program, instead
  of recognising the sea by its material's specular colour (the prototype's `isSea()` would also
  catch any other draw with `SpecularTexture` and a bright specular). Keep Classic on its current
  PSDetail path so it stays bit-identical.
- **Bind what the shader needs.** Keep `specular_dx` on unit 1 (the ripple layer). To stop the
  normal popping, bind the next sea animation frame on a spare unit and cross-fade by `texPhase`
  (`LandscapeRender.cpp` already computes it).
- **Tuning keys** in the existing plumbing (LitConstants block, dev overlay Lighting tab,
  `triSetLighting`): wave, swell and ripple strength, sky reflection (mean and wave-driven parts),
  sheen, glint breadth, sparkle. Two vec4s hold them; update the `LitConstants` layout test and the
  GLSL block together, and add the water program to the glslang compile test.
- **Clean up the dead path**: `s_psWaterRemasterMain`'s constant "bump normal" (detail texture
  bound as a bump map), the missing `misc\waveBumpMap.paa` entry, the unused `GetWaterBumpMap()`;
  and correct the README Water row, which describes a shader the sea does not use.
- **Validate**: motion capture of the sunset and mid-sun views (popping, crawl), GPU timing on an
  idle machine, other islands (Eden, Sinai with @LoBo, whose sea may use other textures), rain and
  overcast, night (moon glint), and the shoreline where water meets terrain.

## 8. Files

| Path | What |
|---|---|
| `NOTES.md` | this file |
| `images/overview_all.png`, `images/overview_<view>.png` | Classic, now, A, B, C side by side |
| `images/triptych_<view>__<A,B,C>.png` | Classic, Remastered now, Remastered + variant |
| `shaders_A`, `shaders_B`, `shaders_C` | final override GLSL (`psDetailRemasterMain.glsl`) and `params.txt` per variant |
| `shaders_*-r1..r3` | earlier tuning rounds (r1: high reflection weight, pale; r2: full material sheen, washed noon; r3: before the anisotropy guard) |
| `shaders_debug`, `shaders_debug2` | the diagnostic overrides that located the sea's shader |
| `src/water_template.glsl`, `src/make_variants.py` | variant source and generator |
| `src/run_variant.ps1`, `src/final_runs.ps1` | how the runs were shot |
| `src/compare.py`, `src/cost.py` | three-way images, metrics, cost medians |
| `cost.md` | GPU timing medians (noisy, see section 5) |
| `cmp/final1/metrics.md` | water-region metrics for the final images |
| `cmp/r1` .. `cmp/r4` | tuning-round comparisons |
| `runs/` | every run: tri output, game logs, raw captures, timings (stock, debug, debug2, r1-r4, final1-3) |
| `tex/` | extracted `more_anim.01`, `specular_dx`, `detail_dx` and the Abel heightmap used to place the views |
| `dump/` | the stock GLSL (remastered pieces and classic) the overrides started from |
