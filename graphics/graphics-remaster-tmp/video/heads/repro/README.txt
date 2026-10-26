Head-wobble shared repro and metric (agent 8, lead)
===================================================

What it is
  One short scene on both builds: Malden (Abel), the checkpoint road
  (7266.4, 8355.9), 5 July 1985 11:00, clear, FIXED camera at 1.7 m looking
  heading 200, 120 frames shot with the v2 clips' exact per-frame loop
  (capClockAt, capPathAt, capPropsAt, triSimFrames 2, triScreenshot), then the
  men are deleted and one empty "plate" frame is shot from the same camera.
  The men, all facing the camera, in the global array capHeads:
    A  SoldierEB  SAFE (the clips' posed man)    7 m ahead, 3 m left
    B  SoldierEB  AWARE (weapon raised)          9 m ahead, 1 m left
    C  Civilian   SAFE, civilian group           8 m ahead, 1.2 m right
    D  SoldierEB  SAFE, the clips' distance     22 m ahead, 8 m right
  The metric numbers heads left to right on screen (see boxes_<label>.png).

Files
  gen_repro.py   writes the two probe tests (overhaul + vanilla) for a variant
  run_repro.ps1  runs one probe on one build, then metric.py on its frames
  metric.py      the head-wobble metric (plate mode for probes, --track for clips)

Make a variant (N = your agent number, X = a short name)
  python gen_repro.py --agent N --name X
      [--setup "<sqf>"]     once, after the men are spawned and posed
      [--preclip "<sqf>"]   once, right before frame 0
      [--perframe "<sqf>"]  every frame, before the sim frames
      [--sim 2]             sim frames per video frame (the clips use 2)
      [--frames 120]
      [--ud-setup/--ud-preclip/--ud-perframe "<sqf>"]  overhaul-only verbs
      [--side ud|van|both]
  Each knob may be given several times; each value becomes one top-level
  statement, so wrap multi-statement code in  call { ... }  and never return
  a "FAIL:" string from a statement. Example (doWatch straight ahead):
    python gen_repro.py --agent 1 --name watch --setup "call { { _x doWatch [...]} forEach capHeads }"
  Output tests:
    D:\Arma_CWA_worktrees\video-work\arma_CWR\tests\rendering\captures\video\look_probe_headN_X.test.sqf
    D:\Arma_CWA_worktrees\vanilla-upstream\tests\rendering\captures\video\van_probe_headN_X.test.sqf

Run it (one game at a time; about 2 to 4 minutes each; run it in the background)
  powershell -NoProfile -File run_repro.ps1 -Side ud  -Agent N -Name X
  powershell -NoProfile -File run_repro.ps1 -Side van -Agent N -Name X
  Output: D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads\N\<ud|van>-X\
    status.txt (exit, frame count), run.log, tri\ (frames + game_stdout.log),
    metric.txt (the table), metric_<label>.json (per pair series),
    crops_<label>_<i>.png (head i over 60 frames, 3x, rows of 10),
    sil_<label>_<i>.png (the same with the plate silhouette outlined),
    boxes_<label>.png (frame 1 with man, head and background boxes).
  Cleanup if a run hangs (scoped to your junction only):
    powershell -NoProfile -File D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -DataDir <engine root>\packages\RemasterUD-headN-<ud|van>
    (engine root: D:\Arma_CWA_worktrees\video-work\arma_CWR for ud,
     D:\Arma_CWA_worktrees\vanilla-upstream for van)

Metric (python metric.py <out dir> [--out DIR] [--label L])
  per head, over every consecutive frame pair:
    shift px    sub-pixel translation of the head crop (phase correlation):
                a head turning, nodding or bobbing; mean / p95 / max
    twitch      number of pairs whose head shift exceeds 0.5 px
    sil px      head silhouette (|frame - plate| > 14) centroid shift
    silchg      1 - IoU of the head silhouette frame to frame
    shade       mean |luma change| (0..255) over pixels that are head in
                both frames: shading / texture change without motion
    torso       the same shift and shade for the torso box: a control that
                tells whole-man motion from head-only motion
    bg          mean |luma change| of a man-free patch: global flicker
  A still head reads shift ~0.0-0.1 px, twitch 0, silchg < 0.02.
  Real clips (moving camera, no plate):
    python metric.py <clip>.mkv --boxes "x,y,w,h;x,y,w,h" --track --start 0 --count 120 --out DIR --label L
    (boxes = whole-man boxes in the first analysed frame; heads are measured
     relative to their own body, so camera motion cancels)
