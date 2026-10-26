// ============================================================================
//  RESEARCH (temporal track): TAA variants on top of capture_lib.sqf.
//  #include "capture_lib.sqf" first.
//
//  [label, "classic"|"remastered", msaa, renderScale, [knob strings]] call capVariant
//  Every knob of the research set is reset first (capKnobsOff), then the
//  variant's own knobs apply, so variants never leak into each other.
// ============================================================================

capKnobsOff = ["taa=0", "taaAlpha=0.1", "taaSharpen=0.25", "taaJitter=1", "taaGamma=1", "taaDepthReject=1", "taaMipBias=0", "taaStochAlpha=0", "aoStochastic=0", "aoTaps=12", "aoRadius=1.2", "shadowStochastic=0", "shadowTaps=12", "sunSize=0", "debug=0"]

capVariant = {
    capModeName = _this select 0;
    capLit = triSetLighting (_this select 1);
    { capKnobRes = triSetLighting _x } forEach capKnobsOff;
    { capKnobRes = triSetLighting _x } forEach (_this select 4);
    capMsaaRes = triSetMsaa (_this select 2);
    capScaleRes = triSetRenderScale (_this select 3);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3;msaa=%4;scale=%5;knobs=%6;last=%7", capView, capModeName, capLit, _this select 2, _this select 3, _this select 4, capKnobRes] call capLog;
}

// Pan rig: per-frame camera steps driven from SQF (one triSetView, one
// rendered frame). capPanYaw0 / capPanSide are the start; each step adds
// capPanDYaw degrees of heading and capPanDSide metres of sideways travel.
capPanSet = {
    private ["_h", "_dx", "_dy", "_dz", "_ox", "_oz"];
    _h = capH + capPanYaw;
    _dx = (sin _h) * (cos capP);
    _dy = sin capP;
    _dz = (cos _h) * (cos capP);
    _ox = capOX + capPanSide * (cos capH);
    _oz = capOZ - capPanSide * (sin capH);
    capViewRes = triSetView [_ox, capEyeASL, _oz, _dx, _dy, _dz];
}
capPanReset = {
    capPanYaw = _this select 0;
    capPanSide = _this select 1;
    [] call capPanSet;
}
// n call capPanRun: n steps, one rendered frame each.
capPanRun = {
    private ["_i"];
    _i = 0;
    while {_i < _this} do {
        capPanYaw = capPanYaw + capPanDYaw;
        capPanSide = capPanSide + capPanDSide;
        [] call capPanSet;
        capSimRes = triSimFrames 1;
        _i = _i + 1;
    };
}
