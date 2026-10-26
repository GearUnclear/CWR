# Round 4 (resumed session): temporal convergence of the stochastic effects
# frame by frame after a history reset, and the same effects under camera
# motion (yaw pan and strafe) with and without TAA. forest_edge_golden, Abel.
. D:\Arma_CWA_worktrees\lighting\tmp\research\temporal\scripts\gen_funcs.ps1
$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$BESTOFF = '"taa=0", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$FX = '"shadowStochastic=1", "shadowTaps=4", "sunSize=2.5", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"'
$REF = '"shadowStochastic=1", "shadowTaps=16", "sunSize=2.5", "aoRadius=2.5"'

$v = 'forest_edge_golden'
$s = Head 'Abel' 7300 10900
$s += "[`"$v`", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin`ncall { [[`"SoldierWB`"], 12, 2, 215, `"man`"] call capSpawn; }`ntriSimFrames 150`n"

# References (still)
$s += Still $v 'r4_remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'r4_ref16_noTAA' 'remastered' 4 1.0 $REF 3

# Convergence: FX without TAA, then TAA switched on (fresh history) and shots
# at 0, 1, 2, 4, 8, 16, 32, 64 frames of accumulation.
$s += "[`"r4_conv`", `"remastered`", 4, 1.0, [$BESTOFF, $FX]] call capVariant`ntriSimFrames 90`n[] call capPin`ntriSimFrames 3`n"
$s += "triScreenshot `"$($v)__conv_f000`"`n`"$($v)__conv_f000`" call capRecord`n"
$s += "call { capKnobRes = triSetLighting `"taa=1`" }`n"
$prev = 0
foreach ($k in 1, 2, 4, 8, 16, 32, 64) {
    $step = $k - $prev
    if ($step -gt 1) { $s += "triSimFrames $($step - 1)`n" }
    $lab = "{0}__conv_f{1:D3}" -f $v, $k
    $s += "triScreenshot `"$lab`"`n`"$lab`" call capRecord`n"
    $prev = $k
}

# Motion: yaw pan and strafe with the stochastic effects on.
$s += Pan $v 'panfx_noTAA' 4 1.0 "$BESTOFF, $FX" 0.05 0 -3 0
$s += Pan $v 'panfx_TAA' 4 1.0 "$BEST, $FX" 0.05 0 -3 0
$s += Pan $v 'strafefx_noTAA' 4 1.0 "$BESTOFF, $FX" 0 0.03 0 -1
$s += Pan $v 'strafefx_TAA' 4 1.0 "$BEST, $FX" 0 0.03 0 -1
$s += Pan $v 'strafemsaa' 4 1.5 '' 0 0.03 0 -1

$s += "[] call capClear`n`ncall { `"view=done;event=done`" call capLog }`ntriEndTest`n"
[IO.File]::WriteAllText("$d\abel_taa_r4.test.sqf", $s)
Copy-Item "$d\abel_taa.test.toml" "$d\abel_taa_r4.test.toml" -Force
