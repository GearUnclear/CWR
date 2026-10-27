# Reflections on water and wet ground (track: ssr)

Rank 4 of 22. Critic scores: worth building **6/10**, RTX-on **5/10**. Area: water and shine.

## What the player sees

Calm water mirrors boats, rocks and hills, broken up by moving ripples. Rainy streets get puddles that reflect houses, lamp posts and vehicles. Grass stays dry.

## The technique

Screen-space reflections (SSR): for each reflective pixel, a ray is marched through the depth buffer of the current frame to find what it would reflect. It can only see what is on screen. Everything runs at the end of the world pass (`EngineGL33::EndWorldScene`), before ambient occlusion, bloom and shafts, so reflected highlights bloom.

1. **Surface classes from the stencil buffer (C++).** Every world draw already writes the stencil. The prototype writes a tag per draw: water, terrain, road or decal, vehicle body, other glossy material. The sea is told apart from terrain by its glossy material. Draws that write no depth (sun and lamp flares, particles) get a zero stencil write mask; without it the sun glare quad wiped the tags in a big rectangle.
2. **Tag readout** into a texture, and the resolved frame copied into a mip chain for blurry reflections.
3. **The march** (`psPostSsr.glsl`): up to 80 steps, a 6-step refine. Most of the work went into misses: a ray that leaves the frame over open sky takes that sky; a ray that slips behind a nearby surface returns a dark estimate (this removed white flecks under vehicles).
4. **Water:** one mirror trace, then four layers of drifting ripples distort the mirror image. Fresnel (more reflection at grazing angles) with a floor so a rough sea does not turn into a perfect mirror.
5. **Wet ground:** wetness from the engine's rain or a knob. Puddles only on flat ground: roads pool water, bare ground a little, grass none. Wet asphalt is darkened and takes a rough, streaked reflection.
6. **Vehicles:** rough paint reflection.

Knobs: `ssr`, `ssrWater`, `ssrWave`, `ssrWet`, `ssrGlossy`, `ssrDebug`, `ssrRough`.

## What real data showed

Eleven rounds of captures on Abel and Eden (final shader v12).

- **Calm bay** (`BEST_bay_offshore.png`, `bay_offshore_sequence.gif`): the patrol boat sits on its own reflection; the shore rocks and the hill are mirrored and broken up by ripples. A strong win. The water is lighter and bluer than the original look because it now reflects the sky.
- **Rainy town street** (`BEST_town_rain.png`): wet asphalt with streaked reflections of houses, lamp posts and the jeep; a puddle mirrors the half-timbered house. A strong win.
- **Wet street at dusk, lamps off** (`ALT_town_wet_dusk_lampsoff_r10.png`): a pink sky in a flooded street with black silhouettes. The most "RTX" frame of the set, but it was shot with the older shader v11.
- **Sunset sea** (`BEST_coastline_sunset.png`): a glitter path from the reflected sun, but the sea turns pale lilac-grey and loses some of the warm column. Mixed.
- **Vehicle paint:** nothing visible. CWA hulls are large flat matte faces; a blurred reflection only greys them.
- **Night streets:** weak. The wet road reads as a smooth light-grey sheet; lamps and fires are not reflected (lamp streaks need a separate point-light highlight on wet ground).

## Cost

+0.35 to 1.9 ms at 1440x960 internal with 4x MSAA (from the `ao` timer section, which holds AO plus the whole SSR pass). Critic's estimate: perhaps 2.5 to 3 ms worst case at 1080p, which fits an RTX 3060 at 60 fps. Full-resolution march, no hierarchical depth, no half resolution yet. The forced queue flushes around every vehicle draw are an unmeasured CPU cost.

## Verdict

Critic: "Real and visible on calm water and wet daytime streets." The sunset sea washes out, paint shows nothing, nights get nothing, and there are more edge and box artefacts than reported. Water reflections alone justify a production pass.

## What production needs

- A real per-draw material class instead of a global flag in `Transport::Draw`; glass as its own class.
- Feed the sea reflection into the water shader instead of blending over the frame (see `water.md`).
- Wetness as world state (wet in rain, dry afterwards) and puddle masks from road geometry and terrain shape, not value noise.
- A half-resolution trace with hierarchical depth, a bilateral upsample and temporal smoothing (TAA, see `temporal.md`).
- Fade reflections near the frame edges. A fog-correct composite.
- Lamp streaks on wet ground at night need analytic point-light highlights (see `materials.md`).
- Keep the sunset sea's contrast and warmth, or it reads as a wash.

## Overclaims the critic found

- Edges are not "clean except a 2 px line": the right edge of the bay has a broken dark seam about 10 px in, and the hill reflection ends in a hard vertical cut. The town views have a lighter band at the left edge and pale blocks on the grass at the right edge.
- Pale translucent rectangles with hard edges sit under the jeep and around the soldier in the wet `lowsun_fp` view.
- The overcast rain view does show pale puddle-like blotches on grass, and a hill the fog hid comes back as a pale streaky shape (the composite is not faded by fog correctly).
- `prototype.patch` has 10 files, not 11: the `triSetLighting` knob registration exists only in the scratch worktree.
- The best dusk frame uses the older shader v11 and a lamp state the final run did not reproduce.

## Taste risks

The sunset sea becomes pale lilac-grey (a wash). Lamp-lit wet asphalt becomes a flat grey sheet. The bay is lighter and bluer overall. Rainy grass is mostly darker and greyer.

## Where the work lives

- Notes and patch: `tmp\research\ssr\NOTES.md`, `prototype.patch` (44 KB, 10 of the 11 changed files), shaders `glsl_v1` to `glsl_v12\psPostSsr.glsl`.
- Scratch worktree: `gfx-ssr` (holds the missing knob registration), build `dist\gfx-ssr-rwdi`.
- Best images (not committed): `tmp\research\ssr\BEST_bay_offshore.png`, `BEST_town_rain.png`, `BEST_coastline_sunset.png`, `ALT_town_wet_dusk_lampsoff_r10.png`, `bay_offshore_sequence.gif`, `detail_overcast_bush.png`.
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written; check it for the production state.
