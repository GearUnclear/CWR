// ============================================================================
//  kj\combat\init.sqf  -  Dedushka COMBAT: how he is hurt, how it shows, how
//  he dies. Tuning and shared functions. Loaded once by kj\combat\start.sqs:
//      call compile preprocessFile "kj\combat\init.sqf"
//
//  Works on the stock engine binary and on the kaiju binary. On the kaiju
//  binary every hit is read from the engine's own hit report (what round, where
//  on his body, from whom). On the stock binary only the rise of his damage
//  can be read, so a hit has a weight and no place.
//
//  The health is the script's own, counted in WOUND POINTS: one point is one
//  tank HEAT round into plain body. The engine's own damage value is only the
//  messenger: it is read and wiped every tick, and never kills him.
// ============================================================================

// ============================ TUNING (the one block) =========================
if (isNil "KJCB_TICK") then { KJCB_TICK = 0.1 };               // s between two reads of the hit report

// ---- what a round is worth ---------------------------------------------------
if (isNil "KJCB_REF") then { KJCB_REF = 700 };                 // ammo "hit" that is worth one wound point (a tank's HEAT round)
if (isNil "KJCB_POW") then { KJCB_POW = 1.5 };                 // how steeply heavier ordnance counts for more
if (isNil "KJCB_NOTICE") then { KJCB_NOTICE = 15 };            // ammo "hit" below this is nothing to him: rifles, machine guns
if (isNil "KJCB_REAL") then { KJCB_REAL = 150 };               // from here a hit truly hurts; between the two it stings
if (isNil "KJCB_STING") then { KJCB_STING = 0.25 };            // a sting counts for this share of its points
if (isNil "KJCB_STING_FLOOR") then { KJCB_STING_FLOOR = 0.72 };// stings alone never take him below this share of his health
if (isNil "KJCB_HARD") then { KJCB_HARD = 500 };               // ammo "hit" that is a hard hit to the brain (severity 1)
if (isNil "KJCB_STING_GAP") then { KJCB_STING_GAP = 2.5 };     // s at least between two hit signals raised by stings (a cannon burst is one annoyance, not thirty)

// ---- where it lands (fractions of his height) ----------------------------------
if (isNil "KJCB_Z_FURNACE") then { KJCB_Z_FURNACE = 2.5 };     // into the open furnace chest
if (isNil "KJCB_Z_OPEN") then { KJCB_Z_OPEN = 4 };             // the same while the ribs stand open for the beam
if (isNil "KJCB_Z_HEAD") then { KJCB_Z_HEAD = 1.5 };
if (isNil "KJCB_Z_BODY") then { KJCB_Z_BODY = 1 };             // back, flanks, arms, belly
if (isNil "KJCB_Z_LEGS") then { KJCB_Z_LEGS = 0.4 };
if (isNil "KJCB_Z_BLIND") then { KJCB_Z_BLIND = 1.4 };         // stock binary: the place is not known
if (isNil "KJCB_FURNACE_R") then { KJCB_FURNACE_R = 0.2 };     // the furnace is this far round its point
if (isNil "KJCB_HEAD_UP") then { KJCB_HEAD_UP = 0.88 };        // above this is head
if (isNil "KJCB_LEGS_UP") then { KJCB_LEGS_UP = 0.45 };        // below this is legs
if (isNil "KJCB_BODY_R") then { KJCB_BODY_R = 0.18 };          // his body as a column, for a blast beside him
if (isNil "KJCB_GEO") then { KJCB_GEO = 0.492 };               // geometry sphere radius over height (measured on the proxy: 19.7 m of 40), when the addon has no kjGeoRadius
if (isNil "KJCB_STOCK_UP") then { KJCB_STOCK_UP = 1.6 };       // stock binary: the rise of his damage shows only the striking part of a round; its whole weight is about this much more

// ---- how much he can take ------------------------------------------------------
if (isNil "KJCB_POOL") then { KJCB_POOL = 320 };               // wound points from whole to dead
if (isNil "KJCB_RATE") then { KJCB_RATE = 1.6 };               // a wound opens no faster than this many points a second
if (isNil "KJCB_BUFFER") then { KJCB_BUFFER = 24 };            // hurt waiting to open; more than this at once is wasted
if (isNil "KJCB_SCAR") then { KJCB_SCAR = 0.3 };               // this share of every wound never mends
if (isNil "KJCB_MEND_WAIT") then { KJCB_MEND_WAIT = 40 };      // s without a real hit before he starts to mend
if (isNil "KJCB_MEND") then { KJCB_MEND = 0.5 };               // points a second, awake
if (isNil "KJCB_MEND_DORMANT") then { KJCB_MEND_DORMANT = 3 }; // times that while he is dormant
if (isNil "KJCB_STAGES") then { KJCB_STAGES = [0.85, 0.6, 0.35, 0.15] }; // health share at which each wound stage starts
if (isNil "KJCB_STAGE_BACK") then { KJCB_STAGE_BACK = 0.04 };  // he must mend this far past a mark to drop a stage

// ---- what he does about it -----------------------------------------------------
if (isNil "KJCB_FLINCH_GAP") then { KJCB_FLINCH_GAP = 8 };     // s between two flinches, unhurt; shorter with every stage
if (isNil "KJCB_ROAR_GAP") then { KJCB_ROAR_GAP = 30 };        // s at least between two roars of pain
if (isNil "KJCB_RETREAT_STAGE") then { KJCB_RETREAT_STAGE = 3 };// from this stage he turns for the sea
if (isNil "KJCB_RETREAT_PULL") then { KJCB_RETREAT_PULL = 40 };// the brain's interest in the sea while he retreats
if (isNil "KJCB_RETREAT_R") then { KJCB_RETREAT_R = 2500 };    // m: the sea is looked for this far
if (isNil "KJCB_ACT") then { KJCB_ACT = true };                // false: only the numbers and the signals, no moves, roars or orders

// ---- how it shows --------------------------------------------------------------
if (isNil "KJCB_FX") then { KJCB_FX = true };                  // false: no particles and no light
if (isNil "KJCB_FX_R") then { KJCB_FX_R = 2500 };              // m: nearer than this to the viewer, effects are made
if (isNil "KJCB_GLOW") then { KJCB_GLOW = "" };                // a fullbright sprite model for sparks and the flare ("" = find out)
// `drop` with an array position: the kaiju build reads the height as above the sea, the older stock build as above the
// ground (the beam feature measured the latter). "" = by the engine found; a mission may set true or false.
if (isNil "KJCB_DROP_ASL") then { KJCB_DROP_ASL = "" };
if (isNil "KJCB_LOG") then { KJCB_LOG = 0 };                   // tests: this many of the next hits he feels are written to the log
if (isNil "KJCB_RAW") then { KJCB_RAW = 0 };                   // tests: this many of the next raw hit reports are written to the log

// ---- the death -----------------------------------------------------------------
if (isNil "KJCB_D_STAGGER") then { KJCB_D_STAGGER = 5 };       // s he staggers
if (isNil "KJCB_D_FLARE") then { KJCB_D_FLARE = 4.5 };         // s the furnace climbs to white
if (isNil "KJCB_D_DARK") then { KJCB_D_DARK = 2.5 };           // s he stands with the furnace out
if (isNil "KJCB_D_FALL") then { KJCB_D_FALL = 5 };             // s of the fall clip
if (isNil "KJCB_BODY_REST") then { KJCB_BODY_REST = 0.1 };      // the furnace point of the lying body rests this far above the ground, in heights (kj\combat\settle.sqs)
if (isNil "KJCB_D_IMPACT") then { KJCB_D_IMPACT = 0.72 };      // the body meets the ground this far into the fall clip
if (isNil "KJCB_VENT") then { KJCB_VENT = true };              // the furnace vents as it dies
if (isNil "KJCB_VENT_KILL") then { KJCB_VENT_KILL = 0.45 };    // men nearer than this (heights) to the furnace die
if (isNil "KJCB_VENT_BURN") then { KJCB_VENT_BURN = 1.1 };     // men out to here are burnt; soft vehicles scorched
if (isNil "KJCB_FALL_LEN") then { KJCB_FALL_LEN = 1.0 };       // the fallen body reaches this far ahead of his feet (heights)
if (isNil "KJCB_FALL_HALF") then { KJCB_FALL_HALF = 0.26 };    // and this far to each side
if (isNil "KJCB_FALL_SHOVE") then { KJCB_FALL_SHOVE = 1.9 };   // the ground wave throws men down out to here (heights)
if (isNil "KJCB_RALLY_R") then { KJCB_RALLY_R = 40 };          // heights: fleeing men inside this stop running when he is down
// ========================== end of the tuning block ==========================

// ---- public state ---------------------------------------------------------------
KJ_woundStage = 0;                         // 0 unhurt, 1 stung, 2 wounded, 3 badly hurt, 4 dying on his feet, 5 dead
KJ_woundHealth = 1;                        // share of his health left, 1..0
KJ_woundScar = 0;                          // share of his health that will never come back
KJ_woundSide = 0;                          // -1: his left side has taken more, 1: his right; he favours it
KJ_woundMending = false;                   // true while he is left alone and mending
KJ_woundPace = 1;                          // how fast he should move now, 1..0.45 of his usual pace (for whoever sets his pace)
KJ_woundRetreat = false;                   // true while he is badly hurt and turning for the sea
KJ_woundRetreatPos = [];                   // where he is heading then
KJ_woundLog = [];                          // [[time, stage, health], ...] every change of stage
KJ_dying = false;                          // true from the killing blow to the body lying still
KJ_deathPhase = "";                        // "", stagger, flare, vent, dark, fall, down, body
KJ_deathLog = [];                          // [[phase, time], ...]
KJ_deathStats = [0, 0, 0, 0];              // men killed, men burnt or thrown, vehicles wrecked, statics felled by his death
if (isNil "KJ_dead") then { KJ_dead = false };
if (isNil "KJ_hurt") then { KJ_hurt = [0, -1, -1, 0, objNull] };   // [time, bearing from north, bearing from his front, severity 0..1, who]
if (isNil "KJ_hurtCount") then { KJ_hurtCount = 0 };
KJ_combatOn = true;                        // set false to end the loop (he then cannot be hurt or healed by it)
KJ_combatStats = [0, 0, 0, 0, 0, 0, 0];    // unnoticed rounds, stings, real hits, points taken, points wasted, flinches, roars of pain
KJ_combatLast = [0, "", "", 0, 0, objNull];// the last hit he felt: [time, ammo, zone, weight, points, who]
KJ_combatByZone = [0, 0, 0, 0, 0];         // real hits by place: open or plain furnace, head, body, legs, unknown
KJ_combatByAmmo = [];                      // [[ammo class, hits, points], ...]

// ---- private state ----------------------------------------------------------------
KJCB_H = 40;
KJCB_native = false;
KJCB_k = objNull;
KJCB_perm = 0;                 // points that never mend
KJCB_temp = 0;                 // points that can
KJCB_buf = 0;                  // points waiting to open
KJCB_tReal = -1000;            // time of the last real hit
KJCB_tFlinch = -1000;
KJCB_tSignal = -1000;
KJCB_tRoar = -1000;
KJCB_tPull = -1000;
KJCB_tickBest = [0, objNull, "", "", 0, []];
KJCB_tickNote = [0, objNull];
KJCB_sideSum = 0;
KJCB_furnace = [0, 5.3, 32];   // the furnace point in his frame: [right, forward, up]
KJCB_furnacePoint = "zamerny";
// [models] the creature's own hit regions (set by KJCB_fnSetup from its class); 0 or "" = the old rule
KJCB_furnaceR = 8;
KJCB_legsUp = 18;
KJCB_headR = 0;
KJCB_headPoint = "";
KJCB_head = [0, 0, 36];
KJCB_riseK = 1;
KJCB_ammoNames = [];
KJCB_ammoVals = [];
KJCB_hitEvent = [-100, objNull];
KJCB_dropASL = false;         // true: `drop` wants heights above the sea
KJCB_gz = 0;                  // what to add to a height above his ground before it goes to `drop`
KJCB_probe = objNull;
KJCB_revived = 0;
KJCB_hasBrain = false;
KJCB_hasRoar = (loadFile "kj\roar\roar.sqs") != "";
KJCB_hasShake = (loadFile "kj\roar\shake.sqs") != "";
KJCB_hasBeam = (loadFile "kj\beam\fire.sqs") != "";

// ---- small helpers ------------------------------------------------------------------

// [obj, "entry", default] call KJCB_fnCfg : a number the creature's class may carry
KJCB_fnCfg = {
    private ["_v", "_n"];
    _n = _this select 2;
    _v = format ["%1", (typeOf (_this select 0)) GetVehicleParam (_this select 1)];
    if ((_v != "<null>") and (_v != "") and (_v != format ["%1", KJCB_noSuchVariable_])) then { _n = call compile _v };
    _n
};

// [obj, "entry", default] call KJCB_fnCfgStr : a text the creature's class may carry
KJCB_fnCfgStr = {
    private ["_v"];
    _v = format ["%1", (typeOf (_this select 0)) GetVehicleParam (_this select 1)];
    if ((_v == "<null>") or (_v == "") or (_v == format ["%1", KJCB_noSuchVariable_])) then { _v = _this select 2 };
    _v
};

// "ammo class" call KJCB_fnAmmo : [hit, indirectHit, indirectHitRange], remembered
KJCB_fnAmmo = {
    private ["_i", "_r", "_a", "_v", "_n"];
    _i = KJCB_ammoNames find _this;
    if (_i >= 0) then { _r = KJCB_ammoVals select _i } else {
        _r = [];
        {
            _n = 0;
            _v = format ["%1", _this GetAmmoParam _x];
            if ((_v != "<null>") and (_v != "") and (_v != format ["%1", KJCB_noSuchVariable_])) then { _n = call compile _v };
            _r = _r + [_n]
        } forEach ["hit", "indirectHit", "indirectHitRange"];
        KJCB_ammoNames = KJCB_ammoNames + [_this];
        KJCB_ammoVals = KJCB_ammoVals + [_r]
    };
    _r
};

// [object, [x, y]] call KJCB_fnD2 : metres along the ground (`distance` to an array reads its height as above the sea)
KJCB_fnD2 = {
    private ["_p"];
    _p = getPos (_this select 0);
    sqrt (((_p select 0) - ((_this select 1) select 0)) ^ 2 + ((_p select 1) - ((_this select 1) select 1)) ^ 2)
};

// [unit, [x, y, zASL]] call KJCB_fnToLocal : [right, forward, up] in his frame
KJCB_fnToLocal = {
    private ["_p", "_d", "_dx", "_dy"];
    _p = getPosASL (_this select 0);
    _d = getDir (_this select 0);
    _dx = ((_this select 1) select 0) - (_p select 0);
    _dy = ((_this select 1) select 1) - (_p select 1);
    [_dx * cos _d - _dy * sin _d, _dx * sin _d + _dy * cos _d, ((_this select 1) select 2) - (_p select 2)]
};

// [unit, [right, forward, up]] call KJCB_fnToWorld : [x, y, z above the ground he stands on]
KJCB_fnToWorld = {
    private ["_p", "_d", "_o"];
    _p = getPos (_this select 0);
    _d = getDir (_this select 0);
    _o = _this select 1;
    [(_p select 0) + (_o select 1) * sin _d + (_o select 0) * cos _d, (_p select 1) + (_o select 1) * cos _d - (_o select 0) * sin _d, _o select 2]
};

// unit call KJCB_fnFurnaceNow : refreshes the furnace point in his frame from his present pose (kaiju binary)
KJCB_fnFurnaceNow = {
    private ["_p"];
    _p = [];
    if (KJCB_native) then { _p = kjPoint [_this, KJCB_furnacePoint] };
    if ((count _p) == 3) then { KJCB_furnace = [_this, _p] call KJCB_fnToLocal };
    // [models] and the head point
    if (KJCB_native and (KJCB_headPoint != "")) then { _p = kjPoint [_this, KJCB_headPoint]; if ((count _p) == 3) then { KJCB_head = [_this, _p] call KJCB_fnToLocal } };
    KJCB_furnace
};

// call KJCB_fnRibsOpen : true while the ribs stand open for the beam
KJCB_fnRibsOpen = {
    private ["_o"];
    _o = false;
    if (!(isNil "KJ_beamPhase")) then { if (KJ_beamPhase in ["charge", "fire", "sky"]) then { _o = true } };
    _o
};

// [right, forward, up] call KJCB_fnZone : [name, factor, index into KJ_combatByZone]
KJCB_fnZone = {
    private ["_z", "_f", "_i", "_d2", "_o", "_u"];
    _o = KJCB_furnace;
    _u = _this select 2;
    _z = "body";
    _f = KJCB_Z_BODY;
    _i = 2;
    // [models] legs: under the hip joints of this creature; head: a ball round its own head point
    if (_u < KJCB_legsUp) then { _z = "legs"; _f = KJCB_Z_LEGS; _i = 3 };
    if (KJCB_headR > 0) then {
        _d2 = ((_this select 0) - (KJCB_head select 0)) ^ 2 + ((_this select 1) - (KJCB_head select 1)) ^ 2 + (_u - (KJCB_head select 2)) ^ 2;
        if (_d2 <= (KJCB_headR ^ 2)) then { _z = "head"; _f = KJCB_Z_HEAD; _i = 1 }
    } else {
        if (_u >= (KJCB_HEAD_UP * KJCB_H)) then { _z = "head"; _f = KJCB_Z_HEAD; _i = 1 }
    };
    _d2 = ((_this select 0) - (_o select 0)) ^ 2 + ((_this select 1) - (_o select 1)) ^ 2 + (_u - (_o select 2)) ^ 2;
    if (_d2 <= (KJCB_furnaceR ^ 2)) then {
        _z = "furnace";
        _f = KJCB_Z_FURNACE;
        _i = 0;
        if (call KJCB_fnRibsOpen) then { _z = "open furnace"; _f = KJCB_Z_OPEN }
    };
    [_z, _f, _i]
};

// [[right, forward, up], blast range] call KJCB_fnNear : 1..0, how much of a blast beside him reaches his body
KJCB_fnNear = {
    private ["_p", "_u", "_d", "_r", "_f"];
    _p = _this select 0;
    _r = _this select 1;
    if (_r < 0.5) then { _r = 0.5 };
    _u = _p select 2;
    if (_u < 0) then { _u = 0 };
    if (_u > KJCB_H) then { _u = KJCB_H };
    _d = (sqrt ((_p select 0) ^ 2 + (_p select 1) ^ 2 + ((_p select 2) - _u) ^ 2)) - KJCB_BODY_R * KJCB_H;
    _f = 1;
    if (_d > _r) then { _f = 1 - (_d - _r) / (3 * _r) };
    if (_f < 0) then { _f = 0 };
    _f
};

// rise in damage call KJCB_fnFromRise : the ammo "hit" that one direct hit of that rise stands for
KJCB_fnFromRise = {
    private ["_e"];
    _e = 0;
    if (_this > 0) then { _e = sqrt (_this * KJCB_riseK) };
    _e
};

// unit call KJCB_fnSetup : once. Height, furnace point, engine, traits.
KJCB_fnSetup = {
    private ["_k", "_a", "_s", "_r", "_p"];
    _k = _this;
    KJCB_k = _k;
    KJCB_H = [_k, "kjHeight", 40] call KJCB_fnCfg;
    if (!(isNil "KJ_height")) then { KJCB_H = KJ_height };
    KJCB_furnacePoint = [_k, "kjFurnacePoint", "zamerny"] call KJCB_fnCfgStr;
    KJCB_furnace = [0, [_k, "kjFurnaceFwd", 0.132 * KJCB_H] call KJCB_fnCfg, [_k, "kjFurnaceUp", 0.79 * KJCB_H] call KJCB_fnCfg];
    // [models] the hit regions of this creature's own model
    KJCB_furnaceR = [_k, "kjFurnaceR", KJCB_FURNACE_R * KJCB_H] call KJCB_fnCfg;
    KJCB_legsUp = [_k, "kjLegsUp", KJCB_LEGS_UP * KJCB_H] call KJCB_fnCfg;
    KJCB_headR = [_k, "kjHeadR", 0] call KJCB_fnCfg;
    KJCB_headPoint = [_k, "kjHeadPoint", ""] call KJCB_fnCfgStr;
    KJCB_head = [[_k, "kjHeadRight", 0] call KJCB_fnCfg, [_k, "kjHeadFwd", 0] call KJCB_fnCfg, [_k, "kjHeadUp", 0.92 * KJCB_H] call KJCB_fnCfg];
    logInfo format ["KJCOMBAT regions: legs under %1 m, head %2 m round %3 (%4), furnace %5 m round its point", KJCB_legsUp, KJCB_headR, KJCB_headPoint, KJCB_head, KJCB_furnaceR];
    // the engine: the integrated set says so; alone, the same question is asked here
    KJCB_native = (format ["%1", kjVersion]) != (format ["%1", KJCB_noSuchVariable_]);
    if (!(isNil "KJ_forceStock")) then { if (KJ_forceStock) then { KJCB_native = false } };
    if (!(isNil "KJ_coreReady")) then { KJCB_native = KJ_native };
    if (isNil "KJ_native") then { KJ_native = KJCB_native };
    // what one direct hit adds to the engine's damage value: hit^2 x 0.0729 / (armor x armorStructural x radius^2)
    _a = [_k, "armor", 700] call KJCB_fnCfg;
    _s = [_k, "armorStructural", 17] call KJCB_fnCfg;
    _r = [_k, "kjGeoRadius", KJCB_GEO * KJCB_H] call KJCB_fnCfg;
    KJCB_riseK = _a * _s * _r * _r / 0.0729;
    if (KJCB_native) then {
        // every hit reported, and no single blow may kill: his death is the script's to give
        kjSet [_k, "hitReport", 1];
        kjSet [_k, "damageCeiling", 0.5];
        _p = kjHits _k;
        _k call KJCB_fnFurnaceNow
    };
    // which height `drop` wants: said by the mission, else as the beam feature measured it, else by the engine
    KJCB_dropASL = KJCB_native;
    if (!(isNil "KJ_beamDropAGL")) then { KJCB_dropASL = !KJ_beamDropAGL };
    if ((format ["%1", KJCB_DROP_ASL]) != "") then { KJCB_dropASL = KJCB_DROP_ASL };
    if (KJCB_GLOW == "") then { if (KJCB_hasBeam) then { KJCB_GLOW = "\kjbeam\kjbeam_glow" } else { KJCB_GLOW = "cl_basic" } };
    KJCB_probe = "HeliHEmpty" createVehicle [70, 70, 0];
    _k addEventHandler ["Hit", { KJCB_hitEvent = [time, _this select 1] }];
    logInfo format ["KJCOMBAT setup: native %1, height %2, furnace %3 at %4, pool %5, rise factor %6 (armor %7 x %8, radius %9), glow %10", KJCB_native, KJCB_H, KJCB_furnacePoint, KJCB_furnace, KJCB_POOL, KJCB_riseK, _a, _s, _r, KJCB_GLOW];
    true
};

// ---- taking a hit ---------------------------------------------------------------------

// [unit, weight, [right, forward, up] or [], who, ammo, [x, y, zASL] or []] call KJCB_fnTake
// One hit of that weight (ammo "hit" scale). Counts it, puts its points in the
// waiting wound, remembers the heaviest of the tick, shows it.
KJCB_fnTake = {
    private ["_k", "_e", "_mp", "_zone", "_pts", "_real", "_i", "_row", "_am", "_side"];
    _k = _this select 0;
    _e = _this select 1;
    _mp = _this select 2;
    _am = _this select 4;
    if (_e < KJCB_NOTICE) then {
        KJ_combatStats set [0, (KJ_combatStats select 0) + 1]
    } else {
        _zone = ["unknown", KJCB_Z_BLIND, 4];
        if ((count _mp) == 3) then { _zone = _mp call KJCB_fnZone };
        _pts = ((_e / KJCB_REF) ^ KJCB_POW) * (_zone select 1);
        _real = _e >= KJCB_REAL;
        if (_real) then {
            KJCB_tReal = time;
            KJ_combatStats set [2, (KJ_combatStats select 2) + 1];
            KJ_combatByZone set [_zone select 2, (KJ_combatByZone select (_zone select 2)) + 1]
        } else {
            _pts = _pts * KJCB_STING;
            if (KJ_woundHealth <= KJCB_STING_FLOOR) then { _pts = 0 };
            KJ_combatStats set [1, (KJ_combatStats select 1) + 1]
        };
        KJCB_buf = KJCB_buf + _pts;
        if (KJCB_buf > KJCB_BUFFER) then {
            KJ_combatStats set [4, (KJ_combatStats select 4) + KJCB_buf - KJCB_BUFFER];
            KJCB_buf = KJCB_BUFFER
        };
        // which side of him takes it
        if ((count _mp) == 3) then {
            _side = 0;
            if ((_mp select 0) > 1) then { _side = 1 };
            if ((_mp select 0) < -1) then { _side = -1 };
            KJCB_sideSum = KJCB_sideSum + _side * _pts
        };
        // the book: by ammo
        _i = 0;
        _row = [];
        { if ((_x select 0) == _am) then { _row = _x } } forEach KJ_combatByAmmo;
        if ((count _row) == 0) then { _row = [_am, 0, 0]; KJ_combatByAmmo = KJ_combatByAmmo + [_row] };
        _row set [1, (_row select 1) + 1];
        _row set [2, (_row select 2) + _pts];
        if (_e > (KJCB_tickBest select 0)) then { KJCB_tickBest = [_e, _this select 3, _zone select 0, _am, _pts, _this select 5] };
        if (KJCB_LOG > 0) then { KJCB_LOG = KJCB_LOG - 1; logInfo format ["KJCOMBAT hit %1 weight %2 zone %3 at %4 points %5 real %6 from %7 health %8", _am, _e, _zone select 0, _mp, _pts, _real, _this select 3, KJ_woundHealth] };
        if (KJCB_FX and ((count (_this select 5)) == 3)) then { [_k, _this select 5, _e, _zone select 0] call KJCB_fnImpactFx }
    };
    true
};

// [unit, reports] call KJCB_fnReports : the engine's hit reports of one tick (kaiju binary).
// Each is [attacker, ammo class, [right, forward, up], rise in damage, time, [x, y, zASL]].
// A round that strikes him is reported twice in the same instant at the same
// place (the strike, then its blast); a blast beside him is reported once.
KJCB_fnReports = {
    private ["_k", "_hs", "_n", "_i", "_h", "_nx", "_am", "_p", "_a", "_e", "_twin", "_q", "_d", "_x1", "_n1", "_strike"];
    _k = _this select 0;
    _hs = _this select 1;
    _n = count _hs;
    _i = 0;
    if (_n > 0) then { _k call KJCB_fnFurnaceNow };
    while { _i < _n } do {
        _h = _hs select _i;
        _am = _h select 1;
        _p = _h select 2;
        _e = 0;
        if (KJCB_RAW > 0) then { KJCB_RAW = KJCB_RAW - 1; logInfo format ["KJCOMBAT raw %1 from %2 at %3 rise %4 t %5 world %6", _am, _h select 0, _p, _h select 3, _h select 4, _h select 5] };
        if (_am == "") then {
            // no projectile (scripted damage, a crash): the engine's own number says how hard
            _e = (_h select 3) call KJCB_fnFromRise
        } else {
            _a = _am call KJCB_fnAmmo;
            _twin = false;
            if ((_i + 1) < _n) then {
                _nx = _hs select (_i + 1);
                _q = _nx select 2;
                if (((_nx select 1) == _am) and ((abs ((_nx select 4) - (_h select 4))) < 0.02)) then {
                    if ((((_q select 0) - (_p select 0)) ^ 2 + ((_q select 1) - (_p select 1)) ^ 2 + ((_q select 2) - (_p select 2)) ^ 2) < 0.01) then { _twin = true }
                }
            };
            _d = (_a select 0) - (_a select 1);
            if (_twin) then {
                _e = _a select 0;
                _i = _i + 1;
                // the engine's own number for this strike says exactly how its damage value scales on this model
                if (((_h select 3) >= 0.0012) and (_d > 0)) then { KJCB_riseK = _d * _d / (_h select 3) }
            } else {
                // one report only. A strike whose blast did not reach his origin (a hit high on him) looks like
                // this too: it is told from a blast beside him by the engine's number, or, for a round too small
                // to move that number, by lying on his body.
                _x1 = _d * _d / KJCB_riseK;
                _n1 = [_p, _a select 2] call KJCB_fnNear;
                _strike = false;
                if (_x1 >= 0.0012) then { if ((abs ((_h select 3) - _x1)) < (0.3 * _x1)) then { _strike = true } } else { if (([_p, 0.5] call KJCB_fnNear) >= 1) then { _strike = true } };
                if (_strike) then { _e = _a select 0 } else { _e = (_a select 1) * _n1 }
            }
        };
        [_k, _e, _p, _h select 0, _am, _h select 5] call KJCB_fnTake;
        _i = _i + 1
    };
    true
};

// [unit, "ammo class", [right, forward, up] or [], who] call KJCB_fnScriptHit
// A direct hit by that round from a script (a mission's own ordnance, a test).
KJCB_fnScriptHit = {
    private ["_k", "_mp", "_wp"];
    _k = _this select 0;
    _mp = _this select 2;
    _wp = [];
    if ((count _mp) == 3) then { _wp = [_k, _mp] call KJCB_fnToWorld; _wp set [2, (_wp select 2) + ((getPosASL _k) select 2)] };
    [_k, ((_this select 1) call KJCB_fnAmmo) select 0, _mp, _this select 3, _this select 1, _wp] call KJCB_fnTake
};

// [unit, points, who] call KJCB_fnWound : that many wound points straight into the wound,
// past the rule that a wound opens slowly. For a mission's own events and for tests
// (a scripted bomb, a fall, "start him half dead"). 10000 kills.
KJCB_fnWound = {
    private ["_p"];
    _p = _this select 1;
    KJCB_perm = KJCB_perm + KJCB_SCAR * _p;
    KJCB_temp = KJCB_temp + (1 - KJCB_SCAR) * _p;
    KJCB_tReal = time;
    KJ_combatStats set [3, (KJ_combatStats select 3) + _p];
    if (_p > 0) then { KJCB_tickNote = [_p, _this select 2] };
    true
};

// ---- the tick -------------------------------------------------------------------------

// [unit, seconds since the last tick] call KJCB_fnTick : true while he lives
KJCB_fnTick = {
    private ["_k", "_dt", "_hs", "_d", "_who", "_open", "_h", "_s", "_was", "_b", "_wp", "_kp", "_abs", "_rel", "_sev", "_mend", "_gap"];
    _k = _this select 0;
    _dt = _this select 1;
    _hs = [];
    // ---- what hit him since the last tick
    if (KJCB_native) then {
        _hs = kjHits _k;
        [_k, _hs] call KJCB_fnReports
    } else {
        if (!(alive _k)) then {
            // one blow killed the engine's Man: undone, and taken as the heaviest thing there is
            _k setDammage 0;
            KJCB_revived = KJCB_revived + 1;
            [_k, 5000, [], objNull, "", []] call KJCB_fnTake
        } else {
            _d = getDammage _k;
            if (_d >= 0.0009) then {
                _who = objNull;
                if ((time - (KJCB_hitEvent select 0)) < 0.6) then { _who = KJCB_hitEvent select 1 };
                if (isNull _who) then { if (!(isNil "KJC_lastShot")) then { if ((time - (KJC_lastShot select 0)) <= 4) then { _who = KJC_lastShot select 1 } } };
                [_k, KJCB_STOCK_UP * (_d call KJCB_fnFromRise), [], _who, "", []] call KJCB_fnTake
            }
        }
    };
    // the engine's own damage is only the messenger
    if ((getDammage _k) > 0) then { _k setDammage 0 };

    // ---- the wound opens, or mends
    _open = KJCB_RATE * _dt;
    if (_open > KJCB_buf) then { _open = KJCB_buf };
    if (_open > 0) then {
        KJCB_buf = KJCB_buf - _open;
        KJCB_perm = KJCB_perm + KJCB_SCAR * _open;
        KJCB_temp = KJCB_temp + (1 - KJCB_SCAR) * _open;
        KJ_combatStats set [3, (KJ_combatStats select 3) + _open]
    };
    KJ_woundMending = false;
    if ((KJCB_buf <= 0) and ((time - KJCB_tReal) > KJCB_MEND_WAIT) and (KJCB_temp > 0)) then {
        _mend = KJCB_MEND * _dt;
        if (!(isNil "KJ_state")) then { if (KJ_state == "dormant") then { _mend = _mend * KJCB_MEND_DORMANT } };
        KJCB_temp = KJCB_temp - _mend;
        if (KJCB_temp < 0) then { KJCB_temp = 0 };
        KJ_woundMending = true
    };
    if (KJCB_perm > KJCB_POOL) then { KJCB_perm = KJCB_POOL };
    if ((KJCB_perm + KJCB_temp) > KJCB_POOL) then { KJCB_temp = KJCB_POOL - KJCB_perm };
    _h = 1 - (KJCB_perm + KJCB_temp) / KJCB_POOL;
    if (_h < 0.0001) then { _h = 0 };
    KJ_woundHealth = _h;
    KJ_woundScar = KJCB_perm / KJCB_POOL;
    KJ_woundSide = 0;
    if (KJCB_sideSum > 3) then { KJ_woundSide = 1 };
    if (KJCB_sideSum < -3) then { KJ_woundSide = -1 };

    // ---- the stage
    _was = KJ_woundStage;
    _s = 0;
    { if (_h <= _x) then { _s = _s + 1 } } forEach KJCB_STAGES;
    if (_s < _was) then {
        _s = 0;
        { if (_h <= (_x + KJCB_STAGE_BACK)) then { _s = _s + 1 } } forEach KJCB_STAGES;
        if (_s > _was) then { _s = _was }
    };
    if (_s != _was) then {
        KJ_woundStage = _s;
        KJ_woundLog = KJ_woundLog + [[time, _s, _h]];
        logInfo format ["KJCOMBAT stage %1 -> %2 at %3: health %4, scar %5, points taken %6, wasted %7", _was, _s, time, _h, KJ_woundScar, KJ_combatStats select 3, KJ_combatStats select 4];
        if ((_s > _was) and (_s >= 2) and (_h > 0)) then { [_k] call KJCB_fnPainRoar }
    };

    KJ_woundPace = [1, 0.92, 0.78, 0.6, 0.45, 0] select KJ_woundStage;

    // ---- the signal, for the brain and the animations
    _b = KJCB_tickBest;
    // a wound a script gave him straight (KJCB_fnWound) is felt as a hard hit from whoever was named
    if (((KJCB_tickNote select 0) > 0) and ((_b select 0) < KJCB_HARD)) then { _b = [KJCB_HARD, KJCB_tickNote select 1, "unknown", "", KJCB_tickNote select 0, []] };
    KJCB_tickNote = [0, objNull];
    KJCB_tickBest = [0, objNull, "", "", 0, []];
    if (((_b select 0) >= KJCB_REAL) or (((_b select 0) >= KJCB_NOTICE) and ((time - KJCB_tSignal) >= KJCB_STING_GAP))) then {
        KJCB_tSignal = time;
        _who = _b select 1;
        _abs = -1;
        _rel = -1;
        // never himself, and never a shooter on his own spot: the blast of something he has struck is
        // credited to HIM by the engine, and "0 atan2 0" is a script error that ends a test run
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
        _sev = (_b select 0) / KJCB_HARD;
        if (_sev > 1) then { _sev = 1 };
        KJ_hurtCount = KJ_hurtCount + 1;
        KJ_hurt = [time, _abs, _rel, _sev, _who];
        KJ_combatLast = [time, _b select 3, _b select 2, _b select 0, _b select 4, _who];
        // alone (no brain), the flinch is played here; with the brain it is the brain's
        _gap = KJCB_FLINCH_GAP / (1 + KJ_woundStage);
        if (KJCB_ACT and (not KJCB_hasBrain) and (_h > 0) and (_sev >= 0.5) and ((time - KJCB_tFlinch) >= _gap)) then {
            KJCB_tFlinch = time;
            KJ_combatStats set [5, (KJ_combatStats select 5) + 1];
            _k playMove "DedFlinch"
        }
    };

    // ---- badly hurt and still being hit: he turns for the sea
    KJ_woundRetreat = (_h > 0) and (KJ_woundStage >= KJCB_RETREAT_STAGE) and ((time - KJCB_tReal) < 90);
    if (KJ_woundRetreat and KJCB_ACT and ((time - KJCB_tPull) > 20)) then {
        KJCB_tPull = time;
        [_k] call KJCB_fnRetreat
    };
    _h > 0
};

// [unit] call KJCB_fnPainRoar : he roars in pain, when he is free to
KJCB_fnPainRoar = {
    private ["_k", "_free"];
    _k = _this select 0;
    if (KJCB_ACT and ((time - KJCB_tRoar) >= KJCB_ROAR_GAP)) then {
        _free = true;
        if (KJCB_hasBrain) then {
            // the brain owns his body: it is asked, and only when he is doing nothing that a roar would break
            _free = false;
            if (!(isNil "KJ_state")) then { if (KJ_state in ["idle", "wander", "drawn"]) then { KJ_brainOrder = ["roar"]; _free = true } }
        } else {
            if (KJCB_hasRoar) then { [_k] exec "kj\roar\roar.sqs" } else { _k playMove "DedRoar" }
        };
        if (_free) then {
            KJCB_tRoar = time;
            KJ_combatStats set [6, (KJ_combatStats select 6) + 1];
            logInfo format ["KJCOMBAT roar of pain at %1 (stage %2, brain %3)", time, KJ_woundStage, KJCB_hasBrain]
        }
    };
    true
};

// [x, y] call KJCB_fnGroundAt : ground (or sea surface) height above the sea there
KJCB_fnGroundAt = {
    private ["_g"];
    _g = 0;
    if (!(isNull KJCB_probe)) then {
        KJCB_probe setPos [_this select 0, _this select 1, 0];
        _g = (getPosASL KJCB_probe) select 2
    };
    _g
};

// [x, y] call KJCB_fnWet : true when that point is sea
KJCB_fnWet = {
    private ["_w"];
    _w = false;
    if (!(isNull KJCB_probe)) then {
        KJCB_probe setPos [_this select 0, _this select 1, 0];
        _w = ((getPosASL KJCB_probe) select 2) <= 2.95
    };
    _w
};

// [unit] call KJCB_fnSea : the nearest point of the sea, or [] (looked for once)
KJCB_fnSea = {
    private ["_kp", "_r", "_a", "_p", "_found"];
    if ((count KJ_woundRetreatPos) == 0) then {
        _kp = getPos (_this select 0);
        _found = [];
        _r = 100;
        while { ((count _found) == 0) and (_r <= KJCB_RETREAT_R) } do {
            _a = 0;
            while { ((count _found) == 0) and (_a < 360) } do {
                _p = [(_kp select 0) + _r * sin _a, (_kp select 1) + _r * cos _a];
                if (_p call KJCB_fnWet) then { _found = [_p select 0, _p select 1, 0] };
                _a = _a + 20
            };
            _r = _r + 100
        };
        KJ_woundRetreatPos = _found;
        logInfo format ["KJCOMBAT the sea: %1 (from %2)", _found, _kp]
    };
    KJ_woundRetreatPos
};

// [unit] call KJCB_fnRetreat : badly hurt, he is drawn to the sea (through the brain, when it runs)
KJCB_fnRetreat = {
    private ["_p"];
    _p = _this call KJCB_fnSea;
    if (((count _p) == 3) and KJCB_hasBrain) then {
        if (!(isNil "KJB_fnDraw")) then { [_p, KJCB_RETREAT_PULL] call KJB_fnDraw }
    };
    true
};

// ---- how it shows ------------------------------------------------------------------------

// the viewer: the player, or a cutscene camera a mission names in KJ_shakeCam
KJCB_fnEye = {
    private ["_e"];
    _e = player;
    if (!(isNil "KJ_shakeCam")) then { if (!(isNull KJ_shakeCam)) then { _e = KJ_shakeCam } };
    _e
};

// unit call KJCB_fnGz : sets KJCB_gz, what `drop` needs added to a height above the ground he stands on
KJCB_fnGz = {
    KJCB_gz = 0;
    if (KJCB_dropASL) then { KJCB_gz = (getPosASL _this) select 2 };
    KJCB_gz
};

// [unit, [x, y, zASL]] call KJCB_fnDropPos : the same point as `drop` wants it
KJCB_fnDropPos = {
    private ["_p", "_z"];
    _p = _this select 1;
    _z = (_p select 2) - ((getPosASL (_this select 0)) select 2);
    if (_z < 0.3) then { _z = 0.3 };
    [_p select 0, _p select 1, _z + ((_this select 0) call KJCB_fnGz)]
};

// [unit, [x, y, zASL], weight, zone] call KJCB_fnImpactFx : what a hit looks like where it lands
KJCB_fnImpactFx = {
    private ["_k", "_p", "_e", "_n", "_i", "_s", "_hot", "_g", "_kp", "_ox", "_oy", "_ol"];
    _k = _this select 0;
    if (((call KJCB_fnEye) distance _k) < KJCB_FX_R) then {
        _p = [_k, _this select 1] call KJCB_fnDropPos;
        // a little way out from his body, or half of every sprite is inside him
        _kp = getPos _k;
        _ox = (_p select 0) - (_kp select 0);
        _oy = (_p select 1) - (_kp select 1);
        _ol = sqrt (_ox * _ox + _oy * _oy);
        if (_ol < 0.5) then { _ox = sin (getDir _k); _oy = cos (getDir _k); _ol = 1 };
        _ox = _ox / _ol;
        _oy = _oy / _ol;
        _p = [(_p select 0) + 2 * _ox, (_p select 1) + 2 * _oy, _p select 2];
        _e = _this select 2;
        _hot = (_this select 3) in ["furnace", "open furnace"];
        if (_e < KJCB_REAL) then {
            // a sting: one small grey puff
            drop ["cl_basic", "", "Billboard", 1, 1.4, _p, [_ox + 1 - random 2, _oy + 1 - random 2, 0.5], 0, 1.275, 1, 0.05, [1, 3.5], [[0.45, 0.42, 0.4, 0.6], [0.45, 0.42, 0.4, 0]], [0], 0.5, 0.4, "", ""]
        } else {
            _s = 0.8 + _e / 600;
            if (_s > 4) then { _s = 4 };
            _n = 4 + _s * 2;
            // torn hide and smoke, darker than gun smoke, thrown out from him
            _i = 0;
            while { _i < _n } do {
                _g = 0.1 + random 0.1;
                drop ["cl_basic", "", "Billboard", 1, 3 + random 2.5, [(_p select 0) + _s - random (2 * _s), (_p select 1) + _s - random (2 * _s), (_p select 2) + _s - random (2 * _s)], [5 * _ox + 2 - random 4, 5 * _oy + 2 - random 4, 0.5 + random 2], 0, 1.3, 1, 0.08, [2.5 * _s, 6 * _s, 9 * _s], [[_g, _g * 0.9, _g * 0.85, 0.8], [_g, _g * 0.9, _g * 0.85, 0.55], [_g, _g, _g, 0]], [0], 0.4, 0.6, "", ""];
                _i = _i + 1
            };
            // the flash
            drop [KJCB_GLOW, "", "Billboard", 1, 0.3, _p, [0, 0, 0], 0, 1.275, 1, 0, [6 * _s, 1.5 * _s], [[1, 0.9, 0.7, 1], [1, 0.5, 0.1, 0]], [0], 1, 0, "", ""];
            if (_hot) then {
                // out of the furnace: embers thrown out and falling
                _i = 0;
                while { _i < (3 * _n) } do {
                    drop [KJCB_GLOW, "", "Billboard", 1, 2 + random 2, [(_p select 0) + 1 - random 2, (_p select 1) + 1 - random 2, (_p select 2) + 1 - random 2], [9 * _ox + 7 - random 14, 9 * _oy + 7 - random 14, 6 - random 8], 0, 2.2, 1, 0.02, [1.6, 0.5], [[1, 0.8, 0.4, 1], [1, 0.4, 0.05, 0.9], [0.7, 0.12, 0.02, 0]], [0], 0.3, 1.2, "", ""];
                    _i = _i + 1
                }
            }
        }
    };
    true
};

// [[x, y], radius, how many, size, rise, life, grey, alpha] call KJCB_fnDust : a ring of rolling dust on the ground (after KJCB_fnGz)
KJCB_fnDust = {
    private ["_c", "_r", "_n", "_i", "_a", "_s", "_g", "_v"];
    _c = _this select 0;
    _r = _this select 1;
    _n = _this select 2;
    _s = _this select 3;
    _g = _this select 6;
    _i = 0;
    while { _i < _n } do {
        _a = random 360;
        _v = 2 + random 6;
        drop ["cl_basic", "", "Billboard", 1, (_this select 5) * (0.7 + random 0.6), [(_c select 0) + _r * (random 1) * sin _a, (_c select 1) + _r * (random 1) * cos _a, KJCB_gz + 1 + random 3], [_v * sin _a, _v * cos _a, (_this select 4) * (0.3 + random 0.7)], 0, 1.3, 1, 0.12, [_s, _s * 2.2, _s * 3.2], [[_g, _g * 0.95, _g * 0.84, 0], [_g, _g * 0.95, _g * 0.84, _this select 7], [_g, _g * 0.95, _g * 0.84, (_this select 7) * 0.6], [_g, _g * 0.95, _g * 0.84, 0]], [0], 0.3, 0.4, "", ""];
        _i = _i + 1
    };
    true
};

// [unit, 0..2] call KJCB_fnGlow : the furnace painted on his body, 0 dark, 1 the painted colour, 2 white
// hot (the engine's "glow" trait, kaiju binary of version 2). An older kaiju binary answers false to
// the unknown trait and nothing changes; on the stock binary the block never runs.
KJCB_glowNow = -1;
KJCB_fnGlow = {
    KJCB_glowNow = _this select 1;
    if (KJCB_native) then { kjSet [_this select 0, "glow", _this select 1] }
};

// [[x, y, z above ground], peak, seconds, full radius, zero radius] call KJCB_fnShake : the ground shakes
KJCB_fnShake = {
    private ["_p", "_d", "_s"];
    _p = _this select 0;
    if (KJCB_native) then {
        kjShake [_p, _this select 1, _this select 3, _this select 4, _this select 2, 4.5]
    } else {
        if (KJCB_hasShake) then {
            _d = [call KJCB_fnEye, _p] call KJCB_fnD2;
            _s = 0;
            if (_d < (_this select 4)) then { _s = ((_this select 4) - _d) / ((_this select 4) - (_this select 3)) };
            if (_s > 1) then { _s = 1 };
            _s = _s * _s * (_this select 1);
            if (_s > 0.01) then { [_s, _this select 2, 0] exec "kj\roar\shake.sqs" }
        }
    };
    true
};

// [victim, killer] call KJCB_fnKill : dead or destroyed, credited to him where the engine can
KJCB_fnKill = {
    private ["_o"];
    _o = _this select 0;
    if (KJCB_native) then { kjDamage [_o, _this select 1, 0, 200000, 8] };
    if ((getDammage _o) < 1) then { _o setDammage 1 };
    true
};

// [object, [vx, vy, vz]] call KJCB_fnThrow
KJCB_fnThrow = {
    if (KJCB_native) then { kjImpulse [_this select 0, _this select 1] } else { if (("Man" countType [_this select 0]) == 0) then { (_this select 0) setVelocity (_this select 1) } };
    true
};

KJCB_ready = true;
