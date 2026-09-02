# World names: internal vs display

## The convention

Every island has two names, and they are not interchangeable.

- **Internal name** = the `CfgWorlds` class name, and the `.wrp` file's base name.
  A config identifier: letters, digits, underscore. `Abel`, `Noe`, `Lebanon80`.
- **Display name** = `CfgWorlds >> <class> >> description`, a free-form string
  shown to the player. `Malden`, `Nogova`, `Lebanon (80's)`.

The island listboxes (GUERRILLA menu, editor) **show** the display name but **act
on** the class name: `SelectedIsland()` returns the row *data*, which is the class
name, and `OptionsUIApp.cpp` builds `missions\Guerrilla.<class>` from it.

**So a template folder is always named `Guerrilla.<internal>`.** `Guerrilla.Abel`,
never `Guerrilla.Malden`. A display-named folder lists fine in the repo, passes the
script-core parity test, and is then never found by the new-game launch path - a
silent content bug. `tests/unit/engine/Poseidon/Game/Guerrilla/test_mission_world_names.cpp`
pins it: a template's suffix must look like a config class name, and the display
names we know about are rejected outright.

## Why the stock names look like that

BIS named the stock islands after Genesis, in order: **Eden** (Everon),
**Abel** and **Cain** (Malden, Kolgujev), **Noe** (Nogova). The display names are
fictional geography with no relation to the codename. Deliberate BIS convention
(community wiki: "Operation Flashpoint: Troubleshooting"). Mod worlds follow no
such scheme - `sinai`, `Lebanon80` - and often use the same word for both names
with different capitalisation or wording (`sinai` / "Southern Sinai").

Handy pairs, for orientation only - **this is not a registry and nothing has to
be added to it when an island pack ships**:

| Internal (class / .wrp) | Display        | Ships in |
|-------------------------|----------------|----------|
| Eden                    | Everon         | base `Worlds\eden.wrp` |
| Abel                    | Malden         | base `Worlds\abel.wrp` |
| Cain                    | Kolgujev       | base `Worlds\cain.wrp` |
| Intro                   | Desert Island  | base `Worlds\intro.wrp` |
| Noe                     | Nogova         | Resistance `AddOns\Noe.pbo` (adds its own CfgWorlds entry) |
| Demo                    | Malden - Demo  | NOT in full CWA (CfgWorlds entry extends Abel; `\demo\demo.wrp` shipped only with the 2001 demo). Menu filters it out: no wrp. |
| sinai                   | Southern Sinai | @LoBo `addons\lost.pbo` |
| Lebanon80               | Lebanon (80's) | @LoBo `addons\LoBo_Leb.pbo` |

## How to list the worlds a package actually offers

No mapping table exists in engine source, and none is maintained here. Read it
out of the shipped configs instead:

```sh
# the base game's own world list and their display names
PoseidonTools config dump "<GameDir>\BIN\CONFIG.BIN" CfgWorldList
PoseidonTools config dump "<GameDir>\BIN\CONFIG.BIN" CfgWorlds

# a mod's world: its pbo carries both the .wrp and the CfgWorlds entry
pbo show "<mod>\addons\lost.pbo"                 # the entry table - find the .wrp
pbo show "<mod>\addons\lost.pbo" config.cpp      # its CfgWorlds >> <class> >> description
```

Main `CONFIG.BIN`'s `CfgWorldList` is Eden, Abel, Cain, Demo, Intro only; `Noe.pbo`
and every mod world append their own entries. All stock `description=` values are
literal strings (no `$STR_` keys).

`guerrilla-mode/install-missions.ps1` needs none of this: its world gate finds a
world by looking for `<World>.wrp` in `<GameDir>\Worlds` and then in the **entry
tables** of the pbos under `AddOns\`, `Dta\` and each mod's `addons\`. Reading a
pbo header is a few KB, so an island pack becomes installable the moment its pbo
is on disk - no doc edit, no test edit, no `-IncludeWorld`.
