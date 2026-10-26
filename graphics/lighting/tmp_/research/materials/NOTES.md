# Materials track: runtime material reconstruction and wetness

TRACKID `materials`. Research prototype, not production code. Two agents worked on this track. The first
built the C++ prototype and ran capture rounds r1 to r6 (17 Oct, 14:10 to 15:26); the owner paused the
research before it wrote any notes. The resumed agent (these notes, 18:28 to 19:25) reviewed every
r6 image, found three defects, fixed them over shader rounds v7, v8 and v9 (capture runs r7, r8, r9),
added lamp reflections for wet nights, and wrote this file. Every claim about a look points at an image in
this folder; every image was opened and checked.

**Verdict: promising, with one important bug found on the way.** Wet weather is the part that makes it
look like a different game: rain-darkened cloth and gravel, puddles with screen-space reflections, a low
sun glaring off wet ground, and above all a rainy night where every street lamp and the camp fire streak
across the wet square (`BEST_town_night_rain.png`). Dry-weather material reconstruction is real but
subtle: the derived ground relief is a clear gain, glass now reflects the sky, and a satin grazing sheen
gives vehicle hulls shape at low sun; the paint itself stays the authored olive. The bug: scripted vehicles
never get the remastered lighting at all (section 2).

## 1. Technique

RTX Remix rebuilds PBR materials from old textures. CWA has no material data beyond a texture, a
per-section TLMaterial colour set and sometimes a specular power, so the prototype derives everything at
runtime from what the engine already knows.

1. **Surface class per draw (C++).** `EngineGL33::UploadLitSurface` runs once per textured section (hooked
   into `PrepareTriangleTL`). It classifies the section from the lower-cased texture path against Czech and
   English token lists (stock data is Czech: `sklo` glass, `okno` window, `kolo` / `pneu` / `pasy` wheel,
   tyre, tracks, `hlaven` barrel, `plech` sheet metal, `plachta` tarp, `xicht` face, `bota` boot,
   `silnice` / `cesta` road, `strecha` roof, `drevo` wood; @LoBo is mostly English), plus the kind of object
   being drawn (`GDrawObjectKind`, set in `Shape::Draw` from the entity's simulation name: car / tank /
   motorcycle, soldier, helicopter / airplane, ship, weapon proxy, static; tents and camouflage nets by
   config class name) and the texture's alpha class. Result: 14 classes (paint, metal, glass, rubber,
   fabric, skin, road, masonry, wood, leather, weapon, lamp lens, roof, unknown), cached per (texture,
   kind), pushed in a new LitConstants slot (`MatClass`, 58). Terrain tiles are anonymous mixes
   (`p4p4ttp4.pac`), so terrain is classified per pixel in the shader from the ground colour (vegetation,
   sand and soil, rock and gravel). `classification_log_r6.txt` lists all 653 textures classified in the r6
   sessions. Debug view `r2=1`: `runs/r6/sinai_lowsun_fp__dbgclass.png` (also in r7) shows HMMWV paint,
   rubber wheels, glass, fabric camo net and uniform, weapon and terrain separated correctly. Known misses:
   the M4 rifle textures (`m4_1.pac`) land in fabric; gun barrels on vehicles are deliberately paint.
2. **Per-class response (GLSL `shadeMaterials`).** Each class maps to roughness, metalness, porosity,
   sheen, F0, a puddle flag and a derived-bump height. On top:
   - texture statistics: a texel brighter than its mip+3 neighbourhood (worn edges, raised detail) reads
     smoother, a darker one (cavities, grime) rougher and with specular occlusion;
   - **albedo-derived bump** (luminance as height, Mikkelsen 2010 surface-gradient bump, no tangents
     needed), energy neutral: the relief moves N.L around its geometric value with a symmetric clamp, so a
     grazing sun gains contrast but not average brightness;
   - GGX sun specular, split-sum sky reflection (Karis 2014 analytic env BRDF) against the same sky model
     the remastered water uses, fabric and grass sheen;
   - v7/v8: a **satin grazing lobe** on hard objects (paint 0.5, metal and weapons 0.7, leather 0.3,
     rubber 0.12): the grazing gain of a roughness-0.45 lobe minus its face-on value, so only faces near
     the silhouette pick up the horizon behind the line of sight.
3. **Wetness.** `litWeather.x` follows the engine rain density (smoothstep 0.05..0.55; the `wetness=` knob
   forces an "after the rain" state). Wet surfaces darken and saturate by porosity, get a water film
   (roughness drops), vertical faces carry run-down streaks from the cloud noise, and ground under a canopy
   (horizon-map sky visibility) stays drier.
4. **Puddles.** Flat ground near the terrain surface, three octaves of the existing cloud-noise texture plus
   the heightfield concavity from the horizon map's height grid (hollows fill first). Near mirrors
   (roughness 0.025) with animated rain-ring ripples while it rains. Vegetation holds none (v7/v8 gate).
5. **Screen-space reflections in puddles (C++ post pass `psPostSsr`).** The puddle mask is a pure function of
   world position, so the post pass recomputes it from depth (no extra G-buffer channel), marches the
   mirrored ray through the depth buffer and replaces the forward pass's sky reflection with what it hits:
   `colour += Fenv * mask * (hit - sky)`. The forward pass writes alpha 1 on puddle pixels and the SSR pass
   only touches alpha-1 pixels, so the two masks always agree.
6. **v7/v9: reflections of the local lights.** Classic and the remastered path give street lamps, fires and
   headlights a diffuse term only. `localSpec` adds GGX specular per light (up to 8), each light a 0.2 m
   sphere (Karis 2013 representative-point widening). v9 splits a wet surface into its dry base (reflects
   the lights only if it is glossy itself) and a smooth water film (roughness 0.16, 0.05 in puddles; weight
   = wetness x (1 - 0.75 porosity) x (1 - vegetation)), which gives the long streak toward the viewer.

Why it suits CWA: low-poly content with painted-in shading and no material data, so the gain has to come
from what is already there (names, object kinds, texture contrast, terrain height) and from weather the
engine already simulates. No new art; the classifier is a text table mods can extend.

## 2. Finding beyond this track: scripted vehicles skip the remastered lighting

The first agent found (r1diag) that **every shape first loaded after the mission's `Shapes.OptimizeAll()`
has no vertex buffer**, so `Shape::Draw` sends it down the CPU T&L path. In GL33 those pre-transformed
vertices run `VSPreTransformed`, which sets `vLitMode = 0`, and the remastered fragment shaders then pass
the CPU-lit vertex colour straight through: no per-pixel lighting, no shadow-map lookup, no sky
visibility, no materials. Such objects look like Classic inside a Remastered frame.

Evidence: r6 to r9 `game_stdout.log`, lines `MATRESEARCH draw sim car ... tlAble 1 buffer 0` and
`sim tank ... buffer 0` for the props the capture lib spawns with `createVehicle`; with `vbfix=1` the same
shapes log `vbfix: vertex buffer for a late-loaded shape` and then draw with `buffer 1`. Pixel proof on
`runs/r8/noon_fields__*.png`, jeep side panel box (640,390)-(700,425): Classic mean RGB 70.8 72.2 65.6,
current Remastered **70.9 72.4 65.8 (Classic)**, vbfix 49.7 49.9 46.5. Visually, the shipped Remastered
jeep in `work/r7/lowsun_jeep_crop.png` has a flat, slightly ghostly look; with vbfix it is solid and
shadowed.

Consequences:
- Every vehicle prop in the 13-view capture set is spawned by `capSpawn` with `createVehicle`, so **every
  "Remastered" vehicle hull in the baseline captures is Classic CPU lighting**. The known problems "low-sun
  shaded faces near silhouette crushed" and "evenings judged on vehicle hulls" were judged on Classic-lit
  hulls. Tracks that tune the rig against hulls should re-check with this fix on.
- Guerrilla Mode spawns traffic, garrisons and convoys by script, so very likely most of its vehicles also
  miss the remastered lighting in the shipped build (not verified in a Guerrilla session here).
- Under the true per-pixel path the noon jeep is about 30 % darker than Classic (numbers above). That is
  the rig, not this track, but the hull problem is bigger than the baseline shows.

The prototype fix (`ShapeDraw.cpp`, knob `vbfix`) calls `ConvertToVBuffer` on the first draw of such a
shape. Production: create the buffer when a late shape loads; independent of materials; Remastered only
if Classic must stay bit-identical.

## 3. Rounds r1 to r6 (first agent), as reviewed

Superseded r6 comparison images are kept in `best_r6/`. What they showed:
- Dry views subtle; ground relief from the derived bump was the clearest gain.
- `best_r6/BEST_noon_fields.png`: **worse than current**, the jeep turned grey-blue (side box: vbfix
  50 50 46, v6 59 61 63, blue above red). Rough paint added a sky reflection outside exposure, a blue wash.
- `best_r6/BEST_overcast_heavy_rain.png`: **defect**, flat grey puddle sheets all over the Eden meadow. The
  greenness test `(g - max(r, b))` missed olive grass (g only 4 to 26 % above r), so the meadow was soil.
- `best_r6/BEST_abel_rain.png`, `best_r6/BEST_lowsun_fp_wet.png`: the wet looks, already the strongest.
- r2 was abandoned (second build, no images); r1, r1diag, r3 to r6 are complete runs.

## 4. Rounds r7 to r9 (resumed agent)

**v7** (run r7, `override_v7`): vegetation gate by colour ratios (g/b and g/r) so olive and yellow-green
grass holds no puddles; paint roughness 0.66 -> 0.74 and a tighter env fade so dry paint adds no sky haze;
the satin grazing lobe (A/B knob `r2=5`); local-light specular; two new night views (`town_night`,
`town_night_rain` at Saint Louis, 22:00, the capture set's lamp cluster and camp fire). Results:
- Eden meadow fixed: `BEST_overcast_heavy_rain.png` (v7) shows wet, darker, more saturated grass and no
  grey sheets; `work/r7/overcast_heavy_rain.png` debug panel shows zero puddle mask on the meadow.
- Noon jeep still desaturated (sat 0.026 vs vbfix 0.068): texture contrast lowered the roughness below the
  env-fade threshold. The rim at full strength made the HMMWV fender and tyres look silvery
  (`work/r7/sinai_hmmwv_crop.png`). Both fixed in v8.
- **Failure at night**: wet ground turned into white sheets (`work/r7/town_night_rain_crop.png`).
  The first four r7 town_night shots also caught a weather transition left over from the previous view
  (lamps off, dark), so r8/r9 shoot the night views first, after 400 settle frames.

**v8** (run r8, `override_v8`): env fade judged on the class roughness while dry; rim halved and rubber
nearly off; vegetation thresholds widened; diagnostic knobs `r2=6/7/8` (light specular / sky reflection /
sun specular off). Results:
- Noon jeep keeps its olive: side box vbfix 49.7 49.9 46.5 (sat 0.068), v8 without rim 48.8 49.0 45.3
  (sat 0.075), v8 51.2 51.8 49.1 (sat 0.052). `BEST_crop_noon_jeep.png`, `BEST_noon_fields.png`.
- HMMWV: glass reflects the sky, edges and door seams catch a soft sheen, no silver
  (`BEST_crop_sinai_hmmwv.png`, compare the "rim off" panel).
- The night white sheets were the light specular alone: off with `r2=6`, still there with sky reflection
  or sun specular off. `FAIL_v8_night_white_sheets.png`, `work/r8_town_wetdiag.png`,
  `work/r8/town_rain_diag.png`. Cause: lamps reflected through the wet ground's own roughness (0.35 to
  0.55), wide lobes from six lamps and a fire summing into a white floor under the high night exposure.

**v9** (run r9, `override_v9`): dry base plus water-film lobe for the light specular (section 1, item 6).
- `BEST_town_night_rain.png`: the lamp and fire now draw separate vertical streaks across the wet square,
  the road shines toward the lamps, puddles reflect the half-timbered houses (SSR). Classic and current
  Remastered show a dry lawn under rain.
- `BEST_town_night_wet.png` (wetness 1, clear night after rain) and `BEST_crop_town_night_wet.png`
  (dry vs wet vs wet without SSR): the streaks come from the film lobe, the house reflections from SSR.
  The repeat shot `wet_b` matches `wet` (stable state).
- Dry night (`materials` panel in `work/r9/town_night.png`) looks essentially like current Remastered.

Final images (Classic | Remastered (current) | prototype, same session, camera, clock and weather, GPU
frame ms printed on each panel):

| image | run, shader | what it shows |
|---|---|---|
| `BEST_town_night_rain.png` | r9, v9 | rainy night: lamp streaks, wet road, SSR puddles. The strongest result |
| `BEST_town_night_wet.png`, `BEST_crop_town_night_wet.png` | r9, v9 | after-rain night, plus SSR on/off |
| `BEST_lowsun_fp_wet.png` | r8, v8 | low sun glaring off wet gravel, puddles reflecting the vehicles |
| `BEST_abel_rain.png`, `BEST_crop_abel_rain_puddles.png` | r8, v8 | rain: wet cloth and hulls, SSR puddles vs sky-only puddles |
| `BEST_overcast_heavy_rain.png` | r7, v7 | rain on the Eden meadow: wet grass, no puddles on grass |
| `BEST_lowsun_fp.png`, `BEST_crop_lowsun_m113.png`, `BEST_crop_lowsun_ground.png` | r8, v8 | dry low sun: hull detail, ground relief |
| `BEST_sinai_lowsun_fp.png`, `BEST_crop_sinai_hmmwv.png` | r8, v8 | @LoBo: glass and grazing sheen on the HMMWV |
| `BEST_noon_fields.png`, `BEST_crop_noon_jeep.png` | r8, v8 | noon: paint stays olive (the v6 regression is gone) |
| `BEST_tank_gunner_optics.png` | r7, v7 | no visible change through the optics |
| `FAIL_v8_night_white_sheets.png` | r8, v8 | the negative result that led to v9 |

## 5. GPU cost

Measured honestly but on a shared GPU: up to four games from other agents ran at once, and single readings
swing 2x (Classic noon_fields read 11.0 ms in r6, 19.6 ms in r7, 6.5 ms in r8). `timings.md` holds the
per-run medians and the **paired deltas** (each materials shot minus the Remastered shot taken seconds
before on the same view). Over the 33 pairs from views shot three times (r6 to r9): **median +0.37 ms
frame, interquartile range -0.27 to +1.75 ms** (960x640 x 1.5 scale, 4x MSAA; this includes vbfix,
which moves late shapes from CPU T&L to vertex buffers). The SSR pass shows up in the post "ao" bucket:
+0.15 to +0.41 ms in the rain views (paired), about 0 when dry since it early-outs (one dry outlier,
r6 sinai, read +0.68). Shader work added per
fragment: 3 extra texture reads (coarse mip, two bump taps), 3 noise reads and a 9-cell ripple loop on
wet terrain near the camera, and one or two loops over up to 8 lights when local lights exist. Expect
well under 1 ms on a quiet GPU outside rain, about 1 ms with SSR in rain; a quiet-machine measurement is
still owed.

## 6. What a production version needs

1. vbfix as a real fix in the shape load path (section 2); it matters for every track, not just this one.
2. Material classes baked at texture load (`TextureGL33` gets a class) from a config table (a UD
   `CfgSurfaceMaterials`-style list of name patterns per class), so mods such as @LoBo can declare their
   own; per-model overrides for the misses (rifles as fabric, gun barrels).
3. Wetness as engine state: integrate wetting during rain and drying over minutes (sun, wind), with
   puddles filling and draining slower than surfaces; today it follows the rain density instantly.
4. Fold `shadeMaterials` into `shadeRemastered` (one path) and keep Classic untouched; tune against the
   rig once the night brightness and hull problems from other tracks are fixed (the night streaks will
   read much stronger in a darker night).
5. SSR: run it at half resolution with a temporal resolve, add a roughness-based blur, extend it to wet
   roads and glass; reuse the depth pyramid other tracks may build.
6. Light specular: headlights and muzzle flashes would get the same streaks on wet roads for free (they are
   local lights); worth a capture with a moving convoy at night in rain.
7. Terrain classification from colour is fragile (it needed two rounds to stop puddles on olive grass); a
   per-terrain-texture class table, keyed by the island's texture names, is the robust version.

## 7. Risks against the owner's taste list

- **No blue wash on paint**: v6 had one at noon; v8 removes it (numbers in section 4). The satin rim still
  adds a little sky colour on grazing faces (sat 0.075 -> 0.052 on the noon jeep side); `r2=5` turns it off.
- **"Only darker"**: wet surfaces are darker by design (water fills pores); at night the lamp streaks and
  at low sun the wet glare add light, so rain is not just darker. Dry looks are not darker than vbfix.
- **Haze**: the dry low-sun ground toward the sun picks up a bright forward glare strip near the horizon
  (`BEST_lowsun_fp.png`, left edge); it is localised, but watch it.
- **Wet hulls**: the run-down streaks on a wet M113 read as fine vertical stripes, a little corrugated
  (`BEST_crop_lowsun_m113.png`, last panel), and wet paint picks up the grey sky.
- **Strong wet glare** at low sun (`BEST_lowsun_fp_wet.png`) is bright and fairly white; it reads as wet,
  but it is a bold look and only appears when the ground is wet.
- Nothing here touches tree crowns (foliage keeps `shadeRemastered`), colour grading or textures.
- Classic: the prototype keeps Classic untouched (materials only run with `materials=1` in Remastered),
  but `vbfix` was not checked for Classic bit-identity; a production vbfix should be Remastered-gated or
  verified.

## 8. Files

- `prototype.patch`: the C++ prototype (diff of `D:\Arma_CWA_worktrees\gfx-materials`, detached at the
  lighting branch tip, built to `arma_CWR\dist\gfx-materials-rwdi`). Unchanged by the resumed session
  (verified identical to the worktree diff). Knobs via `triSetLighting`: `materials`, `wetness`,
  `puddles`, `matBump`, `vbfix`, `r1` (1 = SSR off), `r2` (1 class, 2 roughness, 3 wet/puddle, 4 normal
  debug views; 5 rim off, 6 light specular off, 7 sky reflection off, 8 sun specular off).
- `override_v9/`: final GLSL (psRemasterCommon.glsl, the three remastered mains, psPostSsr.glsl).
  `override_v1` to `override_v8` are the earlier rounds. `scripts/materials_glsl_v*.txt` are the
  materials sources; `scripts/build_override.py` assembles an override dir from `orig/` + a source.
- `scripts/gen_tests.py` (research capture tests; generated `.sqf` / `.toml` copies in
  `scripts/research_tests/`, r1 to r9, also in the scratch worktree under
  `arma_CWR\tests\rendering\research\materials\`), `compare.py` (labelled strips with GPU ms),
  `make_best.py` (all BEST images), `timings.py`, `paired.py`, `boxstats.py` (mean colour in a box),
  `glslcheck/` (glslang validator used before every run).
- `runs/r1` ... `runs/r9`: complete capture runs (screens, `tri/*/game_stdout.log`, timings, contact
  sheets); `runs/r2` is empty (abandoned). `work/`: working comparison sheets cited above.
- `timings.md`: medians per run and paired deltas. `classification_log_r6.txt`: every texture's class.
