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
