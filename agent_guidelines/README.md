# Agent guidelines

Short notes for AI agents, and for people, working in this repository. Each file
covers one topic: what goes wrong, why it happens, and what to do about it. They
add to the two `CLAUDE.md` files (repo root and `arma_CWR/`), which stay the
main guide.

| File | Topic |
| --- | --- |
| [flashing-main-menu.md](flashing-main-menu.md) | The main menu flickers during a `tri` mission run: the test mission never booted. How to find the reason and avoid it. |
| [scoped-game-cleanup.md](scoped-game-cleanup.md) | A `tri` run dies with exit -1 and an empty stderr: another session killed every `PoseidonGame`. Stop only your own games with `tools/stop-owned-games.ps1`. |
| [seated-man-deletion.md](seated-man-deletion.md) | `::DeleteVehicle` on a man in a vehicle seat leaves him in `World::_outVehicles` forever (invisible, saved). Move him out of the seat first. |

When you add a file: one finding per file, the date it was verified, code cited
as `path:line`, and a clear line between what was observed and what was
inferred. No em dashes.
- [tri splits a test line on every top-level semicolon](tri-splits-lines-on-semicolons.md): one-line set/measure/clear sequences are several harness commands; wrap them in a block.
