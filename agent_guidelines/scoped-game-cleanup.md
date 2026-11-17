# Scoped game cleanup: never kill PoseidonGame globally

Verified 2026-10-13 at commit `6163d40` (branch `occupier-forces`), on this
machine while four other sessions were running Trident tests. Paths are
relative to the repo root.

## Symptom

A `tri test` run fails with exit code -1 and an empty stderr, and nothing in
the test or the engine log explains why. The game window simply vanished
mid-run. Retries may pass or may die the same way.

## Why it happens

Observed:

- Several AI sessions share this machine. At verification time four
  `PoseidonGame.exe` processes were running at once, each launched by a
  different session's `tri` from its own build dir
  (`arma_CWR/dist/agent-rwdi`, `agent2-rwdi`, `agent67-rwdi`, `agent71-rwdi`)
  against its own shadow (`-C D:\Arma_CWA\tmp\remaster-shadow-of1` .. `of4`).
- `arma_CWR/CLAUDE.md` used to tell agents to end every game-spawning command
  with `Get-Process PoseidonGame | Stop-Process -Force`. That stops every game
  on the machine, including the other sessions' games in the middle of a test.
- Trident already stops its own child: the game process is spawned with
  `kill_on_drop(true)` (`arma_CWR/engine/Trident/src/client/instance.rs:126`),
  so a normal `tri` run leaves nothing behind and needs no external kill.

Inferred: the exit -1 with empty stderr is the losing side of a global kill
from another session. Real
stragglers come from crashed `tri` runs (parent gone before the drop ran) and
from `run-game.ps1`, which starts the game detached.

## What to do

- Never run a global kill (`Get-Process PoseidonGame | Stop-Process`,
  `taskkill /IM PoseidonGame.exe`, and so on).
- Use `arma_CWR/tools/stop-owned-games.ps1`. It selects only games you own:
  - `-DataDir <dir>`: the game's command line carries `-C <dir>` (every tri
    launch). Case and trailing slashes are ignored.
  - `-Pid <pid>[,<pid>]`: exact PIDs. `run-game.ps1` prints the matching
    command after it launches (it sets the working directory and passes no
    `-C`, so `-DataDir` cannot see its games).
  - `-OwnerPid <pid>`: the game's parent chain contains that PID. From Git
    Bash pass `-OwnerPid $(cat /proc/$$/winpid)`.
  - `-Orphans`: games whose parent process is gone. Another session's
    `run-game.ps1` launch also looks like an orphan, so prefer `-Pid` or
    `-DataDir`.
  - `-DryRun` (or `-WhatIf`) kills nothing. Every run lists all games with
    PID, parent, age and data dir, marked KILLED, WOULD KILL or LEFT ALONE,
    so other sessions' games stay visible. Exit 0 when there is nothing to do.
- Give each agent its own shadow data dir under `D:\Arma_CWA\tmp\`
  (`remaster-shadow-<tag>`) and its own build dir
  (`cmake --preset win-x64-clang-rwdi -B build/<tag>-rwdi`, staged to
  `dist/<tag>-rwdi`). Separate shadows keep `-DataDir` selection unambiguous;
  separate build dirs keep one session's rebuild from replacing another's exe
  under a running test.

Example, after a crashed tri run on your own shadow:

```powershell
powershell -NoProfile -File arma_CWR\tools\stop-owned-games.ps1 -DataDir D:\Arma_CWA\tmp\remaster-shadow-67
```

## Evidence

Test on 2026-10-13, with other sessions' games on `of1`, `of2`, `of3`, `of4`
running throughout:

- Own game started with `-C D:\Arma_CWA\tmp\remaster-shadow-67` (PID 44500).
  Before: `11656,16744,45696,46804`. `-DryRun -DataDir d:/arma_cwa/tmp/REMASTER-SHADOW-67/`
  marked only 44500 WOULD KILL. The real run killed 44500. After:
  `11656,16744,45696,46804`.
- Owner chain through a grandparent (this shell 43236, then a powershell,
  then the game 43852): `-OwnerPid 43236` killed only 43852. Before and
  after: `43524,46804,47712`.
- `-Orphans -DryRun` picked exactly the two games whose launching shell had
  exited (45300, 47892), both ours, and they were then killed by `-Pid`.
