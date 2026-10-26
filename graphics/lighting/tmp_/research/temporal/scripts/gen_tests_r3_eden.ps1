. D:\Arma_CWA_worktrees\lighting\tmp\research\temporal\scripts\gen_funcs.ps1
$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$FX = '"shadowStochastic=1", "shadowTaps=4", "sunSize=2.5", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"'
$v = 'overcast_broken'
$s = Head 'Eden' 9200 3700
$s += "[`"$v`", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin`ncall { [[`"Jeep`"], 12, -5, 70, `"veh`"] call capSpawn; }`ntriSimFrames 150`n"
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'best10msaa4' 'remastered' 4 1.0 $BEST 30
$s += Still $v 'best10msaa4fx' 'remastered' 4 1.0 "$BEST, $FX" 30
$s += Pan $v 'panbest10msaa4' 4 1.0 $BEST 0.05 0 -3 0
$s += "[] call capClear`n`ncall { `"view=done;event=done`" call capLog }`ntriEndTest`n"
[IO.File]::WriteAllText("$d\eden_taa_r3.test.sqf", $s)
Copy-Item "$d\eden_taa.test.toml" "$d\eden_taa_r3.test.toml" -Force