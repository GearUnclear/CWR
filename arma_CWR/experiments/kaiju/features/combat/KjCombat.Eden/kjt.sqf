// ============================================================================
//  kjt.sqf  -  helpers for the combat tests (plain script, no test verbs).
//  Loaded by every combat test:  call compile preprocessFile "kjt.sqf"
//
//  The site: the Everon airfield. The runway runs north to south: flat (24.7 m
//  above the sea) from 350 m south of kjtAt to 900 m and more north of it, and
//  about 100 m to each side; the sea is 500 m to the west. He stands at kjtAt
//  facing north, up the runway; shooters are put north of him, in front of
//  the furnace.
// ============================================================================
kjtAt = [4800, 11260, 0];
// the combat mount carries the beam feature's fullbright sprite (kjbeam.pbo): sparks and the death flare use it
KJCB_GLOW = "\kjbeam\kjbeam_glow";
kjtDed = objNull;
kjtProbe = "HeliHEmpty" createVehicle [4800, 11520, 0];
kjtShots = [];        // [[ammo class, rounds fired, time of the first, class of the shooter], ...] one row per ammo and shooter class
kjtShotCount = 0;
kjtCrewAll = [];
kjtCrewAsked = 0;
kjtPending = [];
kjtMenAll = [];
kjtVehAll = [];
kjtGroups = [];

// [class, dir] call kjtSpawn : the creature, in a Resistance group, standing where the tests want him
kjtSpawn = {
    kjtG = createGroup resistance;
    kjtDed = objNull;
    (_this select 0) createUnit [kjtAt, kjtG, "kjtDed = this", 0.5, "PRIVATE"];
    kjtDir = _this select 1;
    true
};

// call kjtHold : he stands still and upright whatever the AI would like (the tests have no brain)
kjtHold = {
    kjtDed disableAI "MOVE";
    kjtDed disableAI "TARGET";
    kjtDed disableAI "AUTOTARGET";
    kjtG setBehaviour "CARELESS";
    kjtDed allowFleeing 0;
    kjtDed setUnitPos "UP";
    kjtDed stop true;
    kjtDed setDir kjtDir;
    kjtDed setPos kjtAt;
    // he keeps looking up the runway: left to itself the AI turns him to watch whatever flies round him
    kjtDed doWatch [(kjtAt select 0) + 3000 * sin kjtDir, (kjtAt select 1) + 3000 * cos kjtDir, 30];
    true
};

// [forward, right] call kjtRel : world [x, y, 0] of a point in his frame
kjtRel = {
    private ["_p", "_d"];
    _p = getPos kjtDed;
    _d = getDir kjtDed;
    [(_p select 0) + (_this select 0) * sin _d + (_this select 1) * cos _d, (_p select 1) + (_this select 0) * cos _d - (_this select 1) * sin _d, 0]
};

// [x, y] call kjtGround : ground height above the sea there
kjtGround = {
    kjtProbe setPos [_this select 0, _this select 1, 0];
    (getPosASL kjtProbe) select 2
};

// [bearing from him, distance, eye height above the local ground, aim height above his feet, aim point this far ahead of him (optional)] call kjtView : a triSetView array
// The bearing and the distance are from the aim point. His heading is the one he was made with (a dead body's own heading is not trusted).
kjtView = {
    private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g", "_f", "_ax", "_az"];
    _b = _this select 0;
    _d = _this select 1;
    _e = _this select 2;
    _a = _this select 3;
    _f = 0;
    if ((count _this) > 4) then { _f = _this select 4 };
    _p = getPosASL kjtDed;
    _ax = (kjtAt select 0) + _f * sin kjtDir;
    _az = (kjtAt select 1) + _f * cos kjtDir;
    _cx = _ax + _d * sin _b;
    _cz = _az + _d * cos _b;
    _g = [_cx, _cz] call kjtGround;
    [_cx, _g + _e, _cz, _ax - _cx, ((_p select 2) + _a) - (_g + _e), _az - _cz]
};

// "Fired" on a shooter: [unit, weapon, muzzle, mode, ammo]
kjtNote = {
    private ["_am", "_row", "_cl"];
    _am = _this select 4;
    _cl = typeOf (_this select 0);
    kjtShotCount = kjtShotCount + 1;
    _row = [];
    { if (((_x select 0) == _am) and ((_x select 3) == _cl)) then { _row = _x } } forEach kjtShots;
    if ((count _row) == 0) then { _row = [_am, 0, time, _cl]; kjtShots = kjtShots + [_row] };
    _row set [1, (_row select 1) + 1];
    true
};

// "ammo class" call kjtFiredOf : rounds of that ammo fired so far
kjtFiredOf = {
    private ["_n"];
    _n = 0;
    { if ((_x select 0) == _this) then { _n = _n + (_x select 1) } } forEach kjtShots;
    _n
};

// "ammo class" call kjtHitsOf : [hits he felt of that ammo, points]
kjtHitsOf = {
    private ["_r"];
    _r = [0, 0];
    { if ((_x select 0) == _this) then { _r = [_x select 1, _x select 2] } } forEach KJ_combatByAmmo;
    _r
};

// side call kjtGroup : a new group of that side, remembered
kjtGroup = {
    private ["_g"];
    _g = createGroup _this;
    kjtGroups = kjtGroups + [_g];
    _g
};

// [class, [x, y], dir, group, crew class, ["driver", "gunner", "commander"]] call kjtVehicle : the vehicle. Its crew gets in at the next kjtMount.
kjtVehicle = {
    private ["_v", "_p"];
    _p = _this select 1;
    _v = (_this select 0) createVehicle [_p select 0, _p select 1, 0];
    _v setDir (_this select 2);
    _v setPos [_p select 0, _p select 1, 0];
    kjtPending = kjtPending + [[_v, kjtCrewAsked, _this select 5]];
    kjtCrewAsked = kjtCrewAsked + (count (_this select 5));
    { (_this select 4) createUnit [[(_p select 0) + 7, (_p select 1) + 7, 0], _this select 3, "kjtCrewAll = kjtCrewAll + [this]", 0.85, "PRIVATE"] } forEach (_this select 5);
    kjtVehAll = kjtVehAll + [_v];
    _v addEventHandler ["Fired", { _this call kjtNote }];
    _v
};

// call kjtMount : every crew made since the last call gets in (a few frames after kjtVehicle)
kjtMount = {
    private ["_v", "_s", "_i", "_u"];
    {
        _v = _x select 0;
        _s = _x select 1;
        _i = 0;
        {
            _u = kjtCrewAll select (_s + _i);
            if (_x == "driver") then { _u moveInDriver _v };
            if (_x == "gunner") then { _u moveInGunner _v };
            if (_x == "commander") then { _u moveInCommander _v };
            _i = _i + 1
        } forEach (_x select 2)
    } forEach kjtPending;
    kjtPending = [];
    true
};

// [[classes], [x, y], group] call kjtMen : soldiers on foot, each with a "Fired" note
kjtMen = {
    private ["_p", "_i"];
    _p = _this select 1;
    _i = 0;
    {
        _x createUnit [[(_p select 0) + 4 * _i, _p select 1, 0], _this select 2, "kjtMenAll = kjtMenAll + [this]; this addEventHandler [""Fired"", { _this call kjtNote }]", 0.8, "PRIVATE"];
        _i = _i + 1
    } forEach (_this select 0);
    true
};

// call kjtCease : every group made here holds its fire and forgets him as a target
kjtCease = {
    { _x setCombatMode "BLUE" } forEach kjtGroups;
    true
};

// call kjtBook : one line of the state of the fight, for the log
kjtBook = {
    format ["t %1 health %2 stage %3 hurtCount %4 stats %5 zones %6 shots %7 byAmmo %8 dmg %9 alive %10 pos %11 dir %12", time, KJ_woundHealth, KJ_woundStage, KJ_hurtCount, KJ_combatStats, KJ_combatByZone, kjtShots, KJ_combatByAmmo, getDammage kjtDed, alive kjtDed, getPos kjtDed, getDir kjtDed]
};

// ---- checks. A tri assert is retried until it passes or the test's assert_timeout runs out, with the
// game running on meanwhile: a failed one holds the machine's one game slot for minutes, and a check
// on a moving value can pass on a later try. So every check is taken once, where it stands, into a
// list, and the test ends with one verdict line that run-combat.ps1 reads.
kjtFails = [];
kjtChecks = 0;

// [condition, "what it means"] call kjtCheck
kjtCheck = {
    kjtChecks = kjtChecks + 1;
    if (!(_this select 0)) then {
        kjtFails = kjtFails + [_this select 1];
        logInfo format ["KJT CHECK FAILED at %1: %2", time, _this select 1]
    };
    true
};

// "test name" call kjtVerdict : the one line the runner reads
kjtVerdict = {
    if ((count kjtFails) == 0) then {
        logInfo format ["KJT VERDICT PASS %1 (%2 checks)", _this, kjtChecks]
    } else {
        logInfo format ["KJT VERDICT FAIL %1: %2 of %3 checks failed: %4", _this, count kjtFails, kjtChecks, kjtFails]
    };
    true
};

kjtReady = true;
