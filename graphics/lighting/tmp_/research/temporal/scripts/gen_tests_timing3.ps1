. D:\Arma_CWA_worktrees\lighting\tmp\research\temporal\scripts\gen_funcs.ps1
# Third interleaved timing run (resumed session): run on a quiet machine, adds
# TAA without MSAA and the stochastic-effect costs. forest_edge_golden, Abel.
$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$FX = '"shadowStochastic=1", "shadowTaps=4", "sunSize=2.5", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"'
$PCSS12 = '"shadowStochastic=1", "shadowTaps=12", "sunSize=2.5"'
$v = 'forest_edge_golden'
$s = Head 'Abel' 7300 10900
$s += "[`"$v`", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin`ncall { [[`"SoldierWB`"], 12, 2, 215, `"man`"] call capSpawn; }`ntriSimFrames 150`n"
$modes = @(
    @('A_msaa4_15', 4, 1.5, ''),
    @('D_msaa4_10', 4, 1.0, ''),
    @('G_taa_plain_msaa4_10', 4, 1.0, '"taa=1"'),
    @('B_taa_best_msaa4_10', 4, 1.0, $BEST),
    @('E_taa_best_nomsaa_15', 0, 1.5, $BEST),
    @('K_taa_best_nomsaa_10', 0, 1.0, $BEST),
    @('C_taa_best_fx_msaa4_10', 4, 1.0, "$BEST, $FX"),
    @('P_pcss12_noTAA_msaa4_15', 4, 1.5, $PCSS12)
)
for ($round = 0; $round -lt 3; $round++) {
  foreach ($t in $modes) {
    $s += "[`"$($t[0])`", `"remastered`", $($t[1]), $($t[2]), [$($t[3])]] call capVariant`n[] call capPin`ntriSimFrames 60`ncall { `"$($t[0])_r$round`" call capTimeSamples }`n"
  }
}
$s += "[] call capClear`n`ncall { `"view=done;event=done`" call capLog }`ntriEndTest`n"
[IO.File]::WriteAllText("$d\abel_taa_timing3.test.sqf", $s)
Copy-Item "$d\abel_taa.test.toml" "$d\abel_taa_timing3.test.toml" -Force
