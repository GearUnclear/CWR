# Rendering audit of CWR-CE main

Reviewed all 61 assigned commits, hydrated and inspected commit patches, and compared current local implementation under `D:/Arma_CWA/arma_CWR`. Coverage and individual evidence: [rendering-coverage.json](rendering-coverage.json). Cached patches are rendering-patch-*.txt. This is read-only source review; upstream benchmark claims were not reproduced and no ports/tests were run.

## Small correctness tranche worth taking first

1. **ed6119f9f65244231ed0cc393e5e238bc98d3c5f — detail texture reload after remount.** Local EngineGL33_Lifecycle.cpp:510-524 preserves the engine and drops cached textures, but `_detail`, `_specular`, `_grass`, `_waterBump` stay owned. TextureBankGL33_Cache.cpp:214-218 returns immediately if `_detail` exists, so a changed mod's CfgDetailTextures can retain the previous set. Port ReleaseDetailTextures + remount call and integration counter/assertion. Preserve the local PortraitService release hook. Low risk; inspect changed mod names, not just reload count, if strengthening the test.
2. **ea7602ced6fc83d81fd184f643d3479264420bc3 — initialize dummy texture members.** Local TextureDummy.hpp:17-21 has uninitialized scalar members. TextureDummy::Init can return without assigning `_w/_h`; constructor only invokes Init. The poisoned-storage test is useful and guards the exact failure. No dependencies.
3. **f30ba0a9a07f0f8f5ff72ec1f0be22b55942b3ad — inclusive allocation maximum.** Local JimboAllocator.cpp:64 rejects `size >= kMaxSingleAllocation`; header limit is 256MiB. Local Extreme detail already maps to 3.125m (GraphicsApply.cpp:28), so an 8192² float heightmap hits exactly that limit. Change only `>=` to `>`; preserve local process-wide budget accounting. Extreme can still have substantial peak allocation, but the current advertised option should not fail on the inclusive boundary.
4. **6f0d7fecf7e85b9c1858c4fb445139ea9c8c5782 — release instancing buffers (adapt subset).** Local Shaders.cpp:841 creates `s_worldUBO`; DeinitVertexShaders:846-872 only deletes `s_vsUBO`. Delete/reset world UBO and invalidate bindings now. CE's local light index/parameter UBO portions wait until local-light instancing exists. Verify repeated hard reset/shutdown/re-init, including recycled IDs.
5. **7ca93a57fe6fec150140d5f3f26b630721387fde — destructive deformation exclusion (adapt).** Local SceneDraw.cpp:1555 and following run-member predicate admit static non-tree objects without asking whether destruction deforms their shared shape. Add the CE predicate to both head and members and its test. Requires adaptation because our existing batching predicate predates GPU clipping/local-light changes; no need to import those features first.

**Do not take 08e67e93 blindly:** current Vertex.hpp:139 already initializes `_minMaxDirty=true` and Vertex.cpp copies it. This is stronger than CE's constructor false policy for a fresh table populated through setters; importing their reset would change that policy.

## Performance in escalating scope

**First:** e4a18a10b048c72a6a0b14f199cc8003a8bca69d + d8870c1170d97a7eb3f6ea71b8ead5bb28f1a107 (`Ref<T>` noexcept moves and focused tests). Local Pointers.hpp:275-320 only has copying. Transfer ownership without atomic increments; self move guarded and destination old owner released. CE claims 23% improvement in object sorting, not total frame rate.

**Also small:** 406aaff09eb655d7dc1263dacf72ed31691d1636 (`VBSmallDiscardable` is static) corrects local VertexBuffer.cpp:119's unnecessary dynamic classification. Afterwards 03413fe91ca3c2fc0087c1f136fec4d7a39b4092 can simplify legacy buffer enums; it reports no separate measurable performance gain. Preserve local shadow deformation/private-buffer fixes.

**State/UBO bundle, exact dependency sequence:**

1. f5d08d3ab52ac32945ea20ca3e18aed69166128c — cache generic uniform buffer bindings.
2. 4ffc0511972a704e94918d26939f3946bd032e2b — sampling texture binds needn't select a cached active unit.
3. 8a338067693e8f728ff1ac429b45aadaf690b27b — VS snapshot and world slot-0 dedupe; defer world upload until actual non-instanced EmitDraw.
4. e19872ca7a4699a3c2bc710d87aafdaa4233dc29 — avoid repeated polygon offset changes.
5. 780f50c1375bfc119cd8d1296da33150f747e640 — mandatory VS snapshot reset when recreating UBO.
6. 4bc72d42829fa5bb56387ce01d545d8c895797a0 and 6db0d1aa16e7e1c8dd72f81df2777148102ffc53 — bind-cache mocks and pure pipeline/upload comparison helper tests.
7. bc3014a96eee37d39b0c083322b9f4caffacbb59 when GPU clipping lands — raw SetLandClipParams UBO bind must update cache.

Current local PS constants already dedupe; VS and world don't (Shaders.cpp:782,1092,1103). Main risk is stale cache after raw GL writes, deleted/recreated names and external rendering; port as a verified unit with reset/remount coverage. Preserve portrait/gamma/shadow rendering invalidation. CE reports fewer GL calls (18K to 12.4K for UBO changes and 12.4K to 11.5K after polygon offset), without a reliable isolated end-to-end frame-time gain. f1b832fcd28a52a9a71f4234bba6770c6c7db5cf adds Apple-specific buffer orphaning before updates; defer until macOS is an actual target and measure on Apple hardware.

**Sorting/bounds bundle:** e4a18a10+d8870c11, then 8f3de01c8a7b7a24d7bacb2f74491abc40066125 + 816e615ddbc759dc17afe8080dccdec5af306293 (stable descending radix and memoized keys), then 775a26259b59a2ec6653c457e705f4fd522fdaf0 + c8229780a404e7ba49d86e33e8fc38165834f6ff (static land-conforming bounds cache). 90f90ec4 is the clipping extraction used by later GPU path. Include 9a298d09fd225c7019ce6195dc012950ece4833a with later GPU path to prevent nested bounds queries undoing active deformation. Bounds invalidation must cover movement, relevant terrain changes and damage/recovery; local Object.cpp:431 always animates today. Stable equal-depth order matters for alpha blending, so never import the initial radix implementation alone.

cc4bd699d14d9214a75a694106eea823f0122d86 replaces opaque sort with bucketing and filters surface sort input. Worth measuring after the GPU clipping commit removes the local fog-band requirement (SceneDraw.cpp:1566-1580). Don't take alone while those distance constraints still govern batching. CE reports sort costs dropping from 7%/3% of frame time to 0.7%/0.2%; watch local decal/shadow ordering and shared mutable epoch state if rendering becomes concurrent.

## Larger rendering modernization: valuable, keep as one project

Sequence from assigned main commits:

1. bounds/sorting prerequisites above, state/UBO bundle as applicable.
2. ef6534b1ea9c0ffa62a0ce94aa997c4f4e3cb699 — GPU land clipping, selective local-light exclusion and remove fog distance constraint.
3. f845a7318cfb29f349b3446f00362292bcc620a1 — selected LOD clip eligibility.
4. 315733ab32b2e06d9be31274fcf3aa396abcba00 — invalidate uploaded heightmap on parameter changes.
5. b04ae1ae54983672f6473215f9200b9521d86126 + 9a02f8eca8887bbb589797110b613fb522b9b733 — clip decision coverage and ForestPlain clipping mode.
6. 7ca93a57 + 9a298d09 + a918a0232a7642377bafb31553a2a14951166967 + bc3014a9 — deformation guards, active bounds safety, heightmap cleanup and bind coherence.
7. c551e7838816ad63302e261c22e96dd6d1613068 — per-instance local-light indices plus per-view light parameter buffers; include helper tests in 3541affa assigned to gameplay and 6f0d7fec cleanup.
8. cc4bd699 — bucketing after fog-band removal.
9. 8c61786ddba773dc13fc9ddb1f5e0aaa5ac93a5c — proxy weapon/building gun clip state reset.
10. 82147189e8f7ca053414f99aa7fb0a04a15f586c — instanced terrain texture arrays and heightmap; c1f76798 debug A/B switch; do not import redundant b534fd79 count assertions.
11. 825b3b15d2c84bf3047eb3f3d4e62022a83f8c09 — shader fragments.
12. ad332672ee252426d271ad8db359c969831bd4c2 then d70fdbd56ed46de6bcca90217f4158bf2ec10fbc — water and move all draws to one instance per terrain segment.
13. 618bdcd41b987e79718fa943dfe3dcbfd01ec014 — sampler rebind correction.
14. 235e9181e0fbe067be5e5433792ace1340d779c1 — shader catalog/source extraction.
15. 06bd4a1a0b486bc35c280df7a792bde755c4ab30 + 8d878a0898d26fdbd7f6d2e245b7049a84cc8d31 (gameplay inventory) — map-edge open sea and hardened segment indices.
16. 3b5849e397123ba915a508c119916d7e3bd4a016 + 3ffb0338c0c7c62d1d4c8bfb9a0dd37923dedebf — hash catalog and link instanced shader pairs.
17. b67bf3bd623a52d62456c3ef65f7e0e0163dcd9b — enable default only after local A/B validation.

Terrain is absent locally (`EngineGL33_Terrain.cpp`, `LandClipInVS`, `DrawGroundInstanced` absent); this is a substantive port. Claimed benchmarks are compelling: vegetation path Nogova 37.7ms to 12.5ms; terrain at 5km view distance 8.6ms to 6.2ms, draw calls 2886 to 1131. They are upstream authors' measurements. Texture arrays pre-load all mipmaps and add VRAM; test mixed format/dimension mod textures, array layer limits, roads/decals, animated water, remounts, extreme/detail changes, ForestPlain, selected LODs, destruction, lights, proxy weapons and out-of-map sea. Retain scalar A/B path while validating and do not enable default on the initial implementation alone.

## Already covered locally

16 assigned commits are already semantically covered: gamma shader/default/migration/value UI/snapping, custom profile FOV, HUD/world width, optic stretching, alpha vehicle/cloud order, sun/weather initialization, compressed texture interpolation, shape selections/sections and font lifetime, rendered debrief extraction, extreme terrain tier and vertex dirty state. Per-commit local evidence in coverage JSON. No need to re-import under their CE SHAs.
