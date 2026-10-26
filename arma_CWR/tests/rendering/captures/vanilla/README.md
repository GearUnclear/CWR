# Vanilla capture rig (copy)

These files are the capture rig that shot the vanilla (Bohemia upstream 3.05) side of the
Uslu dur! lighting comparisons and of trailer part 1. They run from a separate checkout of
upstream 3.05, the `vanilla-upstream` worktree (`D:/Arma_CWA_worktrees/vanilla-upstream` on
the owner's machine), not from this tree: the test files name their mission as
`tests/rendering/captures/missions/...`, a path relative to that checkout's root.

Kept here so the rig is not local-only. To use it:

1. Check out upstream 3.05 (tag or commit `ffc61838`) somewhere and build it.
2. Copy this folder's contents into that checkout's `tests/rendering/captures/`, and copy
   `../missions/udcap_*` (identical to the vanilla ones) next to them.
3. Run `run-captures.ps1` there (see its header for the parameters).

The vanilla video clip tests (`video/van_<clip>.test.sqf`) are generated, not stored:
`../video/gen_video_v2.py --side vanilla` writes them into the vanilla checkout.

`capture_lib.sqf` and `video/capture_motion.sqf` here are the vanilla-safe variants: they only
use verbs the 3.05 engine has, so they differ from the copies one folder up.
