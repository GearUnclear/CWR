$w='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix4/sun'
New-Item -ItemType Directory -Force "$w/shots_p6" | Out-Null
powershell -NoProfile -File "$w/work/run.ps1" -Tests tests/rendering/captures/look_r4suns_abel.test.sqf -Out "$w/shots_p6" -Ovr "$w/work/ovC"
