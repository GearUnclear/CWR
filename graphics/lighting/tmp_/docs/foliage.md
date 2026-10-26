# Tree crowns lit as volumes (track: foliage)

Rank 16 of 22. Critic scores: worth building **4.5/10**, RTX-on **3/10**. Area: light and shadow.

## What the player sees

Leaves glow when the sun is behind them, shade inside a crown is green rather than grey, and each tree gets a slightly different tint. A separate canopy map adds forest-scale shading. Obvious only when looking toward the sun through trees.

## The technique

**The crown model** (shader only, `shadeFoliageResearch` in `ov8\psRemasterCommon.glsl`). The engine already uploads an ellipsoid fitted to each tree shape's bounding box (`CrownEllipsoid`). The vertex shader also passes the leaf's position inside that ellipsoid, so one ray-sphere test gives how deep the leaf sits toward the sun, the sky and the viewer (as a fraction of the crown size, so a 3 m bush and a 20 m hornbeam behave alike). Per leaf:

1. **Self-shadowing:** sun visibility `exp(-tau * depth)`, combined with the shadow map.
2. **Translucency:** leaves facing away from the sun transmit it toward the viewer, strongest near the sun direction and at thin crown edges. A 4-tap shadow lookup where the sun ray leaves the crown, so a tree in front still blocks the glow.
3. **Leaf tint:** transmitted light coloured by the leaf texture, leaned toward a chlorophyll yellow-green, only for texels that are saturated and greener than blue (so flowers and bark keep their colour).
4. **Light scattered inside the crown,** tinted green.
5. **Sky occlusion** by crown depth, a **sky rim** at thin edges, **per-tree variation** (value, hue, saturation from a hash of the object position).
6. **A trunk mask:** alpha test turned out to be on for whole tree draws, trunks included, so bark is told apart by geometry (a thin column below the crown base).

**The canopy map** (C++): every tree, bush taller than 2.5 m and forest block is drawn into a 2048x2048 texture over the island (6.25 m texels) when the horizon map is uploaded: Abel 68,764 trees and bushes and 1,424 forest blocks in 123 ms. The shader uses it for sky occlusion and for a 10-step sun march beyond the shadow-map range.

Knobs: `f0` to `f7` (crown model), `c0` to `c7` (canopy map). Variants: vH2 (recommended), vJ (milder), vHc (vH2 plus the canopy map).

## What real data showed

Nine runs.

- **Into a 12-degree sun through a hornbeam grove** (`BEST_backlit_hornbeam.png`): current Remastered shows pale grey-olive crowns; vH2 shows deep green crowns, the tree in front of the sun glows yellow-green, per-tree colour variety, bark stays bark. The one view where a player would notice.
- **Golden-hour birch** (`BEST_forest_edge_golden.png`): 7% darker, 12% more saturated, warmer (red/blue 1.84 against 1.72).
- **Distant forest blocks with the canopy map** (`BEST_overcast_broken.png`): the grey "wall of trunks" becomes a dark forest interior. A real but tiny win.
- **Pine stand at noon with the canopy map:** the forest floor only gets darker (frame -25.6%). A negative result.
- **Night:** no change.
- **Failed on the way:** depth in metres (big crowns got no glow); texture-colour tint made olive shrubs glow mustard; a strong green push went lime and cartoonish; a reserved GLSL word (`packed`) broke one round's compile (quarantined).

**Finding outside the prototype:** in views facing a low sun, the shipped sun visibility was 1 everywhere and the shadow pass cost 0.00 ms. Trees cast no shadows toward the camera. That was the start-up bug that leaves tree shadows off (see `shadows.md`); the object-shadows bug fix confirmed it is the same bug, and that the flat grey clearing in `pine_stand_noon` was the clear-sky cloud shadow (see `bugs.md`).

## Cost

Crown model 0 to 1 ms (quiet samples +0.0 to +0.4 ms). Canopy map another 0.3 to 0.9 ms, 43 MB of GPU memory and about 125 ms of CPU at island load.

## Verdict

Critic: the crown shader is "real, cheap and loaded correctly". Outside the into-the-sun hornbeam shot it mostly makes crowns a bit greener, and the canopy map mostly makes things darker. A modest step toward the RTX goal, plus a valuable bug find.

## What production needs

- Keep the self-shadow, the per-tree variation and the translucency. Drop or heavily gate the green lean, which recolours authored art.
- Fold it into `shadeRemastered` with fixed constants; pass the crown data per instance instead of an `inverse(mat3)` per vertex.
- A better bark test (a per-texture flag at load).
- A crown box per LOD that excludes the trunk.
- Canopy map on the horizon-map worker thread, memory capped; skip its sky term on forest floors. Keep the far-field march as a fallback.

## Overclaims the critic found

- "A different, lusher game" is overstated: frame-wide, only 0.1 to 5.4% of pixels move by more than 16 levels.
- "Dark crown cores" are not visible on the key tree: the tree in front of the sun becomes a fairly uniform light green.
- The canopy map's light streaks between trunks are faint; the main effect is a uniform darkening and greying of the foreground (frame -11.4%, saturation -16.6% in the hornbeam view).
- "Flowers stay white" is only partly true: the bush in the overcast view turns yellow-green with yellowish flowers.
- Per-crown numbers come from hand-picked boxes and overstate how much of the frame changes.

## Taste risks

Recolours authored foliage (blue-grey leaves to yellow-green, backlit trees lime), which is close to new art. The tree in front of the sun turns into one flat green mass (flattened crown). The canopy map is only darker where it acts.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\foliage\NOTES.md`, `prototype.patch` (18 KB, knobs and `EngineGL33::BuildCanopyMap`), final shaders `ov8\`, `ov8_vs_shipped.diff`, timings `work\t2_summary.txt`, `work\t3_summary.txt`, crown statistics `work\r9_regstats.txt`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-foliage`, build `arma_CWR\dist\gfx-foliage-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\foliage\BEST_backlit_hornbeam.png`, `BEST_backlit_birch.png`, `BEST_forest_edge_golden.png`, `BEST_pine_stand_noon.png` (the negative), `r9\backlit_hornbeam__dbgsunvis.png` (the missing shadows), `work\r9_ob_flower.png`.
- Integration: a reduced crown model (no green lean) is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.
