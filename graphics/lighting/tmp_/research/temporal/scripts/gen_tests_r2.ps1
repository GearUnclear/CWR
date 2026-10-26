# Generates the round-2 research capture tests of the temporal track into the
# scratch worktree's git-excluded research folder. Run from anywhere.
$d = 'D:\Arma_CWA_worktrees\gfx-temporal\arma_CWR\tests\rendering\research\temporal'

$BEST = '"taa=1", "taaSharpen=0.5", "taaMipBias=-0.5", "taaStochAlpha=1"'
$FX = '"shadowStochastic=1", "shadowTaps=4", "sunSize=2.5", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"'

function Still($v, $lab, $mode, $msaa, $scale, $knobs, $settle, [switch]$twice) {
    $s = "[`"$lab`", `"$mode`", $msaa, $scale, [$knobs]] call capVariant`ntriSimFrames 90`n[] call capReadTimers`n[] call capPin`ntriSimFrames $settle`ntriScreenshot `"$($v)__$lab`"`n`"$($v)__$lab`" call capRecord`n"
    if ($twice) { $s += "triSimFrames 1`ntriScreenshot `"$($v)__$($lab)_b`"`n`"$($v)__$($lab)_b`" call capRecord`n" }
    return $s
}
function Pan($v, $tag, $msaa, $scale, $knobs, $dyaw, $dside, $yaw0, $side0) {
    $s = "[`"$tag`", `"remastered`", $msaa, $scale, [$knobs]] call capVariant`ntriSimFrames 60`n"
    $s += "call { capPanDYaw = $dyaw; capPanDSide = $dside; [$yaw0, $side0] call capPanReset }`ntriSimFrames 30`ncall { 40 call capPanRun }`n"
    for ($i = 0; $i -lt 6; $i++) {
        $lab = "{0}__{1}_f{2:D2}" -f $v, $tag, $i
        $s += "call { 2 call capPanRun }`ntriScreenshot `"$lab`"`n`"$lab`" call capRecord`n"
    }
    return $s
}
function Ghost($v, $tag, $msaa, $knobs, $fwd, $right) {
    $s = "[`"$tag`", `"remastered`", $msaa, 1.5, [$knobs]] call capVariant`n[] call capPin`ntriSimFrames 60`n"
    $s += "call { capMover = [[`"Jeep`"], $fwd, $right, 90, `"veh`"] call capSpawn; _p = [$fwd, $right] call capRel; capMvX = _p select 0; capMvZ = _p select 1; capMvDX = 0.12 * (cos capH); capMvDZ = -0.12 * (sin capH) }`n"
    $s += "call { 20 call capMoveRun }`n"
    for ($i = 0; $i -lt 6; $i++) {
        $lab = "{0}__{1}_f{2:D2}" -f $v, $tag, $i
        $s += "call { 3 call capMoveRun }`ntriScreenshot `"$lab`"`n`"$lab`" call capRecord`n"
    }
    $s += "call { deleteVehicle capMover }`n"
    return $s
}
function AoDbg($v) {
    $s = Still $v 'aodbg_current' 'remastered' 4 1.5 '"debug=4"' 3
    $s += Still $v 'aodbg_stoch_noTAA' 'remastered' 4 1.5 '"debug=4", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"' 3
    $s += Still $v 'aodbg_stoch_TAA' 'remastered' 0 1.5 '"debug=4", "taa=1", "aoStochastic=1", "aoTaps=4", "aoRadius=2.5"' 30
    return $s
}
function Head($world, $px, $pz) {
    $h = [IO.File]::ReadAllText("$d\sinai_taa2.test.sqf")
    $h = $h.Substring(0, $h.IndexOf('["sinai_lowsun_fp"'))
    $h = $h.Replace('capWorld = "Sinai"', "capWorld = `"$world`"").Replace('[8350, 2950, 0]', "[$px, $pz, 0]").Replace('[8350, 2950, 2]', "[$px, $pz, 2]").Replace('world=Sinai', "world=$world")
    return $h
}
$tail = "call { `"view=done;event=done`" call capLog }`ntriEndTest`n"

# ---------------- Abel
$v = 'forest_edge_golden'
$s = Head 'Abel' 7300 10900
$s += "[`"$v`", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin`ncall { [[`"SoldierWB`"], 12, 2, 215, `"man`"] call capSpawn; }`ntriSimFrames 150`n"
$s += Still $v 'classic' 'classic' 4 1.5 '' 3
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'taa15' 'remastered' 0 1.5 '"taa=1"' 30
$s += Still $v 'taa15_sa' 'remastered' 0 1.5 '"taa=1", "taaStochAlpha=1"' 30
$s += Still $v 'taa15_mip' 'remastered' 0 1.5 '"taa=1", "taaMipBias=-0.5"' 30
$s += Still $v 'best15' 'remastered' 0 1.5 $BEST 30 -twice
$s += Still $v 'best10' 'remastered' 0 1.0 $BEST 30
$s += Still $v 'best15fx' 'remastered' 0 1.5 "$BEST, $FX" 30
$s += Still $v 'best10fx' 'remastered' 0 1.0 "$BEST, $FX" 30
$s += Still $v 'best10msaa4' 'remastered' 4 1.0 $BEST 30 -twice
$s += Still $v 'best10msaa4fx' 'remastered' 4 1.0 "$BEST, $FX" 30
$s += AoDbg $v
$s += Pan $v 'panmsaa' 4 1.5 '' 0.05 0 -3 0
$s += Pan $v 'panbest15' 0 1.5 $BEST 0.05 0 -3 0
$s += Pan $v 'panbest10msaa4' 4 1.0 $BEST 0.05 0 -3 0
$s += Ghost $v 'ghostmsaa' 4 '' 16 -9
$s += Ghost $v 'ghosttaa' 0 $BEST 16 -9
$s += Ghost $v 'ghosttaa_norej' 0 "$BEST, `"taaDepthReject=0`"" 16 -9
$s += "[] call capClear`n`n"
$v = 'town_night'
$s += "[`"$v`", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin`ncall { capFire = [[`"Fire`"], 9, 3, 0, `"veh`"] call capSpawn; if (not (isNull capFire)) then { capFire inflame true }; }`ntriSimFrames 150`n"
$s += Still $v 'classic' 'classic' 4 1.5 '' 3
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'taa15' 'remastered' 0 1.5 '"taa=1"' 30
$s += Still $v 'best15' 'remastered' 0 1.5 $BEST 30
$s += Still $v 'best10' 'remastered' 0 1.0 $BEST 30
$s += Still $v 'best10fx' 'remastered' 0 1.0 "$BEST, $FX" 30
$s += Still $v 'best10msaa4' 'remastered' 4 1.0 $BEST 30
$s += Still $v 'best10msaa4fx' 'remastered' 4 1.0 "$BEST, $FX" 30
$s += AoDbg $v
$s += Pan $v 'panmsaa' 4 1.5 '' 0.05 0 -3 0
$s += Pan $v 'panbest15' 0 1.5 $BEST 0.05 0 -3 0
$s += Pan $v 'panbest10msaa4' 4 1.0 $BEST 0.05 0 -3 0
$s += "[] call capClear`n`n" + $tail
[IO.File]::WriteAllText("$d\abel_taa_r2.test.sqf", $s)
Copy-Item "$d\abel_taa.test.toml" "$d\abel_taa_r2.test.toml" -Force

# ---------------- Sinai
$v = 'sinai_lowsun_fp'
$s = Head 'Sinai' 8350 2950
$s += "[`"$v`", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin`ncall {`n    [[`"LoBoHMWV_Mag_Patrol`", `"Jeep`"], 10, -5, -107.8, `"veh`"] call capSpawn;`n    [[`"LoBo_Zelda`", `"M113`"], 16, 5, 72.2, `"veh`"] call capSpawn;`n    [[`"LoBoGolaniWB`", `"SoldierWB`"], 7, 1, -112.8, `"man`"] call capSpawn;`n    [[`"LoBo_camnet`", `"CampEast`", `"Camp`"], 22, -3, -152.8, `"veh`"] call capSpawn;`n}`ntriSimFrames 150`n"
$s += Still $v 'classic' 'classic' 4 1.5 '' 3
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'taa15' 'remastered' 0 1.5 '"taa=1"' 30
$s += Still $v 'best15' 'remastered' 0 1.5 $BEST 30 -twice
$s += Still $v 'best10' 'remastered' 0 1.0 $BEST 30
$s += Still $v 'best15fx' 'remastered' 0 1.5 "$BEST, $FX" 30
$s += Still $v 'best10fx' 'remastered' 0 1.0 "$BEST, $FX" 30
$s += Ghost $v 'ghostmsaa' 4 '' 11.5 1.5
$s += Ghost $v 'ghosttaa' 0 $BEST 11.5 1.5
$s += Ghost $v 'ghosttaa_norej' 0 "$BEST, `"taaDepthReject=0`"" 11.5 1.5
$s += Pan $v 'panmsaa' 4 1.5 '' 0.05 0 -3 0
$s += Pan $v 'panbest15' 0 1.5 $BEST 0.05 0 -3 0
$s += Pan $v 'strafemsaa' 4 1.5 '' 0 0.04 0 -1.5
$s += Pan $v 'strafebest15' 0 1.5 $BEST 0 0.04 0 -1.5
$s += "[] call capClear`n`n" + $tail
[IO.File]::WriteAllText("$d\sinai_taa_r2.test.sqf", $s)
Copy-Item "$d\sinai_taa.test.toml" "$d\sinai_taa_r2.test.toml" -Force

# ---------------- Eden
$v = 'overcast_broken'
$s = Head 'Eden' 9200 3700
$s += "[`"$v`", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin`ncall { [[`"Jeep`"], 12, -5, 70, `"veh`"] call capSpawn; }`ntriSimFrames 150`n"
$s += Still $v 'classic' 'classic' 4 1.5 '' 3
$s += Still $v 'remastered' 'remastered' 4 1.5 '' 3
$s += Still $v 'best15' 'remastered' 0 1.5 $BEST 30
$s += Still $v 'best10' 'remastered' 0 1.0 $BEST 30
$s += Still $v 'best10fx' 'remastered' 0 1.0 "$BEST, $FX" 30
$s += Pan $v 'panmsaa' 4 1.5 '' 0.05 0 -3 0
$s += Pan $v 'panbest15' 0 1.5 $BEST 0.05 0 -3 0
$s += "[] call capClear`n`n" + $tail
[IO.File]::WriteAllText("$d\eden_taa_r2.test.sqf", $s)
Copy-Item "$d\eden_taa.test.toml" "$d\eden_taa_r2.test.toml" -Force
Get-ChildItem $d -Filter '*_r2*' | Select-Object Name, Length
