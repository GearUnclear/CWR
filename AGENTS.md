# AGENTS.md

Guidance for AI coding agents working in this repository. The full project guide is [`CLAUDE.md`](CLAUDE.md) (and [`arma_CWR/CLAUDE.md`](arma_CWR/CLAUDE.md) for engine work); read those first. They apply to every agent, not only Claude. One-topic notes on repo findings (what goes wrong, why, what to do) live in [`agent_guidelines/`](agent_guidelines/README.md).

## Target platform

**UD targets the full official *Arma: Cold War Assault* Remaster. Classic 1.99 compatibility is being dropped (2026-10-11).** Build new engine, content, and test work against Remaster data and its `remaster.cpp` contract. Do not add new Classic-only shims, fallbacks, or test lanes; existing Classic support is legacy awaiting retirement. See the "Target platform" section of [`CLAUDE.md`](CLAUDE.md).

## Branches

**Make no new branches unless the user grants permission.** Work on the current branch (normally `main`). Do not run `git branch`, `git checkout -b`, `git switch -c`, `git worktree add -b`, or push a new branch to any remote without the user's explicit OK in chat for that branch. Approval for one branch does not extend to the next. This applies to subagents too.

## Game processes

**Never kill `PoseidonGame` globally** (no `Get-Process PoseidonGame | Stop-Process`): other sessions run games on this machine at the same time. Stop only your own with `arma_CWR/tools/stop-owned-games.ps1`, scoped by `-Pid` or `-DataDir`; see [`agent_guidelines/scoped-game-cleanup.md`](agent_guidelines/scoped-game-cleanup.md).
