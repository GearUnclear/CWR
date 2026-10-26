$d = 'D:\Arma_CWA_worktrees\gfx-temporal\arma_CWR\tests\rendering\research\temporal'
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
