# Look lane "surface": surfaces and ambient light (critic round 2)

Scratch copy `D:\Arma_CWA_worktrees\look-surface`, detached at **fec1075** (graphics-remaster HEAD
with B6 SSR, B7 wet materials and B4 combat light). Nothing committed, no branch. The change is
`lane.patch` (`git diff --binary`, new files `git add -N`): 33 files, 2,191 lines in, 54 out.
`git apply --cached --check` passes on a fresh index of fec1075 (checked after the last edit). The
round-1 notes are in `E:\look-lanes\look-surface\r3\NOTES_round1.md`; the diffs the three rebases
started from are `r2/pre_rebase_r2.patch` (on 06c7e4d), `r2/pre_rebase_r3.patch` (on c36ee4b) and
`r2/pre_rebase_r4.patch` (on 4a66d74).

Rebases this round (no branch, no commit): the detached HEAD was moved with `git reset --hard` and
the saved diff re-applied with `git apply --3way`, conflicts merged by hand, three times, as the
other workflow landed:
- onto c36ee4b (B6 SSR): B6's debug views 9 and 10 come first; knob and landed counts add up;
- onto 4a66d74 (B7): B7's material classes replace the lane's own classifier (point 9); B7's wet
  terms stay inside the lane's shading branch (`wetKeep` on the diffuse, `wetLit` and `wetEnv`
  added, `wetMirror` taken out of the occlusion's share);
- onto fec1075 (B4): B4's combat mask is debug view 11, the lane's views are 12 to 14; the combat
  light shades with the local lights in the lane's branch and its highlight adds; the composite
  keeps both B4's scattering and the lane's occlusion; the two composite shader pieces were merged
  by hand (one `fragColor` declaration).

Every number below is from the final build on fec1075 (hero run `u10`, timing `t11`, cost split
`cpu1`, motion `pan6`) unless a run name says otherwise. Build `build/look-surface-rwdi`,
`dist/look-surface-rwdi` (junctions to `E:\look-lanes\look-surface\`; D: was nearly full). Captures
in `E:\look-lanes\look-surface\captures\`. Games only through run-captures.ps1 with the lane's
junction (`RemasterUD-look-surface`; `RemasterUD-look-surface2` for one parallel variant run); my
games were stopped twice with `stop-owned-games.ps1 -DataDir` on my junction (it reported the other
lanes' games as left alone).

## Verdict first

The lane is no longer a supporting term only. Share of pixels whose luma moves by more than 16
levels at 480 px, lane off against lane on (`pixel_stats.md`):

| view | lane off vs on | Original vs Overhaul |
|---|---|---|
| lowsun_fp | 10.5% | 24.5% |
| sinai_lowsun_fp | 7.6% | 22.6% |
| noon_fields | 13.4% | 16.9% |
| forest_edge_golden | 11.9% | 36.3% |
| town_night | 9.4% | 79.1% |
| tank_gunner_interior | 10.6% | 15.4% |
| jeep_close_noon | 17.3% | 27.5% |
| town_noon | 11.0% | 35.1% |
| forest_morning | 23.5% | 41.9% |
| sinai_desert_noon | 13.7% | 18.4% |

(The critics measured 3.7% for round 1; noon_fields was 1.4% by more than 8 levels.) What does it
at a glance: noon shade that reads as shade (the ground's cast shadows at about a quarter of the
sunlit ground, darker shaded hull sides), visible ground texture at noon (stones, ruts, sand
ripples), deeper low-sun shadows, darker interior corners. I looked at `contact_sheet.png`
(Original | Overhaul, 480 px a panel) and the difference is plain in every day view, including
noon_fields and sinai_desert_noon, which were near white sheets in both before. The interior and
the town's facades are the lane's weakest views; the rest of the jump has to come from the haze,
exposure and sky lanes.

## The critics' points, one by one

Pixel figures are from `scripts/metrics3.py` on the 960x640 hero frames, lane off
(`remastered,surfaces=0`) against lane on. My boxes are in the script; they are not the critics'.

### 1. Noon ground: relief and cavities in the sky and sun share

New cavity term (`surfCavity`, default 2.0): a texel's height below the mean of its neighbourhood
loses sun and sky.
- The land: the detail texture's grain (its alpha at 2.5 pixels' footprint against 10) plus the
  square's own texture at about a metre (its texel against the mean of about 11 texels, mips at
  least 0.5 and 3.5 and the pixel's own footprint beyond that). Fades out by 80 m (grain) and from
  60 to 200 m (square).
- Objects: the texture's luminance in the same band, by class (road, masonry, roof 1, wood 0.8,
  default 0.5; paint, metal, glass, rubber, skin, cloth 0).
- Shading: the sun term times `pit = clamp(1 + 3 * cavity, 0.5, 1.25)`, the sky term times
  `clamp(pit, 0.7, 1)` (a bump never gains sky, a pit loses at most 30% of it).

Bottom-third luma std (full frame), lane off / on:

| view | off | on | ratio | asked |
|---|---|---|---|---|
| noon_fields | 21.8 | 30.8 | 1.41 | 1.5 |
| sinai_desert_noon | 20.4 | 30.5 | 1.50 | 1.5 |
| jeep_close_noon | 19.3 | 30.2 | 1.57 | 1.5 |
| town_noon (extra) | 16.3 | 22.5 | 1.38 | |

noon_fields misses the 1.5. `surfCavity=2.6` (`n2`) gave 1.46 there, `2.4` (`u7`) 1.43, for changes
I could barely see at 480 px; the clamps limit it, and wider clamps brought back stains (see "Tried
and dropped"). The bottom third's mean drops 11 to 12 levels at noon (noon_fields 161.6 to 149.4):
less of a white sheet. Crop: `critic_round2/z_noon_ground.png`.

### 2. Shade-to-sun ratio

New knob `surfShade` (default 0.45): the share of the rig's sky fill a non-leaf surface keeps under
a clear day sky. It fades back to 1 below 18 degrees of sun (gone at 3), under overcast
(`skyThrough` 0.85 to 0.35) and at night (`SurfaceShadeFactor`, unit-tested), so overcast days and
nights keep their fill. Only the sky's part of the fill is cut (the rig hemisphere's ground share is
kept), so a hull's lower side keeps the ground's warm bounce and the paint its colour. Leaves keep
their fill (their crowns are lit as a canopy). The sky reflection takes the same factor.

| jeep_close_noon boxes | off: shade / sunlit | on: shade / sunlit | ratio off / on |
|---|---|---|---|
| ground: the jeep's cast shadow / open ground | 77.8 / 156.6 | 35.7 / 136.1 | 0.50 / 0.26 |
| jeep side panel / canvas top | 35.2 / 76.4 | 28.2 / 69.3 | 0.46 / 0.41 |
| jeep side / fender top | 42.7 / 74.6 | 29.2 / 51.9 | 0.57 / 0.56 |

The ground reaches the 25 to 35% asked. The hull's shaded sides sit at 28 to 29 levels (7 to 14
darker), but their ratio to the hull's lit parts moves little: the lit parts lose fill too, and the
hull keeps the ground's bounce. Cutting the bounce as well made the shaded olive black-grey and
lowered paint saturation (`u1` to `u4`). Crop: `critic_round2/z_jeep_shade.png` (Original, lane
off, Overhaul).

### 3. Building occlusion

- Static architecture (object kind Static, or the classes road, masonry, wood, roof) writes an
  occlusion strength code into the surface target's alpha (1 against 0.5): the composite raises its
  occlusion power by `kSurfaceStaticAoBoost` 1.8, and the GTAO pass reads the code from the
  resolved target and searches twice as far (`kGtaoStaticRadiusScale`: 8 m by default).
- The cavity term darkens mortar, plaster cracks and roof gaps (masonry and roof class 1).
- The broad GGX sun sheen on rough dielectrics keeps 0.15 of itself from roughness 0.55 to 0.9: the
  specular view (`u1`) showed it as an even grey veil over the whole sunlit facade, taking the
  warmth out of the plaster.

town_noon, brick front (620,300)-(750,355): (49, 43, 38) off, (39, 34, 30) on. The occlusion view
(`critic_round2/z_town_facade.png`, third panel) shows the eave line, the storey seam under the
jetty and the window heads dark; in colour they read as a darker seam and darker window heads at
full size. The windows are painted on flat walls (no geometry), so no screen-space occlusion finds
a recess; the cavity darkens them a little. At 480 px the town's change (11.0% by more than 16
levels) is mostly the ground, the tree's shadow, the parked jeep and the shaded brick fronts, not
the eaves.

### 4. Tank interior

The interior is drawn after a depth clear and pre-lit (classic vertex light), so the lane gave it
nothing: the Ao stage was held off without the world depth, and pre-lit fragments wrote no share.
The near-pixel slice reduction was not the cause. Now:
- `PostFrameInputs::surfaceInterior`: with the surface target written and the world depth cleared,
  the Ao stage runs on the interior's own depth (`PlanPostStages`, unit-tested);
- the GTAO radius there is `kGtaoInteriorRadius` 0.6 m (no static boost, no contact shadows);
- `SurfaceEnterInterior` sets `litSurface3.y`; pre-lit opaque fragments then hand the occlusion
  `kSurfaceInteriorShare` 0.9 of their light.

tank_gunner_interior: 10.6% of pixels by more than 16 levels. The corners around the breech, the
sight housing's underside, panel seams and the hatch rim darken (`critic_round2/
z_tank_interior.png`: lane off, Overhaul, the occlusion view).

### 5. Vehicle paint

The matt sky reflection face-on is gone (rough dielectrics from roughness 0.5: none face-on, up to
0.3 at grazing, paint 0.2, rubber none), and the ground bounce stays in the shade (point 2).

| view, box | S off | S on, round 1 | S on, now | RGB on, now |
|---|---|---|---|---|
| jeep_close_noon (400,240)-(620,400) | 0.147 | 0.112 | 0.152 | (34.7, 34.9, 32.9) |
| noon_fields (530,350)-(720,450) | 0.146 | 0.114 | 0.150 | (34.6, 34.7, 31.9) |

At least the lane-off saturation, red above blue. The body is darker (the shade), not greyer.

### 6. The lane's visible signature

See "Verdict first", `contact_sheet.png` and `sheet_3way.png` (Original | lane off | Overhaul).

### 7. Static low-sun occlusion noise: temporal accumulation

New post shader `psPostGtaoAccum` (`PostSurfaceAccumulate`, after the stock blur): under TAA each
half-resolution occlusion texel is reprojected into the last frame with the TAA resolve's camera
mapping (`BuildTaaReprojection`), the history is kept at `kGtaoHistoryWeight` 0.85 where its depth
matches (3% plus 10 cm), and this frame's occlusion mixes in at the rest; a cut, a resize, a frame
without TAA or a missing history starts again. With the history the GTAO takes 2 slices a pixel
(`kGtaoTemporalSlices`) instead of 3.

Static camera, round to round, mean |d| of 8-bit luma, bottom third (1080p; `temporal.md` from
`t11`, `temporal_isolation.md` from `u10`, each mode its own rounds in the same game):

| view | lane off | lane on | occlusion only (cavity 0, relief 0) | relief only (cavity 0) |
|---|---|---|---|---|
| lowsun_fp | 0.31-0.35 | 0.51-0.60 | 0.30-0.35 | 0.44-0.55 |
| noon_fields | 0.76-0.79 | 0.73-0.79 | 0.61-0.73 | 0.69-0.87 |
| forest_edge_golden | 0.42-0.47 | 0.49-0.54 | | |
| forest_morning | 0.41-0.45 | 0.53-0.56 | | |
| sinai_lowsun_fp | 0.38 | 0.34-0.41 | | |
| town_noon | 0.33-0.37 | 0.23-0.35 | | |

The occlusion now adds nothing over the lane-off frame (lowsun_fp 0.30-0.35 with only the
occlusion on, against 0.53-0.56 in round 1 for the whole lane, most of which was the occlusion).
What remains at a low sun is the texture's relief and cavities: fine ground detail that the TAA
jitter moves. I band-limited both this round (the relief reads a mip coarser; the cavity is a band
from 2.5 to 10 pixels' footprint, which took lowsun_fp from 0.71 to 0.57 in `n1` against `n2`),
and the square texture's cavity now follows the pixel's footprint (fixed mips aliased in the
distance). lowsun_fp stays at about 1.7 times the lane-off frame's change there.

In motion (`pan6`, `pan_stats.md`): the raw occlusion buffer (debug 4) moves 0.36 (lowsun_fp) and
0.45 (town_noon) levels in the bottom third between two pan frames, against 0.54 and 0.57 in round 1.
The lowsun_fp pair of this run was shot with a 1.7 px pan shift for the lane-on and debug modes and
0.34 px for lane off, so its lane-on figure (2.79) is resampling blur, not comparable; town_noon:
1.01 off, 1.28 on.

### 8. Glass (blocking)

A blending texture (its alpha class Blend, in the per-draw uniform's z) is a cover: it writes no
occlusion share and no sun share, opaquely (alpha 1) wherever its alpha is above 0.02, so the post
pass never darkens what is seen through a windshield or a canvas window. It gets no matt sky
reflection either (glass and lenses keep their mirror reflection), so a see-through canvas panel is
no longer a pale card.

lowsun_fp windshield (108,305)-(135,340): Original (83, 74, 74), lane off (105, 93, 84), Overhaul
(102, 87, 76). The crop `critic_round2/z_windshield.png` (Original, lane off, Overhaul, 3x) shows
the field and the horizon through the glass in all three; the 3 to 8 levels are the darker ground
behind it (cavity and shade), not the glass. The rear canvas window and the spare tyre:
`critic_round2/z_jeep_tyre_curtain.png` (the mesh stays see-through, the tread keeps its grooves).

### 9. Sky reflection times occlusion; slot 58; CPU cost

- The sky reflection is part of the occludable share (the GTAO darkens it), it is multiplied by the
  canopy visibility and the shade, and the face-on matt reflection is gone (point 5). town_noon
  upper storey (190,185)-(360,250): B 28.8 off, 31.8 on (round 1 build: 34.2); windows: left
  (34, 31, 25) to (32, 31, 26), right (36, 32, 24) to (31, 29, 23). The upper storey's 3 levels of
  blue remain; I did not find their source.
- Slot 58 is B7's `MatClass` (landed in 4a66d74). The lane uploads nothing per draw into the
  LitConstants block any more: its class rides in a plain uniform of the lit programs (`surfClass`:
  x B7's `SurfaceClass`, y B7's `DrawObjectKind`, z 1 for a blending texture), set with
  `glUniform4f` on the bound program only when that program's cached value changes
  (`EngineGL33::ApplySurfaceClass`; location cached per program, cache cleared when the programs are
  deleted). The class is B7's (`TextureGL33::SurfaceClassFor`, once per texture and kind); the
  lane's own classifier, draw kind and `Shape::Draw` hook are gone. The shader's class table uses
  B7's numbering (the enums were already the same). B7 uploads `litMatClass` only while its wet
  gate is on, so the lane cannot read that slot on a dry day.
- CPU: the critics' +0.6 ms is mostly not per-draw work. The cost split `cpu1` (`timing_split.md`,
  three clean rounds each): drw:obj CPU 5.59 off, 5.79 with the switch on and every term at zero
  ("bare": the class uniform, the shade, no target), 6.47 with the shader terms on and no occlusion
  target, 6.55 all on (lowsun_fp); town_noon 5.68, 5.78, 6.53, 6.59. The CPU time follows the GPU's
  world cost (the shader terms), so it is the driver waiting on a busier GPU; the lane's per-draw
  CPU work is the 0.10 to 0.20 ms between off and bare.

### 10. Noon gate, slices

Under TAA (every hero view but the interior) the GTAO runs 2 slices a pixel instead of 3, the
accumulation covering the rest; the ao stage costs 0.24 ms at noon_fields. I did not add a noon
gate: at noon the lane now changes 13 to 17% of the pixels by more than 16 levels, and most of its
noon cost is the world pass's cavity and relief reads, which make that change.

### 11. Crops re-exported

`critic_round2/fm_left.png`, `fm_right.png`, `fm_top_edge.png` (forest_morning, lane off |
Overhaul) are cut from the final frames by `scripts/crops3.py`, which matches the exact file name
(round 1's `crops.py` matched a suffix pattern that also matched the debug runs' frames). They show
the crowns with their colour and no sheet, the forest floor's shadows deeper, no band along the top
edge.

### 12. Rubber

Rubber reflects no sky and takes no relief or cavity. jeep_close_noon spare tyre
(395,300)-(430,380): (31, 32, 35) off, (27, 27, 30) on; the tread's grooves stay
(`critic_round2/z_jeep_tyre_curtain.png`).

## What is built (state after this round)

All behind `surfaces` (default 1; registry row "Surfaces and ambient light"; GPU section `ao`).
Gate as round 1 (off once the sky is dark and no lamp is lit). `baseline` turns it off.

1. Surface target (MRT, RGBA8 multisampled): r the occludable share, g the albedo's square root,
   b the sun's share, a the occlusion strength code (0.5, static architecture 1); covers write
   share 0.
2. GTAO, half resolution, 2 slices under TAA (3 without) x 2 sides x 8 steps, static architecture
   twice the radius, interiors 0.6 m; the stock depth-aware blur; the temporal accumulation.
3. Composite: depth-aware upsample, power `surfAo` (x1.8 on static architecture), multi-bounce,
   contact shadows (not in interiors).
4. Canopy sky occlusion (as round 1).
5. Sky light: the rig's hemisphere, its sky part times the shade (`surfShade`).
6. Specular: GGX sun lobe (cut on rough dielectrics), split-sum sky reflection (glass, metal; matt
   surfaces only at grazing), none on leaves, rubber and non-glass covers; B7's wet film on top.
7. Relief (as round 1, a mip coarser) and the cavity term.
8. Vehicle interiors: their own occlusion.

## Knobs (A1 registry, sub-knobs of `surfaces`)

| key | default | what |
|---|---|---|
| `surfaces` | 1 | master switch |
| `surfAo` | 3.0 | occlusion strength (a power); 0 = the stock occlusion |
| `surfAoRadius` | 4.0 | metres the horizon search reaches (static architecture twice that) |
| `surfCanopyAo` | 1.5 | sky lost under crowns, woods, building blocks |
| `surfSkyLight` | 0.0 | share of the sky light from the sky table's harmonics |
| `surfSkyTint` | 0.25 | share of the table's colour in that light |
| `surfSkyLevel` | 0.0 | the table's sky level against the rig's (with `surfSkyLight`) |
| `surfSpec` | 1.0 | specular on every surface but leaves |
| `surfContact` | 1.0 | screen-space contact shadows |
| `surfRelief` | 1.5 | relief from the textures |
| `surfShade` (new) | 0.45 | share of the sky fill kept in clear daylight; 1 = the original's |
| `surfCavity` (new) | 2.0 | sun and sky a texture's pits lose; 0 = none |

Constants (SurfaceLight.hpp, GLSL): `kGtaoTemporalSlices` 2, `kGtaoHistoryWeight` 0.85,
`kGtaoInteriorRadius` 0.6, `kSurfaceStaticAoBoost` 1.8, `kGtaoStaticRadiusScale` 2,
`kSurfaceInteriorShare` 0.9, `kSurfCavityGain` 3, `kSurfBaseCavity` 1.5. LitConstants: `Surface3`
(88: x cavity, y interior), `Count` 89. Debug views 12 (sky light), 13 (canopy sky), 14 (specular).

## GPU cost

1920x1080, render scale 1, 4x MSAA, RTX 4060 Ti, two timing passes of three interleaved rounds
(lane off / on in the same game), the triGpuTimes average of the 90 frames before each shot. A
watcher logged every other lane's PoseidonGame every 5 s; only shots with no other game in the 30 s
before them count (`scripts/clean_timing.py`). Every view had 6 clean shots a mode this time
(`timing.md`). Medians, ms:

| view | GPU frame off -> on | delta | world | ao stage | composite | CPU drw:obj |
|---|---|---|---|---|---|---|
| lowsun_fp | 8.32 -> 9.50 | +1.18 | 6.52 -> 7.51 | 0.13 -> 0.27 | 0.04 -> 0.10 | 5.59 -> 6.54 |
| sinai_lowsun_fp | 6.40 -> 7.01 | +0.61 | 4.28 -> 4.74 | 0.17 -> 0.31 | 0.04 -> 0.09 | 3.90 -> 4.56 |
| noon_fields | 8.84 -> 10.01 | +1.17 | 7.73 -> 8.64 | 0.11 -> 0.24 | 0.03 -> 0.10 | 5.62 -> 6.44 |
| forest_edge_golden | 6.33 -> 7.42 | +1.09 | 4.95 -> 5.72 | 0.13 -> 0.32 | 0.04 -> 0.10 | 5.18 -> 5.85 |
| town_night | 10.91 -> 12.05 | +1.14 | 9.59 -> 10.48 | 0.15 -> 0.35 | 0.04 -> 0.08 | 8.39 -> 9.32 |
| jeep_close_noon | 6.87 -> 8.07 | +1.20 | 5.80 -> 6.74 | 0.15 -> 0.31 | 0.03 -> 0.11 | 5.53 -> 6.44 |
| town_noon | 6.96 -> 8.18 | +1.22 | 5.70 -> 6.59 | 0.15 -> 0.35 | 0.03 -> 0.10 | 5.67 -> 6.57 |
| forest_morning | 6.32 -> 7.42 | +1.10 | 4.77 -> 5.53 | 0.14 -> 0.34 | 0.06 -> 0.12 | 4.96 -> 5.64 |

So +0.6 to +1.2 ms a frame on the 4060 Ti at 1080p (round 1: +0.3 to +1.0); on a 3060 expect about
1.2 to 1.5 times that. The split (`cpu1`, `timing_split.md`, lowsun_fp): the surface target and the
lane's per-draw state +0.28 ms of world (6.41 to 6.69), the specular, relief, cavity and canopy
terms +0.67 (to 7.36), the occlusion's own world share +0.08, the ao stage +0.14, the composite
+0.06. The world-pass terms are the bulk; the cavity's extra texture reads (a band of two reads on
objects and the land's grain, two fixed-scale reads of the land's square texture) are new this round.
CPU: see point 9 (it tracks the GPU; the per-draw work is 0.1 to 0.2 ms).

## Tried and dropped this round

- The footprint-relative mean for the land's square texture (`u2`): no effect up close (the
  texture is magnified there).
- Fixed mips 0.5 and 3.5 for it (`u3` to `u9`): right up close, but they alias in the distance and
  shimmered under the TAA jitter; now the pixel's footprint sets the mips beyond those.
- The cavity squared on the sky term with a 0.45 floor (`u4`): black stains in shadowed ground at a
  low sun (lowsun_fp, sinai_lowsun_fp, forest_morning). The sky now loses at most 30%.
- Square-texture cavity 2.5 with `surfCavity` 2 (`u3b`): dark, wet-looking blotches on the near
  noon ground; 1.5 kept.
- `surfCavity` 2.4 (`u7`) and 2.6 (`n2`): noon_fields std ratio 1.43 and 1.46 against 1.41, little
  visible gain, more grain.
- The cavity from the fragment's own texel (1.25 pixels' footprint): the finest grain shimmered
  (lowsun_fp bottom third 0.71 in `n1`, 0.57 with the band in `n2`).
- The shade's fade from 2 to 12 degrees of sun (`u4`): too dark low-sun shadows with the cavity;
  4 to 25 (`u5`) lost most of the low-sun change; 3 to 18 kept.
- Cutting the ground bounce in the shade as well (`u1` to `u4`): darker hull sides, but paint
  saturation fell below lane off (0.138 to 0.143).

## Problems and open points

- noon_fields' ground std ratio is 1.41, under the 1.5 asked; sinai_desert_noon 1.50 and
  jeep_close_noon 1.57 reach it.
- Static low-sun change between rounds is still about 1.7 times the lane-off frame's in lowsun_fp's
  bottom third (0.51 to 0.60 against 0.31 to 0.35); it is the relief and cavity grain, not the
  occlusion (point 7).
- The hull's shaded side against its lit top stays at 0.41 to 0.56 in display luma (the ground's
  cast shadow is at 0.26): the hull keeps the warm ground bounce on purpose.
- GPU cost rose to +0.6 to +1.2 ms (from +0.3 to +1.0), mostly the world pass's texture reads.
- town_noon's upper storey still gains 3 levels of blue; source not found.
- Eaves and window recesses: the buildings have little geometry there, so the occlusion reads
  mainly as seams; painted windows get only the cavity.
- The B8 haze is applied before the composite, so the occlusion darkens a little haze on near
  surfaces (unchanged from round 1).
- The occlusion's strength code rides in the surface target's alpha; pre-lit smoke over a surface
  overwrites it with its own alpha (its share is zero there anyway).
- Two carriers of the class exist: B7's `litMatClass` (block slot, only while wet) and the lane's
  `surfClass` (program uniform, while `surfaces` is on).
- A blended draw is recognised by its texture's alpha class, not by B6's cover flag (stencil bit 5).
- The pan run's lowsun_fp pair had unequal pan shifts between modes, so it does not measure the
  lane's motion there.

## Gate

Build: no warnings in the lane's files (the full rebuilds after the rebases printed 5 warnings, all
in `Network/NetworkServerAuth.hpp` and `Network/NetworkImplClient.hpp`, which nothing here
touches). clang-format clean on the changed lines (`clang-format-diff.py`, applied). Unit tags
`[SurfaceLight],[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[ssr],[wet],[Canopy],
[ShadowSampler],[Atmosphere],[NightLights],[AirMedium],[triSetFixedStep],[triPinCloudDrift]` on the
fec1075 build: 265 cases, 942,121 assertions pass (B4's `[CombatLights]` are among them: 24 cases).
New this round: the shade factor's fades and frame constants, the interior Ao plan, the
accumulation constants; updated: knob count 36, landed 27/27, post shader count 28, debug views 12
to 14 after B4's 11, the slot layout (B7's `MatClass` at 58, `Surface3` at 88, `Count` 89).

## Files here

- `lane.patch`, `NOTES.md`, `contact_sheet.png` (Original | Overhaul, all ten views, 480 px),
  `sheet_3way.png` (Original | lane off | Overhaul), `sheet_debug.png` (Overhaul, occlusion view,
  specular view), `pixel_stats.md` (metrics3.py on `u10`), `timing.md` (`t11`), `timing_split.md`
  (`cpu1`), `temporal.md` (`t11`), `temporal_isolation.md` (`u10`), `pan_stats.md` (`pan6`).
- `critic_round2/`: z_windshield, z_jeep_tyre_curtain, z_jeep_shade, z_noon_ground,
  z_town_facade, z_tank_interior, fm_left, fm_right, fm_top_edge.
- `scripts/`: round 1's plus `metrics3.py` (this round's checks), `crops3.py` (labelled crops by
  exact name), `temporal_modes.py` (round-to-round change per mode). `tests/`: the capture tests as
  run. `r2/`: the diffs the rebases started from.
- Round 1 files kept for reference: `critic_round1/`, `critic_art_r1/`, `critic_realism_r1/`.
