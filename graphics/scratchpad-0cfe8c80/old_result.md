**Result (2026-10-18, uncommitted change on 807ce4b in the scratch copy `gr-B8`, a parallel lane;
evidence in `tmp/integration/B8/` of the graphics-remaster worktree, gitignored; light gate).**

- Built: `Graphics/Remaster/AirMedium.{hpp,cpp}` (pure: the haze's, the valley mist's and the beams'
  constants; the weather and time rules `MorningWeight`, `BeamMistWeight`, `ComputeAirMedium`; the
  closed forms the shaders mirror, `ExpOpticalLength` and `LayerOpticalLength`; the valley floor and
  its depth, `LocalValleyAt` and `ValleyDepthWeight`; the classic fog's moved edge, `FogEdgeStart`
  and `FogEdgeScale`; `AirOpticalDepth` for a line of sight from any start distance, the hook for AI
  sight); the `Air` post stage (`EngineGL33_PostAir.cpp`, its GLSL in the GL-free
  `EngineGL33_PostAirShaders.cpp`): a half-resolution march on the linear depth into two targets
  (sun beams, 24 steps over the first 250 m with the shadow map, the horizon map and the cloud
  field; headlight cones, up to 4 spotlights within 150 m at night, 24 steps through each cone; the
  haze's sun visibility, 12 samples beyond 150 m; the valley mist's top and density where the ray
  ends), a depth-aware blur across and down, the beams' depth-matched local mean, and a
  full-resolution apply (haze and mist in closed form, `C' = C T + S`, toward the sky table's fog
  colour, then the beams added like light on a screen). The `Haze` slot (3.3), the three switches
  with the sub-knob `volGain` (3.1), debug view 6 (the air's in-scatter alone).
- Kept: volumetric's march, the phase `0.85 HG(0.75) + 0.15 HG(-0.3)`, the sun radiance from the
  rig, the spotlight march with the shading's cone and the probe 6 phase sign, the depth-aware mean
  with gain and floored high-pass and probe 8's edge clamp, the 11-tap depth-aware blur; haze's
  closed-form height extinction (sigma 1.1e-4, scale height 350 m, chroma 0.85 / 1 / 1.2), the
  valley mist (2.5e-3, top 40 m above the lowest of 15 taps, noise banks at 1700 and 520 m, fall
  25 m), the density-weighted sun visibility, the mist's near-white sky term and broad sun lobe,
  nothing inside 150 m, and the classic fog's start moved out to 55% of its end in clear weather.
- Rewritten: one post stage instead of a composite rewrite and haze in every remastered pixel
  shader; the temporal accumulation is the TAA history (the march's start turns per frame, as A5
  did for AO and shafts). The forward fog's edge moves by a scale of the vertex fog factor in
  `finishFragment` and the water main (`min(vFogTC * (1 + litHaze.x), 1)` is exactly the factor of
  the moved start), so no vertex shader changed. The haze's limit is the sky table's fog colour
  (`atmoFogColor`, `fogColorToward` with the table off), the colour the forward fog converges to.
  The apply puts the haze under the forward fog's edge (`mix(limit (1 - T), S, f)`), so toward the
  view distance the mist dissolves into the sky: the hard line where the mist met the horizon (the
  critic item) is gone. The medium follows the weather (`ComputeAirMedium`): beam mist 0.12 in
  clear air, 1.5 from setFog 0.1, more above; contrast gain 4 and high-pass 0.85 in clear air, 2.5
  and 0.6 in mist; the sun within a degree of the horizon, rain and setFog count as mist; the haze
  thickens with setFog (x4 at 1) and rain (x2); the valley mist lies in the morning and in setFog
  from 0.05 (full at 0.3), 40% of it under an overcast.
- Corrections found in the look check (dev runs 1 to 3): (1) with the haze notes' morning (full to
  22 deg, gone by 38) `lowsun_fp` (08:00 in March, sun 15 deg, clear) took a mist veil over the
  plain and the hillside: the morning is now full to 5 deg and gone by 15 (3.2), and the mist's
  density takes `ValleyDepthWeight` (the ring taps' mean ground 20 to 60 m above the floor), so a
  plain and the open sea pool none; (2) the research's clear-air beam mist 0.3 with high-pass 0.6
  drew a warm band along the horizon toward the low sun at `lowsun_fp` (+7% horizon-band luma from
  the beams alone); 0.12 and 0.85 bring it to +1.2% and keep the beam through the cypress gap at
  `pine_eve`; (3) the haze's air has no sun lobe of its own: its limit already holds the glow toward
  the sun (the table's aureole, the classic glow), and the prototype's extra HG(0.72) lobe would
  light far ground toward a low sun brighter than the sky behind it; the mist keeps its lobe.
  Not built: the precomputed mist floor texture. The 15 height taps run once per half-resolution
  pixel where the mist runs (the prototype's cost came from running them in every lit fragment, with
  overdraw), so no texture or unit is needed.
- Dropped as planned: halos for lamps and fires, `volCarve`, the un-floored high-pass, the sky
  backdrop blit, the per-fragment haze, the variant channel, the other haze variants (grey extinction
  is not offered as a knob); also the research's own temporal resolve.
- Off equals before (item 5; `20261018-080035-before` on the base build against
  `20261018-084929-final2-new`, the lane test `b8lane` with the features_ab clocks and cloud pin):
  with TAA off (a shot repeats there) the three switches off are pixel-identical to the base build
  at `pinewood_morn`, `pine_eve` and `sunrise_ridge`; `valley_0515` 51 pixels over 2 levels (the base
  build's own repeat 384, the water), `lowsun_fp` 1,144 (base 1,143, the same moving thing on the
  horizon), `noon_fields` 1,608 (the posed soldier's head), `hl_front` 236 (the AI driver's head);
  `town_night` fire and smoke as the base build's own repeat. With TAA on shots do not repeat (the
  jitter phase), as in B11.
- On against off (TAA on, 1080p, 4x MSAA; mean display luma per band, sky / horizon / ground):
  `pinewood_morn` (setFog 0.1) +3.6 / +17.0 / +3.3%, R/B 0.934 to 0.950 (warmer): shafts through the
  crowns and a pale veil over the backlit crowns near the sun; `valley_0515` -2.7 / -3.4 / +4.9%:
  pink mist banks along the shore and the low ground, islands and ridges out of it, the far sea into
  the sky; `sunrise_ridge` -2.9 / +15.1 / +0.4%: the cypresses' shadow volume through glowing mist,
  the far massif under pink mist; `hl_front` (22:00, setFog 0.2) +14.4 / +2.9 / +0.1%: the Jeep's
  cones glow above the lit road, the night sky unchanged; `lowsun_fp` -1.0 / +2.3 / 0.0%, the M113's
  hull R/B 1.220 to 1.218; `pine_eve` -1.7 / -0.1 / +1.6%, the beam through the gap kept;
  `noon_fields` 0.0 / +0.3 / 0.0%; `town_night` within its fire and smoke noise. The skies lose 1 to
  3% where the legacy shafts' glow toward the sun is gone; the frame's R/B drops 3 to 7% on the
  sunrise views (the white-pink mist is less saturated than the lit ground), hulls unchanged.
- Cost (single readings, the backend's smoothed `air` section, RTX 4060 Ti, 1 or 2 other games on
  the machine, no clock log; `gpu-final2.md`): with beams 0.51 to 0.97 ms (half-resolution march,
  blur, mean and apply), haze and mist without beams 0.12 to 0.25 ms, the apply alone at noon and at
  night 0.04 to 0.05 ms; the legacy shafts it replaces took 0.27 to 0.44 ms. Under the plan's 2 to 4
  ms at dawn, dusk and in mist on a 3060 (MENU's 1.5 to 2x rule: about 0.8 to 2 ms).
- Gate: build 0 warnings; touched C++ files clang-format clean (all were clean at the base); unit
  gate tags plus `[FeatureSwitches],[FeatureGates],[PostStages],[TemporalAA],[TerrainDetail],
  [LumaSharpen],[atmosphere],[AirMedium]` 536/536 (13 new cases); the full suite the 6 known
  localisation failures only; smoke_abel 1/1 (GL error count 0, KHR_debug ids 131154, 131169 and
  131218 only); 19 of 19 contract mutations turn their cases red (`mutations.log`).
- Open: the veil over backlit crowns in setFog (physical, close to "flattened crowns"; `volGain` and
  the misty high-pass are the levers); the far massif under sunrise mist at 05:08 (the haze notes'
  "too washed out" risk); the 06:00 to 06:30 valley mist of the haze notes is gone with the shorter
  morning; with TAA off thin foliage against the glow shows speckles; half-resolution shafts beside
  trunks have blocky edges; a smoke puff in front of far terrain takes that terrain's haze; the beams
  and the legacy shafts swap at the beams' gate edge (sun 30 deg, 60 deg from the view), not seen in
  motion; AI sight ignores the medium (`AirOpticalDepth` is ready; the follow-up issue is not filed);
  no RTX 3060 measured; interiors, optics and night vision skip the stage (the fog edge follows a
  frame later).

