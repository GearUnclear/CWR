r"""Integration changes to the brain feature's payload (kj\brain), applied once to the COPY of
features\brain\kj\brain. Run from anywhere:  python tools\patch_brain.py
What changes, and why, is in kaiju-notes\integration\NOTES.md ("What changed in each feature")."""
from patchlib import File

f = File(r"brain\init.sqf")

# ---- tuning ---------------------------------------------------------------------------------
f.after('''if (isNil "KJB_LOOK") then { KJB_LOOK = 1.2 };               // strides ahead that must be dry before he takes the next one
''', '''if (isNil "KJB_WADE_MAX") then { KJB_WADE_MAX = 0.42 };      // [integration] when he may wade (KJB_wade): the deepest water, in heights, he takes another stride into
if (isNil "KJB_START_FIX") then { KJB_START_FIX = true };    // [integration] hold the standing foot where it is while the walk blends in (KJB_fnStartFix)
''')
f.after('''if (isNil "KJB_HIT_GAP") then { KJB_HIT_GAP = 14 };          // he does not stop for a second hit sooner than this
''', '''if (isNil "KJB_HIT_REACT") then { KJB_HIT_REACT = true };    // [integration] a hard hit makes him shudder and turn even while a director drives him (KJ_brainAuto false)
''')
f.rep('''if (isNil "KJB_STOMP_R") then { KJB_STOMP_R = 0.55 };        // a crewed tank nearer than this in front of him is stomped''',
      '''if (isNil "KJB_STOMP_R") then { KJB_STOMP_R = 0.4 };         // a crewed tank nearer than this in front of him is stomped ([integration] the reach of the stomping foot of the clip)''')

# ---- private state ----------------------------------------------------------------------------
f.after('''KJB_dormantSecs = 16;
''', '''// [integration]
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
''')

# ---- water: depth, and the line check the mover uses ---------------------------------------------
f.before('''// anything call KJB_fnIsArr : true for an array (this evaluator has no typeName)''',
         '''// [integration] call KJB_fnDepth : metres of water he stands in now (0 on land). The probe
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
        _ok = KJ_brainDepth < (KJB_WADE_MAX * KJB_H)
    };
    _ok
};

''')
f.rep('''                    if ([_pos, _dir, KJB_LOOK * KJB_stride] call KJB_fnDryLine) then {
                        call KJB_fnStartWalk''',
      '''                    if ([_pos, _dir, KJB_LOOK * KJB_stride] call KJB_fnPassLine) then {
                        call KJB_fnStartWalk''')
f.rep('''                if (!([[_pos, _dir, _left] call KJB_fnAhead, _dir, KJB_LOOK * KJB_stride] call KJB_fnDryLine)) then {''',
      '''                if (!([[_pos, _dir, _left] call KJB_fnAhead, _dir, KJB_LOOK * KJB_stride] call KJB_fnPassLine)) then {''')

# ---- the start of a walk: hold the standing foot ---------------------------------------------------
f.rep('''    KJB_walkT = time;
    KJB_mv = "walk";
    KJB_k playMove KJB_mWalk
};''', '''    KJB_walkT = time;
    KJB_mv = "walk";
    if (KJB_START_FIX) then { call KJB_fnStartFixBegin };
    KJB_k playMove KJB_mWalk
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
    if (KJ_native) then { if (KJ_hasFootPoints) then { KJB_fixFoot = kjPoint [KJB_k, KJB_plantPt] } };
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
        _add = KJB_walkSpeed * KJB_blendSecs * 0.5 * _t * _t - KJB_fixDone
    };
    if (_add > 0.005) then {
        if (_add > 0.4) then { _add = 0.4 };
        _p = getPosASL KJB_k;
        KJB_k setPosASL [(_p select 0) + _add * sin _dir, (_p select 1) + _add * cos _dir, _p select 2];
        KJB_fixDone = KJB_fixDone + _add;
        KJ_brainStartFix set [1, KJB_fixDone]
    }
};''')
f.rep('''    if (KJB_mv == "walk") then {
        _w = [KJB_legStart, _pos] call KJB_fnDist;
        _mv = getMove _k;''', '''    if (KJB_mv == "walk") then {
        if (_now < KJB_fixUntil) then { call KJB_fnStartFix; _pos = getPos _k };
        _w = [KJB_legStart, _pos] call KJB_fnDist;
        _mv = getMove _k;''')

# ---- sequences: a step that turns him to a bearing worked out when it starts ---------------------
f.rep('''//   ["face", posOrObj]    turn on the spot to face it
''', '''//   ["face", posOrObj]    turn on the spot to face it
//   ["bear", {code}]      [integration] turn on the spot to the bearing the code gives
''')
f.rep('''            if (_kind == "move") then { if ((_s select 1) != "") then { KJB_k playMove (_s select 1) } };''',
      '''            if (_kind == "bear") then { KJB_face = call (_s select 1); KJB_faced = false };
            if (_kind == "move") then { if ((_s select 1) != "") then { KJB_k playMove (_s select 1) } };''')
f.rep('''        if (_kind == "face") then { _done = KJB_faced and (KJB_mv == "stand") };''',
      '''        if (_kind == "face") then { _done = KJB_faced and (KJB_mv == "stand") };
        if (_kind == "bear") then { _done = KJB_faced and (KJB_mv == "stand") };''')

# ---- the hit signal: looked at on every tick, not every two seconds --------------------------------
f.rep('''    // the hit signal (the crush feature's hurt.sqs, or the brain's own stub)
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
};''', '''    // [integration] the hit signal is KJB_fnHitCheck, on every tick
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
};''')
f.rep('''    if (KJ_brainAuto and (_sev >= KJB_HIT_HARD) and ((time - KJB_lastHit) >= KJB_HIT_GAP) and (KJ_state in ["idle", "wander", "drawn", "dormant"])) then {''',
      '''    if ((KJ_brainAuto or KJB_HIT_REACT) and (_sev >= KJB_HIT_HARD) and ((time - KJB_lastHit) >= KJB_HIT_GAP) and (KJ_state in ["idle", "wander", "drawn", "dormant"])) then {''')
f.rep('''        } else {
            KJ_state = "drawn";
            KJ_brainPhase = "walking";
            KJB_drawI = -1;
            KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
            KJ_brainDraw call KJB_fnGo
        }''', '''        } else {
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
        }''')
f.rep('''        call KJB_fnFollow;
        if (_now >= KJB_tSense) then {''', '''        call KJB_fnFollow;
        call KJB_fnHitCheck;
        if (_now >= KJB_tSense) then {''')

# ---- stomp and swat: in step with the clips --------------------------------------------------------
f.rep('''    if (_s == "stomp") then {
        [[["phase", "stopping"], ["halt"], ["phase", "turning"], ["face", KJB_blowTgt], ["phase", "stomping"], ["call", { call KJB_fnDoStomp }], ["wait", KJB_BLOW_SECS]], "idle"] call KJB_fnSeq
    };
    if (_s == "swat") then {
        [[["phase", "stopping"], ["halt"], ["phase", "swatting"], ["call", { call KJB_fnDoSwat }], ["wait", KJB_BLOW_SECS]], "idle"] call KJB_fnSeq
    };''', '''    if (_s == "stomp") then {
        // [integration] he turns so that the stomping foot of the clip comes down on it, then waits the clip out
        [[["phase", "stopping"], ["halt"], ["phase", "turning"], ["bear", { call KJB_fnStompBear }], ["phase", "stomping"], ["call", { call KJB_fnDoStomp }], ["wait", KJB_stompSecs]], "idle"] call KJB_fnSeq
    };
    if (_s == "swat") then {
        // [integration] the swat is the right arm, swung from his right rear round to dead ahead:
        // something outside that sweep is turned to first
        [[["phase", "stopping"], ["halt"], ["phase", "turning"], ["bear", { call KJB_fnSwatBear }], ["phase", "swatting"], ["call", { call KJB_fnDoSwat }], ["wait", KJB_swatSecs]], "idle"] call KJB_fnSeq
    };''')
f.before('''KJB_fnDoStomp = {''', '''// [integration] the bearing he must face for the stomping foot to come down on the target
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

''')

f.save()

# ---------------------------------------------------------------------------------------------------
s = File(r"brain\start.sqs")
s.rep(''';  kj\\brain\\start.sqs  -  Dedushka BRAIN: how he moves and what he decides.
;  Runs for his lifetime.
'''.replace("\\\\", "\\"), ''';  kj\\brain\\start.sqs  -  Dedushka BRAIN: how he moves and what he decides.
;  Runs for his lifetime. INTEGRATED set: started by kj\\start.sqs, which also
;  loads the shared core (kj\\init.sqf). Changes against the brain feature's own
;  file are marked [integration].
'''.replace("\\\\", "\\"))
s.rep('''call compile preprocessFile "kj\\brain\\init.sqf"
KJ_brainOn = true
KJB_k = _k
'''.replace("\\\\", "\\"), '''call compile preprocessFile "kj\\brain\\init.sqf"
; [integration] the shared core, when this is started without kj\\start.sqs
? isNil "KJ_coreReady" : call compile preprocessFile "kj\\init.sqf"
? isNull KJ_unit : _k call KJ_fnSetup
KJ_brainOn = true
KJB_k = _k
'''.replace("\\\\", "\\"))
s.rep('''? KJB_mWalk == "DedWalk" : KJB_strideSecs = KJB_strideSecs / 0.55
''', '''? KJB_mWalk == "DedWalk" : KJB_strideSecs = KJB_strideSecs / 0.55

; ---- [integration] the start of a walk: which foot he stands on while the first
; ---- stride starts (the other one of the walk's first footfall), how long the
; ---- blend lasts, how fast he walks. With the fix on, the first stride loses no
; ---- ground to the blend, and the crush feature's legs are not ahead of his body.
KJB_walkSpeed = KJB_stride / KJB_strideSecs
? not (isNil "KJ_animWalkBlend") : KJB_blendSecs = KJ_animWalkBlend
? not (isNil "KJ_animWalkFalls") : if (((KJ_animWalkFalls select 0) select 1) == "L") then { KJB_plantPt = "stopaP" }
? KJB_START_FIX : KJB_strideLoss = 0; KJC_START_LAG = 0

; ---- [integration] wading: the engine's wadeDepth trait lets him stand in the sea
; ---- unhurt; a mission can say so itself (KJ_brainWade) on a binary without it
? KJ_native : if ((kjGet [_k, "wadeDepth"]) > 0) then { KJB_wade = true }
? not (isNil "KJ_brainWade") : KJB_wade = KJ_brainWade

; ---- [integration] stomp and swat follow the clips: where the stomping foot lands
; ---- (to the right of dead ahead), and how long each clip holds him
? not (isNil "KJ_animStompPos") : KJB_stompAng = (KJ_animStompPos select 0) atan2 (KJ_animStompPos select 1); KJB_stompSecs = KJ_animStompSecs + 1
? not (isNil "KJ_animSwatSecs") : KJB_swatSecs = KJ_animSwatSecs + 0.5
''')
s.rep('''["start", format ["features roar %1 beam %2 crush %3, auto %4, first state %5", KJB_hasRoar, KJB_hasBeam, KJB_hasCrush, KJ_brainAuto, _first]] call KJB_fnLog''',
      '''["start", format ["features roar %1 beam %2 crush %3, auto %4, first state %5, wade %6, start fix %7 on %8, stomp angle %9", KJB_hasRoar, KJB_hasBeam, KJB_hasCrush, KJ_brainAuto, _first, KJB_wade, KJB_START_FIX, KJB_plantPt, KJB_stompAng]] call KJB_fnLog''')
s.save()
