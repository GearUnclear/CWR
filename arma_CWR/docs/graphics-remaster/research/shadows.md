# Tree and building shadows (track: shadows)

Rank 10 of 22. Critic scores: worth building **5/10**, RTX-on **3/10**. Area: light and shadow.

## What the player sees

Trees, bushes and buildings cast sun shadows again. Forest floors get dappled sun, slopes get long golden-hour tree shadows. Also tested: soft sun shadows that are sharp at contact and softer farther away (PCSS), and short screen-space contact shadows. Those two are close to invisible.

## The main finding: a start-up order bug

In every capture run, and in any fresh profile, trees, bushes and buildings cast **no sun shadow at all**, in either look. `Scene::_objectShadows` starts false. At boot, `LoadAndApplyGraphicsConfig()` runs before `InitializeWorld()` (`GameApplication.cpp` lines 681 then 683), so the part of the graphics apply that would turn object shadows on (behind `if (GScene)`) never runs. Only the Graphics options page, or an old per-profile key, turns them on. The sun-visibility debug view of `forest_edge_golden` shows only the soldier casting.

With object casters on, a golden-hour forest edge gets long tree shadows across the slope, shade under every crown and bushes anchored to the ground (`BEST_forest_edge_golden.png`, `breakdown_forest_edge_golden.png`). This is the one change in the track that makes a frame read differently at a glance.

The bug was later fixed in a scratch worktree (see `bugs.md`, object-shadows), but that fix was **not landed** because it also applied the terrain-detail tier at boot. A second attempt was in progress when this page was written.

## The technique (the rest of the track)

- **PCSS** (percentage-closer soft shadows): the sun is treated as a disk. A 16-tap search in the raw shadow depth finds the blockers; the soft edge grows with the distance from blocker to receiver. A 1-degree sun was the sweet spot; 0.53 (the real sun) looks like stock with crisper edges; 2 to 3 degrees look mushy.
- **Contact shadows:** a short march toward the sun through the depth buffer in the post pass.
- **Shadow map setup:** 4096 maps and tighter near tiers through existing test verbs.
- Knobs: `pcssSun`, `pcssSearch`, `pcssMin`, `contact*`, `objShadows`, `casterLod`, `debug=5`.

## What real data showed

Eight rounds on Abel and Sinai.

- **forest_edge_golden:** stock has no tree shadows; casters give long diagonal shadows and shade under the crowns. PCSS at 1 degree softens their far ends; the soldier's shadow joins his boots (visible only in a 4x crop).
- **Woodland** (round 8): dappled sun on the forest floor at noon (`BEST_woods_mixed_noon.png`) and long raking tree shadows over a meadow at 17:30 (`BEST_woods_pine2_evening.png`).
- **Town at 18:15:** houses and the big tree put most of the street in shade. Physically right, but the frame is mostly darker.
- **Open ground and noon:** almost no change. Contact shadows invisible.
- **A cheap production set** ("prod": casters plus PCSS 1 degree on the stock 2048 map, stock caster LOD, tighter near tiers) looks the same as the 4096 "best" at a fraction of the shadow-pass cost.
- **Motion:** no extra flicker in colour during a slow dolly. The sun-visibility term alone flickers on tree crowns (leaf cards shading each other through alpha aliasing): 14% of crown pixels against 0.8% without casters.
- **Woods get darker:** two of three woodland frames are 14% and 21% darker than the original look. Exposure x1.25 recovers only a quarter to a half of that.
- **Failed:** contact shadows (the shadow map already covers almost all of them); doubled shadow range (no visible gain for +5 to 8 ms).

## Cost

Shadow pass goes from about 0.03 ms (vehicles and soldiers only) to 0.2 to 1.4 ms with "prod", 1.2 to 3.2 ms with "best" (4096). PCSS itself is unresolved, 0 to 3 ms. Contact shadows +0.07 ms. 4096 maps use four times the memory of 2048. Whole-frame numbers swung by 2 to 10 ms on the shared GPU.

## Verdict

Critic: switching the tree and building shadow casters back on "is the one change a player would see". PCSS and contact shadows are close to invisible, the woods get 14 to 21% darker and greyer than the original look, and the 4096 tight-tier setup has an unflagged shadow artefact on Sinai.

## What production needs

1. Fix the start-up order so the Graphics shadow tier really drives object shadows.
2. A light fill under tree crowns: a transmittance layer for leaf casters in the shadow pass (let 15 to 25% of the sun through, tinted by the leaf colour), or bounce light. Without it woods go dark and grey.
3. PCSS only as optional polish, with temporal or bilateral smoothing of the penumbra (the fixed per-pixel dither can crawl).
4. A larger normal offset or bias for foliage receivers against leaf-card acne.
5. 4096 only on a High or Ultra tier. Validate the tighter tiers before shipping them.
6. Drop contact shadows and the doubled range.

## Overclaims the critic found

- "Sinai: almost no change (no static casters at the camp)" is wrong. The camouflage net casts. With 4096 and tight tiers the near tier puts almost the whole right foreground in solid shadow behind a tier-shaped edge (lit ground about 20% darker), which the other setups do not show. At least one setup is broken and it will pop in motion.
- PCSS is close to invisible at frame scale (mean difference 0.001 to 0.004), despite the track's title.
- The missing-casters finding was partly documented already (the `triEnableShadows` comment in `GameStateExtTestViewer.cpp`). The boot-order root cause is new.
- Not reported: every run of this track logged a GL undefined-behaviour warning (a comparison sampler on texture 0). It was later fixed on the branch (f102499, see `bugs.md`).

## Taste risks

Woodland frames 14 to 21% darker than the original look. The evening woods read cooler (red/blue 1.43 against 1.64 for the original look). Grey, desaturated shade lit only by blue sky (shade against lit about 0.52, closer to the "crushed" 0.42 than to the 0.6 target). Flat, uniform shade in town. Crown self-shadow is a flicker source.

## Where the work lives

- Notes and patch: `tmp\research\shadows\NOTES.md`, `prototype.patch` (22 KB), cost tables `run4\cost_paired.md`, `run5cost\cost_paired.md`, brightness `run8prod\frame_brightness.md`.
- Scratch worktree: `gfx-shadows`, build `dist\gfx-shadows-rwdi`.
- Best images (not committed): `tmp\research\shadows\BEST_woods_mixed_noon.png`, `BEST_woods_pine2_evening.png`, `BEST_forest_edge_golden.png`, `breakdown_forest_edge_golden.png`, `scratch\r6_soldier_feet_zoom.png`, `run4\shots\sinai_lowsun_fp__dbgsunvisbest.png` (the Sinai artefact).
- Integration: the start-up fix is the priority (see `bugs.md`). PCSS is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.
