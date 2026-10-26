// RESEARCH (temporal): moving-object rig (ghosting test) and repeated GPU timing samples.
// [object, dx per frame, dz per frame] set via globals capMover / capMvDX / capMvDZ.
capMoveRun = {
    private ["_i", "_p"];
    _i = 0;
    while {_i < _this} do {
        capMvX = capMvX + capMvDX;
        capMvZ = capMvZ + capMvDZ;
        capMover setPos [capMvX, capMvZ, 0];
        capSimRes = triSimFrames 1;
        _i = _i + 1;
    };
}
// "<tag>" call capTimeSamples: six GPU timing reads 25 frames apart, one log line each.
capTimeSamples = {
    private ["_i", "_g"];
    _i = 0;
    while {_i < 6} do {
        capSimRes = triSimFrames 25;
        _g = triGpuTimes true;
        format ["view=%1;event=timing;tag=%2;i=%3;gpu=%4", capView, _this, _i, _g] call capLog;
        _i = _i + 1;
    };
}