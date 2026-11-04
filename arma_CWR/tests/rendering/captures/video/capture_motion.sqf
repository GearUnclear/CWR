// ============================================================================
//  Motion helpers for the side-by-side feature videos (INTEGRATION-PLAN.md
//  section 7.2). #included after ../capture_lib.sqf by the tests that
//  gen_video_tests.py writes; the same harness rules apply (globals `cap*`,
//  every FAIL-capable verb inside a helper, triScreenshot top level with a
//  literal label, so the generator spells out every frame).
//
//  A clip is a camera path over capN frames (capFps frames per second):
//
//    capPathKind = "line":  capPathA and capPathB are [east, north, eye, heading,
//                           pitch]; the camera eases (smoothstep) from A to B,
//                           the heading the short way round.
//    capPathKind = "orbit": capPathA is [centre east, centre north, radius, eye,
//                           start angle, pitch] and capPathB is [end angle]; the
//                           camera stands at the bearing `angle` from the centre
//                           and looks at it.
//
//  capEyeAbs (capture_lib) keeps the eye at a fixed height above sea level.
//  Moving props are teleported (setPos, setDir) along their own key path, and
//  the clock follows the frame: the sim clock steps 1/capFps s per frame, and
//  a time-lapse (capLapse > 0) also moves the date by capLapse game seconds per
//  frame (setDate to the clip's date, then skipTime).
// ============================================================================

capFps = 30
capPathKind = "line"
capPathA = [0, 0, 1.75, 0, 0]
capPathB = [0, 0, 1.75, 0, 0]
capN = 1
capLapse = 0
capMovers = []
// The clip's staging view (capBegin's origin and heading), saved by
// capStageSave: props, movers and shells are placed relative to it while the
// camera moves.
capStageX = 0
capStageZ = 0
capStageH = 0

capStageSave = {
    capStageX = capOX;
    capStageZ = capOZ;
    capStageH = capH;
}

// [forward, right] call capStageRel -> [east, north] relative to the staging view.
capStageRel = {
    [capStageX + (_this select 0) * (sin capStageH) + (_this select 1) * (cos capStageH), capStageZ + (_this select 0) * (cos capStageH) - (_this select 1) * (sin capStageH)]
}

// t in [0, 1] call capEase -> smoothstep
capEase = { _this * _this * (3 - 2 * _this) }

// [a, b, t] call capLerpAngle -> heading from a toward b the short way round
capLerpAngle = {
    private ["_a", "_d"];
    _a = _this select 0;
    _d = (_this select 1) - _a;
    if (_d > 180) then { _d = _d - 360 };
    if (_d < -180) then { _d = _d + 360 };
    _a + _d * (_this select 2)
}

// [k, n] call capPathAt: put the camera on frame k of n.
capPathAt = {
    private ["_k", "_n", "_t", "_s", "_ang"];
    _k = _this select 0;
    _n = _this select 1;
    _t = 0;
    if (_n > 1) then { _t = _k / (_n - 1) };
    _s = _t call capEase;
    if (capPathKind == "orbit") then {
        _ang = (capPathA select 4) + ((capPathB select 0) - (capPathA select 4)) * _s;
        capOX = (capPathA select 0) + (capPathA select 2) * (sin _ang);
        capOZ = (capPathA select 1) + (capPathA select 2) * (cos _ang);
        capEye = capPathA select 3;
        capH = _ang + 180;
        capP = capPathA select 5;
    } else {
        capOX = (capPathA select 0) + ((capPathB select 0) - (capPathA select 0)) * _s;
        capOZ = (capPathA select 1) + ((capPathB select 1) - (capPathA select 1)) * _s;
        capEye = (capPathA select 2) + ((capPathB select 2) - (capPathA select 2)) * _s;
        capH = [capPathA select 3, capPathB select 3, _s] call capLerpAngle;
        capP = (capPathA select 4) + ((capPathB select 4) - (capPathA select 4)) * _s;
    };
    [] call capPinView;
}

// [object, [[k, forward, right, dir], ...]] call capMoverAdd
// Registers a moving prop: at frame k of each key it stands at [forward,
// right] from the view origin with heading `dir` relative to the view heading
// (capRel, as capSpawn places props), linearly in between and held before the
// first and after the last key. Keys are taken relative to the view at the
// time of the call (the clip's staging view).
capMoverAdd = {
    private ["_o", "_keys", "_p"];
    _o = _this select 0;
    if (not (isNull _o)) then {
        _keys = [];
        {
            _p = [_x select 1, _x select 2] call capStageRel;
            _keys = _keys + [[_x select 0, _p select 0, _p select 1, capStageH + (_x select 3)]];
        } forEach (_this select 1);
        capMovers = capMovers + [[_o, _keys]];
        format ["view=%1;event=mover;object=%2;keys=%3", capView, _o, _keys] call capLog;
    };
}

// [k] call capPropsAt: teleport every mover to frame k.
capPropsAt = {
    private ["_k", "_keys", "_a", "_b", "_t", "_i"];
    _k = _this select 0;
    {
        _keys = _x select 1;
        _a = _keys select 0;
        _b = _a;
        _i = 1;
        while { _i < (count _keys) } do {
            if (_k >= ((_keys select (_i - 1)) select 0)) then {
                _a = _keys select (_i - 1);
                _b = _keys select _i;
            };
            _i = _i + 1;
        };
        _t = 0;
        if ((_b select 0) > (_a select 0)) then { _t = (_k - (_a select 0)) / ((_b select 0) - (_a select 0)) };
        if (_t < 0) then { _t = 0 };
        if (_t > 1) then { _t = 1 };
        (_x select 0) setDir ((_a select 3) + ((_b select 3) - (_a select 3)) * _t);
        (_x select 0) setPos [(_a select 1) + ((_b select 1) - (_a select 1)) * _t, (_a select 2) + ((_b select 2) - (_a select 2)) * _t, 0];
    } forEach capMovers;
}

// [[forward, right], ammo class] call capBoom: an explosion at that point
// relative to the staging view (the combat clip's shells).
capBoom = {
    private ["_p", "_o"];
    _p = [(_this select 0) select 0, (_this select 0) select 1] call capStageRel;
    _o = (_this select 1) createVehicle [_p select 0, _p select 1, 0.3];
    format ["view=%1;event=boom;ammo=%2;pos=%3;object=%4", capView, _this select 1, _p, _o] call capLog;
}

// [k] call capClockAt: the sim clock (water, fire, the pinned clouds stay put)
// and, for a time-lapse, the date of frame k.
capClockAt = {
    private ["_k"];
    _k = _this select 0;
    capSimRes = triSetSimTime (capSimT + _k / capFps);
    if (capLapse > 0) then {
        setDate capDate;
        skipTime (_k * capLapse / 3600);
    };
}

// ["<label>", k] call capFrameLog: one line per frame with the switches and
// gates (triFeatureState) and, every 10th frame, the GPU sections.
capFrameLog = {
    private ["_k", "_gpu"];
    _k = _this select 1;
    _gpu = "skip";
    if ((_k mod 10) == 0) then { _gpu = triGpuTimes true };
    format ["view=%1;event=frame;label=%2;k=%3;mode=%4;feat=%5;gpu=%6", capView, _this select 0, _k, capModeName, triFeatureState, _gpu] call capLog;
}

// "<setting>" call capModeQuick: P mode's switch between the two shots of a
// pair, without the load wait or the timer reset of capMode.
capModeQuick = {
    capModeName = _this;
    capLit = triSetLighting capModeName;
}
