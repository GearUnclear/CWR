# Water research: giving the sea its waves back

Research prototype, 2026-10-17. Nothing here is wired into the engine yet: the
three variants are shader overrides loaded at runtime through
`CWR_SHADER_OVERRIDE_DIR`. Branch `claude/remaster-water-research`, cut from the
remastered lighting branch `claude/youthful-lovelace-8imbdh` at 3c53141.

## The short version

- **The problem.** With the new (remastered) lighting the sea looks smooth and
  flat. The owner put it as "the water is so much less defined, looks so
  smooth".
- **The cause.** The sea is not drawn by the engine's water shader. It is drawn
  like ground (the terrain "detail" shader), with an animated sea texture. The
  original look shows that texture at full contrast and adds a sun glint on top.
  The new lighting treats the sea as a perfectly flat surface and lays a broad,
  smooth sheen and a smooth sky reflection over it, which washes the waves out.
- **The fix tried.** The sea texture's own light and dark pattern is turned into
  wave shapes (a height field), and those wave shapes catch the sun and reflect
  the sky. No new art: it only uses textures the game already ships.
- **Result.** Variant C (the pick) shows waves everywhere and a broken,
  glittering sun path instead of a blurred column. At sunset it is a clear win.
- **Open problem.** At noon and mid-afternoon the sea turns cool grey-blue,
  where the original look is green-yellow. That is close to the "blue wash" the
  owner has rejected before, so the colour must be fixed before this ships.
- **Not tested yet.** Only still pictures were taken. The sea texture flips
  through 7 animation frames, so the derived waves may pop when it moves. The
  GPU cost could not be measured cleanly (see "GPU cost").

## How to review

Look at the pictures. They are renders of game data, so they are not committed;
they are kept on the development machine in `D:\Arma_CWA\tmp\water-research-images\`
(gitignored). Re-shoot them with the steps under "Reproduce" if that folder is
gone.

| Image | Panels |
|---|---|
| `overview_all.png` | one row per view: original look, new lighting today, A, B, C |
| `triptych_<view>__C.png` | original look, new lighting today, new lighting + C |
| `triptych_<view>__A.png`, `__B.png` | the same for the other variants |

Views: `coastline_sunset`, `sea_noon`, `sea_away_from_sun`, `coast_shore_midsun`.
In the image labels, "Classic" means the original look (the engine's stock
lighting), not the old Classic 1.99 game; all shots use the full Remaster data.

## The variants

All three replace only the sea branch of the remastered terrain shader
(`psDetailRemasterMain`); everything else in that shader is stock. They are
generated from `src/water_template.glsl` by `src/make_variants.py`, with the
constants listed in each `shaders_<id>/params.txt`.

- **Wave normal (all three).** There is no bump map for the sea, so the
  normal comes from the sea texture's brightness, treated as a height field.
  The texture-to-world mapping is solved per pixel from screen derivatives
  (exact on the flat sea, so no tangent frame and no C++ is needed), and the
  sampling step follows the pixel footprint so the waves are filtered like the
  colour is. Two extra texture reads.
- **A, "stock plus wave normal".** The existing remastered shading, evaluated
  with the wave normal. Adds detail close up, but at range and at noon it looks
  like today's version. Not enough on its own.
- **B, "Fresnel sky plus glint".** A new water model on the wave normal:
  - body: the lit sea texture;
  - sky reflection: the visible sky in the reflected direction, weighted by the
    Fresnel term of the wave normal. The average weight stays low so the sea
    never turns into a pale mirror, while the part that changes from wave to
    wave goes in at full weight. Most of the definition comes from that;
  - sun: a share of the material's own broad glint, a medium glint and a tight
    sparkle, all on the wave normal and shadowed like the rest of the sunlight
    (shadow maps, hill shadow, cloud shadow);
  - wave strength drops where the texture filter cannot keep up at grazing
    range.
- **C, "multi-scale" (the pick).** B plus a swell layer (the same sea texture
  at about a quarter of the scale, rotated), a ripple layer (the existing
  `specular_dx` ripple texture), the waves fading out between 400 m and 3 km,
  and a sparkle that widens with distance so far glitter turns into sheen
  instead of single-pixel sparks.

## Why the sea is drawn by the terrain shader

Recorded here because the first diagnosis (in the session that started this)
blamed the wrong shader.

- Landscape water faces carry only `SpecularTexture`
  (`World/Terrain/LandscapeRender.cpp`, around line 320: the `IsWater` flag is
  commented out). `BuildRenderPassDescriptor` therefore sends the sea to the
  terrain detail family, `PSDetail` / `psDetailRemasterMain`, with:
  - tex0: the animated sea texture `more_anim.01..07` (512 px, one repeat per 50 m);
  - tex1: `data\specular_dx.paa` (luminance 255 everywhere, alpha a ripple
    pattern), 32 repeats per 50 m.
- Proved in game: diagnostic colour bands written into the `psWater` and
  `psWaterRemasterMain` overrides never appeared on the sea; bands in `psDetail`
  and `psDetailRemasterMain` covered it. The same run read back the sea's normal
  (straight up), its material specular (about 0.6 to 0.7, power about 5 to 14)
  and the texture scale.
- Original look: `vColor * t0 * (t1.a * 2) + vSpecColor`. By day `vColor` clamps
  to 1, so the sea texture shows at full contrast, and the sun path is the
  per-vertex specular added on top.
- New lighting today: `shadeRemastered` plus `glossy()` with the flat normal. The
  material's very broad glint gives a blurred column at a low sun and a pale
  sheen at noon, and the sky term adds a smooth veil. Both lift the average and
  flatten the texture.
- The unused water shader path is broken too, which matters only if anyone
  revives it: `misc\waveBumpMap.paa` (named in `remaster.cpp`) does not exist in
  the Remaster data, `GetWaterBumpMap()` has no callers, and `PSWater` binds the
  plain white detail texture as its "bump map", so its glint is one value for
  the whole surface. Restoring `IsWater` as it stands could add six times the
  sun colour to all water for some sun directions.

## Measurements

From `measurements/metrics.md` (final images, water region only, luminance
0-255). "fine sd" is the wave detail (standard deviation of a 9x9 high-pass);
"mean" is the average brightness.

| View | Original look: mean / fine sd | New lighting today | A | B | C |
|---|---|---|---|---|---|
| coastline_sunset | 70 / 3.7 | 100 / 3.0 | 96 / 12.3 | 112 / 22.2 | 113 / 23.7 |
| sea_noon | 145 / 6.1 | 129 / 4.4 | 130 / 7.7 | 125 / 10.1 | 128 / 12.7 |
| sea_away_from_sun | 112 / 7.4 | 114 / 5.7 | 115 / 6.1 | 124 / 14.4 | 124 / 15.1 |
| coast_shore_midsun | 176 / 7.0 | 154 / 3.5 | 151 / 8.6 | 138 / 12.4 | 139 / 13.3 |

What the pictures show:

- **coastline_sunset.** Today: a smooth, blurred copper column on flat water. B
  and C: wave rows across the whole sea and a broken, glittering warm path. The
  far sea is lighter and greyer than the original look (it picks up the sky).
- **sea_noon (sun behind).** Today: an even grey-blue sheet. B and C: wave
  relief over the whole frame. The original look still has more large-scale
  colour variation (whitecaps on a greener sea), and the water reads cooler and
  bluer than the original.
- **sea_away_from_sun.** B and C give near-shore water clear wave relief with
  sky-lit crests; the far sea is like today's.
- **coast_shore_midsun.** Today: a pale, smooth sheen on a washed-out sea. B and
  C: a glittering glint patch, wave relief everywhere, darker troughs. Darker on
  average than the original's yellow-green sheen, but with brighter highlights.
- Outside the sea nothing changes: the beach is pixel-identical; the sky near
  the sun differs by at most 28/255 at single pixels (bloom from the brighter
  glints).

## GPU cost

Not measured reliably. The shots were taken on an RTX 4060 Ti (1440x960
internal, 4x MSAA) while other research experiments ran games on the same GPU,
and the run-to-run spread swamps any shader difference: the original look alone
ranged 0.55 to 4.69 ms on one view. `measurements/cost.md` has the medians.
By instruction count the sea branch adds 2 texture reads (A, B) or 7 (C) and
about 60 to 100 instructions per water pixel, which should be well under 0.1 ms
at this resolution. That is an estimate; time it on an idle machine before
shipping.

## What a production version needs

1. **Colour.** Keep C's waves and glints but bring back the sea's own colour at
   noon and mid-sun (less sky veil, more of the texture's green-yellow body).
   This is the first thing to fix.
2. **Route the sea explicitly (C++).** Give landscape water faces their own
   render flag and a dedicated water program, instead of recognising the sea by
   its bright material specular (the prototype's `isSea()` would also catch any
   other draw with `SpecularTexture` and a bright specular).
3. **Stop the popping.** Bind the next sea animation frame on a spare texture
   unit and cross-fade by `texPhase` (`LandscapeRender.cpp` already computes it),
   so the derived waves move smoothly.
4. **Tuning keys** in the existing plumbing (`LitConstants`, the dev overlay
   Lighting tab, `triSetLighting`): wave, swell and ripple strength, sky
   reflection, sheen, glint breadth, sparkle. Update the `LitConstants` layout
   test and add the water program to the glslang compile test.
5. **Clean up the dead water path** (`s_psWaterRemasterMain`, the missing
   `waveBumpMap.paa` entry, `GetWaterBumpMap()`) and fix the Water row of
   `engine/Poseidon/Graphics/Remaster/README.md`, which describes a shader the
   sea does not use.
6. **Validate**: watch the sunset and mid-sun views in motion (popping, crawling
   rows), time it on an idle machine, and check other islands (Eden; Sinai with
   @LoBo, whose sea may use other textures), rain and overcast, night (moon
   glint) and the shoreline.

The owner decided on 2026-10-17 that the new lighting will replace the original
look rather than sit beside it as an option. So the old requirement to keep the
original look bit-identical no longer applies to this work.

## Reproduce

From `arma_CWR` on a machine with the Remaster shadow
(`D:\Arma_CWA\tmp\remaster-shadow-67`) and a build in `dist/lighting-rwdi`:

```
python tests\rendering\water_research\src\make_variants.py
powershell -NoProfile -File tests\rendering\water_research\src\run_variant.ps1 -Id stock
powershell -NoProfile -File tests\rendering\water_research\src\run_variant.ps1 -Id C
python tests\rendering\water_research\src\compare.py --stock ..\tmp\water_research\runs\stock --variant C=..\tmp\water_research\runs\C --out ..\tmp\water_research\cmp
```

`run_variant.ps1` shoots the four views through
`tests/rendering/captures/run-captures.ps1` with its own data junction (so its
cleanup only stops its own games) and writes to `tmp\water_research\runs\` at
the git root. `src/final_runs.ps1` shoots three passes of stock, A, B and C for
timing medians; `src/cost.py <runs dir> <out.md>` builds the cost table.

The four views (Abel west coast, beach at 1550, 5150, open sea toward heading
278, date 1985-07-05):

| View | Eye height / heading / pitch | Time | Sun |
|---|---|---|---|
| coastline_sunset | 3 m / 278 / -4 | 18:35 | 8.4 deg, ahead and 15 deg right |
| sea_noon | 12 m / 278 / -12 | 12:30 | 71.9 deg, left and behind |
| sea_away_from_sun | 3 m / 278 / -5 | 09:00 | 48.4 deg, behind |
| coast_shore_midsun | 15 m / 278 / -22 | 16:30 | 31.6 deg, ahead |

## Files

| Path | What |
|---|---|
| `abel_water.test.sqf`, `.toml` | the four water views (Trident capture test) |
| `capture_lib.sqf` | copy of `tests/rendering/captures/capture_lib.sqf` the test includes |
| `shaders_A`, `shaders_B`, `shaders_C` | the override GLSL (`psDetailRemasterMain.glsl`) and constants per variant |
| `src/water_template.glsl`, `src/make_variants.py` | variant source and generator |
| `src/run_variant.ps1`, `src/final_runs.ps1` | how the shots were taken |
| `src/compare.py`, `src/cost.py` | comparison images, water metrics, cost medians |
| `measurements/metrics.md`, `measurements/cost.md` | the numbers above |
