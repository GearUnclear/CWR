# Common rules for occupier-forces agents (binding, read fully before any edit)

Repo root `D:\Arma_CWA`, engine under `D:\Arma_CWA\arma_CWR` (git root is `D:\Arma_CWA`). Branch `occupier-forces` is already checked out. Read `D:\Arma_CWA\CLAUDE.md` and `D:\Arma_CWA\arma_CWR\CLAUDE.md` first. The design is `arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md`: section 0.2 (D1 to D24) is binding and never reinterpreted; section 14 is the phase plan; section 12 names the tests; section 16 the knobs. `arma_CWR/guerrilla-mode/OFFSCREEN-CONTRACT.md` must stay true or be updated by the same change.

## Git
- Never create a branch, worktree branch or remote branch (`git branch`, `checkout -b`, `switch -c`, `worktree add -b`, push of a new branch are all forbidden). Work in the checked-out tree.
- Never commit, never push, never `git add`, never stash, never reset or checkout files you do not own. The orchestrator commits by path from your report.
- Other sessions keep uncommitted edits in `roadmap/roadmap.yaml`, `arma_CWR/guerrilla-mode/README.md`, `arma_CWR/voice-lines/*`, `arma_CWR/tools/lobo/lobo-modfolder-assembly.txt`, untracked `GATE-ZERO-TEST-MISSION-PLAN.md`, `REALISTIC-RECRUITING.md`, `voice-lines/samples/compare_20261012/`. Never touch them.
- Edit only the files your brief owns. Exception: if the tree does not compile because of an interface mismatch between your set and another agent's set, you may make the smallest possible compile fix in the other file with the Edit tool (never Write, never reformat), and you list it under "edits outside my set" in your report. Other agents are editing their own sets concurrently, so never rewrite whole files you do not own.
- No em dashes (U+2014) anywhere: code, comments, docs, tests, messages. Use a comma, colon or full stop. Before reporting, grep your changed files for U+2014.

## Toolchain
- Every PowerShell shell needs: `$env:VCPKG_ROOT='C:\dev\vcpkg'; $env:Path="C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;"+$env:Path`. pwsh 7 (needed by setup-guerrilla.ps1 and new-remaster-shadow.ps1) is `C:\dev\vcpkg\downloads\tools\powershell-core-7.6.2-windows\pwsh.exe`.
- Build ONLY in your own build dir, from `D:\Arma_CWA\arma_CWR`: `cmake --build build/<dir> --target PoseidonGame PoseidonTools PoseidonTests -- -j 3`. Never more than `-j 3`: several agents build at once and the default parallelism crashes clang-cl with out-of-memory. If a build dies with 0x5AF or "paging file too small", retry at `-j 2`.
- If your dir shows stale-object symptoms (link errors about old symbols, in-game flicker, broken rendering), reconfigure: `cmake --preset win-x64-clang-rwdi -B build/<dir>` then rebuild.
- `PoseidonTests.exe` is at `build/<dir>/tests/unit/engine/Poseidon/PoseidonTests.exe` (not in dist). Run tags like `PoseidonTests.exe "[occupierforces]"`.
- Known pre-existing reds (not yours): 5 unit failures in test_map_legend_localized / test_equipment_localization (missing packages/Remaster); tri `guerrilla_outfit_scope_label_e2e` and three stock-menu wizard tests; `remaster_ud_guerrilla_sinai_lobo` flakes once sometimes.
- Never touch `D:\Arma_CWA\tmp\remaster-shadow-67`, `build/win-x64-clang-rwdi`, `dist/x64-win-rwdi`, any `D:\Arma_CWA\tmp\remaster-shadow-b*`, the Steam folder (`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered`, read-only), Bohemia packaged data, or `@LoBo` pbo bytes.

## Shadows and tri
- Each agent runs tri against its own shadow `D:\Arma_CWA\tmp\remaster-shadow-ofN` with `--game-dir dist/<dir>`. After any edit under `guerrilla-mode/core`, `guerrilla-mode/mission` or a template, re-run `& $pwsh -File guerrilla-mode/install-missions.ps1 -GameDir D:/Arma_CWA/tmp/remaster-shadow-ofN` on your shadow (and `setup-guerrilla.ps1 -GameDir <shadow> -LoBoDir <shadow>/@LoBo -Tools ./dist/<dir>/PoseidonTools.exe` when engine-side resources changed).
- Making a new shadow (only when your brief says so): `& $pwsh -File tools/runtime/new-remaster-shadow.ps1 -Source 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered' -Destination 'D:/Arma_CWA/tmp/remaster-shadow-ofN' -LoBoSource 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered/@LoBo'` then setup-guerrilla.ps1 and install-missions.ps1 as above. Never delete a shadow recursively (its junctions reach the Steam files).
- Tri: from `D:\Arma_CWA\arma_CWR`: `.\target\debug\tri.exe test --retries 3 -j2 --game-dir dist/<dir> --data-dir D:/Arma_CWA/tmp/remaster-shadow-ofN <test paths>`. Tests pinned to `packages/RemasterUD` (a `data_dir` line in the toml) ignore `--data-dir`: copy them to `arma_CWR/tmp/ofN-tests/` with the `data_dir` line removed, run from the copy, delete the copies afterwards. Tri prints only FAILs; a short green run is real.
- Birth and pop-in tests need a real camera (not headless): assert `gmTrafficPercept` reports one and the 750 m day floor. Tests that reach an END transition need `no_autotest = true` and `--focus` (copy the pattern of `tests/integration/ui/remaster/remaster_ud_guerrilla_abel_win_screen.test.toml`). The owner watches the desktop: run tri at `-j1` when a test carries `--focus`, and never leave game windows up.
- Never stop PoseidonGame processes wholesale (other sessions run instances). Stop only PIDs you spawned; close what you open.
- `tests/contracts/test_human_suite.py` requires every `engine/Poseidon/Game/Guerrilla/*.cpp` to be classified in `guerrilla-mode/mission/Showcase.Abel/human/coverage.json`; new source files need entries. Run `python -m unittest tests.contracts.test_human_suite` from arma_CWR when you add one.
- SQS gotchas: a mid-line `;` executes as code; no short-circuit evaluation; no `bool !=`; `triAssert` with a bare bool kills the harness. Group Move / arcade waypoints never move a seated crew: use the doMove `IssueCommand` idiom.

## Agents and handoff
- You may spawn your own sub-agents with the Agent tool, always with `model: "opus"`, each with the same rules and a disjoint file set inside your own set; they build in your dir only, one build at a time.
- If your context grows long or the work is far from done, hand the remainder to a fresh Opus agent with a self-contained brief (ownership, build dir, shadow, what is done, what is left, how to verify) and end your turn naming the handoff and its agent.
- Stop and report (do not guess) when a D-decision proves impossible or self-contradictory in the engine, when something would need Bohemia data, Steam, @LoBo pbo bytes or a new branch, or after two consecutive failed attempts at the same verification.

## Report format (final message)
1. Files changed (full paths), and any edits outside your set.
2. The rule as implemented, in a few sentences each, with deviations from the plan doc called out explicitly.
3. Verification: the exact commands and their results, verbatim tails (unit counts, tri pass/fail per test). Anything not run is listed as unverified, never implied green.
4. Problems hit, parked items, and anything the orchestrator must know before committing.

## Desktop focus (owner request 2026-10-17)
- The owner uses the desktop while tests run. Run every tri.exe (and any game launch) through the private-desktop wrapper so no window reaches the owner's desktop:
  `powershell -NoProfile -File D:\Arma_CWA\tmp\tri-hidden-desktop.ps1 -Log <your log file> -WorkDir D:\Arma_CWA\arma_CWR -TimeoutSec <s> -Cmd 'target\debug\tri.exe test ...'`
  then read the log file (tri output goes there). Exit code is tri's. Verified: GL 3.3 works there, tests pass. To turn it off, run tri directly.
- Tests that carry `--focus` or need real input focus may behave differently on the private desktop; if one goes red only there, re-run it once directly and say so in the report.
