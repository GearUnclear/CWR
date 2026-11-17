// ============================================================================
//  kj\init.sqf  -  Dedushka, the integrated script set: the shared core.
//  Loaded once by kj\start.sqs (and by any feature entry point that is called
//  before it):   if (isNil "KJ_coreReady") then { call compile preprocessFile "kj\init.sqf" }
//
//  What is here: which engine runs (KJ_native), the engine traits, and the few
//  helpers every feature uses (kill with credit, throw, memory point, shake at
//  a place, the furnace point). Each native command sits inside a block that
//  only runs when KJ_native is true, so the stock binary never parses it
//  (features\engine\API.md, "Detecting it from script").
//
//  A mission may set, BEFORE kj\start.sqs:
//    KJ_forceStock = true   use the script fallbacks even on the kaiju binary
//    KJ_height              his height in metres (else the addon's kjHeight)
//    KJ_traits              the engine traits to hold on him (see below)
// ============================================================================

// ---- which engine ----------------------------------------------------------
KJ_engineVersion = 0;
KJ_nativeBinary = (format ["%1", kjVersion]) != (format ["%1", KJ_noSuchVariable_]);
KJ_native = KJ_nativeBinary;
if (isNil "KJ_forceStock") then { KJ_forceStock = false };
if (KJ_forceStock) then { KJ_native = false };
if (KJ_native) then { KJ_engineVersion = kjVersion };

// ---- the engine traits held on him (name, value). The creature addon's
// ---- `class Kaiju` block gives the same ones; this list is what is checked
// ---- and, where the block did not deliver, set by kjSet.
// ---- wadeDepth -1 means "three quarters of his height".
if (isNil "KJ_traits") then { KJ_traits = [["farDraw", 6000], ["farContrast", 0.5], ["wadeDepth", -1], ["obstacleBlind", 1], ["noSpeedDamage", 1], ["damageCeiling", 0.6], ["hitReport", 1], ["stepShake", 0], ["nearMip", 1]] };
// ---- [finishing] the traits of engine version 2, held the same way but counted apart (the
// ---- creature addon's block may or may not carry them): the night floor of the far draw, so the
// ---- furnace shows from past the view distance by night, the planted foot at the start of a walk, and
// ---- his LOD chosen by distance alone (the scene's LOD scale grows with the frame load).
if (isNil "KJ_traits2") then { KJ_traits2 = [["farGlow", 0.9], ["plantBlend", 1], ["lodPin", 1]] };
KJ_traits2Log = [];
KJ_traitLog = [];        // [[name, value found, value wanted, "config" or "kjSet"], ...]
KJ_traitsFromConfig = 0; // how many of them the config block had already set

// ---- [finishing] the furnace painted on his body (the engine's "glow" trait, version 2):
// ---- 0 dark, 1 the painted colour, to 2 white hot. On the stock binary, and on a kaiju binary
// ---- of version 1, nothing can change it and KJ_fnGlow only keeps the number.
KJ_glowNative = false;
KJ_glowNow = -1;
// [unit, 0..2] call KJ_fnGlow
KJ_fnGlow = {
    KJ_glowNow = _this select 1;
    if (KJ_glowNative) then { kjSet [_this select 0, "glow", _this select 1] }
};

KJ_unit = objNull;
if (isNil "KJ_heightM") then { KJ_heightM = 40 };
KJ_furnacePoint = "zamerny";
KJ_furnaceOfs = [0, 5.3, 32];   // [right, forward, up] of the furnace in his frame, rest pose
KJ_hasFootPoints = false;

// obj call KJ_fnCfgNum : a number the creature addon may carry (kjHeight, ...), or -1
KJ_fnCfgNum = {
    private ["_v", "_n"];
    _n = -1;
    _v = format ["%1", (typeOf (_this select 0)) GetVehicleParam (_this select 1)];
    if ((_v != "<null>") and (_v != "") and (_v != format ["%1", KJ_noSuchVariable_])) then { _n = call compile _v };
    _n
};

// [obj, "entry"] call KJ_fnCfgStr : a text the creature addon may carry (kjVoice, ...), or ""
KJ_fnCfgStr = {
    private ["_v"];
    _v = format ["%1", (typeOf (_this select 0)) GetVehicleParam (_this select 1)];
    if ((_v == "<null>") or (_v == format ["%1", KJ_noSuchVariable_])) then { _v = "" };
    _v
};

// The creature is a data entry: everything the scripts need to know about one
// is read off the unit's own class (CfgVehicles >> <class>, written by the
// addon build from the model) and off its model. Two creature classes share
// this one script set.
//   kjHeight                 metres; every script size is a fraction of it
//   kjFurnacePoint           memory point of the furnace (light, embers, beam)
//   kjFurnaceFwd, kjFurnaceUp   the same point in metres, for the stock binary
//   kjSnd_<Name>             its own voice: the CfgSounds class it uses for the shared class KJ_<Name>
//                            (kjSnd_Roar, kjSnd_RoarFar, kjSnd_Hurt, kjSnd_HurtFar, ...)
//   kj\anim\timing_<class>.sqf   the timing table of its own clips (KJ_anim*)
KJ_voiceLog = [];   // [[shared class, class used], ...] each first use

// "KJ_Roar" call KJ_fnVoice : the class to play for a shared sound class. The
// creature's own take when its class names one (kjSnd_Roar) and the mounted
// sound set has it; else the shared class when the set has it; else "" (no
// sound: KJ_Hurt, the cry at a hard hit, exists only as a creature's own take).
KJ_fnVoice = {
    private ["_c", "_own", "_r"];
    _c = _this;
    _r = "";
    if ((soundLength _c) > 0) then { _r = _c };
    if (((substr [_c, 0, 3]) == "KJ_") and (!(isNull KJ_unit))) then {
        _own = [KJ_unit, "kjSnd_" + (substr [_c, 3, sizeofstr _c])] call KJ_fnCfgStr;
        if (_own != "") then { if ((soundLength _own) > 0) then { _r = _own } }
    };
    if ((({ (_x select 0) == _c } count KJ_voiceLog) == 0) and ((count KJ_voiceLog) < 40)) then { KJ_voiceLog = KJ_voiceLog + [[_c, _r]] };
    _r
};

// unit call KJ_fnSetup : once per creature. Height, voice, timing table, furnace point, traits.
KJ_fnSetup = {
    private ["_k", "_h", "_w", "_have", "_want", "_p", "_f"];
    _k = _this;
    KJ_unit = _k;
    KJ_traitLog = [];
    KJ_traitsFromConfig = 0;
    _h = [_k, "kjHeight"] call KJ_fnCfgNum;
    if (_h > 0) then { KJ_heightM = _h };
    if (!(isNil "KJ_height")) then { KJ_heightM = KJ_height };
    KJ_voiceLog = [];
    _f = [_k, "kjFurnacePoint"] call KJ_fnCfgStr;
    if (_f != "") then { KJ_furnacePoint = _f };
    // the timing table of this creature's own clips; the default table when its class has none
    _f = format ["kj\anim\timing_%1.sqf", typeOf _k];
    if ((loadFile _f) == "") then { _f = "kj\anim\ded_anim_timing.sqf" };
    if ((loadFile _f) != "") then { call compile preprocessFile _f };
    KJ_timingFile = _f;
    // where its feet come down, from its own walk clip, for the crush and roar features' stride
    // arithmetic (their own numbers are the stand-in's): stance, how far ahead of its origin a foot
    // lands, ground between two footfalls, and which foot lands first
    if (!(isNil "KJ_animWalkFalls")) then {
        _w = (KJ_animWalkFalls select 0) select 2;
        KJC_STANCE = (abs (_w select 0)) / KJ_heightM;
        KJC_FOOT_AHEAD = (_w select 1) / KJ_heightM;
        KJC_HALF_STRIDE = KJ_animWalkStep / 2 / KJ_heightM;
        KJC_LEG_HALF = KJC_STANCE + 0.065;   // outer edge of the legs: half a foot's width outside the stance
        KJC_FIRST_FOOT = 1;
        if (((KJ_animWalkFalls select 0) select 1) == "L") then { KJC_FIRST_FOOT = -1 };
        KJR_STANCE = KJC_STANCE;
        KJR_FOOT_AHEAD = KJC_FOOT_AHEAD;
        KJR_HALF_STRIDE = KJC_HALF_STRIDE
    };
    _h = [_k, "kjFurnaceFwd"] call KJ_fnCfgNum;
    if (_h > 0) then { KJ_furnaceOfs = [0, _h, [_k, "kjFurnaceUp"] call KJ_fnCfgNum] };
    // where its sounds come from, as the roar feature wants them (fractions of its height)
    KJR_CHEST_H = (KJ_furnaceOfs select 2) / KJ_heightM;
    _h = [_k, "kjMouthUp"] call KJ_fnCfgNum;
    if (_h > 0) then { KJR_MOUTH_H = _h / KJ_heightM };
    if (KJ_native) then {
        {
            _want = _x select 1;
            if (((_x select 0) == "wadeDepth") and (_want < 0)) then { _want = 0.75 * KJ_heightM };
            _have = kjGet [_k, _x select 0];
            if ((abs (_have - _want)) < 0.001) then {
                KJ_traitsFromConfig = KJ_traitsFromConfig + 1;
                KJ_traitLog = KJ_traitLog + [[_x select 0, _have, _want, "config"]]
            } else {
                kjSet [_k, _x select 0, _want];
                KJ_traitLog = KJ_traitLog + [[_x select 0, _have, _want, "kjSet"]]
            }
        } forEach KJ_traits;
        // [finishing] engine version 2: its traits, and the glow of the furnace on the body
        KJ_glowNative = KJ_engineVersion >= 2;
        KJ_traits2Log = [];
        if (KJ_glowNative) then {
            {
                _have = kjGet [_k, _x select 0];
                if ((abs (_have - (_x select 1))) < 0.001) then {
                    KJ_traits2Log = KJ_traits2Log + [[_x select 0, _have, "config"]]
                } else {
                    kjSet [_k, _x select 0, _x select 1];
                    KJ_traits2Log = KJ_traits2Log + [[_x select 0, _have, "kjSet"]]
                }
            } forEach KJ_traits2
        };
        // the furnace point off the model, in his own frame (rest or current pose: he stands)
        _p = kjPoint [_k, KJ_furnacePoint];
        if ((count _p) == 3) then { KJ_furnaceOfs = [_k, _p] call KJ_fnToLocal };
        KJ_hasFootPoints = ((count (kjPoint [_k, "stopaL"])) == 3) and ((count (kjPoint [_k, "stopaP"])) == 3)
    };
    logInfo format ["KJ setup: native %1 (binary %2, version %3), height %4, furnace %5, foot points %6, traits from config %7 of %8: %9", KJ_native, KJ_nativeBinary, KJ_engineVersion, KJ_heightM, KJ_furnaceOfs, KJ_hasFootPoints, KJ_traitsFromConfig, count KJ_traits, KJ_traitLog];
    logInfo format ["KJ setup: engine version %1, glow on the body %2, version 2 traits %3", KJ_engineVersion, KJ_glowNative, KJ_traits2Log];
    logInfo format ["KJ setup: creature %1, roar class %2, hurt class %3, timing table %4, furnace point %5", typeOf _k, "KJ_Roar" call KJ_fnVoice, "KJ_Hurt" call KJ_fnVoice, KJ_timingFile, KJ_furnacePoint]
};

// [unit, [x, y, zASL]] call KJ_fnToLocal : [right, forward, up] in his frame
KJ_fnToLocal = {
    private ["_p", "_d", "_dx", "_dy"];
    _p = getPosASL (_this select 0);
    _d = getDir (_this select 0);
    _dx = ((_this select 1) select 0) - (_p select 0);
    _dy = ((_this select 1) select 1) - (_p select 1);
    [_dx * cos _d - _dy * sin _d, _dx * sin _d + _dy * cos _d, ((_this select 1) select 2) - (_p select 2)]
};

// [unit, [right, forward, up]] call KJ_fnToWorld : [x, y, zASL]
KJ_fnToWorld = {
    private ["_p", "_d", "_o"];
    _p = getPosASL (_this select 0);
    _d = getDir (_this select 0);
    _o = _this select 1;
    [(_p select 0) + (_o select 1) * sin _d + (_o select 0) * cos _d, (_p select 1) + (_o select 1) * cos _d - (_o select 0) * sin _d, (_p select 2) + (_o select 2)]
};

// [unit, "memory point", [right, forward, up] fallback] call KJ_fnPoint : [x, y, zASL].
// The point in his CURRENT pose on the kaiju binary; the fallback offset turned
// with his body on the stock one (or when the model has no such point).
KJ_fnPoint = {
    private ["_p"];
    _p = [];
    if (KJ_native) then { _p = kjPoint [_this select 0, _this select 1] };
    if ((count _p) != 3) then { _p = [_this select 0, _this select 2] call KJ_fnToWorld };
    _p
};

// unit call KJ_fnFurnace : where the furnace is now, [x, y, zASL]
KJ_fnFurnace = { [_this, KJ_furnacePoint, KJ_furnaceOfs] call KJ_fnPoint };

// [victim, owner] call KJ_fnKill : destroys or kills it. On the kaiju binary
// the damage goes through the engine's own path first, so the owner is
// credited (Killed, reveal) and a felled map object logs no error; whatever is
// then still standing gets setDammage, as on the stock binary.
KJ_fnKill = {
    private ["_o"];
    _o = _this select 0;
    if (KJ_native) then { kjDamage [_o, _this select 1, 0, 200000, 8] };
    if ((getDammage _o) < 1) then { _o setDammage 1 }
};

// [object, [vx, vy, vz]] call KJ_fnThrow : a velocity change. An impulse
// through the physics on the kaiju binary (a man is thrown, a hull tumbles);
// setVelocity on the stock one (moves vehicles, not men).
KJ_fnThrow = {
    if (KJ_native) then { kjImpulse [_this select 0, _this select 1] } else { (_this select 0) setVelocity (_this select 1) }
};

// [[x, y, z above ground], peak, seconds, full radius, zero radius, delay] call KJ_fnShake
// A tremor at a place. The engine's camera shake on the kaiju binary (every
// camera, nothing in the world moves); the roar feature's stand-in on the
// stock one, at the strength the listener feels at his distance.
KJ_fnShake = {
    private ["_p", "_peak", "_full", "_far", "_ear", "_d", "_s", "_probe"];
    _p = _this select 0;
    _peak = _this select 1;
    _full = _this select 3;
    _far = _this select 4;
    if (KJ_native) then {
        [_peak, _this select 2, _this select 5, [_p, _peak, _full, _far]] exec "kj\roar\shake.sqs"
    } else {
        if (isNil "KJR_ready") then { call compile preprocessFile "kj\roar\init.sqf" };
        _ear = call KJR_fnEar;
        _d = sqrt ((((getPos _ear) select 0) - (_p select 0)) ^ 2 + (((getPos _ear) select 1) - (_p select 1)) ^ 2);
        _s = 0;
        if (_d < _far) then { _s = (_far - _d) / (_far - _full) };
        if (_s > 1) then { _s = 1 };
        _s = _s * _s * _peak;
        if (_s > 0.01) then { [_s, _this select 2, _this select 5] exec "kj\roar\shake.sqs" }
    }
};

// ---- he can die (owner, 2026-11-12) -----------------------------------------
// KJ_unkillable  THE ONE SWITCH for the crush feature's heal loop. true: every
//                read heals him and a killing blow is undone (kj\crush\hurt.sqs).
//                false: hurt.sqs does not run, the engine's damage ceiling is
//                lifted, and whoever owns his health (the combat feature,
//                kj\combat\start.sqs) raises KJ_hurt, KJ_woundStage and KJ_dead.
//                kj\start.sqs sets it false by itself when the combat feature is
//                in the mission; a mission may set it before kj\start.sqs.
// KJ_dead        true once he is dead for good. Everything of the set stops
//                (kj\start.sqs watches): the brain, the walk, the crush loop,
//                the beam, the furnace light, the drone.
// KJ_woundStage  the combat feature's: how badly he is hurt (0 unhurt, rising).
if (isNil "KJ_unkillable") then { KJ_unkillable = true };
if (isNil "KJ_dead") then { KJ_dead = false };
KJ_hasCombat = false;

// call KJ_fnWound : his wound stage as a number (0 when nobody keeps one, or it is not a number)
KJ_fnWound = {
    private ["_w", "_s"];
    _w = 0;
    if (!(isNil "KJ_woundStage")) then {
        _s = format ["%1", KJ_woundStage];
        if ((substr [_s, 0, 1]) in ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]) then { _w = call compile _s }
    };
    _w
};

// call KJ_fnPace : how fast he should move now, 1..0 of his usual pace (the combat feature's
// KJ_woundPace; 1 when nobody keeps one)
KJ_fnPace = {
    private ["_p", "_s"];
    _p = 1;
    if (!(isNil "KJ_woundPace")) then {
        _s = format ["%1", KJ_woundPace];
        if ((substr [_s, 0, 1]) in ["0", "1"]) then { _p = call compile _s }
    };
    _p
};

KJ_coreReady = true;
