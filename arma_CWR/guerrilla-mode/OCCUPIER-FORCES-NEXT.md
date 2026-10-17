# Occupier forces: the path forward (check here before proceeding)

Owner-approved plan of 2026-10-16 for the work after PR #95. Every agent and
session working on the occupier forces reads this file first and updates the
round status when something lands. Rules for agents are in
`D:\Arma_CWA\tmp\of-agent-common-rules.md` (branch `occupier-forces`, commit by
path, no new branches, no em dashes, gate with the forces set before a push).
The running log is `D:\Arma_CWA\tmp\occupier-forces-progress.md`.

The loop: a round is a set of Opus agents with disjoint files, committed by
path, gated (`tmp/of-gate.ps1` with every `guerrilla_forces_*` test, the
`guerrilla_bases_detect_*` tests and `ai_group_id_churn` as `-ExtraTests`),
pushed only green, and ended by a commentated video from a real engine trace
whose ranked findings seed the next round. The loop runs until a video shows
no finding that contradicts `OCCUPIER-FORCES.md` or `OFFSCREEN-CONTRACT.md`.

## Status

| Round | Content | Status |
|---|---|---|
| 1 | Video 1 findings: arrival census log (fe228a0), group-id allocator leak (2921bcf), air re-plan band flicker (197a7d3); owner decision "a more playable rate" (acf2311) | Landed, gates 16 and 17 green, pushed |
| 2 | The five `reviewable_forces` fixes applied onto `occupier-forces` (birth budget within one step, abstract presence against the current bubble, same-tick captures and k, the pool floor rounding, cleared bindings across the v5 migration) per `tmp/of-reviewable-forces-review.md` | Landed 6eefc87 (2026-10-16) |
| 3 | The comparison video's findings on both branches: A traffic held to the live-body budget; B the `AIUnitImpl.cpp(748) _state == InCargo` check near births and dismounts (#78 family); C no strike opener on an empty base with no fighters reported, refunds capped at poolStart; D the arrival-rule ETA quotes the road route, not the straight line. Then video 4 on the fixed tree (a protagonist taking six points over two hours of game time, stability sparklines) | Landed 2026-10-16: B 973a36d (the real cause: NativeMoveIn never set InCargo, Traffic riders tripped DoGetOut), D 3f4972b and 98cba8a (RoadQuoteCache, forcesRoadFactor 1.5), C 3c9947b (opener needs a reported defender, BandHold per launch, pool ceiling with wave headroom and dropped), A 797173a (Traffic and civilians.sqs held to the live-AI budget; the video overshoot was the town civilians). Then c8fe6a8 (base_retake pins its source), df9457e (owner: the excess over the ceiling banks into garrisons). Gate 19 green (756 unit, 63/63 tri), pushed; video 4 in flight |
| 3.5 | Video 4 findings: E re-arm after a rest (forcesRearmHeat 40), the strike keeps STRONG after both debits, push-outs under the arrival rule, honest HQ and launch lines (1ae2dc9); F arrival-live columns born with carrier, escort and crews (06414a4); G the group budget counts only groups with a living member, idle empty groups released after 60 s (0c43877). Then video 5 | Landed 2026-10-16; gate 20 green (767 unit, 67/67 tri), pushed. Video 5 (tmp/of-director-video-5, 4:34, one strict-mode run of 3 h 36 min game time, 0 errors, 22 operations, 9 re-arms all attacked within one token interval, 0 idle minutes while STRONG with a candidate): no finding contradicts the design or the contract. The loop stops here; what remains is tuning (below) |
| 4a | Owner approved decisions 1 to 15 on 2026-10-16. Phase 1: A the repel ladder, massing, value-weighted candidates, jitter, wave strikes under the arrival rule, conservation for generated bases (d5fa57b); B break-off below a third, hulls leave after the infantry, one road factor (16acfd5); C the chooser takes the cheapest package meeting the ETA, air only across water, past forcesAirEta or with fighters reported (d90a942); D spent hold groups released, the shakedown pair under the budget (64d447a); E generated bases step 1, zones only, stored (811c5c3, 2943711); F PR #213 (the Remaster-only runtime gate, stacked on #95). Phase 2: H the outpost layout props (0d5e73a); G the wave surge, the band on strength, stored spare hulls (7765904). Also 4d0cc45, an upstream centre radio-channel defect the column break-off test exposed | Gate 21: 820 unit cases, human suite OK, tri 80 of 83 with the three reds resolved afterwards (two test premises on the new poolStart 48, and the LoBo helicopter drop error fixed in e371a68 and verified 3 of 3 on a shadow); the strike band sequence is held out. Also e371a68 (the Remaster's drop rejected the two-element position every LoBo helicopter engine script uses; 1.99 accepted it). Handed off 2026-10-17: tmp/occupier-forces-handoff-2.md; the next agent runs gate 22 first, then phase 3 (the eight-lever consolidation) and video 6 |
| 4b | Phase 3, the eight levers with four presets and `forcesDifficulty`, old keys as warned overrides, hull refunds through RefundHull (a5b40f0); the strike band sequence back (b97d323: its observers face the squad); generated bases step 3, clutter-free sites, per-kind layouts, minor posts as fallback, Regina covered (e9698d8). Then video 6 (tmp/of-director-video-6, two defended towns, 3 h 34 min game time) | Landed 2026-10-17; gate 22 (5fbe95c) 82/82, gate 23 832 unit and 83/83 tri, pushed. Video 6 found five contradictions: a carrier born 297 m out never dismounted (6.5); the band stepped mid-booking (5.6, D15); the Abel 60 pool figures in 5.6, 6.3, 16 (shipped 48); the strike line prints the strength band (7.3); the reinforce settle line says retaken (6.6). Owner calls: may one ground launch take a STRONG pool under forcesPoolFloor; the minor-kind rule; income from minor posts; helipads as air origins |
| 5 | Video 6 findings: A the stuck carrier and the CounterAttack top-up line (CounterAttack*); B the band stepped once per booking, the strike and reinforce lines, the doc numbers (OccupierForces*, OCCUPIER-FORCES.md). Then gate 24 and video 7 | In flight |
| 4 | Owner design decisions applied: generated bases step 1 (zones only, stored, behind a switch, the density rule) and step 2 (one outpost layout); vehicle dealer discoverability (#99); journal lines for director actions (#98) | Waiting on the decisions below |
| 5 | The second human playtest on Malden from the refreshed crib sheet (`tmp/of-owner-summary.md`) | After round 4 |

## Tuning questions from video 5 (owner calls, not defects)

1. On a 60 pool a director-bought town strike cannot pass the keep-STRONG rule outside wave headroom (ground 11 plus helicopter leaves at most 31 of 60; STRONG needs 36): accept, scale strike prices with poolStart, or allow a strike that keeps STRETCHED.
2. A lone survivor holds an operation LIVE until the 1200 s live timeout (op 12 and op 15 sat 47 m from Arudy for 18 minutes each): withdraw when living strength falls below a share of the launch while the town is still held.
3. After a wave the ceiling mostly drops (9 dropped, 1 banked, every base at garrisonMax): let the wave surplus decay instead, or raise garrisonMax at hubs.
4. Insertions demote the band while their airframe is out (two Mi17s took 52 to 31): extend the keep-STRONG rule to insertions, or accept.
5. A wave strike on the HQ walks 41 minutes (wave strikes are exempt from the arrival rule by design): keep or lift the exemption.
6. capture.sqs pins dead hold groups (GM_HOLD_GRPS, GM_cOld) so they are never released; harmless to the budget since 0c43877 but they hold ids: deleteGroup them when the zone is lost.

## Decisions the owner made on 2026-10-16 (all of 1 to 15 in the handoff list approved; the base rules below are now the implemented ones)

1. The base density rule: every town within 2.5 km of a ground base on its
   landmass, at least three sources per landmass, bases 1.6 km apart, or
   something looser (`tmp/of-generated-bases-design.md` section 2).
2. Conservation (generated garrisons subtracted from poolStart: more presence
   at the same strength) or a stronger occupier.
3. Whether minor posts (checkpoints, coastal posts, helipads, radar sites)
   count toward the win.
4. Whether generated bases show on the map from the start or are discovered.
5. Whether the detected outposts are seeded at runtime (reverses a #76 rule).
6. Whether the Remaster-only runtime gate and the CLAUDE.md wording from
   `reviewable_forces` go up as their own PR after #95 (needs a branch).

## Ownership map for the rounds

- Round 3 A: `Traffic.cpp`, the `ZoneRegistry` census and `budgetLiveBodies`,
  a tri test that parks 120 bodies and asserts traffic stops spawning.
- Round 3 B: `engine/Poseidon/AI/**` (the InCargo check, the seat state
  machine); `CounterAttack.cpp` read-only, hunks reported not made.
- Round 3 C: `OccupierForces.{hpp,cpp}` strike buying and the refund path,
  unit cases.
- Round 3 D: `ForcePackages.{hpp,cpp}` quote, `RoadRoute`, a unit case on
  Malden distances.
- Round 4: a new `BaseGenerate.{hpp,cpp}` plus `ZoneRegistry` seeding;
  `Market.cpp` and `market.sqs`; `Journal` and the director event hooks.

## Reference material

- Assessments: `tmp/of-abstract-director-assessment.md` (travel is already
  abstract), `tmp/of-generated-bases-design.md` and `-engine.md`,
  `tmp/of-reviewable-forces-review.md`.
- Videos and traces: `tmp/of-director-video` (round 1),
  `tmp/of-director-video-2` (the stopped round 2 run), `tmp/of-director-video-3`
  (the branch comparison; its stats and cues are the template for video 4).
- Issues: #95 (the PR), #97 (follow-ups), #98 (journal), #99 (vehicle dealer),
  #76 (occupation network), #78 (InCargo), #94 (HQ respawn).
