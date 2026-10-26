**LoBo Reloaded** is the Lost Brothers mod for Arma: Cold War Assault / Operation Flashpoint. It brings together every LoBo release we could find, with all repairs to date, packaged for the official CWA Remaster and for Uslu dur!.

> Family use only. It bundles third-party work under many licences (credits and readmes inside). Please do not re-upload it.

## Download

- `LoBo-Reloaded-2026-10-11.7z` (840 MB). Unpack with 7-Zip.
- SHA-256 `dd19681a2853a84fb243cb13bd23a7d4e4d36cca6e73d92ce898251272d9a1ed` (also in the `.sha256` file).
- Inside, `SHA256SUMS.txt` checks every file.

## What's inside

| Folder | What it is |
|---|---|
| `@LoBo` | The mod: islands Sinai, Lebanon, Gaza, Iran/Iraq and 73 Easting; IDF, Egypt, Syria, Jordan, Hizballah and Arab factions; demo missions. Repaired, and safe on every engine. |
| `@LoBo_Deps` | The 40 third-party addons LoBo needs, with their licences. |
| `@LoBo_Extras` | Optional recovered packs. Each pack's original readme and credits are in `@LoBo_Extras\docs\`. The packs are: H&K 1.75 weapons and units, HYK US infantry with 2 missions, the Iraqi Shilka, BAS Little Birds, the Bundeswehr basic pack and the FAMAS. |
| `@LoBo_UD` | For Uslu dur! only: Guerrilla faction rosters and full-strength scripts. |

## Install

1. Delete any older `@LoBo*` folders in your game folder.
2. Copy the folders next to the game's `AddOns`.
3. Start the game with the line for your engine. Order matters.
   - **Official Remaster:** `PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps"`
   - **Uslu dur!:** `PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps;@LoBo_UD"` (`@LoBo_UD` must be last)
   - **Legacy 1.99:** `ColdWarAssault.exe -mod=@LoBo_Extras;@LoBo;@LoBo_Deps`

`@LoBo_Extras` is optional; leave it out of the line if you don't want it. `README-FIRST.txt` in the package has the details. In multiplayer, everyone needs the identical package.

## Fixed in this build

- **Mod doctor:** wreck and poster classes that could not be placed, buried M60A1 wrecks, and a malformed tracer colour.
- **Repair 2026-10-11.1:**
  - ammunition links and AI engagement ranges;
  - blast and damage consistency;
  - armor bindings;
  - missile and landing-craft scripts;
  - divers and display text.
- **Repair 2026-10-11.2:** AP mine pickup, UH-60 fast insertion, MiG-23 wing sweep.
- **Repair 2026-10-11.3:**
  - Mk-20 Rockeye cluster bombs now open on the A-4, F-4 and IL-28;
  - the Syrian RBK-500 now works;
  - tail numbers appear on the F-15, F-16, F-4 and IL-28;
  - wreck fires get their glow and sound;
  - vehicle scripts and mines no longer run every frame;
  - 7 missing H&K weapon icons are restored;
  - crew textures and config fixes.

Repair details: [CHANGELOG](https://github.com/GearUnclear/lobo/blob/main/CHANGELOG.md). Source: GearUnclear/CWR `98bf49b` (repairs) and `a5daeb2` (package builder).

## Known issues

- **Testing so far:** the cluster bombs, tail numbers and wreck glow were checked in game, on the Uslu dur! engine with Remaster data. Multiplayer, saves and long-range AI fire are untested. The official Remaster exe itself has not been run with this package yet.
- **Limited wreck fire on the stock exes:** on the official Remaster exe and the legacy 1.99 exe, the burning-wreck scripts only reach the player's and the wreck's own groups. Uslu dur! with `@LoBo_UD` gets the full version.
- **Helicopter wreck fires:** a burning helicopter wreck can hurt a player on foot within about 10 m, as the original script intended.
- **IL-28 tail gun:** the script can stop with an error if the bomber spawns with a gunner but no pilot.
- **Missing files:** a few files are missing from the original release, so some textures and one MiG crash sound do not appear.
- **Lost content:** some LoBo content was released once and is now lost everywhere, including the OFP Isherman, T-34 and M3 halftrack packs, the Tavor rifle and several missions, so it is not in this package.
