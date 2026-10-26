#!/bin/sh
# run_one.sh <ud|vanilla> <name>   one probe game, scoped junction, output under heads/2/<side>-<name>
H=/d/Arma_CWA_worktrees/graphics-remaster/tmp/video/heads/2
side=$1; name=$2
if [ "$side" = "ud" ]; then
  cd /d/Arma_CWA_worktrees/video-work/arma_CWR
  powershell -NoProfile -ExecutionPolicy Bypass -File tests/rendering/captures/run-captures.ps1 -Tests tests/rendering/captures/video/look_probe_head2_$name.test.sqf -GameDir 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi' -DataJunction RemasterUD-head2-ud -OutDir "D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads\2\ud-$name" > $H/ud-$name.log 2>&1
else
  rm -rf $H/van-$name-user; cp -r /d/Arma_CWA_worktrees/vanilla-upstream/tmp/video-userseed $H/van-$name-user
  cd /d/Arma_CWA_worktrees/vanilla-upstream
  POSEIDON_USER_DIR="D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads\2\van-$name-user" powershell -NoProfile -ExecutionPolicy Bypass -File tests/rendering/captures/run-captures.ps1 -Tests tests/rendering/captures/video/van_probe_head2_$name.test.sqf -GameDir 'D:\Arma_CWA_worktrees\vanilla-upstream\dist\vanilla-upstream-rwdi' -DataJunction RemasterUD-head2-van -OutDir "D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads\2\van-$name" > $H/van-$name.log 2>&1
fi
echo EXIT $? >> $H/$( [ "$side" = "ud" ] && echo ud || echo van )-$name.log
