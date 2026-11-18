// Lebanon80 promo shoot: staging and camera helpers. Loaded by the shoot tests with
//   call compile preprocessFile "promo_lib.sqf"
// Positions are getPos order [east, north, height above the ground] unless a name says ASL.

psProbe = "HeliHEmpty" createVehicle [0, 0, 0];
// [x, y] call psGnd -> ground (or sea) height above sea level there
psGnd = { psProbe setPos [_this select 0, _this select 1, 0]; (getPosASL psProbe) select 2 };
// [a, b, k] call psMix -> a + k (b - a), arrays of 3
psMix = { private ["_a", "_b", "_k"]; _a = _this select 0; _b = _this select 1; _k = _this select 2; [(_a select 0) + _k * ((_b select 0) - (_a select 0)), (_a select 1) + _k * ((_b select 1) - (_a select 1)), (_a select 2) + _k * ((_b select 2) - (_a select 2))] };
// [from, to, k] call psAng -> heading from toward to by k, the short way round
psAng = { private ["_a", "_d"]; _a = _this select 0; _d = (_this select 1) - _a; while { _d > 180 } do { _d = _d - 360 }; while { _d < -180 } do { _d = _d + 360 }; _a + _d * (_this select 2) };
// [pos, heading, forward, right] call psOff -> pos moved forward/right in the heading's frame
psOff = { private ["_p", "_h", "_f", "_r"]; _p = _this select 0; _h = _this select 1; _f = _this select 2; _r = _this select 3; [(_p select 0) + _f * (sin _h) + _r * (cos _h), (_p select 1) + _f * (cos _h) - _r * (sin _h), 0] };
// [p, q] call psDir -> heading from p to q
psDir = { private ["_dx", "_dy"]; _dx = ((_this select 1) select 0) - ((_this select 0) select 0); _dy = ((_this select 1) select 1) - ((_this select 0) select 1); if ((abs _dx) + (abs _dy) < 0.001) then { 0 } else { _dx atan2 _dy } };

// [side, [classes], pos, heading, skill] call psInf -> group; units in a loose file behind pos
psInf = {
    private ["_g", "_cl", "_p", "_h", "_k", "_i", "_q"];
    _g = createGroup (_this select 0); _cl = _this select 1; _p = _this select 2; _h = _this select 3; _k = _this select 4;
    _i = 0;
    while { _i < (count _cl) } do {
        _q = [_p, _h, -2.5 * _i, 1.2 * ((_i mod 2) * 2 - 1)] call psOff;
        (_cl select _i) createUnit [_q, _g, "psU = this", _k, "PRIVATE"];
        psU setDir _h;
        _i = _i + 1;
    };
    _g
};
// [vehicle class, pos, heading] call psVeh -> the empty vehicle, placed and turned
psVeh = { private ["_v"]; _v = (_this select 0) createVehicle (_this select 1); _v setDir (_this select 2); _v setPos (_this select 1); _v };
// [veh, side, crew class, gunner (bool), commander (bool)] call psCrew -> the crew's group
psCrew = {
    private ["_v", "_g", "_c"];
    _v = _this select 0; _g = createGroup (_this select 1); _c = _this select 2;
    _c createUnit [getPos _v, _g, "psU = this", 0.7, "SERGEANT"]; psU moveInDriver _v;
    if (_this select 3) then { _c createUnit [getPos _v, _g, "psU = this", 0.7, "PRIVATE"]; psU moveInGunner _v };
    if (_this select 4) then { _c createUnit [getPos _v, _g, "psU = this", 0.7, "PRIVATE"]; psU moveInCommander _v };
    _g
};
// [veh, group, [classes]] call psCargo -> nothing; seats the classes in the cargo
psCargo = { private ["_v", "_g"]; _v = _this select 0; _g = _this select 1; { _x createUnit [getPos _v, _g, "psU = this", 0.6, "PRIVATE"]; psU moveInCargo _v } forEach (_this select 2) };
// [veh, pos, speed mode] call psDrive -> the driver is ordered to pos
psDrive = { private ["_v"]; _v = _this select 0; (group (driver _v)) setSpeedMode (_this select 2); (driver _v) doMove (_this select 1) };
// [group, pos] call psMoveGrp -> every unit of the group ordered to pos (a doMove each)
psMoveGrp = { private ["_p"]; _p = _this select 1; { _x doMove _p } forEach (units (_this select 0)) };
// group call psKill -> deletes the group's units and their vehicles
psDelGrp = { { if ((vehicle _x) != _x) then { deleteVehicle (vehicle _x) }; deleteVehicle _x } forEach (units _this) };
// pos call psWhiteFlags -> every flagpole within 3 km flies the neutral white flag (no national flags on film)
psWhiteFlags = { { _x setFlagTexture "\flags\bis_white.jpg" } forEach (nearestObjects [_this, ["FlagCarrier"], 3000]) };

// ---- the camera (stepped every frame by promo_cam.sqs while PC_RUN, or by helifly.sqs after the helicopters)
// Every per-frame step runs in World::SimulateScripts, before the frame is drawn and before the entities
// simulate, so a unit's getPos read here is the position that frame shows.
PC_RUN = false; PC_QUIT = false; PC_EXT = false; PC_TICK = false;
PC_KIND = "lerp"; PC_T0 = 0; PC_DUR = 1; PC_EASE = 1; PC_ASL = false;
PC_E0 = [0, 0, 10]; PC_E1 = [0, 0, 10]; PC_G0 = [0, 0, 0]; PC_G1 = [0, 0, 0];
PC_FOV0 = 0.7; PC_FOV1 = 0.7;
PC_C = [0, 0, 0]; PC_R0 = 100; PC_R1 = 100; PC_H0 = 50; PC_H1 = 50; PC_A0 = 0; PC_A1 = 90; PC_TZ = 0;
PC_OBJ = objNull; PC_O0 = [0, -10, 3]; PC_O1 = [0, -10, 3]; PC_TO = [0, 0, 1.5]; PC_HD = 0; PC_HDA = 0.06; PC_AP = [0, 0, 0]; PC_PA = 0.3;
PC_TG = [0, 0, 0]; PC_TA = 1;
PC_SHAKE = 0; PC_LZ = 0;
// the recorder starts with the move held on its first pose for PC_HEAD seconds (15 frames), which
// encode_promo.py cuts: the delivered clip starts on the first moving frame
PC_HEAD = 0.5;
// the engine's lens at 16:9 (UI/Settings/AspectRatio.cpp, Hor+): tan of the half view angle is
// camSetFov x 4/3 across and camSetFov x 0.75 up
PC_KX = 1.3333333; PC_KY = 0.75;
// composition: the subject (an object, or a position [x, y, metres above the ground]) and the place on
// the screen it should sit when PC_AIM (0..1 across from the left, 0..1 down from the top); a second
// subject is only logged. The aim follows a filtered subject point (alpha-beta, PC_FA / PC_FB per frame).
PC_AIM = false; PC_SX = 0.5; PC_SY = 0.5;
PC_SUBJ = objNull; PC_SUBJP = []; PC_SUBJZ = 1.5; PC_SUBJ2 = objNull; PC_SUBJP2 = []; PC_SUBJZ2 = 1.5;
PC_SS = [0, 0, 0]; PC_SV = [0, 0, 0]; PC_FA = 0.3; PC_FB = 0.05; PC_FT = -1;
PC_CS = [0, 0, 0]; PC_CV = [0, 0, 0]; PC_CT = -1;
// per-frame log (judder and composition gates): PC_LOG on while a clip records
PC_LOG = false; PC_CLIP = ""; PC_FN = 0; PC_REF = [0, 0, 0];
// anchor of a camera: an object's position (filtered) or a position
PC_COBJ = objNull;
// [obj, pos, metres up] call pcSubj -> the subject point above sea level, or [] when there is none
pcSubj = {
    private ["_o", "_p", "_r"];
    _o = _this select 0; _p = _this select 1; _r = [];
    // an object's height from getPos (metres above the surface under it) plus the ground there: getPosASL of the
    // @LoBo vehicles reads several metres above their wheels (it is right for men)
    if (!(isNull _o)) then { _r = getPos _o; _r = [_r select 0, _r select 1, ([_r select 0, _r select 1] call psGnd) + (_r select 2) + (_this select 2)] } else {
        if ((count _p) > 1) then { _r = [_p select 0, _p select 1, ([_p select 0, _p select 1] call psGnd) + (_p select 2) + (_this select 2)] };
    };
    _r
};
// [raw, smoothed, velocity, alpha, beta] call pcAB -> [smoothed, velocity] one frame on (alpha-beta filter)
pcAB = {
    private ["_r", "_s", "_v", "_a", "_b", "_p", "_e"];
    _r = _this select 0; _s = _this select 1; _v = _this select 2; _a = _this select 3; _b = _this select 4;
    _p = [(_s select 0) + (_v select 0), (_s select 1) + (_v select 1), (_s select 2) + (_v select 2)];
    _e = [(_r select 0) - (_p select 0), (_r select 1) - (_p select 1), (_r select 2) - (_p select 2)];
    [[(_p select 0) + _a * (_e select 0), (_p select 1) + _a * (_e select 1), (_p select 2) + _a * (_e select 2)],
     [(_v select 0) + _b * (_e select 0), (_v select 1) + _b * (_e select 1), (_v select 2) + _b * (_e select 2)]]
};
// raw call pcFilt -> PC_SS: the subject point filtered once a frame (reset after a gap)
pcFilt = {
    private ["_q"];
    if ((PC_FT < 0) || (time - PC_FT > 0.1) || (time < PC_FT)) then { PC_SS = +_this; PC_SV = [0, 0, 0]; PC_FT = time } else {
        if (time > PC_FT + 0.001) then { _q = [_this, PC_SS, PC_SV, PC_FA, PC_FB] call pcAB; PC_SS = _q select 0; PC_SV = _q select 1; PC_FT = time };
    };
    PC_SS
};
// raw call pcFiltC -> PC_CS: the orbit anchor filtered the same way
pcFiltC = {
    private ["_q"];
    if ((PC_CT < 0) || (time - PC_CT > 0.1) || (time < PC_CT)) then { PC_CS = +_this; PC_CV = [0, 0, 0]; PC_CT = time } else {
        if (time > PC_CT + 0.001) then { _q = [_this, PC_CS, PC_CV, PC_FA, PC_FB] call pcAB; PC_CS = _q select 0; PC_CV = _q select 1; PC_CT = time };
    };
    PC_CS
};
pcPos = { private ["_r"]; _r = _this; if (!(isNull PC_COBJ)) then { _r = (getPos PC_COBJ) call pcFiltC }; _r };
// [eye ASL, target ASL, point ASL, fov] call pcProj -> [across, down] on the screen (0..1), or [-1, -1] behind
pcProj = {
    private ["_e", "_t", "_s", "_f", "_dx", "_dy", "_dz", "_n", "_rx", "_ry", "_ux", "_uy", "_uz", "_vx", "_vy", "_vz", "_xc", "_yc", "_zc"];
    _e = _this select 0; _t = _this select 1; _s = _this select 2; _f = _this select 3;
    _dx = (_t select 0) - (_e select 0); _dy = (_t select 1) - (_e select 1); _dz = (_t select 2) - (_e select 2);
    _n = sqrt (_dx * _dx + _dy * _dy + _dz * _dz); _dx = _dx / _n; _dy = _dy / _n; _dz = _dz / _n;
    _n = sqrt (_dx * _dx + _dy * _dy); _rx = _dy / _n; _ry = -(_dx / _n);
    _ux = _ry * _dz; _uy = -(_rx * _dz); _uz = _rx * _dy - _ry * _dx;
    _vx = (_s select 0) - (_e select 0); _vy = (_s select 1) - (_e select 1); _vz = (_s select 2) - (_e select 2);
    _xc = _vx * _rx + _vy * _ry; _yc = _vx * _ux + _vy * _uy + _vz * _uz; _zc = _vx * _dx + _vy * _dy + _vz * _dz;
    if (_zc < 0.05) then { [-1, -1] } else { [0.5 + 0.5 * (_xc / _zc) / (_f * PC_KX), 0.5 - 0.5 * (_yc / _zc) / (_f * PC_KY)] }
};
// [eye ASL, subject ASL, across, down, fov] call pcAim -> a target point (ASL) that puts the subject there
pcAim = {
    private ["_e", "_s", "_f", "_vx", "_vy", "_vz", "_h", "_p", "_t", "_q"];
    _e = _this select 0; _s = _this select 1; _f = _this select 4;
    _vx = (_s select 0) - (_e select 0); _vy = (_s select 1) - (_e select 1); _vz = (_s select 2) - (_e select 2);
    _h = (_vx atan2 _vy) - (atan (((_this select 2) - 0.5) * 2 * _f * PC_KX));
    _p = (_vz atan2 (sqrt (_vx * _vx + _vy * _vy))) - (atan ((0.5 - (_this select 3)) * 2 * _f * PC_KY));
    _t = [(_e select 0) + 100 * (sin _h) * (cos _p), (_e select 1) + 100 * (cos _h) * (cos _p), (_e select 2) + 100 * (sin _p)];
    // one correction: the angles above are exact only for a level lens
    _q = [_e, _t, _s, _f] call pcProj;
    if ((_q select 0) >= 0) then {
        _h = _h + (atan (((_q select 0) - (_this select 2)) * 2 * _f * PC_KX));
        _p = _p - (atan (((_q select 1) - (_this select 3)) * 2 * _f * PC_KY));
        _t = [(_e select 0) + 100 * (sin _h) * (cos _p), (_e select 1) + 100 * (cos _h) * (cos _p), (_e select 2) + 100 * (sin _p)];
    };
    _t
};
pcStep = {
    private ["_t", "_s", "_e", "_g", "_f", "_c", "_a", "_r", "_h", "_d", "_hd", "_tg", "_eA", "_gA", "_sr", "_s2", "_q", "_q2", "_pz", "_hz", "_rel"];
    _t = (time - PC_T0) / PC_DUR;
    if (_t < 0) then { _t = 0 };
    if (_t > 1) then { _t = 1 };
    _s = _t;
    if (PC_EASE == 1) then { _s = _t * _t * (3 - 2 * _t) };
    if (PC_EASE == 2) then { _s = 1 - (1 - _t) * (1 - _t) };
    _f = PC_FOV0 + (PC_FOV1 - PC_FOV0) * _s;
    _e = [0, 0, 10]; _g = [0, 0, 0];
    if (PC_KIND == "lerp") then {
        _e = [PC_E0, PC_E1, _s] call psMix;
        _g = [PC_G0, PC_G1, _s] call psMix;
        if (PC_ASL) then {
            _e set [2, (_e select 2) - ([_e select 0, _e select 1] call psGnd)];
            _g set [2, (_g select 2) - ([_g select 0, _g select 1] call psGnd)];
        };
    };
    if (PC_KIND == "orbit") then {
        _c = PC_C call pcPos;
        _a = PC_A0 + (PC_A1 - PC_A0) * _s; _r = PC_R0 + (PC_R1 - PC_R0) * _s; _h = PC_H0 + (PC_H1 - PC_H0) * _s;
        _e = [(_c select 0) + _r * (sin _a), (_c select 1) + _r * (cos _a), _h];
        _g = [_c select 0, _c select 1, PC_TZ];
        if (PC_ASL) then {
            _e set [2, (_e select 2) - ([_e select 0, _e select 1] call psGnd)];
            _g set [2, (_g select 2) - ([_g select 0, _g select 1] call psGnd)];
        };
    };
    if (PC_KIND == "follow") then {
        if (!(isNull PC_OBJ)) then {
            PC_AP = [PC_AP, getPos PC_OBJ, PC_PA] call psMix;
            PC_HD = [PC_HD, getDir PC_OBJ, PC_HDA] call psAng;
        };
        _c = PC_AP; _hd = PC_HD;
        _d = [PC_O0, PC_O1, _s] call psMix;
        _e = [(_c select 0) + (_d select 0) * (cos _hd) + (_d select 1) * (sin _hd), (_c select 1) - (_d select 0) * (sin _hd) + (_d select 1) * (cos _hd), _d select 2];
        _g = [(_c select 0) + (PC_TO select 0) * (cos _hd) + (PC_TO select 1) * (sin _hd), (_c select 1) - (PC_TO select 0) * (sin _hd) + (PC_TO select 1) * (cos _hd), PC_TO select 2];
    };
    if (PC_KIND == "track") then {
        _e = [PC_E0, PC_E1, _s] call psMix;
        if (PC_ASL) then { _e set [2, (_e select 2) - ([_e select 0, _e select 1] call psGnd)] };
        if (!(isNull PC_OBJ)) then {
            _c = getPos PC_OBJ;
            _tg = [(_c select 0) + (PC_TO select 0), (_c select 1) + (PC_TO select 1), PC_TO select 2];
            PC_TG = [PC_TG, _tg, PC_TA] call psMix;
        };
        _g = PC_TG;
    };
    if (PC_KIND == "dz") then {
        // dolly zoom: the eye backs away from the fighter's head along the line from the beast's chest through
        // it, the lens narrows so fov x distance stays PC_DZK (the fighter keeps his size and place in frame)
        private ["_dd", "_ea", "_ta"];
        _dd = PC_D0 + (PC_D1 - PC_D0) * _s;
        _ea = [(PC_H select 0) - (PC_A select 0) * _dd + (PC_SIDE select 0) * PC_L, (PC_H select 1) - (PC_A select 1) * _dd + (PC_SIDE select 1) * PC_L, (PC_H select 2) - (PC_A select 2) * _dd + PC_LZ];
        _ta = [(_ea select 0) + (PC_A select 0) * 100, (_ea select 1) + (PC_A select 1) * 100, (_ea select 2) + (PC_A select 2) * 100];
        _e = [_ea select 0, _ea select 1, (_ea select 2) - ([_ea select 0, _ea select 1] call psGnd)];
        _g = [_ta select 0, _ta select 1, (_ta select 2) - ([_ta select 0, _ta select 1] call psGnd)];
        _f = PC_DZK / _dd;
    };
    // everything above is metres above the ground; the composition maths works above sea level
    _eA = [_e select 0, _e select 1, (_e select 2) + ([_e select 0, _e select 1] call psGnd)];
    _gA = [_g select 0, _g select 1, (_g select 2) + ([_g select 0, _g select 1] call psGnd)];
    _sr = [PC_SUBJ, PC_SUBJP, PC_SUBJZ] call pcSubj;
    if ((count _sr) > 0) then { _sr call pcFilt };
    // the eye first, then its real height above sea level read back from the camera: camSetPos heights are
    // over the surface under the eye (roads and village ground included), which the probe in psGnd can miss
    // by metres, and an aim computed from a wrong eye height puts the subject off its line
    PC_CAM camSetPos _e;
    PC_CAM camSetFov _f;
    PC_CAM camCommit 0;
    _eA = getPosASL PC_CAM;
    if (PC_AIM && ((count _sr) > 0)) then {
        _gA = [_eA, PC_SS, PC_SX, PC_SY, _f] call pcAim;
        PC_CAM camSetDir (((_gA select 0) - (_eA select 0)) atan2 ((_gA select 1) - (_eA select 1)));
        PC_CAM camSetDive (((_gA select 2) - (_eA select 2)) atan2 (sqrt (((_gA select 0) - (_eA select 0)) ^ 2 + ((_gA select 1) - (_eA select 1)) ^ 2)));
    } else {
        PC_CAM camSetTarget _g;
    };
    PC_CAM camCommit 0;
    if (PC_LOG && PC_TICK) then {
        _q = [-2, -2]; _q2 = [-2, -2];
        if ((count _sr) > 0) then { _q = [_eA, _gA, _sr, _f] call pcProj };
        _s2 = [PC_SUBJ2, PC_SUBJP2, PC_SUBJZ2] call pcSubj;
        if ((count _s2) > 0) then { _q2 = [_eA, _gA, _s2, _f] call pcProj };
        _pz = ((_gA select 2) - (_eA select 2)) atan2 (sqrt (((_gA select 0) - (_eA select 0)) ^ 2 + ((_gA select 1) - (_eA select 1)) ^ 2));
        _hz = 0.5 + 0.5 * (tan _pz) / (_f * PC_KY);
        _rel = [0, 0, 0];
        if ((count _sr) > 0) then { _rel = [(_sr select 0) - (PC_REF select 0), (_sr select 1) - (PC_REF select 1), (_sr select 2) - (PC_REF select 2)] };
        logInfo format ["PCF|%1|%2|%3|%4|%5,%6,%7|%8,%9,%10|%11,%12|%13,%14|%15|%16|%17,%18,%19|%20", PC_CLIP, PC_FN, time, triFrameCount,
            (_eA select 0) - (PC_REF select 0), (_eA select 1) - (PC_REF select 1), (_eA select 2) - (PC_REF select 2),
            (_gA select 0) - (PC_REF select 0), (_gA select 1) - (PC_REF select 1), (_gA select 2) - (PC_REF select 2),
            _q select 0, _q select 1, _q2 select 0, _q2 select 1, _hz, kjShakeGain, _rel select 0, _rel select 1, _rel select 2, _f];
        PC_FN = PC_FN + 1;
    };
};
// the per-frame step (promo_cam.sqs, helifly.sqs): counted and logged
pcTick = { PC_TICK = true; [] call pcStep; PC_TICK = false };
// start a move of dur seconds after the recorder's head (the shoot test arms the recorder in the same statement)
pcGo = { PC_DUR = _this; PC_T0 = time + PC_HEAD; PC_RUN = true; [] call pcStep };
// hold the camera on the move's first pose (PC_T0 in the future); the subject filters restart
pcHold = { PC_T0 = time + 100000; PC_RUN = true; PC_FT = -1; PC_CT = -1; [] call pcStep };
// "name" call pcLogOn -> per-frame log for this clip, positions relative to the eye where it starts
pcLogOn = { PC_CLIP = _this; PC_FN = 0; PC_REF = [0, 0, 0]; PC_REF = getPosASL PC_CAM; PC_LOG = true };

// ---- roads
// [path, k] call psPt -> point k of a road path (clamped), height 0
psPt = { private ["_p", "_k", "_q"]; _p = _this select 0; _k = _this select 1; if (_k < 0) then { _k = 0 }; if (_k > (count _p) - 1) then { _k = (count _p) - 1 }; _q = _p select _k; [_q select 0, _q select 1, 0] };
// [path, k] call psPd -> heading of the path at point k
psPd = { private ["_p", "_k"]; _p = _this select 0; _k = _this select 1; if (_k > (count _p) - 2) then { _k = (count _p) - 2 }; if (_k < 0) then { _k = 0 }; [_p select _k, _p select (_k + 1)] call psDir };
// [path, metres] call psAlong -> index of the first point at least that far along the path
psAlong = { private ["_p", "_m", "_d", "_i"]; _p = _this select 0; _m = _this select 1; _d = 0; _i = 0; while { (_i < (count _p) - 1) && (_d < _m) } do { _d = _d + (((_p select _i) select 0) - ((_p select (_i + 1)) select 0)) * 0 + sqrt ((((_p select _i) select 0) - ((_p select (_i + 1)) select 0)) ^ 2 + (((_p select _i) select 1) - ((_p select (_i + 1)) select 1)) ^ 2); _i = _i + 1 }; _i };
// [path, k, side metres, height] call psBeside -> a point beside the path at k (positive = right of travel)
psBeside = { private ["_q", "_h"]; _q = [_this select 0, _this select 1] call psPt; _h = [_this select 0, _this select 1] call psPd; _q = [_q, _h, 0, _this select 2] call psOff; [_q select 0, _q select 1, _this select 3] };
// [path, k, ahead metres, side metres, height] call psNear -> a point ahead of and beside the path at k
psNear = { private ["_q", "_h"]; _q = [_this select 0, _this select 1] call psPt; _h = [_this select 0, _this select 1] call psPd; _q = [_q, _h, _this select 2, _this select 3] call psOff; [_q select 0, _q select 1, _this select 4] };

// "Class" call psHas -> the class exists in CfgVehicles; [classes] call psPick -> the first that exists, or ""
psHas = { (triAssertConfigClass (format ["CfgVehicles/%1", _this])) == "OK" };
psPick = { private ["_r"]; _r = ""; { if (_r == "") then { if (_x call psHas) then { _r = _x } } } forEach _this; _r };
// [a, b] call psDist2 -> horizontal distance between two positions
psDist2 = { sqrt ((((_this select 0) select 0) - ((_this select 1) select 0)) ^ 2 + (((_this select 0) select 1) - ((_this select 1) select 1)) ^ 2) };
// [class, pos, heading] call psProp -> a prop (static) placed and turned; objNull when the class is missing
psProp = { private ["_o"]; _o = objNull; if ((_this select 0) != "") then { _o = (_this select 0) createVehicle (_this select 1); _o setDir (_this select 2); _o setPos (_this select 1) }; _o };
// [path, step metres] call psDense -> the path with points at most step metres apart (straight between its points)
psDense = {
    private ["_p", "_st", "_o", "_i", "_a", "_b", "_d", "_n", "_j"];
    _p = _this select 0; _st = _this select 1; _o = [];
    _i = 0;
    while { _i < (count _p) - 1 } do {
        _a = _p select _i; _b = _p select (_i + 1); _d = [_a, _b] call psDist2; _n = 1 + ((_d / _st) - ((_d / _st) mod 1));
        _j = 0;
        while { _j < _n } do { _o set [count _o, [(_a select 0) + ((_b select 0) - (_a select 0)) * _j / _n, (_a select 1) + ((_b select 1) - (_a select 1)) * _j / _n, 0]]; _j = _j + 1 };
        _i = _i + 1;
    };
    _o set [count _o, _p select ((count _p) - 1)];
    _o
};
// [centre, radius, min, max] call psRoadPair -> [a, b, path]: the road point nearest the centre, one min..max m from it, the road path between
psRoadPair = {
    private ["_c", "_rd", "_a", "_b", "_bd", "_d", "_p"];
    _c = _this select 0; _rd = _c nearestRoads (_this select 1);
    _a = _rd select 0; { if (([_x, _c] call psDist2) < ([_a, _c] call psDist2)) then { _a = _x } } forEach _rd;
    _b = _a; _bd = 0;
    { _d = [_x, _a] call psDist2; if ((_d > (_this select 2)) && (_d < (_this select 3)) && (_d > _bd)) then { _b = _x; _bd = _d } } forEach _rd;
    _p = gmRoadPath [_a, _b];
    if ((count _p) < 2) then { _p = [_a, _b] };
    [_a, _b, [_p, 5] call psDense]
};
// [from, to] call psRoute -> the road path between two points (straight line when none)
psRoute = { private ["_p"]; _p = gmRoadPath [_this select 0, _this select 1]; if ((count _p) < 2) then { _p = [_this select 0, _this select 1] }; logInfo format ["PROMO route %1 -> %2: %3 road points", _this select 0, _this select 1, count _p]; [_p, 5] call psDense };
// [veh, group, crew class, gunner (bool), commander (bool)] call psCrewG -> seats a crew of that class into an existing group
psCrewG = {
    private ["_v", "_g", "_c"];
    _v = _this select 0; _g = _this select 1; _c = _this select 2;
    _c createUnit [getPos _v, _g, "psU = this", 0.7, "CORPORAL"]; psU moveInDriver _v;
    if (_this select 3) then { _c createUnit [getPos _v, _g, "psU = this", 0.7, "PRIVATE"]; psU moveInGunner _v };
    if (_this select 4) then { _c createUnit [getPos _v, _g, "psU = this", 0.7, "PRIVATE"]; psU moveInCommander _v };
};
// [unit, pos, heading, stance] call psPost -> the unit stands its post: placed, turned, anchored ("UP", "MIDDLE", "DOWN")
psPost = { private ["_u"]; _u = _this select 0; _u setPos (_this select 1); _u setDir (_this select 2); _u setUnitPos (_this select 3); _u disableAI "MOVE"; _u stop true };
// [pos, radius] call psFree -> true when no object stands within radius of pos (camera spots, props)
psFree = { (count (nearestObjects [[(_this select 0) select 0, (_this select 0) select 1, 0], [], _this select 1])) == 0 };
// [pos, radius] call psFreeNear -> pos, or the first spot on rings of 3, 6, 9 m around it that is free within radius
psFreeNear = {
    private ["_p", "_r", "_best", "_k", "_a", "_q"];
    _p = _this select 0; _r = _this select 1; _best = _p;
    if (!([_p, _r] call psFree)) then {
        _best = [];
        { _k = _x; { if ((count _best) == 0) then { _q = [(_p select 0) + _k * (sin _x), (_p select 1) + _k * (cos _x), _p select 2]; if ([_q, _r] call psFree) then { _best = _q } } } forEach [0, 45, 90, 135, 180, 225, 270, 315] } forEach [3, 6, 9];
        if ((count _best) == 0) then { _best = _p };
    };
    _best
};
// [group, dest, heading of travel, spacing, lateral] call psFile -> every unit gets its own doMove: unit i to
// dest moved back i x spacing along the heading and alternately lateral metres left/right (a group order alone
// moves only the leader on this build, scout 2)
psFile = {
    private ["_g", "_d", "_h", "_sp", "_lt", "_i"];
    _g = _this select 0; _d = _this select 1; _h = _this select 2; _sp = _this select 3; _lt = _this select 4; _i = 0;
    { _x doMove ([_d, _h, -_sp * _i, _lt * ((_i mod 2) * 2 - 1)] call psOff); _i = _i + 1 } forEach (units _g);
};
// [veh, dest, speed mode, behaviour] call psGo -> the vehicle's own crew group drives it to dest
psGo = { private ["_v"]; _v = _this select 0; (group (driver _v)) setBehaviour (_this select 3); (group (driver _v)) setSpeedMode (_this select 2); (driver _v) doMove (_this select 1) };
// [class, pos, heading, side, crew class, gunner, commander] call psVehC -> a crewed vehicle in its own group
psVehC = { private ["_v"]; _v = [_this select 0, _this select 1, _this select 2] call psVeh; [_v, _this select 3, _this select 4, _this select 5, _this select 6] call psCrew; _v };

// ---- scripted helicopters (helifly.sqs steps them every frame, then the camera, when PC_EXT)
// HF_LIST rows: [heli, "line", [right, fwd, z] start, [right, fwd, z] end, seconds, start time]  (his frame)
//               [heli, "wline", [x, y, z] start, [x, y, z] end, seconds, start time]             (world frame)
//               [heli, "orbit", radius, z, start bearing, degrees a second, start time]       (world frame round him)
//               [heli, "escort", [right, fwd, z], bob metres, bob degrees a second]          (his frame)
// z is metres above the ground under the heli. HF_T0 is the clock; HF_KH his smoothed heading.
HF_ON = false; HF_QUIT = false; HF_HOLD = false; HF_LIST = []; HF_PREV = []; HF_T0 = 0; HF_KH = 0; HF_KS = [0, 0, 0]; HF_KV = [0, 0, 0]; HF_KT = -1;
hfStep = {
    private ["_t", "_k", "_kh", "_i", "_h", "_kind", "_s", "_o", "_a", "_p", "_q", "_v", "_d"];
    _t = time - HF_T0; if (HF_HOLD) then { _t = 0 };
    // his position filtered once a frame (alpha-beta, like the camera subject): his root moves with his stride
    _k = [0, 0, 0]; if (!(isNil "kaiju")) then { _k = getPos kaiju };
    if ((HF_KT < 0) || (time - HF_KT > 0.1) || (time < HF_KT)) then { HF_KS = +_k; HF_KV = [0, 0, 0]; HF_KT = time } else {
        if (time > HF_KT + 0.001) then { _q = [_k, HF_KS, HF_KV, 0.25, 0.035] call pcAB; HF_KS = _q select 0; HF_KV = _q select 1; HF_KT = time };
    };
    _k = HF_KS;
    if (!(isNil "kaiju")) then { HF_KH = [HF_KH, getDir kaiju, 0.05] call psAng }; _kh = HF_KH;
    _i = 0;
    {
        _h = _x select 0; _kind = _x select 1; _o = [0, 0, 30]; _d = _kh;
        if (_kind == "line") then { _s = (_t - (_x select 5)) / (_x select 4); if (_s < 0) then { _s = 0 }; if (_s > 1) then { _s = 1 }; _o = [_x select 2, _x select 3, _s] call psMix };
        if (_kind == "wline") then { _s = (_t - (_x select 5)) / (_x select 4); if (_s < 0) then { _s = 0 }; if (_s > 1) then { _s = 1 } };
        if (_kind == "escort") then { _o = +(_x select 2); _o set [2, (_o select 2) + (_x select 3) * (sin ((_x select 4) * _t))] };
        if (_kind == "orbit") then {
            _a = (_x select 4) + (_x select 5) * (_t - (_x select 6));
            _p = [(_k select 0) + (_x select 2) * (sin _a), (_k select 1) + (_x select 2) * (cos _a), _x select 3];
            _d = _a + 90; if ((_x select 5) < 0) then { _d = _a - 90 };
        } else { if (_kind == "wline") then { _p = [_x select 2, _x select 3, _s] call psMix } else {
            _p = [(_k select 0) + (_o select 0) * (cos _kh) + (_o select 1) * (sin _kh), (_k select 1) - (_o select 0) * (sin _kh) + (_o select 1) * (cos _kh), _o select 2];
        } };
        _q = HF_PREV select _i;
        _v = [((_p select 0) - (_q select 0)) * 30, ((_p select 1) - (_q select 1)) * 30, 0];
        if (((_kind == "line") || (_kind == "wline")) && ((abs (_v select 0)) + (abs (_v select 1)) > 2)) then { _d = (_v select 0) atan2 (_v select 1) };
        _h setPos _p; _h setDir _d; _h setVelocity _v; _h setDammage 0; _h setFuel 1;
        HF_PREV set [_i, _p];
        _i = _i + 1;
    } forEach HF_LIST;
};
// [class, pos] call hfHeli -> a crewed helicopter, engine on, undamageable, AI pilot kept from flying it off
hfHeli = { private ["_v"]; _v = [_this select 0, _this select 1, 90] call psVeh; _v setPos (_this select 1); [_v, west, "LoBoGolaniWB", true, false] call psCrew; _v engineOn true; _v allowDammage false; { _x allowDammage false } forEach (crew _v); (group (driver _v)) setBehaviour "CARELESS"; _v flyInHeight 40; _v };
// heli list call hfStart -> starts stepping these helis now (HF_PREV primed so the first velocity is zero)
hfStart = { HF_LIST = _this; HF_PREV = []; { HF_PREV set [count HF_PREV, getPos (_x select 0)] } forEach HF_LIST; HF_T0 = time; if (!(isNil "kaiju")) then { HF_KH = getDir kaiju }; HF_ON = true; [] call hfStep };

// [fighter, head height, chest height on him, d0, d1, fov0, lateral] call dzSetup -> PC_* for a dolly zoom ("dz")
dzSetup = {
    private ["_f", "_hp", "_cp", "_dx", "_dy", "_dz", "_n"];
    _f = _this select 0;
    _hp = getPosASL _f; _hp = [_hp select 0, _hp select 1, (_hp select 2) + (_this select 1)];
    _cp = getPosASL kaiju; _cp = [_cp select 0, _cp select 1, (_cp select 2) + (_this select 2)];
    _dx = (_cp select 0) - (_hp select 0); _dy = (_cp select 1) - (_hp select 1); _dz = (_cp select 2) - (_hp select 2);
    _n = sqrt (_dx * _dx + _dy * _dy + _dz * _dz);
    PC_H = _hp; PC_A = [_dx / _n, _dy / _n, _dz / _n];
    _n = sqrt (_dx * _dx + _dy * _dy); PC_SIDE = [_dy / _n, -(_dx / _n)];
    PC_D0 = _this select 3; PC_D1 = _this select 4; PC_DZK = (_this select 5) * (_this select 3); PC_L = _this select 6;
    PC_KIND = "dz"; PC_FOV0 = _this select 5; PC_FOV1 = PC_FOV0;
};

// ---- walking patrols. A doMove splits the unit into a new subgroup at the default NORMAL speed (AIGroupCmd:
// new AISubgroup), so it jogs whatever its group's speed mode; setSpeedMode reaches only a group's main
// subgroup. One group per man and a group "move" keeps the order on the main subgroup, where LIMITED caps
// the leader at 0.22 of his top speed (AISubgroup::UpdateFormationCoef): a walk.
// [side, [classes], pos, heading, skill] call psInfSolo -> [groups] (one man each), in a loose file behind pos
psInfSolo = {
    private ["_cl", "_p", "_h", "_k", "_i", "_q", "_g", "_gs"];
    _cl = _this select 1; _p = _this select 2; _h = _this select 3; _k = _this select 4; _gs = [];
    _i = 0;
    while { _i < (count _cl) } do {
        _g = createGroup (_this select 0);
        _q = [_p, _h, -2.5 * _i, 1.2 * ((_i mod 2) * 2 - 1)] call psOff;
        (_cl select _i) createUnit [_q, _g, "psU = this", _k, "PRIVATE"];
        psU setDir _h;
        _gs set [count _gs, _g];
        _i = _i + 1;
    };
    _gs
};
// [groups, dest, heading of travel, spacing, lateral, speed mode, behaviour] call psWalk -> each one-man group
// is ordered (group move) to dest moved back i x spacing along the heading, alternately lateral m left/right
psWalk = {
    private ["_gs", "_d", "_h", "_sp", "_lt", "_i", "_g"];
    _gs = _this select 0; _d = _this select 1; _h = _this select 2; _sp = _this select 3; _lt = _this select 4; _i = 0;
    {
        _g = _x;
        _g setBehaviour (_this select 6); _g setSpeedMode (_this select 5);
        _g move ([_d, _h, -_sp * _i, _lt * ((_i mod 2) * 2 - 1)] call psOff);
        _i = _i + 1;
    } forEach _gs;
};
// groups call psUnits -> the men of one-man groups, in order
psUnits = { private ["_r"]; _r = []; { _r = _r + (units _x) } forEach _this; _r };
// groups call psDelGrps
psDelGrps = { { _x call psDelGrp } forEach _this };

// ---- walking: the kaiju engine's direct steer (kjSteer [unit, pos, gait, arrive radius, clip rate], Game/Kaiju)
// walks any man in a straight line at the gait's clip speed, past the planner and the formation. A doMove
// order makes these men jog (2.8 m/s) whatever the speed mode; "walk" at clip rate 0.65 is about 1.6 m/s.
PS_WALKRATE = 0.65;
// [group, dest, heading of travel, spacing, lateral, rate] call psSteer -> each man walks a straight line to
// his slot behind dest (i x spacing back along the heading, alternately lateral m left and right)
psSteer = {
    private ["_d", "_h", "_sp", "_lt", "_r", "_i"];
    _d = _this select 1; _h = _this select 2; _sp = _this select 3; _lt = _this select 4; _r = _this select 5; _i = 0;
    { _x enableAI "MOVE"; _x stop false; kjSteer [_x, ([_d, _h, -_sp * _i, _lt * ((_i mod 2) * 2 - 1)] call psOff), "walk", 1.5, _r]; _i = _i + 1 } forEach (units (_this select 0));
};
// [side, [classes], start, heading, dest, spacing, lateral, behaviour, rate] call psPatrol -> the group, its men
// in a file behind start, every one walking (psSteer) to his slot behind dest
psPatrol = {
    private ["_g", "_cl", "_p", "_h", "_sp", "_lt", "_i", "_q"];
    _g = createGroup (_this select 0); _cl = _this select 1; _p = _this select 2; _h = _this select 3; _sp = _this select 5; _lt = _this select 6;
    _i = 0;
    while { _i < (count _cl) } do {
        _q = [_p, _h, -_sp * _i, _lt * ((_i mod 2) * 2 - 1)] call psOff;
        (_cl select _i) createUnit [_q, _g, "psU = this", 0.7, "PRIVATE"];
        psU setPos _q; psU setDir _h;
        _i = _i + 1;
    };
    _g setBehaviour (_this select 7); _g setSpeedMode "LIMITED";
    [_g, _this select 4, _h, _sp, _lt, _this select 8] call psSteer;
    _g
};
// group call psSteerStop -> the men stop where they are
psSteerStop = { { kjSteerStop _x } forEach (units _this) };

// [fighter, head height, chest height on him, d0, d1, fov0, fighter across, fighter down, his chest across at
// the end, his chest down at the end] call dzSetup2 -> PC_* for a dolly zoom ("dz") composed on thirds. With
// fov x distance constant, any point at the fighter's depth keeps its place on the screen, so the line the eye
// backs along passes beside and below the fighter's head by the offsets that put him on (across, down); its
// direction is aimed past Dedushka's chest so that the chest lands on its own third when the lens is at its
// narrowest (it starts near the middle and moves out to its line as he grows).
dzSetup2 = {
    private ["_f", "_hp", "_cp", "_ux", "_uy", "_uz", "_n", "_rx", "_ry", "_vx", "_vy", "_vz", "_k", "_lf", "_vf", "_p0", "_fe", "_dk", "_lk", "_vk", "_ax", "_ay", "_az"];
    _f = _this select 0;
    _hp = getPosASL _f; _hp = [_hp select 0, _hp select 1, (_hp select 2) + (_this select 1)];
    _cp = getPosASL kaiju; _cp = [_cp select 0, _cp select 1, (_cp select 2) + (_this select 2)];
    _ux = (_cp select 0) - (_hp select 0); _uy = (_cp select 1) - (_hp select 1); _uz = (_cp select 2) - (_hp select 2);
    _n = sqrt (_ux * _ux + _uy * _uy + _uz * _uz); _ux = _ux / _n; _uy = _uy / _n; _uz = _uz / _n;
    _n = sqrt (_ux * _ux + _uy * _uy); _rx = _uy / _n; _ry = -(_ux / _n);
    _vx = _ry * _uz; _vy = -(_rx * _uz); _vz = _rx * _uy - _ry * _ux;
    _k = (_this select 5) * (_this select 3);
    _lf = ((_this select 6) - 0.5) * 2 * _k * PC_KX;
    _vf = (0.5 - (_this select 7)) * 2 * _k * PC_KY;
    _p0 = [(_hp select 0) - _lf * _rx - _vf * _vx, (_hp select 1) - _lf * _ry - _vf * _vy, (_hp select 2) - _vf * _vz];
    _fe = _k / (_this select 4);
    _dk = sqrt (((_cp select 0) - (_p0 select 0)) ^ 2 + ((_cp select 1) - (_p0 select 1)) ^ 2 + ((_cp select 2) - (_p0 select 2)) ^ 2);
    _lk = ((_this select 8) - 0.5) * 2 * _fe * PC_KX * _dk;
    _vk = (0.5 - (_this select 9)) * 2 * _fe * PC_KY * _dk;
    _ax = (_cp select 0) - _lk * _rx - _vk * _vx - (_p0 select 0);
    _ay = (_cp select 1) - _lk * _ry - _vk * _vy - (_p0 select 1);
    _az = (_cp select 2) - _vk * _vz - (_p0 select 2);
    _n = sqrt (_ax * _ax + _ay * _ay + _az * _az);
    PC_H = _p0; PC_A = [_ax / _n, _ay / _n, _az / _n]; PC_SIDE = [0, 0]; PC_L = 0; PC_LZ = 0;
    PC_D0 = _this select 3; PC_D1 = _this select 4; PC_DZK = _k;
    PC_KIND = "dz"; PC_FOV0 = _this select 5; PC_FOV1 = PC_FOV0;
};

// ---- roadblocks
// pos call psRoadHeading -> [heading of the road at pos (0..180), the nearest road-segment centre]: from the two
// road-segment centres nearest pos (nearestRoads, the road net's own segments), so a roadblock is laid out from
// the road itself
psRoadHeading = {
    private ["_p", "_rd", "_a", "_b", "_da", "_db", "_d", "_h"];
    _p = _this; _rd = _p nearestRoads 40; _a = []; _b = []; _da = 1e9; _db = 1e9;
    { _d = [_x, _p] call psDist2; if (_d < _da) then { _b = _a; _db = _da; _a = _x; _da = _d } else { if (_d < _db) then { _b = _x; _db = _d } } } forEach _rd;
    _h = 0;
    if (((count _a) > 1) && ((count _b) > 1)) then { _h = [_a, _b] call psDir };
    while { _h < 0 } do { _h = _h + 180 };
    while { _h >= 180 } do { _h = _h - 180 };
    logInfo format ["PROMO road heading at %1: %2 (segments %3 and %4)", _p, _h, _a, _b];
    [_h, _a]
};
// [pos, heading, forward, right, height] call psOffZ -> psOff with a height (camera spots in a roadblock's frame)
psOffZ = { private ["_q"]; _q = [_this select 0, _this select 1, _this select 2, _this select 3] call psOff; [_q select 0, _q select 1, _this select 4] };
