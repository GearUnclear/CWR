r"""Integration changes to the crush feature's payload (kj\crush), applied once to the COPY of
features\crush\kj\crush. Run from anywhere:  python tools\patch_crush.py
What changes, and why, is in kaiju-notes\integration\NOTES.md ("What changed in each feature")."""
from patchlib import File

# =========================================================================================== init.sqf
f = File(r"crush\init.sqf")
f.rep('''//  Works on the stock engine binary. No engine change is needed or assumed.''',
      '''//  Works on the stock engine binary. INTEGRATED set: on the kaiju binary (KJ_native, kj\\init.sqf)
//  the footfalls are the engine's own events, kills are credited to him, men and hulls are
//  thrown through the physics and hits come with the shooter; every one of those keeps its
//  stock fallback. Changes against the crush feature's own file are marked [integration].''')
f.rep('''if (isNil "KJC_MAN_LANE") then { KJC_MAN_LANE = 0.08 };       // a man this close to the middle of a foot lane is under the foot''',
      '''if (isNil "KJC_MAN_LANE") then { KJC_MAN_LANE = 0.13 };       // a man this close to the middle of a foot lane is under the foot ([integration] was 0.08: a man 4 m off the middle still stood in the way of the foot and shoved him)''')
f.after('''if (isNil "KJC_SINK") then { KJC_SINK = 1.1 };               // m a hull is pressed in when the data has no wreck class for it
''', '''if (isNil "KJC_STAND_R") then { KJC_STAND_R = 0.16 };        // [integration] while he stands or turns: a man or a hull this close to one of his feet is cleared away (it would shove him)
if (isNil "KJC_STAND_THROW") then { KJC_STAND_THROW = 7 };   // [integration] m/s a man is thrown clear of a standing foot (kaiju binary); on the stock binary he is put this far out in heights x 0.05
''')
f.rep('''KJC_stepFile = (loadFile "kj\\roar\\step.sqs") != "";           // the roar feature's footfall, when it is there''',
      '''KJC_stepFile = (loadFile "kj\\roar\\step.sqs") != "";           // the roar feature's footfall, when it is there
// [integration] the shared core (which engine, kill with credit, throw, memory points)
if (isNil "KJ_coreReady") then { call compile preprocessFile "kj\\init.sqf" };
KJC_nativeSteps = false;                                       // true: footfalls are the engine's events (kj\\crush\\steps.sqs)
KJC_stepFlag = false;                                          // steps.sqs to loop.sqs: a real footfall has just landed
KJC_stepNext = 1;                                              // the foot that lands next: 1 right, -1 left
KJ_crushPushes = [];                                           // [[time, what, metres from the foot], ...] things cleared from his standing feet''')

# kills go through the core (credited on the kaiju binary)
f.rep('''                    if ((vehicle _x) == _x) then { _x setDamage 1; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                }
            };
            if ((_kind == "tank") or (_kind == "car")) then {
                _toe = _toePl;''',
      '''                    if ((vehicle _x) == _x) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                }
            };
            if ((_kind == "tank") or (_kind == "car")) then {
                _toe = _toePl;''')
f.rep('''                if (_kind == "man") then {
                    if ((vehicle _x) == _x) then { _x setDamage 1; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                };
                if (((_kind == "tank") or (_kind == "car")) and !(_x getVariable ["kjcFlat", false])) then {''',
      '''                if (_kind == "man") then {
                    if ((vehicle _x) == _x) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] }
                };
                if (((_kind == "tank") or (_kind == "car")) and !(_x getVariable ["kjcFlat", false])) then {''')
f.rep('''                        if (_d < _rf) then { _x setDamage 1; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] } else {''',
      '''                        if (_d < _rf) then { [_x, _k] call KJ_fnKill; KJ_crushStats set [3, (KJ_crushStats select 3) + 1] } else {''')

# a thrown man: really thrown on the kaiju binary
f.rep('''KJC_fnThrowMan = {
    private ["_m"];
    _m = _this select 0;
    if ((vehicle _m) == _m) then {
        if (_this select 3) then { _m setDamage 1 } else { [_m, 3 + 4 * (_this select 2)] exec "kj\\crush\\down.sqs" }
    }
};''', '''// [integration] On the kaiju binary he IS thrown (kjImpulse: away from the point and up),
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
            _v = 4 + 6 * (_this select 2);
            [_m, [_v * _dx / _d, _v * _dy / _d, 2.5 + 2.5 * (_this select 2)]] call KJ_fnThrow
        };
        if (_this select 3) then { [_m, KJ_unit] call KJ_fnKill } else { [_m, 3 + 4 * (_this select 2)] exec "kj\\crush\\down.sqs" }
    }
};''')
f.rep('''    if (_d < 0.5) then { _dx = sin (random 360); _dy = cos (random 360); _d = 1 };
    _o setVelocity [_v * _dx / _d, _v * _dy / _d, _up]
};''', '''    if (_d < 0.5) then { _dx = sin (random 360); _dy = cos (random 360); _d = 1 };
    // [integration] through the physics on the kaiju binary (a hull tumbles), setVelocity on the stock one
    [_o, [_v * _dx / _d, _v * _dy / _d, _up]] call KJ_fnThrow
};''')
f.rep('''    _v setVelocity [_side * KJC_SWING_V * cos _d + 2 * sin _d, 2 * cos _d - _side * KJC_SWING_V * sin _d, 3];''',
      '''    [_v, [_side * KJC_SWING_V * cos _d + 2 * sin _d, 2 * cos _d - _side * KJC_SWING_V * sin _d, 3]] call KJ_fnThrow;''')

# vehicles in a foot lane: a wreck, or a hull that was kicked and is still there, is flattened
f.rep('''                    if (!(_x getVariable ["kjcFlat", false]) and ((time - (_x getVariable ["kjcKickT", -100])) > 2)) then {
                        if ((abs (_s - KJC_STANCE * KJC_H)) <= KJC_FOOT_HALF * KJC_H) then {''',
      '''                    // [integration] a wreck (the beam's, a stomp's) cannot be relied on to slide when
                    // kicked, and a hull that was kicked and is still in the lane did not: both are
                    // flattened, which takes the hull away (a hull left in a lane stops him and
                    // throws him back metres at a stride)
                    if (!(_x getVariable ["kjcFlat", false]) and ((time - (_x getVariable ["kjcKickT", -100])) > 0.7)) then {
                        if (((abs (_s - KJC_STANCE * KJC_H)) <= KJC_FOOT_HALF * KJC_H) or ((getDammage _x) >= 1) or ((_x getVariable ["kjcKickT", -100]) > 0)) then {''')

# the hit signal can carry the shooter straight from the engine's hit report
f.before('''// Event handler bodies, kept as globals so a mission can see what is wired.''',
         '''// [integration] [kaiju] call KJC_fnStandGuard : while he stands or turns on the spot. A man
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
                            [_o, 4] exec "kj\\crush\\down.sqs"
                        };
                        KJ_crushStats set [4, (KJ_crushStats select 4) + 1]
                    };
                    if ((_kind == "car") or (_kind == "tank")) then {
                        if ((count KJ_crushPushes) < 60) then { KJ_crushPushes = KJ_crushPushes + [[time, _kind, _best]] };
                        if ((getDammage _o) >= 1) then {
                            _o setVariable ["kjcFlat", true];
                            [_o, [_bf select 0, _bf select 1]] exec "kj\\crush\\flatten.sqs"
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

''')
f.save()

# =========================================================================================== loop.sqs
f = File(r"crush\loop.sqs")
f.rep('''[_k] exec "kj\\crush\\hurt.sqs"
''', '''[_k] exec "kj\\crush\\hurt.sqs"
; [integration] kaiju binary: the footfalls are the engine's own (the walk's, the turn's and the
; stomp's sound edges, with the real foot and where it is), read by steps.sqs. The arithmetic
; below then only keeps the stride count for the strip ahead (which toes are where).
? KJ_native : KJC_nativeSteps = true
? KJC_nativeSteps : [_k] exec "kj\\crush\\steps.sqs"
''')
f.rep('''; only ground gained counts: when a hull shoves him back the clip plays on
? _fwd > 0 : _acc = _acc + _fwd
''', '''; only ground gained counts: when a hull shoves him back the clip plays on
? _fwd > 0 : _acc = _acc + _fwd
; [integration] a real footfall has just landed: the count starts again from it
? KJC_stepFlag : _acc = 0; _side = KJC_stepNext; KJC_stepFlag = false
''')
f.rep('''? _acc < _half : goto "sleep"
_acc = _acc - _half
''', '''? KJC_nativeSteps : goto "sleep"
? _acc < _half : goto "sleep"
_acc = _acc - _half
''')
f.rep('''#stand
_still = _still + 1
? (_still mod 4) == 1 : [_k, false, 0, 0, _side] call KJC_fnWalkPass
''', '''#stand
_still = _still + 1
? (_still mod 4) == 1 : [_k, false, 0, 0, _side] call KJC_fnWalkPass
; [integration] nothing may stand against his feet (it would shove him): every pass
[_k] call KJC_fnStandGuard
KJC_stepFlag = false
''')
f.save()

# =========================================================================================== hurt.sqs
f = File(r"crush\hurt.sqs")
f.rep('''_watch = -100
_d = 0
''', '''_watch = -100
_d = 0
_hits = []
_h = []
; [integration] kaiju binary: every hit comes from the engine's own hit report (the hitReport
; trait): who fired, with what, how much it added. The damage ceiling trait already keeps any
; one blow from killing him; he is still healed here so nothing adds up or cripples a zone.
''')
f.rep('''#live
_d = getDammage _k
? _d < KJC_HURT_MIN : goto "watch"
? (time - KJC_hitEvent) > 0.5 : [_k, _d, objNull] call KJC_fnHurt
_k setDamage 0
''', '''#live
? KJ_native : goto "report"
_d = getDammage _k
? _d < KJC_HURT_MIN : goto "watch"
? (time - KJC_hitEvent) > 0.5 : [_k, _d, objNull] call KJC_fnHurt
_k setDamage 0
goto "watch"

#report
? KJ_native : _hits = kjHits _k
? (count _hits) == 0 : goto "healed"
; the heaviest hit of this read is the one he feels
_h = _hits select 0
{ if ((_x select 3) > (_h select 3)) then { _h = _x } } forEach _hits
KJ_hurtReport = _h
? (_h select 3) >= KJC_HURT_MIN : [_k, _h select 3, _h select 0] call KJC_fnHurt
#healed
? (getDammage _k) > 0 : _k setDamage 0
''')
f.save()

# =========================================================================================== fell.sqs, flatten.sqs
f = File(r"crush\fell.sqs")
f.rep('''~KJC_DUST_BEAT
_o setDamage 1
''', '''~KJC_DUST_BEAT
; [integration] through the engine's damage path on the kaiju binary: credited to him, and no error line per felled map object
[_o, KJ_unit] call KJ_fnKill
''')
f.rep('''#small
_o setDamage 1
''', '''#small
[_o, KJ_unit] call KJ_fnKill
''')
f.save()

f = File(r"crush\flatten.sqs")
f.rep('''{ _x setDamage 1 } forEach (crew _v)
[[_p select 0, _p select 1], 3, 6, 2, 8, 5, 1.5, 3, [0.3, 0.29, 0.27], 0.6, 1] call KJC_fnDust''',
      '''{ [_x, KJ_unit] call KJ_fnKill } forEach (crew _v)
[[_p select 0, _p select 1], 3, 6, 2, 8, 5, 1.5, 3, [0.3, 0.29, 0.27], 0.6, 1] call KJC_fnDust''')
f.save()

# =========================================================================================== stomp.sqs
f = File(r"crush\stomp.sqs")
f.rep('''_reach = KJC_STOMP_REACH * KJC_H
_res = "hit"
? _d > _reach : _res = "short"
? _d > _reach : _pos = [(_kp select 0) + _dx * _reach / _d, (_kp select 1) + _dy * _reach / _d, 0]
_c = [_pos select 0, _pos select 1]
''', '''_reach = KJC_STOMP_REACH * KJC_H
_res = "hit"
_tgt = [_pos select 0, _pos select 1, 0]
_wind = KJC_STOMP_WIND
_fp = []
; [integration] with the animation's timing table the blow is the clip's: it lands when the
; stomping foot of DedStomp comes down (KJ_animStompTime) and WHERE that foot comes down, which
; is a fixed spot in front of him (the brain turns him so that it is on the target). The spot is
; read off his foot on the kaiju binary. Without the table: the feature's own rule (the point
; asked for, clamped to his reach, after KJC_STOMP_WIND).
? not (isNil "KJ_animStompTime") : goto "clip"
? _d > _reach : _res = "short"
? _d > _reach : _pos = [(_kp select 0) + _dx * _reach / _d, (_kp select 1) + _dy * _reach / _d, 0]
goto "planned"
#clip
_wind = KJ_animStompTime
_pos = [_k, [KJ_animStompPos select 0, KJ_animStompPos select 1, 0]] call KJ_fnToWorld
#planned
_c = [_pos select 0, _pos select 1]
''')
f.rep('''~KJC_STOMP_WIND
? not alive _k : goto "abort"
''', '''~_wind
? not alive _k : goto "abort"
? isNil "KJ_animStompTime" : goto "blow"
; [integration] where the foot really is now
_pos = [_k, [KJ_animStompPos select 0, KJ_animStompPos select 1, 0]] call KJ_fnToWorld
? KJ_native : if (KJ_hasFootPoints) then { if (KJ_animStompFoot == "L") then { _fp = kjPoint [_k, "stopaL"] } else { _fp = kjPoint [_k, "stopaP"] } }
? (count _fp) == 3 : _pos = _fp
_c = [_pos select 0, _pos select 1]
? ([_tgt, _c] call KJC_fnDist2) > _foot : _res = "short"
#blow
''')
f.save()

# =========================================================================================== swat.sqs
f = File(r"crush\swat.sqs")
f.rep('''#swing
KJ_swatting = true
KJ_swatResult = "swing"
KJ_swatHow = _r select 0
_k playMove "DedSwat"
~KJC_SWAT_WIND
''', '''#swing
KJ_swatting = true
KJ_swatResult = "swing"
KJ_swatHow = _r select 0
_wind = KJC_SWAT_WIND
_move = "DedSwat"
; [integration] with the animation's timing table the blow lands when the clip gets there: for
; the arm, when the hand of DedSwat is nearest to the target on its sweep (KJ_animSwatPath); for
; a kick, when the foot of DedStomp comes down (there is no kick clip).
? isNil "KJ_animSwatPath" : goto "wound"
? (_r select 0) == "kick" : _move = "DedStomp"; _wind = KJ_animStompTime; goto "wound"
_l = [_k, getPosASL _t] call KJ_fnToLocal
_best = 1000000
{ _d2 = (((_x select 1) select 0) - (_l select 0)) ^ 2 + (((_x select 1) select 1) - (_l select 1)) ^ 2 + (((_x select 1) select 2) - (_l select 2)) ^ 2; if (_d2 < _best) then { _best = _d2; _wind = _x select 0 } } forEach KJ_animSwatPath
#wound
KJ_swatWind = _wind
_k playMove _move
~_wind
''')
f.rep('''#air
; struck sideways and down: it leaves burning and falls
[_t, _c, KJC_SWAT_AIR_V, 0 - 0.45 * KJC_SWAT_AIR_V] call KJC_fnShove
_t setDamage 1
''', '''#air
; struck sideways and down: it leaves burning and falls
[_t, _c, KJC_SWAT_AIR_V, 0 - 0.45 * KJC_SWAT_AIR_V] call KJC_fnShove
[_t, _k] call KJ_fnKill
''')
f.rep('''#car
{ _x setDamage 1 } forEach (crew _t)
_t setDamage 1
''', '''#car
{ [_x, _k] call KJ_fnKill } forEach (crew _t)
[_t, _k] call KJ_fnKill
''')
f.save()
