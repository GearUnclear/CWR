// ============================================================================
//  Shared rig for the Classic | Remastered lighting captures.
//  #included by every *.test.sqf in this folder (Trident expands #include
//  relative to the including file before it splits statements).
//
//  HARNESS RULES THIS FILE IS BUILT AROUND (engine/Trident/src/scenarios/
//  integration.rs, run_sqf_test):
//   * Every top-level statement is a separate eval, so state shared between
//     statements lives in globals (prefix `cap`). `_locals` only live inside
//     one { } block.
//   * A top-level statement whose result starts with "FAIL:" is retried until
//     assert_timeout and then FAILS THE WHOLE TEST. Every tri verb that can
//     answer FAIL (triSetLighting, triCheatWeather, triSetView,
//     triAssertConfigClass, ...) is therefore only ever called inside a
//     helper block below, its answer stored and logged, never returned.
//   * triScreenshot must be a top-level statement with a string-literal label:
//     the runner keeps its own sequence counter from the literal and waits for
//     <seq>_<label>.png. A triScreenshot inside a block would desync it. That
//     is why each test spells out its screenshot lines by hand.
//
//  LOG LINES. Every helper reports through `capLog`, which writes
//  "UDCAP;key=value;..." at INFO through logInfo (LOG_INFO Script). Trident
//  captures the game's stdout into the test's output folder
//  (game_stdout.log), and run-captures.ps1 / make_pairs.py parse these lines
//  into timings.csv / timings.json / timings.md. One `event=shot` line is
//  written per screenshot; `gpu=` is always the LAST field because the
//  triGpuTimes summary itself contains '|' and ':'.
//
//  COORDINATES. View origins are [east, north] map metres. triSetView takes
//  raw engine order [east, UP, north, dirE, dirUp, dirN]; the eye height comes
//  from a ground probe (an invisible HeliHEmpty set to [x, z, 0] and read back
//  with getPosASL), so no terrain height is hard-coded anywhere.
//  Headings use the setDir/getDir convention: 0 = north, clockwise.
// ============================================================================

capLog = { logInfo (format ["UDCAP;%1", _this]) }

// "Class" call capHas -> true when CfgVehicles >> Class exists in the merged
// config (mods included). triAssertConfigClass answers "OK" or "FAIL:...";
// the comparison turns that into a bool, so nothing FAIL-shaped escapes.
capHas = { (triAssertConfigClass (format ["CfgVehicles/%1", _this])) == "OK" }

// ["A", "B", ...] call capPick -> the first candidate class that exists, or "".
capPick = {
    private ["_r"];
    _r = "";
    { if (_r == "") then { if (_x call capHas) then { _r = _x } } } forEach _this;
    _r
}

// [x, z] call capGround -> terrain (or road / bridge) height ASL at that point.
capGround = {
    capProbe setPos [_this select 0, _this select 1, 0];
    (getPosASL capProbe) select 2
}

// Re-apply date, weather, view distance and (for matched views) the sim clock.
// setDate moves the sun at once (World::SetDate recalculates LightSun);
// triCheatWeather is the instant overcast cheat and also zeroes fog, so fog is
// set after it. Re-pinning Glob.time (triSetSimTime) puts water waves, sky and
// the remastered cloud-shadow drift (EngineGL33_Lighting.cpp reads Glob.time)
// back where the Classic shot had them.
capPinEnv = {
    setDate capDate;
    capWx = triCheatWeather capOvercast;
    0 setOvercast capOvercast;
    0 setRain 0;
    0 setFog capFog;
    setViewDistance capVD;
    if (capPinTime) then { capSimRes = triSetSimTime capSimT };
}

// Put the render camera on the view. The cutscene camera (capCam) is only
// there to hide the gameplay HUD (World::Draw gates the HUD on !_cameraEffect)
// and to give camSetFov a target; triSetView then overrides its transform with
// the exact eye point and direction.
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
    capViewRes = triSetView [capOX, capEyeASL, capOZ, _dx, _dy, _dz];
}

// Everything a matched pair must share, applied right before each shot.
capPin = {
    [] call capPinEnv;
    { (_x select 0) setDir (_x select 3); (_x select 0) setPos [_x select 1, _x select 2, 0] } forEach capProps;
    if (capUseView) then { [] call capPinView };
}

// [id, x, z, eye, heading, pitch, [y, m, d, h, min], overcast, fog, viewDist] call capBegin
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

// [forward m, right m] call capRel -> [east, north] of a point relative to the
// view origin and heading.
capRel = {
    [capOX + (_this select 0) * (sin capH) + (_this select 1) * (cos capH), capOZ + (_this select 0) * (cos capH) - (_this select 1) * (sin capH)]
}

// [[candidate classes], forward m, right m, dir relative to the view heading, kind (, group)] call capSpawn
// kind: "veh" (createVehicle, locked), "man" (a posed figure: captive, MOVE /
// TARGET / AUTOTARGET off, standing) or "fighter" (captive, MOVE off only, so
// the combat AI still aims and fires when it is given a doFire order).
// Men join the optional group (default capGrpW). Places the first existing
// candidate, registers it for re-pinning, and returns it (objNull when nothing
// could be spawned - the view still shoots, it is just missing that prop).
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

// Remove every registered prop (units and vehicles alike).
capClear = {
    { deleteVehicle (_x select 0) } forEach capProps;
    capProps = [];
}

// "classic" / "remastered" call capMode
// Switches the lighting model (triSetLighting also turns the shadow maps on for
// Remastered and off for Classic, as the Options row does), then restarts the
// GPU timer and the CPU frame profiler so their averages cover this mode only.
// triGpuTimes false -> true resets GpuTimings (EngineGL33::SetGpuTimingEnabled).
capMode = {
    capModeName = _this;
    capLit = triSetLighting _this;
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3", capView, _this, capLit] call capLog;
}

// Read the timers after the settle frames. GPU results trail the frame by a
// few frames and are an exponential average (weight 0.1 per frame), so 90+
// frames on one fixed view leave well under 1% of any earlier state in them.
capReadTimers = {
    capGpu = triGpuTimes true;
    capPerf = triPerfStats 0;
}

// "<label>" call capRecord - one line per screenshot.
capRecord = {
    format ["view=%1;event=shot;label=%2;mode=%3;lit=%4;cam=%5;fps=%6;perf=%7;gpu=%8", capView, _this, capModeName, capLit, triCamView, triFps, capPerf, capGpu] call capLog;
}

// ---- research track "tiling": k call capVar ----
// Remastered variant k through the saturation knob (1.12 + 0.001 k; the
// override shader decodes k and uses 1.12). Restarts the timers like capMode.
capVar = {
    capModeName = format ["v%1", _this];
    capLit = triSetLighting (format ["saturation=%1", 1.12 + 0.001 * _this]);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3", capView, capModeName, capLit] call capLog;
}
