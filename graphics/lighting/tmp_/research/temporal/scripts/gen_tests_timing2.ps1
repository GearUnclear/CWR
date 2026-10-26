. D:\Arma_CWA_worktrees\lighting\tmp\research\temporal\scripts\gen_funcs.ps1
$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$v = 'forest_edge_golden'
$s = Head 'Abel' 7300 10900
$s += "[`"$v`", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin`ncall { [[`"SoldierWB`"], 12, 2, 215, `"man`"] call capSpawn; }`ntriSimFrames 150`n"
$modes = @(@('A_msaa4_15', 4, 1.5, ''), @('D_msaa4_10', 4, 1.0, ''), @('G_taa_plain_msaa4_10', 4, 1.0, '"taa=1"'), @('H_mip_msaa4_10', 4, 1.0, '"taaMipBias=-0.5"'), @('B_taa_best_msaa4_10', 4, 1.0, $BEST), @('J_taa_nojitter_msaa4_10', 4, 1.0, '"taa=1", "taaJitter=0"'))
for ($round = 0; $round -lt 4; $round++) {
  foreach ($t in $modes) {
    $s += "[`"$($t[0])`", `"remastered`", $($t[1]), $($t[2]), [$($t[3])]] call capVariant`n[] call capPin`ntriSimFrames 60`ncall { `"$($t[0])_r$round`" call capTimeSamples }`n"
  }
}
$s += "[] call capClear`n`ncall { `"view=done;event=done`" call capLog }`ntriEndTest`n"
[IO.File]::WriteAllText("$d\abel_taa_timing2.test.sqf", $s)
Copy-Item "$d\abel_taa.test.toml" "$d\abel_taa_timing2.test.toml" -Force