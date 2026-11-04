# XP-shadows report (lane xp-shadows, base 243ef1e0)

Worktree D:\Arma_CWA_worktrees\sq-xp-shadows (uncommitted, matches items\XP-xp-shadows\patch.diff, sha256 334c5a22b1a5). Frozen dist dist\sq-xp-shadows-r1-rwdi (exe 23e96eb21b89). Build green, 0 new warnings; unit gate green ([ShadowXp],[ShadowMath],[SunShadows] 51 cases / 610 assertions; PoseidonTests 3531 of 3541 pass with only the known six external-data fails, 4 skipped; Core 823/827, 4 skipped; Foundation 475/475). Formatted with clang-format 22.1.8; 21.1.8 pending.

## What the tip does today (verified in code)
4 cascades at 2048 (2 camera-centred spheres, then 2 frustum slices), reaching only the shadow distance, which the view-distance resolver caps at 500 m (417 m on Abel at its 1500 m view). Past that, only the low-sun canopy map gives tree shadows. Every caster is drawn into all 4 cascades. The soft filter (PCSS) caps its penumbra in texels (48), so on the 42 cm far tier at low sun it smears whole forests into one soft blob.

## Switches (all default off; nothing set = the tip, byte for byte)
- POSEIDON_XP_SHADOW_TIER 0 low, 1 medium, 2 high (= tip), 3 ultra, 4 extreme
- POSEIDON_XP_SM_RES, _SM_REACH (metres cap; maps reach view x 5/18 up to it), _SM_CASCADES, _SM_OMNI, _SM_SPLIT, _SM_TAPS (soft filter taps per TAA frame), _SMCULL (per-cascade caster cull), _SM_TIGHT (minimal slice sphere), _SM_MINPX, _SMLOG (census line)
- Tiers: low 1024 / 3 cascades / 300 m; medium 2048 / 3 / 500 m; high = tip; ultra 4096 / 4 / up to 1000 m; extreme 4096 / 4 / up to 1600 m. Cull and tight on in every tier but high.

## Measured (same exe, runner jobs, all clean)
- Cull is pixel-identical: stills-cull-exact, 6 of 6 views IDENTICAL (0 px). A unit oracle also rasterises culled vs full cascades equal texel for texel. Its timing alone is not measured (the tip only has 30 to 370 casters).
- Ultra vs tip (quick-ultra, 0/54 foreign): CPU work / GPU frame valley +0.70 / +0.65 ms, broken_high +0.53 / +0.71, noon_fields +0.35 / +0.19, sinai +0.09 / +0.05. Worst p95 stays inside 15 ms both sides (broken_high 10.6 CPU, 10.2 GPU).
- Extreme vs low (quick-low-extreme, 0/52): valley +0.83 / +0.99, broken_high +1.52 / +2.00 (pass1 +1.45), sinai -0.13 / +0.11. broken_high extreme p95 11.7 CPU / 11.4 GPU: still inside budget.
- Low is the tip within noise (valley low 9.08 / 4.89 vs tip 9.03 / 5.01). Tip shadows are already cheap, so presets below High save nothing on this machine.
- Caster census at Eden 5 km view: tip 91 casters 0.2 ms collect; ultra 977 casters 0.5 ms; extreme 2098 casters 0.8 to 1.0 ms.

## Look (judged from stills; showcase\ has 1:1 A|B crops, left = tip, right = ultra)
- 03_broken_high: at a 5 km view, bushes along the road and the forest edge get shadows 500 to 1000 m out that the tip does not draw. Clearly better.
- 02_hill_1730: distant tree groups get crisp long shadows instead of faint blobs. Better.
- 01_valley_0515: the sunrise forest keeps the same darkness (mean luminance 46.7 vs 47.1) but shows individual long tree shadows instead of a smudge. Better. Isolation: 4096 alone (stills-res4096) and tight alone (stills-tight) each change little (valley 18k and 13k px over 2 against 138k for ultra); attribution jobs are queued (see below).
- Near views (forest_edge, lowsun_fp): 4096 sharpens shadow edges only.
- Motion is not judged: wider reach and tight spheres change how far tiers snap as the camera moves, and fewer or more filter taps change TAA noise. Owner clip needed before any of this ships.

## Proposed options scale
Low = Medium = High (keep High as the floor on this card; low/medium only for weaker GPUs, unmeasured there). High = tip. Ultra (4096, reach up to 1000 m) about +0.5 to +0.7 ms CPU and GPU on long views: recommended as the top preset for 4 to 5 km views. Extreme (1600 m) about +1.5 CPU / +2 GPU on Eden: optional. Cull: propose default on at every tier (pixel-identical), pending a timing row.

## Still in the queue when this report was written (results land in results\)
stills-medium, stills-low, stills-taps2, stills-taps8, stills-highfar-vs-ultra, stills-ultra-nocull (cull exactness at ultra), stills-restight-vs-ultra (attribution), and the one timed job quick-highfar (2048 + reach 1000 + tight + cull vs tip: the cheaper long-reach candidate). Read them before landing anything.

## Risks
Ultra/extreme raise caster count tenfold on Eden (CPU collect up to 1 ms); 4096 maps use 256 MB of depth array; tight spheres and reach are temporal changes (owner clip). The PCSS penumbra cap in texels makes softness depend on resolution: a design flaw worth fixing in metres.
