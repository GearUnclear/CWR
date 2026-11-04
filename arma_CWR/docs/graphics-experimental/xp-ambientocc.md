# XP lane xp-ambientocc: ambient occlusion and contact shadows

Base 243ef1e0 (graphics-remaster tip, also graphics-experimental). Worktree D:\Arma_CWA_worktrees\sq-xp-ambientocc (detached, nothing committed). Patch: items\XP-xp-ambientocc\patch.diff (3 files, git diff --binary against 243ef1e0). Status: built and unit-gated; the stills and timing jobs are queued but had not run after 5 hours (the shared queue had 29 jobs ahead of them, mostly timed quick jobs at about 14 minutes each). No picture or timing result yet, so no look verdict.

## What the engine already has (checked in code, not taken from the plan)

There was nothing to "add". The AO in use is already a full modern implementation:

- GTAO (Jimenez 2016, laid out as XeGTAO) at half resolution in the post pass: 3 slices with 8 steps per side, 2 slices under TAA with a depth-reprojected history (weight 0.85), a 9-tap depth-aware blur, the cosine-weighted horizon integral normalised by the empty slice, thickness and below-ground weights, Jimenez multi-bounce from the albedo, a 0.35 floor, and a stronger and wider search on static architecture. It darkens only the light it may darken (sky, bounce, sky reflection), never the direct sun, through a second render target the world shaders write. Files: EngineGL33_PostSurfaceShaders.cpp, the composite in EngineGL33_PostShaders.cpp, SurfaceLight.hpp.
- Screen-space contact shadows in the composite: 8 steps toward the sun, 0.12 to 1.2 m long, faded out between 30 and 50 m, acting on the sun's share only (none on leaves).
- Large-scale sky occlusion under tree crowns and building blocks from the canopy heightfield, at any distance.
- Measured cost on this machine's 4060 Ti (survey S7): AO section 0.06 to 0.26 ms. There is little to win on performance here.

What it does not do: GTAO fades out between 150 and 300 m, and contact shadows stop at 50 m, while the target is a 4 to 5 km view. That gap is what this lane works on.

## What I changed (all behind environment switches, default off)

| switch | what it does |
|---|---|
| `POSEIDON_XP_AO_FAR=1` | GTAO fades out from 450 to 900 m instead of 150 to 300 m, and its world radius grows with depth: max(4 m, 0.012 x depth), so 6 m at 500 m and 11 m at 900 m. A house's foot, a tree base or a hedge at 300 to 900 m keeps its contact darkening. Tunables: `POSEIDON_XP_AO_FAR_FADE0`, `_FADE1`, `_RPM`. |
| `POSEIDON_XP_CONTACT=1` | Contact shadows march about 4.5% of the depth (0.12 to 4 m) with 12 steps bunched toward the receiver, out to 150 m (fading from 90 m). Small casters that the far shadow cascades blur away (a wheel, a bush foot, a fence post, a soldier at 60 to 150 m) keep a sun shadow. Tunables: `POSEIDON_XP_CONTACT_MAXZ`, `_FADE0`, `_CAP`. |
| `POSEIDON_XP_AO_VIEW=1` | Diagnostic only: the composite shows ao x contact (the surfaces path's debug view 4) whatever the lighting knobs say, so a stills job can show the occlusion buffer itself. |

The tunables are read from the environment, so a second round can try other numbers on the same exe with no rebuild (for example `POSEIDON_XP_AO_FAR_FADE0=300`, `_FADE1=600` if 900 m shows depth-precision grey).

Files:
- EngineGL33_PostFX.cpp: switch and tunable reading (once, logged), the GTAO fade and radius uniforms, the composite's `xpContact` uniform, the AO view.
- EngineGL33_PostSurfaceShaders.cpp: one guarded line in GTAO (`if (xpAo.x > 0.0) radius = max(radius, z * xpAo.x);`).
- EngineGL33_PostShaders.cpp: a separate `contactShadowXp` function, chosen by a uniform. The shipped `contactShadow` is untouched.

Default-off frame: with every switch off, the GTAO radius line is skipped by a uniform branch, the composite calls the unchanged contactShadow, and every uniform the C++ sets keeps the tip's value. I expect the off frame to be pixel-identical to the tip, but that is not proven yet: job 3 below checks it against the frozen integ7 exe.

## Build and tests

- Build green via squad-heavy: build_id 243ef1e0-dirty+57eb122ac4c5, no new warnings (the 5 baseline ones). Frozen dist arma_CWR\dist\sq-xp-ambientocc-r1-rwdi, exe sha256 6f3f3eca1c72...
- Unit gate PoseidonTests: 3525 of 3535 passed, the 6 known data failures, 4 skipped. The glslang shader tests compile the changed post shaders.
- clang-format 22.1.8 on changed lines (21.1.8 pending). Its 5 whitespace and brace fixes were made after the r1 dist was frozen, so the dist and patch.diff differ only in whitespace.

## Jobs queued (dry runs exit 0)

1. `60-20261103-062001` stills-aoview: both sides use the AO view. A is shipped, B is AO_FAR + CONTACT. Views: noon_fields, forest_edge_golden, town_noon, jeep_close_noon, lowsun_fp (Abel), noon_vista (Eden). Same exe, deterministic world, fixed settle 300.
2. `60-20261103-062002` stills-look: same views. A is switches off, B is AO_FAR + CONTACT.
3. `64-20261103-062003` stills-offident: this exe with switches off against integ7 (noon_fields, town_noon, forest_edge_golden). IDENTICAL means the default frame is the tip's.
4. `66-20261103-062500` quick-cost (timed): same exe, off against on, on noon_fields, town_noon, forest_edge_golden, noon_vista. Read the `ao` and `composite` GPU sections and gpu_frame.

Results land in items\XP-xp-ambientocc\results\{stills-aoview, stills-look, stills-offident, quick-cost}.

## How to judge them

- stills-aoview: in B the occlusion should reach the hillside bushes and trees past 300 m (noon_fields right side, noon_vista) and the town houses' feet (town_noon). Watch for a flat grey veil on distant open ground. That would be D24 depth precision: about 0.19 m steps at 500 m and 0.6 m at 900 m with the 0.07 to 0.08 m near plane (World.cpp:1294). If it shows, rerun with FADE0/FADE1 at 300/600. Contact: new dark contacts under bushes, wheels and the soldier past 50 m, and no speckle on open ground at the low sun (lowsun_fp, forest_edge_golden).
- stills-look: the change should read as grounding, not as a darker frame. If it only darkens, drop it (owner rule: nothing only darker).
- Cost expectation, from reasoning and not yet measured: AO_FAR adds the full GTAO for half-resolution pixels between 300 and 900 m. At eye height that is a thin band near the horizon plus far hillsides, so probably under +0.1 ms. CONTACT runs 12 rather than 8 full-resolution taps on sunlit pixels out to 150 m rather than 50 m, probably +0.1 to 0.3 ms. Together they should stay inside the 1 ms budget.

## Temporal flag

Both look switches rotate their noise per TAA frame, as the shipped ones do. A longer contact march at a low sun can crawl on rough ground in motion. The owner must judge it from a clip before it becomes a default or a quality tier.

## Recommendation for now

Quality tier (a "High" or "Ultra" AO setting), not default, until the stills and cost jobs are read. AO_VIEW stays a diagnostic.
