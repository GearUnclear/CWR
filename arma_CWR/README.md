# Uslu dur! — a Total Game Overhaul of Arma: Cold War Assault

This repository is a **total game overhaul** built on top of *Arma: Cold War Assault* (the game first released in 2001 as *Operation Flashpoint: Cold War Crisis*): a from-the-ground-up rework of how the AI fights, perceives, and is commanded, culminating in **Guerrilla Mode** — a brand-new persistent open-world insurgency game mode with swappable islands and factions, chosen at new-game. See [`mod-plans/README.md`](mod-plans/README.md) for the full set of engine/AI mod-plans and [`guerrilla-mode/README.md`](guerrilla-mode/README.md) for the flagship game mode built on top of them.

This is a **total overhaul of the engine source, not a mod on a fixed binary** — C++ engine work is a first-class tool here, not a last resort, right alongside SQF mission scripting.

None of it happens in a vacuum: it's built on Bohemia Interactive's own opened engine source, internal codename *Poseidon*, modernized to C++20, built with CMake and Clang, with cross-platform support for Windows x64 and Linux x64.
Bohemia Interactive released that engine source to the community that has kept this game alive for more than two decades — to study it, build on it, fix it, and create from it. Three things are worth keeping separate:

**Source code (this repository)**

The engine and game executables, licensed under GPL-3.0-or-later with additional terms under Section 7. You may use, study, modify, and redistribute it, provided it stays GPL and you follow those terms.

**The name and brand**

"ARMA", "Operation Flashpoint", and the logos are *not* granted. The trademarks stay with their owners ("ARMA" is Bohemia Interactive's). A fork must be renamed and must not present itself as "Arma" or as an official Bohemia Interactive product.

**Game data (separate)**

Models, textures, sounds, missions, and voices. These are not in this repository and are not GPL; they ship separately under the APL-SA license. A free Demo is available on Steam.

In short: the code is free software, the name is not, and the game data comes separately. This license covers the source code only and grants no rights to the trademarks.


## Quick Start

```sh
cmake --preset win-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi
```

On GNU/Linux, use the matching `linux-x64-clang-rwdi` preset.

## Run

```powershell
.\run-game.ps1                                      # build + launch to the main menu
.\run-game.ps1 -Mission 'Missions\Guerrilla.Demo'   # build + jump straight into a mission
.\run-game.ps1 -SkipBuild                           # launch the existing build, no rebuild
```

`run-game.ps1` wraps the toolchain-on-PATH setup, an incremental `PoseidonGame`-target
build, and launching the exe windowed (`--window --no-splash`) with its working directory
set to the game data dir (default `D:\Arma_CWA\ARMA Cold War Assault`, override with
`-DataDir`). From the main menu, hit **GUERRILLA** to pick an island/faction and start a
Guerrilla Mode game. See [`guerrilla-mode/README.md`](guerrilla-mode/README.md#run-the-mode)
for mission template details.

## Layout

- [Mod plans](mod-plans/README.md) - the total-game-overhaul AI/engine mod-plans (the point of this fork)
- [Guerrilla Mode](guerrilla-mode/README.md) - the flagship open-world insurgency game mode built on the mod-plans
- [Apps](apps/README.md) - executable targets
- [Engine](engine/README.md) - engine libraries and Rust Trident tooling
- [Master server tools](mserver/README.md) - Rust service and CLI crates
- [Tests](tests/README.md) - test source trees; CI currently compiles them only
- `cmake/` - presets, toolchains, vcpkg triplets, and overlay ports
- `docker/` - container support for service and runtime environments
- `packages/` - ignored local game data staging area
- `resources/` - application icon resources
- `thirdparty/` - vendored third-party headers and sources

## Project Notes

- [Contributing](CONTRIBUTING.md)
- [Credits](CREDITS.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)
- [Vendored dependencies](thirdparty/README.md)

## License

The source in this repository is licensed under the **GNU General Public License
v3.0 *or later***, with additional terms under **Section 7** of the GPL. See [`LICENSE`](LICENSE) for the
full text.
This license does not grant you any right to use "ARMA" or any other Bohemia Interactive trademark.

The [`thirdparty/`](thirdparty) directory is **excluded** from the project's GPL
license: it contains vendored third-party code (glad, the RenderDoc API header)
under their own respective licenses — see [`thirdparty/README.md`](thirdparty/README.md).
Dependencies pulled in via vcpkg ([`vcpkg.json`](vcpkg.json)) likewise remain under
their own licenses.

*"ARMA" is a registered trademark of BOHEMIA INTERACTIVE a.s. "OPERATION FLASHPOINT" is a registered trademark of Electronic Arts Inc.
See [`LICENSE`](LICENSE) for information concerning trademarks. This credits file is
informational and does not constitute any grant and/or waiver of rights.*

### Game data / assets — Arma Public License Share Alike (APL-SA)

Game data and assets (models, textures, sounds, missions, etc.) are **not part of
this repository** and are **not** covered by the GPL. They are released separately
by Bohemia Interactive under the **Arma Public License Share Alike (APL-SA)**:

- APL-SA license text: <https://www.bohemia.net/community/licenses/arma-public-license-share-alike>

### Getting game data to run what you build

The compiled binaries need game data to run. You can obtain the **free Demo game
data** on Steam:

- *Arma: Cold War Assault Remastered* Demo on Steam: <https://store.steampowered.com/app/4819000>

The full game data ships with the retail game. Whatever you do with assets is
governed by the APL-SA linked above; whatever you do with this source is governed by
the GPL with additional terms per Section 7 in [`LICENSE`](LICENSE).


## Contributing

This is a **locked** repository: pull requests are not accepted here, and this
repository will not be continuously updated.
Issues are only for bugs in official Bohemia Interactive builds distributed on
Steam. For ideas, development builds, ports, and community work, fork the code or
join the community continuation. See [`CONTRIBUTING.md`](CONTRIBUTING.md) for more information.
