# graphics-experimental reports

The overnight experiment swarm of 2026-11-03 (perf squad, issue #216): one switch-gated lane per idea on top of graphics-remaster 243ef1e0.

- `MORNING.md`: the summary (what landed, what is on by default, costs, what needs the owner's eye in motion).
- `xp-*.md`: lane reports kept by the integrator.
- `xp-farshade-unbuilt.patch`: the cheaper far-object shader lane. Its build and tests never ran, so it is NOT on the branch as code; apply it to a scratch worktree at 243ef1e0 to continue it.

Every experiment sits behind a `POSEIDON_XP_*` switch and is off by default, except xp-cputhreads (pixel-identical, 0.8 to 1.0 ms less CPU work).
