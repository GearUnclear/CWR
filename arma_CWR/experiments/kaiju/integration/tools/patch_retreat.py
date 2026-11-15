r"""Thirteenth round, from the first real fight (runs\fight2-1112-043409): the combat feature raised
its sea retreat (KJ_woundRetreat, with a pull through KJB_fnDraw) at wound stage 3 and the brain never
went: every hard hit made him shudder, turn to the shooter and walk at it, and the beam's anger was
always earned, so he fought where he stood until he died 60 m from where he started.
Now the brain owns the retreat. While KJ_woundRetreat holds (badly hurt and still being hit) and he
decides for himself:
  - nothing else draws him: he walks for the sea, on past the shore as far as he can wade, and there
    he goes dormant (where the combat feature lets him mend three times as fast);
  - a hard hit no longer stops him to shudder and turn, and he does not stand to use the beam;
  - a blow at a crewed tank or an aircraft in his reach on the way is still his.
When the flag falls (90 s without a real hit) the brain is its old self.
Run:  python tools\patch_retreat.py"""
from patchlib import File

f = File(r"brain\init.sqf")

# the helpers, in front of the decisions
f.before('''// what he decides by himself (KJ_brainAuto), once a second
KJB_fnDecide = {''', '''// [integration] The combat feature's sea retreat (KJ_woundRetreat: badly hurt and still being
// hit). While it holds and he decides for himself, nothing else draws him, he does not stop to
// shudder or to use the beam: he walks for the sea, on past the shore as far as he can wade, and
// goes dormant there. A blow at something in his reach on the way is still his.
KJB_retreatGoal = [];
KJB_retreatWalking = false;
KJB_retreatStood = false;
KJB_fnRetreating = {
    private ["_r"];
    _r = false;
    if (KJ_brainAuto and (!(isNil "KJ_woundRetreat")) and (!(isNil "KJ_woundRetreatPos"))) then {
        if (KJ_woundRetreat and ((count KJ_woundRetreatPos) >= 2)) then { _r = true }
    };
    if (!_r) then { KJB_retreatGoal = []; KJB_retreatWalking = false; KJB_retreatStood = false };
    _r
};
// where he walks to: the combat feature's nearest point of the sea, and 4 of his heights on past it
KJB_fnRetreatGoal = {
    private ["_kp", "_dx", "_dy", "_d"];
    if ((count KJB_retreatGoal) == 0) then {
        _kp = getPos KJB_k;
        _dx = (KJ_woundRetreatPos select 0) - (_kp select 0);
        _dy = (KJ_woundRetreatPos select 1) - (_kp select 1);
        _d = sqrt (_dx * _dx + _dy * _dy);
        if (_d < 1) then { _d = 1; _dx = 0; _dy = -1 };
        KJB_retreatGoal = [(KJ_woundRetreatPos select 0) + 4 * KJB_H * _dx / _d, (KJ_woundRetreatPos select 1) + 4 * KJB_H * _dy / _d];
        ["retreat", format ["badly hurt, he turns for the sea: the shore at %1, he makes for %2 (he is at %3)", KJ_woundRetreatPos, KJB_retreatGoal, _kp]] call KJB_fnLog
    };
    KJB_retreatGoal
};

''')

# no shudder and turn while he retreats
f.rep('''and ((time - KJB_lastHit) >= (KJB_HIT_GAP / (1 + (call KJ_fnWound)))) and (KJ_state in ["idle", "wander", "drawn", "dormant"])) then {''',
      '''and ((time - KJB_lastHit) >= (KJB_HIT_GAP / (1 + (call KJ_fnWound)))) and (KJ_state in ["idle", "wander", "drawn", "dormant"]) and (!(call KJB_fnRetreating))) then {''')

# the decisions
f.rep('''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r"];
    _kp = getPos KJB_k;

    // the beam, from any waking state
    if (KJ_state in ["idle", "wander", "drawn"]) then {''',
      '''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret"];
    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;

    // the beam, from any waking state ([integration] not while he retreats to the sea)
    if ((KJ_state in ["idle", "wander", "drawn"]) and (!_ret)) then {''')

f.rep('''    if (KJ_state == "dormant") then {
        if ((count KJB_seq) == 0) then {
            _f = false;''',
      '''    // [integration] the sea retreat: nothing else draws him (see KJB_fnRetreating)
    if (_ret) then {
        _p = call KJB_fnRetreatGoal;
        if ((KJ_state in ["idle", "wander"]) and ((count KJB_seq) == 0)) then {
            if (KJB_retreatStood) then {
                // he is there, or as deep as he can wade: he goes still (and mends, if they let him)
                ["retreat", format ["stands at %1, water %2 m deep: he goes dormant", _kp, _kp call KJB_fnDepth]] call KJB_fnLog;
                "dormant" call KJB_fnEnter
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
            _f = false;''')

f.rep('''    if (KJ_state == "idle") then {
        if ((count KJB_seq) == 0) then {
            _i = call KJB_fnPickDraw;''',
      '''    if ((KJ_state == "idle") and (!_ret)) then {
        if ((count KJB_seq) == 0) then {
            _i = call KJB_fnPickDraw;''')

f.rep('''    if (KJ_state == "wander") then {
        _i = call KJB_fnPickDraw;''',
      '''    if ((KJ_state == "wander") and (!_ret)) then {
        _i = call KJB_fnPickDraw;''')

f.rep('''    if (KJ_state == "drawn") then {
        KJB_lastDrawn = time;
        // a much stronger place takes him over; the place he walks to may move (a vehicle)''',
      '''    if ((KJ_state == "drawn") and (!_ret)) then {
        KJB_lastDrawn = time;
        // a much stronger place takes him over; the place he walks to may move (a vehicle)''')

# arriving (or the water stopping him) on the retreat
f.rep('''            ["drawn", format ["reached %1 (blocked by water %2)", KJ_brainDraw, KJB_blocked]] call KJB_fnLog;
            KJB_arrived = false;''',
      '''            ["drawn", format ["reached %1 (blocked by water %2)", KJ_brainDraw, KJB_blocked]] call KJB_fnLog;
            // [integration] the end of the sea retreat's walk
            if (KJB_retreatWalking) then { KJB_retreatWalking = false; KJB_retreatStood = true };
            KJB_arrived = false;''')
f.save()
