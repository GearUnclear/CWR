$d = 'D:\Arma_CWA_worktrees\gfx-temporal\arma_CWR\tests\rendering\research\temporal'
. D:\Arma_CWA_worktrees\lighting\tmp\research\temporal\scripts\gen_funcs.ps1
$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$FX = '"shadowStochastic=1", "shadowTaps=4", "sunSize=2.5", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"'
$v = 'sinai_lowsun_fp'
$s = Head 'Sinai' 8350 2950
$s += "[`"$v`", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin`ncall {`n    [[`"LoBoHMWV_Mag_Patrol`", `"Jeep`"], 10, -5, -107.8, `"veh`"] call capSpawn;`n    [[`"LoBo_Zelda`", `"M113`"], 16, 5, 72.2, `"veh`"] call capSpawn;`n    [[`"LoBoGolaniWB`", `"SoldierWB`"], 7, 1, -112.8, `"man`"] call capSpawn;`n    [[`"LoBo_camnet`", `"CampEast`", `"Camp`"], 22, -3, -152.8, `"veh`"] call capSpawn;`n}`ntriSimFrames 150`n"
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'msaa4_10' 'remastered' 4 1.0 '' 3
$s += Still $v 'best15_g15' 'remastered' 0 1.5 "$BEST, `"taaGamma=1.5`"" 30
$s += Still $v 'best15msaa2' 'remastered' 2 1.5 $BEST 30
$s += Still $v 'best15msaa4' 'remastered' 4 1.5 $BEST 30
$s += Still $v 'best10msaa2' 'remastered' 2 1.0 $BEST 30
$s += Still $v 'best10msaa4' 'remastered' 4 1.0 $BEST 30
$s += Still $v 'best15msaa4fx' 'remastered' 4 1.5 "$BEST, $FX" 30
$s += Still $v 'best10msaa4fx' 'remastered' 4 1.0 "$BEST, $FX" 30
$s += Pan $v 'panmsaa' 4 1.5 '' 0.05 0 -3 0
$s += Pan $v 'panbest10msaa4' 4 1.0 $BEST 0.05 0 -3 0
foreach ($t in @(@('t_msaa15', 4, 1.5, ''), @('t_msaa10', 4, 1.0, ''), @('t_best15', 0, 1.5, $BEST), @('t_best10', 0, 1.0, $BEST), @('t_best15msaa4', 4, 1.5, $BEST), @('t_best10msaa4', 4, 1.0, $BEST), @('t_best10msaa4fx', 4, 1.0, "$BEST, $FX"), @('t_best10msaa2', 2, 1.0, $BEST))) {
  $s += "[`"$($t[0])`", `"remastered`", $($t[1]), $($t[2]), [$($t[3])]] call capVariant`n[] call capPin`ntriSimFrames 90`ncall { `"$($t[0])`" call capTimeSamples }`n"
}
$s += "[] call capClear`n`ncall { `"view=done;event=done`" call capLog }`ntriEndTest`n"
[IO.File]::WriteAllText("$d\sinai_taa_r3.test.sqf", $s)
Copy-Item "$d\sinai_taa.test.toml" "$d\sinai_taa_r3.test.toml" -Force