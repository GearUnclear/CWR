# World names: internal vs display

The island listboxes (GUERRILLA menu, editor) SHOW `CfgWorlds >> <class> >> description`
but ACT on the class name (`SelectedIsland()` returns row data = class name;
`OptionsUIApp.cpp` builds `missions\Guerrilla.<class>`). Templates are therefore named
`Guerrilla.<internal>` - `Guerrilla.Abel`, never `Guerrilla.Malden`. No mapping table
exists in engine source; it lives only in game-data configs. This file is the repo copy,
enforced by `tests/unit/engine/Poseidon/Game/Guerrilla/test_mission_world_names.cpp`.

| Internal (class / .wrp) | Display        | Ships in |
|-------------------------|----------------|----------|
| Abel                    | Malden         | base `Worlds\abel.wrp` |
| Cain                    | Kolgujev       | base `Worlds\cain.wrp` |
| Eden                    | Everon         | base `Worlds\eden.wrp` |
| Intro                   | Desert Island  | base `Worlds\intro.wrp` |
| Noe                     | Nogova         | Resistance `AddOns\Noe.pbo` (adds its own CfgWorlds entry) |
| Demo                    | Malden - Demo  | NOT in full CWA (CfgWorlds entry extends Abel; `\demo\demo.wrp` shipped only with the 2001 demo). Menu filters it out: wrp missing. |
| sinai                   | Southern Sinai | @LoBo `addons\lost.pbo` |
| Lebanon80               | Lebanon (80's) | @LoBo `addons\LoBo_Leb.pbo` |

Internal codenames follow Genesis in order (Eden, Abel/Cain, Noe); display names are
fictional geography. Deliberate BIS convention (community wiki: "Operation Flashpoint:
Troubleshooting").

Derived from the shipped configs via `PoseidonTools config dump <GameDir>\BIN\CONFIG.BIN
CfgWorldList|CfgWorlds`, `pbo extract Noe.pbo` + `config debin`, and `pbo show` on the
two @LoBo world PBOs. All `description=` values are literal strings (no `$STR_` keys).
Main CONFIG.BIN's CfgWorldList = Eden, Abel, Cain, Demo, Intro only; Noe and mod worlds
append theirs.
