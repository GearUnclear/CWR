# Sea waves (track: water)

Not ranked by a critic: **no critic has reviewed this track.** Area: water and shine. The full write-up lives in its own tag; this page is a pointer and a short summary.

## Where the full document is

- Tag `research/remaster-water-2026-10-17` on `origin` (formerly branch `claude/remaster-water-research`, retired 2026-10-27), commit 935cb5e "research: remastered water prototype (wave normals from the sea texture)", 2026-10-17. It was cut from the lighting branch `claude/youthful-lovelace-8imbdh` at 3c53141, as the owner asked ("put the water stuff on its own branch with good docs to review later").
- The document: `arma_CWR/tests/rendering/water_research/README.md` in that tag; read it with `git show research/remaster-water-2026-10-17:arma_CWR/tests/rendering/water_research/README.md`.
- Measurements in the tag: `arma_CWR/tests/rendering/water_research/measurements/metrics.md` and `cost.md`. Shaders: `shaders_A`, `shaders_B`, `shaders_C` (`psDetailRemasterMain.glsl` plus `params.txt`), generated from `src/water_template.glsl` by `src/make_variants.py`.
- Research notes on the dev machine: `tmp\research\water\NOTES.md`.

## The owner's complaint

After the first in-game captures the owner wrote: "the water is so much less defined looks so smooth".

## The cause (the first diagnosis blamed the wrong shader)

- The sea is not drawn by the engine's water shader. Landscape water faces carry only `SpecularTexture` (`World/Terrain/LandscapeRender.cpp`, around line 320: the `IsWater` flag is commented out), so the sea goes to the terrain detail shader (`PSDetail`, in Remastered `psDetailRemasterMain`), with the animated sea texture `more_anim.01..07` (one repeat per 50 m) and `data\specular_dx.paa` (a ripple pattern in its alpha).
- Proven in game: diagnostic colour bands written into the water shaders never appeared on the sea; bands in the detail shaders covered it.
- The original look shows the sea texture at full contrast and adds a per-vertex sun glint on top. Current Remastered treats the sea as a flat surface with a very broad sheen and a smooth sky reflection, which washes the waves out.
- The unused water-shader path is broken too: `misc\waveBumpMap.paa` (named in `remaster.cpp`) does not exist in the Remaster data, `GetWaterBumpMap()` has no callers, and `PSWater` binds the plain white detail texture as its "bump map". Restoring `IsWater` as it stands could add six times the sun colour to all water for some sun directions. The Water row of `engine/Poseidon/Graphics/Remaster/README.md` describes a shader the sea does not use.

## The fix tried

The sea texture's own brightness is treated as a height field, and the wave normal comes from it (the texture-to-world mapping is solved per pixel from screen derivatives, so no C++ and no tangents). Three variants of the sea branch only:

- **A:** the stock Remastered shading with the wave normal. Not enough on its own.
- **B:** a new water model: the lit sea texture, a sky reflection weighted by Fresnel on the wave normal (low on average, full weight for the part that changes from wave to wave), and three sun lobes, all shadowed.
- **C (the pick):** B plus a swell layer, a ripple layer, waves fading out between 400 m and 3 km, and a sparkle that widens with distance.

## What real data showed

Water region only, brightness 0 to 255; "fine sd" is the wave detail:

| View | Original look: mean / fine sd | Current Remastered | A | B | C |
|---|---|---|---|---|---|
| coastline_sunset | 70 / 3.7 | 100 / 3.0 | 96 / 12.3 | 112 / 22.2 | 113 / 23.7 |
| sea_noon | 145 / 6.1 | 129 / 4.4 | 130 / 7.7 | 125 / 10.1 | 128 / 12.7 |
| sea_away_from_sun | 112 / 7.4 | 114 / 5.7 | 115 / 6.1 | 124 / 14.4 | 124 / 15.1 |
| coast_shore_midsun | 176 / 7.0 | 154 / 3.5 | 151 / 8.6 | 138 / 12.4 | 139 / 13.3 |

At sunset, C shows wave rows to the horizon and a broken, glittering sun path instead of a blurred column. The beach is pixel-identical; the sky near the sun differs by at most 28 of 255 at single pixels (bloom).

## Cost

Estimated under 0.1 ms (2 extra texture reads for A and B, 7 for C, about 60 to 100 instructions per water pixel). The measurement was too noisy to use (the original look alone ranged 0.55 to 4.69 ms on one view).

## Open problems and risks

- **Colour:** at noon and mid-sun the sea turns cool grey-blue where the original look is green-yellow. That is close to the rejected blue wash and must be fixed first. The far sea at sunset is lighter and greyer than the original look.
- **Motion:** nobody has watched it move. The sea texture flips through 7 frames (every 0.14 s), so the derived waves may pop; rows may crawl at grazing range.
- **No critic review.**

## What production needs (from the branch README)

1. Bring back the sea's own colour at noon and mid-sun.
2. Route the sea explicitly in C++ (its own render flag and water program) instead of recognising it by its bright specular.
3. Cross-fade between sea animation frames (`texPhase` is already computed in `LandscapeRender.cpp`) to stop the popping.
4. Tuning keys in the existing plumbing, with the `LitConstants` layout test and the glslang compile test updated.
5. Clean up the dead water path and fix the README's Water row.
6. Validate in motion, on an idle machine, on other islands (Eden, Sinai with @LoBo), in rain, overcast and at night.
7. The reflections track's sea reflection should feed this shader instead of being blended over the frame (see `ssr.md`).

## Images

Renders of game data, never committed. On the dev machine: `tmp\water-research-images\` (named in the tag's README) and `tmp\research\water\images\`: `overview_all.png` (original look, current Remastered, A, B, C per view), `triptych_<view>__C.png` (original look, current Remastered, Remastered plus C), and the same for A and B. In the image labels, "Classic" means the original look.
