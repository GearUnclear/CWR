**LoBo Reloaded** is the Lost Brothers mod for Arma: Cold War Assault / Operation Flashpoint. It brings together every LoBo release we could find, with all repairs to date, packaged for the official CWA Remaster and for Uslu dur!.

> Family use only. It bundles third-party work under many licences (credits and readmes inside). Please do not re-upload it.

## Download

- `LoBo-Reloaded-2026-10-12.7z` (844 MB). Unpack with 7-Zip.
- SHA-256 `9dd1e854f748c489ad0524a3a9370074cf234b700c9442f556c59de49bcaca0e` (also in the `.sha256` file).
- Inside, `SHA256SUMS.txt` checks every file.

This build replaces 2026-10-11. Please do not keep using 2026-10-11 on the official Remaster: one of its dependency addons logs a script error at the main menu there, and its burning-wreck scripts stop with a script error.

## What's new since 2026-10-11

- The official Remaster exe was tested with this package: it reaches the main menu and runs a short test mission with no script error.
- Fixed two problems that 2026-10-11 has on the official Remaster: a malformed tracer colour in one dependency addon, and burning-wreck scripts that stopped with an error.
- The IL-28 tail gun no longer breaks when the bomber has a gunner but no pilot.
- Missing textures, pictures and one crash sound now show, and the Mk-81 and LDGB bombs and an Egyptian helmet texture are back, restored from older LoBo releases.
- Every LoBo pilot shows wounds in his own suit colour.

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

- **Mod doctor:** wreck and poster classes that could not be placed, buried M60A1 wrecks, and malformed tracer colours, now also in the packaged `@LoBo_Deps` (`IRAR_EE9`), plus two buried models in `bas_mah60`.
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
- **Repair 2026-10-11.4:**
  - the IL-28 tail gun no longer stops with an error when the bomber has a gunner but no pilot;
  - references to files LoBo ships under another name are pointed at those files: the MiG-21 crash sound, the Toyota trim, the Egyptian pilot's back, the AIM-9, the Snunit boat and radio pictures, the medic barrack icon, the Egyptian Guard M1 at a distance, the F-4 Wild Weasel pods and the F-5 cockpit;
  - the Mk-81 bomb, the Mk-81 to Mk-84 LDGB bombs and an Egyptian helmet texture, missing from the last LoBo release, are restored unchanged from older LoBo releases;
  - every LoBo pilot shows wounds front and back in his own suit colour;
  - the burning-wreck scripts no longer stop with an error on the official Remaster;
  - burning wrecks were checked in game: no burn while seated in a vehicle, a slow burn on foot within about 10 m, "Put out fire", the glow's cleanup a few minutes later, and the fire sound.

Repair details: [CHANGELOG](https://github.com/GearUnclear/lobo/blob/main/CHANGELOG.md). Source: GearUnclear/CWR <CWR-COMMITS>.

## Known issues

- **Testing so far:** every repair .4 fix, the cluster bombs, tail numbers and wreck fires were checked in game, on the Uslu dur! engine with Remaster data. The official Remaster exe passed a short smoke test with this package. Multiplayer, saves, long-range AI fire and tank damage animations are untested.
- **Limited wreck fire on the stock exes:** on the official Remaster exe and the legacy 1.99 exe, the burning-wreck scripts burn only soldiers on foot within about 10 m who belong to the player's or the wreck's own group. Uslu dur! with `@LoBo_UD` burns every soldier on foot in that range.
- **Helicopter wreck fires:** a burning helicopter wreck can hurt a player on foot within about 10 m, as the original script intended.
- **Island intros:** Sinai, Iran/Iraq and Gaza point their menu intro at an addon the mod team never released. Uslu dur! works around it; the official Remaster and legacy 1.99 exes may complain about it when those intros load. Harmless.
- **Still missing:** one far-view face of every F-16 names a texture no LoBo release ever shipped and shows untextured; a few vehicle pictures, editor icons and one old A-4 tail fin texture are also missing everywhere. Harmless.
- **Extras:** the H&K pack's wounded-soldier textures are missing (log noise only), and the Bundeswehr pack shows a "BWMOD_Server Gamelogic missing" hint in missions that do not place that logic.
- **Lost content:** some LoBo content was released once and is now lost everywhere, including the OFP Isherman, T-34 and M3 halftrack packs, the Tavor rifle and several missions, so it is not in this package.
