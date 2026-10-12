# @LoBo_Extras: optional packs from the recovered downloads

`build-lobo-extras.py` rebuilds the optional `@LoBo_Extras` modfolder from the
LoBo downloads recovered on 2026-10-11 (`D:/LoBo-Reloaded/incoming/recovered-2026-10-11/files`).
Which archive goes where, which patches each pack needs and what must never ship
come from the fold-in review of those 26 downloads. Nothing in this folder is
needed by `@LoBo` or its missions.

The pbos are third-party game data (APL-SA and the pack authors' own terms). They
are built locally and never committed; only this script, its tests and this note
are tracked.

## What it ships

| Pack | Files | Patch applied by the script |
| --- | --- | --- |
| OFP: HK pack 1.75 (`ofphk-175.exe`, read as a RAR, never run) | `addons/TheHKpack.pbo` | `config.bin`: `scopeWeapon`/`scopeMagazine` of HK36Cmag, HKG36Kcmag, HKSL8mag, HKG37mag from the strings `"private"`/`"public"` (read as 0) to `0`/`2`; the `M16GrenadeLauncher: Default` stub made base-less like stock |
| HYK US Army infantry 1.52 | `addons/HYK_USsol.pbo`, `Missions/HYK_Infantry.Abel.pbo`, `Missions/HYK_Revenge.Abel.pbo` | `CfgTextureToMaterial>>GoggleGlass` renamed `HYK_GoggleGlass` (`@LoBo_Deps` CBT_Crew owns that name) |
| Shilka Iraq (`.ace`) | `addons/Vit_Iraq_Shilka.pbo` | `MAF_ZSU234` crew `Vit_Iraq_Crew` (never released) to stock `SoldierECrew`; `CfgPatches` `units[]` to `MAF_ZSU234` |
| BAS Littlebird 1.1 | `addons/bas_lbs.pbo`, `addons/bas_lbs185.pbo` | none (the zip's 2002 `bas_soar*.pbo` are left out) |
| BWMod Basic Pack 1.56 (skip with `--no-bwmod`) | `addons/BWMOD_{Config,i,w,Misc,Scripts}.pbo` | none |
| FAMAS 1.00 | `addons/FAMAS.pbo` | `CfgModels` `famas: Weapon` and `famasgranat: Weapon` (stops the permanent muzzle flash); the three `CfgPatches` `units[]` point at the real soldier classes |

Each pack's original readme or credit note is copied unchanged to `docs/<pack>/`.
`README.txt` (generated) lists what each pack adds, the credits, the patches and
the load order; `docs/BUILD-MANIFEST.json` records every pin and check.

Load order: `@LoBo_Extras` goes FIRST, so core wins every pbo-name clash.
`--mod "@LoBo_Extras;@LoBo;@LoBo_Deps"` on the official Remastered exe,
`--mod "@LoBo_Extras;@LoBo;@LoBo_Deps;@LoBo_UD"` on Uslu dur!, and
`-mod=@LoBo_Extras;@LoBo;@LoBo_Deps` on legacy 1.99. Never put a `Bin` folder in it.

## Guarantees the script enforces

- Every source archive, every extracted payload and every patched pbo is
  SHA-256 pinned; any mismatch aborts before anything is published. A deliberate
  rule change shows up as an output-pin mismatch: review it, then update
  `OUTPUT_SHA256` together with the rule.
- Payloads come out through 7-Zip (`7z x -so`) or, for the `.ace`, through
  acefile 0.6.14 run by a separate venv Python with `-I -B` from an empty folder.
  Nothing from the downloads is executed.
- Every patch target is re-verified first (class present exactly once, value
  exactly as the plan describes, renamed name unused); anything else is refused.
- Only the patched config member is replaced. Every other member keeps its bytes
  and header fields. The raP codec round-trips the original `config.bin` byte for
  byte before it edits it. Old StuffPBO trailers (FAMAS, Shilka) are dropped,
  because their checksum would be stale; the engine ignores trailing data.
- The never-ship list (TheHKpack 1.0, TheHKpack2, the small HK packs, LoBoWeapons
  1.1, LoBoTzahal, the LoBoC130 beta, JAM2, BN_tracer 1.25, coc_diver 1.0,
  CSJairlines, OPGWC `SWI_UI`, the 2002 `bas_soar` pair, every exe/dll, the videos
  and the HAYAL letter) is checked against the payload table and the finished
  folder. The finished folder may only hold `addons/`, `Missions/`, `docs/` and
  `README.txt`.
- With PoseidonTools, the engine's own parser reads each patched config back:
  the debinned HK config must differ from the original in exactly the 9 planned
  lines, and each patched `config.cpp` must binarize with all its classes and the
  new values. (`config bin` exits 0 even on unreadable input, so the output is
  parsed, not trusted.)
- With PoseidonTools, every shipped addon config, patched or not, is also read
  the way the engine reads it (compressed members decoded, text configs
  binarized) and scanned for the mod doctor's config defects: an undefined scope
  keyword or a malformed float, as the engine parser reports them for a text
  config, or a non-numeric scope string in a binarized one. Any hit stops the
  build. This covers the two configs `--doctor` cannot read (the binarized HK
  config and the compressed `bas_lbs185` one).
- `--against` / `--stock-game` add the class-clash check the plan asks for: an
  extras config usually merges after core, so it must be class-clean, not only
  pbo-name clean. These fail the build: a shared pbo name or CfgPatches name; a
  class value that differs from core, deps, UD or the stock AddOns; any differing
  base (whichever config merges last sets it), including against a class a stock
  AddOns pbo creates (such as `HKG3` in `O.pbo`, which no access lock protects);
  and an extras config the engine parser does not read in full (a forward
  declaration `class X;` makes it drop the rest of the class), since its
  classes could not be checked. Warnings in the manifest: additive merges, values
  core already uses, a rebase a locked stock `CONFIG.BIN` class refuses, keys
  added to an unlocked stock class, and the overlaps listed in
  `ACCEPTED_OVERLAPS` (only `bas_lbs185`'s stock-shaped `CfgModels>>Helicopter`).
  The manifest records each `--against` folder's pbo count and a digest over its
  pbo names and hashes.
- `package-lobo-family.ps1 -ExtrasDir` runs the same class check
  (`build-lobo-extras.py --check`) against the folders it actually stages and
  the stock game config, and refuses a clash, so an extras folder built against
  an older core cannot ship unchecked.

## Running it

Python 3.13, 7-Zip at `C:\Program Files\7-Zip\7z.exe` (or `--seven-zip`), and a
venv with acefile for the Shilka archive. Keep the venv outside the downloads
folder; the requirement pins the PyPI sdist:

```powershell
python -m venv C:\venvs\acefile
Set-Content -Encoding ascii acefile-req.txt 'acefile==0.6.14 --hash=sha256:96957fd167136ef5bc1c133fe74da2453229ad19308d6b986cbcc3c76ed5511d'
C:\venvs\acefile\Scripts\python.exe -m pip install --require-hashes -r acefile-req.txt
```

Full build with every check (PowerShell 5.1). The staged modfolders of a
`package-lobo-family.ps1` run serve as `--against`; stage the current core
first (for example `package-lobo-family.ps1 -SkipFetch -NoArchive`), so the
check sees the repair state that will ship:

```powershell
$fam = 'D:\LoBo-Family\stage\LoBo-Reloaded-<Version>'
python -I arma_CWR\tools\lobo\build-lobo-extras.py `
  --recovered 'D:\LoBo-Reloaded\incoming\recovered-2026-10-11\files' `
  --out 'D:\LoBo-Reloaded\modfolders\@LoBo_Extras' `
  --ace-python C:\venvs\acefile\Scripts\python.exe `
  --tools D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe `
  --against "$fam\@LoBo" --against "$fam\@LoBo_Deps" --against "$fam\@LoBo_UD" `
  --stock-game 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered' `
  --doctor --report "$env:TEMP\lobo-extras-report.json"
```

The output folder must be named `@LoBo_Extras`. An existing folder is replaced
only with `--replace` and only if it holds a previous `docs/BUILD-MANIFEST.json`.
`--doctor` runs `PoseidonTools mod doctor` in report-only mode; it cannot scan
the binarized HK config or the compressed `bas_lbs185` config, which the
build's own config scan above covers. File times are fixed to 2026-10-11, the
folder's own time included, so the folder archives reproducibly.

To check an extras folder that is already built (this is what the packager
runs after staging; exit 0 clean, 1 clash, 2 could not check):

```powershell
python -I arma_CWR\tools\lobo\build-lobo-extras.py --check 'D:\LoBo-Reloaded\modfolders\@LoBo_Extras' `
  --tools D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe `
  --against "$fam\@LoBo" --against "$fam\@LoBo_Deps" --against "$fam\@LoBo_UD" `
  --stock-game 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered' `
  --report "$env:TEMP\extras-check.json"
```

Without `--stock-game` the check does not know the stock access locks, so core
stubs that the stock config refuses to rebase (BWMod's `AALauncher`, several
`Riffle` stubs) read as clashes; always pass it.

Tests (synthetic data only): `python arma_CWR/tests/ci/test_lobo_extras.py`.

## Licensing and open points

- The HK pack readme asks that its authors be contacted before the pack is
  distributed. Settle that before `@LoBo_Extras` leaves the family.
- The readmes are shipped unchanged and include the authors' published contact
  lines. The HAYAL letter (personal contact details) and LoBoDemoVideo4
  (commercial music) are never shipped.
- In-game checks still to do: the BWMod callsign override and its `AALauncher`
  stub should be refused by stock access locks (engine source reading only), and
  nobody has yet loaded these packs together with `@LoBo` in a running game.
