# Frame cost (track: perf)

Rank 3 of 22. Critic scores: worth building **7/10**, RTX-on **2/10**. Area: GPU and CPU budget.

## What the player sees

Nothing, by design. Long views get much cheaper, which frees time for the heavy effects.

## The question

In the first in-game run (`20261017-final`), `sunrise_ridge` cost 18.4 ms of GPU time with the new lighting against 7.2 ms with the original look. Where does the time go?

## The main findings

1. **That comparison was a measurement mistake.** Replaying the capture flow with the draw count logged showed the original-look shot drew 1,465 draws and the new-lighting shot right after it 2,202. Once the view had settled, both drew 9,500 to 10,400 objects and cost the same. (This track blamed view-distance streaming. The later capture-harness fix found the real cause: the adaptive object detail holds a coarse level for 5 s after a camera jump. See `bugs.md`.)
2. **Long views are bound by draw calls, not by shading, in both looks.** The GPU "world" timer mostly measures the GPU waiting for the CPU. Switching off every Remastered shader feature one at a time changed nothing measurable at `sunrise_ridge`.
3. **The new shading itself is cheap:** +0 to +0.9 ms in the world pass at the capture settings, plus 0.3 to 1.8 ms in the post pass (ambient occlusion is the largest piece).
4. **Two engine problems make the draw count 4 to 7 times larger than it needs to be:**
   - Instancing (drawing many copies of one object in a single call) is switched off for the whole scene whenever any light slot exists (`SceneDraw.cpp`, `NLights() == 0`). Abel's street lamps register 12 light slots, so instancing is off at dawn, noon and night alike.
   - No terrain occlusion: at `sunrise_ridge` 6,555 of the 7,669 objects in the draw list sit behind the ridge in front of the camera.
5. **Turning instancing on exposed a stock bug:** an instanced run draws every member with the first member's terrain fit, so bushes on slopes change silhouette. This bug was later fixed on the branch (d80f868).

## The technique

- **Per-object light gate:** batch an object when its own light list is empty, instead of checking the whole scene.
- **UBO dedupe:** upload only the changed part of the shader constant buffers.
- **Animated objects out of instanced runs** (the bush fix), using `Object::IsAnimated`.
- **Terrain horizon occlusion:** from the camera, 1,024 rays march the heightfield and keep the highest elevation seen. An object whose top lies under that line on every ray it spans is skipped. Objects closer than 75 m are never tested.

## What real data showed

Run 12, detail level pinned, medians, three other games on the GPU:

| View | Setup | Draws | CPU frame ms | GPU world ms |
|---|---|---|---|---|
| sunrise_ridge, view distance 4000 | current Remastered | 11,172 | 31.9 | 34.11 |
| | Remastered plus the fixes | 1,728 | 12.1 | 1.97 |
| noon_fields, view distance 2500 | current Remastered | 5,566 | 12.8 | 18.20 |
| | Remastered plus the fixes | 1,500 | 8.3 | 7.11 |

Pixel identity: `sunrise_ridge` 0 pixels over 2 levels outside the drifting sky. Walking (camera moving every frame): CPU frame 40.8 ms before, 14.3 ms with the fixes.

Shader shortcuts (early alpha cutout, 4 shadow taps on foliage and far cascades) are visually identical but save at most about 0.5 ms, inside the noise. Not worth it.

The harness's per-frame centre-pixel readback under `--autotest` serialises CPU and GPU and adds 3 to 16 ms of CPU frame in capture runs.

## Cost

Frees time. About 7 to 17 ms of CPU saved on long views, 1 to 2 ms in forests. The GPU does only 1 to 4 ms of real world work after the fixes, so a bold effect has 10 ms or more of GPU to spend at 60 fps on every view measured.

## Verdict

Critic: "Real, verified and pixel-exact cost work." It adds nothing a player would see; its value is the budget it frees.

## What production needs

- The per-object light gate and the instancing fix (the fix landed as d80f868; the gate did not).
- A faster terrain horizon table: today it costs 2.3 to 5.7 ms of CPU per rebuild, and it rebuilds on every camera move. Options: steps that grow with distance, fewer rays, SIMD, a worker thread. Move the cull before LOD selection and sorting.
- Prove the cull safe: long sun shadows from culled objects onto visible ground were never tested, and narrow notches between hills can slip through.
- UBO uploads that track what each buffer holds.

## Overclaims the critic found

- "World pass 34 to 2 ms" mostly measures removed CPU stalls. The honest number is the CPU frame, 32 to 12 ms. The best-frame GPU world goes 15.8 to 1.15 ms.
- On `noon_fields` the full set is slower than the simpler light gate alone (GPU frame median 7.48 against 5.72 ms): the bush fix gives back more than the cull saves there.
- The terrain cull is not strictly conservative (it uses the near-edge distance and the object position, not the sphere centre). It held on the tested views because of the safety margins.
- Culled shadow casters were never tested (the shadow pass read 0.00 ms in these runs, because tree shadows were off at start-up; see `bugs.md`).
- Nothing measured at 1080p.

## Where the work lives

- Notes and patch: `tmp\research\perf\NOTES.md`, `prototype.patch` (31 KB, mostly `SceneDraw.cpp`).
- Scratch worktree: `gfx-perf`, build `arma_CWR\dist\gfx-perf-rwdi`.
- Best images (not committed): `tmp\research\perf\BEST_sunrise_ridge.png`, `BEST_noon_fields.png`, `run8_quiet\noon_treeline_zoom_m0_top_m0all_bottom.png` (the bush bug).
- Integration: the bush fix is on `claude/youthful-lovelace-8imbdh` (d80f868). The light gate and terrain occlusion are candidates for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.

## Update 2026-10-31: the long views were the dynamic vertex upload

Issue #216 read the long-view cost as CPU time per draw call. That came from the capture harness: under tri the frame observer read a pixel back every frame (a blocking `glReadPixels`), which drained the GPU queue and counted as CPU time (the per-frame readback this page already flagged above). Measured again with the readback at 1 frame in 16, as in play (`POSEIDON_OBSERVER_SAMPLE=1`), at 1080p with 4x MSAA on the RTX 4060 Ti:

- The long views are GPU-bound, and the GPU time is pass1. The dawn valley at view distance 4,000 reads 29.6 ms CPU, 28.9 ms GPU frame, 23.7 ms pass1; Eden at view distance 5,000 reads 25.9 / 25.2 / 20.7.
- Pass1 is per-draw work: about 3,100 dynamic vertex buffers are re-uploaded every frame (3,099 uploads, 2.1 MB, at the valley), so pass1 ignores the pixel count and falls with the number of draws. Rewrites of shared static buffers by land-following objects, the cause suspected earlier, measured 0.
- A stand-in that uploads only the buffers whose vertices changed takes the valley to 12.3 / 9.2 / 1.6 ms and Eden to 12.4 / 9.6 / 2.0: CPU-bound, under the 15 ms CPU budget for 60 fps. It skips the land fit, so its CPU column is a little optimistic.
- Without the upload stall the full Remastered shader costs 0.7 ms more than a trivial one at the valley (1.63 against 0.96 ms). The "1.75x Classic" ratio per drawn object was the stall exposing each draw's shading latency, not shading cost.

The fix is perf item W1-9 (upload a dynamic buffer only when it changed, plus a per-object fitted geometry cache); it has not landed. Change log: `ITERATIONS.md` P1.

**GL debug context.** Debug context off: no measurable effect (within 0.1 to 0.3 ms on every row). The patch that turned it off is not landed; an opt-in debug context for Mesa's threaded GL stays a low-priority portability item.

## Update 2026-11-01: the upload fix landed

W1-9 landed as fb043ec with four smaller items (W1-3 uniform dedupe on every row, W1-2 the asynchronous observer probe, W1-4 one link per world program, W1-6 small CPU cleanups). A dynamic vertex buffer now uploads only when its bytes changed, and land-following objects keep a fitted copy per object and LOD level. At the valley the dynamic uploads fall from about 3,099 (2.1 MB) to 21 (364 KB) per frame, and the occlusion test's CPU time drops with them (3.9 to about 2.3 ms), since it reads the cached bounds.

Measured with the per-frame rings (4 ABBA pairs, 1080p, 4x MSAA, RTX 4060 Ti, no frame cap, observer at 1 in 16), the batch build against the tip: the dawn valley at view distance 4,000 goes from frame p50 32.9 ms (p95 97.8) to 12.8 (13.9), GPU frame 32.9 to 7.6, pass1 25.9 to 2.1, CPU work 18.8 to 12.6; Eden at 5,000 from 29.7 to 12.9 (GPU 29.8 to 12.7); sunrise_ridge 13.7 to 10.6; noon_fields 10.8 to 6.4. No view got slower. The real fix lands close to the stand-in's numbers (12.3 / 9.2 / 1.6 at the valley): the fit it keeps costs little once it runs only on a change. The long views are now CPU-bound at about 12.6 ms, under the 15 ms budget at p95 too, and the valley's GPU frame is under 9 ms; Eden's is not (12.7 ms, with 6.9 ms in the ground pass). The p95 drop is the observer probe (W1-2): it used to stall 1 frame in 16 for about 100 ms. That build also carried the exact shader gates of W1-5 (about 0.1 ms per long view), which are not landed yet. Change log: `ITERATIONS.md` P1.

## Update 2026-11-01: the budget moves to this card, and the occlusion range

The owner set the bar to 60 fps at a realistic 4 to 5 km view distance on this machine's RTX 4060 Ti, with the GPU and the CPU each under about 15 ms at p95 of the 16.7 ms frame. The 9 ms GPU figure was a stand-in for an RTX 3060 and is retired. The batch 1 tip (fb043ec) already meets the new bar on every standard view in its full bench: CPU work p95 at most 13.9 ms (Eden, valley 13.6), GPU frame p95 at most 13.8 ms (Eden; the wet town 11.2). Eden's ground pass (6.6 ms) is still the largest GPU block.

The occlusion test was the largest CPU block left after the upload fix: every drawn object, out to 4 to 5 km, was tested against the occlusion buffer for a handful of culls. W3-1b (9e5d74b) tests only within max(600 m, the shadow casters' reach), except occluders, which keep their test so the buffer is unchanged; far non-occluders are drawn and the depth test hides them. Alone it saves about 0.9 to 1.2 ms of CPU work on the long views (valley -0.86, sunrise -1.17, Eden -1.23 ms) and 0.5 ms at sea, with a few far-field pixels of difference in two views. The draw-list sorts (W3-11) are the next CPU block: together with W3-1 and W1-7 they take 3.3 to 3.7 ms off the valley and sunrise. The exact W1-5 shader gates c to h landed with it (not timed alone; byte-identical stills). Change log: `ITERATIONS.md` P1.
