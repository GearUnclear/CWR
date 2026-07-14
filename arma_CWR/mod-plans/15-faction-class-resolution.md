# Plan 15 — Faction class resolution & squad diversity

> Status: IMPLEMENTED (2026-07-13) — resolution pass + squad composer +
> `gmFactionSquad`/`gmClassExists` landed in the engine, scripts and all four
> playable descriptors updated, unit tests green. The six-agent class survey
> (three unit-class + three dedicated vehicle-research agents, one per data
> package) is archived in `tmp/class-survey/*.md` — the authoritative
> inventories behind every classname below.
> Related: roadmap `dual-package-compat` (issues #11/#13 — the missing
> SoldierG* posture) and the squad-diversity item.

## Survey headlines (2026-07-13, six subagents)

- **Classic 1.99** (`classic-infantry.md`, `classic-vehicles.md`): full
  rosters on all four sides (64 Man classes; 100 drivable vehicles after
  merging the official AddOns configs — the core CONFIG.BIN alone lacks every
  Resistance-era vehicle). Zero missing classnames in the shipped
  descriptors. Safest per-side roots: `SoldierWB`/`SoldierEB`/`SoldierGB`/
  `Civilian`.
- **Demo [Remaster]** (`demo-infantry.md`, `demo-vehicles.md`): a strict
  subset — 35 Man classes, no GUER roster (sole survivor `SoldierGFakeE`,
  side-correct, createUnit-safe), no civilians at all, no WEST tank, no GUER
  or CIV vehicles. Second-order trap: some config-present classes are
  MODEL-absent (`M21`, `SVDDragunov`, `AK47`-base, `GeneralE`, `Angelina`) —
  fallback lists must avoid them. Ground-vehicle intersection with Classic:
  WEST `{Jeep, Truck5t, M113}`, EAST `{UAZ, Ural, BMP, T72}` (the EAST QRF
  ladder ports across packages unchanged).
- **@LoBo** (`lobo-infantry.md`, `lobo-vehicles.md`): 1,306 units; every
  IDF tier family carries full role siblings; Egypt Frontier Corps lacks
  medic/sniper (borrowed same-side army kin); Jordan is a genuine
  side-2 (GUER) regular army — the standout future resistance faction;
  requiredAddons metadata is unreliable (mount the whole modfolder); all
  armed vehicle picks depend on the @lobofixup patched LoBoammo pbos;
  Classic-lane only (JAM requires BIS_Resistance).

## Problem

Guerrilla Mode faction descriptors (`description.ext` →
`CfgGuerrillaFactions`) carry raw classnames. Three failure/quality gaps:

1. **Unknown incoming faction data is trusted verbatim.**
   `ZoneRegistry::LoadFactions` (ZoneRegistry.cpp:297) stores every string
   unvalidated. A classname the loaded data package doesn't ship is:
   - *fatal* when it sits in `mission.sqm` (strict-mode config error at boot —
     the `SoldierGB`-on-Demo 60 ms boot-fail, #13);
   - *silently sterile* at garrison spawn (`GarrisonCache::SpawnGarrison`
     drops empty groups, "retried next tick" forever — an invisible no-op
     occupation);
   - *silently sterile* in script consumers (`createUnit` of a bad class).
   The same descriptor schema must run on three very different packages:
   Classic 1.99 (full roster), Remaster Demo (no GUER roster, no civilians),
   and total-conversion mods (@LoBo — nothing vanilla present at all beyond
   inherited parents).

2. **Monoculture squads.** Every garrison/QRF unit is the single
   `FactionTierClass(side, warLevel)` pick plus one officer
   (GarrisonCache.cpp:418-458, qrf.sqs #qrfLaunch). No MG, no AT, no medic —
   neither realistic nor tactically interesting.

3. **Vehicle ladders are stubbed.** `vehicleThresholds[]` (full ladder
   support) already exists engine-side but every shipped descriptor uses the
   legacy 2-step `vehicleThreshold`, and the resistance factions field no
   vehicles at all.

## Design

### A. Resolution pass (engine, ZoneRegistry)

Run once per campaign, at the end of `LoadFromParams` after `ResolveSides`,
so the *selected* factions are known and the world config (`Pars`) is merged.

**Probe primitive:** `(Pars >> "CfgVehicles").FindEntry(name)` — cheap,
non-instantiating, non-fatal (same probe `ShellCreate` uses,
GameStateExtWorldConfig.cpp:1684). Weapons and magazines probe
`CfgWeapons` (OFP-era magazines *are* CfgWeapons entries).

For unit tests the probe is injected (`ClassProbe` functor) so
`LoadFromParams` stays data-driven-testable without a live config.

**Resolution order, per key kind** (first hit wins; every substitution is
logged with `RptF("GM faction '%s': key '%s' class '%s' unresolved -> '%s'")`):

| Key kind | 1st | 2nd | 3rd | last resort |
|---|---|---|---|---|
| `tiers[i]` | itself | nearest *lower* resolved tier | nearest higher resolved tier | side fallback probe list |
| role tiers (`tiersMG[i]`, …) | itself | resolved `tiers[i]` (monoculture for that slot) | — | — |
| `officer` | itself | resolved `tiers[0]` | side fallback | "" |
| `holdClass`/`recruit*`/`companionClass` | itself | resolved resistance `tiers[0]` | side fallback | "" |
| `vehicles[i]` | itself | **dropped** (ladder + thresholds compacted) | — | empty ladder is legal (qrf.sqs tolerates "") |
| `baseWeapon`/`loot*Weapon` | itself | resolved `baseWeapon` | "" | |
| `*Mag` keys | itself | resolved matching weapon key (OFP self-magazine convention) | "" | |
| `civClass<N>` | itself | another resolved `civClass<M>` | side fallback (CIV) | "" (managers already no-op on empty CIV) |

**Side fallback probe lists** (engine built-ins, probed in order; a
descriptor may prepend its own via a new plain key `fallbackClass`):

- `WEST`: `SoldierWB`
- `EAST`: `SoldierEB`
- `GUER`: `SoldierGB`, then `SoldierGFakeE` (the Demo's sole side-2
  survivor — scope=1 but createUnit-safe and model-complete), then
  `SoldierEB`, then `SoldierWB` (cross-side bodies spawned into GUER-side
  groups still *fight* as resistance, the same side-comes-from-group
  mechanism civilians.sqs relies on)
- `CIV`: `Civilian`, then `SoldierWCaptive` (the Demo's only unarmed public
  human)

(Lists verified against the survey inventories — all names scope-accessible
and model-complete in the packages that ship them; the Demo's
config-present-but-MODEL-absent classes like `M21`/`SVDDragunov` are
deliberately absent from the lists.)

An entirely unresolvable faction (empty tiers after the pass) leaves
`FactionTierClass` returning "", which every consumer already treats as
"system inert" — degraded, but never crashed.

**New script surface:** `gmClassExists "<classname>"` → bool
(CfgVehicles probe), so missions/tests/future Gate-Zero substitution can ask
the same question the engine does. The resolution summary is also queryable:
`gmFactionValue [side, "resolutionLog"]` is *not* added — the RptF log is
the record; scripts don't branch on it.

**Out of scope, noted:** `mission.sqm` classes (the actual Demo boot-fail)
are the mission's business, not the descriptor's. The posture there is the
per-package mission variant / Gate-Zero substitution tracked on #13; this
plan makes everything *descriptor-sourced* package-proof.

### B. Squad diversity (engine composer + role tiers)

**Descriptor schema** grows optional parallel role arrays, one entry per
tier, mirroring `tiers[]`:

```cpp
tiers[]       = {"SoldierEB",  "SoldierEG",  "SoldierECrew"};
tiersMG[]     = {"SoldierEMG", "SoldierEMG", "SoldierEMG"};
tiersAT[]     = {"SoldierELAW","SoldierELAW","SoldierELAW"};
tiersMedic[]  = {"SoldierEMedic", ...};
tiersSniper[] = {"", "", "SoldierESniper"};   // "" = role absent at this tier
```

Missing array, short array, or "" entry ⇒ that role resolves to the tier's
rifleman (`tiers[i]`) — fully backward compatible: existing descriptors keep
producing monoculture squads until they opt in.

**Composer** (pure, unit-testable):
`ZoneRegistry::FactionSquad(side, warLevel, count, out)` fills `out` with
`count` classnames using a fixed military template — for a group of n:

- 1 leader slot (the caller decides officer vs rifleman),
- MG: 1 per started 5 (cap 2),
- AT: 1 per started 6 (cap 2),
- medic: 1 when n ≥ 6,
- sniper: 1 when n ≥ 10 *and* the tier fields one (elite tiers),
- rest riflemen.

Deterministic interleave (leader, MG, rifle, AT, rifle, medic, …) so
partial/small groups still come out mixed. No RNG — same inputs, same squad
(save/load and test friendly).

**Consumers:**
- `GarrisonCache::SpawnGarrison` calls the composer per group instead of the
  single `tierClass` (officer-first behavior unchanged).
- New command `gmFactionSquad [side, warLevel, count]` → array of classnames
  for the script layer: qrf.sqs squad spawn and capture.sqs hold garrisons.
  New lib helper `GM_fnSpawnSquad [sideVALUE, classes[], pos] → group`
  (scripts stay classname-free; the array comes from the command).

**Realism guardrails ("only as makes realistic sense"):**
- Role ratios follow squad organization, not uniform randomness.
- Snipers only where the faction's tier authors one (regulars don't embed
  snipers in a checkpoint garrison).
- Elite tiers may deliberately author *fewer* roles (a Shayetet team is
  riflemen + MG, no medic split) — absence of a role array entry is a
  statement, and the composer honors it by substituting riflemen.
- Civilian diversity stays where it is (civClass1..N random pick,
  civilians.sqs) — no template applies.

### C. Vehicle ladders (data, from the vehicle survey)

Move every shipped descriptor to `vehicleThresholds[]` full ladders;
give resistances their phase-2 vehicles. Concrete per-package ladders come
from the three vehicle-survey reports (`tmp/class-survey/*-vehicles.md`);
realism ordering: patrol car → truck-mobile → APC → armor, thresholds spread
across war levels 1..10. qrf.sqs constraints hold: ground, AI-drivable,
no aircraft/boats/statics in `vehicles[]`.

## Implementation record (all landed 2026-07-13)

1. ✔ Engine: `ClassProbe` interface + `ParsClassProbe` + resolution pass in
   `ZoneRegistry::LoadFromParams` (probe param, engine path always passes
   one) + `gmClassExists`; ladder-compaction semantics: thresholds are the
   LADDER's gates, surviving hulls slide down into dropped rungs.
2. ✔ Engine: `tiersMG/AT/Medic/Sniper[]` parsing + `FactionSquad` composer
   (MG from 3 men / 2 from 9; AT from 5 / 2 from 11; medic from 6; sniper
   from 10 and only where the tier authors one; deterministic interleave)
   + `gmFactionSquad`.
3. ✔ GarrisonCache composes per group; officer-first behavior unchanged;
   role-less descriptors reproduce the pre-plan squads exactly.
4. ✔ Scripts: `GM_fnSpawnSquad` (lib.sqs); qrf.sqs squads via
   `gmFactionSquad` + the escort now mans its GUNNER seat (the vehicle
   survey's headline finding — the turret never fired before); capture.sqs
   hold squads role-mixed with holdClass monoculture fallback; synced
   byte-identical to all seven copies (parity green).
5. ✔ Data: role tiers + full `vehicleThresholds[]` ladders in
   Guerrilla.Demo / Guerrilla.Abel / Showcase.Abel (vanilla EAST+GUER incl.
   the GUER captured-vehicle pool) and Guerrilla.Sinai (IDF three-family
   role kit, HMMWV→Zelda→Magach 6→Shot-Kal; Egypt Frontier Corps medic/
   sniper kin + technical→BTR-60→BMP-1→T-55 pool).
6. ✔ Tests: 3 new unit test cases (resolution, composition, no-probe
   passthrough), 2302/2302 PoseidonTests green; Classic-lane guerrilla
   integration suite re-run — 6/7 green (including guerrilla_native_capture
   exercising the NEW gmFactionSquad hold-squad path in-game).
   guerrilla_native_spawn's doMove flake fails 3/3 on plain HEAD too
   (bisected 2026-07-14: engine/scripts/resolution all ruled out; see the
   roadmap log) — pre-existing environmental flakiness, not plan-15.
7. ✔ Docs + roadmap: ARCHITECTURE.md §A.3/§A.5/§6, STATUS.md rows 3b/4,
   roadmap item `faction-class-resolution` + `dual-package-compat` log.

**No longer owed (asset-targeting policy, 2026-07-14):** the Demo-world
tests' `mission.sqm` player class (`SoldierGB`) is not descriptor-sourced,
and making those tests run on Demo data was the last Demo-roster work item —
dropped. **Standing policy: Guerrilla content targets the Classic 1.99 +
@LoBo asset sets; the Remaster obligation is *future compatibility with the
full Remaster's assets*, carried by this plan's resolution pass** (missing
classes degrade with a logged substitution, never fatal or silently sterile;
descriptors must not hard-require assets the full Remaster is expected to
lack). The Demo-package fallback entries (`SoldierGFakeE`,
`SoldierWCaptive`) stay in the built-in side lists as that insurance.
