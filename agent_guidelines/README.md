# Agent guidelines

Short notes for AI agents, and for people, working in this repository. Each file
covers one topic: what goes wrong, why it happens, and what to do about it. They
add to the two `CLAUDE.md` files (repo root and `arma_CWR/`), which stay the
main guide.

| File | Topic |
| --- | --- |
| [flashing-main-menu.md](flashing-main-menu.md) | The main menu flickers during a `tri` mission run: the test mission never booted. How to find the reason and avoid it. |
| [p3d-texturing-reminders.md](p3d-texturing-reminders.md) | A model that looks right in Blender shows hollow heads and missing eyes in game: it was exported inside out. Turn on backface culling, compare winding with a stock model and photograph it in game. |

When you add a file: one finding per file, the date it was verified, code cited
as `path:line`, and a clear line between what was observed and what was
inferred. No em dashes.
