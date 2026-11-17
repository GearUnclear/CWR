// ============================================================================
//  kj\crush\init.sqf  -  Dedushka CRUSH: tuning and shared functions.
//  Loaded once by loop.sqs, stomp.sqs, swat.sqs and hurt.sqs:
//      call compile preprocessFile "kj\crush\init.sqf"
//  Works on the stock engine binary. INTEGRATED set: on the kaiju binary (KJ_native, kj\init.sqf)
//  the footfalls are the engine's own events, kills are credited to him, men and hulls are
//  thrown through the physics and hits come with the shooter; every one of those keeps its
//  stock fallback. Changes against the crush feature's own file are marked [integration].
// ============================================================================

// ============================ TUNING (the one block) =========================
// Every size is a fraction of his height, so a taller or shorter final model
// only needs the height. The height comes from the unit's class when the addon
// gives it (CfgVehicles >> <class> >> kjHeight); else from KJ_height when a
// mission sets it; else the proxy's 40 m.
if (isNil "KJC_H") then { KJC_H = 40 };

// ---- the walk --------------------------------------------------------------
if (isNil "KJC_TICK") then { KJC_TICK = 0.25 };              // s between passes while he moves
if (isNil "KJC_IDLE_TICK") then { KJC_IDLE_TICK = 0.5 };     // s between passes while he stands
if (isNil "KJC_HALF_STRIDE") then { KJC_HALF_STRIDE = 0.22 };// ground covered between two footfalls
if (isNil "KJC_FIRST_FOOT") then { KJC_FIRST_FOOT = 1 };     // the foot that lands first after the walk clip starts: 1 right, -1 left
if (isNil "KJC_START_LAG") then { KJC_START_LAG = 0.03 };    // setting off from a stand, the legs are this much ground ahead of his body (the blend-in)
if (isNil "KJC_FOOT_AHEAD") then { KJC_FOOT_AHEAD = 0.11 };  // a foot lands this far ahead of his origin
if (isNil "KJC_STANCE") then { KJC_STANCE = 0.125 };         // foot centre, sideways from his origin
if (isNil "KJC_FOOT_R") then { KJC_FOOT_R = 0.15 };          // under a foot: dead, flattened
if (isNil "KJC_NEAR_R") then { KJC_NEAR_R = 0.32 };          // near a footfall: thrown down, hurt
if (isNil "KJC_LEG_HALF") then { KJC_LEG_HALF = 0.19 };      // outer edge of the legs from his centre line
if (isNil "KJC_BESIDE") then { KJC_BESIDE = 0.05 };          // extra room beside the feet
if (isNil "KJC_LEAD_TIME") then { KJC_LEAD_TIME = 2.8 };      // s of walking: a building that far beyond his toes starts to go (the dust beat plus the collapse)
if (isNil "KJC_LEAD_MIN") then { KJC_LEAD_MIN = 0.06 };      // and never less than this beyond his toes
if (isNil "KJC_LEAD_SMALL") then { KJC_LEAD_SMALL = 0.26 };  // a tree, fence or pole this far ahead goes over
if (isNil "KJC_BACK") then { KJC_BACK = 0.12 };              // and this far behind his origin (the heel)
if (isNil "KJC_TOE") then { KJC_TOE = 0.17 };                // while he stands: his toes reach this far ahead
if (isNil "KJC_BIG_R") then { KJC_BIG_R = 0.18 };            // guessed half-size of a building (the script cannot measure one)
if (isNil "KJC_BIG_MASS") then { KJC_BIG_MASS = 10000 };     // kg: a static at or above this collapses like a building, below it it topples like a tree (the engine's own split, Object.cpp:1532)
if (isNil "KJC_MIN_MASS") then { KJC_MIN_MASS = 20 };        // kg: a static lighter than this is left alone (roads 0, bushes 9, helpers 0)
if (isNil "KJC_DUST_BEAT") then { KJC_DUST_BEAT = 0.55 };    // s from the first dust to the collapse
if (isNil "KJC_NEAR_HURT") then { KJC_NEAR_HURT = 0.45 };    // damage to a man at the edge of a foot, falling to 0 at KJC_NEAR_R
if (isNil "KJC_TOE_LEN") then { KJC_TOE_LEN = 0.15 };        // foot centre to the tip of his toes
if (isNil "KJC_HULL") then { KJC_HULL = 0.20 };              // a vehicle in a foot lane is dealt with when the toes are this close to its centre (half a hull, and room for the pass to be late: at 0.14 a hull still shoved him back 2 m)
if (isNil "KJC_GAP") then { KJC_GAP = 0.02 };                // a vehicle this close to his centre line passes between his legs
if (isNil "KJC_MAN_LANE") then { KJC_MAN_LANE = 0.13 };       // a man this close to the middle of a foot lane is under the foot ([integration] was 0.08: a man 4 m off the middle still stood in the way of the foot and shoved him)
if (isNil "KJC_MAN_LEAD") then { KJC_MAN_LEAD = 0.09 };       // and dies when the toes are this close to him (time to fall before the foot is on him)
if (isNil "KJC_FOOT_HALF") then { KJC_FOOT_HALF = 0.07 };    // a vehicle this close to the middle of a foot lane is flattened; further out it is kicked aside
if (isNil "KJC_SWING_V") then { KJC_SWING_V = 14 };          // m/s sideways given to a wreck the swinging foot knocks aside
if (isNil "KJC_WRECKS") then { KJC_WRECKS = [["JeepWreck1", 0.8], ["UralWreck", 1.5], ["M113Wreck", 1.1]] }; // what a flattened car, truck and tracked hull become, and how many metres each is pressed into the ground
if (isNil "KJC_TRUCK_MASS") then { KJC_TRUCK_MASS = 2500 };  // kg: a light vehicle at or above this is a truck
if (isNil "KJC_SINK") then { KJC_SINK = 1.1 };               // m a hull is pressed in when the data has no wreck class for it
if (isNil "KJC_STAND_R") then { KJC_STAND_R = 0.16 };        // [integration] while he stands or turns: a man or a hull this close to one of his feet is cleared away (it would shove him)
if (isNil "KJC_STAND_THROW") then { KJC_STAND_THROW = 7 };   // [integration] m/s a man is thrown clear of a standing foot (kaiju binary); on the stock binary he is put this far out in heights x 0.05

// ---- the stomp -------------------------------------------------------------
if (isNil "KJC_STOMP_REACH") then { KJC_STOMP_REACH = 0.40 };// how far from his origin he can plant a stomp
if (isNil "KJC_STOMP_WIND") then { KJC_STOMP_WIND = 1.3 };   // s the foot hangs before it comes down
if (isNil "KJC_STOMP_FOOT") then { KJC_STOMP_FOOT = 0.17 };  // under the stomp: everything is destroyed
if (isNil "KJC_STOMP_KILL") then { KJC_STOMP_KILL = 0.30 };  // men die, light vehicles are wrecked and thrown, buildings fall
if (isNil "KJC_STOMP_RING") then { KJC_STOMP_RING = 0.75 };  // the shock ring: light vehicles wrecked, men thrown down
if (isNil "KJC_STOMP_SPEED") then { KJC_STOMP_SPEED = 45 };  // m/s the ring travels
if (isNil "KJC_LIGHT_MASS") then { KJC_LIGHT_MASS = 9000 };  // kg: a vehicle below this is light (cars, trucks, jeeps)

// ---- the swat --------------------------------------------------------------
if (isNil "KJC_SHOULDER_H") then { KJC_SHOULDER_H = 0.825 }; // shoulder height
if (isNil "KJC_SHOULDER_X") then { KJC_SHOULDER_X = 0.29 };  // shoulder, sideways from his centre line
if (isNil "KJC_ARM") then { KJC_ARM = 0.56 };                // shoulder to fingertip, with the lean of the swing
if (isNil "KJC_KICK") then { KJC_KICK = 0.42 };              // how far from his origin a foot reaches a thing on the ground
if (isNil "KJC_SWAT_WIND") then { KJC_SWAT_WIND = 0.9 };     // s from the start of the swing to the blow
if (isNil "KJC_SWAT_GRACE") then { KJC_SWAT_GRACE = 1.15 };  // the blow still lands if the target is within reach x this
if (isNil "KJC_SWAT_AIR_V") then { KJC_SWAT_AIR_V = 38 };    // m/s given to a struck aircraft
if (isNil "KJC_KICK_V") then { KJC_KICK_V = 20 };            // m/s given to a kicked tank (measured: 13 moves a live T-80 6 m, 25 moves it 18 m)
if (isNil "KJC_KICK_DAMAGE") then { KJC_KICK_DAMAGE = 0.8 }; // a kicked tank is left at least this damaged

// ---- being shot ------------------------------------------------------------
if (isNil "KJC_HEAL_TICK") then { KJC_HEAL_TICK = 0.2 };     // s between damage reads
if (isNil "KJC_HURT_MIN") then { KJC_HURT_MIN = 0.001 };     // a rise in damage below this is not a hit (the engine's own floor)
if (isNil "KJC_HURT_HARD") then { KJC_HURT_HARD = 0.010 };   // a rise of this much is a hard hit (severity 1)
if (isNil "KJC_SHOT_MEMORY") then { KJC_SHOT_MEMORY = 4 };   // s a shot stays the likely cause of a hit
if (isNil "KJC_SHOT_HIT_MIN") then { KJC_SHOT_HIT_MIN = 170 };// ammo "hit" below this cannot mark him: its shots are not remembered
if (isNil "KJC_WATCH_R") then { KJC_WATCH_R = 1200 };        // m: shooters are listened to inside this
if (isNil "KJC_WATCH_TICK") then { KJC_WATCH_TICK = 8 };     // s between looks for new shooters
if (isNil "KJC_FLINCH_GAP") then { KJC_FLINCH_GAP = 5 };     // s at least between two flinch moves
if (isNil "KJC_FLINCH_ANIM") then { KJC_FLINCH_ANIM = true };// play DedFlinch on a hard hit (the brain may take this over)
// ========================== end of the tuning block ==========================

// ---- public state (read by the brain, the animations and the tests) ----------
if (isNil "KJ_crushOn") then { KJ_crushOn = true };          // set false to end loop.sqs and hurt.sqs
KJ_footfall = [0, 0, [0, 0, 0], "L"];                         // [count, time, pos, side] of the last footfall
KJ_hurt = [0, -1, -1, 0, objNull];                            // [time, bearing from north, bearing from his front, severity, who]
KJ_hurtCount = 0;
KJ_revived = 0;
KJ_crushStats = [0, 0, 0, 0, 0];                              // buildings, small things, vehicles, men dead, men thrown
KJ_crushLog = [];                                             // [object, time marked, time felled, forward m when marked]
KJ_stomping = false;
KJ_stompCount = 0;
KJ_stompResult = "";
KJ_stompPos = [0, 0, 0];
KJ_swatting = false;
KJ_swatCount = 0;
KJ_swatResult = "";
KJ_swatHow = "";
KJC_pending = [];                                             // statics that are marked and not yet felled
KJC_lastShot = [-100, objNull, ""];                           // [time, who, ammo] of the last heavy shot heard
KJC_hitEvent = -100;                                          // time of the last "Hit" event
KJC_animT = -100;                                             // time of his last change of clip (AnimChanged with a new move name)
KJC_animMove = "";
KJC_lastFlinch = -100;
KJC_stepFile = (loadFile "kj\roar\step.sqs") != "";           // the roar feature's footfall, when it is there
// [integration] the shared core (which engine, kill with credit, throw, memory points)
if (isNil "KJ_coreReady") then { call compile preprocessFile "kj\init.sqf" };
KJC_nativeSteps = false;                                       // true: footfalls are the engine's events (kj\crush\steps.sqs)
KJC_stepFlag = false;                                          // steps.sqs to loop.sqs: a real footfall has just landed
KJC_stepNext = 1;                                              // the foot that lands next: 1 right, -1 left
KJ_crushPushes = [];                                           // [[time, what, metres from the foot], ...] things cleared from his standing feet

// ---- functions -----------------------------------------------------------------

// [kaiju] call KJC_fnSize : reads the height once.
KJC_fnSize = {
    private ["_h"];
    if (!(isNil "KJ_height")) then { KJC_H = KJ_height };
    _h = format ["%1", (typeOf (_this select 0)) GetVehicleParam "kjHeight"];
    if (_h != "<null>") then { KJC_H = call _h };
    KJC_sized = true
};

// [kaiju, pos] call KJC_fnLocal : [forward, right] of pos in his frame, metres.
KJC_fnLocal = {
    private ["_kp", "_p", "_d", "_dx", "_dy"];
    _kp = getPos (_this select 0);
    _p = _this select 1;
    _d = getDir (_this select 0);
    _dx = (_p select 0) - (_kp select 0);
    _dy = (_p select 1) - (_kp select 1);
    [_dx * sin _d + _dy * cos _d, _dx * cos _d - _dy * sin _d]
};

// [kaiju, forward, right] call KJC_fnWorld : world [x, y, 0] of a point in his frame.
KJC_fnWorld = {
    private ["_kp", "_d", "_f", "_r"];
    _kp = getPos (_this select 0);
    _d = getDir (_this select 0);
    _f = _this select 1;
    _r = _this select 2;
    [(_kp select 0) + _f * sin _d + _r * cos _d, (_kp select 1) + _f * cos _d - _r * sin _d, 0]
};

// [a, b] call KJC_fnDist2 : flat distance between two positions.
KJC_fnDist2 = {
    private ["_a", "_b"];
    _a = _this select 0;
    _b = _this select 1;
    sqrt (((_a select 0) - (_b select 0)) ^ 2 + ((_a select 1) - (_b select 1)) ^ 2)
};

// object call KJC_fnKind : "man", "air", "tank", "car", "big", "small" or "skip".
// getMass is only ever called on a thing that is known to have a model: on the
// stock engine it crashes the game on one that has none (Object.hpp:456), and
// the engine's own collapse helper ("ObjectDestructed", made for every felled
// building and tree) is such a thing. So a static must be a plain map object
// (typeOf "") or a config building, thing or street lamp, and its name must
// not be the engine's "<no shape>".
KJC_fnKind = {
    private ["_k", "_t", "_m", "_n", "_l", "_shaped"];
    _k = "skip";
    if (!(isNull _this)) then {
        if (("Man" countType [_this]) > 0) then { _k = "man" } else {
            if (("Air" countType [_this]) > 0) then { _k = "air" } else {
                if (("LandVehicle" countType [_this]) > 0) then {
                    if ((("Tank" countType [_this]) > 0) or ((getMass _this) >= KJC_LIGHT_MASS)) then { _k = "tank" } else { _k = "car" }
                } else {
                    if (("Ship" countType [_this]) > 0) then { _k = "car" } else {
                        _t = typeOf _this;
                        _n = format ["%1", _this];
                        _l = sizeofstr _n;
                        _shaped = true;
                        if (_l >= 10) then { _shaped = (substr [_n, _l - 10, _l]) != "<no shape>" };
                        if (_shaped and ((_t == "") or (_t in KJC_SAFE_TYPES) or (("Static" countType [_this]) > 0) or (("Thing" countType [_this]) > 0))) then {
                            // a static. Roads, bushes and helpers have no mass; forest blocks cannot fall.
                            _m = getMass _this;
                            if ((_m >= KJC_MIN_MASS) and (("Forest" countType [_this]) == 0)) then {
                                if (_m >= KJC_BIG_MASS) then { _k = "big" } else { _k = "small" }
                            }
                        }
                    }
                }
            }
        }
    };
    _k
};
KJC_SAFE_TYPES = ["StreetLamp", "StreetLampWood", "StreetLampMetal"];

// [[x, y], radius, count, size0, size1, life, up, out, [r, g, b], alpha, height] call KJC_fnDust
// A burst of dust puffs spread over a disc on the ground; each drifts outwards
// and up, then thins out.
KJC_fnDust = {
    private ["_c", "_r", "_n", "_s0", "_s1", "_life", "_up", "_out", "_col", "_a", "_h", "_i", "_ang", "_rr", "_cr", "_cg", "_cb", "_v"];
    _c = _this select 0;
    _r = _this select 1;
    _n = _this select 2;
    _s0 = _this select 3;
    _s1 = _this select 4;
    _life = _this select 5;
    _up = _this select 6;
    _out = _this select 7;
    _col = _this select 8;
    _a = _this select 9;
    _h = _this select 10;
    _cr = _col select 0;
    _cg = _col select 1;
    _cb = _col select 2;
    _i = 0;
    while { _i < _n } do {
        _ang = random 360;
        _rr = _r * sqrt (random 1);
        _v = _out * (0.4 + random 0.6);
        drop ["cl_basic", "", "Billboard", 1, _life * (0.7 + random 0.6), [(_c select 0) + _rr * sin _ang, (_c select 1) + _rr * cos _ang, 0.5 + _h * random 1], [_v * sin _ang, _v * cos _ang, _up * (0.3 + random 0.7)], 0, 1.3, 1, 0.15, [_s0, _s1 * 0.6, _s1 * 0.85, _s1], [[_cr, _cg, _cb, _a * 0.7], [_cr, _cg, _cb, _a], [_cr, _cg, _cb, _a * 0.5], [_cr, _cg, _cb, 0]], [0], 0.6, 0.25, "", ""];
        _i = _i + 1
    }
};

// [[x, y], radius, count, size0, size1, life, up, out, [r, g, b], alpha, height] call KJC_fnPuff
// As KJC_fnDust, but a ball of smoke at a height (a blow in the air).
KJC_fnPuff = {
    private ["_c", "_r", "_n", "_col", "_h", "_i", "_ang", "_v"];
    _c = _this select 0;
    _r = _this select 1;
    _n = _this select 2;
    _col = _this select 8;
    _h = _this select 10;
    _i = 0;
    while { _i < _n } do {
        _ang = random 360;
        _v = (_this select 7) * (0.3 + random 0.7);
        drop ["cl_basic", "", "Billboard", 1, (_this select 5) * (0.7 + random 0.6), [(_c select 0) + _r * (random 1) * sin _ang, (_c select 1) + _r * (random 1) * cos _ang, _h - _r + 2 * _r * random 1], [_v * sin _ang, _v * cos _ang, (_this select 6) * ((random 2) - 1)], 0, 1.3, 1, 0.3, [_this select 3, _this select 4], [[_col select 0, _col select 1, _col select 2, _this select 9], [_col select 0, _col select 1, _col select 2, 0.5 * (_this select 9)], [_col select 0, _col select 1, _col select 2, 0]], [0], 0.6, 0.25, "", ""];
        _i = _i + 1
    }
};

// [[x, y], radius, count, speed, size, life] call KJC_fnRing : a ring of low dust racing outwards.
KJC_fnRing = {
    private ["_c", "_r", "_n", "_sp", "_s", "_life", "_i", "_ang"];
    _c = _this select 0;
    _r = _this select 1;
    _n = _this select 2;
    _sp = _this select 3;
    _s = _this select 4;
    _life = _this select 5;
    _i = 0;
    while { _i < _n } do {
        _ang = (360 / _n) * _i + random 6;
        drop ["cl_basic", "", "Billboard", 1, _life, [(_c select 0) + _r * sin _ang, (_c select 1) + _r * cos _ang, 0.8], [_sp * sin _ang, _sp * cos _ang, 0.6], 0, 1.3, 1, 0.02, [_s * 0.5, _s, _s * 1.6], [[0.62, 0.58, 0.5, 0.45], [0.62, 0.58, 0.5, 0.55], [0.62, 0.58, 0.5, 0.3], [0.62, 0.58, 0.5, 0]], [0], 0, 0, "", ""];
        _i = _i + 1
    }
};

// [man, [x, y] thrown from, strength 0..1, kill] call KJC_fnThrowMan
// A man caught by the shock. Killed, or thrown down: he goes to ground at once
// and is held there for a few seconds by down.sqs. The stock engine does not
// let a script throw a man (setVelocity moves vehicles, not men: measured, see
// NOTES.md), so he drops where he stands.
// [integration] On the kaiju binary he IS thrown (kjImpulse: away from the point and up),
// dead or alive, and the kill is credited.
KJC_fnThrowMan = {
    private ["_m", "_p", "_c", "_dx", "_dy", "_d", "_v"];
    _m = _this select 0;
    if ((vehicle _m) == _m) then {
        if (KJ_native) then {
            _p = getPos _m;
            _c = _this select 1;
            _dx = (_p select 0) - (_c select 0);
            _dy = (_p select 1) - (_c select 1);
            _d = sqrt (_dx * _dx + _dy * _dy);
            if (_d < 0.5) then { _dx = sin (random 360); _dy = cos (random 360); _d = 1 };
            // measured: 10 m/s out and 5 up kills a man when he lands. A throw that is not
            // meant to kill is a shove off his feet.
            _v = 2 + 2.5 * (_this select 2);
            if (_this select 3) then { _v = 6 + 6 * (_this select 2) };
            [_m, [_v * _dx / _d, _v * _dy / _d, 0.4 * _v]] call KJ_fnThrow
        };
        if (_this select 3) then { [_m, KJ_unit] call KJ_fnKill } else { [_m, 3 + 4 * (_this select 2)] exec "kj\crush\down.sqs" }
    }
};

// [vehicle, [x, y] thrown from, speed, up] call KJC_fnShove : gives a vehicle a velocity away from the point.
KJC_fnShove = {
    private ["_o", "_c", "_v", "_up", "_p", "_dx", "_dy", "_d"];
    _o = _this select 0;
    _c = _this select 1;
    _v = _this select 2;
    _up = _this select 3;
    _p = getPos _o;
    _dx = (_p select 0) - (_c select 0);
    _dy = (_p select 1) - (_c select 1);
    _d = sqrt (_dx * _dx + _dy * _dy);
    if (_d < 0.5) then { _dx = sin (random 360); _dy = cos (random 360); _d = 1 };
    // [integration] through the physics on the kaiju binary (a hull tumbles), setVelocity on the stock one
    [_o, [_v * _dx / _d, _v * _dy / _d, _up]] call KJ_fnThrow
};

// [static, big, forward m] call KJC_fnMark : stages one building, tree, fence or pole
// (dust first, the collapse a beat later). An object is only ever marked once.
KJC_fnMark = {
    private ["_o"];
    _o = _this select 0;
    if (((getDammage _o) < 1) and !(_o in KJC_pending)) then {
        KJC_pending = KJC_pending + [_o];
        _this exec "kj\crush\fell.sqs"
    }
};

// [kaiju, vehicle, kind] call KJC_fnKickAside
// A vehicle in the way of a swinging leg: wrecked and knocked sideways out of
// the strip his legs use. His body collides with vehicles, wrecks included, and
// a hull is a wall to him (measured: a T-80 in a foot lane stopped him dead), so
// nothing may be left standing in the strip.
KJC_fnKickAside = {
    private ["_k", "_v", "_l", "_side", "_d", "_p"];
    _k = _this select 0;
    _v = _this select 1;
    _l = [_k, getPos _v] call KJC_fnLocal;
    _side = 1;
    if ((_l select 1) < 0) then { _side = -1 };
    _d = getDir _k;
    _p = getPos _v;
    _v setVariable ["kjcKickT", time];
    if ((getDammage _v) < 1) then {
        KJ_crushStats set [2, (KJ_crushStats select 2) + 1];
        _v setDamage 1
    };
    [_v, [_side * KJC_SWING_V * cos _d + 2 * sin _d, 2 * cos _d - _side * KJC_SWING_V * sin _d, 3]] call KJ_fnThrow;
    [[_p select 0, _p select 1], 3, 4, 2, 7, 4, 1.5, 3, [0.55, 0.51, 0.44], 0.5, 1] call KJC_fnDust;
    true
};

// [kaiju, walking, speed m/s, ground since the last footfall, foot that lands next] call KJC_fnWalkPass
// One pass over the strip his legs are about to cross (walking) or are standing
// in (not walking).
//   Statics are marked once each and staged by fell.sqs. A building is marked
//   far enough ahead for its dust and its collapse to be over as his toes
//   arrive, so the lead grows with his speed.
//   A vehicle in a foot lane goes as the toes of that foot reach it (worked out
//   from the stride, like the footfalls): flattened if it lies under the middle
//   of the lane, kicked aside if the foot only clips it. One between his legs
//   is left alone.
KJC_fnWalkPass = {
    private ["_k", "_walk", "_lead", "_leadS", "_vlead", "_far", "_back", "_lat", "_bigR", "_kp", "_dir", "_sd", "_cd", "_mid", "_r", "_op", "_dx", "_dy", "_f", "_s", "_sr", "_kind", "_toeSw", "_toePl", "_toe", "_hull"];
    _k = _this select 0;
    _walk = _this select 1;
    _leadS = KJC_LEAD_SMALL * KJC_H;
    _lead = _leadS + KJC_LEAD_TIME * (_this select 2);
    if (_lead < _leadS + KJC_LEAD_MIN * KJC_H) then { _lead = _leadS + KJC_LEAD_MIN * KJC_H };
    // how far ahead of his origin the toes of each foot are: the foot in the air
    // (it lands next) swings from behind to its landing spot, the planted one falls behind
    _toeSw = (KJC_FOOT_AHEAD + KJC_TOE_LEN - KJC_HALF_STRIDE) * KJC_H + (_this select 3);
    _toePl = (KJC_FOOT_AHEAD + KJC_TOE_LEN) * KJC_H - (_this select 3);
    _hull = KJC_HULL * KJC_H;
    _vlead = (KJC_FOOT_AHEAD + KJC_TOE_LEN + KJC_HULL) * KJC_H;
    if (!_walk) then { _lead = KJC_TOE * KJC_H; _leadS = _lead; _vlead = 0 };
    _back = KJC_BACK * KJC_H;
    _lat = (KJC_LEG_HALF + KJC_BESIDE) * KJC_H;
    _bigR = KJC_BIG_R * KJC_H;
    _far = _lead + _bigR;
    if (_far < _vlead) then { _far = _vlead };
    _kp = getPos _k;
    _dir = getDir _k;
    _sd = sin _dir;
    _cd = cos _dir;
    _mid = (_far - _back - _bigR) / 2;
    _r = sqrt ((((_far + _back + _bigR) / 2)) ^ 2 + (_lat + _bigR) ^ 2);
    {
        _op = getPos _x;
        _dx = (_op select 0) - (_kp select 0);
        _dy = (_op select 1) - (_kp select 1);
        _f = _dx * _sd + _dy * _cd;
        _sr = _dx * _cd - _dy * _sd;
        _s = abs _sr;
        if ((_s <= _lat + _bigR) and (_f >= 0 - (_back + _bigR)) and (_f <= _far)) then {
            _kind = _x call KJC_fnKind;
            if ((getDammage _x) < 1) then {
                if (_kind == "big") then {
                    if (_f <= _lead + _bigR) then { [_x, true, _f] call KJC_fnMark }
                };
                if (_kind == "small") then {
                    if ((_s <= _lat + 1) and (_f >= 0 - _back) and (_f <= _leadS)) then { [_x, false, _f] call KJC_fnMark }
                }
            };
            // a man in a foot lane dies a moment before the foot is on him: a standing
            // man's body shoves the giant aside (measured: 3 m), a fallen one does not
            if ((_kind == "man") and _walk) then {
                _toe = _toePl;
                if ((_sr * (_this select 4)) > 0) then { _toe = _toeSw };
                if ((_x != _k) and (alive _x) and ((_op select 2) < 3) and ((abs (_s - KJC_STANCE * KJC_H)) <= KJC_MAN_LANE * KJC_H) and (_f >= 0 - _back) and (_f - KJC_MAN_LEAD * KJC_H <= _toe)) then {
                    if ((vehicle _x) == _x) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                }
            };
            if ((_kind == "tank") or (_kind == "car")) then {
                _toe = _toePl;
                if ((_sr * (_this select 4)) > 0) then { _toe = _toeSw };
                if (_walk and ((_op select 2) < 3) and (_s >= KJC_GAP * KJC_H) and (_s <= _lat + 2) and (_f >= 0 - _back) and (_f - _hull <= _toe)) then {
                    // [integration] a wreck (the beam's, a stomp's) cannot be relied on to slide when
                    // kicked, and a hull that was kicked and is still in the lane did not: both are
                    // flattened, which takes the hull away (a hull left in a lane stops him and
                    // throws him back metres at a stride)
                    if (!(_x getVariable ["kjcFlat", false]) and ((time - (_x getVariable ["kjcKickT", -100])) > 0.7)) then {
                        if (((abs (_s - KJC_STANCE * KJC_H)) <= KJC_FOOT_HALF * KJC_H) or ((getDammage _x) >= 1) or ((_x getVariable ["kjcKickT", -100]) > 0)) then {
                            _x setVariable ["kjcFlat", true];
                            [_x, [_op select 0, _op select 1]] exec "kj\crush\flatten.sqs";
                            KJ_crushStats set [2, (KJ_crushStats select 2) + 1]
                        } else {
                            [_k, _x, _kind] call KJC_fnKickAside
                        }
                    }
                }
            }
        }
    } forEach (nearestObjects [[(_kp select 0) + _mid * _sd, (_kp select 1) + _mid * _cd, 0], [], _r]);
    true
};

// [kaiju, [x, y] foot, foot radius, near radius] call KJC_fnFootfall
// What one planted foot does to men and vehicles. Statics are the walk pass's job.
KJC_fnFootfall = {
    private ["_k", "_c", "_rf", "_rn", "_d", "_kind", "_s", "_hurt"];
    _k = _this select 0;
    _c = _this select 1;
    _rf = _this select 2;
    _rn = _this select 3;
    {
        if ((_x != _k) and (alive _x)) then {
            _d = [getPos _x, _c] call KJC_fnDist2;
            _kind = _x call KJC_fnKind;
            if (_d <= _rf) then {
                if (_kind == "man") then {
                    if ((vehicle _x) == _x) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                };
                if (((_kind == "tank") or (_kind == "car")) and !(_x getVariable ["kjcFlat", false])) then {
                    _x setVariable ["kjcFlat", true];
                    [_x, _c] exec "kj\crush\flatten.sqs";
                    KJ_crushStats set [2, (KJ_crushStats select 2) + 1]
                }
            } else {
                if (_d <= _rn) then {
                    _s = 1 - (_d - _rf) / (_rn - _rf);
                    if (_kind == "man") then {
                        if ((vehicle _x) == _x) then {
                            _hurt = (getDammage _x) + KJC_NEAR_HURT * _s;
                            if (_hurt > 0.9) then { _hurt = 0.9 };
                            _x setDamage _hurt;
                            [_x, _c, _s, false] call KJC_fnThrowMan;
                            KJ_crushStats set [4, (KJ_crushStats select 4) + 1]
                        }
                    };
                    if (_kind == "car") then { [_x, _c, 2 + 3 * _s, 2 + 3 * _s] call KJC_fnShove }
                }
            }
        }
    } forEach (nearestObjects [[_c select 0, _c select 1, 0], ["Man", "LandVehicle"], _rn]);
    true
};

// [kaiju, [x, y], foot r, kill r, ring r, from r, to r] call KJC_fnShock
// The stomp's effect on everything between two radii. What happens depends on
// the zone the thing is in: under the foot, the kill zone, or the ring.
KJC_fnShock = {
    private ["_k", "_c", "_rf", "_rk", "_rr", "_r0", "_r1", "_d", "_kind", "_fall", "_hurt", "_low", "_o"];
    _k = _this select 0;
    _c = _this select 1;
    _rf = _this select 2;
    _rk = _this select 3;
    _rr = _this select 4;
    _r0 = _this select 5;
    _r1 = _this select 6;
    {
        if (_x != _k) then {
            _d = [getPos _x, _c] call KJC_fnDist2;
            if ((_d >= _r0) and (_d < _r1) and ((getDammage _x) < 1)) then {
                _kind = _x call KJC_fnKind;
                _low = ((getPos _x) select 2) < 0.3 * KJC_H;
                // 1 at the edge of the kill zone, 0 at the edge of the ring
                _fall = 1 - (_d - _rk) / (_rr - _rk);
                if (_fall > 1) then { _fall = 1 };
                if ((_kind == "man") and _low) then {
                    if ((vehicle _x) == _x) then {
                        if (_d < _rf) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] } else {
                            if (_d < _rk) then { [_x, _c, 1, true] call KJC_fnThrowMan; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] } else {
                                _hurt = (getDammage _x) + 0.1 + 0.4 * _fall;
                                if (_hurt > 0.9) then { _hurt = 0.9 };
                                _x setDamage _hurt;
                                [_x, _c, 0.3 + 0.7 * _fall, false] call KJC_fnThrowMan;
                                KJ_crushStats set [4, (KJ_crushStats select 4) + 1]
                            }
                        }
                    }
                };
                if ((_kind == "car") and _low) then {
                    if (_d < _rf) then { _x setVariable ["kjcFlat", true]; [_x, _c] exec "kj\crush\flatten.sqs" } else {
                        _o = _x;
                        if (_d < _rk) then { { _x setDamage 1 } forEach (crew _o) };
                        _o setDamage 1;
                        [_o, _c, 5 + 9 * _fall, 3 + 4 * _fall] call KJC_fnShove
                    };
                    KJ_crushStats set [2, (KJ_crushStats select 2) + 1]
                };
                if ((_kind == "tank") and _low) then {
                    if (_d < _rf) then { _x setVariable ["kjcFlat", true]; [_x, _c] exec "kj\crush\flatten.sqs"; KJ_crushStats set [2, (KJ_crushStats select 2) + 1] } else {
                        if (_d < _rk) then {
                            if ((getDammage _x) < KJC_KICK_DAMAGE) then { _x setDamage KJC_KICK_DAMAGE };
                            [_x, _c, 5, 3] call KJC_fnShove
                        } else {
                            _x setDamage ((getDammage _x) + 0.15 * _fall)
                        }
                    }
                };
                if ((_kind == "air") and _low) then {
                    if (_d < _rk) then { _x setDamage 1; KJ_crushStats set [2, (KJ_crushStats select 2) + 1] } else { _x setDamage ((getDammage _x) + 0.4 * _fall) }
                };
                if (_kind == "big") then { if (_d < _rk) then { [_x, true, _d] call KJC_fnMark } };
                if (_kind == "small") then { if ((_d < _rk) or (_fall > 0.45)) then { [_x, false, _d] call KJC_fnMark } }
            }
        }
    } forEach (nearestObjects [[_c select 0, _c select 1, 0], [], _r1 + 2]);
    true
};

// [kaiju, object, grace] call KJC_fnReach : how he can reach it.
// [how, distance used, side] with how "arm", "kick" or "out"; side -1 left, 1 right.
// grace widens both reaches (1 = as tuned).
KJC_fnReach = {
    private ["_k", "_o", "_g", "_kp", "_op", "_l", "_side", "_sh", "_dz", "_dArm", "_dFlat", "_how", "_dUse"];
    _k = _this select 0;
    _o = _this select 1;
    _g = _this select 2;
    _kp = getPosASL _k;
    _op = getPosASL _o;
    _l = [_k, getPos _o] call KJC_fnLocal;
    _side = 1;
    if ((_l select 1) < 0) then { _side = -1 };
    // the nearer shoulder sits at [0, side * shoulder x, shoulder height] in his frame
    _sh = (_l select 1) - _side * KJC_SHOULDER_X * KJC_H;
    _dz = ((_op select 2) - (_kp select 2)) - KJC_SHOULDER_H * KJC_H;
    _dArm = sqrt ((_l select 0) ^ 2 + _sh ^ 2 + _dz ^ 2);
    _dFlat = sqrt ((_l select 0) ^ 2 + (_l select 1) ^ 2);
    _how = "out";
    _dUse = _dArm;
    if (_dArm <= KJC_ARM * KJC_H * _g) then { _how = "arm" } else {
        if ((((_op select 2) - (_kp select 2)) < 0.12 * KJC_H) and (_dFlat <= KJC_KICK * KJC_H * _g)) then { _how = "kick"; _dUse = _dFlat }
    };
    [_how, _dUse, _side]
};

// [kaiju] call KJC_fnWatch : wires "Fired" on the heavy shooters around him, once each.
KJC_fnWatch = {
    private ["_k"];
    _k = _this select 0;
    {
        if ((_x != _k) and !(_x getVariable ["kjcFired", false])) then {
            _x setVariable ["kjcFired", true];
            _x addEventHandler ["Fired", { _this call KJC_ehFired }]
        }
    } forEach (nearestObjects [getPos _k, ["Tank", "Car", "Air", "Ship"], KJC_WATCH_R]);
    {
        if ((_x != _k) and !(_x getVariable ["kjcFired", false])) then {
            _x setVariable ["kjcFired", true];
            _x addEventHandler ["Fired", { _this call KJC_ehFired }]
        }
    } forEach (nearestObjects [getPos _k, ["Man"], 0.35 * KJC_WATCH_R]);
    true
};

// [kaiju, rise in damage, who or objNull] call KJC_fnHurt : raises the hit signal.
KJC_fnHurt = {
    private ["_k", "_rise", "_who", "_sev", "_abs", "_rel", "_kp", "_wp"];
    _k = _this select 0;
    _rise = _this select 1;
    _who = _this select 2;
    _sev = _rise / KJC_HURT_HARD;
    if (_sev > 1) then { _sev = 1 };
    _abs = -1;
    _rel = -1;
    if (isNull _who) then {
        if ((time - (KJC_lastShot select 0)) <= KJC_SHOT_MEMORY) then { _who = KJC_lastShot select 1 }
    };
    // [integration] never himself, and never a shooter standing on his own spot ("0 atan2 0" is a script error)
    if (!(isNull _who)) then { if (_who == _k) then { _who = objNull } };
    if (!(isNull _who)) then {
        _kp = getPos _k;
        _wp = getPos _who;
        if (((abs ((_wp select 0) - (_kp select 0))) + (abs ((_wp select 1) - (_kp select 1)))) > 0.01) then {
            _abs = ((_wp select 0) - (_kp select 0)) atan2 ((_wp select 1) - (_kp select 1));
            if (_abs < 0) then { _abs = _abs + 360 };
            _rel = _abs - (getDir _k);
            if (_rel < 0) then { _rel = _rel + 360 }
        }
    };
    KJ_hurtCount = KJ_hurtCount + 1;
    KJ_hurt = [time, _abs, _rel, _sev, _who];
    if (KJC_FLINCH_ANIM and (_sev >= 1) and ((time - KJC_lastFlinch) >= KJC_FLINCH_GAP)) then {
        KJC_lastFlinch = time;
        _k playMove "DedFlinch"
    };
    true
};

// [integration] [kaiju] call KJC_fnStandGuard : while he stands or turns on the spot. A man
// who runs against one of his feet, or a hull that rolls or is thrown against it, shoves HIM
// (the engine pushes a Man out of men and vehicles with no regard for mass: a standing man moved
// him 3 m, a hull 6 m). So whatever comes within KJC_STAND_R of a foot is cleared away first:
//   a man     thrown clear and down (kaiju binary), or put out of reach and down (stock)
//   a car     wrecked and knocked away
//   a tank    shoved away and damaged; a wreck of any kind is flattened where it lies
// The feet are his foot memory points on the kaiju binary, else his stance.
KJC_fnStandGuard = {
    private ["_k", "_kp", "_feet", "_r", "_o", "_op", "_d", "_best", "_bf", "_kind", "_dx", "_dy", "_out", "_i"];
    _k = _this select 0;
    _kp = getPos _k;
    _r = KJC_STAND_R * KJC_H;
    _feet = [[_k, 0.02 * KJC_H, KJC_STANCE * KJC_H] call KJC_fnWorld, [_k, 0.02 * KJC_H, 0 - KJC_STANCE * KJC_H] call KJC_fnWorld];
    if (KJ_native) then { if (KJ_hasFootPoints) then { _feet = [kjPoint [_k, "stopaP"], kjPoint [_k, "stopaL"]] } };
    {
        _o = _x;
        if ((_o != _k) and !(_o getVariable ["kjcFlat", false])) then {
            _op = getPos _o;
            if ((_op select 2) < 3) then {
                _best = 100000;
                _bf = _feet select 0;
                { _d = [_op, _x] call KJC_fnDist2; if (_d < _best) then { _best = _d; _bf = _x } } forEach _feet;
                if (_best < _r) then {
                    _kind = _o call KJC_fnKind;
                    if ((_kind == "man") and (alive _o) and ((vehicle _o) == _o)) then {
                        if ((count KJ_crushPushes) < 60) then { KJ_crushPushes = KJ_crushPushes + [[time, "man", _best]] };
                        if (KJ_native) then {
                            [_o, [_bf select 0, _bf select 1], 0.5, false] call KJC_fnThrowMan
                        } else {
                            _dx = (_op select 0) - (_bf select 0);
                            _dy = (_op select 1) - (_bf select 1);
                            _d = sqrt (_dx * _dx + _dy * _dy);
                            if (_d < 0.5) then { _dx = sin (random 360); _dy = cos (random 360); _d = 1 };
                            _out = _r + 0.05 * KJC_H;
                            _o setPos [(_bf select 0) + _out * _dx / _d, (_bf select 1) + _out * _dy / _d, 0];
                            [_o, 4] exec "kj\crush\down.sqs"
                        };
                        KJ_crushStats set [4, (KJ_crushStats select 4) + 1]
                    };
                    if ((_kind == "car") or (_kind == "tank")) then {
                        if ((count KJ_crushPushes) < 60) then { KJ_crushPushes = KJ_crushPushes + [[time, _kind, _best]] };
                        if ((getDammage _o) >= 1) then {
                            _o setVariable ["kjcFlat", true];
                            [_o, [_bf select 0, _bf select 1]] exec "kj\crush\flatten.sqs"
                        } else {
                            if (_kind == "car") then { [_k, _o, _kind] call KJC_fnKickAside } else {
                                if ((time - (_o getVariable ["kjcKickT", -100])) > 1) then {
                                    _o setVariable ["kjcKickT", time];
                                    _o setDamage ((getDammage _o) + 0.3);
                                    [_o, [_bf select 0, _bf select 1], 0.5 * KJC_KICK_V, 2] call KJC_fnShove
                                }
                            }
                        }
                    }
                }
            }
        }
    } forEach (nearestObjects [[_kp select 0, _kp select 1, 0], ["Man", "LandVehicle"], (KJC_STANCE + KJC_STAND_R + 0.05) * KJC_H]);
    true
};

// Event handler bodies, kept as globals so a mission can see what is wired.
// "Hit": [unit, who, damage]. Fires only for a single hit worth 0.05 of total
// damage or more (AI/VehicleAICombat.cpp, EntityAI::HitBy).
KJC_ehHit = { KJC_hitEvent = time; if (!KJ_native) then { [_this select 0, _this select 2, _this select 1] call KJC_fnHurt } };
// "IncomingMissile": [unit, ammo, who fired]. The missile is still in the air,
// so the memory is stamped a few seconds ahead.
KJC_ehMissile = { KJC_lastShot = [time + 3, _this select 2, _this select 1] };
// "Fired" on a watched shooter: [unit, weapon, muzzle, mode, ammo].
KJC_ehFired = { if ((call ((_this select 4) GetAmmoParam "hit")) >= KJC_SHOT_HIT_MIN) then { KJC_lastShot = [time, _this select 0, _this select 4] } };

KJC_ready = true;
