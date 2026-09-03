# Authoring a Guerrilla faction pack

A **faction pack** is a `class CfgGuerrillaFactions` block that names the units,
vehicles, weapons and flag one order of battle fields. Ship one and your roster
appears in the OCCUPIER and RESISTANCE cyclers of the GUERRILLA new-game screen
**on every installed island**, with no edit to any mission template and no
engine change.

This document is the contract. Its worked example is a real, tested fixture:
`tests/fixtures/mods-factionpack/@udfaction/bin/config.cpp`, which passes the
lint in [Validation](#validation) below.

Related reading: the vanilla library
[`config/guerrilla-factions.hpp`](config/guerrilla-factions.hpp) is the
annotated reference roster; the @LoBo pack
[`config/lobo-factions.hpp`](config/lobo-factions.hpp) is the large real-world
example (installed into `@LoBo\bin\config.cpp` by
`tools/lobo/install-lobo-factions.ps1`); [`ISLAND-PACKS.md`](ISLAND-PACKS.md) covers the other half, islands.

---

## 1. Where a pack can live

The engine reads the faction table out of the **global config** (`Pars`) and out
of the **island template's `description.ext`**. Three places land in `Pars`, and
all three work identically:

| Location | Use it when |
|----------|-------------|
| An addon pbo's `config.cpp` / `config.bin` | The pack ships with the units it names. The normal case for a mod. |
| A mod folder's `bin\config.cpp` | The units come from someone else's addons and you are adding only the descriptors (this is what `install-lobo-factions.ps1` writes into `@LoBo\bin\config.cpp`). |
| A `#include` from `<GameDir>\bin\config-extra.cpp` | UD's own vanilla library, installed by `install-missions.ps1`. |

The engine merges `bin\config-extra.cpp` into the global config **last**, and
the text preprocessor resolves an `#include` against the including file's
directory, which is why `guerrilla-factions.hpp` sits beside `config-extra.cpp`
in `bin\`.

## 2. The merge rule

`Game/Guerrilla/FactionSources.{hpp,cpp}` builds the table every consumer reads
as the **union**:

```
Pars >> CfgGuerrillaFactions  U  <island description.ext> >> CfgGuerrillaFactions
```

- The **island wins** on a class-name collision, by **whole-class replacement**.
  There is no per-key merge: an island author who redeclares `IDF` gets exactly
  the `IDF` they wrote, key for key.
- Order is island classes first in their config order, then the global-only
  classes in theirs.
- Inheritance inside a source block (`class Pack : Base {}`) is flattened into
  the merged copy, so a consumer walking the entries sees every key the class
  resolves to.

Three consumers go through that one helper so they cannot drift: the new-game
menu (`GuerrillaNewGame::RefreshFactionsForIsland`), the mission
(`ZoneRegistry::LoadFromConfig`) and the player-body seam (`OutfitSelect`).

## 3. A minimal pack that works

This is the whole of `@udfaction/bin/config.cpp`, trimmed of comments. It
inherits vanilla bodies, so it needs no models of its own:

```cpp
#define private   0
#define protected 1
#define public    2

class CfgPatches
{
    class UDFaction
    {
        units[] = {"UDSoldier", "UDOfficer", "UDCivFighter"};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {"UDMiniPackage"};
    };
};

class CfgVehicles
{
    // Re-declared, not forward-declared: see Pitfall 1.
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land {};
    class Soldier : Man {};
    class SoldierWB : Soldier {};

    class UDSoldier : SoldierWB
    {
        scope = public;
        side = 1;
        displayName = "UD Fighter";
        model = "\mini\man.p3d";
        weapons[] = {"UDRifle"};
        magazines[] = {"UDRifleMag"};
    };
    // UDOfficer and UDCivFighter follow the same shape.
};

class CfgGuerrillaFactions
{
    class UDFaction
    {
        side = "WEST";
        tiers[] = {"UDSoldier", "UDOfficer"};
        tierThresholds[] = {4};
        civTier[] = {"UDCivFighter"};
        playerClassWarrior = "UDSoldier";
        playerClassCiv = "UDCivFighter";
        officer = "UDOfficer";
        vehicles[] = {"UDCar"};
        vehicleThresholds[] = {3};
        civVehicles[] = {"UDCar"};
        baseWeapon = "UDRifle";
        baseMagazine = "UDRifleMag";
    };
};
```

The **only** hard requirement is `tiers[0]` naming a class the loaded data set
carries. Everything else has a fallback (section 5) or is simply absent.

**The cycler label is the class name**, not `displayName`:
`GuerrillaListFactions` pushes the config class name. A `displayName` key is
stored and readable from script via `gmFactionValue "displayName"`, but the
player sees `UDFaction`. Name the class the way you want it read.

## 4. The key reference

`ZoneRegistry::LoadFactions` reads the arrays below by name; **every other
non-class, non-array entry** is stored verbatim as a named value and reaches
script through `gmFactionValue`. So an unknown key is harmless, and a
misspelled known key is silently inert.

### Arrays

| Key | Bank | Meaning |
|-----|------|---------|
| `tiers[]` | CfgVehicles | The garrison/QRF **rifleman** per war-level rung. Required. |
| `tierThresholds[]` | numeric | War levels at which the ladder steps up. `n-1` entries for `n` tiers. |
| `tiersMG[]`, `tiersAT[]`, `tiersMedic[]`, `tiersSniper[]` | CfgVehicles | Per-tier role variants, parallel to `tiers[]`. `""` = the tier fields no such role. For `tiersSniper` specifically, `""` **suppresses** the slot rather than substituting a rifleman. |
| `civTier[]` | CfgVehicles | The civilian-clothes AI ladder (guards, militia, part-time fighters). Gated by the same `tierThresholds[]`, clamped to its own length. |
| `vehicles[]` | CfgVehicles | QRF escort ladder. Rung `k>0` unlocks at `vehicleThresholds[k-1]`. |
| `vehicleThresholds[]` | numeric | The escalation gates for `vehicles[]`. |
| `civVehicles[]` | CfgVehicles | Ambient road-traffic hulls. In practice a CIV-descriptor key; no ladder, no thresholds. |

### Plain values

| Key | Bank | Meaning |
|-----|------|---------|
| `side` | string | `"WEST"`, `"EAST"`, `"GUER"` or `"CIV"`. **Defaults to the class name**, which is why the vanilla `class CIV` needs no `side` key to be filtered off the cyclers. |
| `sideTwin` | class name | The same roster authored on another side. See section 6. |
| `officer` | CfgVehicles | Squad-leader class for garrisons and QRFs. |
| `holdClass`, `holdCount` | CfgVehicles / int | Post-capture hold-garrison class and size. |
| `recruitFighter`, `recruitSpecialist` | CfgVehicles | What the Camp recruits. |
| `companionClass` | CfgVehicles | The named-companion body. |
| `playerClassWarrior`, `playerClassCiv` | CfgVehicles | Character-select outfit bodies. Section 7. |
| `recruitFighterCiv`, `recruitSpecialistCiv`, `companionClassCiv`, `holdClassCiv` | CfgVehicles | Civilian-outfit twins of the four keys above. |
| `fallbackClass` | CfgVehicles | Your own last-resort body, tried before the built-in per-side list. |
| `baseWeapon`, `baseMagazine` | CfgWeapons / CfgMagazines | Always-available fallback kit. |
| `loot<Role>Weapon`, `loot<Role>Mag` | CfgWeapons / CfgMagazines | Role loadouts, `<Role>` in `Rifleman`, `Medic`, `MG`, `AT`, `Sniper`. |
| `flag` | texture path | Town flagpole texture, looked up by the resolved **side** string. |
| `vehicleThreshold` | numeric | Legacy single-gate 2-step ladder. Prefer `vehicleThresholds[]`. |
| `civClassCount`, `civClass1..N` | int / CfgVehicles | **CIV only.** Numbered keys, not an array, because `gmFactionValue` skips array entries. |

**OFP magazine convention:** a simple weapon **is** its own magazine class
(`CfgWeapons AK47` has `scopeMagazine=2`), so `baseMagazine` and the `loot*Mag`
keys legitimately reuse the weapon classname. There is no `AK47Mag` in the 1.99
data set.

The resolution pass probes each key against the **right bank**: `vehicles[]`
against `CfgVehicles`, `loot*`/`base*` against `CfgWeapons`. That is why the
vanilla WEST block can name `M60` in both `vehicles[]` (the tank) and
`lootMGWeapon` (the machine gun) without collision.

## 5. What happens when a class is missing

`ZoneRegistry::ResolveFactionClasses` runs on every load. Unknown classnames
never reach a spawn: they are substituted or dropped, and every change is
logged as `ZoneRegistry: faction '<X>' key '<k>': class '<c>' not in the loaded
data package - using '<sub>'`. Absent keys keep their semantics; only
**present-but-unresolvable** values are rewritten.

The **side fallback** is computed first: your `fallbackClass` if it resolves,
else the first entry of the built-in list for the descriptor's side.

| Key family | Ladder |
|------------|--------|
| `tiers[]` | nearest resolved **lower** rung, then nearest higher, then the side fallback. An all-unresolved ladder with no side fallback **empties** (systems go inert rather than spawning wrong bodies). |
| `civTier[]` | nearest lower, then higher, then the civilian-outfit list (`SoldierGFakeC`, `SoldierGFakeC2`, `Civilian`), then the already-resolved `tiers[0]`. Empties if nothing lands. |
| `tiersMG/AT/Medic/Sniper[]` | the entry blanks to `""`, and query time falls back to that tier's rifleman. |
| `vehicles[]` | the rung is **dropped** and the ladder compacts; `vehicleThresholds[]` is truncated to `count-1` so the surviving hulls slide down and the authored pacing is preserved. |
| `civVehicles[]` | dropped, no compaction needed. An empty result leaves civilian traffic inert. |
| unit keys (`officer`, `holdClass`, `playerClass*`, the `*Civ` twins, ...) | `tiers[0]`, else the side fallback. |
| `civClass<N>` | the first `civClass<N>` that did resolve. If none resolve, `civClassCount` is forced to 0 and the civilian layer soft-disables. |
| `baseWeapon` | blanks to `""` (it anchors the rest, so it is resolved first). |
| `loot*Mag`, `baseMagazine` | the sibling weapon key (`lootMGMag` tries `lootMGWeapon`, `baseMagazine` tries `baseWeapon`), then `baseWeapon`. |
| other weapon keys | `baseWeapon`. |

Built-in per-side lists: WEST `SoldierWB`; EAST `SoldierEB`; GUER `SoldierGB`,
`SoldierGFakeE`, `SoldierEB`, `SoldierWB`; CIV `Civilian`, `SoldierWCaptive`.

## 6. `side` and `sideTwin`

Sides and rosters are separate axes.

The campaign's **resistance side is welded to the template's `mission.sqm`
player**: the engine counts resistance presence out of that side's center, so a
resistance on any other side would be a resistance the player is not part of.
`ZoneRegistry::ResolveSideCollisions` therefore:

1. **Pins the picked resistance roster onto `playerSide`.** If the roster has a
   `sideTwin` chain member already declared on that side, that twin is used (the
   config-clean path). Otherwise the descriptor's side is overridden, logged at
   INFO.
2. **Steps the occupier off the resistance's side** when the two collide: first
   a `TwinOffSide` chain member, else the first free war side of
   `{GUER, WEST, EAST}`. CIV is never a landing site (the civilian center is
   friendly to everyone, so a rebased occupier would never fight).
3. **Rebinds the records** so the faction table agrees with the resolved sides,
   including cloning the roster when one descriptor was picked for both cyclers
   (a mirror match: both sides then field the identical order of battle).

So `sideTwin = "<OtherClass>"` means "the same order of battle, re-authored on
another side; substitute freely". The chain walk is bounded at 8 hops, and a
`sideTwin` naming a class the config does not carry ends the walk empty instead
of aborting. Author `sideTwin = "";` explicitly when your roster genuinely has
no re-sided variant, as @LoBo's IDF does.

A template that authors **no** `playerSide` disables the whole pass and behaves
exactly as it did before it existed; the menu blocks a same-side pair instead.

## 7. Outfit keys

`Game/Guerrilla/OutfitSelect.hpp` owns the player-body seam. Precedence at
launch:

1. An explicit **BODY browser** pick (`gmSelPlayerClass`, any side's CfgVehicles
   classname) wins outright.
2. Otherwise a `CIVILIAN` outfit token resolves `playerClassCiv` of the
   resistance descriptor (`ResolveCivilianPlayerClass`).
3. Otherwise (`WARRIOR`, or no token) the **resistance pick** substitutes its
   `playerClassWarrior` (`ResolveWarriorPlayerClass`).

`ResolveWarriorPlayerClass` returns empty, keeping the template's authored
class, when: nothing was picked; the pick names no block (WARN); **the pick IS
the template's default resistance** (its warrior key documents the authored
class, so there is nothing to do and nothing is logged); the block authors no
`playerClassWarrior` (WARN); or the class fails the probe (WARN).

On any failure the **authored `mission.sqm` class is kept**. The seam never
invents a substitute body. That is stricter than the resolution pass in section
5, on purpose.

Two extra gates apply to a player body and not to an AI body:

- **The shape gate.** A `model[]` naming a `.p3d` the package does not ship
  access-violates in `Man::Init` during creation, so `PlayerBodyModelIssue`
  refuses such a class. Give every outfit class a model the package actually
  carries.
- **Config side is kept for identification.** A substituted body keeps its
  config side for distant identification (the stolen-uniform band), while the
  instance side stays the mission side. Any body class fights as resistance.
  This is documented behaviour, not a bug.

## 8. `class CIV` is island-owned

**Never ship a `CIV` block in a faction pack.** The civilian descriptor names
the population models and ambient-traffic hulls of *one island's data set*
(`Civilian`/`Civilian2`/`Civilian3` on the stock islands, `LoBo_Civ_*` on the
@LoBo worlds). A global CIV would be wrong on every island whose package lacks
those classes, and the union rule cannot help: an island that wants a different
population has to redeclare the whole class anyway.

Each island template's `description.ext` therefore carries exactly one faction
class, `CIV`. A unit test pins this.

The cyclers filter CIV out (side `CIV` is ambience, never a combatant choice),
and the lint exempts it from the `tiers[0]` requirement, reporting
`ISSUE CIV civ-exempt authors no tiers[]`.

## 9. Menu gating

`GuerrillaFactionIssue` (in `UI/Guerrilla/GuerrillaNewGame.hpp`) decides whether
a listed faction is selectable. In order, it returns a reason string when:

1. the class is not in the merged table (`no such faction`);
2. it has no `tiers[]` array with at least one entry (`authors no tiers[]`);
3. `tiers[0]` is empty or not in `CfgVehicles`
   (`tiers[0] '<c>' is not in the loaded data`);
4. `playerClassWarrior` is present and not in `CfgVehicles`;
5. `playerClassWarrior` fails the shape gate (menu only, see Pitfall 3).

A faction with a non-empty reason is rendered dimmed with the suffix
`" (not in loaded data)"`, one INFO line per greyed row goes to the log, and
pressing OK on it is **refused** with a message naming the role and the reason
rather than launching into fallback bodies.

That is the whole player-visible consequence of an unmounted mod: your pack is
listed and greyed until its addons are mounted.

---

## Validation

`PoseidonTools` ships three checks. None of them launch the game.

| Command | Answers |
|---------|---------|
| `mod doctor <modfolder> [--fix]` | Does the mod's own data carry known defects (undefined `scope` keyword, malformed float literal, model origin above the mesh)? |
| `guerrilla lint --data-dir <pkg> [--mod ...] [--island ...]` | Does every descriptor key resolve against this package, and would the menu accept the faction? |
| `guerrilla probe --data-dir <pkg> --mod <folder>` | Can the mod's own classes actually be built (model shipped, EventHandler scripts present)? |

### `guerrilla lint`

```powershell
# the doc's own example, on the committable mini fixture
dist\x64-win-rwdi\PoseidonTools.exe guerrilla lint `
  --data-dir arma_CWR\tests\fixtures\packages\mini `
  --mod arma_CWR\tests\fixtures\mods-factionpack\@udfaction

# a real install with a real pack
dist\x64-win-rwdi\PoseidonTools.exe guerrilla lint `
  --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --mod D:\Arma_CWA\@LoBo

# with an island template, so the union that island actually sees is linted
dist\x64-win-rwdi\PoseidonTools.exe guerrilla lint `
  --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --island arma_CWR\guerrilla-mode\mission\Guerrilla.Abel
```

With no `--island`, only the global table is linted. `--island` and `--mod` are
both repeatable, and `--mod` order is `-mod` order (first listed is lowest
priority). Each island gets its own `== TABLE <name> ==` section.

**Output grammar.** One line per fact, all greppable:

```
== TABLE <label> ==
FACTION <class> side=<s> origin=Island|Global owner=<addon|-> overrodeGlobal=0|1
LINT    <class> <key> <authored value> -> ok | <substitute> | Dropped
TWIN    <class> <SIDE> on=<class|-> off=<class|->
ISSUE   <class> ok | civ-exempt <reason> | UNRESOLVABLE <reason>
PROBEMISS <label> <Bank>/<classname>
SUMMARY factions=<n> substituted=<n> dropped=<n> unresolvable=<n>
```

`TWIN` is printed once per war side and reports the `sideTwin` chain result:
what this roster becomes when it must move **on** to that side and **off** it.
`PROBEMISS` lists every class lookup that came back empty during the run, which
is usually the fastest way to spot a typo. The exit code is **1 when
`unresolvable > 0`**, else 0, so it drops straight into CI.

The example in section 3 passing, verbatim (excerpt):

```
== TABLE (global) ==
FACTION UDFaction side=WEST origin=Global owner=- overrodeGlobal=0
LINT UDFaction tiers[0] UDSoldier -> ok
LINT UDFaction civTiers[0] UDCivFighter -> ok
LINT UDFaction vehicles[0] UDCar -> ok
LINT UDFaction playerClassWarrior UDSoldier -> ok
TWIN UDFaction WEST on=UDFaction off=-
TWIN UDFaction EAST on=- off=UDFaction
ISSUE UDFaction ok
SUMMARY factions=1 substituted=0 dropped=0 unresolvable=0
```

And its deliberately broken twin `@udbroken`, whose `tiers[0]` names a class
nothing ships, exiting 1:

```
LINT UDBrokenFaction tiers[0] NoSuchClass -> SoldierWB
LINT UDBrokenFaction playerClassWarrior NoSuchClass -> SoldierWB
ISSUE UDBrokenFaction UNRESOLVABLE tiers[0] 'NoSuchClass' is not in the loaded data
PROBEMISS (global) CfgVehicles/NoSuchClass
SUMMARY factions=1 substituted=2 dropped=0 unresolvable=1
```

Note what the broken pack shows: the campaign would still have "worked". Every
spawn would simply have worn the wrong body. Catching that is the whole point of
the exit code.

### `guerrilla probe`

The lint asks whether a name resolves. The probe asks whether the class can be
**built**: a createable `Man`/`Land` class must author a `model` whose shape
file the package ships, and every script its `EventHandlers` exec must exist.
`--mod` is required and repeatable; the **last** one listed is the one reported
on. `--filter` takes a wildcard over class names.

```powershell
dist\x64-win-rwdi\PoseidonTools.exe guerrilla probe `
  --data-dir arma_CWR\tests\fixtures\packages\mini `
  --mod arma_CWR\tests\fixtures\mods-factionpack\@udfaction
```

```
PROBE-OWNERS UDFaction
PROBE-NEWCLASSES 3
PROBE UDSoldier ok
PROBE UDOfficer ok
PROBE UDCivFighter ok
SUMMARY probed=3 ok=3 fail=0
```

Grammar: `PROBE-OWNERS <addon> ...`, `PROBE-NEWCLASSES <n>`,
`PROBE <class> ok [scripts=<n>]`, `PROBE <class> FAIL <reason>`,
`SUMMARY probed=<n> ok=<n> fail=<n>`.

### `mod doctor`

Run this **first** on any third-party mod, before the other two:

```powershell
dist\x64-win-rwdi\PoseidonTools.exe mod doctor "D:\Arma_CWA\@LoBo"          # report only
dist\x64-win-rwdi\PoseidonTools.exe mod doctor "D:\Arma_CWA\@LoBo" --fix    # repair in place
```

It scans every pbo under `<modfolder>\addons` and reports each known defect
class with its file and class: `MISSING_DEFINE_HEADER`,
`UNDEFINED_SCOPE_KEYWORD`, `MALFORMED_FLOAT`, `BURIED_MODEL_ORIGIN`. A defect
that cannot be repaired byte-for-byte is marked `[not patchable]` in the report.
`--fix` applies same-length in-place byte patches (the
pbo entry table never moves) and copies the originals to
`<modfolder>\_ud-orig` first. It is idempotent. `--pbo <wildcard>` narrows the
scan.

---

## Pitfalls

1. **This config parser has no forward declaration.** `class X;` is a parse
   **abort**, not a no-op: `ParamClass::Parse` demands `:` or `{` after the
   name, reports `';' encountered instead of '{'` and returns, so everything
   after that point in the file is silently missing. Re-declare the base chain
   empty instead (`class All {}; class AllVehicles : All {}; ...`), which merges
   back onto the originals without changing them. Both fixture configs do this
   and say why.
2. **A mod's `bin\config.cpp` is parsed standalone**, before the deferred merge
   layers it onto `Pars`. The base package's classes are not visible during that
   parse, which is the real reason for pitfall 1.
3. **The lint does not run the shape gate.** It calls `GuerrillaFactionIssue`
   without the model probe, so a `playerClassWarrior` whose `.p3d` is missing
   lints clean and is greyed out in the menu anyway. Run `guerrilla probe` too.
4. **The lint is not a substitute for mounting the mod.** It reports what
   *this* mount resolves. A pack that lints clean against a package containing
   its addons still greys out for a player who has not mounted them, which is
   correct behaviour, not a defect.
5. **`side` defaults to the class name.** A block named `EAST` with no `side`
   key is on EAST. A block named `Hizballah` with no `side` key is on a side
   called "Hizballah", which no center will ever match. Always write `side`.
6. **`tierThresholds[]` has `n-1` entries for `n` tiers**, and the role arrays
   are parallel to `tiers[]`, so they need `n` entries each, including the empty
   ones.
7. **`civClass<N>` is numbered keys, not an array**, because `gmFactionValue`
   returns raw config text and skips array entries. `civVehicles[]` *is* an
   array, because `LoadFactions` reads it by name.
8. **Lookups are case-insensitive** throughout the resolution and twin walks,
   but the class name you write is what the player sees in the cycler.
