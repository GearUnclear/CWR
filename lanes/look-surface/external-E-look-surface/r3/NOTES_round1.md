# Look lane "surface": surfaces and ambient light (critic round 1)

Scratch copy `D:\Arma_CWA_worktrees\look-surface`, now detached at **06c7e4d** (graphics-remaster with
B8 air pass and B3 night). Nothing committed. The change is `lane.patch` (`git diff --binary` with
the new files `git add -N`; see "Files here" for its size and the apply check).

How the rebase was done (no commit, no branch): the round-0 diff was saved
(`r2/pre_rebase.patch`, a copy on E:), the scratch copy's detached HEAD was moved to 06c7e4d with
`git reset --hard 06c7e4d` (untracked build junctions and capture tests untouched), the diff was
re-applied with `git apply --3way` and the 12 conflicted files were merged by hand: B8 keeps the haze
at slot 78 and the lane's slots follow it (79 to 87, `Count` 88); B8's "Air scattering" debug view
stays 8, the lane's views move to 9, 10, 11; B3's lamp gleam (`g_physicalSpec`) is added in the
lane's branch too; the lane reads the sky's own sun (`litSkySun`) for its sky frame, because B3 moves
the moon into `litSunDir` under the moon key. The GGX lobe and the relief keep `litSunDir` (the key
light). Post shader count, knob count and landed count in the tests add up both lanes.

Build `build/look-surface-rwdi` and `dist/look-surface-rwdi` are junctions to
`E:\look-lanes\look-surface\...` (D: was full in round 0). Captures in
`E:\look-lanes\look-surface\captures\` (`s1`..`s5` hero rounds, `v1`, `v2` variants, `t3`..`t5`
timing). Junction `packages\RemasterUD-look-surface`. Games only through run-captures.ps1.

## The critics' points, one by one

Pixel figures are from my metrics script (`scripts/metrics.py`) on the 960x640 hero frames, lane off
(`remastered,surfaces=0`) against lane on, unless noted. HSV saturation is (max-min)/max per pixel.
My boxes are not the critics' boxes, so the absolute values differ from theirs; the direction and
the before/after on my boxes are what I report. "Round 0" is the build the critics judged
(`E:\...\captures\final2`), "now" is `s5` (final build) unless noted.

### 1. Foliage specular (blocking: grey-blue veil, planar sheets on crowns)

Leaves now get no specular at all: no sky reflection, no sun lobe, and the diffuse keeps all its
energy (`surfaceSpecular` returns at once for foliage). The specular debug view (debug 11) shows
the crowns black in forest_morning, forest_edge_golden and town_noon (`sheet_debug.png`). Leaves also
take the rig's own sky light, not the sky table's (the table's bluer shade light is off by default
anyway, point 6).

Crown saturation, foliage pixels of the lane-off frame in fixed boxes:

| view, box | lane off | round 0 on | now on |
|---|---|---|---|
| forest_morning, big tree right (560,0)-(960,330) | 0.330 | (critics: 0.40 to 0.25 on their box) | 0.341 |
| forest_morning, broadleaf left (100,80)-(360,300) | 0.357 | | 0.357 |
| forest_edge_golden, crowns (0,0)-(960,330) | 0.465 | (critics: 134 to 98 of 255) | 0.470 |
| town_noon, birch (250,0)-(620,260) | 0.379 | (critics: 0.39 to 0.26) | 0.372 |

Blue does not rise: forest_morning big tree RGB (74.2, 73.6, 51.6) off, (66.6, 65.8, 45.3) on (the
occlusion darkens it; the hue is kept). s1 (first build of this round, before the occlusion fix of
point 6) had 0.312 there, so most of the remaining loss was the occlusion bias, not the specular.

### 2. Foliage occlusion (blocking: crowns near black, golden-hour crowns lose their warmth)

- A leaf hands the post pass only `kSurfLeafShare` = 0.4 of its occludable share: whatever the search
  finds among a crown's cards, at most 40% of a leaf's sky light goes (visibility floor 0.6).
- Leaves never take the contact shadows (their sun share is written as 0); the sun-lit share of a
  crown is never darkened by the lane.
- The canopy heightfield takes at most 0.35 of a leaf's sky (`kSurfCanopyLeafMax`; it was 0.92).
- The occlusion debug (debug 4) still shows crowns as grey with dark flecks, but no longer the near
  black mass of round 0, and the frame keeps it to the 40%.
- forest_edge_golden crowns: saturation 0.465 off, 0.470 on; RGB (56.6, 56.7, 32.3) off, (52.1,
  51.9, 29.1) on (darker by about 8%, same hue).

### 3. Occlusion bleeding past crown silhouettes

- Thickness: an occluder in front of the receiver by more than 0.2 of the search radius (view depth)
  starts to fade and is gone at 0.7 of it, so a crown or trunk well in front of the grass or a wall
  behind it no longer darkens it (XeGTAO-style thin-object heuristic).
- The 4-tap depth-aware upsample's tolerance is about 1% of the depth (`0.008 z + 0.04` m, was
  `0.03 z + 0.1`), its taps clamped to the target.
- Crops of forest_morning's conifers (`critic_round1/fm_right.png`, s1) show no translucent smudge on
  the grass behind the crowns.

### 4. Frame-edge border

GTAO samples are clamped to the 3D viewport (the edge's own depth stands in past it) and the
contact-shadow march stops at the viewport's edge. forest_morning, x 780-900, on minus off:

| rows | round 0 (critics) | s1 (off-screen counted as open) | now |
|---|---|---|---|
| 0, 1, 2, 3 | +28, +13 (brighter) | -0.7, -1.8, -4.9, -6.9 | -1.4, -1.1, -1.1, -1.3 |
| 8-19 (inside) | | -8.2 | -1.7 |

The round-0 bright band was the leaf specular sheet (point 1); counting off-screen samples as open
(first try) left a lighter rim; clamping removes it.

### 5. Vehicle paint (blocking: olive goes grey at noon)

Three causes, measured one by one with parts switched off (s3 `ab2` run):
- the sky table's sky light: with it, the jeep's body box was (44.5, 46.3, 46.2), blue above red;
  with it off (44.5 / 43.7, 44.1, 42.2). It is now off by default (point 6);
- the paint's broad sheen: rough 0.6 plus the "dark texels read rougher" term gave olive drab a
  roughness of about 0.75, whose lobe spread into an even grey sheen; paint now has roughness 0.5,
  F0 0.02, and is exempt from the dark-texel term, so its sun highlight sits where the half vector
  meets the hull;
- the sky reflection on matt dielectrics fades to 0.15 from roughness 0.5 (was 0.75), returns at
  grazing angles only up to 0.45 (was to 1), and the diffuse gives up only the energy the reflection
  actually shows (was all of fEnv).

Body box (400,240)-(620,400) of jeep_close_noon and (530,350)-(720,450) of noon_fields, pixels under
110 luma:

| view | lane off S | round 0 (critics, their box) | now S | now RGB |
|---|---|---|---|---|
| jeep_close_noon | 0.147 | 0.07, (40,42,40) | 0.112 | (43.8, 44.2, 42.3), red above blue |
| noon_fields | 0.146 | 0.08 | 0.114 | (42.8, 43.2, 40.5) |

Still below lane off: the occlusion darkens the body and a small sheen remains (with the specular off
it is 0.157). The critics' bar (at least 0.11, red at least blue) is met on my box.

### 6. Noon open terrain and the sky-light harmonics

The noon ground darkening had two causes, both fixed:
- **GTAO read open flat ground as about 10% occluded.** The cosine-weighted slice integral is 1 only
  averaged over all slice directions; with 2 slices clamped to 1 per pixel, ground seen at a grazing
  angle came out at about 0.9, 0.81 after the power. The visibility is now the ratio of the slices'
  integral to the same slices with nothing in them, so open ground is exactly 1 (AO debug on
  jeep_close_noon's open ground: 214 of 255 before, white now). Snapping every depth read to a texel
  centre (done first, on the theory that sub-texel positions put the samples off the plane) changed
  nothing measurable; it is kept as it is the correct reconstruction.
- **The relief took light from the land under a high sun**: a normalised bumped normal has on average
  a smaller N.L than the flat one. The sun term now moves by the unnormalised gradient's share (zero
  on average).
- `surfSkyLevel` default 0 (the rig's calibrated fill; 0.5 dimmed the sunlit land), and the
  terrain's diffuse no longer gives up energy for a reflection it does not show.

Noon ground box, mean 8-bit luma:

| view | lane off | round 0 | now |
|---|---|---|---|
| sinai_desert_noon (0,450)-(960,640) | 165.2 | (critics: about -15) | 163.6 |
| noon_fields (0,470)-(960,640) | 163.2 | | 161.9 |
| jeep_close_noon (0,520)-(960,640) | 166.5 | | 165.3 |

The remaining 1.2 to 1.6 levels are the occlusion around the soldier and the vehicle in the box.

**The sky-light harmonics are dropped by default** (`surfSkyLight` 0, code and knob kept). The sky
light debug was uniform on slopes because the rig's hemisphere already has a sun-side and an
away-side horizon, and the table's shape added nothing visible on top; what it did add was a brighter,
bluer vertical sky that turned olive hulls blue-grey (point 5). With the knob at 0 the worker never
projects it. I also moved its colour handling into the shader (the rig's own colour toward each face,
`surfSkyTint` of the table's mixed in, default 0.25), so turning it on no longer paints every face in
the zenith's blue.

**Visible noon contribution.** The occlusion is stronger now that it no longer biases open ground:
radius 4 m (was 3), power 3 (was 2) after a variant run (`v1`: radius 3/4/5, power 2/3). At noon the
lane grounds the vehicles and soldiers (a darker core under the hull, darker wheel arches, the ground
at a soldier's boots) and keeps the land's brightness. On the open noon_fields frame that is 1.4% of
pixels changed by more than 8 levels: at noon on open ground the lane's share is small, and I did not
find a realistic term that would make it larger. Raising the relief (variant `v2`: 1.5, 2.5, 3.5)
changed the noon ground by under 1 level under its N.L limit.

### 7. Night

I dropped the night claim. At town_night the lane changes little (lane on vs off, 12% of pixels by
more than 8 levels, most of it fire and smoke that move between shots; a crop of the lamp and the
fire shows a slightly darker tree foot and wall feet). Changes:
- the local lights now count fully in the occlusion share (was half): lamps and fires cast no
  shadows, so this is the only grounding they get; with B3's physical lamps (N.L-shaded, Oren-Nayar)
  the undersides are already dark, and the visible gain is small;
- new gate: once the sun is below the sky table's night fade (-9 degrees) and no lamp is lit, the
  lane holds off entirely (`surfaces=1:off:night without lamps`), so a moonlit field costs nothing.

town_night cost: see "GPU cost".

### 8. Low-sun relief (blocking: blocky "hammered metal", unstable over time)

The relief no longer comes from screen derivatives of the shaded albedo. It reads the height from
the texture itself: the land from its detail texture's alpha (times the square's luminance, as the
albedo takes it), objects from their texture's luminance, at the fragment and one pixel's footprint
away in x and y (`surfaceReliefDelta`: three `textureGrad` reads through the same footprint widened
by 1.25, a third of a mip), so the gradient is a smooth function of position: no 2x2 quad steps, and
the TAA jitter moves it continuously. The sun term moves by the gradient's unnormalised share
(point 6) and by at most `kSurfReliefSunLimit` = 0.35 of N.L, so a grazing sun gets contrast but no
black pockets. The land's relief fades out by 80 m (was 120), objects' by 60 m; leaves take none (and
skip the reads).

Foreground luma std, sunlit ground box (hero frames):

| view | lane off | round 0 (critics) | now | ratio now |
|---|---|---|---|---|
| lowsun_fp (0,470)-(960,640) | 15.6 | 9.6 to 19.6 (2.0x, their box) | 17.5 | 1.12 |
| forest_edge_golden (0,500)-(960,640) | 13.3 | 8.6 to 15.7 (1.8x) | 13.7 | 1.03 |
| sinai_lowsun_fp (0,470)-(960,640) | 12.3 | | 12.7 | 1.03 |

`critic_round1/relief_hd.png`: 1080p crop of lowsun_fp's sunlit ground at 3x, lane off, lane on
round 1, lane on round 2: a soft gravel grain, the same in both rounds.

Static camera, same game, round to round (1080p timing frames, `scripts/temporal.py`), mean
|d| of 8-bit luma, bottom third of the frame:

| view | lane off | round 0 on (critics) | now on |
|---|---|---|---|
| lowsun_fp | 0.32 | 1.48 | 0.53-0.56 |
| forest_edge_golden | 0.42-0.47 | 1.47 | 0.47-0.58 |
| sinai_lowsun_fp | 0.34-0.37 | 0.83 | 0.35-0.40 |
| forest_morning | 0.41-0.45 | | 0.63-0.67 |

(`temporal.md`, runs t4 to t7, rounds 1-2 and 2-3.) Most of what is left is the occlusion's turned
pattern, not the relief: in the cost-split run, lowsun_fp with the occlusion off fell to the lane-off
level and with the relief off it did not.

### 9. Stability in motion

`scripts/gen_pan.py` shoots two back-to-back frames while the cutscene camera pans (6 degrees over 6
s, the sim on a fixed 1/60 s step), per mode; `scripts/pan_stats.py` aligns the second frame by the
pan's measured shift (phase correlation, sub-pixel) and reports the mean |d| of 8-bit luma. The
measured shifts are 0.1 to 0.35 px, so the two frames are about one frame apart. Final build (s5,
`pan_stats.md`, x8 difference images in `critic_round1/pan_diff_x8/`):

| view | lane off: whole / bottom third | lane on: whole / bottom third | raw occlusion buffer (debug 4) |
|---|---|---|---|
| lowsun_fp | 0.55 / 0.65 | 0.68 / 1.02 | 0.92 / 0.54 |
| forest_morning | 1.19 / 1.32 | 1.27 / 1.59 | 1.34 / 1.39 |
| town_noon | 1.19 / 1.00 | 1.15 / 0.93 | 1.22 / 0.57 |

The raw occlusion view moved 2.2 to 3.4 levels frame to frame with 2 slices and the 1-slice near tier
(s4) and 0.5 to 1.4 with 3 slices (s5). In the final frame the lane adds 0.1 to 0.4 levels on the
near ground; the x8 difference images show a fine grain on sunlit ground (the relief's added detail,
which the sub-pixel alignment cannot cancel exactly) and no block or crawl pattern.

### 10. GPU cost, cheaper tiers

See "GPU cost" for the re-measured numbers. Tiers and savings added this round:
- GTAO takes 3 slices, near pixels whose search spans more than 60 half-resolution pixels one fewer
  (the critics' suggestion was 1 slice there; with 1 slice the near grounding, the most visible part,
  was the noisiest, and the whole pass costs 0.2 to 0.35 ms, so the third slice is spent on noise);
- the canopy at two mips (6 m and 25 m), the 100 m level dropped;
- the night gate (point 7);
- the per-draw class upload (a 16-byte UBO update) is skipped for the land, grass, sea and leaves,
  which never read it.

### 11. Rebase onto 06c7e4d and the night views

Done (top of this file). The night views were re-shot on the rebased build (s1 to s5).

## What the player sees now

`contact_sheet.png` (Original | Overhaul at 480 px a panel, every hero view and the lane's own
views), `sheet_3way.png` (Original | Remastered with the lane off | Overhaul), `sheet_debug.png`
(Overhaul, occlusion times contact shadows, specular alone). At thumbnail size the Original against
the Overhaul is obvious in every day view, mostly from the base remaster; the lane's own share shows
as grounded vehicles, soldiers, tents and tree feet (a darker core under the hull and in the wheel
arches, contact shadows), darker wood interiors and crowns that keep their colour, and a soft grain
on low-sun ground. It no longer greys crowns or paint and no longer darkens open ground.

Lane on vs lane off, share of pixels moved by more than 8 levels at 480 px (s5, `pixel_stats.md`):
lowsun_fp 9.1%, sinai_lowsun_fp 5.1%, noon_fields 1.4%, forest_edge_golden 4.1%, town_night 10.7%
(mostly moving fire and smoke), tank_gunner_interior 4.8%, jeep_close_noon 4.5%, town_noon 7.1%,
forest_morning 10.4%, sinai_desert_noon 6.4%. Original vs Overhaul: 9.4% (noon_fields) to 84.7%
(town_night). Round 0 had larger lane shares (13% to 41%), most of it the leaf veil, the noon
darkening and the blocky relief that this round removed.

## What is built (state after this round)

All behind the feature switch `surfaces` (FeatureSet, default 1; registry row "Surfaces and ambient
light", track `surface`, step `look`, landed, GPU section `ao`). Gate: on for every lit surface,
off once the sky is dark and no lamp is lit; the post occlusion holds off where the Ao stage does
(vehicle interior, optics). `baseline` turns it off.

1. **Surface target (MRT)**: a second multisampled RGBA8 colour attachment written by the remastered
   lit programs during the world pass (`EngineGL33_Surface.cpp`): r the share of the pixel's light the
   occlusion may darken (after fog and the haze's fog edge; leaves 0.4 of theirs), g the square root
   of the albedo's luminance, b the sun's share (0 on leaves), a the alpha. Unchanged otherwise from
   round 0.
2. **Horizon occlusion (GTAO, `psPostGtao`)**: half resolution, 3 slices (2 for near pixels) x 2
   sides x 8 steps, turned per pixel and per TAA frame; depth reads snapped to texel centres; samples
   clamped to the viewport; weights for distance, thickness and height (an occluder below the
   receiver counts at 0.3: the ground bounces the light it hides, so a hull's side is not darkened by
   the ground it stands on); visibility normalised by the open slice integral. Radius 4 m, capped at
   25% of the frame height, faded out from 150 to 300 m. Composite: depth-aware 4-tap upsample, power
   `surfAo` (3), Jimenez multi-bounce, applied to the share only.
3. **Contact shadows** (composite): as round 0, stopping at the 3D viewport's edge, not on leaves.
4. **Canopy occlusion**: the B9 canopy heightfield at two mips, leaves at most 0.35.
5. **Sky light**: the rig's hemisphere by default; the sky table's harmonics behind `surfSkyLight`.
6. **Specular**: GGX sun lobe and split-sum sky reflection per surface class; none on leaves; paint
   0.5 / 0.02; matt dielectrics keep 0.15 of the sky reflection from roughness 0.5, at most 0.45 at
   grazing, the land none at grazing; the diffuse gives up only what the reflection shows.
7. **Relief**: from the texture (point 8).
8. **Vehicle interior**: as round 0.

Debug views (`debug`): 4 occlusion times contact shadows; 9 the sky light a white surface takes; 10
canopy sky visibility; 11 specular alone.

## Knobs (A1 registry, sub-knobs of `surfaces`)

| key | default | what |
|---|---|---|
| `surfaces` | 1 | master switch |
| `surfAo` | 3.0 (was 2.0; range now 0-4) | occlusion strength (a power on the GTAO visibility); 0 = no surface target, the stock occlusion |
| `surfAoRadius` | 4.0 (was 3.0) | metres the horizon search reaches |
| `surfCanopyAo` | 1.5 | sky lost under crowns, woods, building blocks |
| `surfSkyLight` | 0.0 (was 1.0) | share of the sky light from the sky table's harmonics |
| `surfSkyTint` | 0.25 (was 0.35) | share of the table's colour in that light (the rest the rig's own toward each face) |
| `surfSkyLevel` | 0.0 (was 0.5) | 0 = the rig's shade fill, 1 = the sky the air gives against the sun |
| `surfSpec` | 1.0 | specular on every surface but leaves |
| `surfContact` | 1.0 | screen-space contact shadows |
| `surfRelief` | 1.5 | relief from the textures (multiplier of each surface's height) |

Constants (not knobs): `kSurfLeafShare` 0.4, `kSurfCanopyLeafMax` 0.35, `kSurfReliefSunLimit` 0.35
(GLSL); `kGtaoSlices` 3, `kGtaoOneSliceRadiusPx` 60, `kGtaoBelowWeight` 0.3 (SurfaceLight.hpp).

## GPU cost

1920x1080, render scale 1, 4x MSAA, RTX 4060 Ti, 3 interleaved rounds (lane off / on, same game),
the triGpuTimes average of the 90 frames before each shot. Other lanes' games (look-light, look-fog,
gr-B7) were on the GPU for most of the session, so a watcher logged every other PoseidonGame every 5
s and only shots with no other game in the 30 s before them count (`scripts/clean_timing.py`,
`timing.md`). Medians of clean shots (ms):

| view | clean shots off / on | GPU frame off -> on | delta | world | ao stage | composite | CPU drw:obj |
|---|---|---|---|---|---|---|---|
| lowsun_fp | 3 / 3 | 7.90 -> 8.77 | +0.87 | +0.64 | 0.14 -> 0.26 | +0.06 | 5.25 -> 5.96 |
| noon_fields | 3 / 3 | 8.55 -> 9.21 | +0.66 | +0.49 | 0.11 -> 0.23 | +0.06 | 5.18 -> 5.79 |
| forest_edge_golden | 6 / 6 | 6.33 -> 6.59 | +0.26 | +0.11 | 0.14 -> 0.31 | +0.06 | 4.95 -> 5.14 |
| forest_morning | 3 / 2 | 6.33 -> 7.13 | +0.80 | +0.63 | 0.15 -> 0.33 | +0.05 | 4.60 -> 5.35 |
| town_night | 3 / 3 | 9.48 -> 10.50 | +1.02 | +0.58 | 0.15 -> 0.33 | +0.03 | 7.36 -> 7.93 |
| sinai_lowsun_fp | 1 / 1 | 6.41 -> 6.84 | +0.43 | +0.36 | 0.17 -> 0.30 | +0.05 | 4.03 -> 4.26 |
| jeep_close_noon | 0 (all loaded) | min frame 6.04 -> 6.78 | about +0.7 | | 0.15 -> 0.68 (loaded) | | |
| town_noon | 1 / 0 | min frame 6.14 -> 7.03 (loaded) | about +0.9 | | | | |

So about +0.3 to +1.0 ms a frame on the 4060 Ti at 1080p: the GTAO pass +0.1 to +0.2 (3 slices,
half resolution), the composite +0.05, the rest the world pass. One clean cost-split round
(t3, lowsun_fp, build before the third slice): the specular about 0.24 ms, the relief's texture reads
about 0.34, the canopy about 0.13 of the world's +0.78; with every part at zero ("bare") the world
cost was gone (6.19 against 6.17), so the surface target itself costs nothing measurable there. On a
3060 expect roughly 1.2 to 1.5 times these.

**town_night**: the round-0 outlier (lane on 12.2 to 17.4 ms against 8.0 to 8.3, growing round by
round) does not reproduce: on clean shots it is +1.0 ms and flat across rounds (10.50 median; 3
rounds). Round 0's t2 run had other games on the GPU for 63 of 65 samples, and the GPU timers count
time slices given to other processes; the lane-off rounds of the same run happened to be the less
loaded ones.

**CPU**: the object draw's CPU time rises 0.2 to 0.75 ms (drw:obj), from the per-draw class lookup
and its 16-byte UBO update (the build that skips it for the land, grass, sea and leaves is in t5-t7:
town_night +0.57, forest_edge_golden +0.19).

Noon open fields are the costliest for the least effect (+0.66 ms, 1.4% of pixels); a noon gate
that drops the relief and the specular on the land would save part of it, not built.

## Tried and dropped this round

- **Off-screen GTAO samples counted as open** (s1): a lighter rim along the top edge (rows 0-3 at -0.7
  to -6.9 against -8.2 inside). Clamped samples instead.
- **One slice for near pixels**: the near grounding is the most visible occlusion and was the
  noisiest; one slice fewer of three instead.
- **The sky table's harmonics with the rig's colour** (s2): still bluer hull sides (its vertical sky
  is brighter than the rig's); the term is off by default.
- **Relief 2.5 and 3.5** (v2): no gain under the N.L limit (lowsun_fp std ratio 1.12 at all three).
- **Occlusion radius 5 m** (v1): the same as 4 m at these views.
- **Depth snapping as the fix for the flat-ground bias** (s2): no change; the cause was the per-pixel
  clamp of a two-slice estimate.

## Problems and open points

- **The lane's share at thumbnail size is modest.** It is visible on vehicles, soldiers, tents, tree
  feet and wood interiors, and as low-sun ground grain at full size; at noon on open ground and at
  night it is small by nature (point 6, point 7).
- **CPU**: the per-draw class (a hash lookup and, when the class changes, a 16-byte UBO update) adds
  CPU time in the object draw; see "GPU cost". A cheaper carrier would be a per-program uniform, which
  would leave slot 58 to the materials step.
- **The B8 haze is applied before the composite**, so the occlusion darkens the haze over near
  surfaces a little (the occlusion is faded out by 150 to 300 m, where the haze is strong).
- **Occlusion still slightly noisier than lane off on a static camera** at a low sun (see
  "Stability").
- **`MatClass` (58)** is also the materials step's planned slot (B7). The encoding is the materials
  prototype's; B7 should reuse the classifier here.
- Memory: one more RGBA8 4x MSAA renderbuffer (about 33 MB at 1080p) and an RGBA8 resolve.

## Gate

No compiler warnings in the build logs of PoseidonGame and PoseidonTests since the rebase (the first
rebased build warned of NUL characters in FeatureSwitches.cpp, from a conflict fix of mine; fixed).
clang-format clean on the changed lines (`clang-format-diff.py`, applied). Unit tags
`[SurfaceLight],[remaster],[FeatureSwitches],[FeatureGates],[Canopy],[PostStages],[triSetFixedStep],
[triPinCloudDrift],[ShadowSampler],[Atmosphere],[NightLights],[AirMedium]`: 210 cases, 716,358
assertions pass. New this round: the gate test (dark night without lamps), the GLSL slice and step
counts against `kGtaoSlices`/`kGtaoSteps`, the tint in surface2.z; updated: layout (B8's haze at 78,
the lane at 79 to 87), debug view range (8 air, 9 to 11 the lane), knob count 26, landed 22/27, post
shader count 20.

## Files here

- `lane.patch`: 31 files, 2,054 lines in, 69 out, against 06c7e4d (`git apply --cached --check`
  passes on a fresh index of 06c7e4d).
- `NOTES.md`, `contact_sheet.png`, `sheet_3way.png`, `sheet_debug.png` (all from s5, the final
  build), `pixel_stats.md` (metrics.py on s5), `timing.md` (clean-shot medians), `temporal.md`,
  `pan_stats.md`, `critic_round1/` (crops: fm_right, fm_left, fm_top_edge, feg_crowns,
  jeep_close_noon (off, on, specular), relief_hd, town_night; pan_diff_x8/). `r2/pre_rebase.patch` is
  the round-0 diff the rebase started from.
- `scripts/`: `gen_hero.py` (hero, variant and timing tests), `gen_pan.py` (pan pairs), `metrics.py`
  (the critic checks), `temporal.py` (static round to round), `pan_stats.py` (pan pairs), `clean_shots.py`
  and `clean_timing.py` (timing from shots with no other game on the GPU), `crops.py`, `sheet.py`,
  `timing.py`.
- `tests/`: the generated capture tests as run (untracked in the scratch copy).
