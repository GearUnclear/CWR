#!/bin/sh
# shoot.sh NAME [tests...]  -> tmp/captures/NAME
N=$1; shift
cd /d/Arma_CWA_worktrees/look-director/arma_CWR
T=${*:-"tests/rendering/captures/abel_lighting.test.sqf,tests/rendering/captures/eden_lighting.test.sqf,tests/rendering/captures/sinai_lobo_lighting.test.sqf,tests/rendering/captures/look_final_abel.test.sqf,tests/rendering/captures/look_final_eden.test.sqf,tests/rendering/captures/look_final_sinai.test.sqf,tests/rendering/captures/look_director_optics.test.sqf"}
powershell -NoProfile -File tests/rendering/captures/run-captures.ps1 -Tests "$T" -GameDir dist/look-director-rwdi -DataJunction RemasterUD-look-director -OutDir D:/Arma_CWA_worktrees/look-director/tmp/captures/$N > ../tmp/$N.log 2>&1
tail -6 ../tmp/$N.log
