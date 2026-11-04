# Overnight experiments: morning summary (2026-11-03)

**Headline.** Ten of the eleven experiment lanes are now on branch `graphics-experimental` (worktree `D:\Arma_CWA_worktrees\graphics-experimental`, not pushed), one commit per lane, every switch kept, and the combined tree builds and passes the unit gate (only the six known external-data failures). Only one experiment is turned on by default: **xp-cputhreads**, which spreads some CPU frame preparation over worker threads. Its pictures are identical to today's (one Sinai shot differs by exactly the tip's own boot-to-boot noise, and its built-in cross-check found zero differences), and its timed run cut CPU work by **0.8 to 1.0 ms** on valley, sunrise and Eden views. Everything else stays off: most of it changes the look, much of it in motion, and needs your eye on clips first. The clearest look win is **xp-water** (real beaches instead of a hard sea edge, cost within noise). xp-farshade never built. xp-impostor crashed on exit whenever it was on; that is fixed on the branch but has no pictures yet.

## What each lane tried

| lane | what it tried | result | measured | look |
|---|---|---|---|---|
| xp-cputhreads | draw-list sorts, hill tests, shadow copies, LOD finds and object prep on worker threads | works, **on by default** | CPU work -0.80 ms valley_0515, -0.98 sunrise_ridge, -0.88 broken_high (clean run) | identical to the tip (7 of 8 IDENTICAL, Sinai at the boot floor), 0 verify differences |
| xp-water | shallow water colour, surf line, foam and wet sand where the sea meets land | works | +0.03 ms GPU, within noise | big win at shorelines, see pictures below |
| xp-viewdist | exact early cull of far objects; optional far ring of big objects out to the fog; optional tiny-object cull | partial | not timed yet | early cull IDENTICAL on 6 views; the far ring fills bare far hills (NEAR on 4 views) |
| xp-presets | Low / Medium / High / Ultra presets that really change the frame | partial | Low timing run came back contended, rest pending | High is IDENTICAL to today; Low and Medium drop many far objects (valley draws 5242 to 2137 at Low); Ultra adds objects and longer shadows |
| xp-gpuprofile | early-Z program twins, terrain front to back, a fine GPU profiler | partial | not timed yet | early-Z IDENTICAL on 6 of 7 (Sinai at floor; all 6 twins linked); front to back changes a few pixels at segment seams |
| xp-foliage | backlit leaf glow, fuller distant crowns (alpha mip coverage), wind sway | partial | not timed yet | off is IDENTICAL; fuller crowns change most forest views; leaf glow is subtle except into a low sun |
| xp-particles | soft smoke edges against the ground, smoke lit by sun, moon and fires | partial | +0.1 to 0.3 ms CPU, up to +0.7 ms GPU at 2x scale (ffh) | off is IDENTICAL; fire smoke changes on night views and the day smoke test |
| xp-post | contrast-adaptive sharpen, longer bloom tail | partial | not timed yet | bloom changes sunset, sunrise and coast scenes a little, noon not at all; CAS changes texture detail slightly |
| xp-ambientocc | ambient occlusion out to 900 m, contact shadows to 150 m | partial | not timed yet | small changes on 6 views (NEAR); the off-identity proof is still queued |
| xp-impostor | far trees drawn as baked cards in one instanced draw | partial, **crashed on exit when on** | none | none: both of its jobs lost their boots to the exit crash; fixed in b49627d9, needs a rerun |
| xp-farshade | cheaper shader programs for far objects | **failed: never built** | none | none; skipped, not on the branch |

xp-atmosphere, xp-nightlight and xp-shadows were not part of this integration batch.

## Pictures

- Water (the best ones): `items\XP-xp-water\showcase\r3_shore_north_noon_top_off_bottom_on.png`, `r3_coastline_sunset_waterline_x2_top_off_bottom_on.png`, `r3_shore_north_sunset_A_off.png` / `B_on.png`. The opt-in wave masking: `r2_bay_offshore_mask_option_top_off_bottom_on.png`.
- No other lane made a showcase folder. Each finished stills job has an A/B `blink.gif` and `diff_x8.png` per view under `items\XP-<lane>\results\<job>\vgate\<view>__gold\`. Worth a look:
  - far ring: `items\XP-xp-viewdist\results\stills-far4\vgate\valley_0515__gold\blink.gif`
  - presets: `items\XP-xp-presets\results\stills-low-vs-off\vgate\` and `stills-ultra-vs-off\vgate\`
  - fuller crowns: `items\XP-xp-foliage\results\r1-stills-mipcov\vgate\forest_edge_golden__gold\`
  - leaf glow: `items\XP-xp-foliage\results\r1-stills-transmit\vgate\pinewood_morn__gold\`
  - lit smoke by day: `items\XP-xp-particles\results\20261103-065037-stills-r2-dayboth\vgate\`
  - bloom: `items\XP-xp-post\results\20261103-062010-stills-bloom\vgate\sunrise_ridge__gold\`
  - ambient occlusion: `items\XP-xp-ambientocc\results\stills-look\vgate\`

## Needs your eye (clips, not stills)

Water (foam drift, the moving waterline), far ring (objects appearing as the camera moves), presets Low and Medium (pop-in at the cut distances), wind sway (shadows do not sway, TAA may smear), leaf glow (sparkle when panning into the sun), lit and soft smoke, CAS sharpening and the bloom tail (shimmer), far ambient occlusion (grey veil or crawl on far ground), impostors (a pop at 600 m, view switching on orbit). None of these is on by default.

## What failed or was changed by the integrator

- xp-farshade: the build and tests never ran before the lane had to stop; left off the branch.
- xp-impostor: with the switch on, every boot crashed at shutdown (the last reference to a baked tree shape was dropped after its textures were gone). Fixed by keeping those shapes alive; not yet seen in a game.
- xp-viewdist plus xp-cputhreads: both rewrote the same object walk. The merge keeps both, and the early cull now also runs on the parallel path, so the two can be on together. That combination has not been run in a game.
- Untested combinations to watch: xp-particles and xp-impostor both use texture unit 17 (different texture types, so probably fine); xp-viewdist's far ring and impostor FAR=1 both fill the far band.

## Checks

- Build green (5 warnings, the known Network ones); unit gate: Foundation 475/475, Core 823/827 (4 skipped), PoseidonTests 3547/3557 (the 6 known data failures, 4 skipped), determinism tags 39 cases green. Same after the default-on commit.
- Frozen dist: `D:\Arma_CWA_worktrees\graphics-experimental\arma_CWR\dist\xp-integ-rwdi` (exe sha256 bce8cdaf7ca9...). Do not rebuild `build/xp-integ-rwdi` while its jobs are pending.
- Queued at prio 70: `70-20261103-145500-XP-INTEG` (stills, branch default against integ7, should be IDENTICAL) and `70-20261103-145501-XP-INTEG` (timed quick against integ7). Results land in `items\XP-INTEG\results\`.
- Still queued from the lanes: timings for viewdist, impostor, ambientocc, post, foliage, presets and gpuprofile. Impostor's jobs use its old pre-fix exe and will crash again; requeue them on the xp-integ dist with `POSEIDON_XP_IMPOSTOR=1`.

## Commits on graphics-experimental (on 243ef1e0, oldest first)

1. b8b681ee xp-ambientocc: far GTAO and longer contact shadows behind switches
2. 2752d88e xp-post: contrast-adaptive sharpen and long-tail bloom behind switches
3. 00d74ee3 xp-water: shallow water, surf and wet sand at the shoreline behind a switch
4. 42e9b798 xp-particles: soft particles and lit smoke behind switches
5. 0fcbc1c6 xp-presets: quality presets that change the frame behind switches
6. e0d8aef5 xp-impostor: baked far-tree impostors in one instanced draw behind a switch
7. 6b609779 xp-foliage: leaf transmission, alpha mip coverage and wind behind switches
8. a9c3e9cd xp-gpuprofile: early-Z twin programs, front-to-back terrain and a fine GPU profiler behind switches
9. 30fe1162 xp-viewdist: exact early cull, far ring of big objects and ground counters behind switches
10. 54d02187 xp-cputhreads: parallel draw-list sorts, hill tests, shadow, LOD and prep stages behind a switch
11. b49627d9 xp-impostor: keep baked shapes alive past GL shutdown
12. e3f5f20d graphics-experimental: turn on the recommended experiments (xp-cputhreads only)

## Second batch (shadows, atmosphere, night lights)

**Headline.** All three lanes are now on `graphics-experimental`, one commit each, every switch kept and off. The combined tree builds (only the 5 known Network warnings) and passes the unit gate (only the six known external-data failures). Nothing new is on by default: all three change the look, and all three are temporal, so you need to see clips first. The default frame should still be the tip's; a stills job against integ7 is queued to check that.

| lane | what it tried | result | measured cost | look |
|---|---|---|---|---|
| xp-shadows | shadow quality tiers (`POSEIDON_XP_SHADOW_TIER` 0 to 4, 2 = today), sharper 4096 maps, shadows reaching 1000 m (ultra) or 1600 m (extreme) instead of 500 m, a per-cascade caster cull, plus single knobs | works | ultra vs today: +0.70 ms CPU / +0.65 ms GPU valley, +0.53 / +0.71 broken_high, +0.35 / +0.19 noon_fields, +0.09 / +0.05 Sinai. Extreme vs low: up to +1.5 CPU / +2.0 GPU on broken_high. Every row stays inside the 15 ms p95 budget | ultra is clearly better on long views: shadows 500 to 1000 m out, crisp tree shadows instead of smudges |
| xp-atmosphere | lens glare round a low sun (`POSEIDON_XP_SUNGLARE=1`), faded by how much of the sun is really visible | works | +0.01 to +0.05 ms CPU and GPU, at the noise floor | a softer, wider warm glow at sunset; a small polish, not a big change |
| xp-nightlight | street lamp shadows and light bouncing off the ground (`POSEIDON_XP_NIGHTLIGHT=1`), plus lit windows (separate switch) | lamp shadows and bounce work; window glow failed | town_night_rain +0.11 ms CPU / +0.48 ms GPU (one pair, rain, so a rough number); combat_fires nothing measurable | the night town looks grounded: the lamp throws the tree across the square |

### What changed visually (pictures)

- Shadows, tip on the left and ultra on the right: `items\XP-xp-shadows\showcase\01_valley_0515_tip_vs_ultra.png` (sunrise forest: same darkness, but individual tree shadows), `02_hill_1730_tip_vs_ultra.png` (far tree groups get crisp long shadows), `03_broken_high_tip_vs_ultra.png` (bushes and the forest edge 500 to 1000 m out now cast shadows).
- Glare: `items\XP-xp-atmosphere\showcase\coastline_sunset_off.png` / `coastline_sunset_glare.png`, `valley_0515_off.png` / `valley_0515_glare.png`, and the taste call `sinai_lowsun_fp_off.png` / `sinai_lowsun_fp_glare.png` (a mild veil over the dark jeep under the sun). Crop strip: `crop_coast_off_r3soft_r4final.png`.
- Night: `items\XP-xp-nightlight\showcase\final_town_night_A_off.png` / `final_town_night_B_nightlight.png`, the trunk shadow crop `r1_town_night_lampshadow_crop.png`, fog and rain `r3_town_fog_night_*` and `r3_town_night_rain_*`, the burning hull lit by its own fire `final2_combat_fires_A_off.png` / `final2_combat_fires_B_nightlight_quarterfire.png`. noon_fields is identical (nothing runs by day).

### What the late shadow jobs said (all finished, all clean)

- The caster cull is pixel-identical at today's settings (6 of 6) and at ultra (3 of 3, `results\stills-ultra-nocull`). It has no timing row of its own, so it stays off.
- The cost is the reach, not the map size: 2048 maps reaching 1000 m cost the same as ultra (+0.75 CPU / +0.58 GPU valley, `results\quick-highfar`) and look close to ultra but not the same (NEAR on 5 views).
- Medium and Low change the look (Low is CHANGED on forest_edge, hill and valley, with a sharpness-loss tripwire in the valley) and save nothing on this card. Keep High as the floor here.
- Filter taps 2 and 8 change little (NEAR or PI-T); they mainly matter for TAA noise in motion.

### Needs your eye (clips, not stills)

- Shadows: wider reach and tight cascades change how the far shadow tiers snap while the camera moves.
- Glare: it fades as leaves or a ridge cross the sun (smoothed over about 4 frames); also judge the Sinai veil.
- Lamp shadows: they are screen-space, so a shadow can appear or vanish as the camera turns, and the noise changes every frame under TAA.

### What failed or was changed by the integrator

- Window glow failed on the stock houses: the windows are painted into the facade textures in the same dark colours as the timber beams, so every colour rule lit beams instead of panes. It stays as its own off switch and needs authored window masks to work.
- Tip bug found by xp-nightlight: in dry weather every texture uses surface class 0 (the class table is only built for wet weather). `POSEIDON_XP_SURFTABLE=1` is the probe (the Jeep windscreen gains a glass reflection, `tipbug_surftable_noon_fields_windscreen_crop.png`). The one-line fix belongs on graphics-remaster after your look.
- Merge fixes: xp-shadows and xp-presets both set the shadow reach. Now xp-presets picks the base reach and xp-shadows widens it. The occlusion test range follows the same number, so a hidden far object never casts from the wrong detail level. The xp-shadows lane alone did not do this. xp-nightlight's constant slot moved from 91 to 92 (xp-water has 91). Its vertex shader change and xp-foliage's defines are combined, and its per-draw window hint also reaches xp-gpuprofile's early-Z programs. When both xp-presets and xp-shadows set a map size, xp-presets wins.
- Untested combinations: xp-atmosphere borrows texture unit 14 (the combat mask's) in the final composite and restores it, while xp-nightlight writes lamps into that mask earlier in the frame. Neither has been run with the other, or with xp-presets Ultra plus a shadow tier.

### Checks

- Build green (5 known Network warnings). Unit gate: Foundation 475/475, Core 823/827 (4 skipped), PoseidonTests 3554/3564 (the 6 known data failures, 4 skipped), determinism tags 39 cases green.
- Frozen dist: `D:\Arma_CWA_worktrees\graphics-experimental\arma_CWR\dist\xp-integ2-rwdi` (exe sha256 fcc86b5651a6...), build dir `build/xp-integ2-rwdi`. `build/xp-integ-rwdi` and `dist/xp-integ-rwdi` were not touched.

### New commits on graphics-experimental (on e3f5f20d, oldest first; not pushed)

13. d43f4cee xp-shadows: shadow quality tiers, longer reach and per-cascade caster cull behind switches
14. ee36ceb0 xp-atmosphere: low-sun lens glare scaled by the visible disc behind a switch
15. 51a290c9 xp-nightlight: street lamp shadows and ground bounce behind switches

### Queued (prio 71, dry runs exit 0, results in `items\XP-INTEG2\results\`)

- `71-20261103-172200-XP-INTEG2`: branch defaults vs integ7, claim exact, 10 views (`stills-xpinteg2-vs-integ7`). Expect IDENTICAL, with Sinai at the boot floor and town_night possibly one boot frame off.
- `71-20261103-172201-XP-INTEG2`: shadows tier 3 on vs off, same exe (`stills-shadows-ultra`).
- `71-20261103-172202-XP-INTEG2`: sun glare on vs off, same exe (`stills-atmosphere-glare`).
- `71-20261103-172203-XP-INTEG2`: night lights on vs off, same exe (`stills-nightlight`).
