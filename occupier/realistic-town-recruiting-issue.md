# Guerrilla: optional realistic town recruiting with upfront hire fees and zero-default hourly wages

Destination: https://github.com/GearUnclear/CWR/issues

## Problem and intended behavior

Recruiting currently spends a global manpower reserve at the Camp. Add an optional **Realistic** new-game mode that hires local volunteers from owned, supportive towns for one-time treasury payments. Traditional remains the default.

**Hourly salary defaults silently to zero.** At zero there are no wage labels, deductions, warnings or accumulated debt. Enable wages only if the user later requests it for balance; never automatically enable them to compensate for failed tuning.

Target the full official **CWA Remaster** and supported island/faction packs. This is planned work: the source-grounded design and illustrative code below are not an implemented or playtested feature.

## Initial tuning (provisional)

| Setting | Proposed value |
|---|---:|
| Required town | Owned CITY, support >=60, calm and free of effective enemy occupation |
| Rifleman hire | 30 R; skill 0.35 |
| Experienced fighter hire | 60 R; skill 0.60; equipment subject to existing unlocks |
| Shared local pool | Seed 2 once; capacity 3 |
| Replenishment | 1 per 600 eligible simulation seconds |
| Local purchase | Alive player on foot within 150 m |
| Owned-town trust recovery | 5 support points per 600 qualifying seconds, capped at hiring threshold |
| Salary | 0 R/hour |

Use the existing R treasury. Keep existing income and training prices; disable HR in Realistic. Distributed waiting pools are not an active-fighter cap. Measure the mild-rebalance target on small and large islands before treating these values as settled.

## Delivery checklist

- [ ] Add saved Traditional/Realistic selection, defaulting to Traditional and explaining islands without usable CITYs.
- [ ] Resolve first-town organizing and the companion-loss opening; exposure does not clear the standing undercover flag.
- [ ] Implement local persistent supply, distinct supply/purchase/recovery predicates and fresh side-aware presence checks.
- [ ] Implement bounded, deduplicated request handling and a failure-safe synchronous purchase: successful spawn and verified equipment before any debit or candidate consumption.
- [ ] Add narrow owned-town goodwill recovery through the existing support helper.
- [ ] Preserve town fractions, initialization and hired identities across travel, overflow groups and save/load, without duplicated recruits or reseeding.
- [ ] Implement invisible zero wages, saved prospective developer tuning and tested nonzero prorated billing.
- [ ] Make shared training monotonic and traditional failed hires charge nothing.
- [ ] Complete mode-specific menus, blocked reasons, objectives, journal and manual.
- [ ] Exercise full-Remaster behavioral/failure/persistence checks and paired opening/economy playtests, with wages kept at zero.

## Related work

- [#79 D1: first full-island playthrough](https://github.com/GearUnclear/CWR/issues/79): the player remains excluded from capture presence even after exposure; resolve the opening deliberately.
- [#72: town security](https://github.com/GearUnclear/CWR/issues/72): the eventual defended-town opening must work, not only today's undefended neutral towns.

Canonical local plan: `arma_CWR/guerrilla-mode/REALISTIC-RECRUITING.md`.
Roadmap item: `realistic-town-recruiting`. The complete rules and implementation examples are reproduced below so the issue is usable before the new plan file is committed.

<details>
<summary>Detailed rules, ownership map, implementation examples and acceptance</summary>

## Player contract

New-game selection: **Recruiting: Traditional / Realistic**. Traditional remains
the default. The choice is saved and fixed for that campaign. Direct mission
launches without a selection use Traditional. Existing campaigns continue their
saved system; converting them is outside this delivery because running SQS
scripts restore their serialized text, not newly installed script sources.

Realistic recruits come from resistance-owned CITY zones with support >=60 and
no civilian panic. The player visits the town, hires a local volunteer for a
one-time treasury payment, and the fighter joins a player-controlled group.
Both hire types use one shared town pool. Hiring itself does not reduce support.
Falling goodwill or changed ownership stops new hiring, not existing fighters'
service. These are abstract local volunteers, not converted ambient civilians.

Use the existing treasury and its current R label. Dollar conversion is separate
work. The public option can be called Realistic while its description explicitly
says "Hire local volunteers in friendly towns for a one-time fee."

## Initial tuning, subject to gameplay acceptance

| Parameter | Default |
|---|---:|
| Minimum support | 60/100 |
| Rifleman fee | 30 R |
| Experienced fighter fee | 60 R |
| Initial candidates on first eligibility | 2 |
| Local waiting pool capacity | 3 |
| Replenishment | 1 per 600 eligible simulation seconds |
| Rifleman starting skill | 0.35 |
| Experienced fighter starting skill | 0.60 |
| Training price | 30 R |
| Hourly wage per hired fighter | 0 R |
| Owned-town goodwill recovery | 5 points per 600 qualifying simulation seconds, capped at 60 |

Retain faction body/outfit selection, role equipment and existing unlock rules.
The experienced fighter uses the existing specialist/MG preference, but may
carry the base rifle before the MG is unlocked. UI must not promise an MG kit.
No new medic/sniper/AT selection, finite weapons consumption, population model,
per-hire support penalty, automatic dismissal, or loyalty system in this change.

## Opening and eligibility

Preserve the existing player and starting companion. Disable the opening 2 HR
and all passive HR income for Realistic; they do not become camp hires, a second
payment, or extra starting fighters. The first objective becomes organizing a
town, then hiring there. Never equate choosing an HQ town with owning it.

The player must be able to raise a first town without hires. Source suggests
that presence in a clear neutral town can raise support from 20 to 60 in eight
normal 3-second support ticks, but this is a source-derived expectation, not a
verified opening. The native presence predicate excludes the undercover player,
but includes the companion, including civilian outfit bodies. Acceptance must
verify that the companion can organize the opening and that at least one
reachable CITY is viable on each offered
island. If an island has no CITYs, do not offer Realistic for that island and
explain why; do not silently switch the selected mode at launch.

**Confirmed opening blocker (issue preparation):** there is no existing
exposed-player organizing route. undercover.sqs deliberately keeps gmUndercover
true even after per-observer exposure, while ZoneRegistry excludes that player
from capture presence using the global flag. Losing the companion before the
first town therefore removes the only opening organizer in Realistic. Resolve
this before release; see #79 D1. Do not present exposure or firing as a working
solution. A narrowly scoped explicit town-organizing action is an implementation
option to evaluate; changing global undercover/perception rules or introducing
free conventional hires is not assumed approved by this plan.

**Proposed resolution to implement and verify:** offer an explicit "Organize
this town" / "Stop organizing" action in Realistic at a neutral or
occupier-administered CITY, matching the existing CITY capture branch. Bind
the intent to one stable town name and the actual local player. The native
CITY tick may treat that intent as civic organizing presence only while the
player is alive, on foot and within 150 m horizontally, the town is still
neutral or occupier-administered, and there is neither panic nor effective
occupation/reserve. Third-party towns remain excluded. Use the
existing CITY support rate and threshold/capture event path; combine it with
ordinary resistance presence as a boolean, so a companion does not double the
gain. This is an explicit new civic route, not a claim that the existing
undercover player counts as an armed fighter. It contributes nothing to military
capture, force counts, remote presence or AI identification. Do not toggle
gmUndercover, setCaptive or observer memories, and do not write ownership from
the action script. Death, leaving range, entering a vehicle, panic, occupation,
ownership change or load cancels the intent; restarting requires a new action.
No money, candidate or companion is required. Already owned towns use the
separate recovery rule below. This proposal resolves the design gap only when
the real action-to-native-tick route passes the companion-loss acceptance test;
it is not implemented by this document and remains linked to #79 D1.

TownSupplyEligible means CITY, owner equals resolved gmResistanceSide, support
>= the saved campaign threshold, no civilian panic, and no effective occupying
troops/reserve. Only these town conditions gate initial seeding and replenishment;
neither player proximity nor money nor available group space gates supply.
PurchaseEligible additionally requires the
actual caller to be the alive local player, on foot, within 150 m horizontally
of the bound town centre, with no effective occupying troops/reserve in that
town, a valid offered role/price, and no resource/group/spawn failure. Use a
required fresh, read-only native presence query at purchase commit, sharing the
existing effective-unit/reserve predicate. gmZoneSim is a last-tick diagnostic
snapshot, not a current occupation check. Reserve attribution must be side-aware:
friendly holding forces never become occupier reserves just because a generic
garrison field is nonzero. No additional heat threshold or mandatory companion.

If radii overlap, prefer the nearest supply-eligible centre, deterministic tie
by stable zone name; retain an open eligible town selection while still in range
and eligible. If no eligible town is in range, show the nearest CITY's blocked
status within 150 m, with the same tie rule. A status action does not confer
eligibility. Outside all town radii, journal guidance identifies a reachable
organizing/hiring goal instead of mounting a local hire menu.

Each request binds unique monotonically increasing nonce, menu generation,
town name, role and quoted fee. Walking to another town never redirects it.
Eligibility is rechecked at commit, including distance and current price. A
changed price rejects and refreshes the quote without spending. Deduplicate
request nonces; a valid current menu generation may issue multiple deliberate
hires, each with its own nonce.

## Local pool state machine and clocks

Persist one row per stable zone name: initialization flag, candidate count,
eligible-time remainder and last accounted simulation time. No town state is
keyed by transient civilian bodies, zone array indices or current player range.

On first qualification, seed 2 candidates exactly once. Thereafter accumulate
eligible simulation seconds; each 600 seconds adds one candidate up to 3.
Each hire consumes one candidate regardless of role. Qualifying remote towns
replenish too; proximity is only a purchase condition.

Use a one-simulation-second accounting cadence, sampling all towns' current
supply/recovery conditions in one pass. Refresh authoritative presence in a
batched native GatherInputs-style query for this pass too; a three-second-old
gmZoneSim snapshot would invalidate the stated accounting error bound.
Credit an interval only when its
previous and current samples qualify; if the accounting gap exceeds two seconds,
discard the gap and rebase instead of granting lazy catch-up. Normal scheduler
jitter up to two seconds uses actual elapsed time. This is explicitly sampled
eligibility: an entirely unobserved sub-interval can affect at most two seconds
of progress; it cannot seed again or complete a large deferred refill. Known
eligibility-changing events between samples invalidate that interval. Test
both observed changes and short between-sample toggles with this bound. A fresh
commit query always prevents buying while actually ineligible, regardless of
sampled supply progress. Do not credit arbitrary elapsed time from a single
current endpoint.

Ineligible or occupied/panicked towns retain candidates and fractional progress
but freeze replenishment. At capacity, discard surplus and hold progress at
zero: hiring after a long wait does not trigger immediate backlog replacements.
Recapture, regained support, travel, reopening the menu and save/load never
repeat initial seeding. Missing/orphaned zone names remain unavailable, with no
automatic reassignment to a different town.

All timings use elapsed simulation seconds, not real wall time, calendar time,
daytime or loop counts. Pause/offline time and calendar-only skipTime do not
advance recruitment or wages. Simulation acceleration follows actual simulation
elapsed time during normal accounting cadence. Long accounting gaps receive
no credit as specified above. Load restores time/progress without repeating an accounted
interval. An explicit clock discontinuity rebases the last-sample timestamp
without awarding positive elapsed time. Ineligibility discards that interval's
elapsed time, not the accumulated fraction from previous qualifying intervals.

## Keeping goodwill recoverable

The current native CITY support tick excludes resistance-owned towns. A town
captured at 60 can therefore become permanently recruitment-ineligible after a
small script penalty. Realistic needs a narrow recovery rule rather than a
threshold exception or faster initial capture.

RecoveryEligible means an owned CITY with either effective resistance presence
or the alive local player on foot within 150 m, no effective occupying troops
or reserve, and no civilian panic. The player can repair trust while undercover
in an already owned town; this never changes neutral-town armed-presence or
capture rules. While RecoveryEligible, restore support fractionally at
5/600 points per accounted simulation second, capped at the hiring threshold.
Never reduce support already above that threshold. Recovery needs no hire or
paid action, works with the player alone, and stops while conditions fail.
Existing neutral/occupier-town support/capture behavior stays unchanged.
Recovery applies only in Realistic. Fractional recovery avoids a 10-minute wait
after a 0.05-point loss; one point takes two qualifying minutes.

The recruitment manager owns this recovery policy and its accounted elapsed
time, applying deltas through the existing GM_fnSupportAdd helper and native
gmZoneSet seam. Use the same freshly sampled presence/panic inputs as candidate
accounting. Add no new direct support writer or inference that
ownership naturally reverses: the current native subsystem has no general
town-recapture path. Scripted ownership changes still invalidate hiring.

## Money and balance boundaries

Existing town income remains support *0.5 R every 600 seconds regardless of
ownership; military income, donations, dealers, training and HQ costs remain
on the same treasury. Realistic disables HR production/consumption/UI only.
Thirty R for a rifleman equals one support-60 town's entire 10-minute income.
Neutral support-20 towns each add 10 R per tick, making money less restrictive
on large islands. Do not normalize island income or add a second treasury here.

Illustration with one owned support-60 town and all other towns neutral at 20,
excluding military income and donations:

| Total towns | R per 10 minutes | Eligible replenishment/hour | Waiting pool |
|---:|---:|---:|---:|
| 1 | 30 | 6 | 3 |
| 5 | 70 | 6 | 3 |
| 20 | 220 | 6 | 3 |

With 1/4/10 eligible towns, maximum distributed candidates become 3/12/30.
Traditional caps unspent HR at 12; that is **not** a total-soldier limit.
Do not add an aggregate candidate allocation system or a new active-fighter
cap. Preserve 12-per-group splitting and existing engine resource limits;
block an unsafe spawn with an explicit reason and no charge.

The 2-candidate first-eligibility grant per town and larger distributed reserves
are deliberate availability changes. Therefore "mild rebalance" is a measured
target, not a proven property of these numbers. Evaluate the opening, cumulative
hires, replacement delay, treasury left for equipment/training, and travel
friction on a small and a large island. Tune hire fees, seed count, capacity or
replenishment first; keep wages at zero unless the user requests a wage change.

## Dormant salary feature

Store a stable hire record containing monotonic hireId, unit reference, origin
town and role; it survives overflow groups, reassignment, name and rank changes. Track
employment, not current group membership or GM_PLAYER_GROUPS, which is reset
on load. Dead/deleted hired units stop accruing; never respawn them from this
ledger. Player, starting companion and automatically created holding/HQ
garrisons are excluded. No ordinary-hire-to-companion adoption path currently
exists; adding one or recreating missing hire bodies is outside this delivery.
A future body-transfer helper must preserve hireId and transfer the reference
without duplicating employment; no present adoption feature is implied.

Default hourly rate is zero, saved in campaign tuning. At zero, skip monetary
accrual/deductions and all wage UI, reports, warnings, arrears and morale logic.
Do not accumulate a debt or elapsed bill to be charged when the rate becomes
positive. There is no player-facing wage slider in this delivery.

Provide a tested developer configuration setter for a prospective nonzero rate.
Rebase accrual when the rate changes; elapsed zero-rate service is free. For a
nonzero rate, accrue proportional employed simulation time and settle one
aggregate treasury bill each 3600 simulation seconds. New hires and deaths are
prorated; already earned accrual is preserved when an employee dies. Charge at
most the available treasury. Record a shortfall for diagnostics without debt,
negative treasury, deserters or future catch-up charges. When positive, show
hourly total, next settlement and actual payment/shortfall; monetary amounts
retain fractional precision internally. Player punishment for shortfalls would
be a separate user decision. Changing the rate settles current positive-rate
accrual once and starts a fresh interval; it is a developer action, not a
player-accessible exploit. Rates must be finite and nonnegative.

## Transaction, UI and architecture

Keep economy/recruitment policy in the canonical shared core, using synchronous
helpers and a single manager. A hire must perform validation, successful
creation, equipment, treasury debit, candidate consumption and employment
registration in one non-yielding operation. No cached balance across a wait.
Failure leaves treasury, candidates, employment and objective unchanged and
cleans up newly created empty groups/bodies. Save cannot capture half a hire.
One request nonce can commit at most once. Other treasury producers and
consumers use synchronous read/modify/write at their commit seams too.

Add narrowly scoped native queries or transaction assistance if evaluator
semantics cannot uphold that contract; do not claim atomicity merely because
there is one recruiting manager. Character initialization scripts must not
yield or mutate other game-state during the transaction. Changes are against
Remaster only, with no Classic fallbacks.

At Camp retain training and remove conventional hires in Realistic. Make shared
training monotonic: max(oldSkill, min(oldSkill + step, warLevelCap)), so buying
training cannot lower an experienced fighter's 0.60 skill to WL1's 0.45 cap.
This correctness fix is a prerequisite and preserves the 30 R price.

Local menu identifies the town, stock and upfront prices. Show useful blocked
reasons: not friendly, support below threshold, panic, occupation, no candidates,
insufficient funds, or engine capacity. Empty stock shows a countdown only
while replenishing; otherwise show the blocking condition. Failed purchases
report an actionable result. Refresh town/price/count labels when state changes.

Update new-game help, first-town/first-recruit objectives, journal advice,
Supply/Strength entries, income hints and field manual to replace Camp/HR
language in Realistic. Salary information is absent at zero. On load discard
queued intent and old action IDs, reconcile employment without recreating units,
and mount exactly one correct menu. Traditional UI and recruitment remain the
baseline except the shared correctness fixes: monotonic training and failure-safe
hire transactions.

Implementation surfaces: core/init.sqs, scripts/recruit.sqs and its dispatcher,
a dedicated recruit helper library, scripts/economy.sqs/campaign.sqs/companions.sqs;
native ZoneRegistry query/commit-assistance seams; GuerrillaNewGame selection; journal
input collection and composition. The installer ships the canonical gmcore;
do not resurrect per-mission script copies or edit installed game data directly.

## Implementation examples and ownership

The following are implementation sketches, not shipped functions or a new
framework. Names prefixed `GM_fnRecruit*`, `GM_RECRUIT_*` below and the native
presence/organizing interfaces are **proposed** unless explicitly identified
as existing. Keep one recruitment manager in `recruit.sqs`; extract its
synchronous helpers into a small `scripts/recruit_lib.sqs`, with documented
arguments/results and a readiness flag. No second economy, zone registry,
generic event bus or background payroll manager is needed.

| Surface | Implementation responsibility |
|---|---|
| `core/init.sqs` | Resolve selection once; initialize saved mode, tuning and ledgers before dependent managers; load helper library. Keep direct-launch Traditional default. |
| `core/scripts/recruit_lib.sqs` (new) | Named tuple fields, read-only predicates, sampled accounting, hire commit and wage setter. Separate helper scratch prefixes prevent nested-call clobbering. |
| `core/scripts/recruit.sqs`, `recruit_action.sqs` | One manager owns ledger mutations and menus; dispatcher snapshots bound action data and caller into a request. UI reads results, never debits. Retain Camp training. |
| `core/scripts/lib.sqs`, `loot.sqs` | Reuse existing support/treasury helpers and faction equipment selection; expose reliable equipment validation for a newly staged recruit. Avoid copying loadout policy. |
| `core/scripts/economy.sqs`, other treasury consumers | Mode-gate HR production and hints; retain R income; make every check/debit commit synchronous. |
| `core/scripts/campaign.sqs`, `companions.sqs` | Post-load recruitment reconciliation is requested through the existing load event path; ordinary hires are reconciled separately from companion respawning. |
| `Game/Guerrilla/ZoneRegistry.{hpp,cpp}`, `ZoneRegistryCommands.cpp` | Shared fresh presence census and side-aware reserve attribution; narrow CITY organizing input, validation and existing capture-event path. No recruitment money/pool policy in C++. |
| `UI/Guerrilla/GuerrillaNewGame.{hpp,cpp}`, `UI/OptionsUIApp.cpp`, active Remaster menu resources | Recruiting selector, help and no-CITY validation; publish selection through the existing launch/replay bridge. |
| `UI/Guerrilla/GuerrillaJournalPages.{hpp,cpp}`, `JournalComposeOps.cpp`, `JournalManual.cpp` | Gather typed mode/recruitment facts once; pure composition branches for advice, Supply, Strength and manual. Do not duplicate eligibility logic in text rendering. |

Paths beginning `Game/` or `UI/` in the table are relative to
`engine/Poseidon/`. New helpers should be small enough to review individually;
reuse existing `GM_Z_*` zone indices and give each new record field a named
constant. A few domain-specific helpers are preferable to one giant line with
all policy embedded in it.

### 1. Saved records and synchronous script boundaries

Use ordinary serializable arrays, consistent with shared-core state. A proposed
record contract is shown as **data notation**, not executable SQF; indices and
schema version belong together in the helper library:

```text
campaign = [schemaVersion, recruitingMode, savedTuning]
town     = [zoneName, initialized, candidates, eligibleSeconds, lastSimTime,
            previousSupplyEligible, previousRecoveryEligible]
hire     = [hireId, unitRef, originZoneName, role, lastWageSimTime]
request  = [nonce, menuGeneration, zoneName, role, quotedFee, caller]
result   = [ok, reasonCode, hireId]  // e.g. PRICE_CHANGED, OCCUPIED, KIT_FAILED
```

Persist the town/hire rows, next hire ID, next nonce and payroll totals/clock.
Resolve a name with existing `gmZoneIndex` at use time; an index is a temporary
lookup, never persisted identity. Never infer employment from names, ranks or
the current group. Single-precision script scalars cannot represent arbitrarily
large consecutive integers: use a checked exact-integer range for IDs/nonces
and reject exhaustion before incrementing, or a small native integer/string
allocator. Never wrap into an old ID. Menu generations and processed request
state must prevent replay within a session; after load invalidate the generation
and clear queued intent, while keeping allocation monotonic from the save.
Use a bounded FIFO (for example, eight pending requests), with nonce allocation
and append in one synchronous dispatcher call. On overflow report "Recruiting
busy; try again" without allocating/overwriting a queued request. Resolve the
action ID against its immutable mounted binding before enqueueing; snapshot
that generation/town/role/fee, not whatever menu is current when dequeued.
With this single ordered queue, a consumed-nonce high-water mark replaces an
indefinitely growing set; explicitly reject older/out-of-order nonces and
consume failures too. A retry gets a new nonce. Refreshing labels after a hire
does not invalidate other valid requests solely because stock changed; each
still rechecks current stock, price and eligibility at commit.

`scripts/lib.sqs` documents the evaluator contract: `call` is synchronous,
`exec` is asynchronous, and an SQS `{...}` helper definition must occupy **one
physical line**. Do not paste the multiline pseudocode below into an SQS file.
For example, this proposed helper follows the actual source convention:

```sqf
; Input [now, lastSample, previousEligible, currentEligible, invalidated].
GM_fnRecruitElapsed = {GM_tmpREdt = (_this select 0) - (_this select 1); if ((GM_tmpREdt < 0) or (GM_tmpREdt > 2) or (not (_this select 2)) or (not (_this select 3)) or (_this select 4)) then {GM_tmpREdt = 0}; GM_tmpREdt}
```

Only a manager loop waits (`~1`); its accounting or commit is one synchronous
call on one evaluated line. Do not use `exec` for equipment, ledger writes or
debits inside that call. Library readiness must include the existing loot
helper's availability. Scratch owned by `GM_fnSupportAdd` (`GM_tmpSA*`) and
`GM_fnEquipFromUnlocked` (`GM_tmpE*`) must not overlap recruitment scratch.

### 2. Fresh facts, separate decisions

Add a read-only native batch query, proposed name `gmZonePresenceNow`, that
calls a shared census implementation once and returns rows keyed by zone name.
The existing `GatherInputs` enumerates each resolved side's alive units using
`CountSidePresence`; reuse its effective-presence semantics and configured zone
area. Refactor common code instead of duplicating those loops in a command.
Return current resistance/occupier counts and effective occupier reserve plus
a sample timestamp; do not tick support, spawn garrisons, create centers or
modify `lastGuer`/`lastOcc` diagnostics. Reserve attribution needs an explicit
ownership/cache-side rule: current `z.garrison >= 1` alone is not a new
side-aware API. Define and test attribution with occupied, spawned, despawned
and friendly holding cases before using it for recruitment.

Each one-second accounting pass uses one batch, joins current zone tuples and
`GM_PANIC_ZONES`, and computes these separate predicates. The purchase commit
obtains a fresh batch again; it cannot reuse the manager's previous sample.
Illustrative **pseudocode**:

```text
clearTown = (occupierCount == 0 && occupierReserve == 0 && !panic)
supply = isCITY && owner == resistanceSide && support >= threshold && clearTown
recovery = isCITY && owner == resistanceSide && clearTown
           && (resistanceCount > 0 || aliveLocalPlayerOnFootWithin150m)
purchase = supply && validBoundCallerWithin150m && candidates > 0
           && currentRoleAndQuoteValid && currentFundsAndCapacitySufficient
organizing = realistic && isCITY && (isNeutral || owner == occupierSide)
             && clearTown && boundOrganizingIntent
             && aliveLocalPlayerOnFootWithin150m
```

Supply is independent of purchase affordability and distance. Recovery does
not depend on support already meeting the hiring threshold. Pass facts to
predicates; return a stable reason code for UI translation rather than emitting
hints from predicates. The organizing intent is **new** native tick input:
gather/validate it for the bound town and OR it with existing CITY organizing
presence only within that CITY branch. Never add it to `guerCount` or armed
presence. Native validation must reject forged/stale intent even if the menu
already checked it. Cancellation must be latched, including a known invalidating
event between ticks; merely becoming eligible again must not resume an old
intent. Leave the military branch and per-observer undercover system untouched.

### 3. Hire commit and rollback

Current `recruit.sqs` checks money on earlier SQS lines, spawns on one line,
then equips and charges later; null creation still reaches charging and the
objective. Moving that flow into a single synchronous helper is necessary.
It is not enough to atomize just `gmResources = gmResources - fee`.
Illustrative **transaction pseudocode**, with all called operations synchronous:

```text
commit(request):
  consume fresh nonce; validate generation, caller, bound town, role and quote
  facts = freshPresenceBatch(); validate purchase against CURRENT treasury
  stage group selection and create one body with an empty init string
  if body missing or equipment postcondition fails:
      delete staged body; delete only newly created empty group; return failure
  publish successful group membership, candidate decrement and hire record
  gmResources = gmResources - currentValidatedFee
  return [true, OK, newHireId]  // firstRecruit completion belongs to this commit
```

Preflight body class, kit and group/resource limits before staging. Existing
`createUnit` returns no unit; the source discovers it via the before/after
`units` difference. Require exactly one valid new body, and keep a newly
created overflow group out of `GM_PLAYER_GROUPS` until success. Existing
`GM_fnEquipFromUnlocked` returns the unit, **not a success flag**: select the
existing unlocked-or-base kit, validate it and check resulting required kit;
do not treat a non-null return as proof that weapons were installed. Cleanup
must remove only objects staged by this transaction.

Prepare all potentially failing values before publishing, so the final writes
cannot ordinarily fail. A handled staging failure changes no treasury, pool,
employment or objective. UI hints and decorative notes can follow success;
the saved first-hire objective state cannot lag the commit. Consume the nonce
on failure without consuming a candidate. Never restore a saved old treasury
value as "rollback": that could erase another writer's income. Audit other
spenders' check/debit seams; `GM_fnResourcesAdd` already reads and updates the
current balance synchronously. Prove save exclusion and no evaluator/event
reentrancy across staging/publication with failure injection. If scripts cannot
guarantee cleanup or exclusion, add narrow native transaction assistance before
claiming the contract; an SQS helper has no assumed exception/RAII facility.
Traditional uses the same safe spawn/commit machinery with its existing prices
and HR cost. Training's check, monotonic skill updates and 30 R debit also
belong in one synchronous commit.

### 4. Town clocks and goodwill share one accounting pass

The following **pseudocode** uses the elapsed helper above; recovery and supply
have separate previous-eligibility bits and invalidation, but the same sample
time. Compute both predicates from one pre-update snapshot, so crossing the
support threshold cannot retroactively credit supply time:

```text
for each stable town row in fresh facts:
  supplyDt = qualifiedElapsed(now, last, oldSupply, supply, supplyInvalidated)
  repairDt = qualifiedElapsed(now, last, oldRecovery, recovery, repairInvalidated)
  if supply && !initialized: initialized = true; candidates = seed; fraction = 0
  else if supply: fraction += supplyDt; convert whole refill periods up to cap
  if candidates == cap: fraction = 0
  if recovery && support < threshold:
      delta = min(threshold - support, (5 / 600) * repairDt)
      [resolvedZoneIndex, delta] call existing GM_fnSupportAdd
  last = now; oldSupply = supply; oldRecovery = recovery; clear invalidations
```

Advance/rebase `last` even while ineligible or after a discarded gap; otherwise
the next eligible sample accidentally catches up. Keep previously earned
fractions while frozen. First seeding adds no preceding interval. A threshold
crossed by recovery becomes supply-eligible at the next fresh sample. Known
support/owner/panic mutations invalidate the relevant interval through the
existing support and event seams; no duplicate direct support writer. An
unobserved brief physical presence change remains subject to the stated sampled
two-second error bound, and a purchase always uses current facts. At load,
retain earned fractions but rebase timestamps and clear previous-eligibility
bits before accounting resumes, preventing credit across reconciliation.

### 5. Prospective wage configuration, without a payroll subsystem

Keep a saved aggregate earned amount and settlement elapsed time plus per-hire
accounting timestamps; integrate live-hire service in the recruitment manager.
A death/deletion must account service to its observed simulation boundary
(prefer a synchronous killed-event stamp), retain already earned money and
then retire the unit reference. Do not drop the aggregate when pruning a row.
Use a tested prospective setter, illustrated in **pseudocode**:

```text
setHourlyRate(newRate, now):
  reject unless finite and newRate >= 0
  if newRate == savedRate: return  // no forced settlement for a no-op
  account existing service through now under OLD rate and clock rules
  if savedRate > 0: settle earned aggregate once against CURRENT treasury
  savedRate = newRate; earned = 0; settlementElapsed = 0
  rebase all live employment timestamps to now
```

At rate zero the accounting branch creates neither earned accrual nor monetary
UI; changing zero to positive starts at `now`. For positive settlement,
`payment = min(earned, max(0, gmResources))`; subtract that payment synchronously,
record actual payment/shortfall and clear the settled earned amount once. No
unpaid remainder becomes next period's debt. Positive-to-zero settles earnings
under the old rate before hiding wage UI. Persist accrued fractions and clock
state; load rebases timestamps without paying twice or billing the reconciliation
gap. Use the same pause/discontinuity/long-gap rules as specified above. The
setter stays a developer/configuration seam, not another player action.

### Lifecycle wiring and tests that exercise the seams

New-game work includes the `OptionsUIApp.cpp` launch bridge: the existing
`gmSel*` pattern uses both `GGameState.VarSet` and a `GStats._campaign` variable,
because world initialization resets the former and later replays the latter.
Follow that route for the proposed recruiting selection; testing only the
dialog getter misses a lost selection at launch. Copy selection into immutable
campaign state in `init.sqs`, not on every manager tick. Gate starting HR,
economy HR and displays from that state. Register helpers before the recruitment
manager can accept actions; do not rerun initialization when loading a save.

The existing `campaignLoaded` event already queues into `gmEvtLoaded`, and
`campaign.sqs` drains it. Extend that path to signal a recruitment reconciliation
barrier, not a second competing consumer of the event queue. Gate purchases
and accounting until reconciliation completes; clear intent, remove old action
IDs, invalidate menu generations, validate persisted names/unit links and
reconstruct needed overflow-group links from live hire records after the current
`GM_PLAYER_GROUPS = [group aP]` reset. Do not respawn dead ordinary hires via
the companion manager. Remount once after deriving current eligibility. A
loaded town with an orphaned name stays unavailable. Existing old saves retain
their serialized SQS text, so this is not an old-campaign migration promise.

`GatherGuerrillaJournalInputs` should gather mode, town availability and positive
wage facts into `JournalPageInputs`; `ComposeJournal` remains pure. Add mode
cases to advice, Supply and Strength instead of teaching composition to read
script globals or calculate eligibility. Keep stable objective IDs where useful,
but make the new-campaign objective text mode-specific and update completion
from committed outcomes. The explicit organizing action must have player-facing
start/cancel/blocked guidance, including after companion loss.

Extend the existing zone-registry unit tests and journal composition tests;
add scripting integration cases alongside `guerrilla_human_suite` and the
new-game UI end-to-end cases. Examples of meaningful assertions:

- Queue a 60 R hire, spend 50 R elsewhere from an initial 100 R before commit:
  reject for current funds; treasury stays 50 R, pool and objective unchanged.
  Inject body/kit failure after a new group is staged; assert no empty group,
  body, hire record or charge leaks. Replay the nonce; no second outcome.
- Start with fraction 599 at capacity minus one: one eligible second fills the
  pool and clears the fraction; hire after a long full interval, then verify
  another complete 600 eligible seconds is needed. A ten-second scheduler gap
  adds zero. Support 59.95 recovers to 60 after six qualifying seconds (within
  float tolerance) without reducing support 60.1.
- With the companion dead, start organizing through the real action in a clear
  neutral town: support advances once per native tick and normal capture fires.
  Assert `gmUndercover` stays true, military progress is unchanged, and an
  occupier/panic/range interruption cancels intent until explicitly restarted.
  Merely firing/exposing the player without organizing must not pass this test.
- At zero wage, wait then set 12 R/hour: no historical charge. Account a live
  hire for 300 accounted employment seconds, change the rate to zero: settle 1 R once.
  Repeat across save/load and a death boundary; assert no duplicated service,
  settlement or debt, allowing only the documented sampling error.
- Select Realistic in the actual new-game dialog, reach the mission, save/load
  with a pending hire, and verify saved mode, discarded intent, one menu and
  unchanged stock. A pure journal fixture for zero wages contains no HR/Camp
  recruiting advice or salary blocks; Traditional retains its advice.

These are proposed verification cases, not tests run by this planning change.
Use full Remaster fixtures; adapt older test patterns without adding Classic
lanes. Timing and failure-injection tests must exercise production dispatch and
helpers, not a test-only reimplementation of their arithmetic.

## Delivery and verification gates

1. Prove and fix shared monotonic training and hire-failure charging first.
2. Add saved mode/tuning, local candidate ledger, eligibility query, goodwill
   recovery and transaction helper. Disable Realistic HR paths.
3. Complete mode-specific menus/objectives/journal, salary-zero behavior and
   employment persistence; add the dormant nonzero configuration behavior.
4. Verify using full Remaster fixtures; reinstall the core into a prepared
   shadow, never the Steam install. Deliver only after the opening/balance
   playtests, tune values if necessary, and leave Traditional selectable.

Required automated behavior coverage:

- Traditional default/direct launch; explicit Realistic new game and reload;
  no mid-campaign mode change; missing-CITY selection explained.
- Real opening player/companion -> neutral town -> owned town -> first paid
  hire; reverse faction-side choices, civilian outfit and multiple islands.
- Boundary support 59.99/60, changed owner, enemy presence/reserve, panic and
  death; friendly staffed towns continue supply/recovery/hiring. Stale
  town/role/action, changed fee, duplicate nonce, vehicle/range exit; two
  deliberate purchases from one menu both succeed with distinct nonces.
- Shared role pool; initialization exactly once; finite refill; capacity surplus
  discarded; eligibility loss/restoration, distance independence and pause.
- Paid spawn failure, equipment failure, insufficient funds, group/budget
  exhaustion; no debit, consumed candidate, empty-group leak or objective award.
- Save/load before/after hire, fraction and cap, overflow group/reassignment,
  employment death/deletion/name/rank change and transient menu reset. Future
  body adoption is not a present behavior test or scope requirement.
- Zero wage produces no UI, debit, debt or delayed bill; positive test rate
  exercises prorating, death, rate changes, fractional rounding, insufficient
  funds, clock/save boundaries and no duplicate settlement.
- Owned-town fractional recovery to threshold, no above-threshold reduction,
  interruption by panic/occupation/no presence; undercover player can repair
  owned-town trust but does not contribute to neutral/occupier CITY capture
  without explicit organizing intent. Verify companion-loss
  opening via the deliberately implemented blocker resolution, without assuming
  per-observer exposure clears gmUndercover; no-eligible-town status remains
  informative and money/group capacity do not stop distant town replenishment.
- Low-war-level training never reduces veteran skill; other squad members
  retain the existing training cap/progression. Journal contains no misleading
  HR/Camp advice or guaranteed specialist weapon in Realistic.

Gameplay acceptance records paired Traditional/Realistic sessions on a small
and a large supported island with the same faction/outfit, comparable town
holdings and recorded income. Observe first-town time, first hire, opening
squad affordability, town exhaustion/refill, casualty replacement and remaining
funds for equipment. Salary remains zero throughout. Report differences rather
than declaring parity; unresolved opening or severe pacing differences block
the default tuning, not grounds to enable wages automatically.

</details>

## Source grounding

The values and constraints above were checked against current source, not the
older design documents:

- [Recruitment](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/scripts/recruit.sqs): fees/skills, Camp menu, overflow
  groups, current failed-spawn charging and training downgrade.
- [Economy](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/scripts/economy.sqs) and [initial state](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/init.sqs): income,
  HR reserve ceiling, treasury and starting companion.
- [ZoneRegistry](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/engine/Poseidon/Game/Guerrilla/ZoneRegistry.cpp): owned-town
  support exclusion, capture/presence rules, undercover exclusion and side
  resolution. [Zone commands](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/engine/Poseidon/Game/Guerrilla/ZoneRegistryCommands.cpp)
  document gmZoneSim as a last-tick snapshot.
- [Campaign restoration](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/scripts/campaign.sqs): native global serialization,
  saved SQS text, transient action/group reconciliation.
- [Time command](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/engine/Poseidon/Game/Commands/GameStateExtGrp.cpp): time reads
  Glob.time. [Calendar skip](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/engine/Poseidon/Game/Commands/GameStateExtUi.cpp)
  modifies Glob.clock. [World serialization](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/engine/Poseidon/World/WorldImpl.cpp)
  preserves Glob.time as SimulationTimeMs.
- [Companion manager](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/scripts/companions.sqs): existing-roster rank progression,
  no ordinary-recruit adoption path. [Equipment](https://github.com/GearUnclear/CWR/blob/main/arma_CWR/guerrilla-mode/core/scripts/loot.sqs): unlocked
  role equipment falls back to the faction's base rifle.

