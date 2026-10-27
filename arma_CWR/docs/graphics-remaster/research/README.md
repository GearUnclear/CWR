# Remastered lighting research (October 2026)

This folder documents the research run that tried to give Uslu dur! an "RTX on" look from the models and textures the game already ships. It covers what was tried, what real game data showed, what an independent critic said about each idea, the bugs found on the way and how they were fixed, the GPU budget, and the road from here.

In this folder "the original look" means the stock CWA lighting of the Remaster engine (the "Classic" option of the Lighting row, labelled "Classic" in every comparison picture). "Current Remastered" means the new lighting as it stood on branch `claude/youthful-lovelace-8imbdh` at 3c53141 when the research ran. The current design is in `../DESIGN.md`; the change history is in `../ITERATIONS.md`.

These pages are a snapshot of 2026-10-17 and 2026-10-18 and are not updated. Since then the integration plan landed in full and the photoreal overhaul replaced much of the look (`../ITERATIONS.md`, O1), so statements such as "not landed", "candidate for the new branch" or "AI ignores the mist" describe that date, not the code today.

## Pages in this folder

| Page | What |
|---|---|
| `README.md` | this summary |
| `combat.md`, `shade.md`, `perf.md`, `ssr.md`, `volumetric.md`, `materials.md`, `atmosphere.md`, `hfrt.md`, `haze.md`, `shadows.md`, `night.md`, `clouds.md`, `texsharp.md`, `temporal.md`, `gbuffer.md`, `foliage.md`, `tiling.md`, `normals.md`, `aa.md`, `exposure.md`, `ssgi.md`, `wildcard.md` | one page per research track, in rank order below |
| `water.md` | the separate sea study, with a pointer to its own branch |
| `bugs.md` | every bug found, its root cause, its fix commit and its tests |

## Images are not in the repository

Every picture named in these pages is a render of game data, so none is committed. A `tmp\` path below and in the track pages is the untracked `tmp` folder of the `lighting` worktree the research ran in. That worktree and the scratch worktrees (`gfx-<track>`, `fix-*`, `fix2-*`) were removed on 2026-10-26; their text notes, patches and diffs are kept on the archive branch `archive/local-work-2026-10` (`graphics/lighting/tmp_/` and `lanes/<worktree>/`), and most pictures were deleted. The layout was:

- `tmp\research\<track>\` holds each track's notes, `prototype.patch`, `BEST_*.png` comparison sheets and raw runs.
- `tmp\research\gallery\` holds one picture per track named by rank (`01_combat.png` to `22_wildcard.png`, `W_water.png`) and `SHEET.png` with all of them on one sheet.
- `tmp\research\MENU.md` is the ranked menu the research produced; these pages are built from it, from each track's `NOTES.md` and from the critic verdicts.
- `tmp\bugfix\<bug>\` holds the bug-fix notes and their before and after pictures.
- `tmp\captures\` holds the full capture sets: `20261017-final` (the first in-game run) and `20261018-bugfix` (after the bug fixes).

The pictures can be re-shot with `tests/rendering/captures/run-captures.ps1` and the research tests each track lists.

## 1. What the research was

### The goal

The first in-game captures of the new lighting (`20261017-final`, see `../ITERATIONS.md`) did not impress the owner. On 2026-10-17 the owner wrote "these are marginally better, at most?" (one typo corrected) and, about the sea, "the water is so much less defined looks so smooth". The owner then set the bar:

> aim for the stars we want this to be like RTX on haha maybe not raytracing but you see the vision. a remaster without changing model or texture

and added "dont be afraid to try and fail".

"RTX on" here means light that looks path traced: light that bounces and carries colour, soft shadows that harden at contact, reflections, light visible in the air, emissive light from fires and flashes. All of it from the existing models and textures.

### The owner's decisions (interview, 2026-10-17)

| Question | Owner's answer |
|---|---|
| Should the original look stay as an option? | **No, replace it.** The new lighting becomes the only look; the original stays only as the "before" picture. So the old rule "keep the original look bit-identical" no longer applies. |
| What computer should it run well on? | **A modern gaming PC:** an RTX 3060 class card, 60 fps at 1080p. Heavy effects are allowed if they fit. |
| Which improvements matter? | **All four areas:** light and shadow; sky and air; water and shine; night and combat. |
| How will results be judged? | **Side-by-side pictures:** same spot, same time, before and after. |

The owner also stated that the project targets the Remaster engine only ("'classic' engine code is no longer used in this project").

Looks the owner had already rejected before this work (the owner's taste list in the brief for this work, built partly from the earlier lighting branch's log, `arma_CWR/tests/rendering/ITERATIONS.md` on `origin/graphics/lighting-improvements`): blue haze or a wash over the frame, "Lambert-only" flat shading, mustard or khaki ground, crushed shade on vehicles and soldiers (about 0.42 of lit was "crushed"; the target is about 0.6), flattened tree crowns, any result that is only darker, and evenings that read as a neutral 5600 K instead of at least as warm as the original look.

## 2. How it was run

- **22 research tracks**, each one technique. Each track got an agent that built a working prototype in its own scratch worktree (`gfx-<track>`, detached at 3c53141, no branch) or as shader overrides (`CWR_SHADER_OVERRIDE_DIR`), and saved its engine change as `prototype.patch` (the shader-only tiling track kept only its override shaders).
- **Real captures on the Remaster data**, never stand-ins: Abel, Eden, and Sinai with @LoBo, through `tests/rendering/captures/run-captures.ps1` with the track's own data junction (so cleanup only stopped that track's games), on a hidden desktop. 960x640 window at 150% render scale (1440x960 internal) with 4x MSAA. Every comparison shows the original look, current Remastered and the prototype at the same spot and time.
- **An independent critic per track.** A second agent checked every claim against the files on disk: that each cited image exists, that the game logs show the prototype loaded and compiled, that the before and after pictures differ in pixels where claimed. It then looked at the pictures itself and gave two scores out of 10: **worth building** (overall) and **RTX-on** (how far the result moves toward the path-traced look). It also listed overclaims and taste violations.
- **A synthesis** ranked the tracks and wrote `MENU.md`.
- The run was paused once at the owner's request ("pause all verifiers. we will run out of usage before they can finish") and resumed later ("gogo"), so many tracks have two sessions.
- **A separate water study** ran alongside, on its own branch. No critic reviewed it (`water.md`).
- **Nothing was fabricated.** Every critic found every cited file and confirmed every prototype really ran.

## 3. Ranked results

Ranked by the critic's "worth building" score, ties broken by the RTX-on score. GPU costs are rough (see section 9).

| # | Track | What the player sees | Area | Worth building | RTX-on | GPU cost (measured on a shared GPU) |
|---|---|---|---|---|---|---|
| 1 | [combat](combat.md) | Fires, flares and shell bursts light the men and cast long shadows | night and combat | 7 | 6 | too small to measure; est. 1 to 3 ms in fights |
| 2 | [shade](shade.md) | Spawned vehicles get the new lighting; no grey veil; no phantom cloud shadows | light and shadow | 7 | 2 | about 0 |
| 3 | [perf](perf.md) | Nothing visible; long views get much cheaper | budget | 7 | 2 | saves 7 to 17 ms CPU on long views |
| 4 | [ssr](ssr.md) | Calm water and wet streets reflect boats, hills and houses | water and shine | 6 | 5 | +0.35 to 1.9 ms |
| 5 | [volumetric](volumetric.md) | Sun shafts through pine woods; headlight cones in night mist | sky and air | 6 | 4 | +0.4 to 2.4 ms |
| 6 | [materials](materials.md) | Wet weather: puddles, wet cloth and stone, lamp streaks on wet ground | water and shine | 6 | 4 | +0.4 ms median |
| 7 | [atmosphere](atmosphere.md) | A physically based sky: golden glow at low sun, orange twilight band | sky and air | 6 | 4 | near 0 GPU; a CPU hitch to fix |
| 8 | [hfrt](hfrt.md) | Every tree casts a long golden-hour shadow, near and far | light and shadow | 6 | 3 | too high as built: +2.5 to +16 ms |
| 9 | [haze](haze.md) | Distant hills fade into the real sky; pink morning mist in valleys | sky and air | 5.5 | 3 | unknown, probably high as built |
| 10 | [shadows](shadows.md) | Trees and buildings cast sun shadows again | light and shadow | 5 | 3 | +0.2 to 1.4 ms |
| 11 | [night](night.md) | Real low-beam headlights, stars, moon shadows, warm lamps | night and combat | 5 | 3 | +0.8 to 3.3 ms |
| 12 | [clouds](clouds.md) | Sky clouds glow on their sun side at dawn and dusk | sky and air | 5 | 3 | +0.2 to 0.9 ms |
| 13 | [texsharp](texsharp.md) | Grass and rock grain on slopes instead of an airbrushed smear | surface detail | 5 | 2 | +0.5 to 2 ms |
| 14 | [temporal](temporal.md) | TAA: smoother edges; cheap noisy effects come out clean | groundwork | 5 | 2 | +0.07 to 0.14 ms |
| 15 | [gbuffer](gbuffer.md) | Nothing visible yet; groundwork for reflections and bounce light | groundwork | 5 | 1.5 | est. +3 to 5 ms with its bounce light |
| 16 | [foliage](foliage.md) | Leaves glow when backlit; greener shade inside crowns | light and shadow | 4.5 | 3 | +0 to 1 ms |
| 17 | [tiling](tiling.md) | Fewer repeating ground patterns; desert gets real variation | surface detail | 4.5 | 2 | +0.5 to 2.5 ms |
| 18 | [normals](normals.md) | Pebble relief on gravel at low sun | surface detail | 4 | 2 | +1 to 2 ms |
| 19 | [aa](aa.md) | Anti-aliasing switched on; less shimmer on foliage | image quality | 4 | 1 | about 0 at native resolution |
| 20 | [exposure](exposure.md) | Dark vehicle sides readable at low sun; orange sun and fire cores | light and shadow | 3.5 | 1.5 | about 1 ms |
| 21 | [ssgi](ssgi.md) | Invisible by day; a hull lit by a nearby headlight pool at night | light and shadow | 3 | 2 | +1.5 to 3.5 ms |
| 22 | [wildcard](wildcard.md) | Soft dark pools under vehicles on overcast days | light and shadow | 3 | 2 | +1.6 to 3.9 ms, 160 MiB |
| W | [water](water.md) | Wave relief on the sea and a glittering sun path | water and shine | not reviewed | not reviewed | est. under 0.1 ms |

**The short version:**

- **No single track makes it look like a different game on its own.** The two closest are combat light at night (RTX-on 6) and reflections on water and wet streets (RTX-on 5).
- **What worked**, as RTX ingredients: emissive light with shadows (combat); reflections on water and wet ground (ssr, materials, water); visible light in the air (volumetric, haze); a physical sky and long low-sun tree shadows (atmosphere, clouds, hfrt).
- **What did not work: bounce light** (section 4).
- **The most valuable findings were bugs** that made every current Remastered picture wrong (section 7).

## 4. Why bounce light failed four times

Bounce light (light that reflects off one surface onto another and carries its colour) is the core of the RTX look. Four tracks tried it, and a fifth tried a small version. All made almost no visible change.

| Track | What it tried | What happened |
|---|---|---|
| [gbuffer](gbuffer.md) | Screen-space one-bounce light from a G-buffer, with ray sky visibility | Strongest variant lifted shaded vehicle faces by about 5 to 10 levels; with its own occlusion the frame got slightly darker. Measured on the Jeep's shaded rear: the bounce adds about 0.0016 linear to a surface at about 0.05. |
| [ssgi](ssgi.md) | Screen-traced bounce with temporal accumulation | Daytime change 0.0 to 0.5 levels on average, inside the shot-to-shot noise, even with every object treated as fully sky-lit. Only a non-physical double-counted upper bound reached +9 to +13 levels on lower hulls. Replacing the AO with the traced result only darkened. One narrow night win: a hull beside a headlight pool. |
| [hfrt](hfrt.md) | Tracing the terrain heightfield for sky visibility and one bounce | The islands see 90 to 100% of the sky almost everywhere; the bounce was invisible even at three times strength; +12 to 30 ms. |
| [wildcard](wildcard.md) | 24 sky-visibility maps from the shadow casters, plus one grey bounce | About +-1 level on open views; soft pools under vehicles only on overcast close-ups; the physically straight setting only made woods darker. |
| [night](night.md) | Street-lamp light bounced off the ground | 0.21 of 255 mean difference: no visible effect. |

**Why:**

1. **The existing fill already covers that range.** Current Remastered has a hand-tuned sky hemisphere (zenith, a warm sunward horizon, a cool anti-sun horizon) plus a ground-coloured bounce term. The owner asked for shade at about 0.6 of lit, so that fill is bright. A traced ray only adds light where what it hits is brighter than the sky it hides. Sunlit Abel and Sinai ground sits close to the rig's assumed ground albedo, so it adds nearly nothing.
2. **CWA content is dark.** Military paint has an albedo of about 0.16 linear; one more bounce off it is tiny.
3. **The test views are backlit.** In the acceptance views every face the camera sees looks down onto its own cast shadow.
4. **Gentle terrain.** The islands' hills are gentle for their spacing, so terrain blocks little sky.

**The experiment nobody ran:** every attempt added traced light *on top* of the existing fill at 1 to 3 times strength. Nobody turned the hand-tuned sky and ground fill down hard and let traced sky visibility plus one coloured bounce carry the shade. That is the only route found here toward shade that looks path traced, with colour bleeding and contact darkening, and it is a bet, not a plan (section 8, phase 4).

## 5. Other honest negatives

Each of these saves someone from trying it again. Details are on the track pages.

- **Air:** lamp and campfire halos at night tinted the whole sky brown (volumetric, night). Carving beams out of the haze darkened the frame (volumetric). Strongly blue distance haze was rejected (haze rounds 1 to 5). A 48-sample sun march inside the mist changed about 2% of pixels (haze).
- **Sky:** lighting the ground from the physical sky turned ground and shadows blue (atmosphere). One brightness setting for the whole physical sky gave a pale overcast and a grey sunrise (atmosphere). Stronger cloud relief at midday looked glossy and painterly (clouds).
- **Shadows:** screen-space contact shadows (the shadow map already covers them). Soft sun shadows are close to invisible at frame size; a sun 2 to 3 degrees wide looks mushy. A doubled shadow range gave no visible gain for +5 to 8 ms (shadows, temporal).
- **Surface detail:** texture filtering, streaming, rebuilt mip chains, sharper magnification, relief from the base texture, coarse detail layers (texsharp): no gain; filtering is already at 16x. Normal maps generated at load time for vehicles: nothing visible, because the paint already has the lines painted in (normals). Relief from each terrain square's base texture: seams every 50 m (normals). Warping the terrain texture lookup: marbled swirls (tiling). Ground variation that also changes saturation: washed the ground pale (tiling). A strong green push on leaves: lime and cartoonish (foliage).
- **Tone:** a floating-point HDR frame (no gain by day; smoke blew out to white at night), the AgX curve (muted colours, grey sun), eye adaptation toward a fixed target (+2 stops, blown skies), brightening shade everywhere (the sea went flat grey) (exposure). Tuning only the existing lighting knobs cannot reach the target shade without flattening ground shadows (shade).
- **Reflections and materials:** reflections on vehicle paint show nothing (ssr, materials). Lamp reflections using the ground's own roughness put white sheets over the wet square (materials v7 and v8; fixed in v9). Material guessing on dry surfaces is almost invisible (materials).
- **Other:** a regular 2x2 dither in the combat shadow printed a grid on the ground (combat). Cheaper foliage shader shortcuts saved at most 0.5 ms (perf). TAA used only as anti-aliasing is a sideways move (temporal). Sharpening before the final downscale was blurred away; strong sharpening looks processed (aa). Tree crowns reading the sky-visibility maps cost +10 ms and went heavy and dark (wildcard). The canopy map's sky term on the forest floor only darkened (foliage).

## 6. What the critics did not accept

The critics found no fabrication. They did find overclaims in every track; each track page lists them. The common ones:

- **GPU cost.** Nothing was measured on an idle machine, at 1080p, or on an RTX 3060. Every reading came from an RTX 4060 Ti shared with up to four other test games. The same unchanged view often swung 2 to 10 times between shots. The millisecond numbers printed on the comparison pictures are single noisy readings.
- **Motion.** Almost every judgement is from still pictures. Shimmer, crawling patterns, ghosting and popping are untested in most tracks.
- **The before pictures were wrong in two ways:** script-spawned vehicles were drawn with the original look's lighting, and trees and buildings cast no shadows. Some tracks' "current Remastered" panels also came from their own scratch build.
- **Mixed runs:** several headline images compare shots from different runs (different cloud positions, different code versions).

## 7. Bugs found, and which commits fixed them

The research found engine bugs that made every current Remastered picture wrong. The owner said to fix them first ("Fix bugs with opus agents first"). Each got a fix agent and an independent reviewer; accepted fixes landed on `claude/youthful-lovelace-8imbdh`. Full detail: `bugs.md`.

| Bug | Fix |
|---|---|
| Capture rig: coarse first shot (adaptive object detail held for 5 s), knobs leaking between modes, lamps switching at random after `setDate` | 874638c |
| NaN crown normals from leaf cards with zero mesh normals | d57b712 |
| Shadow sampler bound to texture 0 (GL undefined behaviour) | f102499 |
| Cloud shadows under a clear sky (about 4% of the map) | 5cf85cf |
| Sun-shaft composite lays a grey veil on near vehicles | 463506f |
| Grouped bushes take the first bush's terrain fit (stock) | d80f868 |
| `switchLight` on a non-lamp crashes the game (stock) | ab61aa5 |
| Script-spawned vehicles skip the new lighting | 747ffc8 |
| Over-bright, yellow-green night: a 1.5 local-light cap and an invisible moon | 4515770 |
| Street lamps read their 3-entry config colour as white with ten times the ambient (stock) | 72f32d4 |
| Tree and building shadows off at start-up | **not landed** (the fix also applied the terrain grid tier at boot); second attempt in progress |
| No anti-aliasing in the Remastered default | **not landed** (the migration also added MSAA on top of a chosen supersampling scale); second attempt in progress |

After the landed fixes, the full capture set `20261018-bugfix` shows: every mode settled, the GL undefined-behaviour warnings gone (only performance and info messages remain), the night town at the original look's brightness (lower frame 157.5 to 121.4, original look 120.4), and the low-sun vehicles solid instead of veiled.

## 8. The road to RTX-on

The four RTX ingredients that worked are emissive light with shadows, reflections, visible light in the air, and a physical sky with long low-sun tree shadows. Bounce light did not work. So the plan stacks the working pieces on a corrected base, shares groundwork so each piece costs less, and leaves real bounce light as the last step and a research bet.

### Step 0: owed before any build

- A clean timing run: an idle machine, 1080p, ideally an RTX 3060 class card.
- Fix the capture harness. **Done** in 874638c (detail pin, scene settle, knob reset, per-shot draw counts).

### Phase 1: correct the base (small to medium engine changes, almost no GPU cost)

1. The spawned-vehicle fix. **Done** (747ffc8). Follow-up: crew-view lighting now depends on load time.
2. Tree and building shadows on at start-up. **Not landed yet.**
3. No cloud shadows under a clear sky. **Done** (5cf85cf).
4. 4x MSAA on by default. **Not landed yet.**
5. The frame-cost fixes: the per-object light gate and a hardened, faster terrain-occlusion test (perf); the bush fix is **done** (d80f868).
6. Shoot every before picture again. Earlier judgements of vehicles and trees were made on wrong pictures.

### Phase 2: shared groundwork

- **A sky table on the GPU** (atmosphere): one table for sky colour, horizon, fog colour and water reflection, later a physical sky light. Half-way hue, saturation at most 1.15.
- **TAA** (temporal) with per-object motion and a particle mask, resolved before the tone curve.
- **A packed G-buffer** (gbuffer): surface direction, material class (from the materials track's name-based guess, moved into a config table), colour, and light stored after the night darkening.
- **One shared light list**, culled per screen tile, for combat light, night lamps and headlights, and wet-street reflections.
- **One half-resolution air pass**: haze, valley mist, sun and headlight beams in a single march against the shadow map, driven by weather and time of day, and fed to AI sight.

### Phase 3: the visible layer, one piece per area

- **Night and combat:** combat light with its shadow in a half-resolution pass, 2 to 4 shadowed lights and a brief flash brightening; the night moon and low beams on the shared light list.
- **Water and shine:** sea waves, reflections feeding the water shader, wetness.
- **Light and shadow:** long tree shadows at reduced resolution, the shade calibration, a light fill under tree crowns so woods do not go dark.
- **Sky and air:** the air pass at dawn, dusk and in mist; lit clouds only at low sun.
- **Polish:** terrain detail, downscale and sharpen, desert ground variation.

### Phase 4: the bounce-light bet

Turn the hand-tuned sky and ground fill down hard. Let traced sky visibility (the wildcard track's maps, or G-buffer screen tracing) plus one coloured bounce carry the shade, smoothed by TAA. Bounce light added on top of the existing fill was invisible four times, so this is a bet, not a plan.

### How the pieces depend on each other

```
Step 0 timing + harness (harness done)
  -> Phase 1: vehicle fix (done), tree shadows, clear-sky fix (done), MSAA, frame-cost fixes
       -> re-shoot the before pictures
  -> sky table ---------> air pass (haze + beams) -> lit clouds
  -> TAA ---------------> cleaner combat shadows, cheaper beams, reflections, tree shadows
  -> G-buffer ----------> reflections + wetness -> later: bounce light (phase 4)
  -> shared light list -> combat + night + lamp reflections on wet ground
  -> tree shadows ------> beams (need casters), long tree shadows near the camera
```

### The first three to build (the synthesis's recommendation)

1. **The phase 1 base fixes**, then re-shoot the before pictures. Two of the four are now landed.
2. **The frame-cost fixes** (perf): they change no pixels, cut the CPU frame on long views from 32 to 12 ms, and pay for what follows.
3. **Combat light, production version**: the best critic scores (7 and 6), no G-buffer needed, and its half-resolution shadow pass with temporal smoothing builds the first piece of the TAA groundwork.

**Cheap side task:** the sea waves are shader-only and answer a complaint the owner already made. They need a critic check and a look in motion first.

The owner asked on 2026-10-17 for the ideas that worked to be compiled on a new branch, `graphics-remaster`, and for a video with a side-by-side comparison in motion of each approach. That branch did not exist yet when these pages were written; the track pages say which parts are candidates.

## 9. The GPU budget at 1080p on an RTX 3060 (rough)

Rule of thumb: about 1.5 to 2 times the measured numbers (1080p has 1.5 times the pixels of the test resolution, and a 3060 is slower than the test card). Everything here is an estimate built on noisy readings.

| Piece | Measured (shared 4060 Ti, 1440x960) | Estimate, 1080p on a 3060 | When it runs |
|---|---|---|---|
| Tree and building shadows | +0.2 to 1.4 ms | 0.5 to 2.5 ms | day |
| Sky table | under 0.3 ms (estimate) | about 0.5 ms | always |
| Air pass (haze + beams, half res) | beams 0.4 to 2.4 ms; haze unknown | 2 to 4 ms | dawn, dusk, mist, headlights |
| TAA | 0.07 to 0.14 ms | about 0.3 ms | always |
| G-buffer (packed) | 1 to 2 ms (estimate) | 1.5 to 3 ms | always |
| Reflections | 0.35 to 1.9 ms | 1 to 3 ms | water or wet ground on screen |
| Wetness | +0.4 ms | about 0.8 ms | rain |
| Long tree shadows (reduced res) | unmeasured; forward version +2.5 to 16 ms | target 1.5 to 3 ms | low sun |
| Lit clouds | 0.2 to 0.9 ms | 0.5 to 1.5 ms | day |
| Sea waves | under 0.1 ms (estimate) | about 0.2 ms | sea on screen |
| Terrain detail | 0.5 to 2 ms | 1 to 3 ms | always |
| Downscale and sharpen | 0.16 ms at 1080p | about 0.2 ms | always |
| Combat light | under 1 to 2 ms (unresolved) | 1 to 3 ms | fights |
| Night lights (shared list) | +0.8 to 3.3 ms | 1 to 2 ms | night |

- **Base frame:** after the frame-cost fixes, current Remastered measured 2.5 ms (sunrise ridge) to 7.5 ms (noon fields) of GPU time on the shared card; perhaps 4 to 12 ms at 1080p on a 3060.
- **The Remastered shading itself is cheap:** 0 to 1 ms in the world pass and 0.3 to 1.8 ms in the post pass (perf). Long views are bound by draw calls, not shading.
- **Day, typical:** about 8 to 10 ms of effects. **Day, worst case** (low sun, water and woods in one view): about 20 ms, which does not fit.
- **Night, typical:** about 6 ms. **Night, worst case** (a fight in the rain): about 13 ms.
- **Verdict:** the 16.6 ms budget for 60 fps holds only if effects are gated by time of day, weather and what is on screen, and the heavy ones run at half resolution. Many never run together: long tree shadows and beams only at low sun, combat light only in fights, reflections only where water or wet ground is in view.
- **Caveats:** all readings are from one RTX 4060 Ti shared with up to four other test games, at 1440x960 internal with 4x MSAA. The capture runs before 874638c also let the adaptive object detail vary between shots, so some before and after pairs drew different objects.

## 10. Open questions

- **Timing:** a clean run on an idle machine at 1080p, ideally on an RTX 3060, is still owed.
- **Motion:** almost everything was judged from stills; the owner's requested side-by-side video in motion is pending.
- **The two unlanded base fixes** (tree shadows at start-up, MSAA by default), then a re-shoot of every before picture.
- **Owner calls** the research could not make:
  - Physical sky hue against the half-way hue, and the saturation (atmosphere).
  - Whether to accept darker woods once tree shadows are on, or wait for light through the crowns (shadows, foliage).
  - How dark a moonless night may be for play (night).
  - The saturated yellow-orange sun disc from the hue-keeping curve (exposure).
  - The sea colour at noon, which reads cooler and bluer than the original look (water).
  - The flash frame of an explosion, which in the combat prototype is dimmer than the original look's whiteout (combat).
  - Smaller cloud-shadow patches at overcast 0.5, or leave as is (bugs.md, item 4).
  - The lamp data fix also changes night vision near towns (bugs.md, item 9).
- **Follow-ups from the bug fixes:** crew-view lighting depends on load time; eight more engine classes lack their own casting id, the defect behind the `switchLight` crash (`Airplane` the most dangerous); the moon may lift dusk by up to 1.3 EV; the dead water-shader path.
- **Gameplay:** valley mist is visual only; AI would see through it.
- **The bounce-light bet** (phase 4) is untried.
- **Stale advice:** many track notes say "keep the original look bit-identical". Owner decision 1 removed that requirement.
