# graphics-remaster: final check after the parallel lanes

Branch `graphics-remaster` at **fec1075** (= origin/graphics-remaster), worktree
`D:\Arma_CWA_worktrees\graphics-remaster`, build `build/graphics-remaster-rwdi` (incremental
rebuild of the tip, exit 0: `FINAL/build.log`), game `dist/graphics-remaster-rwdi`. Run on
2026-10-18 from 20:00. Data junction `packages\RemasterUD-gr-final` (one probe used
`RemasterUD-gr-final2`). All evidence is under `tmp/` (gitignored).

Regressions fixed in this check: **none** (no commit made). Nothing found needed an engine
change; the open items below are owner decisions or documented risks the lanes already carry.

## 1. Gate results

| Gate item (plan 8) | Result | Evidence |
|---|---|---|
| 1 Build | exit 0 on the tip | `FINAL/build.log` |
| 3 Unit, full suites from `D:\` | PoseidonTests 3331 passed / 6 failed / 4 skipped of 3341 cases (7 of 4,056,661 assertions); the 6 are the known environment set: 5 `[ui]` localization cases (`STRINGTABLE_MAP` legend keys, `STRINGTABLE_EQUIPMENT` M16 keys, three `equipment.utf8.html` cases: the files are not found) and `MP version rejection message follows Czech language selection` (its Game and Demo sections). PoseidonCoreTests 823 passed, 4 skipped, 0 failed. PoseidonFoundationTests 475 cases, all passed | `FINAL/unit-*.log` |
| 4 Smoke, three islands | `passed 3/3` (smoke_abel, smoke_eden, smoke_sinai_lobo); KHR_debug ids 131169 (LOW), 131218 and 131154 (MEDIUM) only, once each per game, the known set; no shader compile or link error, no ERROR line; Trident's GL error post-check passed | `tmp/captures/final-smoke` |
| 5 Off equals baseline, whole branch | `triSetLighting "baseline"` (every switch off) on the final build against the A1 build's `remastered` (nothing landed) at the 16 hero views: 11 views within 0 to 2 pixels over 2 levels (most 0 or 1 level max); the rest are within A1's own boot-to-boot noise (table 3) | `FINAL/baseline-vs-A1.md`, `tmp/captures/final-baseline` |
| 6 Capture set, everything on | 3 islands, 30 shots, 15 pairs | `tmp/captures/final` |
| 6 Capture pairs, off/on per feature | 16 hero views, 32 shots | `tmp/captures/final-features` |
| 7 Timing | see section 4 | |
| 8 Look check | see section 5 | |

TIMING_PLACEHOLDER
