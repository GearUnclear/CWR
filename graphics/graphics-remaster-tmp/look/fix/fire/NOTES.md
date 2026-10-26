# Fire fixer: fire, shade and golden hour

Scratch copy `D:\Arma_CWA_worktrees\look-fix-fire` (fec1075 + `fix\base.patch`, base tree
07d0ae092969bc26cff732fbdbae2c5c1397db20). Nothing committed, no branch. Build
`arma_CWR\build\look-fix-fire-rwdi`, game `arma_CWR\dist\look-fix-fire-rwdi`, junction
`RemasterUD-fix-fire`. Patch: `fix.patch` (15 files, +352 / -47, only my delta over the base; it applies
with `git apply --cached --check` on a fresh index of the base tree).

Images (before = `director\final\<view>__1080__overhaul.png`, after = my build, same view, 1920x1080):
- `a_fire_town_night.png`: town_night before | after, a 2x crop of the fire, then town_fog_night and
  town_night_rain.
- `b_shade_pine_rays.png`: pine_rays with a 2x crop of the shaded ground, then woods_pine2_evening,
  lowsun_fp, sinai_lowsun_fp.
- `c_golden_forest_edge.png`: forest_edge_golden with a 2x crop of the sky, then coastline_sunset,
  woods_pine2_evening and sunrise_ridge (side-effect check).

Shots: `shots_3\` (all day views and sinai, final daylight code), `shots_4\` (the three night views,
final build). The fire constants changed after shots_3; they do nothing in those day views (no fire).
Probe runs: `probe1\` (look-director game, knobs), `probe_gold1\`, `probe_p3\`, `probe_film\`
(shader overrides). Numbers below are my numpy means over the named boxes (1920x1080 pixels).

## (a) town_night fire: was still wrong, fixed

What I saw: no flame at all, a white-grey steam column over the fire, and the fire's stones a flat
cream slab. Fire box 1150..1500 x 350..820: 1450 pixels with every channel at 200 or more.

Cause, proven by probes on the look-director game (`probe1\`):
- `combatSmoke=0`: plume foot (134, 113, 90), saturation 0.33, against (179, 166, 151), 0.16 with it on.
  The combat light on the smoke is what made it white. In the code
  (`CombatParticleChannel`, used per channel by `Object.cpp CombatLitParticle`) each channel was
  soft-saturated on its own toward the same ceiling (4), and the 8-bit vertex colour then clipped
  each channel at 1: strong orange light came out white. The round-2 smoke tint tinted the puff's
  ambient light (tiny at night), not the fire's light, so it did nothing.
- `combatFire=0`: the stones go dark (96, 82, 58). Their cream comes from the combat fire light on
  surfaces: a 0.45 m point ball at the foot of the stones, several stops past white, rolled to cream
  by the curve.
- `camAdditive=2`: the flame sprite only grows a little. The flame was never the additive sprite. In
  the classic the flame is the foot of the smoke column lit past white by the classic fire light,
  which the combat light switched off; the combat light on smoke only scatters, so the overhaul had
  no flame body at all.

Change (engine, fire only):
- `CombatParticleLight` (new, CombatLights): the fire's light on a puff keeps its hue. The soft
  saturation is taken on the peak channel and scales all three alike (ceiling `kParticleLightCap`
  12); the tint now deepens the added light itself (channel share to the power `kSmokeTintPower`).
- `KeepHueUnderOne` + `Engine::SetDecalGlow` (new): `Object::DrawDecal` brings the puff's colour under
  1 by its peak (no per-channel clip), and the light past 1 goes to the backend as the decal's glow;
  `EngineGL33::DrawDecal` puts it in the vertex's specular rgb, which the pre-lit shader path already
  adds (`r0 + vSpecColor`) and blends by coverage. Only under the photoreal camera (stored above
  white); otherwise dropped.
- `CombatFlameEmission` (new): a puff within `kFlameBodyCores` core radii of a fire's light emits the
  flame's colour, linear, falling off squared, times the fire's strength and flicker. This is the
  flame body the classic had. Not scaled by the puff's own colour (it glows, it does not scatter).
- The round-2 night boost of 3x on the smoke is gone (with the hue kept it lit the column flat orange
  4 m up).
- Fire light on surfaces under the camera (`EngineGL33_PostCombat.cpp`): deeper orange at equal
  luminance, and a 1.3 m flame body instead of the 0.45 m point (same power, so from 3 m out the
  pool loses under a quarter stop). The smoke reach still uses the 0.45 m core (a 0.6 m core broke
  the burning wreck's dark column test).

Result (before -> after, `shots_4\...\000_town_night__overhaul.png`):
- pixels with min channel >= 200 in the fire box: 1450 -> 73; none at 250+.
- plume foot (1280..1340 x 620..720): (179, 166, 153) sat 0.16 -> (221, 178, 128) sat 0.43.
- plume 1 to 2 m up (1270..1350 x 480..600): (155, 151, 144) sat 0.08 -> (161, 128, 99) sat 0.38.
- flame core (1290..1330 x 680..750): (200, 179, 147) -> (246, 208, 161), a bright core with orange edges.
- stones (1240..1380 x 740..775): (218, 191, 142), top tenth (255, 243, 221) -> (198, 158, 98), top
  tenth (254, 223, 183); saturation 0.37 -> 0.52. They now read as fire-lit stones with shading.
- lit grass (1100..1250 x 780..880): (72, 59, 36) -> (79, 60, 34), red over green 13 -> 19.
At thumbnail size the fire reads as an orange flame with a warm plume that fades to grey smoke, in
town_night, town_fog_night and town_night_rain.

## (b) shaded grass and soil: already fine, left alone

Ground means (before = round 2 final; mine within a level after the golden change):
- pine_rays ground (30..90% x 70..100%): (51, 48, 35), R-B 15.4, a dark olive (classic (67, 58, 40)).
- woods_pine2_evening shade (bottom 15%): (59, 57, 38), R-B 20.9, dark olive.
- lowsun_fp ground (bottom 20%): (74, 66, 53), R-B 21.6; the Jeep's shadow core (37, 37, 35) neutral.
- sinai_lowsun_fp ground (bottom 20%): (66, 56, 45), R-B 20.5, brown.
No blue-grey anywhere (B is the lowest channel in every box). At 2x the pine_rays shade is olive with
paler grass tips; the mid-ground greyness there is the morning mist (fog 0.1), the veil fixer's area.
I changed nothing for (b).

## (c) golden hour: part wrong, fixed

What I saw: the land was golden (forest_edge_golden sunlit slope (144, 116, 64), saturation 0.55),
but the clear sky and its clouds were teal-grey, (129, 148, 163) and (153, 167, 165) at the horizon,
so at thumbnail the view read like a dull afternoon (the same sky as forest_morning).

Cause: the golden hour's warm balance (`kCameraGoldenWarm` 1.1, 1, 0.76) was a diagonal gain over the
whole frame; on a blue sky it drops blue but not green, so blue turns teal. Probe
(`probe_gold1\`, shader override that undoes it on blue light only): clouds (121, 144, 176), blue; land
identical to the level.

Change (camera white balance): the warm gain moves out of the colour matrix into its own uniform
(`camGolden`, `GoldenWarmGain`) and the composite lays it on light by its warmth
((r - b) / peak, `GoldenWarmShare`): none from -0.7 (a blue sky), all from 0.4 (a sunlit field), a
grey cloud about 0.7. A first ramp of -0.25..0.1 drew a visible edge round the sunrise glow where it
met the blue (`probe_p3\`); the wide ramp is smooth. The warm constant itself is unchanged.

Result (before -> after, `shots_3\`):
- forest_edge_golden clouds (121, 144, 175) (was teal (129, 148, 163)); horizon sky (145, 163, 176)
  (was (153, 167, 165)); sunlit slope and near ground unchanged to the level.
- coastline_sunset sun glow (232, 192, 143) and sea unchanged; upper sky (143, 138, 134), a touch less
  beige (was (145, 138, 128)).
- woods_pine2_evening: sun patches unchanged (131, 110, 69 top tenth); sky a little bluer.
- sunrise_ridge (side effect): upper sky (147, 153, 172) (was (153, 155, 160)), the peach glow
  unchanged, no edge between them.
Golden hour now reads as golden land under a blue sky. I did not push the warmth further: the sunlit
slope is already at saturation 0.55, more would start an orange wash.

## Constants touched (name: old -> new)

- CombatLights.cpp: new `kSmokeTintPower` 2.2 (the tint's power was an inline 1.6 on the wrong colour);
  new `kSmokeTintRate` 0.6 (was inline 0.6); new `kParticleLightCap` 12 (the colour version; the scalar
  `CombatParticleChannel` keeps 4); new `kFlameBodyCores` 3.0, `kFlameEmission` 8.0, `kFlameRgb`
  (1, 0.55, 0.16); new `kFirePower` 7 (the existing inline 7 named). `kFireRgb` and the 0.45 m fire
  core unchanged.
- EngineGL33_Lighting.cpp: night smoke gain under the camera `1 + 2 night` -> 1 (removed);
  `smokeTint` 0.85 x night unchanged.
- EngineGL33_PostCombat.cpp: fire warmth under the camera warmG 0.74 -> 0.6, warmB 0.48 -> 0.3; new
  `kFireSurfaceBody` 1.3 m (was the 0.45 m core) at the same power.
- PhotoCamera.hpp: new `kGoldenShareFrom` -0.7, `kGoldenShareTo` 0.4 (mirrored in the composite);
  `kCameraGoldenWarm` unchanged, now applied by `GoldenWarmGain` in the composite, not in the matrix.
No exposure, curve or haze constant changed. I tried a linear chroma keep in the curve and a film-like
per-channel roll-off past white (`probe_p3\`, `probe_film\`); neither helped the stones, both dropped.

## Tests

- Unit gate tags (`[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],
  [AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera]`): 304 cases,
  953,848 assertions, all pass (glslang compiles every program, including the composite with
  `camGolden`).
- Full suite: 3384 cases, 6 fail, the same known localisation and data cases as round 2 (no
  `packages\Remaster` junction in the scratch copy).
- New: "a fire lights its smoke in its own colour however close, brighter at the foot", "the smoke
  inside a fire's flames glows as the flame", "the golden hour warms the light the low sun shows and
  leaves the blue sky blue". Updated: the DrawDecal source contract (new call forms, hue keep, glow).
- clang-format clean on my lines (`git-clang-format --diff`; its one remaining hunk is base code in
  VolumetricFog.cpp).

## AI sight and fog

`Target.cpp` lines 636 to 645 still multiply AI visibility by `FogSightTransmittance` while
`FogSightActive()`; I did not touch it or the fog.

## Still open

- The flame core is a warm cream-white with orange edges, not a yellow one: the hue-keeping curve
  whitens bright orange toward cream. Fine at thumbnail, a film-like yellow roll-off would be nicer.
- The upper plume is warm peach partly from the street lamp next to the fire, not only the fire.
- The stones' top faces still touch 254 red; they read as fire-lit, not as a slab.
- The C++ mirror `ApplyToneCurveHueKeeping` still lacks round 2's chroma keep (unchanged here).
- Not checked: the firefight views (burning wreck, explosions) of `abel_lighting`; the fire changes
  touch the wreck's embers too (flame body glow at its foot, deeper orange on surfaces).
