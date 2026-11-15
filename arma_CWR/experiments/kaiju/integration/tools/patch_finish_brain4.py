r"""Finishing round (2026-11-12), fourth brain patch, after two fights on the Hound without a blow
(gate-fight-d-r3 of 09:33, g2-fight-d-r2 of 09:59): each time the tank sent to his feet was still on
its way in, 17 to 25 m off, when he set off walking, and he trod on it.

A crewed tank near his feet holds him where he stands. While one is within KJB_TANK_HOLD of the
reach of his stomp (about 25 m) and he may strike (the gap since his last blow is over), he does not
set off (not after a hit, not on a wander, not drawn, not on the retreat) and a walk he is on ends
at the end of the stride; when it comes into reach he stomps it (the blow rule, all round). A tank
that stays out of reach holds him KJB_TANK_HOLD_SECS at most, then he goes his way.

    python patch_finish_brain4.py
"""
from patchlib import File

f = File(r"brain\init.sqf")
f.rep('''if (isNil "KJB_BLOW_ARC") then''',
      '''if (isNil "KJB_TANK_HOLD") then { KJB_TANK_HOLD = 1.6 };       // [finishing] a crewed tank within this many stomp reaches of his feet holds him where he stands (he waits for it, he does not walk over it)
if (isNil "KJB_TANK_HOLD_SECS") then { KJB_TANK_HOLD_SECS = 30 }; // [finishing] for this long at most, when it stays out of reach
if (isNil "KJB_BLOW_ARC") then''')
f.rep('''// [finishing] call KJB_fnTankAtFeet : a crewed, live tank within the reach of his stomp and in
// front of him (the object), or objNull''',
      '''// [finishing] call KJB_fnTankHolds : true while a crewed, live tank near his feet holds him where
// he stands (see KJB_TANK_HOLD). KJB_tankSince is when the one that holds him now was first seen.
KJB_tankSince = -1;
KJ_brainTankHold = false;
KJB_fnTankHolds = {
    private ["_n", "_kp", "_h"];
    _n = objNull;
    _h = false;
    _kp = getPos KJB_k;
    if ((time - KJB_lastBlow) >= KJB_BLOW_GAP) then {
        { if ((isNull _n) and (alive _x) and ((count (crew _x)) > 0)) then { _n = _x } } forEach (nearestObjects [_kp, ["Tank"], KJB_TANK_HOLD * KJB_STOMP_R * KJB_H])
    };
    if (isNull _n) then { KJB_tankSince = -1 } else {
        if (KJB_tankSince < 0) then { KJB_tankSince = time };
        _h = (time - KJB_tankSince) < KJB_TANK_HOLD_SECS
    };
    KJ_brainTankHold = _h;
    _h
};

// [finishing] call KJB_fnTankAtFeet : a crewed, live tank within the reach of his stomp and in
// front of him (the object), or objNull''')
# the decide: the hold is worked out first; a walk ends on it
f.rep('''    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;
''',
      '''    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;
    // [finishing] a crewed tank near his feet holds him: a walk ends at the end of the stride
    _hold = call KJB_fnTankHolds;
    if (_hold and (KJ_state in ["drawn", "wander"]) and ((count KJB_seq) == 0)) then {
        ["hold", format ["a crewed tank near his feet at %1: he stops for it", _kp]] call KJB_fnLog;
        KJB_retreatWalking = false;
        "idle" call KJB_fnEnter
    };
''')
f.rep('''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret", "_free"];''',
      '''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret", "_free", "_hold"];''')
# the retreat walk does not begin while he is held
f.rep('''                if (!(isNull _e)) then {
                    KJB_blowTgt = _e;
                    "stomp" call KJB_fnEnter
                } else {
                    "drawn" call KJB_fnEnter;
                    KJB_drawI = -1;
                    KJ_brainDraw = [_p select 0, _p select 1];
                    KJ_brainDraw call KJB_fnGo;
                    KJB_lastDrawn = time;
                    KJB_retreatWalking = true
                }''',
      '''                if (!(isNull _e)) then {
                    KJB_blowTgt = _e;
                    "stomp" call KJB_fnEnter
                } else {
                    if (!_hold) then {
                        "drawn" call KJB_fnEnter;
                        KJB_drawI = -1;
                        KJ_brainDraw = [_p select 0, _p select 1];
                        KJ_brainDraw call KJB_fnGo;
                        KJB_lastDrawn = time;
                        KJB_retreatWalking = true
                    }
                }''')
# idle: he does not set off while he is held
f.rep('''    if ((KJ_state == "idle") and (!_ret)) then {
        if ((count KJB_seq) == 0) then {
            _i = call KJB_fnPickDraw;''',
      '''    if ((KJ_state == "idle") and (!_ret) and (!_hold)) then {
        if ((count KJB_seq) == 0) then {
            _i = call KJB_fnPickDraw;''')
# after a hit: he does not set off while he is held
f.rep('''                if (!(isNull _p)) then {
                    KJB_blowTgt = _p;
                    "stomp" call KJB_fnEnter
                } else {
                    KJ_state = "drawn";
                    KJ_brainPhase = "walking";
                    KJB_drawI = -1;
                    KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
                    KJ_brainDraw call KJB_fnGo
                }''',
      '''                if (!(isNull _p)) then {
                    KJB_blowTgt = _p;
                    "stomp" call KJB_fnEnter
                } else {
                    if (call KJB_fnTankHolds) then {
                        // a crewed tank is coming in at his feet: he stands and waits for it
                        "idle" call KJB_fnEnter
                    } else {
                        KJ_state = "drawn";
                        KJ_brainPhase = "walking";
                        KJB_drawI = -1;
                        KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
                        KJ_brainDraw call KJB_fnGo
                    }
                }''')
f.save()
