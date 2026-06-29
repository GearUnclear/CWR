# Target-priority retuning: focus fire on officers, MGs, and immediate threats

*Teach AI groups to shoot the dangerous and decisive enemies first by reshaping the single cost function that already ranks every target.*

## Summary

Every AI group keeps a `_targetList` of known enemies. Once per think cycle it scores
each target with one number, `Target::subjectiveCost`, sorts the list by that number,
and hands the sorted list to `AssignTargets()`, which walks it top-down and commits
shooters to the most "expensive" targets first. That same score is also the gate a
*single* soldier uses to decide whether to switch fire (`TargetFire.cpp`).

Today the score is almost purely "how much vehicle value can kill me" — it barely
distinguishes a rifleman from a machine-gunner and does not know an officer from a
private. This mod biases `AIGroup::GetSubjectiveCost()` so groups prefer, in order:
enemy **leaders/officers**, **machine-gun / crew-served** weapons, **closer** and
**already-wounded** targets, and **de-prioritize** targets a friendly element is
already killing.

Observable result: in a firefight the enemy squad leader and the MG gunner get dropped
first instead of whichever rifleman happened to sort highest; wounded enemies get
finished rather than abandoned; and two friendly squads stop dogpiling one soldier while
ignoring the rest. All of this is visible in-game and tunable with a few constants.

## Why it's interesting

It is a high-leverage AI change with a tiny blast radius. One function, ~40 lines,
controls the priority of every AI engagement in the game. Because the sort key already
exists and is already consumed in two places, you get a dramatic, observable behavior
shift (focus-fire that *looks* tactically smart) without touching the targeting,
pathing, or fire-control machinery. It is also a pure gameplay/engine change — no
models, no textures, no new config classes required.

## Difficulty & prerequisites

**Intermediate.** You need to read C++ comfortably and understand the AI target/threat
data model, but the edit itself is localized. Prerequisites:

- A configured `win-x64-clang-rwdi` build (see `CLAUDE.md`).
- Demo game data to actually watch AI fight (Trident / in-game).

**No art assets are required.** No 3D models, no textures, no new PBOs. Everything is
C++ plus optional config-driven tuning of existing fields.

## Key files & entry points

All paths absolute under repo root `C:/dev/arma_CWR`.

- `engine/Poseidon/AI/AIGroupImpl.cpp`
  - `AIGroup::GetSubjectiveCost(Target*)` — **line 834**. The sort key. Primary edit site.
  - `AIGroup::GetDammagePerMinute(Target*)` — **line 806**. The threat term; tune alongside.
  - `BetterTarget()` comparator — **lines 411-416**: `sign(t1->subjectiveCost - t0->subjectiveCost)` (descending).
  - `QSort(_targetList ... BetterTarget)` — **line 688**, inside the per-frame "prepare subjective costs" loop (**lines 655-685**, score assigned at **line 672**).
  - `AIGroup::AssignTargets()` — **line 892**; consumes the sorted list at the loop starting **line 972** ("first targets in the list are the most dangerous").
- `engine/Poseidon/AI/AIGroup.hpp`
  - Declarations: `GetDammagePerMinute` (**line 711**), `GetSubjectiveCost` (**line 712**), `AssignTargets` (**line 713**), `Leader()` (**line 87**).
- `engine/Poseidon/AI/VehicleAI.hpp`
  - `struct Target` — **line 256**; fields `idExact` (284), `type` (265), `side` (263), `position` (258), `dammagePerMinute` (289), `subjectiveCost` (290).
  - `enum Rank` — **line 204**: `RankPrivate..RankColonel`, `RankUndefined=-1`, `NRanks`; `ClampRankIndex()` at 217.
- `engine/Poseidon/World/Entities/Infantry/Person.hpp`
  - `Person::GetRank()` — **line 46** (`_info._rank`); `Person::CommanderUnit()` — **line 38** (returns `_brain`).
- `engine/Poseidon/AI/EntityAI.hpp`
  - `EntityAI::CommanderUnit()` virtual — **line 1288** (base returns `nullptr`; overridden by `Person` and `Transport`).
- `engine/Poseidon/AI/AIUnit.hpp`
  - `GetPerson()` (239), `GetVehicle()` (243), `GetGroup()` (253), `IsSubgroupLeader()` (249), `Position()` (318).
- `engine/Poseidon/AI/EntityAIType.hpp`
  - `GetCost()` — **line 589**; `GetDammagePerMinute(...)` virtual — **line 649** (returns a `Threat` indexed by `VehicleKind`: `VSoft`/`VArmor`/`VAir`).
- `engine/Poseidon/World/Detection/TargetFire.cpp`
  - **Second consumer** of `subjectiveCost`: per-soldier fire switching uses
    `targetMinSubjCost = tgt->subjectiveCost * 0.5` (**line 1743**) and compares at **line 1840**.
- `engine/Poseidon/World/Entities/Weapons/Weapons.hpp`
  - `WeaponType::_weaponType` slot bitmask (`MaskSlotPrimary` etc., **lines 248-254**); `MuzzleType` carries fire-rate/magazine data used for the MG heuristic.

## How it works today

`GetSubjectiveCost()` (AIGroupImpl.cpp:834-878):

1. `baseCost = tar->type->GetCost()` — the static config value of the vehicle/soldier type.
2. If the target isn't an `EntityAI` or can't fire (`IsAbleToFire()`), it returns `baseCost` unchanged.
3. Otherwise it loops over **this group's own units** and, for each, asks
   `type->GetDammagePerMinute(dist2, 1.0)[vehKind]` — how fast the target could kill that
   unit. It converts that into `danger = unit->GetTimeToLive() / targetTimeToLive`,
   clamps to 10, scales by the *firing unit's* vehicle cost, and keeps the max.
4. Returns `baseCost + maxCost`.

So the score is essentially **target value + "how fast it can kill the scariest member
of my group."** Notable gaps:

- **No leadership term.** An enemy officer and a private with the same rifle score identically.
- **MGs barely stand out.** They are captured only indirectly through `GetDammagePerMinute`'s
  anti-soft term; a rifleman at the same range scores nearly the same.
- **No proximity term.** Distance only enters through the threat falloff, not as a direct
  "closer = more urgent" bias.
- **No "already wounded" term.** A near-dead enemy and a fresh one score the same, so the
  group may walk away from an almost-free kill.
- **No cross-group coordination.** Each group scores in isolation; two squads happily focus
  the same target. Within a single group `dammagePerMinute`/`_assignTarget` already track
  commitment, but there is no shared field across groups (`AITargetInfo` in
  `AICenter.hpp:17` stores side/type/position but **no per-group engagement marker**).

The list is sorted descending by this score (`BetterTarget`, line 411) and consumed in
order by `AssignTargets()` (line 972). The same score also gates individual soldiers'
fire-switching in `TargetFire.cpp` — so any change has **two** observable consumers.

## Implementation approach

All edits are additive multipliers/bonuses layered on the existing `baseCost + maxCost`
return so the existing threat logic stays intact and tunable.

1. **Introduce tuning constants** near the top of `GetSubjectiveCost()` (or as
   `static constexpr float` above it, mirroring the existing `COEF_TTL` / `COEF_TIMEOUT`
   macros at AIGroupImpl.cpp:880-882). Suggested:
   `LEADER_BONUS`, `RANK_SCALE`, `MG_WEIGHT`, `PROX_WEIGHT`, `PROX_FULL_DIST`,
   `WOUNDED_WEIGHT`, `FRIENDLY_ENGAGED_PENALTY`.

2. **Leader / officer bonus.** In the existing block where `enemy = dyn_cast<EntityAI>(obj)`
   is already computed (line 840), derive the target's command status:
   - `AIUnit *tCmd = enemy->CommanderUnit();` (EntityAI.hpp:1288; `Person` overrides it).
   - If `tCmd && tCmd->GetGroup() && tCmd->GetGroup()->Leader() == tCmd` → group leader →
     add `LEADER_BONUS`. (`Leader()` at AIGroup.hpp:87; `GetGroup()` at AIUnit.hpp:253.)
   - Add a rank ramp: `Person *p = dyn_cast<Person>(enemy); if (p) bonus += RANK_SCALE *
     ClampRankIndex(p->GetRank());` (rank enum/clamp at VehicleAI.hpp:204/217; `GetRank()`
     at Person.hpp:46). This degrades gracefully: vehicles (no `Person`) just skip it.

3. **MG / crew-served weight.** Prefer the data already computed rather than a new config
   flag. The cleanest, art-free signal is the target type's **anti-soft** damage rate:
   `Threat thr = type->GetDammagePerMinute(Square(PROX_FULL_DIST), 1.0); cost += MG_WEIGHT *
   thr[VSoft];` — MGs and crew-served guns have by far the highest anti-infantry DPM, so
   this naturally elevates them without enumerating weapons. *Optional, more explicit*
   variant: walk the live entity's magazine slots
   (`enemy->NMagazineSlots()` / `GetMagazineSlot(i)._weapon->_weaponType`, pattern at
   VehicleAI.cpp:1279-1281) and bump the bonus when a slot's `_muzzle` indicates a
   high-rate-of-fire belt weapon. Start with the threat-based version; it requires no new
   data and no config.

4. **Proximity bias.** Use distance to the group leader (the decision-maker):
   `float d2 = Leader() ? tar->position.Distance2(Leader()->Position()) : 0;`
   then add `PROX_WEIGHT * (1 - saturate(sqrt(d2)/PROX_FULL_DIST))`. Keep the weight small
   so it only breaks ties between comparable threats; do not let it override the armor/threat
   term (you don't want infantry charging a tank because it's near).

5. **Already-wounded bias.** Read the target entity's damage and add
   `WOUNDED_WEIGHT * damageFraction` so near-dead enemies get finished. Use the entity's
   existing damage accessor (the same value `IsDammageDestroyed()` is derived from — verify
   the getter name on `EntityAI` when implementing; if a normalized 0..1 getter isn't
   exposed, compute from current vs. max armor via `GetArmor()`).

6. **De-prioritize what a friendly group already engages.** *Hardest term — be honest about
   the data gap.* Within the **same** group this is already implicit (`GetDammagePerMinute`
   sums committed shooters; `_assignTarget[]` records coverage). For **cross-group**
   coordination there is no existing shared marker — `AITargetInfo` (AICenter.hpp:17) does
   not store "who is shooting this." Two options:
   - *Minimal:* skip cross-group for v1; rely on the within-group commitment already encoded
     in `GetDammagePerMinute()` (line 806) which you can additionally subtract here.
   - *Full:* add an engagement timestamp/owner to `AITargetInfo` (and serialize it like the
     other fields), set it from `AssignTargets()` when a group commits, and subtract
     `FRIENDLY_ENGAGED_PENALTY` here when a *different* group's stamp is fresh. This is a
     larger, networked change (the center DB is shared/synced) and should be a follow-up.

7. **Keep the return shape.** Final line becomes
   `return baseCost + maxCost + leaderBonus + mgWeight + proxBonus + woundedBonus - friendlyPenalty;`
   Preserve the early-out `return baseCost;` for non-firing targets (line 843) unless you
   *want* leaders who are out of ammo still prioritized (a deliberate design choice — call it out).

8. **Sanity-check the second consumer.** `TargetFire.cpp:1743` uses
   `subjectiveCost * 0.5` as a fire-switch threshold; inflating costs globally raises that
   bar too. Keep bonuses on the same order of magnitude as existing `baseCost`/`maxCost`
   (tens–hundreds, not 1e5) so you don't accidentally make soldiers refuse to switch off a
   high-scored-but-unreachable target.

## Config / data / SQF touchpoints

- **Pure-engine path (recommended first):** all weights are `constexpr` in
  `AIGroupImpl.cpp`; no config edits. Fastest to iterate, fully self-contained.
- **Config-tunable path (optional):** read the weights once from a `class CfgAISettings`
  style param block at load via the existing `ParamFile` / `ParamEntry` plumbing
  (`engine/Poseidon/IO`), so mission/mod makers can retune without recompiling. Reuses
  existing config infrastructure — no new asset types.
- **Rank source:** ranks already come from unit config / mission editor (`Person::_info._rank`);
  the officer bonus rides on data missions already set. No new data authoring required.
- **SQF observability:** `rank`, `leader`, and `getDammage` are already scriptable, so a
  test mission can assert "the officer died before the riflemen" from script (see Testing).

## Risks & gotchas

- **Two consumers, not one.** `subjectiveCost` drives both group `AssignTargets()` and the
  per-soldier fire switch in `TargetFire.cpp:1743/1840`. Validate both: a target the *group*
  prioritizes must not become one a *soldier* refuses to abandon.
- **Magnitude discipline.** The disabled-target sentinel is `-1e10`/`-1e5` (AIGroupImpl.cpp:664/683).
  Keep positive bonuses far below those magnitudes so ordering of valid vs. invalid targets is unaffected.
- **Don't let proximity beat lethality.** A strong proximity term can make infantry ignore a
  distant tank for a near rifleman. Keep `PROX_WEIGHT` a tiebreaker.
- **`Target` is bitwise-serialized & list-managed.** `Target::subjectiveCost`/`dammagePerMinute`
  are serialized (`Target.cpp:310-311`). You are *recomputing* these fields per frame, not adding
  new ones, so save format is unchanged. If you add a field to `AITargetInfo` for the cross-group
  term (step 6 Full), follow the existing `Serialize()` pattern and be mindful of the
  repo's `ClassIsMovableZeroed` `memcpy`/`memmove` movable-type convention — don't add a
  non-trivially-relocatable member to a struct that's bulk-moved.
- **Heavy intentional warning suppression** (per `CLAUDE.md`): the AI files contain
  deliberately incomplete `switch` statements and tautological compares. Don't "tidy" code
  you didn't change; match the surrounding style.
- **`__FILE__` asymmetry / PCH:** irrelevant to this edit, but note PCH is per-target PRIVATE —
  if you add a new header include, rebuild the configured dir rather than assuming the PCH covers it.
- **FileSize target:** `AIGroupImpl.cpp` is already large; the `FileSize` target warns >3000
  and errors >5000 lines. Keep the change compact (it easily fits as ~40 added lines); push the
  optional config-loader into a small helper if it bloats the file.

## Testing

1. **Build:** `cmake --build build/win-x64-clang-rwdi` (engine link target `Poseidon.lib`).
2. **Unit (Catch2, `tests/unit/`):** the AI scoring is engine-internal. Add a focused test
   to the `PoseidonTests` (or `PoseidonServerTests`) suite that constructs a small group with
   two known `Target`s — one flagged as a leader / higher rank, one a private — and asserts
   `GetSubjectiveCost(leader) > GetSubjectiveCost(private)` after your change, and similarly
   for an MG-class vs rifle-class type and for wounded vs healthy. Run:
   `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonServerTests --output-on-failure`.
   (If no existing AI-group fixture exists, this may require a small test harness — note that
   as part of the slice.)
3. **Integration (Trident, `tests/integration/`):** author an SQF scenario placing a squad
   with one officer + one MG gunner + riflemen against an AI group; after first contact,
   assert via script that the officer and MG gunner are dead/wounded before the riflemen
   (`getDammage`, `alive`, `rank`). Build the runner then:
   `cargo build --manifest-path engine/Trident/Cargo.toml` →
   `tri test -j6 --retries 2 tests/integration`. Needs `OFPR_GAME_DIR`/`OFPR_DATA_DIR` set
   per `.trident.env`.
4. **In-game eyeball:** run the `win-x64-clang-rwdi` client on a small editor mission;
   temporarily flip the `DIAGS` macro region in `AssignTargets()` (lines 1007-1020 already
   log per-target `subjectiveCost`/`dammagePerMinute`) to confirm the officer/MG sort to the
   top of `_targetList`. Watch that the enemy leader and gunner drop first.

## Scope estimate

- **Minimal first slice (~half a day, fastest visible win):** steps 1-2 only — leader +
  rank bonus in `GetSubjectiveCost()`, constants hardcoded. This alone produces the most
  noticeable behavior ("they shot my officer first"), is ~15 lines, touches one function,
  and is trivially A/B-tested with the existing `DIAGS` logging.
- **Full gameplay pass (1-2 days):** add MG weight (step 3), proximity (4), wounded (5),
  plus Catch2 + Trident coverage.
- **Stretch (separate PR, +1-2 days):** cross-group de-confliction (step 6 Full) — requires
  extending and serializing `AITargetInfo` in the shared/networked `AICenter`; treat as its
  own change with MP testing.
