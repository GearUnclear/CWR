// kj\beam\fns.sqf: the functions of Dedushka's breath. fire.sqs loads this once
// (call compile preprocessFile). Tuning values are the KJ_beamT_* globals set at
// the top of fire.sqs; nothing here holds a number of its own that shapes the feel.
//
// Positions here are [x, y(north), z(above sea level)] unless a name says otherwise.

// [x, y] -> ground height above sea level there (0 over the sea).
KJ_beam_fnGround = {
    private ["_h"];
    KJ_beamProbeObj setPos [_this select 0, _this select 1, 0];
    _h = ((getPosASL KJ_beamProbeObj) select 2) - KJ_beamProbeOfs;
    if (_h < 0) then { _h = 0 };
    _h
};

// kaiju -> the furnace (where the beam leaves the chest), from his position and
// facing and the offsets measured off his own model at the start.
// [integration] On the kaiju binary: the furnace point of his model in his CURRENT pose (he
// straightens for the beam, and the point goes with him).
KJ_beam_fnChest = {
    private ["_p", "_d", "_c"];
    _c = [];
    if (KJ_native) then { _c = kjPoint [_this, KJ_furnacePoint] };
    if ((count _c) != 3) then {
        _p = getPosASL _this;
        _d = getDir _this;
        _c = [(_p select 0) + KJ_beamChestFwd * sin _d, (_p select 1) + KJ_beamChestFwd * cos _d, (_p select 2) + KJ_beamChestUp]
    };
    _c
};

// [from, to] angles in degrees -> signed shortest turn, -180..180.
KJ_beam_fnTurn = {
    private ["_d"];
    _d = ((_this select 1) - (_this select 0)) mod 360;
    if (_d > 180) then { _d = _d - 360 };
    if (_d < -180) then { _d = _d + 360 };
    _d
};

// [chest, aim, length]: lays the beam pieces from the chest along the line to
// the aim point, as far as length. Pieces past the end go under the ground.
KJ_beam_fnDraw = {
    private ["_c", "_a", "_len", "_dx", "_dy", "_dz", "_l", "_ux", "_uy", "_uz", "_s", "_up", "_dir", "_i", "_n", "_m", "_seg", "_used"];
    _c = _this select 0;
    _a = _this select 1;
    _len = _this select 2;
    _dx = (_a select 0) - (_c select 0);
    _dy = (_a select 1) - (_c select 1);
    _dz = (_a select 2) - (_c select 2);
    _l = sqrt (_dx * _dx + _dy * _dy + _dz * _dz);
    if (_l < 1) then { _l = 1; _dx = 0; _dy = 1; _dz = 0 };
    if (_len <= 0) then { _len = _l + 5 };
    _ux = _dx / _l;
    _uy = _dy / _l;
    _uz = _dz / _l;
    _s = sqrt (_ux * _ux + _uy * _uy);
    if (_s < 0.01) then { _up = [0, 1, 0] } else { _up = [0 - _ux * _uz / _s, 0 - _uy * _uz / _s, _s] };
    _dir = [_ux, _uy, _uz];
    _n = count KJ_beamSegs;
    _i = 0;
    _used = 0;
    while { _i < _n } do {
        _seg = KJ_beamSegs select _i;
        if (KJ_beamSegLen * _i < _len) then {
            _m = KJ_beamSegLen * (_i + 0.5);
            _seg setPosASL [(_c select 0) + _ux * _m, (_c select 1) + _uy * _m, (_c select 2) + _uz * _m];
            _seg setVectorDirAndUp [_dir, _up];
            _used = _i + 1;
        } else {
            if (_i < KJ_beamDrawn) then { _seg setPosASL [_c select 0, _c select 1, -400] };
        };
        _i = _i + 1;
    };
    KJ_beamDrawn = _used;
    KJ_beamDir = _dir;
    KJ_beamLen = _len;
};

// Puts every beam piece out of sight.
KJ_beam_fnHide = {
    private ["_p"];
    _p = getPosASL KJ_beamOwner;
    { _x setPosASL [_p select 0, _p select 1, -400] } forEach KJ_beamSegs;
    KJ_beamDrawn = 0;
};

// [x, y, z above sea level] -> the same point as `drop` wants it. In the game
// binary the tests run on, `drop` takes the height of an array position above
// the ground, not above the sea (measured at the start of fire.sqs with a
// particle that reports where it was; KJ_beamDropAGL holds the answer).
KJ_beam_fnDropPos = {
    if (KJ_beamDropAGL) then { [_this select 0, _this select 1, (_this select 2) - ([_this select 0, _this select 1] call KJ_beam_fnGround)] } else { _this }
};

// [pos, size at birth, size at death, life, [r, g, b], alpha]: one fullbright glow sprite.
KJ_beam_fnGlow = {
    drop ["\kjbeam\kjbeam_glow", "", "Billboard", 1, _this select 3, (_this select 0) call KJ_beam_fnDropPos, [0, 0, 0], 0, 1.275, 1, 0, [_this select 1, _this select 2], [(_this select 4) + [_this select 5], (_this select 4) + [0]], [0], 1, 0, "", ""];
};

// [pos, how many, scatter, speed up, speed sideways]: sparks that rise, drift and die.
KJ_beam_fnSparks = {
    private ["_p", "_i", "_sc", "_up", "_side"];
    _p = (_this select 0) call KJ_beam_fnDropPos;
    _sc = _this select 2;
    _up = _this select 3;
    _side = _this select 4;
    _i = 0;
    while { _i < (_this select 1) } do {
        drop ["\kjbeam\kjbeam_glow", "", "Billboard", 1, 1.1 + random 1.6, [(_p select 0) + _sc - random (2 * _sc), (_p select 1) + _sc - random (2 * _sc), (_p select 2) + _sc - random (2 * _sc)], [_side - random (2 * _side), _side - random (2 * _side), _up * (0.5 + random 1)], 0, 1.5, 1, 0.04, [0.9, 0.25], [[1, 0.85, 0.5, 1], [1, 0.4, 0.05, 0.8], [0.8, 0.15, 0.02, 0]], [0], 0.3, 1.5, "", ""];
        _i = _i + 1;
    };
};

// [pos, size at birth, size at death, life, grey, alpha, speed up]: one puff of smoke or haze.
KJ_beam_fnPuff = {
    private ["_g"];
    _g = _this select 4;
    drop ["cl_basic", "", "Billboard", 1, _this select 3, (_this select 0) call KJ_beam_fnDropPos, [1 - random 2, 1 - random 2, _this select 6], 0, 1.275, 1.04, 0.06, [_this select 1, _this select 2], [[_g, _g, _g, 0], [_g, _g, _g, _this select 5], [_g, _g, _g, 0]], [0], 0.5, 0.6, "", ""];
};

// [[x, y], big]: the scripted after-fire, called only by KJ_beam_fnIgnite. A fire
// where the beam has been, unless one already burns close by or the budget is spent. Every other fire gets a smoke column that outlasts it.
KJ_beam_fnSite = {
    private ["_p", "_ok", "_f", "_s", "_cls", "_n", "_g2"];
    _p = _this select 0;
    _n = count KJ_beamSites;
    if (_n >= KJ_beamT_fireMax) exitWith {};
    _ok = true;
    _g2 = KJ_beamSiteGap * KJ_beamSiteGap;
    { if (_ok) then { if ((((_x select 1) select 0) - (_p select 0)) ^ 2 + (((_x select 1) select 1) - (_p select 1)) ^ 2 < _g2) then { _ok = false } } } forEach KJ_beamSites;
    if (!_ok) exitWith {};
    _cls = "KJBeamFlame";
    if (_this select 1) then { _cls = "KJBeamFlameBig" };
    _f = _cls createVehicle [_p select 0, _p select 1, 0];
    _f setPos [_p select 0, _p select 1, 0];
    _f inflame true;
    _s = objNull;
    if ((_n mod 2) == 0) then {
        _s = "KJBeamSmoke" createVehicle [_p select 0, _p select 1, 0];
        _s setPos [(_p select 0) + 1, (_p select 1) + 1, 0];
        _s inflame true;
    };
    KJ_beamSites = KJ_beamSites + [[_f, [_p select 0, _p select 1], time + KJ_beamT_fireTime + random 50, _s, time + KJ_beamT_smokeTime + random 50]];
    KJ_beamFires = KJ_beamFires + 1;
};

// ======================= THE ONE PLACE A FIRE IS STARTED =======================
// [[x, y], strength 0..1, why]: every ignition point of the beam goes through here
// and nowhere else. why is "trail" (the line burnt into the ground), "wreck" (a
// vehicle) or "razed" (a building or tree that fell). Each one is recorded in
// KJ_beamIgnitions as [x, y, strength, time].
//
// Today it lights the scripted after-fire (KJ_beam_fnSite: Fire-class objects from
// the kjbeam addon, with a budget and a least gap). When the native fire-spread
// simulation is in the engine, the mission sets KJ_fireSpreadNative = true and this
// hands the point to it: fireSpreadIgnite [x, y, radius] lights the fuel cells in
// that radius and returns how many it lit. The command is compiled from a string
// so a stock binary, which does not know it, never parses it. Where the native
// fire finds nothing to burn (sand, a road) the scripted fire is lit as before.
KJ_beam_fnIgnite = {
    private ["_p", "_s", "_why", "_lit", "_go"];
    _p = _this select 0;
    _s = _this select 1;
    _why = _this select 2;
    _go = true;
    if (_why == "trail") then {
        if ((((KJ_beamLastIgn select 0) - (_p select 0)) ^ 2 + ((KJ_beamLastIgn select 1) - (_p select 1)) ^ 2) < (KJ_beamT_igniteGap * KJ_beamT_igniteGap)) then { _go = false } else { KJ_beamLastIgn = [_p select 0, _p select 1] };
    };
    if (_go) then {
        KJ_beamIgnitions = KJ_beamIgnitions + [[_p select 0, _p select 1, _s, time]];
        _lit = 0;
        if (KJ_fireSpreadNative) then { _lit = call compile format ["fireSpreadIgnite [%1, %2, %3]", _p select 0, _p select 1, 2 + 6 * _s] };
        if (_lit < 1) then { [_p, _s > 0.8] call KJ_beam_fnSite };
    };
};
// ================================================================================

// [object, height of the beam above the ground there, x, y of the point on the line]:
// what the beam does to one thing.
KJ_beam_fnTouch = {
    private ["_o", "_h", "_t", "_p"];
    _o = _this select 0;
    _h = _this select 1;
    if (_o == KJ_beamOwner) exitWith {};
    _t = typeOf _o;
    if (_t in KJ_beamOwnTypes) exitWith {};
    if (("Man" countType [_o]) > 0) exitWith {
        _p = getPos _o;
        if (_h < KJ_beamT_reachMan && (((_p select 0) - (_this select 2)) ^ 2 + ((_p select 1) - (_this select 3)) ^ 2) < (KJ_beamT_manRadius * KJ_beamT_manRadius)) then {
            KJ_beamBurnt = KJ_beamBurnt + [_o];
            if (alive _o) then { [_o, KJ_beamOwner] call KJ_fnKill; KJ_beamKills = KJ_beamKills + 1 };
        };
    };
    if (("AllVehicles" countType [_o]) > 0) exitWith {
        if (_h < KJ_beamT_reachVeh) then {
            KJ_beamBurnt = KJ_beamBurnt + [_o];
            if (alive _o) then {
                [_o, KJ_beamOwner] call KJ_fnKill;
                KJ_beamWrecks = KJ_beamWrecks + 1;
                _p = getPos _o;
                [[_p select 0, _p select 1], 0.7, "wreck"] call KJ_beam_fnIgnite;
            };
        };
    };
    // A building, a tree, a wall: it stands a beat longer, then it goes.
    if (_h < KJ_beamT_reachUp) then {
        KJ_beamBurnt = KJ_beamBurnt + [_o];
        if ((damage _o) < 1) then { KJ_beamQueue = KJ_beamQueue + [[_o, time + KJ_beamT_fallDelay + random KJ_beamT_fallSpread]] };
    };
};

// [chest, unit direction, length of the drawn beam, ground too]: everything the
// line touches right now. With "ground too" it walks back from the far end for as
// long as the beam is low enough to reach things on the ground. Then it tests
// every aircraft against the line.
KJ_beam_fnBurn = {
    private ["_c", "_u", "_len", "_m", "_go", "_steps", "_px", "_py", "_pz", "_g", "_h", "_wx", "_wy", "_wz", "_t", "_d2", "_p", "_r2"];
    _c = _this select 0;
    _u = _this select 1;
    _len = _this select 2;
    _m = _len;
    _go = _this select 3;
    _steps = 0;
    while { _go } do {
        _px = (_c select 0) + (_u select 0) * _m;
        _py = (_c select 1) + (_u select 1) * _m;
        _pz = (_c select 2) + (_u select 2) * _m;
        _g = [_px, _py] call KJ_beam_fnGround;
        _h = _pz - _g;
        if (_h > KJ_beamT_reachUp || _m < 15 || _steps >= KJ_beamT_burnSteps) then {
            _go = false;
        } else {
            { if (!(_x in KJ_beamBurnt)) then { [_x, _h, _px, _py] call KJ_beam_fnTouch } } forEach (nearestObjects [[_px, _py, 3], [], KJ_beamT_burnRadius]);
        };
        _m = _m - KJ_beamT_burnStep;
        _steps = _steps + 1;
    };
    _r2 = KJ_beamT_airRadius * KJ_beamT_airRadius;
    {
        if (alive _x) then {
            _p = getPosASL _x;
            _wx = (_p select 0) - (_c select 0);
            _wy = (_p select 1) - (_c select 1);
            _wz = (_p select 2) - (_c select 2);
            _t = _wx * (_u select 0) + _wy * (_u select 1) + _wz * (_u select 2);
            if (_t > 0 && _t < _len + 20) then {
                _d2 = _wx * _wx + _wy * _wy + _wz * _wz - _t * _t;
                if (_d2 < _r2) then {
                    [_x, KJ_beamOwner] call KJ_fnKill;
                    KJ_beamAirKills = KJ_beamAirKills + 1;
                    KJ_beamLastAirKill = time;
                    [_p, 30, 4, 1.2, [1, 0.9, 0.7], 1] call KJ_beam_fnGlow;
                };
            };
        };
    } forEach KJ_beamAir;
};

// Brings down whatever is due to fall, and lights what it leaves.
KJ_beam_fnDue = {
    private ["_keep", "_o", "_p", "_big"];
    if ((count KJ_beamQueue) > 0) then {
        _keep = [];
        {
            if ((_x select 1) <= time) then {
                _o = _x select 0;
                if ((damage _o) < 1) then {
                    [_o, KJ_beamOwner] call KJ_fnKill;
                    if ((damage _o) >= 1) then {
                        KJ_beamRazed = KJ_beamRazed + 1;
                        _p = getPos _o;
                        _big = 0.6; if ((KJ_beamRazed mod 3) == 0) then { _big = 1 }; [[_p select 0, _p select 1], _big, "razed"] call KJ_beam_fnIgnite;
                    };
                };
            } else {
                _keep = _keep + [_x];
            };
        } forEach KJ_beamQueue;
        KJ_beamQueue = _keep;
    };
};

// Puts out the fires and the smoke whose time is up.
KJ_beam_fnExpire = {
    private ["_keep"];
    _keep = [];
    {
        if ((_x select 2) <= time) then { if (!(isNull (_x select 0))) then { deleteVehicle (_x select 0) } };
        if ((_x select 4) <= time) then {
            if (!(isNull (_x select 3))) then { deleteVehicle (_x select 3) };
            if (!(isNull (_x select 0))) then { deleteVehicle (_x select 0) };
        } else {
            _keep = _keep + [_x];
        };
    } forEach KJ_beamSites;
    KJ_beamSites = _keep;
};

// [chest, level 0..4]: which furnace light burns (0 none, 1 ember .. 4 white).
KJ_beam_fnLight = {
    private ["_i", "_c", "_l"];
    _c = _this select 0;
    _i = 0;
    while { _i < (count KJ_beamLights) } do {
        _l = KJ_beamLights select _i;
        _l setPosASL _c;
        _l inflame ((_i + 1) == (_this select 1));
        _i = _i + 1;
    };
    KJ_beamLightLevel = _this select 1;
};

KJ_beamFnsLoaded = true;
