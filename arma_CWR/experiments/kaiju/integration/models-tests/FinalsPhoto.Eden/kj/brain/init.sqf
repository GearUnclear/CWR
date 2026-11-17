// ============================================================================
//  kj\brain\init.sqf  -  Dedushka BRAIN: the tuning block and the functions.
//  Loaded once by start.sqs:  call compile preprocessFile "kj\brain\init.sqf"
//
//  The brain is one creature's (globals, like the other features). start.sqs
//  is only the clock: it calls KJB_fnTick every KJB_TICK seconds. Nothing in
//  here waits; every timed thing is a step of a sequence (KJB_seq).
//
//  How he moves (proved in game, see kaiju-notes\features\brain\NOTES.md):
//  the AI never walks him. A walk is a run of playMove cycles of the walk
//  clip; the engine moves the body by the clip's own step along his facing,
//  so the feet never slide and nothing plans a path, and he goes straight
//  through whatever stands there. The heading is turned by setDir, a little
//  every tick. A turn on the spot is the turn clip with setDir at the clip's
//  own rate. He can only stop at the end of a stride.
// ============================================================================

// =============================== TUNING ====================================
// Distances are in HEIGHTS (his own height, read off the unit), times in seconds.
if (isNil "KJB_TICK") then { KJB_TICK = 0.05 };              // the clock: mover and sequences
if (isNil "KJB_SENSE") then { KJB_SENSE = 2 };               // how often he takes in the world
if (isNil "KJB_EARS") then { KJB_EARS = 8 };                 // how often new shooters are listened to
if (isNil "KJB_LAMPS") then { KJB_LAMPS = 15 };              // how often he looks for lights (night only)
// ---- how he moves
if (isNil "KJB_STEER") then { KJB_STEER = 3 };               // degrees a second his heading swings while walking
if (isNil "KJB_SPOT_MIN") then { KJB_SPOT_MIN = 32 };        // off by more than this he stops and turns on the spot
if (isNil "KJB_SPOT_RATE") then { KJB_SPOT_RATE = 7 };       // degrees a second on the spot when the addon has no turn clip
if (isNil "KJB_ARRIVE") then { KJB_ARRIVE = 0.6 };           // strides: nearer than this he has arrived
if (isNil "KJB_PAUSE_TURN") then { KJB_PAUSE_TURN = 2.2 };   // he stands this long after a turn before the first step
if (isNil "KJB_PAUSE_STOP") then { KJB_PAUSE_STOP = 1.6 };   // the walk clip blending out into the stand
if (isNil "KJB_SEA_ASL") then { KJB_SEA_ASL = 2.95 };        // ground at or below this is sea: he does not step there
if (isNil "KJB_LOOK") then { KJB_LOOK = 1.2 };               // strides ahead that must be dry before he takes the next one
if (isNil "KJB_WADE_MAX") then { KJB_WADE_MAX = 0.42 };      // [integration] when he may wade (KJB_wade): the deepest water, in heights, he takes another stride into
if (isNil "KJB_BAY_WADE") then { KJB_BAY_WADE = 0.12 };      // [finishing] the retreat to the sea ends where the water is this deep, in heights (his shins): there he turns at bay, so a body is never lost under deep water
if (isNil "KJB_START_FIX") then { KJB_START_FIX = false };   // [integration] OFF: an attempt to hold the standing foot while the walk blends in (KJB_fnStartFix). Measured: it makes the drag worse (a setPos on a walking Man costs it the step it was taking). Kept for the record; the cure is the engine's (integration NOTES, engine request 2)
if (isNil "KJB_START_FIX_MODE") then { KJB_START_FIX_MODE = "auto" }; // [integration] "auto": by the foot on the kaiju binary, by the blend arithmetic on the stock one; "blend": the arithmetic on both
// ---- dormant
if (isNil "KJB_DORMANT_AFTER") then { KJB_DORMANT_AFTER = 150 }; // nothing has drawn him for this long: he goes dormant
if (isNil "KJB_DORMANT_MAX") then { KJB_DORMANT_MAX = 420 };     // he wakes by himself after this long
if (isNil "KJB_WAKE_R") then { KJB_WAKE_R = 12 };                // interest nearer than this wakes him
if (isNil "KJB_WAKE_SECS") then { KJB_WAKE_SECS = 7 };           // the rise out of dormancy, before the roar
if (isNil "KJB_WAKE_ROAR") then { KJB_WAKE_ROAR = true };        // he roars when he has risen
// ---- wander
if (isNil "KJB_IDLE_MIN") then { KJB_IDLE_MIN = 9 };         // the heavy pause between two walks, least
if (isNil "KJB_IDLE_RAND") then { KJB_IDLE_RAND = 12 };      // and up to this much more
if (isNil "KJB_LEG_MIN") then { KJB_LEG_MIN = 3 };           // a wander leg, strides, least
if (isNil "KJB_LEG_RAND") then { KJB_LEG_RAND = 4 };         // and up to this many more
if (isNil "KJB_LEG_SWING") then { KJB_LEG_SWING = 30 };      // a leg leaves his present heading by at most this
// ---- what draws him (interest) and what earns the beam (anger)
if (isNil "KJB_HALF_LIFE") then { KJB_HALF_LIFE = 60 };      // interest and anger halve in this long
if (isNil "KJB_CLUSTER") then { KJB_CLUSTER = 2.5 };         // things nearer than this to each other are one place
if (isNil "KJB_HEAR") then { KJB_HEAR = 25 };                // gunfire is heard this far
if (isNil "KJB_SEE") then { KJB_SEE = 15 };                  // armour, aircraft and engines are noticed this far
if (isNil "KJB_LIGHT") then { KJB_LIGHT = 12 };              // lit lamps and fires are noticed this far, at night
if (isNil "KJB_DRAW_MIN") then { KJB_DRAW_MIN = 1 };         // interest below this does not move him
if (isNil "KJB_DRAW_STICK") then { KJB_DRAW_STICK = 1.6 };   // a new place must beat the one he walks to by this factor
if (isNil "KJB_VISITED") then { KJB_VISITED = 120 };         // a place he has stood on is dull for this long
if (isNil "KJB_W_HIT") then { KJB_W_HIT = 4 };               // a hard hit: interest in where it came from
if (isNil "KJB_A_HIT") then { KJB_A_HIT = 2.5 };             // and anger
if (isNil "KJB_W_GUN") then { KJB_W_GUN = [0.08, 1, 1] };    // a shot heard: interest by shooter kind [man, armour, aircraft]
if (isNil "KJB_A_GUN") then { KJB_A_GUN = [0.04, 1, 1.5] };  // and anger
if (isNil "KJB_W_ARMOUR") then { KJB_W_ARMOUR = 0.08 };      // per sense, per crewed tank, per other tank beside it (massed armour)
if (isNil "KJB_W_ENGINE") then { KJB_W_ENGINE = 0.04 };      // per sense, a running engine (interest only)
if (isNil "KJB_W_AIR") then { KJB_W_AIR = 0.06 };            // per sense, an aircraft circling close (interest only)
if (isNil "KJB_W_LAMP") then { KJB_W_LAMP = 0.12 };          // per look, per lit lamp (interest only)
if (isNil "KJB_NIGHT") then { KJB_NIGHT = [20.5, 4.5] };     // lights draw him between these hours
// ---- the beam
if (isNil "KJB_BEAM_ANGER") then { KJB_BEAM_ANGER = 10 };    // anger at one place that earns the beam
if (isNil "KJB_BEAM_NEAR") then { KJB_BEAM_NEAR = 2 };       // nearer than this he walks on it instead
if (isNil "KJB_BEAM_FAR") then { KJB_BEAM_FAR = 15 };        // farther than this he walks closer first
if (isNil "KJB_SPENT") then { KJB_SPENT = 75 };              // he is spent this long after a beam (the beam feature may say longer)
if (isNil "KJB_BEAM_STUB") then { KJB_BEAM_STUB = 16 };      // without the beam feature: he stands tall this long
// ---- roar
if (isNil "KJB_ROAR_GAP") then { KJB_ROAR_GAP = 75 };        // least time between two roars of his own choosing
if (isNil "KJB_ROAR_SECS") then { KJB_ROAR_SECS = 9.5 };     // a roar holds him this long
// ---- being hit
if (isNil "KJB_HIT_HARD") then { KJB_HIT_HARD = 0.5 };       // severity (0..1) from which a hit stops him
if (isNil "KJB_HIT_STARE") then { KJB_HIT_STARE = 3.5 };     // he stands this long before he turns to it
if (isNil "KJB_HIT_GAP") then { KJB_HIT_GAP = 14 };          // he does not stop for a second hit sooner than this
if (isNil "KJB_HIT_REACT") then { KJB_HIT_REACT = true };    // [integration] a hard hit makes him shudder and turn even while a director drives him (KJ_brainAuto false)
// ---- stomp and swat (the crush feature's blows)
if (isNil "KJB_STOMP_R") then { KJB_STOMP_R = 0.4 };         // a crewed tank nearer than this in front of him is stomped ([integration] the reach of the stomping foot of the clip)
if (isNil "KJB_SWAT_R") then { KJB_SWAT_R = 0.85 };          // an aircraft nearer than this to his chest is swatted
if (isNil "KJB_BLOW_SECS") then { KJB_BLOW_SECS = 9.5 };     // a blow holds him this long
if (isNil "KJB_BLOW_GAP") then { KJB_BLOW_GAP = 20 };        // least time between two blows
// ============================ END OF TUNING ================================

// ---- public state ----------------------------------------------------------
KJ_state = "";                 // dormant, idle, wander, drawn, roar, beam, spent, flinch, stomp, swat, dead
KJ_brainPhase = "";            // what he is doing inside the state (plain word, for logs and cameras)
KJ_brainOn = false;            // true while the brain runs; set false to end it
if (isNil "KJ_brainAuto") then { KJ_brainAuto = true };       // false: he only does what KJ_brainOrder says
if (isNil "KJ_brainOrder") then { KJ_brainOrder = [] };       // see order.sqs
if (isNil "KJ_brainStartOthers") then { KJ_brainStartOthers = true }; // start the crush and roar lifetime loops
if (isNil "KJ_brainBeamAnim") then { KJ_brainBeamAnim = true };       // play DedBeamRise / DedBeamLower round the beam
if (isNil "KJ_brainAfterHit") then { KJ_brainAfterHit = [] }; // an order to take when the next hit reaction is over (staging)
if (isNil "KJ_station") then { KJ_station = [] };             // where he was made: he drifts there when nothing draws him
KJ_brainLog = [];              // [[time, state, why], ...]
KJ_brainTrack = [0, 0, 0, 0];  // [strides walked, turns on the spot, metres walked, times blocked by water]
KJ_brainDraw = [];             // [x, y] he is walking to, [] when none
KJ_interest = [];              // [[x, y, interest, anger, last time, object], ...]

// ---- private state ---------------------------------------------------------
KJB_k = objNull;
KJB_H = 40;
KJB_seq = [];
KJB_seqI = 0;
KJB_seqIn = false;
KJB_seqT = 0;
KJB_seqThen = "idle";
KJB_stateT = 0;
KJB_mv = "stand";              // stand, turn, walk, settle
KJB_mvLast = 0;
KJB_goal = [];
KJB_goalLegs = 0;
KJB_halt = false;
KJB_face = -1;
KJB_faced = true;
KJB_arrived = false;
KJB_blocked = false;
KJB_turnRate = 0;
KJB_turnEnd = 0;
KJB_turnTo = 0;
KJB_settleUntil = 0;
KJB_legStart = [0, 0, 0];
KJB_cyc = 0;
KJB_decided = 0;
KJB_lastStride = false;
KJB_walkT = 0;
KJB_stride = 17.6;             // metres a walk cycle covers (two footfalls)
KJB_strideSecs = 6;            // seconds a cycle takes under playMove
KJB_strideLoss = 0.6;          // metres the first cycle loses to the blend out of the stand
KJB_turnSecs = 6;
KJB_turnDeg = 68.75;           // degrees a turn clip cycle steps round
KJB_mIdle = "DedStand";
KJB_mWalk = "DedWalk";
KJB_mTurnL = "";
KJB_mTurnR = "";
KJB_mRoar = "";
KJB_mFlinch = "";
KJB_mDormant = "";
KJB_mRise = "";
KJB_mLower = "";
KJB_hasRoar = false;
KJB_hasBeam = false;
KJB_hasCrush = false;
KJB_probe = objNull;
KJB_tSense = 0;
KJB_tEars = 0;
KJB_tLamps = 0;
KJB_tDecay = 0;
KJB_hurtSeen = 0;
KJB_lastHit = -1000;
KJB_lastRoar = -1000;
KJB_lastBlow = -1000;
KJB_lastDrawn = 0;
KJB_idleUntil = 0;
KJB_dormantSince = 0;
KJB_spentUntil = 0;
KJB_beenTo = [];              // [[x, y, until], ...]
KJB_drawI = -1;
KJB_beamTgt = [];
KJB_beamWas = "";
KJB_lastShot = [-1000, objNull];
if (isNil "KJB_eared") then { KJB_eared = [] }; // everything that already carries the brain's "Fired" handler (kept over a restart)
KJB_hitFrom = [];
KJB_blowTgt = objNull;
KJB_heldUntil = 0;
KJB_dormantSecs = 16;
KJB_deadSince = -1;
KJB_walkNow = "";               // [integration] the walk move of the leg he is on (the stride, or the slow walk when wounded)
KJB_paceNow = 1;
// [integration]
KJB_wade = false;              // he may walk into water (the engine's wadeDepth trait, or the mission's KJ_brainWade)
KJ_brainDepth = 0;             // metres of water he stood in at his last stride decision
KJB_blendSecs = 1;             // the walk blends in over this long
KJB_walkSpeed = 2.3;           // m/s at full rate
KJB_plantPt = "stopaL";        // the foot he stands on while the first stride starts
KJB_fixUntil = 0;
KJB_fixT0 = 0;
KJB_fixDone = 0;
KJB_fixFoot = [];
KJ_brainStartFix = [0, 0, 0];  // [starts fixed, metres put back at the last start, "foot" or "blend"]
KJB_stompAng = 0;              // degrees to the right of dead ahead where the stomping foot of the clip lands
KJB_stompSecs = KJB_BLOW_SECS;
KJB_swatSecs = KJB_BLOW_SECS;

// ---- small helpers ---------------------------------------------------------

// [state, why] call KJB_fnLog
KJB_fnLog = {
    if ((count KJ_brainLog) > 300) then { KJ_brainLog = [] };
    KJ_brainLog = KJ_brainLog + [[time, _this select 0, _this select 1]];
    logInfo format ["KJBRAIN t %1 %2: %3", time, _this select 0, _this select 1]
};

// [a, b] call KJB_fnAng : a minus b, folded into -180..180
KJB_fnAng = {
    private ["_d"];
    _d = ((_this select 0) - (_this select 1)) mod 360;
    if (_d > 180) then { _d = _d - 360 };
    if (_d < -180) then { _d = _d + 360 };
    _d
};

// [from, to] call KJB_fnBear : compass bearing 0..360
// [integration] 0 when the two are the same spot ("0 atan2 0" is a script error in this engine)
KJB_fnBear = {
    private ["_b", "_dx", "_dy"];
    _dx = ((_this select 1) select 0) - ((_this select 0) select 0);
    _dy = ((_this select 1) select 1) - ((_this select 0) select 1);
    _b = 0;
    if (((abs _dx) + (abs _dy)) > 0.001) then { _b = _dx atan2 _dy };
    if (_b < 0) then { _b = _b + 360 };
    _b
};

// [a, b] call KJB_fnDist : flat distance
KJB_fnDist = {
    sqrt ((((_this select 0) select 0) - ((_this select 1) select 0)) ^ 2 + (((_this select 0) select 1) - ((_this select 1) select 1)) ^ 2)
};

// number call KJB_fnRound
KJB_fnRound = {
    private ["_n"];
    _n = _this + 0.5;
    _n - (_n mod 1)
};

// pos call KJB_fnDry : true when the ground there is above the sea
KJB_fnDry = {
    private ["_ok"];
    _ok = true;
    if (!(isNull KJB_probe)) then {
        KJB_probe setPos [_this select 0, _this select 1, 0];
        _ok = ((getPosASL KJB_probe) select 2) > KJB_SEA_ASL
    };
    _ok
};

// [from, bearing, metres] call KJB_fnAhead : the point that far along the bearing
KJB_fnAhead = {
    [((_this select 0) select 0) + (_this select 2) * sin (_this select 1), ((_this select 0) select 1) + (_this select 2) * cos (_this select 1), 0]
};

// [from, bearing, metres] call KJB_fnDryLine : true when every half stride of that line is dry
KJB_fnDryLine = {
    private ["_ok", "_d", "_n"];
    _ok = true;
    _d = KJB_stride * 0.5;
    _n = 0;
    while { _ok and (_d <= (_this select 2)) and (_n < 40) } do {
        if (!([_this select 0, _this select 1, _d] call KJB_fnAhead call KJB_fnDry)) then { _ok = false };
        _d = _d + KJB_stride * 0.5;
        _n = _n + 1
    };
    _ok
};

// [integration] call KJB_fnDepth : metres of water he stands in now (0 on land). The probe
// floats on the sea, so it reads the surface; his own position reads the bed.
KJB_fnDepth = {
    private ["_d", "_s"];
    _d = 0;
    if (!(isNull KJB_probe)) then {
        KJB_probe setPos [(getPos KJB_k) select 0, (getPos KJB_k) select 1, 0];
        _s = (getPosASL KJB_probe) select 2;
        if (_s <= KJB_SEA_ASL) then { _d = _s - ((getPosASL KJB_k) select 2) }
    };
    if (_d < 0) then { _d = 0 };
    _d
};

// [integration] [from, bearing, metres] call KJB_fnPassLine : may he take the stride along that
// line? Dry land always. When he may wade, water too, for as long as the water he stands in is
// shallower than KJB_WADE_MAX heights: no script can read the sea bed ahead, so he feels his way,
// one stride at a time.
KJB_fnPassLine = {
    private ["_ok"];
    _ok = _this call KJB_fnDryLine;
    if ((!_ok) and KJB_wade) then {
        KJ_brainDepth = call KJB_fnDepth;
        _ok = KJ_brainDepth < (KJB_WADE_MAX * KJB_H);
        // [finishing] on the retreat and at bay: no deeper than the surf
        if (KJB_retreatWalking or KJB_bay) then { _ok = KJ_brainDepth < (KJB_BAY_WADE * KJB_H) }
    };
    _ok
};

// anything call KJB_fnIsArr : true for an array (this evaluator has no typeName)
KJB_fnIsArr = { (substr [format ["%1", _this], 0, 1]) == "[" };

// object or position call KJB_fnPos : a position
KJB_fnPos = {
    private ["_p"];
    _p = [0, 0, 0];
    if (_this call KJB_fnIsArr) then { _p = _this } else { if (!(isNull _this)) then { _p = getPos _this } };
    _p
};

// call KJB_fnIsNight
KJB_fnIsNight = {
    (dayTime >= (KJB_NIGHT select 0)) or (dayTime <= (KJB_NIGHT select 1))
};

// ---- the mover -------------------------------------------------------------
// The brain sets KJB_goal (a position) or KJB_face (a bearing) and reads
// KJB_arrived, KJB_blocked, KJB_faced and KJB_mv. KJB_halt asks him to stop at
// the end of the stride he is in.

KJB_fnGo = {
    KJB_goal = [_this select 0, _this select 1, 0];
    KJB_goalLegs = 0;
    KJB_arrived = false;
    KJB_blocked = false;
    KJB_halt = false;
    KJB_face = -1;
    KJB_faced = true
};

KJB_fnStartTurn = {
    // _this: degrees to turn, signed (plus is clockwise)
    private ["_a", "_n", "_t", "_i", "_m"];
    _a = abs _this;
    if (KJB_mTurnL != "") then {
        _n = (_a / KJB_turnDeg) call KJB_fnRound;
        if (_n < 1) then { _n = 1 };
        _t = _n * KJB_turnSecs;
        _m = KJB_mTurnL;
        if (_this > 0) then { _m = KJB_mTurnR };
        _i = 0;
        while { _i < _n } do { KJB_k playMove _m; _i = _i + 1 }
    } else {
        _t = _a / KJB_SPOT_RATE
    };
    KJB_turnRate = _this / _t;
    KJB_turnTo = ((getDir KJB_k) + _this + 720) mod 360;
    KJB_turnEnd = time + _t;
    KJB_mv = "turn";
    KJ_brainTrack set [1, (KJ_brainTrack select 1) + 1]
};

KJB_fnStartWalk = {
    KJB_legStart = getPos KJB_k;
    KJB_cyc = 1;
    KJB_decided = 0;
    KJB_lastStride = false;
    KJB_walkT = time;
    KJB_mv = "walk";
    // [integration] wounded (the combat feature's KJ_woundPace under 0.8), this leg is walked in the
    // slow walk: the same clip at its slowest rate (DedWalk, relSpeedMin 0.6)
    KJB_walkNow = KJB_mWalk;
    KJB_paceNow = 1;
    if ((KJB_mWalk == "DedStride") and ((call KJ_fnPace) < 0.8)) then {
        KJB_walkNow = "DedWalk";
        KJB_paceNow = 0.6;
        if (!(isNil "KJ_animWalkSlowest")) then { KJB_paceNow = KJ_animWalkSlowest }
    };
    if (KJB_START_FIX) then { call KJB_fnStartFixBegin };
    KJB_k playMove KJB_walkNow
};

// [integration] The engine blends the walk in over KJB_blendSecs: the clip runs from its first
// frame, but the body only advances by the blended share of the clip's step
// (SoldierOldMove.cpp, moveFactor = phase change x _primaryFactor). So the foot he stands on is
// dragged back about a metre at every start (measured by the anim feature: 1.0 to 1.4 m). While
// the blend lasts the body is put forward by what is missing:
//   kaiju binary: by the planted foot's own drift, read off the model each tick (kjPoint), so
//                 the foot stays where it stood whatever the model and the clip;
//   stock binary: by the blend's arithmetic (half the blend time at walking speed, quadratic).
KJB_fnStartFixBegin = {
    KJB_fixT0 = time;
    KJB_fixUntil = time + KJB_blendSecs + 0.2;
    KJB_fixDone = 0;
    KJB_fixFoot = [];
    if (KJ_native and (KJB_START_FIX_MODE == "auto")) then { if (KJ_hasFootPoints) then { KJB_fixFoot = kjPoint [KJB_k, KJB_plantPt] } };
    KJ_brainStartFix set [0, (KJ_brainStartFix select 0) + 1];
    KJ_brainStartFix set [1, 0];
    KJ_brainStartFix set [2, "blend"];
    if ((count KJB_fixFoot) == 3) then { KJ_brainStartFix set [2, "foot"] }
};

KJB_fnStartFix = {
    private ["_dir", "_p", "_f", "_t", "_add"];
    _dir = getDir KJB_k;
    _add = 0;
    if ((count KJB_fixFoot) == 3) then {
        _f = kjPoint [KJB_k, KJB_plantPt];
        // how far the planted foot has slid back along his heading since he set off
        if ((count _f) == 3) then { _add = ((KJB_fixFoot select 0) - (_f select 0)) * sin _dir + ((KJB_fixFoot select 1) - (_f select 1)) * cos _dir }
    } else {
        _t = (time - KJB_fixT0) / KJB_blendSecs;
        if (_t > 1) then { _t = 1 };
        _add = KJB_walkSpeed * KJB_paceNow * KJB_blendSecs * 0.5 * _t * _t - KJB_fixDone
    };
    if (_add > 0.005) then {
        if (_add > 0.4) then { _add = 0.4 };
        _p = getPosASL KJB_k;
        KJB_k setPosASL [(_p select 0) + _add * sin _dir, (_p select 1) + _add * cos _dir, _p select 2];
        KJB_fixDone = KJB_fixDone + _add;
        KJ_brainStartFix set [1, KJB_fixDone]
    }
};

KJB_fnEndWalk = {
    // the leg is over: count it, and check the stride length against what was walked
    private ["_w", "_per"];
    _w = [KJB_legStart, getPos KJB_k] call KJB_fnDist;
    KJ_brainTrack set [0, (KJ_brainTrack select 0) + KJB_cyc];
    KJ_brainTrack set [2, (KJ_brainTrack select 2) + _w];
    if (KJB_cyc >= 2) then {
        _per = (_w + KJB_strideLoss) / KJB_cyc;
        if ((abs (_per - KJB_stride)) > (0.08 * KJB_stride)) then {
            logInfo format ["KJBRAIN stride corrected from %1 to %2 m (walked %3 m in %4 cycles)", KJB_stride, _per, _w, KJB_cyc];
            KJB_stride = _per
        }
    };
    KJB_goalLegs = KJB_goalLegs + 1;
    KJB_mv = "settle";
    KJB_settleUntil = time + KJB_PAUSE_STOP
};

KJB_fnMover = {
    private ["_k", "_now", "_dt", "_pos", "_dir", "_d", "_e", "_b", "_w", "_left", "_rem", "_step", "_go", "_mv"];
    _k = KJB_k;
    _now = time;
    _dt = _now - KJB_mvLast;
    KJB_mvLast = _now;
    if (_dt > 0.5) then { _dt = 0.5 };
    _pos = getPos _k;
    _dir = getDir _k;

    if (KJB_mv == "settle") then {
        if (_now >= KJB_settleUntil) then { KJB_mv = "stand" }
    };

    // a held clip (the first cycle of the dormant move) lets nothing else start: wait it out
    if ((KJB_mv == "stand") and (_now < KJB_heldUntil)) then { KJB_mv = "settle"; KJB_settleUntil = KJB_heldUntil };

    if (KJB_mv == "stand") then {
        KJB_halt = false;
        if ((count KJB_goal) > 0) then {
            _d = [_pos, KJB_goal] call KJB_fnDist;
            _go = true;
            if (_d <= (KJB_ARRIVE * KJB_stride)) then { _go = false };
            if ((KJB_goalLegs >= 1) and (_d <= (1.2 * KJB_stride))) then { _go = false };
            if (!_go) then {
                KJB_goal = [];
                KJB_arrived = true
            } else {
                _b = [_pos, KJB_goal] call KJB_fnBear;
                _e = [_b, _dir] call KJB_fnAng;
                _left = KJB_SPOT_MIN;
                if (_d < (1.5 * KJB_stride)) then { _left = 12 };
                if ((abs _e) > _left) then {
                    _e call KJB_fnStartTurn
                } else {
                    if ([_pos, _dir, KJB_LOOK * KJB_stride] call KJB_fnPassLine) then {
                        call KJB_fnStartWalk
                    } else {
                        KJB_goal = [];
                        KJB_blocked = true;
                        KJ_brainTrack set [3, (KJ_brainTrack select 3) + 1]
                    }
                }
            }
        } else {
            if (KJB_face >= 0) then {
                _e = [KJB_face, _dir] call KJB_fnAng;
                if ((abs _e) > 5) then { _e call KJB_fnStartTurn } else { KJB_face = -1; KJB_faced = true }
            }
        }
    };

    if (KJB_mv == "turn") then {
        if (_now >= KJB_turnEnd) then {
            _k setDir KJB_turnTo;
            KJB_mv = "settle";
            KJB_settleUntil = _now + KJB_PAUSE_TURN
        } else {
            _k setDir (_dir + KJB_turnRate * _dt)
        }
    };

    if (KJB_mv == "walk") then {
        if (_now < KJB_fixUntil) then { call KJB_fnStartFix; _pos = getPos _k };
        _w = [KJB_legStart, _pos] call KJB_fnDist;
        _mv = getMove _k;
        // steer: a little every tick, never inside the last stride to a goal
        if ((count KJB_goal) > 0) then {
            _d = [_pos, KJB_goal] call KJB_fnDist;
            if (_d > KJB_stride) then {
                _e = [[_pos, KJB_goal] call KJB_fnBear, _dir] call KJB_fnAng;
                _step = KJB_STEER * _dt;
                if ((abs _e) > 0.3) then {
                    if (_e > _step) then { _e = _step };
                    if (_e < (0 - _step)) then { _e = 0 - _step };
                    _k setDir (_dir + _e)
                }
            }
        };
        // past the middle of the stride he is in: does he take another?
        if ((KJB_decided < KJB_cyc) and ((_w + KJB_strideLoss) >= ((KJB_cyc - 0.45) * KJB_stride))) then {
            KJB_decided = KJB_cyc;
            _left = KJB_cyc * KJB_stride - _w - KJB_strideLoss;
            _go = true;
            if (KJB_halt) then { _go = false };
            if ((count KJB_goal) == 0) then { _go = false };
            if (_go) then {
                _rem = ([_pos, KJB_goal] call KJB_fnDist) - _left;
                if (_rem < (0.5 * KJB_stride)) then { _go = false };
                // the goal has moved far off his heading: he stops and turns on the spot, he does not walk a circle
                if ((abs ([[_pos, KJB_goal] call KJB_fnBear, _dir] call KJB_fnAng)) > (KJB_SPOT_MIN + 12)) then { _go = false }
            };
            if (_go) then {
                if (!([[_pos, _dir, _left] call KJB_fnAhead, _dir, KJB_LOOK * KJB_stride] call KJB_fnPassLine)) then {
                    _go = false;
                    KJB_goal = [];
                    KJB_blocked = true;
                    KJ_brainTrack set [3, (KJ_brainTrack select 3) + 1]
                }
            };
            if (_go) then {
                KJB_cyc = KJB_cyc + 1;
                _k playMove KJB_walkNow
            } else {
                KJB_lastStride = true
            }
        };
        // the last stride has played out: the engine has let go of the walk clip
        if (KJB_lastStride and (_mv != KJB_walkNow)) then { call KJB_fnEndWalk };
        // lost the clip without asking (something else played a move): take it from the top
        if ((KJB_mv == "walk") and (!KJB_lastStride) and (_mv != KJB_walkNow) and (_now > (KJB_walkT + 2))) then {
            logInfo format ["KJBRAIN walk lost at cycle %1 (move %2), restarting", KJB_cyc, _mv];
            call KJB_fnEndWalk
        };
        // never stuck: a cycle cannot take three times its length
        if ((KJB_mv == "walk") and (_now > (KJB_walkT + 3 * KJB_cyc * KJB_strideSecs / KJB_paceNow + 10))) then {
            logInfo "KJBRAIN walk timed out, restarting";
            call KJB_fnEndWalk
        }
    }
};

// ---- sequences -------------------------------------------------------------
// A sequence is a list of steps the tick works through in order:
//   ["halt"]              stop at the end of the stride, wait until he stands
//   ["face", posOrObj]    turn on the spot to face it
//   ["bear", {code}]      [integration] turn on the spot to the bearing the code gives
//   ["wait", seconds]
//   ["until", {cond}, timeout]
//   ["move", name]        playMove (skipped when the name is "")
//   ["call", {code}]
//   ["phase", word]       KJ_brainPhase
// When it runs out the brain enters the state KJB_seqThen.

// [steps, thenState] call KJB_fnSeq
KJB_fnSeq = {
    KJB_seq = _this select 0;
    KJB_seqThen = _this select 1;
    KJB_seqI = 0;
    KJB_seqIn = false
};

KJB_fnSeqTick = {
    private ["_s", "_kind", "_done", "_p"];
    if (KJB_seqI < (count KJB_seq)) then {
        _s = KJB_seq select KJB_seqI;
        _kind = _s select 0;
        _done = true;
        if (!KJB_seqIn) then {
            KJB_seqIn = true;
            KJB_seqT = time;
            if (_kind == "halt") then { KJB_goal = []; KJB_halt = true; KJB_face = -1 };
            if (_kind == "face") then {
                _p = (_s select 1) call KJB_fnPos;
                KJB_face = [getPos KJB_k, _p] call KJB_fnBear;
                KJB_faced = false
            };
            if (_kind == "bear") then { KJB_face = call (_s select 1); KJB_faced = false };
            if (_kind == "move") then { if ((_s select 1) != "") then { KJB_k playMove (_s select 1) } };
            if (_kind == "call") then { call (_s select 1) };
            if (_kind == "phase") then { KJ_brainPhase = _s select 1 }
        };
        if (_kind == "halt") then { _done = KJB_mv == "stand" };
        if (_kind == "face") then { _done = KJB_faced and (KJB_mv == "stand") };
        if (_kind == "bear") then { _done = KJB_faced and (KJB_mv == "stand") };
        if (_kind == "wait") then { _done = time >= (KJB_seqT + (_s select 1)) };
        if (_kind == "until") then {
            _done = time >= (KJB_seqT + (_s select 2));
            if (!_done) then { _done = call (_s select 1) }
        };
        if (_done) then { KJB_seqI = KJB_seqI + 1; KJB_seqIn = false }
    } else {
        if ((count KJB_seq) > 0) then {
            KJB_seq = [];
            KJB_seqI = 0;
            if (KJB_seqThen != "") then { KJB_seqThen call KJB_fnEnter }
        }
    }
};

// ---- interest: what draws him ----------------------------------------------
// One list of places. Each has interest (he walks to the highest) and anger
// (enough of it at one place earns the beam). Both halve every KJB_HALF_LIFE.

// [pos, interest, anger, object] call KJB_fnNote
KJB_fnNote = {
    private ["_p", "_w", "_a", "_o", "_best", "_bd", "_i", "_e", "_d", "_low", "_li", "_f"];
    _p = _this select 0;
    _w = _this select 1;
    _a = _this select 2;
    _o = _this select 3;
    // a place he has just stood on is dull
    _f = 1;
    { if ((time < (_x select 2)) and (([_p, _x] call KJB_fnDist) < (2 * KJB_H))) then { _f = 0.25 } } forEach KJB_beenTo;
    _w = _w * _f;
    // spent, he takes nothing in
    if (KJ_state == "spent") then { _w = 0; _a = 0 };
    _best = -1;
    _bd = KJB_CLUSTER * KJB_H;
    _i = 0;
    while { _i < (count KJ_interest) } do {
        _e = KJ_interest select _i;
        _d = [_p, _e] call KJB_fnDist;
        if (_d < _bd) then { _bd = _d; _best = _i };
        _i = _i + 1
    };
    // nothing to add and no place to add it to: no new place is made
    if ((_best >= 0) or ((_w + _a) > 0)) then { if (_best >= 0) then {
        _e = KJ_interest select _best;
        _e set [2, (_e select 2) + _w];
        _e set [3, (_e select 3) + _a];
        _e set [4, time];
        if (!(isNull _o)) then { _e set [5, _o]; _e set [0, _p select 0]; _e set [1, _p select 1] }
    } else {
        if ((count KJ_interest) >= 14) then {
            // full: the dullest place makes room
            _low = 1000000000;
            _li = 0;
            _i = 0;
            while { _i < (count KJ_interest) } do {
                _e = KJ_interest select _i;
                if (((_e select 2) + (_e select 3)) < _low) then { _low = (_e select 2) + (_e select 3); _li = _i };
                _i = _i + 1
            };
            KJ_interest set [_li, [_p select 0, _p select 1, _w, _a, time, _o]]
        } else {
            KJ_interest = KJ_interest + [[_p select 0, _p select 1, _w, _a, time, _o]]
        }
    } }
};

// [pos or object, weight] call KJB_fnDraw : a mission or another feature names a point of interest
KJB_fnDraw = {
    private ["_o"];
    _o = objNull;
    if (!((_this select 0) call KJB_fnIsArr)) then { _o = _this select 0 };
    [(_this select 0) call KJB_fnPos, _this select 1, 0, _o] call KJB_fnNote
};

KJB_fnDecay = {
    private ["_f", "_keep"];
    _f = 0.5 ^ ((time - KJB_tDecay) / KJB_HALF_LIFE);
    KJB_tDecay = time;
    _keep = [];
    {
        _x set [2, (_x select 2) * _f];
        _x set [3, (_x select 3) * _f];
        // a place that follows a thing moves with it
        if (!(isNull (_x select 5))) then {
            if (alive (_x select 5)) then { _x set [0, (getPos (_x select 5)) select 0]; _x set [1, (getPos (_x select 5)) select 1] } else { _x set [5, objNull] }
        };
        if (((_x select 2) + (_x select 3)) > 0.05) then { _keep = _keep + [_x] }
    } forEach KJ_interest;
    KJ_interest = _keep;
    _keep = [];
    { if (time < (_x select 2)) then { _keep = _keep + [_x] } } forEach KJB_beenTo;
    KJB_beenTo = _keep
};

// object call KJB_fnKind : 0 man, 1 armour or other ground vehicle, 2 aircraft
KJB_fnKind = {
    private ["_k"];
    _k = 0;
    if (("Air" countType [_this]) > 0) then { _k = 2 } else { if (("LandVehicle" countType [_this]) > 0) then { _k = 1 } };
    _k
};

// "Fired" on anything in earshot: [unit, weapon, muzzle, mode, ammo]
KJB_ehFired = {
    private ["_u", "_kind"];
    _u = _this select 0;
    if (KJ_brainOn and (KJ_state != "spent")) then {
        if ((_u distance KJB_k) < (KJB_HEAR * KJB_H)) then {
            _kind = _u call KJB_fnKind;
            KJB_lastShot = [time, _u];
            [getPos _u, KJB_W_GUN select _kind, KJB_A_GUN select _kind, objNull] call KJB_fnNote
        }
    }
};

// listen to everyone in earshot who is not listened to yet
KJB_fnEars = {
    {
        if ((_x != KJB_k) and (!(_x in KJB_eared))) then {
            KJB_eared = KJB_eared + [_x];
            _x addEventHandler ["Fired", { _this call KJB_ehFired }]
        }
    } forEach (nearestObjects [getPos KJB_k, ["AllVehicles"], KJB_HEAR * KJB_H])
};

// lit street lamps, at night: the lit part of a town is one place. lightIsOn
// must only ever be asked of a street lamp (asked of anything else the engine
// answers for it as if it were one; the roar notes have the crash that follows
// a switchLight on a man), so the class name is checked first.
KJB_fnLamps = {
    private ["_sx", "_sy", "_n", "_p"];
    _sx = 0;
    _sy = 0;
    _n = 0;
    {
        if ((substr [typeOf _x, 0, 10]) == "StreetLamp") then {
            if ((lightIsOn _x) != "OFF") then { _p = getPos _x; _sx = _sx + (_p select 0); _sy = _sy + (_p select 1); _n = _n + 1 }
        }
    } forEach (nearestObjects [getPos KJB_k, [], KJB_LIGHT * KJB_H]);
    if (_n >= 3) then { [[_sx / _n, _sy / _n, 0], KJB_W_LAMP * _n, 0, objNull] call KJB_fnNote }
};

// armour, engines, aircraft, and the hit signal
KJB_fnSense = {
    private ["_kp", "_tanks", "_n", "_t", "_sev", "_who", "_p", "_b"];
    _kp = getPos KJB_k;
    call KJB_fnDecay;
    // massed armour: a crewed tank counts once for every other crewed tank beside it
    _tanks = [];
    { if ((alive _x) and ((count (crew _x)) > 0)) then { _tanks = _tanks + [_x] } } forEach (nearestObjects [_kp, ["Tank"], KJB_SEE * KJB_H]);
    {
        _t = _x;
        _n = 0;
        { if ((_x != _t) and ((_x distance _t) < (4 * KJB_H))) then { _n = _n + 1 } } forEach _tanks;
        if (_n > 0) then { [getPos _t, KJB_W_ARMOUR * _n, KJB_W_ARMOUR * _n, _t] call KJB_fnNote };
        if (isEngineOn _t) then { [getPos _t, KJB_W_ENGINE, 0, objNull] call KJB_fnNote }
    } forEach _tanks;
    // aircraft close by
    {
        if ((alive _x) and (((getPos _x) select 2) > 5) and ((count (crew _x)) > 0)) then { [getPos _x, KJB_W_AIR, 0, _x] call KJB_fnNote }
    } forEach (nearestObjects [_kp, ["Air"], KJB_SEE * KJB_H]);
    // [integration] the hit signal is KJB_fnHitCheck, on every tick
    true
};

// [integration] the hit signal (the crush feature's hurt.sqs), on every tick: a hit is answered
// within a twentieth of a second of being raised, not up to two seconds later.
KJB_fnHitCheck = {
    private ["_sev", "_who", "_p"];
    if (!(isNil "KJ_hurtCount")) then {
        if (KJ_hurtCount > KJB_hurtSeen) then {
            KJB_hurtSeen = KJ_hurtCount;
            _sev = KJ_hurt select 3;
            _who = KJ_hurt select 4;
            if (isNull _who) then { if ((time - (KJB_lastShot select 0)) < 4) then { _who = KJB_lastShot select 1 } };
            _p = [];
            if (!(isNull _who)) then { _p = getPos _who };
            [_p, _sev] call KJB_fnHit
        }
    }
};

// [pos or [], severity] call KJB_fnHit : he has been hit from there
KJB_fnHit = {
    private ["_p", "_sev"];
    _p = _this select 0;
    _sev = _this select 1;
    if ((count _p) > 0) then { [_p, KJB_W_HIT * _sev, KJB_A_HIT * _sev, objNull] call KJB_fnNote };
    // [integration] wounded, he flinches sooner again (the gap shrinks with the wound stage)
    if ((KJ_brainAuto or KJB_HIT_REACT) and (_sev >= KJB_HIT_HARD) and ((time - KJB_lastHit) >= (KJB_HIT_GAP / (1 + (call KJ_fnWound)))) and (KJ_state in ["idle", "wander", "drawn", "dormant"]) and (!(call KJB_fnRetreating))) then {
        KJB_lastHit = time;
        KJB_hitFrom = _p;
        "flinch" call KJB_fnEnter
    }
};

// the place he should walk to now: index into KJ_interest, or -1
KJB_fnPickDraw = {
    private ["_best", "_bw", "_i", "_e", "_cur"];
    _best = -1;
    _bw = KJB_DRAW_MIN;
    _i = 0;
    while { _i < (count KJ_interest) } do {
        _e = KJ_interest select _i;
        if ((_e select 2) >= _bw) then { _bw = _e select 2; _best = _i };
        _i = _i + 1
    };
    _best
};

// the place that has earned the beam: index, or -1
KJB_fnPickBeam = {
    private ["_best", "_ba", "_i", "_e"];
    _best = -1;
    _ba = KJB_BEAM_ANGER;
    _i = 0;
    while { _i < (count KJ_interest) } do {
        _e = KJ_interest select _i;
        if ((_e select 3) >= _ba) then { _ba = _e select 3; _best = _i };
        _i = _i + 1
    };
    _best
};

// ---- the other features, through their entry points (or a stub) -------------

KJB_fnDoRoar = {
    KJB_lastRoar = time;
    if (KJB_hasRoar) then { [KJB_k] exec "kj\roar\roar.sqs" } else { if (KJB_mRoar != "") then { KJB_k playMove KJB_mRoar } };
    ["roar", format ["roars (feature %1)", KJB_hasRoar]] call KJB_fnLog
};

KJB_fnDoBeam = {
    KJB_beamWas = "";
    if (isNil "KJ_beamActive") then { KJ_beamActive = false };
    if (KJB_hasBeam) then {
        [KJB_k, KJB_beamTgt] exec "kj\beam\fire.sqs"
    } else {
        KJ_beamActive = true;
        KJ_beamPhase = "charge"
    };
    ["beam", format ["fires at %1 (feature %2)", KJB_beamTgt, KJB_hasBeam]] call KJB_fnLog
};

// true when the beam is over. Plays the rise and the lowering round it.
KJB_fnBeamDone = {
    private ["_done", "_ph"];
    _done = false;
    _ph = "";
    if (!(isNil "KJ_beamPhase")) then { _ph = KJ_beamPhase };
    if (KJ_brainBeamAnim and (_ph != KJB_beamWas)) then {
        if (_ph == "charge") then { KJB_k playMove KJB_mRise }
    };
    KJB_beamWas = _ph;
    if (!KJB_hasBeam) then {
        // the stub: stand tall for a while, then it is over
        if (time >= (KJB_seqT + KJB_BEAM_STUB)) then { KJ_beamActive = false; KJ_beamPhase = "spent" }
    };
    if ((time > (KJB_seqT + 1.5)) and (!KJ_beamActive)) then { _done = true };
    _done
};

KJB_fnBeamOver = {
    private ["_i"];
    if (KJ_brainBeamAnim) then { KJB_k playMove KJB_mLower };
    KJB_spentUntil = time + KJB_SPENT;
    if (!(isNil "KJ_beamSpentUntil")) then { if (KJ_beamSpentUntil > KJB_spentUntil) then { KJB_spentUntil = KJ_beamSpentUntil } };
    // spent: the anger is gone, and most of the interest with it
    { _x set [3, 0]; _x set [2, (_x select 2) * 0.3] } forEach KJ_interest
};

// [integration] the bearing he must face for the stomping foot to come down on the target
KJB_fnStompBear = {
    (([getPos KJB_k, KJB_blowTgt call KJB_fnPos] call KJB_fnBear) - KJB_stompAng + 720) mod 360
};

// [integration] the bearing he must face for the swat to reach the target: none other than the
// one he has while it is inside the sweep of his right arm (from dead ahead round to his right),
// else the one that puts it 45 degrees to his right
KJB_fnSwatBear = {
    private ["_b", "_rel", "_to"];
    _to = getDir KJB_k;
    _b = [getPos KJB_k, KJB_blowTgt call KJB_fnPos] call KJB_fnBear;
    _rel = [_b, _to] call KJB_fnAng;
    if ((_rel < -10) or (_rel > 110)) then { _to = (_b - 45 + 720) mod 360 };
    _to
};

KJB_fnDoStomp = {
    KJB_lastBlow = time;
    if (KJB_hasCrush) then { [KJB_k, KJB_blowTgt call KJB_fnPos] exec "kj\crush\stomp.sqs" };
    ["stomp", format ["stomps %1 (feature %2)", KJB_blowTgt, KJB_hasCrush]] call KJB_fnLog
};

KJB_fnDoSwat = {
    KJB_lastBlow = time;
    if (KJB_hasCrush) then { [KJB_k, KJB_blowTgt] exec "kj\crush\swat.sqs" };
    ["swat", format ["swats %1 (feature %2)", KJB_blowTgt, KJB_hasCrush]] call KJB_fnLog
};

// ---- states ----------------------------------------------------------------

// state call KJB_fnEnter
KJB_fnEnter = {
    private ["_s", "_steps", "_p", "_then"];
    _s = _this;
    KJ_state = _s;
    KJB_stateT = time;
    KJ_brainPhase = "";
    KJB_seq = [];
    KJB_seqI = 0;
    KJB_seqIn = false;
    [_s, "enter"] call KJB_fnLog;

    if (_s == "dormant") then {
        KJB_dormantSince = time;
        KJ_brainDraw = [];
        // the dormant clip is played as a move of its own for one cycle and holds by itself after that;
        // nothing queued behind it starts before that cycle is over, so the rise waits for it (KJB_heldUntil)
        [[["phase", "settling"], ["halt"], ["wait", 2], ["move", KJB_mDormant], ["call", { KJB_heldUntil = 0; if (KJB_mDormant != "") then { KJB_heldUntil = time + KJB_dormantSecs + 0.5 } }], ["phase", "still"]], ""] call KJB_fnSeq
    };
    if (_s == "wake") then {
        // not a state of its own for long: the rise, the roar, then awake
        KJ_state = "dormant";
        KJB_lastDrawn = time;
        // never play the idle loop as a move (it would hold the queue for a whole loop): the shudder
        // takes him out of the dormant hold and ends in the idle by itself
        _steps = [["phase", "rising"], ["until", { time >= KJB_heldUntil }, 30]];
        if (KJB_mDormant != "") then { _steps = _steps + [["move", KJB_mFlinch]] };
        _steps = _steps + [["wait", KJB_WAKE_SECS]];
        if (KJB_WAKE_ROAR) then { _steps = _steps + [["call", { KJ_state = "roar"; call KJB_fnDoRoar }], ["wait", KJB_ROAR_SECS]] };
        [_steps, "idle"] call KJB_fnSeq
    };
    if (_s == "idle") then {
        KJ_brainDraw = [];
        // [integration] wounded, the pause between two walks grows (half as long again per wound stage)
        KJB_idleUntil = time + (KJB_IDLE_MIN + random KJB_IDLE_RAND) * (1 + 0.5 * (call KJ_fnWound));
        [[["phase", "stopping"], ["halt"], ["phase", "standing"]], ""] call KJB_fnSeq
    };
    if (_s == "wander") then {
        KJ_brainDraw = [];
        KJ_brainPhase = "choosing";
        call KJB_fnWanderLeg
    };
    if (_s == "drawn") then {
        KJ_brainPhase = "walking"
    };
    if (_s == "roar") then {
        [[["phase", "stopping"], ["halt"], ["wait", 1.2], ["phase", "roaring"], ["call", { call KJB_fnDoRoar }], ["wait", KJB_ROAR_SECS]], "idle"] call KJB_fnSeq
    };
    if (_s == "beam") then {
        _steps = [["phase", "stopping"], ["halt"], ["phase", "turning"], ["face", KJB_beamTgt], ["wait", 2]];
        if ((time - KJB_lastRoar) >= KJB_ROAR_GAP) then { _steps = _steps + [["phase", "roaring"], ["call", { call KJB_fnDoRoar }], ["wait", KJB_ROAR_SECS]] };
        _steps = _steps + [["phase", "firing"], ["call", { call KJB_fnDoBeam }], ["until", { call KJB_fnBeamDone }, 150], ["call", { call KJB_fnBeamOver }]];
        [_steps, "spent"] call KJB_fnSeq
    };
    if (_s == "spent") then {
        KJ_brainDraw = [];
        if (KJB_spentUntil < time) then { KJB_spentUntil = time + KJB_SPENT };
        [[["phase", "stopping"], ["halt"], ["phase", "spent"], ["until", { time >= KJB_spentUntil }, 900]], "idle"] call KJB_fnSeq
    };
    if (_s == "flinch") then {
        // he finishes the stride he is in, shudders, stands, and only then turns to it
        // [integration] with the shudder comes the creature's own cry at a hard hit (its kjSnd_Hurt), when its voice has one
        _steps = [["phase", "stopping"], ["halt"], ["until", { time >= KJB_heldUntil }, 30], ["phase", "shudder"], ["move", KJB_mFlinch], ["call", { if (KJB_hasRoar) then { [KJB_k, "KJ_Hurt", "KJ_HurtFar", 0, KJR_MOUTH_H] exec "kj\roar\snd.sqs" } }], ["wait", 3], ["phase", "staring"], ["wait", KJB_HIT_STARE]];
        _then = "idle";
        if ((count KJB_hitFrom) > 0) then {
            _steps = _steps + [["phase", "turning"], ["face", KJB_hitFrom], ["wait", 2]];
            _then = "hunt"
        };
        [_steps, _then] call KJB_fnSeq
    };
    if (_s == "hunt") then {
        // after a hit: drawn to where it came from, unless a director has said what comes next
        if ((count KJ_brainAfterHit) > 0) then {
            KJ_state = "idle";
            KJ_brainPhase = "standing";
            KJ_brainOrder = KJ_brainAfterHit;
            KJ_brainAfterHit = []
        } else {
            if (KJ_brainAuto) then {
                KJ_state = "drawn";
                KJ_brainPhase = "walking";
                KJB_drawI = -1;
                KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
                KJ_brainDraw call KJB_fnGo
            } else {
                // [integration] a director drives him: he has turned to it and now stands
                KJ_state = "idle";
                KJ_brainPhase = "standing"
            }
        }
    };
    if (_s == "bay") then {
        // [finishing] the last stand: he turns on what is killing him and roars, then he is himself
        _steps = [["phase", "stopping"], ["halt"]];
        if ((count KJB_bayFace) > 0) then { _steps = _steps + [["phase", "turning"], ["face", KJB_bayFace], ["wait", 1.5]] };
        _steps = _steps + [["phase", "roaring"], ["call", { call KJB_fnDoRoar }], ["wait", KJB_ROAR_SECS]];
        [_steps, "idle"] call KJB_fnSeq
    };
    if (_s == "stomp") then {
        // [integration] he turns so that the stomping foot of the clip comes down on it, then waits the clip out
        [[["phase", "stopping"], ["halt"], ["phase", "turning"], ["bear", { call KJB_fnStompBear }], ["phase", "stomping"], ["call", { call KJB_fnDoStomp }], ["wait", KJB_stompSecs]], "idle"] call KJB_fnSeq
    };
    if (_s == "swat") then {
        // [integration] the swat is the right arm, swung from his right rear round to dead ahead:
        // something outside that sweep is turned to first
        [[["phase", "stopping"], ["halt"], ["phase", "turning"], ["bear", { call KJB_fnSwatBear }], ["phase", "swatting"], ["call", { call KJB_fnDoSwat }], ["wait", KJB_swatSecs]], "idle"] call KJB_fnSeq
    };
    if (_s == "dead") then {
        KJB_goal = [];
        KJ_brainDraw = []
    }
};

// a wander leg: a few strides on a heading near the one he has, bent toward
// the station when he has one, turned away from the sea when it is in the way
KJB_fnWanderLeg = {
    private ["_pos", "_dir", "_len", "_try", "_ok", "_b", "_d"];
    _pos = getPos KJB_k;
    _dir = getDir KJB_k;
    _len = (KJB_LEG_MIN + random KJB_LEG_RAND) * KJB_stride;
    _b = _dir + (random (2 * KJB_LEG_SWING)) - KJB_LEG_SWING;
    if ((count KJ_station) > 0) then {
        _d = [_pos, KJ_station] call KJB_fnDist;
        if (_d > (3 * KJB_H)) then {
            _b = [_pos, KJ_station] call KJB_fnBear;
            if (_len > _d) then { _len = _d }
        }
    };
    _try = 0;
    _ok = false;
    while { (!_ok) and (_try < 6) } do {
        if ([_pos, _b, _len] call KJB_fnDryLine) then { _ok = true } else { _b = _b + 60; _try = _try + 1 }
    };
    if (_ok) then {
        ([_pos, _b, _len] call KJB_fnAhead) call KJB_fnGo;
        KJ_brainPhase = "walking"
    } else {
        // water all round: he stands where he is
        KJB_blocked = true
    }
};

// orders from a mission or a test: KJ_brainOrder = [word, argument]
KJB_fnOrder = {
    private ["_o", "_w", "_a"];
    _o = KJ_brainOrder;
    KJ_brainOrder = [];
    _w = _o select 0;
    _a = [];
    if ((count _o) > 1) then { _a = _o select 1 };
    ["order", format ["%1", _o]] call KJB_fnLog;
    if (_w == "auto") then { KJ_brainAuto = _a };
    if (_w == "dormant") then { "dormant" call KJB_fnEnter };
    if (_w == "wake") then { "wake" call KJB_fnEnter };
    if (_w == "idle") then { "idle" call KJB_fnEnter };
    if (_w == "wander") then { "wander" call KJB_fnEnter };
    if (_w == "roar") then { "roar" call KJB_fnEnter };
    if (_w == "drawn") then {
        "drawn" call KJB_fnEnter;
        KJB_drawI = -1;
        _a = _a call KJB_fnPos;
        KJ_brainDraw = [_a select 0, _a select 1];
        KJ_brainDraw call KJB_fnGo
    };
    if (_w == "beam") then { KJB_beamTgt = _a; "beam" call KJB_fnEnter };
    if (_w == "spent") then { KJB_spentUntil = time + _a; "spent" call KJB_fnEnter };
    if (_w == "hit") then { KJB_lastHit = time; KJB_hitFrom = _a call KJB_fnPos; "flinch" call KJB_fnEnter };
    if (_w == "stomp") then { KJB_blowTgt = _a; "stomp" call KJB_fnEnter };
    if (_w == "swat") then { KJB_blowTgt = _a; "swat" call KJB_fnEnter }
};

// [integration] The combat feature's sea retreat (KJ_woundRetreat: badly hurt and still being
// hit). While it holds and he decides for himself, nothing else draws him, he does not stop to
// shudder or to use the beam: he walks for the sea, on past the shore as far as he can wade, and
// goes dormant there. A blow at something in his reach on the way is still his.
KJB_retreatGoal = [];
KJB_retreatWalking = false;
KJB_retreatStood = false;
// [finishing] at bay: the retreat has ended in the surf (or he is dying on his feet) and he has
// turned on them. KJ_brainBay is the same, for others to read; KJB_retreatFrom is where he was
// when he turned for the sea (what he faces when nothing has hit him hard).
KJB_bay = false;
KJ_brainBay = false;
KJB_retreatFrom = [];
KJB_bayFace = [];
KJB_fnRetreating = {
    private ["_r"];
    _r = false;
    if (KJ_brainAuto and (!(isNil "KJ_woundRetreat")) and (!(isNil "KJ_woundRetreatPos"))) then {
        if (KJ_woundRetreat and ((count KJ_woundRetreatPos) >= 2)) then { _r = true }
    };
    if (!_r) then { KJB_retreatGoal = []; KJB_retreatWalking = false; KJB_retreatStood = false; KJB_bay = false; KJ_brainBay = false };
    // [finishing] at bay he is himself again: nothing of the retreat's rules holds
    if (KJB_bay) then { _r = false };
    _r
};
// where he walks to: the combat feature's nearest point of the sea, and 4 of his heights on past it
KJB_fnRetreatGoal = {
    private ["_kp", "_dx", "_dy", "_d"];
    if ((count KJB_retreatGoal) == 0) then {
        _kp = getPos KJB_k;
        KJB_retreatFrom = [_kp select 0, _kp select 1, 0];
        _dx = (KJ_woundRetreatPos select 0) - (_kp select 0);
        _dy = (KJ_woundRetreatPos select 1) - (_kp select 1);
        _d = sqrt (_dx * _dx + _dy * _dy);
        if (_d < 1) then { _d = 1; _dx = 0; _dy = -1 };
        KJB_retreatGoal = [(KJ_woundRetreatPos select 0) + 4 * KJB_H * _dx / _d, (KJ_woundRetreatPos select 1) + 4 * KJB_H * _dy / _d];
        ["retreat", format ["badly hurt, he turns for the sea: the shore at %1, he makes for %2 (he is at %3)", KJ_woundRetreatPos, KJB_retreatGoal, _kp]] call KJB_fnLog
    };
    KJB_retreatGoal
};

// what he decides by himself (KJ_brainAuto), once a second
KJB_fnDecide = {
    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret", "_free"];
    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;

    // a blow, when something is in reach and he is standing or between strides
    // ([integration] decided before the beam: a tank under his foot comes before a target far off)
    // ([finishing] and also while he stares or turns after a hit: under steady fire he is seldom in
    // any other state, and a tank that stands at his feet through all of it must not be left alone)
    _free = (KJ_state in ["idle", "wander", "drawn"]) or ((KJ_state == "flinch") and (KJ_brainPhase in ["staring", "turning"]));
    if (_free and ((time - KJB_lastBlow) >= KJB_BLOW_GAP)) then {
        {
            if (((KJ_state in ["idle", "wander", "drawn"]) or ((KJ_state == "flinch") and (KJ_brainPhase in ["staring", "turning"]))) and (alive _x) and ((count (crew _x)) > 0)) then {
                if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < 70) then { KJB_blowTgt = _x; "stomp" call KJB_fnEnter }
            }
        } forEach (nearestObjects [_kp, ["Tank"], KJB_STOMP_R * KJB_H]);
        {
            if ((KJ_state in ["idle", "wander", "drawn"]) and (alive _x) and (((getPos _x) select 2) > (0.25 * KJB_H))) then { KJB_blowTgt = _x; "swat" call KJB_fnEnter }
        } forEach (nearestObjects [[_kp select 0, _kp select 1, 0.8 * KJB_H], ["Air"], KJB_SWAT_R * KJB_H])
    };

    // the beam, from any waking state ([integration] not while he retreats to the sea)
    if ((KJ_state in ["idle", "wander", "drawn"]) and (!_ret)) then {
        _i = call KJB_fnPickBeam;
        if ((_i >= 0) and (time >= KJB_spentUntil)) then {
            _e = KJ_interest select _i;
            _d = [_kp, _e] call KJB_fnDist;
            if ((_d >= (KJB_BEAM_NEAR * KJB_H)) and (_d <= (KJB_BEAM_FAR * KJB_H))) then {
                KJB_beamTgt = [_e select 0, _e select 1, 0];
                if (!(isNull (_e select 5))) then { KJB_beamTgt = _e select 5 };
                "beam" call KJB_fnEnter
            }
        }
    };

    // [integration] the sea retreat: nothing else draws him (see KJB_fnRetreating)
    if (_ret) then {
        _p = call KJB_fnRetreatGoal;
        // already in the sea when it begins: he is where he wants to be
        if ((!KJB_retreatWalking) and (!KJB_retreatStood)) then { if ((call KJB_fnDepth) > (0.1 * KJB_H)) then { KJB_retreatStood = true } };
        // [finishing] dying on his feet he goes no further: he stops where he is and turns at bay
        if ((KJ_state == "drawn") and KJB_retreatWalking and ((call KJ_fnWound) >= 4)) then {
            ["retreat", format ["dying on his feet at %1: he goes no further", _kp]] call KJB_fnLog;
            KJB_retreatWalking = false;
            KJB_retreatStood = true;
            "idle" call KJB_fnEnter
        };
        if ((KJ_state in ["idle", "wander"]) and ((count KJB_seq) == 0)) then {
            if (KJB_retreatStood) then {
                // [finishing] he is in the surf, or can go no further: he does not go under. He
                // turns on them and stands at bay (before: he went dormant, in water up to 17 m deep)
                KJB_bay = true;
                KJ_brainBay = true;
                KJB_bayFace = KJB_retreatFrom;
                if ((count KJB_hitFrom) > 0) then { KJB_bayFace = KJB_hitFrom };
                ["retreat", format ["at bay at %1, water %2 m deep: he turns on them (toward %3)", _kp, call KJB_fnDepth, KJB_bayFace]] call KJB_fnLog;
                "bay" call KJB_fnEnter
            } else {
                "drawn" call KJB_fnEnter;
                KJB_drawI = -1;
                KJ_brainDraw = [_p select 0, _p select 1];
                KJ_brainDraw call KJB_fnGo;
                KJB_lastDrawn = time;
                KJB_retreatWalking = true
            }
        };
        // he was walking somewhere else when it began: the sea instead
        if ((KJ_state == "drawn") and (!KJB_retreatWalking) and (!KJB_retreatStood)) then {
            KJB_drawI = -1;
            KJ_brainDraw = [_p select 0, _p select 1];
            KJ_brainDraw call KJB_fnGo;
            KJB_lastDrawn = time;
            KJB_retreatWalking = true
        }
    };

    if ((KJ_state == "dormant") and (!_ret)) then {
        if ((count KJB_seq) == 0) then {
            _f = false;
            { if (((_x select 2) >= (2 * KJB_DRAW_MIN)) and (([_kp, _x] call KJB_fnDist) < (KJB_WAKE_R * KJB_H))) then { _f = true } } forEach KJ_interest;
            if ((time - KJB_dormantSince) > KJB_DORMANT_MAX) then { _f = true };
            if (_f) then { "wake" call KJB_fnEnter }
        }
    };

    if ((KJ_state == "idle") and (!_ret)) then {
        if ((count KJB_seq) == 0) then {
            _i = call KJB_fnPickDraw;
            if (_i >= 0) then {
                _e = KJ_interest select _i;
                "drawn" call KJB_fnEnter;
                KJB_drawI = _i;
                KJ_brainDraw = [_e select 0, _e select 1];
                KJ_brainDraw call KJB_fnGo;
                KJB_lastDrawn = time
            } else {
                if (time >= KJB_idleUntil) then {
                    if ((time - KJB_lastDrawn) > KJB_DORMANT_AFTER) then {
                        _r = true;
                        if ((count KJ_station) > 0) then { if (([_kp, KJ_station] call KJB_fnDist) > (3 * KJB_H)) then { _r = false } };
                        if (_r) then { "dormant" call KJB_fnEnter } else { "wander" call KJB_fnEnter }
                    } else {
                        "wander" call KJB_fnEnter
                    }
                }
            }
        }
    };

    if ((KJ_state == "wander") and (!_ret)) then {
        _i = call KJB_fnPickDraw;
        if (_i >= 0) then {
            _e = KJ_interest select _i;
            "drawn" call KJB_fnEnter;
            KJB_drawI = _i;
            KJ_brainDraw = [_e select 0, _e select 1];
            KJ_brainDraw call KJB_fnGo;
            KJB_lastDrawn = time
        }
    };

    if ((KJ_state == "drawn") and (!_ret)) then {
        KJB_lastDrawn = time;
        // a much stronger place takes him over; the place he walks to may move (a vehicle)
        _i = call KJB_fnPickDraw;
        if (_i >= 0) then {
            _e = KJ_interest select _i;
            _cur = 0;
            { if (([KJ_brainDraw, _x] call KJB_fnDist) < (KJB_CLUSTER * KJB_H)) then { if ((_x select 2) > _cur) then { _cur = _x select 2 } } } forEach KJ_interest;
            _d = [KJ_brainDraw, _e] call KJB_fnDist;
            if ((_d > KJB_stride) and (((_e select 2) > (_cur * KJB_DRAW_STICK + 1)) or (_d < (KJB_CLUSTER * KJB_H)))) then {
                KJ_brainDraw = [_e select 0, _e select 1];
                if ((count KJB_goal) > 0) then { KJB_goal = [_e select 0, _e select 1, 0] }
            }
        }
    }
};

// what happens whoever decides: arrivals, water, death
KJB_fnFollow = {
    private ["_kp"];
    _kp = getPos KJB_k;
    if (KJ_state == "wander") then {
        if (KJB_arrived or KJB_blocked) then {
            KJB_arrived = false;
            KJB_blocked = false;
            "idle" call KJB_fnEnter
        }
    };
    if (KJ_state == "drawn") then {
        if (KJB_arrived or KJB_blocked) then {
            // he stands on it (or at the water's edge before it) and it is dull for a while
            KJB_beenTo = KJB_beenTo + [[KJ_brainDraw select 0, KJ_brainDraw select 1, time + KJB_VISITED]];
            { if (([KJ_brainDraw, _x] call KJB_fnDist) < (KJB_CLUSTER * KJB_H)) then { _x set [2, 0] } } forEach KJ_interest;
            ["drawn", format ["reached %1 (blocked by water %2)", KJ_brainDraw, KJB_blocked]] call KJB_fnLog;
            // [integration] the end of the sea retreat's walk
            if (KJB_retreatWalking) then { KJB_retreatWalking = false; KJB_retreatStood = true };
            KJB_arrived = false;
            KJB_blocked = false;
            "idle" call KJB_fnEnter
        }
    }
};

KJB_fnTick = {
    private ["_now"];
    _now = time;
    if ((!(alive KJB_k)) or KJ_dead) then {
        // [integration] dead (the engine's, or the combat feature's KJ_dead): no walking corpse.
        // While he is unkillable a killing blow is undone by the crush feature within a fifth of a
        // second, so the brain waits half a second before it believes it.
        if (alive KJB_k) then { KJB_deadSince = -1 };
        if ((!(alive KJB_k)) and (KJB_deadSince < 0)) then { KJB_deadSince = _now };
        if (KJ_dead or (!KJ_unkillable) or (_now > (KJB_deadSince + 0.5))) then {
            if (KJ_state != "dead") then { "dead" call KJB_fnEnter }
        }
    } else {
        KJB_deadSince = -1;
        if ((count KJ_brainOrder) > 0) then { call KJB_fnOrder };
        call KJB_fnMover;
        call KJB_fnSeqTick;
        call KJB_fnFollow;
        call KJB_fnHitCheck;
        if (_now >= KJB_tSense) then {
            KJB_tSense = _now + KJB_SENSE;
            call KJB_fnSense;
            if (KJ_brainAuto) then {
                if (_now >= KJB_tEars) then { KJB_tEars = _now + KJB_EARS; call KJB_fnEars };
                if ((_now >= KJB_tLamps) and (call KJB_fnIsNight)) then { KJB_tLamps = _now + KJB_LAMPS; call KJB_fnLamps };
                call KJB_fnDecide
            }
        }
    }
};

KJB_ready = true;
