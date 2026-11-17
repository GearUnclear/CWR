r"""Finishing round (2026-11-12), second brain patch, after the first fight with the last stand
(run fight-a1): he stopped in 7.7 m of water, not 4.9 (he could only feel the depth he stood in,
a stride late), and the tank sent to his feet was trodden on as he set off after a hit.

1. On a kaiju binary of version 2 the surf limit is read AHEAD (kjGround: the sea bed under the
   end of the next stride), so the retreat ends in water no deeper than KJB_BAY_WADE. Elsewhere,
   as before, by the depth he stands in.
2. Setting off after a hit ("hunt"), he first looks at his feet: a crewed tank in his reach is
   stomped, not walked over.

    python patch_finish_brain2.py
"""
from patchlib import File

f = File(r"brain\init.sqf")
f.rep('''        // [finishing] on the retreat and at bay: no deeper than the surf
        if (KJB_retreatWalking or KJB_bay) then { _ok = KJ_brainDepth < (KJB_BAY_WADE * KJB_H) }
    };
    _ok''',
      '''        // [finishing] on the retreat and at bay: no deeper than the surf
        if (KJB_retreatWalking or KJB_bay) then {
            _ok = KJ_brainDepth < (KJB_BAY_WADE * KJB_H);
            // engine version 2 reads the sea bed ahead: the stride is not taken when its end is deeper than the surf
            if (KJ_native) then { if (KJ_engineVersion >= 2) then {
                _a = [_this select 0, _this select 1, KJB_stride] call KJB_fnAhead;
                _g = kjGround [_a select 0, _a select 1];
                if ((count _g) == 2) then { KJ_brainDepthAhead = _g select 1; _ok = (_g select 1) < (KJB_BAY_WADE * KJB_H) }
            } }
        }
    };
    _ok''')
f.rep('''KJB_fnPassLine = {
    private ["_ok"];''', '''KJ_brainDepthAhead = -1;
KJB_fnPassLine = {
    private ["_ok", "_a", "_g"];''')

# a crewed tank in his reach, in front of him: the object, or objNull
f.rep('''// what he decides by himself (KJ_brainAuto), once a second
KJB_fnDecide = {''', '''// [finishing] call KJB_fnTankAtFeet : a crewed, live tank within the reach of his stomp and in
// front of him (the object), or objNull
KJB_fnTankAtFeet = {
    private ["_t", "_kp"];
    _t = objNull;
    _kp = getPos KJB_k;
    {
        if ((isNull _t) and (alive _x) and ((count (crew _x)) > 0)) then {
            if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < 70) then { _t = _x }
        }
    } forEach (nearestObjects [_kp, ["Tank"], KJB_STOMP_R * KJB_H]);
    _t
};

// what he decides by himself (KJ_brainAuto), once a second
KJB_fnDecide = {''')
f.rep('''            if (KJ_brainAuto) then {
                KJ_state = "drawn";
                KJ_brainPhase = "walking";
                KJB_drawI = -1;
                KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
                KJ_brainDraw call KJB_fnGo
            } else {''', '''            if (KJ_brainAuto) then {
                // [finishing] before he sets off: a crewed tank at his feet is stomped, not walked over
                _p = objNull;
                if ((time - KJB_lastBlow) >= KJB_BLOW_GAP) then { _p = call KJB_fnTankAtFeet };
                if (!(isNull _p)) then {
                    KJB_blowTgt = _p;
                    "stomp" call KJB_fnEnter
                } else {
                    KJ_state = "drawn";
                    KJ_brainPhase = "walking";
                    KJB_drawI = -1;
                    KJ_brainDraw = [KJB_hitFrom select 0, KJB_hitFrom select 1];
                    KJ_brainDraw call KJB_fnGo
                }
            } else {''')
f.save()

# the combat feature's copy is not touched here: settle.sqs is changed at source (features\combat)
