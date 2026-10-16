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
| 2 | The five `reviewable_forces` fixes applied onto `occupier-forces` (birth budget within one step, abstract presence against the current bubble, same-tick captures and k, the pool floor rounding, cleared bindings across the v5 migration) per `tmp/of-reviewable-forces-review.md` | Started 2026-10-16 |
| 3 | The comparison video's findings on both branches: A traffic held to the live-body budget; B the `AIUnitImpl.cpp(748) _state == InCargo` check near births and dismounts (#78 family); C no strike opener on an empty base with no fighters reported, refunds capped at poolStart; D the arrival-rule ETA quotes the road route, not the straight line. Then video 4 on the fixed tree (a protagonist taking six points over two hours of game time, stability sparklines) | B and D started 2026-10-16; A and C start when round 2 lands (shared files) |
| 4 | Owner design decisions applied: generated bases step 1 (zones only, stored, behind a switch, the density rule) and step 2 (one outpost layout); vehicle dealer discoverability (#99); journal lines for director actions (#98) | Waiting on the decisions below |
| 5 | The second human playtest on Malden from the refreshed crib sheet (`tmp/of-owner-summary.md`) | After round 4 |

## Decisions the owner still has to make (round 4 waits on them)

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
