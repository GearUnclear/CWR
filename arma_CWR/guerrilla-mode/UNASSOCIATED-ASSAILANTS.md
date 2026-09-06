# UNASSOCIATED-ASSAILANTS.md - civilians who fight back, rogues hostile to both sides (issue #42)

**Status: PLAN, nothing implemented.** This is the solution plan for
[issue #42](https://github.com/GearUnclear/CWR/issues/42), split out of #41.
Roadmap item: `unassociated-assailants`. Sibling contract doc:
[CIVILIAN-INTERACTIONS.md](CIVILIAN-INTERACTIONS.md) (#41, landed `f9cf244` +
`fdb6a71`).

Every claim below was read out of the tree at `8881b63`. The issue's own line
numbers predate the #41/#58 landings; the numbers here are current.

## What #42 asks for

1. **Fight back.** A civilian extorted while his fear score is under 20 resists
   instead of paying.
2. **Go rogue.** Sparingly and at random, civilians turn hostile on their own
   and attack the player's side *and* the occupier's side.

Neither is a faction. They are unassociated: hostile to everyone, loyal to
nobody, and they exist to carry the chaos a real insurgency has.

## The half that already exists

#41 landed the decision and the seam, and deliberately stopped there:

- `GM_CI_fnOutcome` (`core/scripts/civilian_interaction_lib.sqs:71-73`) already
  returns `RESISTED` for `EXTORT` when `fear < GM_CI_RESIST_MAX` (20). The
  threshold #42 asks for is live today.
- `GM_CI_ON_RESIST` (`civilian_interaction_lib.sqs:16`, fired at `:78`) is the
  sanctioned synchronous callback: `[body, caller, zoneName, verb]`, once per
  accepted low-fear extortion. Default is a no-op, so today a resister merely
  refuses to pay. CIVILIAN-INTERACTIONS.md:115-117 reserves it for #42 and adds
  the binding rule: *"This layer never changes global civilian diplomacy."*
- `GM_CI_fnEligible` (`civilian_interaction_lib.sqs:51-52`) already anticipates
  the conversion - its comment says "rogue conversion cannot pay", and because
  eligibility tests `side body == civilian` **and** membership in
  `GM_CIV_GROUPS`, a converted assailant drops out of the interaction menu for
  free.

So #42 is **the consumer of a hook that is already wired**, plus the rogue
director, plus the one engine fact neither exists without: a side the
assailants can be hostile *from*.

## Blocking facts, verified

### 1. The population is welded friendly to both war sides

`ZoneRegistry::ApplyCampaignFriendship` (`ZoneRegistry.cpp:576-624`) ends by
pinning the civilian centre friendly to everyone (`:617-622`), under the
comment "the population is nobody's enemy; the ambience layer's civilians are
not a third army". The weld re-runs after every load: `_friendshipApplied` is
cleared on deserialize (`:2588`) and re-latched on the next `Simulate`
(`:1905-1908`).

**Friendship is per-centre, one-directional per call, and not symmetric.**
`AICenter::SetFriendship` (`AI/AICenter.hpp:270-273`) writes `_friends[side]`
on *that* centre only; `IsEnemy` is `_friends[side] < 0.6`
(`AI/AICenterStats.cpp:1425-1438`); every consumer reads the **observer's own**
centre. Dropping the civilian centre's friendship would turn every civilian on
the island hostile at once - the opposite of "sparingly". And it could not even
be selective: **every** centre hardcodes `_friends[TCivilian] = 1.0` at init
(`AI/AICenterImpl.cpp:1747`, `:1760`, `:1767`), so no matrix edit reaches one
civilian.

Asymmetry is a *tool*, though: the six writes below are deliberately chosen per
direction.

### 2. The third war side is unclaimed - and a ceasefire by default

The doc comment above the weld (`ZoneRegistry.cpp:557-575`) says it outright:

> WEST and GUER are ALLIES out of the box: `ArcadeIntel::Init` seeds
> `friends[TWest][TGuerrila] = friends[TGuerrila][TWest] = 1.0` [...] Both
> Guerrilla templates ship an empty `class Intel {}`, so those defaults apply
> verbatim [...] **The third war side is left exactly as the template set it.**

That is both the opening and the trap. The slot is free, but an assailant
parked on it with no explicit weld is the player's *ally* on Abel.

### 3. The spare slot must be computed, and the helper is nearly there

`ResolveSideCollisions` (`ZoneRegistry.cpp:423-479`) already pins the
resistance to the player's side (`:432-446`) and steps the occupier off it
(`:448-476`), and already calls `FirstFreeWarSide(avoid)`
(`Game/Guerrilla/FactionTwins.cpp:74-85`) - which scans
`{"GUER", "WEST", "EAST"}` avoiding **one** side. #42 needs the two-argument
form. With that scan order the answer is right on both shipped campaigns:
Abel (occ EAST, res GUER) -> **WEST**, which is what #2 reserved; Lebanon80
(occ WEST, res EAST) -> **GUER**. Hardcoding WEST, as #2 assumes, would put
Lebanon80's rogues on the IDF occupier's own side.

### 4. A unit's *perceived* side is fixed at creation - `join` does not move it

This is the correction #42's own text needs. Observers read
`Person::GetTargetSide()` -> `EntityAI::GetTargetSide()` ->
`Vehicle::GetTargetSide()`, which returns the plain field `_targetSide`
(`World/Entities/Vehicles/Vehicle.hpp:298`, `:407-408`), itself seeded from the
**class** config at `AI/VehicleAI.cpp:1154`. Every writer is a **creation** path
or vehicle-crew inheritance:

```
Game/Commands/GameStateExtWorld.cpp:191    createUnit -> SetTargetSide(center->GetSide())
AI/AICenterImpl.cpp:1004-1011              editor / mission.sqm spawn
AI/AICenterImpl.cpp:1506                   crew spawn
World/WorldImpl.cpp:1793                   world spawn
Network/NetworkClient.cpp:349,371          respawn re-applies the saved side
World/Entities/Vehicles/TransportCrew.cpp:1063..1246   crew inherits its driver's
```

The SQF `join` operator (`ListJoin`, registered `GameStateExt.cpp:1322`; body
`ProcessJoinGroups`, `GameStateExtGrp.cpp:624-826`) does
`ForceRemoveFromGroup()` + `grp->AddUnit(...)` (`:659-661`) and **never touches
`_targetSide`**. Neither does `AIGroup::AddUnit` (`AI/AIGroup.cpp:947-1013`) or
`AIUnit::ForceRemoveFromGroup` (`AI/AIUnitImpl.cpp:152-162`).

So joining a live civilian into a third-force group changes **whom he attacks**
(`HowMuchInteresting` reads his group's centre, `AI/AIUnitImpl.cpp:1126-1129`)
but not **who attacks him** (observers read his `_targetSide`, still
`TCivilian`, which every centre is welded friendly to). The issue's "move him
into a group on a different center" is half a mechanism.

### 5. Scripted firing cannot cross the friendship gate

`doFire` is dropped by the `IsEnemy` gate - `shakedown.sqs:245-246` records
this as settled: *"the aim pause: doFire would be ignored (IsEnemy gate) - this
is the mandated `doTarget`/`doWatch` + `setDammage` beat"*. An abstract
scripted execution can be faked; a **firefight** cannot. Real diplomacy is
mandatory, not a nicety.

### 6. The class-vs-instance split gives the disguise for free

`EntityAI::GetTargetSide(float accuracy)` (`AI/VehicleAIDiag.cpp:1165-1193`,
mirrored by the `!sideChecked` fallback at `World/Detection/Target.cpp:1101-1104`):

- side accuracy **< 1.5** -> `type->_typicalSide`, i.e. the **classname**'s side;
- side accuracy **>= 1.5** -> the **instance**'s `_targetSide`.

Keep the civilian classname and a rogue reads as a civilian at range and
resolves to hostile only on positive ID. Free, and exactly the wanted feel.
There is even a built-in hesitation beat: `Target.cpp:971-976` downgrades a
friendly-or-civilian reading whose real side is hostile to
`TSideUnknown, checked` at accuracy >= 1.35 - the "stolen vehicle" rule -
which is worth +10 interest, i.e. *investigate, don't shoot*
(`AI/AIUnitImpl.cpp:1121-1124`).

`tests/integration/scripting/guerrilla_civ_disguise_fire.test.sqf` (issue #25
M3.3) already asserts this whole ladder end to end: a plainclothes body
`createUnit`'d into a **war-side** group reads CIV at ~150 m, resolves to its
true side up close, and **fires**.

### 7. There is a per-unit "hostile to all", and it is not needed here

`EntityAI::GetTargetSide()` (`AI/VehicleAIDiag.cpp:1145-1162`):

```cpp
if (unit->GetPerson()->GetExperience() < ExperienceRenegadeLimit)
{
    // he is crazy - shooting at friendlies - enemy to all sides
    return TEnemy;
}
```

and `AICenter::IsEnemy` returns `true` for `TEnemy` **before** consulting
`_friends` (`AICenterStats.cpp:1428-1431`). `ExperienceRenegadeLimit` is the
config key `renegadeLimit` (`AI/AICenter.cpp:299`); `addRating`
(`GameStateExt.cpp:1253`) reaches it from script; rating lives on
`Person::_info._experience` (`World/Entities/Infantry/Person.hpp:51`) and
serializes with the body. mod-plan 14 (`14-occupation-systems.md:121`) already
flags it as the cheap per-unit route.

**It does not solve #42 on its own** - it changes only how a rogue is *read*,
never whom he attacks, and `doFire` will not cross the gate (fact 5). Once the
third force is welded (below), it adds nothing: the weld already makes both
armies hostile to him. Keep it in the toolbox for a future one-off assailant
that must not spend the side slot, and note the side effects if it is ever
used: it suppresses friendly-fire cease-fire radio
(`AI/VehicleAICombat.cpp:248`, `:307`) and changes kill accounting
(`AI/AICenterStats.cpp:319`, `:483`).

### 8. Kill attribution punishes the player if the weld is skipped

`EntityAI::Destroy` passes `origSide = Entity::GetTargetSide()`
(`AI/VehicleAICombat.cpp:400-402`) - the **base** accessor, deliberately
skipping the renegade override - into `AIUnit::IncreaseExperience`
(`AI/AIUnitImpl.cpp:2328-2360`), which picks `ExperienceDestroyCivilian` when
`origSide == TCivilian` and `ExperienceDestroyFriendly` when the **killer's**
centre does not consider that side an enemy.

**Consequence, and it is a hard requirement:** unless the third-force side is
welded hostile *in both directions*, the player takes a friendly-kill (Abel:
WEST is his ally) or civilian-kill rating hit every time he defends himself
against a rogue - and a long campaign of that walks the player himself toward
`renegadeLimit`. The weld is not decoration. (The MP/stats path already treats
a renegade kill as an enemy kill, `AICenterStats.cpp:318-322` - the two paths
disagree, and only the weld fixes the one that matters.)

### 9. A runtime-created centre has **uninitialized** friendship

`AICenter::AICenter` (`AI/AICenterStats.cpp:992-1016`) never touches
`_friends[]`, and the array has no default member initializer
(`AI/AICenter.hpp:197`). It is filled only by `AICenter::BeginArcade`
(`AI/AICenterImpl.cpp:1735-1767`), which the world runs for the five
template-time centres (`World/WorldInit.cpp:592-645`). `World::CreateCenter`
(`World/WorldSetup.cpp:181-...`) is a bare `new AICenter(side, mode)` with no
`Init`.

And the free `CreateCenter` used at template time returns `nullptr` in
`AICMArcade` unless the template has a group on that side
(`AICenterStats.cpp:1356-1377`) - Guerrilla.Abel's `mission.sqm` seeds only a
GUER group, so **the WEST centre does not exist at boot**. It is created on
demand by `EnsureSideCenter` (`ZoneRegistry.cpp:80-104`) or the SQF
`createCenter`, both of which land in the uninitialized path.

Two consequences:
- the weld must write **all four** slots on the third-force centre before any
  group on it takes a first `Think`, and `GetFriendship` is unguarded
  (`AICenter.hpp:269`) so a stale read is UB, not merely wrong;
- **latent bug found on the way:** today's `ApplyCampaignFriendship` writes only
  three of four slots (`:609-622`), so a runtime-created occupier or resistance
  centre already carries an indeterminate third-war-side friendship. The #42
  weld closes that as a side effect.

### 10. Budget

`MaxGroups` is per-centre and config-derived
(`Core/Config/Configuration.cpp:343-350`: `GroupNameList.letters` x
`GroupColorList.colors` = 21 x 3 = 63 on stock CWA data);
`MAX_UNITS_PER_GROUP` is 12 (`AI/Path/AITypes.hpp:31`). Every overflow site is
a soft refusal: `CreateSideGroup` returns `nullptr` (`ZoneRegistry.cpp:113`),
`createGroup` returns `grpNull` (`GameStateExtWorldConfig.cpp:697`), a
`grpNull` join is silently dropped (`GameStateExtGrp.cpp:711-714`).

The reaping idiom is already in the tree - `Traffic::FreeSpentGroup`
(`Game/Guerrilla/Traffic.cpp:3577-3625`): *"a group whose every unit is dead
still holds its slot in the side's center (dead units stay group members until
their bodies are deleted, and MaxGroups counts them)"*. Copy it.

### 11. The ambient roster refuses persistent combatants

`civilians.sqs:74-86` is a binding rule: a fighter must **never** sit in
`GM_CIV_GROUPS`, because the 1200 m cache, the panic despawn, the 8-group cap
(`GM_CIV_MAX_GROUPS`, `:57`) and the 60 s wander `doMove` will each
independently destroy or derange him. A converted assailant therefore *leaves*
the ambient roster - which is also what makes him ineligible for further
extortion, for free.

### 12. Undercover does not protect the player from a rogue - and must not

`UndercoverSystem::AppliesTo` (`Game/Guerrilla/Undercover.cpp:231-246`) ends:

```cpp
// only the occupying force evaluates the disguise; everyone else keeps
// the vanilla captive result
return (int)center->GetSide() == _occupierSide;
```

with `_occupierSide` cached from `registry.OccupierSide()` (`:207`). A
third-force group is not the occupier, so it never runs the disguise ladder and
falls back to vanilla - where a captive unit reads `TCivilian` at **every**
accuracy (`VehicleAIDiag.cpp:1174`, `:1181`; `Target.cpp:1108`). And
`undercover.sqs` keeps `setCaptive true` for the whole campaign.

**Therefore the third force must be hostile to `TCivilian` as well**, or rogues
would never engage the player at all. That is not a compromise, it is the
issue's own words - "hostile to everyone, loyal to nobody" - and it settles the
trap the issue raises from the other end: a rogue does not respect your cover
because he does not respect anything, and no `gmBreakUndercover` is involved.

## The design

**Diplomacy is static; only membership moves.** The third-force side is welded
once per campaign load, in the same place and the same shape as the existing
weld. Nothing about the civilian centre changes - "the population is nobody's
enemy" stays true, because a rogue is no longer on the population's centre. At
runtime the only thing that happens is that bodies are created into, and reaped
out of, a small pool of groups on a side whose diplomacy never moves.

That ordering is what keeps #42 clear of the #24 class of bug. The seam is
already hardened anyway: the three hand-back sites now erase with the **booked**
category from `AITargetInfo::_exposureEnemy`
(`AI/AICenterImplPreview.cpp:643`, `:739-743`, `:828-833`; field documented
`AI/AICenter.hpp:37-43`; fix commit `a94430a`), and the comment at `:637-642`
names "campaign friendship weld, `setFriend`" as covered cases. The plan still
asserts a clean log (see Tests).

Conversion is a **body swap**, not a runtime side flip: delete the civilian and
create the assailant of the same class, at the same spot, into a third-force
group. The creation path sets `_targetSide` correctly (fact 4), and the delete
runs the ordinary `AICenter::DeleteTarget` hand-back rather than mutating an
already-tracked record.

```
; roughly, inside the assailant service
_class = typeOf _victim                      ; or the CIV descriptor's rogueClass
_pos   = getPos _victim
_dir   = getDir _victim
_victim removeAllEventHandlers "killed"      ; shakedown.sqs:259 - no ledger double-count
deleteVehicle _victim                        ; also drops him from GM_CIV_GROUPS on the next prune
_u = _grp createUnit [_class, _pos, [], 0, "FORM"]   ; _grp is on the third-force side
_u setDir _dir
[_u] call GM_ROG_fnArm                       ; descriptor weapon + magazines
```

Same class, same position, same facing, on the frame the civilian pulls a
weapon: it reads as the draw, not as a pop.

### Rejected

- **Drop the civilian centre's friendship.** Turns the island hostile at once,
  and cannot be selective (fact 1).
- **`join` the victim into a third-force group without a swap.** He fights, but
  nobody fights back - `_targetSide` is creation-time (fact 4).
- **A new native binding for `Vehicle::SetTargetSide`.** Would work - a live
  flip flows through `AICenter::UpdateTarget`, which erases the exposure by its
  booked category at `AICenterImplPreview.cpp:828-833` before writing the new
  side - but it is a runtime mutation of an already-tracked target for no gain
  over the swap, and it is unsafe for a unit that has gone out of the sensor
  pipeline (a booked exposure then waits out the 450 s `ExposureTimeout`).
- **Renegade rating alone.** Fact 7.
- **Script-only, no C++.** Technically possible: `setFriend` is script-reachable
  (`CenterSetFriend`, `GameStateExtWorldConfig.cpp:665-690`), the side strings
  are exposed, and the weld runs once per load so a script `setFriend` stays
  authoritative afterwards. Rejected because it duplicates `FirstFreeWarSide`
  in SQS, leaves campaign diplomacy owned by two places, races the weld's own
  first tick after a load, and cannot fix the uninitialized-`_friends` hazard
  (fact 9) before the first `Think`. Kept as the **Stage 0 spike**, where none
  of those objections apply.
- **A fifth engine side.** Out of scope per #2; a days-class
  enum/network/save audit.

## Engine work (small, ~80 lines + tests)

1. **`SpareWarSide(const char* a, const char* b)`** in
   `Game/Guerrilla/FactionTwins.{hpp,cpp}`, beside `FirstFreeWarSide`
   (`:74-85`): same `{"GUER","WEST","EAST"}` scan, skipping both, never
   offering `CIV`. Returns `RString()` when the two arguments do not cover two
   distinct war sides. Pure, so it unit-tests with no world.
2. **`ZoneRegistry::_thirdForceSide`**, resolved at the end of
   `ResolveSideCollisions` (`ZoneRegistry.cpp:478`, before
   `RebindFactionSides()`), and given a defined value on the legacy path where
   `_tuning.playerSide` is empty and the function returns early (`:427-430`) -
   that early return must not leave the field stale across a reload, since
   `LoadFromConfig` re-runs on every save load pass. Accessor
   `ThirdForceSide()` beside `OccupierSide()` / `ResistanceSide()`
   (`ZoneRegistry.hpp:311-312`).
3. **Weld it** in `ApplyCampaignFriendship` (`ZoneRegistry.cpp:576-624`), after
   the existing pairs. All slots explicit, because `SetFriendship` writes one
   centre and a runtime-created centre starts uninitialized (fact 9):

   ```cpp
   // The third force: hostile to EVERYONE, allied to nobody - #42's
   // unassociated assailants.  Explicit on every slot: WEST/GUER default to
   // 1.0 (ArcadeIntel::Init), so an Abel rogue would otherwise be the
   // player's ally and killing him a friendly kill; and a center built on
   // demand by EnsureSideCenter never ran BeginArcade, so its _friends[] is
   // uninitialized until written here.
   TargetSide third = GetEnumValue<TargetSide>((const char*)_thirdForceSide);
   if (third >= 0 && third < TSideUnknown)
   {
       if (AICenter* thirdCenter = EnsureSideCenter(_thirdForceSide))
       {
           thirdCenter->SetFriendship(occ, 0.0f);
           thirdCenter->SetFriendship(res, 0.0f);
           thirdCenter->SetFriendship(TCivilian, 0.0f);  // fact 12: a captive
           thirdCenter->SetFriendship(third, 1.0f);      // player reads CIV
           occCenter->SetFriendship(third, 0.0f);
           resCenter->SetFriendship(third, 0.0f);
           civCenter->SetFriendship(third, 1.0f);        // the population does
       }                                                 // not fight back
   }
   ```

   Both refusal paths already in the function (`:587-602`) stay authoritative:
   if the campaign sides do not resolve, the third force is not welded either,
   and the script layer's probe (below) leaves the whole feature switched off.
   Update the doc comment - "The third war side is left exactly as the template
   set it" stops being true.
4. **`gmThirdForceSide`** nular in `ZoneRegistryCommands.cpp`, beside
   `gmOccupierSide` (`:385`) and `gmResistanceSide` (`:386`), registered in
   `INIT_MODULE(GuerrillaZoneRegistry, 3)` at `:382-404`; `""` when unresolved.
   Document it in ARCHITECTURE.md A.3.

Deliberately **not** in the engine: rogue selection, caps, arming, reaping, the
fight-back reaction. Those are policy and belong in SQS, per A.2.

## Script work (the bulk)

Two new files, in the `civilian_interaction*` mould:

| File | Role |
|---|---|
| `core/scripts/assailants.sqs` | the director: rogue roll, pool, reaping, caps, journal |
| `core/scripts/assailants_lib.sqs` | synchronous conversion service + the `GM_CI_ON_RESIST` consumer |

Both must be added to the manifest table in **ARCHITECTURE.md A.6** and to
`kScripts[]` in
`tests/unit/engine/Poseidon/Game/Guerrilla/test_mission_script_core.cpp:52-70`
in the same commit - the unit lane asserts directory, doc and script references
all agree.

### Boot and no-op probe

Follow `shakedown.sqs:28-31`: repeat the CIV-descriptor probe verbatim rather
than depending on `GM_CIV_READY`, and add the third-force gate. Note the
`createCenter` - `createGroup` returns `grpNull` on a side with no centre
(fact 9), and `civilians.sqs:91` already does the same for CIV:

```
@GM_LIB_READY
? (gmFactionValue ["CIV", "civClassCount"]) == "" : exit
? gmThirdForceSide == "" : exit           ; unresolved sides: feature off, one journal line
GM_ROG_SIDE = gmThirdForceSide call GM_fnSideFromString
GM_ROG_C = createCenter GM_ROG_SIDE
GM_ROG_READY = true
```

Two-sentinel lib boot as in `civilian_interaction.sqs:5-7,20`.

### State (seed unconditionally in `core/init.sqs`, beside the civilian block at `:150-193`)

| Global | Shape |
|---|---|
| `GM_ROGUE_GROUPS` | live third-force groups - the `GM_PSEUDO_GROUPS` twin (`init.sqs:165-168`) |
| `GM_ROGUE_UNITS` | live assailant bodies; the cap is counted here |
| `GM_ROGUE_ZONE` | parallel zone **NAME** per body (never an index - the `civilians.sqs`/`shakedown.sqs` rule) |
| `GM_ROGUE_BORN` | parallel spawn `time`, for the reap clock |
| `GM_ROG_LASTZ` / `GM_ROG_LASTT` | per-zone cooldown ledger, the `GM_SHK_LASTZ` pattern (`shakedown.sqs:79-80`) |
| `gmRogTicks` | loop-liveness counter, the save/reload observable |

### Tunables (the `civilians.sqs:52-69` comment-table style)

`GM_ROG_TICK` 10 s; `GM_ROG_MAX` **3** live assailants island-wide (hard cap);
`GM_ROG_PER_ZONE` 1; `GM_ROG_COOLDOWN` 900 s per zone; `GM_ROG_CHANCE` 0.02 per
eligible zone per tick, modulated by zone heat and war level the way
`shakedown.sqs:140` does; `GM_ROG_LIFE` 600 s before an unengaged rogue is
reaped; `GM_ROG_FORCE` / `GM_ROG_FORCEZONE` force hooks with the
`shakedown.sqs:87-96` consume-on-effect semantics.

Sparingly means sparingly: 3 island-wide against 8 civilian groups, one rogue
per town, a 15-minute town cooldown.

### Path A - fight back (#42 behavior 1)

Install the callback after `GM_CI_READY`:

```
GM_CI_ON_RESIST = {[_this select 0, _this select 2, "RESIST"] call GM_ROG_fnConvert}
```

`GM_ROG_fnConvert [body, zoneName, cause]` is synchronous and must not wait or
recurse (CIVILIAN-INTERACTIONS.md:121-124). It refuses - returning `false` and
leaving #41's plain refusal as the outcome - when the cap is full, the pool
cannot take a group, the class or weapon does not resolve, or the third force
is unresolved. **A refusal is a normal outcome, not an error**: #41 already
handles a resister who merely refuses to pay.

The resister is converted where he stands, arms himself and engages
immediately: he is the only assailant with a guaranteed initial target, so the
service `reveal`s the caller to the new group. This is a **local** exposure, not
a campaign cover break - it must not call `gmBreakUndercover`. Occupier
witnesses to the ensuing firefight break cover through the existing `fired`-EH
path in `undercover.sqs`, which is the right and already-tested route.

The extortion's support debit and journal line have already landed inside
`GM_CI_fnInteract` - #42 adds a `gmJournalNote` in the town's voice, kind
`danger`, and nothing else to the ledger, or the same act is priced twice.

### Path B - go rogue (#42 behavior 2)

The director loop, per tick, first eligible CITY zone wins (the
`shakedown.sqs:125-143` selection shape):

1. zone is `CITY`, not in `GM_PANIC_ZONES`, off its `GM_ROG_*` cooldown
   (two-step lookup - `and` evaluates both operands in this evaluator, see
   `shakedown.sqs:132-137`);
2. player within `GM_CIV_RADIUS` of the centre - rogue selection rides #41's
   stated budget, nothing computed for towns the player is not near;
3. `count GM_ROGUE_UNITS < GM_ROG_MAX`, and no live rogue already in this zone;
4. the roll, biased by zone heat and `gmWarLevel`;
5. pick a random live body from the zone's `GM_CIV_GROUPS` rows - the
   `shakedown.sqs:162` reservoir pick - and convert it with cause `"ROGUE"`.

A converted rogue gets `setBehaviour "COMBAT"`, `allowFleeing 0` and a wander
anchor on the town so he hunts locally instead of walking off the island. He is
**not** handed a target: he finds his own, which is the whole point - his
group's centre reads every other side as an enemy.

### Reaping (never leak groups)

- **One group per assailant** is affordable *because* the cap is 3: three
  groups on an otherwise-empty centre against a 63-group budget, and separate
  groups are what "unassociated" means - a shared group would make them a fire
  team with a leader. If `GM_ROG_MAX` ever rises above ~8, pool into one group
  (12-unit ceiling, `AITypes.hpp:31`) instead.
- Prune dead/null bodies every tick, `GM_fnCivPrune`-style
  (`civilians.sqs:136`), rebuilding all parallel arrays without the row, and
  `deleteGroup` the emptied group - a group whose units are all dead still
  holds its `MaxGroups` slot until the bodies are deleted
  (`Traffic.cpp:3577-3582`).
- Reap a rogue past `GM_ROG_LIFE` **and** outside `GM_CIV_DESPAWN_R` of the
  player: perception-gated, the traffic rule - never vanish a body in view.
- A `grpNull` from `createGroup` is a refusal, not a retry.

### Kill-ledger integration

`civilians.sqs:184-199` classifies a civilian kill by killer into
`OCCUPIER` / `PLAYER` / `PSEUDO` / `OTHER`. #42 adds a **victim-side** rule
*before* the killer classification: a victim whose group is in
`GM_ROGUE_GROUPS` is not a civilian atrocity - skip the support delta, the
resentment tick and the panic start, and post a `gmJournalNote` instead.
Killing an armed man who was shooting at the town is not a massacre, and the
engine's own stats path already agrees (fact 8).

Symmetrically, a rogue who kills a civilian **is** one. Those records arrive
through the ordinary `GM_fnCivKilledEH` path with a rogue killer and get a new
fifth class `GM_CIV_SUP_ROGUE` (small, e.g. -3): support drops, but it is not
blamed on the player. Panic still starts - a shooting in the street is a
shooting in the street, and the panic FSM emptying the town is exactly the
right self-limiting behaviour for a rogue who has run out of targets.

Note the conversion swap deletes a body carrying the killed-EH: strip it first
(`removeAllEventHandlers "killed"`, the `shakedown.sqs:259` beat) or the delete
reads as a death with no killer and demotes to `OTHER`.

### Persistence

- Third-force **diplomacy** needs nothing: `_friendshipApplied` is cleared on
  load (`ZoneRegistry.cpp:2588`) and the weld re-runs on the next tick.
- Assailant **bodies** are world objects and survive a save like any spawned
  unit.
- Assailant **script state** (`GM_ROGUE_*`) must be rebuilt after a load, not
  trusted: prune null links, re-resolve zones by name via `gmZoneIndex`. Hook
  the existing fan-out `campaignLoaded` handler at `core/init.sqs:262` -
  **do not register a second handler**, native events have one slot
  (CIVILIAN-INTERACTIONS.md:83-88 learned this the hard way).
- A body whose script row did not survive is reaped on the next prune, so the
  worst case is a rogue that stops being counted, not one that lives forever.

## Descriptor keys (CIV descriptor, all optional, clamped at boot)

| Key | Default | Range |
|---|---:|---:|
| `rogueClass` | the victim's own classname | classname |
| `rogueWeapon` / `rogueMagazine` / `rogueMagCount` | package-probed pistol, 2 | classname / 0..10 |
| `rogueMaxLive` | 3 | 0..8 (**0 disables the feature**) |
| `rogueChance` | 0.02 | 0..1 |
| `rogueCooldown` | 900 | 1..86400 s |
| `rogueLife` | 600 | 1..86400 s |

Read with `["CIV", "<key>", default] call GM_fnFactionNum` and clamped in the
`civilian_interaction_lib.sqs:36-45` shape. Classnames go through
`gmClassExists` and degrade per plan 15: an unresolvable weapon means the
service declines to convert (logging once), never a hard failure. No classname
or side literal may appear in the scripts (ARCHITECTURE.md §8).

## Order of work

- **Stage 0 - spike, no C++, ~1 h.** In a scratch mission: `createCenter` the
  spare side, `setFriend` it hostile both ways, `createUnit` a `civClass1` body
  into a group on it, arm it, watch. Answers the two questions that would
  invalidate everything downstream: **(a)** does a *pure civilian class* aim and
  fire, or does the civilian move family lack the combat states? If it does,
  `rogueClass` must default to the descriptor's plainclothes fighter - the
  `guerrilla_civ_disguise_fire` body - rather than to the victim's own class.
  **(b)** Do occupier garrisons engage a third-force body unprompted?
- **Stage 1 - engine.** `SpareWarSide`, `_thirdForceSide`, the weld,
  `gmThirdForceSide`, unit tests. Lands independently and is useful to #2.
- **Stage 2 - path A.** `assailants_lib.sqs` + the `GM_CI_ON_RESIST` consumer.
  The smaller, fully-specified half, and it closes the #41 loose end.
- **Stage 3 - path B.** The director, caps, cooldowns, reaping.
- **Stage 4 - ledger, journal, undercover assertions**, then docs.

## Tests

**Unit (Catch2, `[guerrilla]`)**
- `SpareWarSide` truth table: EAST/GUER -> WEST; WEST/EAST -> GUER;
  GUER/WEST -> EAST; duplicate or empty input -> empty. (Mirror the existing
  `FirstFreeWarSide` case, `test_zone_registry.cpp:1148-1155`.)
- `test_zone_registry.cpp`: a resolved campaign publishes a third-force side
  distinct from both, and the legacy empty-`playerSide` path leaves it defined.
- `test_mission_script_core.cpp`: the two new scripts in `kScripts[]`, named in
  ARCHITECTURE.md A.6, every `\gmcore\scripts\` reference resolving.

**Integration (Trident, `full_cwa` lane, `tests/integration/scripting/`)**
- `guerrilla_rogue_resist` - force fear below 20, extort, assert the outcome is
  `RESISTED`, a body exists on the third-force side, it is out of
  `GM_CIV_GROUPS`, it is armed, and `GM_CI_fnEligible` now refuses it; assert
  `gmUndercoverStatus` did **not** jump on the conversion itself.
- `guerrilla_rogue_hostile` - the headline claim, asserted rather than assumed:
  stage an occupier garrison group and a player-side companion near a forced
  rogue and assert each develops `knowsAbout` and engages. Add the captive-player
  case (fact 12): an undercover player *is* a target for a rogue.
- `guerrilla_rogue_budget` - force conversions past `GM_ROG_MAX`, assert
  refusals rather than growth, `count GM_ROGUE_GROUPS` bounded, and reaping
  past `GM_ROG_LIFE` out of sight.
- `guerrilla_rogue_ledger` - killing a rogue does not debit town support or
  start a panic; a rogue killing a civilian does both.
- `guerrilla_rogue_save.seq` - real save/load: diplomacy re-welds, rows rebuild,
  no double-count, cooldowns preserved.
- **Sinai/Lebanon80 lane** (`lobo`), non-negotiable: the reversed-side campaign
  is exactly where a hardcoded WEST passes on Abel and puts rogues on the IDF
  occupier's side. Mirror `guerrilla_civilian_interaction_sinai`.
- **Log assertion** on the hostile and save cases: zero `Illegal exposure`
  lines - the #24 regression guard for any new diplomacy.

## Open questions

- **Does the third force stay #42's?** #2 reserves the spare slot for a rival
  resistance / sponsored militia. A rival movement wants to be hostile to the
  occupier but *not* to the player - a different diplomacy on the same centre,
  and there is only one. If that feature lands, one of the two tenants must use
  the renegade lever (fact 7) instead of the side. Decide before #2 builds on
  this.
- **Rogues will shoot civilians**, because they must be hostile to `TCivilian`
  for a captive player to be a target at all (fact 12). Accepted, ledgered
  (`GM_CIV_SUP_ROGUE`) and self-limiting via panic - but it is the one place
  where a tuning knob might be wanted if play says otherwise, and the knob is
  not free: it would have to be a rogue-specific target filter, not a
  friendship value.
- **Fear has no writer.** `GM_CI_PROFILES[4]` is drawn once
  (`civilian_interaction_lib.sqs:60`) and never updated - nothing raises fear
  after a shakedown or a street killing. #42 works fine on the static draw, but
  "the town got scared after the raid" is a cheap separate follow-up worth
  filing.
- **Does a rogue block a capture?** He is not on the occupier side, so
  `ZoneRegistry::EvaluateTick`'s `liveOccupiers` accounting should ignore him -
  confirm rather than assume.
- **Voice.** A rogue conversion is a loud, memorable moment. `groupChat` in the
  town's voice at the draw, or silence? The shakedown director's three beats
  are the precedent.
