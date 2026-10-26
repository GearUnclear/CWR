CLEANUP NOTE: Uslu dur! trailer part 1 and the dawn-fog fix (written 2026-10-23, late evening)
==============================================================================================

Written by the Claude session that finished the trailer, at the owner's request, so cleanup agents can
act with confidence. The same facts are in the committed handoff:
graphics-remaster arma_CWR/engine/Poseidon/Graphics/Remaster/research/HANDOFF-2026-10-23.md, "STATUS" block.

WHAT WAS MOVED (owner's instruction: "move all assets to Z:\uploads")
- Every finished trailer file from graphics-remaster\tmp\video\v2\final\ (48 files) and the finished
  thumbnails from ...\v2\thumbs\ were MOVED to Z:\uploads\uslu_dur_trailer_part1\ (network share
  \\192.168.1.55\A7s3). 133 files, 5.43 GB at the destination; sizes checked against the originals;
  README.txt there says what each file is and what to upload.
- v2\final\ is EMPTY ON PURPOSE. Nothing in it was lost.

WHAT IS SAVED IN GIT (branch graphics-remaster, pushed to GearUnclear/CWR)
- 026f8b7c  the fog fix (engine), with its unit test; d91a08e0 before it was the rejected first fix.
- a12e0b67  all the trailer tools (trailer_v2.py --layout short/--timing, owner_vo.py, short_captions.py,
            run_v2_lane.ps1 -Expect, owner_take1/, thumbnail_bay/) and the fog checks (captures/fogcheck/).
- The scratch worktree D:\Arma_CWA_worktrees\video-work has no other uncommitted work worth keeping
  (its other changes are generated probe tests).

WHAT STAYS HERE AND WHAT DELETING IT COSTS (the owner decides)
- graphics-remaster\tmp\video\ (about 50 GB): per-clip frames of both sides (v2\ud, v2\vanilla), the silent
  montages (v2\out-split, out-sbs, out-short), the old clips (mist\backup_ud, mist\percell_ud), the fog
  investigation evidence (mist\, fogwind\, heads\), thumbnail work frames. Only needed to RE-EDIT the
  trailer; without it a re-edit means reshooting the clips (about 2 to 3 hours of game captures).
- D:\Arma_CWA_worktrees\look-director: the overhaul plus 026f8b7c's change as an UNSTAGED diff on fec1075;
  its engine equals graphics-remaster HEAD; its dist look-director-rwdi shot the trailer's overhaul side.
- D:\Arma_CWA_worktrees\vanilla-upstream: the Bohemia upstream 3.05 build used for every vanilla shot.
- Already removed with the owner's OK: mist-1, fog-8, fable-wt (their diffs are in
  tmp\video\fogwind\removed-worktrees\).

DANGER: DIRECTORY JUNCTIONS INTO SHARED GAME DATA
These folders hold junctions to D:\Arma_CWA\tmp\remaster-shadow-67 (the shared Remaster shadow):
  D:\Arma_CWA_worktrees\video-work\arma_CWR\packages        34 junctions
  D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\packages 32 junctions
  D:\Arma_CWA_worktrees\vanilla-upstream\packages            13 junctions
  D:\Arma_CWA_worktrees\look-director\arma_CWR\packages       7 junctions
Remove each junction by itself first (cmd /c rmdir "<junction>"  or  (Get-Item "<junction>").Delete()),
never with a recursive delete, and only then delete the worktree folder. Otherwise the recursive delete
follows the junction and wipes the shadow's game data.
