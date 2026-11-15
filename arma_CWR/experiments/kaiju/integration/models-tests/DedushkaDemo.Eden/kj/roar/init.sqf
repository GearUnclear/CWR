// ============================================================================
//  kj\roar\init.sqf  -  Dedushka ROAR: tuning and shared functions.
//  Loaded once by every script in this folder:
//      call compile preprocessFile "kj\roar\init.sqf"
//  Works on the stock engine binary. Needs the kj_roar addon for the sounds
//  (addon\build-kj-roar.ps1); without it the scripts still run, silently.
// ============================================================================

// ============================ TUNING (the one block) =========================
// Every size is a multiple of his height, so the real model only has to say
// how tall it is: CfgVehicles >> <class> >> kjHeight when the addon gives it,
// else KJ_height when a mission sets it, else the proxy's 40 m.
if (isNil "KJR_H") then { KJR_H = 40 };

// ---- sound ------------------------------------------------------------------
if (isNil "KJR_SOUND_V") then { KJR_SOUND_V = 343 };         // m/s: how late a sound (and the fear) arrives
if (isNil "KJR_GROUND_V") then { KJR_GROUND_V = 3000 };      // m/s: the thump through the ground, well ahead of the sound
if (isNil "KJR_FAR_SWITCH") then { KJR_FAR_SWITCH = 30 };    // past this the listener gets the far version of a sound
if (isNil "KJR_MOUTH_H") then { KJR_MOUTH_H = 0.85 };        // the roar comes from this height
if (isNil "KJR_CHEST_H") then { KJR_CHEST_H = 0.6 };         // the furnace
if (isNil "KJR_ROAR_LEN") then { KJR_ROAR_LEN = 12 };        // s: no second roar starts inside this
if (isNil "KJR_PLAY_ANIM") then { KJR_PLAY_ANIM = true };    // play DedRoar when the anim feature has it (ignored when not)

// ---- the roar: who does what -------------------------------------------------
if (isNil "KJR_BREAK_R") then { KJR_BREAK_R = 7 };           // soldiers this near break and run
if (isNil "KJR_COWER_R") then { KJR_COWER_R = 11 };          // frozen men this near also throw themselves down
if (isNil "KJR_FREEZE_R") then { KJR_FREEZE_R = 17 };        // soldiers this near stop dead and stare
if (isNil "KJR_VEH_R") then { KJR_VEH_R = 25 };              // drivers this near stand on the brakes
if (isNil "KJR_CIV_R") then { KJR_CIV_R = 32 };              // civilians this near run for cover
if (isNil "KJR_ALERT_R") then { KJR_ALERT_R = 45 };          // everyone this near turns to face the sound
if (isNil "KJR_FLEE_DIST") then { KJR_FLEE_DIST = 9 };       // how far a broken section runs
if (isNil "KJR_CIV_RUN") then { KJR_CIV_RUN = 7 };           // how far a civilian with no house to hide in runs
if (isNil "KJR_SHELTER_R") then { KJR_SHELTER_R = 4 };       // a civilian goes indoors when a house is this close
if (isNil "KJR_SHOCK") then { KJR_SHOCK = 0.03 };            // the scare, as the engine counts strength (see panic.sqs)
if (isNil "KJR_FLEE_CHECK") then { KJR_FLEE_CHECK = 6 };     // s the engine is given to start its own flee (the men run meanwhile)
if (isNil "KJR_PANIC_T") then { KJR_PANIC_T = 45 };          // s a broken section stays broken
if (isNil "KJR_CIV_PANIC_T") then { KJR_CIV_PANIC_T = 120 }; // s civilians stay hidden
if (isNil "KJR_FREEZE_T") then { KJR_FREEZE_T = 14 };        // s a frozen man stays frozen (plus up to 4)
if (isNil "KJR_HALT_T") then { KJR_HALT_T = 12 };            // s a stopped vehicle stays stopped (plus up to 4)
if (isNil "KJR_ALERT_T") then { KJR_ALERT_T = 10 };          // s the far ones keep looking his way

// ---- the footfall --------------------------------------------------------------
if (isNil "KJR_STANCE") then { KJR_STANCE = 0.125 };         // foot centre, sideways from his origin (when nobody says where the foot is)
if (isNil "KJR_FOOT_AHEAD") then { KJR_FOOT_AHEAD = 0.11 };  // and ahead of it
if (isNil "KJR_FOOT_R") then { KJR_FOOT_R = 0.15 };          // the dust ring starts at the edge of the foot
if (isNil "KJR_DUST_N") then { KJR_DUST_N = 28 };            // puffs in the ring
if (isNil "KJR_DUST_VIEW") then { KJR_DUST_VIEW = 60 };      // no dust is made when the listener is further than this
if (isNil "KJR_SHAKE_FULL") then { KJR_SHAKE_FULL = 2 };     // the view shakes at full strength inside this
if (isNil "KJR_SHAKE_R") then { KJR_SHAKE_R = 16 };          // and not at all past this
if (isNil "KJR_SHAKE_T") then { KJR_SHAKE_T = 0.9 };         // s one footfall shakes the view
if (isNil "KJR_SHAKE_POS") then { KJR_SHAKE_POS = 0.045 };   // m sideways at full strength
if (isNil "KJR_SHAKE_UP") then { KJR_SHAKE_UP = 0.06 };      // m up and down
if (isNil "KJR_SHAKE_YAW") then { KJR_SHAKE_YAW = 0.6 };     // degrees
if (isNil "KJR_SHAKE_JOLT") then { KJR_SHAKE_JOLT = 2.2 };   // m/s kick to a ground vehicle the player sits in
if (isNil "KJR_ROAR_SHAKE") then { KJR_ROAR_SHAKE = 0.55 };  // the roar shakes the view this hard at his feet
if (isNil "KJR_ROAR_SHAKE_R") then { KJR_ROAR_SHAKE_R = 12 };// and not at all past this
if (isNil "KJR_ROAR_SHAKE_T") then { KJR_ROAR_SHAKE_T = 5 }; // s
if (isNil "KJR_STOMP_SCALE") then { KJR_STOMP_SCALE = 1.6 }; // a stomp against a walking step

// ---- presence without sight ----------------------------------------------------
if (isNil "KJR_TICK") then { KJR_TICK = 1 };                 // s between presence passes
if (isNil "KJR_DRONE_R") then { KJR_DRONE_R = 10 };          // the furnace is heard inside this
if (isNil "KJR_DRONE_LEN") then { KJR_DRONE_LEN = 7.9 };     // s: the loop file is 8.0 s; restarted a hair early
if (isNil "KJR_DARK_R") then { KJR_DARK_R = 5 };             // street lamps this near him go out and stay out
if (isNil "KJR_FLICKER_R") then { KJR_FLICKER_R = 9 };       // lamps this near stutter
if (isNil "KJR_LAMP_SCAN") then { KJR_LAMP_SCAN = 3 };       // he must move this far before the lamps are looked for again
if (isNil "KJR_BIRD_R") then { KJR_BIRD_R = 22 };            // birds near the listener go up when he is this near
if (isNil "KJR_BIRD_GAP") then { KJR_BIRD_GAP = 50 };        // s before they can go up again
if (isNil "KJR_BIRD_N") then { KJR_BIRD_N = 9 };             // birds in a flock
if (isNil "KJR_STATIC_R") then { KJR_STATIC_R = 28 };        // the radio breaks up inside this
if (isNil "KJR_STATIC_NEAR") then { KJR_STATIC_NEAR = 12 };  // and badly inside this
if (isNil "KJR_STATIC_GAP") then { KJR_STATIC_GAP = 28 };    // s between bursts (plus up to 15)
// A HeliHEmpty put on the sea reads this height or less above sea level
// (the pipeline's photo test measured 2.7): below it a point counts as water.
if (isNil "KJR_SEA_ASL") then { KJR_SEA_ASL = 3.1 };
// ========================== END OF TUNING ====================================

// ---- state others may read ---------------------------------------------------
if (isNil "KJ_roarCount") then { KJ_roarCount = 0 };         // roars so far
if (isNil "KJ_roarTime") then { KJ_roarTime = -1000 };       // when the last one started
if (isNil "KJ_roarBusyUntil") then { KJ_roarBusyUntil = -1000 };
if (isNil "KJ_roaring") then { KJ_roaring = false };         // true while a roar is spreading
if (isNil "KJ_roarLog") then { KJ_roarLog = [] };            // [[unit, "BREAK"|"FREEZE"|"ALERT"|"CIV"|"VEH", metres, seconds after the roar], ...]
if (isNil "KJ_panicLog") then { KJ_panicLog = [] };          // [[leader, "SOLDIER"|"CIV", "native"|"script", time, flee point], ...]
if (isNil "KJ_sndLog") then { KJ_sndLog = [] };              // [[class, time sent, time heard, metres], ...]
if (isNil "KJ_stepCount") then { KJ_stepCount = 0 };
if (isNil "KJ_stepLog") then { KJ_stepLog = [] };            // [[count, time, foot pos, metres to listener, shake strength, puffs], ...]
if (isNil "KJ_shakeLog") then { KJ_shakeLog = [] };          // [[time started, strength, seconds, "FOOT"|"VEHICLE"|"CAMERA"], ...]
if (isNil "KJ_presenceLog") then { KJ_presenceLog = [] };    // [["DRONE"|"DARK"|"FLICKER"|"BIRDS"|"STATIC", time, number], ...]
if (isNil "KJ_darkLamps") then { KJ_darkLamps = [] };        // lamps he has put out
if (isNil "KJ_birds") then { KJ_birds = [] };                // birds in the air because of him
if (isNil "KJ_presenceOn") then { KJ_presenceOn = true };    // set false to end presence.sqs
if (isNil "KJR_emit") then { KJR_emit = [] };
if (isNil "KJR_lamps") then { KJR_lamps = [] };
if (isNil "KJR_side") then { KJR_side = 1 };

// [integration] the shared core (which engine: shake.sqs uses the engine's camera shake when KJ_native)
if (isNil "KJ_coreReady") then { call compile preprocessFile "kj\init.sqf" };

// ---- functions -----------------------------------------------------------------

// [kaiju] call KJR_fnSize : reads his height once.
KJR_fnSize = {
    private ["_h"];
    if (!(isNil "KJ_height")) then { KJR_H = KJ_height };
    _h = (typeOf (_this select 0)) GetVehicleParam "kjHeight";
    if (!(isNil "_h")) then { KJR_H = call _h };
    KJR_sized = true
};

// call KJR_fnEar : who is listening. The player (or the vehicle he sits in),
// unless a cutscene names its camera or another object in KJ_listener.
KJR_fnEar = {
    private ["_e"];
    _e = vehicle player;
    if (!(isNil "KJ_listener")) then { if (!(isNull KJ_listener)) then { _e = KJ_listener } };
    _e
};

// n call KJR_fnEmitter : the invisible object sound channel n is played from.
// A unit that speaks queues its lines (Audio/DynSound.cpp:197-209), so nothing
// is ever said by Dedushka himself: the roar would wait behind the furnace.
// Channels: 0 voice, 1 right foot, 2 left foot, 3 chest, 4 beam, 5 birds, 6 spare.
KJR_fnEmitter = {
    while { (count KJR_emit) <= _this } do { KJR_emit = KJR_emit + ["HeliHEmpty" createVehicle [0, 0, 0]] };
    KJR_emit select _this
};

// [x, y] call KJR_fnDry : false over the sea.
KJR_fnDry = {
    private ["_p"];
    _p = 6 call KJR_fnEmitter;
    _p setPos [_this select 0, _this select 1, 0];
    ((getPosASL _p) select 2) > KJR_SEA_ASL
};

// [kaiju, from pos, metres] call KJR_fnAway : a dry point that far from the
// position, straight away from him when that is land, else swung to one side.
KJR_fnAway = {
    private ["_kp", "_p", "_d", "_b", "_best", "_i", "_offs", "_c", "_done"];
    _kp = getPos (_this select 0);
    _p = _this select 1;
    _d = _this select 2;
    _b = ((_p select 0) - (_kp select 0)) atan2 ((_p select 1) - (_kp select 1));
    _offs = [0, 25, -25, 50, -50, 80, -80, 110, -110];
    _best = [(_p select 0) + _d * sin _b, (_p select 1) + _d * cos _b, 0];
    _done = false;
    _i = 0;
    while { (_i < (count _offs)) and (not _done) } do {
        _c = [(_p select 0) + _d * sin (_b + (_offs select _i)), (_p select 1) + _d * cos (_b + (_offs select _i)), 0];
        if ([_c select 0, _c select 1] call KJR_fnDry) then { _best = _c; _done = true };
        _i = _i + 1
    };
    _best
};

// [kaiju, unit, n] call KJR_fnShelter : where a civilian runs. Indoors when a
// house with rooms is close and is not a step toward him, else away.
KJR_fnShelter = {
    private ["_k", "_u", "_b", "_pos", "_bp", "_ok"];
    _k = _this select 0;
    _u = _this select 1;
    _pos = [_k, getPos _u, KJR_CIV_RUN * KJR_H] call KJR_fnAway;
    _b = nearestBuilding _u;
    _ok = false;
    if (!(isNull _b)) then { _ok = ((_b distance _u) < (KJR_SHELTER_R * KJR_H)) and ((_b distance _k) > ((_u distance _k) - 0.5 * KJR_H)) };
    if (_ok) then {
        _bp = _b buildingPos (_this select 2);
        if (((_bp select 0) == 0) and ((_bp select 1) == 0)) then { _bp = _b buildingPos 0 };
        if (((_bp select 0) == 0) and ((_bp select 1) == 0)) then { _bp = getPos _b };
        _pos = _bp
    };
    _pos
};

// [[x, y], radius, strength] call KJR_fnDustRing : the ring a foot throws out.
// [integration] an optional fourth element true makes it spray (a footfall in the sea).
// A circle of puffs at the edge of the foot, pushed outward along the ground,
// swelling and thinning; then a slower column from around the sole.
KJR_fnDustRing = {
    private ["_c", "_r", "_s", "_n", "_i", "_a", "_v", "_g", "_wet", "_gg", "_gb", "_up"];
    _c = _this select 0;
    _r = _this select 1;
    _s = _this select 2;
    _wet = false;
    if ((count _this) > 3) then { _wet = _this select 3 };
    _gg = 0.93;
    _gb = 0.8;
    _up = 0.6;
    if (_wet) then { _gg = 1.02; _gb = 1.06; _up = 3.5 };
    _n = KJR_DUST_N;
    _i = 0;
    while { _i < _n } do {
        _a = (360 / _n) * _i + random 10;
        _v = (4 + random 3) * _s;
        _g = 0.5 + random 0.1;
        if (_wet) then { _g = 0.8 + random 0.1 };
        drop ["cl_basic", "", "Billboard", 1, 6 + random 3, [(_c select 0) + _r * sin _a, (_c select 1) + _r * cos _a, 0.8], [_v * sin _a, _v * cos _a, _up + random 0.8], 0, 1.3, 1, 0.25, [3 * _s, 8 * _s, 14 * _s], [[_g, _g * _gg, _g * _gb, 0], [_g, _g * _gg, _g * _gb, 0.8], [_g, _g * _gg, _g * _gb, 0.45], [_g, _g * _gg, _g * _gb, 0]], [0], 0.6, 0.25, "", "", ""];
        _i = _i + 1
    };
    _i = 0;
    while { (_i < 8) and (!_wet) } do {
        _a = random 360;
        drop ["cl_basic", "", "Billboard", 1, 8 + random 3, [(_c select 0) + 0.6 * _r * sin _a, (_c select 1) + 0.6 * _r * cos _a, 1], [0.8 * sin _a, 0.8 * cos _a, 1.8 + random 1.4], 0, 1.28, 1, 0.1, [4 * _s, 10 * _s, 16 * _s], [[0.5, 0.46, 0.4, 0], [0.5, 0.46, 0.4, 0.6], [0.5, 0.46, 0.4, 0.3], [0.5, 0.46, 0.4, 0]], [0], 0.8, 0.3, "", "", ""];
        _i = _i + 1
    };
    _n + 8
};

// obj call KJR_fnIsLamp : true for a street lamp. The only safe test is the
// class name. lightIsOn and switchLight must never be given anything else:
// StreetLamp has no casting id of its own (World/Scene/ObjectClasses.hpp:88-118
// has no USE_CASTING), so the engine takes every entity for a lamp, lightIsOn
// answers for a soldier, and switchLight writes into him and the game dies a
// frame later (seen: access violation in EntityAI::GetHit, NOTES.md).
KJR_fnIsLamp = { (substr [typeOf _this, 0, 10]) == "StreetLamp" };

// [[x, y], radius] call KJR_fnLampScan : remembers the street lamps in a
// circle. Lamps are not AI objects, so the typed search cannot see them
// (Game/Commands/GameStateExtUi.cpp:633-640): every object in the circle is
// asked for its class.
KJR_fnLampScan = {
    private ["_o", "_n"];
    _o = nearestObjects [[(_this select 0) select 0, (_this select 0) select 1, 0], [], _this select 1];
    _n = 0;
    { if (_x call KJR_fnIsLamp) then { if (!(_x in KJR_lamps)) then { KJR_lamps = KJR_lamps + [_x]; _n = _n + 1 } } } forEach _o;
    _n
};

KJR_ready = true;
