// ============================================================================
//  VANILLA variant of the UD capture rig (look-ref reference set).
//  Same helpers, globals and camera rig as the fork's
//  arma_CWR/tests/rendering/captures/capture_lib.sqf (graphics-remaster
//  fec1075 + look base.patch), using only verbs the vanilla upstream 3.05
//  engine has. The vanilla engine code is untouched; only this script differs.
//
//  Left out (fork-only verbs, an unknown verb fails the whole script):
//    triPinObjectDetail  object detail stays adaptive (Scene::AdjustComplexity)
//    triSetLighting      vanilla has one lighting model
//    triSettleScene      replaced by a fixed triSimFrames wait (capSettleFrames)
//    triGpuTimes, triSceneStats, triFeatureState, triCamView: not logged
//    triPinCloudDrift    vanilla has no cloud-shadow field; sky dome clouds move
//    triCaptureAtPresent vanilla triScreenshot reads mid-frame, BEFORE the
//                        gamma pass; the reference collector applies the
//                        engine's gamma (pow(c, 1/1.2), the default) offline
//  Added:
//    skipTime 0.01 between two setDate calls in capPinEnv: vanilla setDate
//    does not switch the auto street lamps (they wait for their 12 s tick);
//    skipTime over 20 s calls Landscape::OnTimeSkipped, which switches them
//    now, as the fork's setDate does. The second setDate pins the date back.
// ============================================================================

capLog = { logInfo (format ["UDCAP;%1", _this]) }

// Frames each mode waits before its timers and shot (the fork waits until
// the draw count is steady, at most 300). Vanilla object detail is adaptive
// and refines only 5 s after its last coarsening, so the wait is longer.
capDetail = 0
capSettleFrames = 450

capRain = 0
capEyeAbs = false

capHas = { (triAssertConfigClass (format ["CfgVehicles/%1", _this])) == "OK" }

capPick = {
    private ["_r"];
    _r = "";
    { if (_r == "") then { if (_x call capHas) then { _r = _x } } } forEach _this;
    _r
}

capGround = {
    capProbe setPos [_this select 0, _this select 1, 0];
    (getPosASL capProbe) select 2
}

capPinEnv = {
    setDate capDate;
    skipTime 0.01;
    setDate capDate;
    capWx = triCheatWeather capOvercast;
    0 setOvercast capOvercast;
    0 setRain capRain;
    0 setFog capFog;
    setViewDistance capVD;
    if (capPinTime) then { capSimRes = triSetSimTime capSimT };
}

capPinView = {
    private ["_g", "_dx", "_dy", "_dz"];
    _g = [capOX, capOZ] call capGround;
    _dx = (sin capH) * (cos capP);
    _dy = sin capP;
    _dz = (cos capH) * (cos capP);
    capCam camSetPos [capOX, capOZ, capEye];
    capCam camSetTarget [capOX + 1000 * _dx, capOZ + 1000 * _dz, capEye + 1000 * _dy];
    capCam camSetFov capFov;
    capCam camCommit 0;
    capEyeASL = _g + capEye;
    if (capEyeAbs) then { capEyeASL = capEye };
    capViewRes = triSetView [capOX, capEyeASL, capOZ, _dx, _dy, _dz];
}

capPin = {
    [] call capPinEnv;
    { (_x select 0) setDir (_x select 3); (_x select 0) setPos [_x select 1, _x select 2, 0] } forEach capProps;
    if (capUseView) then { [] call capPinView };
}

capBegin = {
    capView = _this select 0;
    capOX = _this select 1;
    capOZ = _this select 2;
    capEye = _this select 3;
    capH = _this select 4;
    capP = _this select 5;
    capDate = _this select 6;
    capOvercast = _this select 7;
    capFog = _this select 8;
    capVD = _this select 9;
    capProps = [];
    capUseView = true;
    capPinTime = true;
    capSimT = time;
    [] call capPin;
    format ["view=%1;event=begin;world=%2;origin=%3;eye=%4;heading=%5;pitch=%6;date=%7;overcast=%8;fog=%9", capView, capWorld, [capOX, capOZ], capEye, capH, capP, capDate, capOvercast, capFog] call capLog;
    format ["view=%1;event=view;vd=%2;fov=%3;eyeASL=%4;setView=%5;weather=%6", capView, capVD, capFov, capEyeASL, capViewRes, capWx] call capLog;
}

capRel = {
    [capOX + (_this select 0) * (sin capH) + (_this select 1) * (cos capH), capOZ + (_this select 0) * (cos capH) - (_this select 1) * (sin capH)]
}

capSpawn = {
    private ["_cls", "_p", "_px", "_pz", "_d", "_o", "_kind", "_grp"];
    _cls = (_this select 0) call capPick;
    _kind = _this select 4;
    _grp = capGrpW;
    if ((count _this) > 5) then { _grp = _this select 5 };
    _o = objNull;
    if (_cls == "") then {
        format ["view=%1;event=skip_spawn;want=%2", capView, _this select 0] call capLog;
    } else {
        _p = [_this select 1, _this select 2] call capRel;
        _px = _p select 0;
        _pz = _p select 1;
        _d = capH + (_this select 3);
        if (_kind != "veh") then {
            capNewUnit = objNull;
            _cls createUnit [[_px, _pz, 0], _grp, "capNewUnit = this", 0.6, "PRIVATE"];
            _o = capNewUnit;
            if (not (isNull _o)) then {
                _o setCaptive true;
                _o allowDamage false;
                _o disableAI "MOVE";
                if (_kind == "man") then {
                    _o disableAI "TARGET";
                    _o disableAI "AUTOTARGET";
                    _o setBehaviour "SAFE";
                } else {
                    _o setBehaviour "COMBAT";
                    _o setCombatMode "RED";
                };
                _o setUnitPos "UP";
            };
        } else {
            _o = _cls createVehicle [_px, _pz, 0];
            if (not (isNull _o)) then { _o lock true; _o allowDamage false };
        };
        if (isNull _o) then {
            format ["view=%1;event=spawn_failed;class=%2", capView, _cls] call capLog;
        } else {
            _o setDir _d;
            _o setPos [_px, _pz, 0];
            capProps = capProps + [[_o, _px, _pz, _d]];
            format ["view=%1;event=spawn;class=%2;pos=%3;dir=%4", capView, _cls, [_px, _pz], _d] call capLog;
        };
    };
    _o
}

capClear = {
    { deleteVehicle (_x select 0) } forEach capProps;
    capProps = [];
}

// "<mode>" call capMode: vanilla has one lighting model, so the mode is only
// a label. Waits capSettleFrames frames (the stand-in for triSettleScene),
// then restarts the CPU frame profiler.
capMode = {
    capModeName = _this;
    capLit = "vanilla";
    capSettleRes = triSimFrames capSettleFrames;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=vanilla;detail=adaptive;settle=%3", capView, capModeName, capSettleRes] call capLog;
}

capReadTimers = {
    capGpu = "n/a";
    capPerf = triPerfStats 0;
}

capRecord = {
    format ["view=%1;event=shot;label=%2;mode=%3;lit=%4;fps=%5;perf=%6", capView, _this, capModeName, capLit, triFps, capPerf] call capLog;
}
