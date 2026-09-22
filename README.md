# Uslu dur! — Guerrilla Mode

Engine and gameplay source lives in [`arma_CWR`](arma_CWR). To develop or play
on another machine, clone this repository and supply fresh **Classic 1.99 game
data** and the complete **LoBo** mod folder.

Follow [the fresh-install guide](arma_CWR/guerrilla-mode/FRESH-INSTALL.md) to
build the engine, install its runtime resources and launch. From `arma_CWR`,
after building `PoseidonGame` and `PoseidonTools`:

```powershell
pwsh -File ./setup-guerrilla.ps1 -GameDir 'C:/Games/CWA Classic' -LoBoDir 'C:/Games/@LoBo' -Tools './dist/x64-win-rwdi/PoseidonTools.exe'
pwsh -File ./play-guerrilla.ps1 -GameDir 'C:/Games/CWA Classic' -LoBoDir 'C:/Games/@LoBo' -GameExe './dist/x64-win-rwdi/PoseidonGame.exe'
```

Use your actual paths. Setup installs the previously local-only compatibility
and menu files, the licensed UI/font resources, missions, factions and LoBo
repairs. Game PBOs remain external; the repo supplies the reproducible setup.

See the [engine README](arma_CWR/README.md), [licenses and notices](arma_CWR/THIRD_PARTY_NOTICES.md),
[Guerrilla documentation](arma_CWR/guerrilla-mode/README.md), and
[internal roadmap](roadmap/README.md).
