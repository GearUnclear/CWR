# Occupation Systems: the counterinsurgency state as a world generator

*Make the generated world read as occupied territory, and give the occupier a population-control playbook — checkpoints, curfews, reprisals, resettlement, informers, pseudo-gangs, Fireforce — that attacks the player's support economy, not just their squad. Drawn from historical COIN doctrine (Rhodesian Bush War, South African Border War / internal security state, Israeli occupation administration), expressed entirely through systems the engine already has. **Zero new animations required.***

> **Status:** design breakout from [13 — Guerrilla Mode](./13-guerrilla-mode.md) (the umbrella plan). Reads its zone/support/Heat/War-Level model as given and extends it. Same conventions: verified engine facts cite this source tree; factions stay swappable data packs, so everything here is setting-agnostic occupier *doctrine*, not a specific state.
>
> **Difficulty:** mostly intermediate SQF over plan-13's managers; one beginner-class C++ command (road-network query); no art, no animations.

---

## The design gap this fills

Plan 13's occupier currently escalates along exactly one axis: **military mass** (War Level swaps the spawn table, Heat sizes the QRF). Historical counterinsurgency states barely worked that way. Rhodesia, apartheid South Africa, and Israel each built an *administrative* machine whose primary weapons were:

- **Movement control** — checkpoints as permanent terrain, curfews, pass/permit systems, closed military zones, road closures that reshape how everyone travels.
- **Population control** — collective reprisals, house demolitions, forced resettlement (Rhodesian "Protected Villages", strategic-hamlet style), economic closure of restive towns.
- **Information control** — paid informer networks, and pseudo-operations (Selous Scouts, Koevoet's turned fighters) where state units masqueraded as guerrillas.
- **Signature rapid reaction** — Rhodesian **Fireforce**: helicopter-borne vertical envelopment of any contact, minutes after the first shot.

Two consequences for our design:

1. **The generated world should *look and route like* occupied territory** before the first shot is fired: flags over towns, wire and watchtowers, a checkpoint at every chokepoint, empty streets after dark. Occupation is legible as *terrain and rhythm*, not just as hostile units.
2. **The occupier should play on the same board as the player.** Plan 13 gives the player a support/HR/₽ economy; the occupier's escalation should attack *those variables* — closing a town's income tap, demolishing the house of a family that fed you, trucking a village's population into a fenced camp — not merely spawn an APC. This is also the anti-grind device: enemy pressure that changes the *map state* is felt even when the player wins every firefight.

And one load-bearing historical dynamic worth modeling honestly: **repression radicalizes**. Every crackdown in the record traded short-term control for long-term recruitment by the insurgency. Mechanically: occupier population-control actions cut the player's *immediate* income/HR from the affected town but schedule a delayed **resentment tick** (+support over the following period). This is both true to the source material and self-balancing — the occupier's strongest moves sow the player's future strength, so crackdown spirals do not dead-end the campaign.

---

## Engine reality (verified against this tree)

Everything below runs on commands already registered, plus **one** new command:

| Capability | Verdict | Evidence |
|---|---|---|
| Time of day / curfew clock | **Present** | `dayTime` (`Game/Commands/GameStateExt.cpp:861`), `skipTime` (`:1056`) |
| In-world ownership signaling | **Present** | `setFlagTexture` / `setFlagSide` (`GameStateExt.cpp:1227-1228`, handlers `GameStateExtUi.cpp:330,349`) — flag poles are stock objects |
| Building demolition (no animation) | **Present** | `setDammage`/`setDamage` (`GameStateExt.cpp:1243-1244`); destroyed buildings swap to native ruin models |
| Reprisal targeting | **Present** | `nearestBuilding` (`GameStateExt.cpp:959`) |
| Depopulation / resettlement | **Present** | `deleteVehicle` (`GameStateExt.cpp:1028`) + `createUnit` elsewhere |
| Sentry posture without anims | **Present** | `setUnitPos` (`GameStateExt.cpp:1307`) |
| Checkpoints/watchtowers/wire as objects | **Present** | `createVehicle` of stock static classes (fortifications, wire, towers ship in retail data) |
| Undercover interaction | **Present** | `setCaptive` + `knowsAbout` (per plan 13); since 2026-07-16 also the native per-observer `UndercoverSystem` (`Game/Guerrilla/Undercover.*`) with tunable knobs (`undercoverForgetSeconds`, war-level detect scaling) |
| **Road-network query from script** | **Missing — add it** | `RoadNet` exists engine-side (`World/Terrain/Roads.hpp`, consumed by `AI/Path/PathPlanner.cpp`) with **no** evaluator command; only `RoadSurfaceYAboveWater` is touched from the command layer (`GameStateExtWorld.cpp:173`). New command in the plans-09/10 cost class |

The new command — working name **`nearestRoads <pos> <radius>`** (returns road-segment positions/directions) — is what lets checkpoint placement, road-cut berms, and convoy road-snapping work on *any* island automatically, which the swappable-islands core requirement demands. It joins the evaluator-QoL list in plan 13's Phase 1.5.

**Animation constraint, stated once:** nothing below needs a new gesture. Searches, arrests, ID checks, and detention are rendered *abstractly* — units halt, face the subject (`doWatch`/`doStop`), hold a stance (`setUnitPos`), a timer runs, and the outcome lands as radio/`sideChat` text plus state change (`setCaptive false`, Heat spike, teleport-to-cell). The three proof-point mods of this era (plan 13) sold far more with less.

---

## System 1 — Control infrastructure: the world-gen layer

*Historical anchor: the Israeli fixed-checkpoint grid; Rhodesian martial-law zones; SADF base architecture.*

At campaign generation (and regenerating as War Level rises), the occupier stamps a **control layer** onto the island:

- **Fixed checkpoints** at road chokepoints between zones — auto-placed via `nearestRoads` at the road segment nearest each zone-graph edge midpoint. A checkpoint = 2–4 statics (wire, barrier, sandbags), one garrison fireteam (counts against zone garrison, so it's a real cost), a search trigger.
- **Watchtowers + wire** at occupied-town edges and around bases (`createVehicle` stock statics; sentries with `setUnitPos "UP"` and guard waypoints).
- **Flags everywhere ownership matters** — flag pole + `setFlagTexture` (occupier colors from the faction descriptor) on every occupied zone; swapped on capture. This retroactively strengthens plan 13's "liberation must visibly change the world" rule: territory is readable *in-world*, not only on the map. Cheap and thematic: tearing the flag down can literally be the capture beat (flag poles support take-flag interactions natively).
- **Density scales with War Level** — WL1–2 sparse (a lazy occupation), WL3+ new checkpoints appear on routes the player has used (see System 8), WL5+ berms/road-cuts (spawned obstacle objects) close secondary roads entirely, funneling traffic through checkpoints.

Checkpoints are *terrain*, and therefore *player routing pressure*: the world generates differently at WL5 than WL1, and veterans read the map by its control infrastructure — exactly how people navigated these territories in reality.

## System 2 — Curfew and pass laws: movement control as a clock

*Historical anchor: apartheid pass laws / influx control; dusk-to-dawn curfews in Rhodesian TTLs and the territories.*

A per-zone **control grade** (LOW / MEDIUM / HIGH / CLOSED, driven by Heat + War Level) modulates the plan-13 undercover system:

- **Curfew** (`dayTime`-driven, e.g. 21:00–05:00 in MEDIUM+): any civilian-class unit — including the undercover player — outside a town after dark is *suspicious by default*: garrison `knowsAbout` gain is multiplied, and detection countdowns run at double speed. Night stops being the free-stealth channel and becomes a **tradeoff**: empty streets and darkness, but *presence itself is probable cause*.
- **Pass laws**: in HIGH-control zones, undercover status decays on a timer near garrisons even unarmed ("papers checks" — abstract, see the animation note). CLOSED zones (post-incident lockdown) treat any non-occupier presence as hostile — the free-fire zone.
- **Flying checkpoints** — the Heat-reactive counterpart to System 1's fixed grid: a temporary roadblock spawned on a road segment (again `nearestRoads`) between the incident region and the player's likely destinations, despawned when Heat decays. The world *tightens* after every action, visibly and locally.

All of it is triggers, timers, and multipliers on numbers plan 13 already tracks.

> **Update (2026-07-16):** plan 13's undercover is now a native per-observer perception system, which gives these sketches a natural engine hook instead of a script-multiplier hack: curfew and control grades can scale the engine's war-level detection boost, and pass laws map directly onto `undercoverForgetSeconds` (compromise decay) plus the per-observer evaluation already running near garrisons. Curfew, pass laws, and flying checkpoints themselves remain future work.

## System 3 — The population-control playbook: the occupier plays your economy

*Historical anchor: collective punishment and house demolitions; town closures; Rhodesian Protected Villages.*

A strategic **occupier director** (one SQF loop, same altitude as the player's economy tick) that spends occupier "attention" on moves against the *support map* rather than the player's squad. Escalating menu, gated by regional Heat:

| Heat | Move | Effect | Engine mechanics |
|---|---|---|---|
| Medium | **Closure** | Town's ₽/HR taps shut until Heat decays; control grade → HIGH | flip zone globals; spawn flying checkpoint |
| High | **Reprisal raid** | Squad sweeps the supportive town nearest the incident; 1–2 houses demolished; support −X *now*, resentment tick +Y *later* | patrol waypoints through town; `nearestBuilding` + `setDammage 1` → native ruins |
| Sustained high | **Protected Village** | Town depopulated: civilians deleted, HR tap moves to a **fenced resettlement camp** spawned near the regional base; town becomes a ghost town (CLOSED) | `deleteVehicle` civs; camp = wire/towers/statics + civ units inside; new zone entry |
| WL5+ | **Cordon sanitaire** | Berm/wire road-cuts sever a region's secondary roads | obstacle objects on `nearestRoads` results |

Why this is the best content-per-line system in the whole mode:

- **It generates missions without authoring missions.** A Protected Village *is* a liberation raid (break the wire, escort the population home → town reopens at +support). A demolished house *is* a revenge hook. A closure *is* a smuggling run.
- **It makes passivity legible** — ghost towns and wire where villages were is the anti-empty-sandbox device working for the *enemy* side.
- **The resentment tick** (delayed +support after each crackdown) keeps the strategic loop live: the occupier's strongest response is also the player's best recruiter, which is the actual history and the actual balance mechanism.

## System 4 — Fireforce: signature rapid reaction

*Historical anchor: Rhodesian Fireforce — Alouette-borne vertical envelopment, the war's defining tactic.*

Promote plan 13's "heli/spawn-on-arrival fallback" from pathing workaround to **WL3+ signature escalation**: on RED alert, the QRF arrives as a helicopter insertion — one heli orbits as eyes (its `knowsAbout` feeds the net per plan 08), troops unload at 2 landing points *behind and beside* the contact (stop-group doctrine: cut the retreat, then sweep). All waypoint choreography: `transportUnload` waypoints at offset positions, then a `doMove` sweep line toward last-known-pos.

This inverts the tactical loop's difficulty exactly the way the history did: ambushing was easy; **breaking contact before the Fireforce arrived** was the skill. The *Fade* beat gets a hard clock, and it doubles as the convoy-pathing risk mitigation plan 13 already wanted.

## System 5 — Pseudo-gangs: trust as a mechanic

*Historical anchor: Selous Scouts pseudo-operations; Koevoet's turned insurgents.*

At WL4+, some "resistance" patrols are occupier pseudo-teams: **resistance-faction classnames spawned into EAST-side groups**. Plan 13's faction architecture makes this *literally free* — the descriptor already decouples classname from side slot, and cross-side spawning is the same verified mechanism that lets any faction fill any slot.

They read as friendly at a distance (model/uniform), don't fire until close or until you're identified, and their kills near towns are *blamed on you* (−support: the historical purpose of pseudo-ops was exactly this poisoning of trust). Counterplay is systemic, not animated: a radio **challenge** `addAction` — real resistance groups respond with the day's countersign (`sideChat`); pseudo-teams answer wrong or go loud. Teaches paranoia at exactly the point in the campaign where the player has started to feel safe among "their own."

**Perception caveat (verified):** engine target identification resolves the *true* side as `knowsAbout` confidence builds, so the disguise fools the player, never friendly AI. Design the leak as the tell — pseudo-teams hold fire at range where ID confidence is low (plan [07](./07-ai-stance-concealment-spotting.md) tunes this), and a veteran companion raising his rifle at "friendlies" is the early warning the player learns to read.

> **Update (2026-07-16), partially superseded:** for the undercover *player* this is no longer true. The native `UndercoverSystem` overrides the perceived side per observing group at the target-tracking sites, so a disguise now fools the simulation itself, not just the player's eyes. Pseudo-teams are not yet undercover subjects (the built system is player-only, SP scope), so for THEM the vanilla true-side resolution above still applies and the hold-fire tell design stands; extending per-observer evaluation to pseudo-teams is the natural upgrade path when this system is built.

### The side-slot budget (verified)

The engine has exactly **four combat sides** — `TEast`/`TWest`/`TGuerrila`/`TCivilian` (`World/Scene/Object.hpp:19`; `TSideUnknown` must stay last — the diplomacy matrix is `_friends[TSideUnknown]`, `AI/AICenter.hpp:191`) — each with a hardcoded world center (`WorldInit.cpp:570-586`; a fifth is `Fail("invalid center")`, `WorldSetup.cpp:203`). Adding a true fifth side is a days-class audit (friendship array, centers, network messages, save format, `mission.sqm` sides, `ASAT*Detected` waypoint enums, map colors) — not planned.

But plan 13 assigns only three: GUER = player, EAST = occupier, CIV = population. **WEST is a spare full combat side** — own AI center, own ~63-group budget, and runtime-settable diplomacy via the already-registered `setFriend` command (`GameStateExt.cpp:1396` → `AICenter::SetFriendship`). Pseudo-gangs stay on EAST (they are occupier assets; EAST diplomacy is exactly right). Bank WEST for a possible **third force**, on-theme options: a *rival resistance movement* (the ZANLA/ZIPRA split — contests town support, occasional skirmishes), *sponsored militia terror* blamed on the resistance (the SA "Third Force"), or late-game foreign intervention. Also available free and per-unit: the **renegade** mechanism — units whose rating falls below config `renegadeLimit` (`AICenter.cpp:299`) are treated as hostile-to-everyone by combat AI, reachable today via `addRating` — right for blown agents or lone bandits without spending a side.

## System 6 — Tracker teams: contact has a tail

*Historical anchor: Koevoet's relentless spoor-tracking pursuit; Selous Scouts recon.*

After any RED contact where the player breaks away, a dedicated **tracker team** spawns on the contact site and follows a breadcrumb of the player's actual recorded positions (a script trail, sampled every ~30 s), moving fast, pausing at each node ("reading spoor" — a halt plus `sideChat`, no animation), and refreshing group `knowsAbout` on proximity. Escape is *procedural*, not just spatial:

- reach a supportive town and go undercover (crowd = broken trail),
- cross water / use a vehicle (trail nodes stop),
- or ambush the trackers — the classic counter-tracker gamble, at the cost of a fresh Heat spike and a new contact clock (System 4).

Synergizes with plan [06](./06-ai-memory-decay.md) (their memory decays slower than line troops) and gives the *Fade* beat a mid-game skill curve beyond "run 500 m."

## System 7 — Detention: the prisoner economy

*Historical anchor: administrative detention; the prison as the occupation's center of gravity.*

A fraction of named-companion "deaths" that occur under occupier control (near a garrison/checkpoint, or while undercover-broken) convert to **captured** instead: unit deleted, name moved to a detainee roster at the regional **prison zone** (a System-1-style compound, generated with the control layer). Effects:

- Permadeath keeps its teeth (conversion is a die roll, and detainees can be *transferred toward execution* at high WL — a rescue clock), but the cruelest RNG deaths gain an appeal process.
- **Rescue raids become the emotional flagship mission type** — attack the compound, reach the cells, walk your veteran out (spawn them at the cell on breach; `setUnitPos`/`disableAI`-frozen guards-and-prisoner staging until then; plan [09](./09-sqf-enable-ai.md)'s `enableAI` un-freezes them — the exact use case it was scoped for).
- Failed rescues feed the resentment loop: a botched raid gets prisoners transferred and the town that watched punished (System 3).

## System 8 — The occupation expands: pressure under passivity

*Historical anchor: outpost/settlement expansion into contested ground; the grid never sleeps.*

If a region stays quiet *and* player-untouched for too long, the occupier **improves its position**: a new outpost spawns in a contested zone (statics + garrison, a real new zone-graph node), a new fixed checkpoint appears on a road the player's trail data says they use, wire goes up around a previously open town. The map *degrades under inaction* — the mirror image of System 3's response to action. Between them, there is no null strategy: act and the world tightens reactively; wait and it tightens structurally. (Cap total occupier infrastructure by War Level so the island stays conquerable — same governor philosophy as plan 13's Heat cooldown.)

## System 9 — Informers: support as counterintelligence

*Historical anchor: the informer networks every one of these states ran as their primary intelligence organ.*

Make plan 13's per-town support scalar *legible through risk*: in low-support towns, 1–2 civilians are flagged **informers**. If one has line of sight to the player doing anything incriminating (armed, planting, meeting), a report fires after a delay — Heat spike, flying checkpoint, possibly a System-6 tracker, all *without* any garrison `knowsAbout` event. The player experiences hostile towns as *leaky* and supportive towns as *safe houses*, which retroactively gives the support stat a moment-to-moment tactical meaning beyond its economic one. Counterplay: intel missions to identify the informer (watch who visits the garrison), then turn (₽), intimidate (support risk), or kill (Heat + support risk) — the insurgency's dirtiest recurring decision, presented as a systems choice, not a cutscene.

---

## Phasing (mapped onto plan 13's roadmap)

| Plan-13 phase | Occupation systems that land there | Why |
|---|---|---|
| **Phase 1 (MVP)** | Flags on all three zones (capture beat = flag swap); curfew clock on the Village; one hand-placed fixed checkpoint on the Camp–Outpost road | Each is <1 day of SQF and makes the MVP *read* occupied; no new C++ |
| **Phase 1.5 (de-hardcode)** | `nearestRoads` command lands with the evaluator-QoL batch; control-layer generation (System 1) written against it; control grades + informer flags join the zone/faction data schema | Placement must be data-driven before a second island exists |
| **Phase 2 (vertical slice)** | Occupier director with Closure + Reprisal (System 3); Fireforce QRF (System 4); flying checkpoints (System 2) | The economy war and the signature escalation are the slice's "asymmetry you feel" |
| **Phase 3 (full mode)** | Protected Villages + cordon; pseudo-gangs; trackers; detention/rescue; expansion-under-passivity; informer missions | Each rides Phase-3 systems (shared intel, nemesis-lite, strategy UI) |

**Money-moment addendum (extends plan 13's acceptance test):** after the Outpost flips and War Level ticks, *the occupier answers on the strategy board* — the Village goes under closure, a flying checkpoint appears on the road out, and the next patrol is heli-inserted. If the player feels the world *tighten around* their victory inside the same 15 minutes, this plan is proven.

---

## Risks & gotchas

| Risk | Severity | Mitigation |
|---|---|---|
| **Checkpoint/static clutter costs frame budget** — every checkpoint adds objects + a fireteam | Medium | statics are cheap, AI is not: cache checkpoint garrisons aggressively (despawn beyond player radius, same DAC pattern as everything else); Gate-Zero-style measurement before density tuning |
| **Occupier director oscillation** — crackdown → resentment → support → crackdown could limit-cycle | Medium | resentment ticks are capped per-town and decay; director has a per-region action cooldown (same shape as Heat cooldown) |
| **Pseudo-gang friendly-fire confusion reads as a bug** | Medium | introduce via a scripted first encounter (radio warning from HQ at WL4: "the enemy walks in our clothes"); challenge/countersign action always available |
| **Building demolition permanence across save/load** | Low–Med | destroyed-building IDs into the persisted globals; re-apply `setDammage` on load (same rebuild-on-load rule as plan 13) |
| **Tracker breadcrumb trail leaks through save or teleport** | Low | trail is transient state — cleared on save/load and on fast-travel by design (fast travel *is* an escape) |
| **Tone: systems trivialize real repression** | — | keep consequences legible and costly on all sides (reprisals hurt, informer killings cost support); factions stay fictional/swappable data; the design's stance *is* the resentment mechanic — repression is modeled as self-defeating, which is the historical verdict |

---

## One-line summary

*Give the occupier the real playbook — checkpoints and curfews as terrain, closures and reprisals and resettlement as moves on the player's own support economy, informers and pseudo-gangs as poisoned trust, Fireforce as the clock on every ambush — so the generated world reads as occupied territory and every crackdown recruits for the resistance; all of it spawned objects, waypoints, timers, and text over plan 13's zone model, with one new road-query command and zero new animations.*
