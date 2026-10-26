# Occupier forces review, phases 1 to 3 (16259cc..6163d40)

Read-only review of 84fbbce (phase 1), e37e79a (phase 2), 8f9482a (director),
7a143ec (movement, hulls, birth chain). Two independent Opus reviewers per
commit, merged here. Every item marked CONFIRMED below was re-read by the lead
in the committed blobs. Line numbers are from 7a143ec (the end of phase 3)
unless noted. CA = CounterAttack.cpp, OF = OccupierForces.cpp,
GC = GarrisonCache.cpp, ZR = ZoneRegistry.cpp (all under
arma_CWR/engine/Poseidon/Game/Guerrilla).

## HIGH

**H1. In release builds, a disguised player "holds" every town once any occupier group knows him (D11 broken).** CONFIRMED
- Where: Undercover.cpp:704, `Glob.time - t->ucWeaponSeenTime <= WeaponSeenWindow`.
- Cause: `ucWeaponSeenTime` starts at TIME_MIN (`Time(-INT_MAX)`, Target.cpp:111). It is transient, so it is TIME_MIN again after every load. Under NDEBUG, `Time::operator-` is `1e-3f * (_time - src._time)` on an int (Foundation/Time/Time.hpp:112). Once Glob.time is above 0, that subtraction overflows to a large negative float, so the test is true. Traffic.cpp:2333-2342 documents this exact trap and guards it with `== TIME_MIN`.
- Scenario: the player is in a civilian outfit, unarmed and not exposed, and some occupier group has a live target record of him.
  - `WeaponSeenCount() > 0`, so `weaponSeen` (ZR:2788) is true and HOLD is true.
  - Every held town he stands in stops draining, against patrols and counter-attacks alike.
  - `gmUndercoverWeaponSeen` and `gmPlayerPresence` slot 7 read wrong.
  - Debug builds behave correctly, which is why it hides.
- Fix: `t->ucWeaponSeenTime != TIME_MIN && ...`, or compare `t->ucWeaponSeenTime >= Glob.time - WeaponSeenWindow`.
- Test gap: add a world-level case (civilian outfit, no weapon, known to an occupier group, must not hold). The unit test only covers the pure float helper.

**H2. Nothing registers the pending-refund hook, so pool-paid pending bodies are thrown away when a zone changes hands (5.5, 3.3).** CONFIRMED
- Where: GC:373 (hook defaults to null) and GC:509-537 (`RefundPending`). `git grep SetPendingRefundHook 7a143ec` finds it only in its definition and test_garrison_cache.cpp:319/321. OccupierForces never installs it.
- Scenario, towns: key on (Guerrilla.Abel opts in), the player is inside the deposit bubble, and a town reverts.
  - `RevertTown` (ZR:1727-1733) pays the refill from the pool, and it becomes `pending` with origin -1.
  - The player re-liberates the town before the deposit releases.
  - `RefundPending` falls into the key-off branch and discards the bodies, with no Refund and no NoteLoss.
  - A player who loiters in a contested town can bleed up to garrisonMax from the pool per re-flip without firing a shot.
- Scenario, bases: `ReturnToSource` (CA:1281) leaves survivors pending at their own base (origin == zone, so `o` is null). If that base flips, they are discarded.
- The surplus over the origin's room (GC:530-534) is also discarded instead of going to the pool.
- Effect: the 5.5 equation breaks on every such cycle.
- Fix: in `OccupierForces::InitMission`, register a hook. While `Active()` it routes `back` to the origin, `Refund`s the rest and returns true. Otherwise it returns false and the key-off rule runs.

## MEDIUM

**M1. The director quotes foot spares that CounterAttack will not grant: the same launch is refused every tick, or goes out undersized. This is also an indirect D9 leak.** CONFIRMED
- Where: OF:1775-1779 (`FillEngineView`: `spare = max(stored, marched)`, where `marched` uses `cache.LiveCount`) against CA:1018-1021 (`LaunchCore`).
- How the two disagree:
  - `LaunchCore` marches live bodies only when the TARGET is in the bubble and `RoomLive` holds. Otherwise it uses the stored reserve.
  - A live base usually has stored reserve 0, because GC:749 sets `z->garrison = remaining` at spawn.
  - `n = MinInt(size, spare)` (CA:1022) then launches whatever is left.
- Scenario: the player scouts the Airfield (10 live, 0 stored) while his held town 2 km away is hot.
  - The director quotes "foot 6 from Airfield", and `LaunchCore` refuses ("cannot spare a man").
  - TryLaunch refunds, and the next tick picks the same quote again. A farther base with stored spare is never tried.
  - With stored = 2, the force launches 2 men under a minimum of 4, while the `[op N]` line still says 6.
- D9: the player's true proximity to a base (that is what makes its garrison live) steers the source choice.
- Fix:
  - Have CounterAttack expose one `SpareFor(source, target, package)` that both services call.
  - Make `LaunchFunded` refuse funded orders when `n < order.bodies`.
  - When the chosen source is refused, try the next one.

**M2. With the key on, a player standing in his town makes it immune: an arriving force freezes at the edge and never fights.** CONFIRMED
- Where: CA:1649-1692. `MustGoLive` (CA:1132) is true while the target is in the bubble, and it is checked before the arrival branch.
  - `GoLive(Progress(r))` gets `CASpawnHold` once the progress point is within 250 m (CA:410-417).
  - `BirthHold` does not set `hiddenLogged` (CA:1374-1377), so `birthGiveUp` never runs and the record retries every 30 s forever.
  - `ArriveLive` (the 6.5 town-edge birth that searches back past the 250 m bound) is reachable only from the arrival branch (CA:1741), which is skipped.
  - `CAHolding` returns on `suspended` the same way (CA:1764-1768).
- Scenario: an operation launches while the player is away. He walks back into the town, and the force sits abstract at the edge indefinitely. `FindByTarget` then blocks every other operation on that town.
- Fix: with `forcesOn` and defenders present (or the player in the bubble at arrival), route Hold and suspended records to `ArriveLive`.

**M3. Hull lifecycle leaks: crew groups are orphaned and abandoned hulls are never deleted.** CONFIRMED
- Where: `CheckHull` (CA:2124-2157), the lost branch of `FinishHull` (CA:2179-2184) and the lost branch of `StepHomeHulls` (CA:2235-2241). All three null or drop `crewGroup` and `hull` without `Remove`.
- Group budget: `CensusCenter` (ZR:2820-2840) counts every group whatever its members' life state, and dead units stay group members. Each lost hull therefore permanently occupies one occupier group slot (budget 40).
- Living crew who bailed become an untracked occupier group. They count as town presence (D5) and are never cleaned up.
- An intact hull whose crew is dead or gone stays in the world, unreferenced.
- Fix:
  - Put lost crew groups and hulls on a tracked cleanup list.
  - Remove each one when unseen, or when every member is dead and unseen.
  - Optionally adopt living stragglers into a garrison.

**M4. `RemoveHull` deletes every member of the crew group, wherever they stand; only the hull's position is checked.** CONFIRMED in code, scenario PLAUSIBLE
- Where: CA:3233-3255, with `HullUnseen` at CA:3227 (hull position only). Callers: CA:1865 (return to abstract travel) and CA:2256 (home hull).
- Scenario: a gunner bails out next to the player and the driver drives off. When the hull passes the unseen test, the gunner (or his corpse) vanishes in front of the player. That violates the S rule.
- Fix: test every crew member's position in `HullUnseen`, or delete only the crew seated in the hull.

**M5. A live top-up squad is deleted in plain view when its town is lost.** CONFIRMED
- Where: CA:1798-1808. `ops->Remove(r.group)` runs with no perception check.
- Scenario: a funded operation becomes a REINFORCE (via `RevertedUnder`, CA:1934) and is walking into the town. The player retakes the town while standing in it, and the squad blinks out.
- Fix: withdraw on foot and despawn only under `SquadUnseen`, as the timeout path does.

**M6. Legacy (unfunded) repels drive director actions while DORMANT, which breaks invariant W.** CONFIRMED
- Where: OF:1665-1684. The `CAOpRepelled` branch applies `HeatRaise(+15)`, the 30 min cooldown and the INFO line without checking `mine`. `Emit` (CA:764) sends events for legacy records too (opId 0).
- Scenario: with Abel opted in, the first town is answered by the legacy Decide.
  - That force is wiped out, so the town gets +15 heat and a cooldown.
  - The log line reads "OccupierForces: [op 0] Repelled ...".
  - All of this happens before the director has woken.
- `CAOpCancelledUnseen` (OF:1646-1662) sets the cooldown unguarded as well. Only funded records raise it today.
- Fix: guard heat, cooldown and log on `mine`, or on `ev.origin != CAOLegacy`.

**M7. Old-save derivation sizes K from current ownership, so control is overstated for the rest of the campaign.** CONFIRMED
- Where: `DeriveFromExistingSave` -> `Activate` -> `Size` (OF:1871 -> 414). `Size` tests `IsAdministeredTown`, which checks the CURRENT owner (ZR:1670).
- What goes wrong:
  - Every town the player already holds goes to `_excluded`, so K, G and poolStart shrink.
  - T is 1.0 at derivation, so C, the derived pool and `WavesCrossed` are all inflated.
  - A restored block is never re-sized, so this lasts the whole campaign. Example: the player holds every town but one, with half the roster: true C = 0.30, derived C = 0.70.
- The unit test at test_occupier_forces.cpp:867 pins this behaviour.
- Fix: classify K by the authored owner (`ResolveOwnerToken(ownerConfig)` is the occupier or NEUTRAL), then measure T over current ownership.

**M8. Hull births and hull despawns use the infantry gate: no headlights, infantry LOS height.** CONFIRMED in code
- Where: CA:2430, `EffectiveBand(..., lightsOn = false, ...)`, which births through `GoLive` (CA:1373) and despawns through `HullUnseen` (CA:3227).
- Scenario: at night, Traffic widens the band because AI vehicles light up (Traffic.hpp:483-491). A truck born just past the object cull pops its headlights into view; the same happens in reverse when it is deleted at home.
- Fix: for mounted births and hull despawns, use `lightsOn = night` and a vehicle LOS height.

## LOW-MEDIUM / LOW

- **L1. A refused launch can demote the band for good, and the op id is reused.** CONFIRMED
  - Where: OF:1450 (`Spend`, which calls `UpdateBand`) and OF:1467 (`Refund`).
  - Scenario: pool at 62 % (STRONG). A truck at price 4 drops it below 60 %, so the band becomes STRETCHED. The refusal refunds to 62 %, but STRONG needs more than 66 %. At pool 4/40 the band goes to SPENT and stays there.
  - `_opCounter` is rolled back (OF:1473), so "CounterAttack: [op N] refused" and the next real "[op N]" are different operations.
  - Fix: snapshot and restore `_band` around a refused launch, or validate before debiting. Do not roll the counter back.
- **L2. Refunds can exceed a base's room.** CONFIRMED
  - `Resource` (CA:2047) and `CancelUnseen` (CA:2089) add `r.strength` back through `AddReserve` with no room check, unlike `ReturnToSource` (CA:1281) and `CrewHome` (CA:2115).
  - `RefundPending`'s room (GC:529) ignores the origin's live garrison.
  - The live top-up adopt (CA:1829-1835) has no room cap.
  - Scenario: a base regrows by D2 while its operation waits 600 s or more, then gets the full force back and ends up above garrisonMax.
  - 5.5 says the surplus goes to the pool. Fix: reuse the `ReturnToSource` / `CrewHome` room logic in all of these.
- **L3. Crew accounting double-credits.** CONFIRMED arithmetic
  - Cause: `HullPrice()` is `priceUnits - crew` (CounterAttack.hpp:473), but `r.crew` is lowered after the crew are already refunded or lost:
    - GoLive's partial-spawn refund (CA:1430-1434);
    - the return to abstract travel (CA:1860-1864).
    - A later HullRefund then re-pays them, capped only by `outstanding`. The trigger needs crew of 2 or more: ForcePackages counts the commander and gunner, but `SpawnMounted` seats only the driver and gunner. That trigger is PLAUSIBLE for today's trucks.
  - Separately, `CAOpCancelledUnseen` refunds the whole of `outstanding` (OF:1653) and ignores `ev.units`. After `CheckHull` has already booked the crew as lost (CA:2136), the cancel refunds the dead crew.
  - Fix: store an immutable hull price, keep `outstanding` as hull-only, and refund `min(ev.units, outstanding)`.
- **L4. Infantry and crew losses go unbooked, so the 5.5 equation and the `lost` count drift.** CONFIRMED
  - `ReturnToRoad` (CA:506, called at :1872) sets `strength = alive` with no `NoteLoss`. `AdoptLive` and the reinforce paths (CA:1806, :1832) do the same.
  - Conversely, `HullState` (CA:3216) counts only crew who are alive AND inside the hull. Living crew who dismounted are booked as `NoteLoss` (CA:2136, :2172, :1862) while their bodies remain in the world.
- **L5. A force that loses its hull keeps truck-pace travel time.** CONFIRMED
  - `CheckHull` (CA:2151-2154) and the hull-not-created path (CA:1413) set `FootSpeed` but never recompute `r.travel`.
  - `ReturnToRoad` keeps any existing travel value (CA:500-503).
- **L6. `PruneOps` can lose refunds.** PLAUSIBLE (needs more than 32 rows)
  - Where: OF:1005-1019.
  - It drops the first finished row even when that row's hull is still driving home with `outstanding > 0`, so the later HullRefund is lost.
  - With no finished row it drops row 0, which may be a live operation.
- **L7. A wiped squad's all-dead group is never removed.** CONFIRMED, pre-existing
  - Where: CA:1821-1828 and the dead path in `OnZoneEvents` (CA:3354-3357). Either one leaks a group slot.
- **L8. Phase-2 saves with a block (key on via test levers only) load DORMANT with an empty `_townsEver`.** PLAUSIBLE
  - Where: OF:1963-1964 and 1986. Phase 2 never filled the set on flips.
- **L9. Sightings with `posReported == VZero` are ingested.** PLAUSIBLE
  - Where: OF:1851-1853. A target known but not yet reported lands in the "" row or near the map origin.
- **L10. `RevertTown` uses `LastTickPlayer()`.** PLAUSIBLE
  - After a load `_tickPlayerValid` is false until the first `EvaluateTick`. A revert driven by `Fight` in that window writes straight to the reserve beside the player.
- **L11. The S5 release test samples a fixed 3x3 grid, but `SpawnGarrison` uses random points in the same square.** PLAUSIBLE
  - Where: GC:453. A point between two samples can be visible.
- **L12. Derivation edge cases.** PLAUSIBLE
  - `_finalFired` is never set on derive.
  - The derived `_townsEver` includes towns authored as resistance-owned (OF:1884-1898), so a migrated save can wake early.
  - A nonzero SPENT `forcesTokenInterval` in config mints tokens in SPENT (OF:197).
- **L13. Further PLAUSIBLE routing traps.**
  - `ArriveLive` deadlocks when the source is within the bound (CA:1955-1968).
  - Return-to-road livelock, with hull churn, on a watched road 800 m or more out.
  - A home hull drives into a base the resistance has since taken (CA:2208-2212, :2264-2268).
- **L14. Test levers only.**
  - `gmCounterAttackLaunchFunded` spends with no ledger row, so the units never return.
  - `gmForcesTune "enabled"` is not persisted across a load.
  - NaN is accepted for `refillHours` and `directorInterval`.
- **L15. 8f9482a does not build on its own.** CONFIRMED
  - It includes `CounterAttackOrders.hpp` and calls `LaunchFunded`, both of which arrive only in 7a143ec.
  - This breaks bisect; land the two commits together.

## Category verdicts

- **Pool conservation (5.5):** see H2, L1 to L4, L6. Otherwise:
  - U1 to U4 are clean.
  - Refill math, clamps and the accumulator are clean.
  - The re-source chain debits and refunds exactly once.
  - `CancelUnseen` sets `hullDone`, so there is no double refund.
- **Unseen birth S / S5:**
  - Every ground birth goes through `PickSpawnPoint`, with the 250 m bound and the cone-only rule limited to the base.
  - Returns into the bubble become pending deposits.
  - Re-source terminates (`tried` grows and is saved) and falls through to cancel-unseen.
  - Exceptions: M4, M5 and M8 (vanishes or pop-ins), and L11.
- **Persistence:** field by field, every OccupierForces, Record, HomeHull and ZoneSaveState field is saved and loaded with absent-field defaults. Hull and crew refs are LLinks resolved on the second pass, and ordering and reset between missions are correct. The defects are only in derivation (M7, L8, L12).
- **D5 / D24:** clean. The ownership filter and the HQ-zone exclusion are both removed.
- **D6:** clean for funded records. With the key on, legacy records still turn back at `birthGiveUp` (CA:1671-1676), and the funded live timeout still withdraws (CA:1899-1913).
- **D9:** direct reads are clean. ScanWorld reads only `posReported`, and FillEngineView reads no player state. The indirect leak is through LiveCount (M1).
- **D11 / D23:** the HOLD versus CAPTURE wiring is right (the drain, consolidation and remote gate use HOLD; the meter, support and Fight use CAPTURE). Its input is wrong (H1).
- **D12:** the issues are M6 (an `[op 0]` line), L1 (a reused id, two lines per refusal), and several lines without the `OccupierForces: [op N]` prefix ("no operation on X", the Spend refusal).
- **Hull lifecycle and group slots:** M3, M4, L7.
- **SQS (init.sqs, escalation.sqs):** clean. `isNil` is registered, there is no short-circuit dependence, no mid-line `;`, no `bool !=`, and the string compare is fine.
- **OFFSCREEN-CONTRACT.md:**
  - The table is broken: the "These rules apply to every mission" paragraph sits between rows 16 and 17, so row 17 renders outside the table (CONFIRMED).
  - Row 2c is false with the key on. It says "Town outside: ABSTRACT arrival ... no entity created or deleted", but 6.5 now births a live force at the town edge against present defenders (CA:1728-1742).
  - Row 9 ("raised only by live events", one decay rate) is false: an abstract repel raises heat by 15, and held zones decay at `gmHeatDecayHeld`.
  - Row 4's "back to its origin's reserve" omits the room cap and the discard.
  - Section 6's "weaponSeen within the last 5 s" is false until H1 is fixed.
  - Phase 3 did not touch the contract.

## Design vs code (the doc says the code wins; fix the doc or decide)

- **Birth chain (3.1 step 3):** re-source happens only after 600 s per base, picks by straight distance and stored spare, and never checks that the route reaches the target or is gated. With N bases that is up to N x 10 min.
- **Source choice:** "nearest by travel time" in the doc, straight distance in the code (ForcePackages.cpp:434-449). The retake minimum is capped at 12 in the code; the doc has no cap.
- **Crews (5.3):** crews are paid by the pool but come home into the source base's reserve through `CrewHome`, which moves pool units into garrisons.
- **N4a / contract row C:** only scheduled records are skipped for the HQ town. A force already travelling or holding can still retake the HQ town.
- **3.2 dismount:** the truck brakes for up to 15 s after crossing 250 m, so the dismount can happen inside the bound.
- **Sightings (6.1):** a zone's sighting row is replaced only when that zone is reported again, so stale counts persist for 30 min. `order.lethality` is never set by TryLaunch.
- **gmForcesTune ranges:** wider than config (`wakeTowns` 1, `lethalityMargin` 0). `opsMax` sets only the director's copy (the known two-reader issue).
- **Section 11:** lists surges and per-town standing, and 7.1 needs a two-tick crossing streak. None of these exist yet (phase 4). Base candidates and base push-out (D16) are logged as deferred.
- **S5 gameplay hole:** a player who keeps a town's spawn points in view keeps a wiped remainder pending (it does not defend). With origin = itself, it is discarded on the flip. That is spec-consistent but worth an owner nod.
- **Comments:** CounterAttackOrders.hpp says HullLost on a return to abstract travel carries "units = its price". The code sends 0, which is right for the ledger, so the comment is wrong.
- **Stale header:** OccupierForces says "ZoneRecord carries no seeded flag", but 7a143ec adds `ZoneRecord::seeded`; Simulate still activates without a seeded table.
