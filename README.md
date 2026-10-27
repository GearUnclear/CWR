# Arma: Cold War Assault - Remastered

This repository holds the engine and game source code (codename *Poseidon*) behind *Arma: Cold War Assault* — the game first released in 2001 as *Operation Flashpoint: Cold War Crisis*. That release launched Bohemia Interactive and began the technology lineage that later grew into Real Virtuality, Arma, and Enfusion. The code has been modernized to C++20, built with CMake and Clang, with cross-platform support for Windows x64 and Linux x64.
Bohemia Interactive is releasing it to the community that has kept this game alive for more than two decades — to study it, build on it, fix it, and create from it. Three things are worth keeping separate:

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

## Remastered lighting

This tree adds a remastered look to the GL 3.3 renderer. It lights every
pixel in high dynamic range and runs the frame through a metered camera,
using only the models and textures the game already ships:

- **Light and shadow:** a physically based sun and sky, soft sun shadows,
  long tree and building shadows at a low sun, hill shadows, drifting cloud
  shadows and ambient occlusion.
- **Sky and air:** volumetric fog (haze, dust, weather and night fog, valley
  mist at dawn) with sun beams through it.
- **Water and rain:** waves and a sun path on the sea, reflections on calm
  water, wet ground in rain where only puddles reflect.
- **Night and combat:** moonlight, lamps, headlights and fires lit per pixel;
  muzzle flashes, flares, fires and explosions light their surroundings, and
  explosions scale with their charge.
- **Image quality:** temporal anti-aliasing over 4x MSAA.

To play it with your own copy of the game, build as in Quick Start, then
start the new executable with your game folder as its working directory
(`-C`). Nothing in the game folder is changed:

```sh
dist/x64-win-rwdi/PoseidonGame.exe -C "<your Arma: Cold War Assault folder>"
```

On GNU/Linux, after a build with the `linux-x64-clang-rwdi` preset:

```sh
dist/x64-linux-rwdi/PoseidonGame -C "<your Arma: Cold War Assault folder>"
```

The new executable keeps its settings, profiles and saves in the same user
folder as the official one, so the two share `graphics.cfg`. When the
official game saves its settings it drops the Lighting choice, which then
reads as Remastered again. To keep them apart, set the `POSEIDON_USER_DIR`
environment variable to another folder before you start the new executable.

**Options > Graphics > Lighting** switches between **Remastered** (the
default) and **Classic**, the original lighting. The choice is stored in
`graphics.cfg`. A few changes also apply with Classic: 4x MSAA by default,
three-entry config colours read correctly (street lamps), and explosions
that scale with their charge.

AI soldiers' sight is cut by the same fog the renderer draws, so they never see
through fog the player cannot see through. With Classic lighting, or with post
effects off (a developer switch, not an Options setting), that is the classic
fog. A dedicated server uses the Remastered fog.

## Layout

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
