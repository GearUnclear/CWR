r"""Finishing round (2026-11-12): two changes to the brain of the integrated set (integration\kj\brain).

1. THE LAST STAND IN THE SURF. A death while wading deep left nothing to see (the body under 17 m of
   water). The retreat to the sea no longer ends dormant in deep water: it ends where the water is
   KJB_BAY_WADE of his height deep (about his shins), or the moment he is dying on his feet (wound
   stage 4), wherever he is on the way. There he turns on what is killing him, roars, and is himself
   again (he flinches, turns on what hits him, uses the beam): at bay, with the furnace toward the
   guns. He falls toward them, so the body comes down across the shallows or the beach with the
   ribcage out of the water. At bay he wades no deeper than that for any reason.
2. A blow at a crewed tank in his reach is also decided while he stares or turns after a hit (state
   flinch, phases staring and turning), not only in idle, wander and drawn. On the Hound a tank
   stood at its feet for three minutes in which it was never in one of those three states, and was
   then trodden on by a footfall instead of stomped (run fight-d1).

    python patch_finish_brain.py
"""
from patchlib import File

f = File(r"brain\init.sqf")

# ---- tuning
f.rep('''if (isNil "KJB_START_FIX") then''',
      '''if (isNil "KJB_BAY_WADE") then { KJB_BAY_WADE = 0.12 };      // [finishing] the retreat to the sea ends where the water is this deep, in heights (his shins): there he turns at bay, so a body is never lost under deep water
if (isNil "KJB_START_FIX") then''')

# ---- he wades no deeper than the surf while he retreats or stands at bay
f.rep('''    if ((!_ok) and KJB_wade) then {
        KJ_brainDepth = call KJB_fnDepth;
        _ok = KJ_brainDepth < (KJB_WADE_MAX * KJB_H)
    };
    _ok''',
      '''    if ((!_ok) and KJB_wade) then {
        KJ_brainDepth = call KJB_fnDepth;
        _ok = KJ_brainDepth < (KJB_WADE_MAX * KJB_H);
        // [finishing] on the retreat and at bay: no deeper than the surf
        if (KJB_retreatWalking or KJB_bay) then { _ok = KJ_brainDepth < (KJB_BAY_WADE * KJB_H) }
    };
    _ok''')

# ---- the retreat is over once he stands at bay
f.rep('''KJB_retreatGoal = [];
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
};''',
      '''KJB_retreatGoal = [];
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
};''')
f.rep('''        _kp = getPos KJB_k;
        _dx = (KJ_woundRetreatPos select 0) - (_kp select 0);''',
      '''        _kp = getPos KJB_k;
        KJB_retreatFrom = [_kp select 0, _kp select 1, 0];
        _dx = (KJ_woundRetreatPos select 0) - (_kp select 0);''')

# ---- the blow: also while he stares or turns after a hit
f.rep('''    if ((KJ_state in ["idle", "wander", "drawn"]) and ((time - KJB_lastBlow) >= KJB_BLOW_GAP)) then {
        {
            if ((KJ_state in ["idle", "wander", "drawn"]) and (alive _x) and ((count (crew _x)) > 0)) then {''',
      '''    // ([finishing] and also while he stares or turns after a hit: under steady fire he is seldom in
    // any other state, and a tank that stands at his feet through all of it must not be left alone)
    _free = (KJ_state in ["idle", "wander", "drawn"]) or ((KJ_state == "flinch") and (KJ_brainPhase in ["staring", "turning"]));
    if (_free and ((time - KJB_lastBlow) >= KJB_BLOW_GAP)) then {
        {
            if (((KJ_state in ["idle", "wander", "drawn"]) or ((KJ_state == "flinch") and (KJ_brainPhase in ["staring", "turning"]))) and (alive _x) and ((count (crew _x)) > 0)) then {''')
f.rep('''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret"];
    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;''',
      '''    private ["_i", "_e", "_p", "_d", "_kp", "_cur", "_t", "_f", "_r", "_ret", "_free"];
    _kp = getPos KJB_k;
    _ret = call KJB_fnRetreating;''')

# ---- the end of the retreat: at bay, not dormant
f.rep('''        if ((KJ_state in ["idle", "wander"]) and ((count KJB_seq) == 0)) then {
            if (KJB_retreatStood) then {
                // he is there, or as deep as he can wade: he goes still (and mends, if they let him)
                ["retreat", format ["stands at %1, water %2 m deep: he goes dormant", _kp, _kp call KJB_fnDepth]] call KJB_fnLog;
                "dormant" call KJB_fnEnter
            } else {''',
      '''        // [finishing] dying on his feet he goes no further: he stops where he is and turns at bay
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
            } else {''')
f.rep('''    if (_s == "stomp") then {
        // [integration] he turns so that''',
      '''    if (_s == "bay") then {
        // [finishing] the last stand: he turns on what is killing him and roars, then he is himself
        _steps = [["phase", "stopping"], ["halt"]];
        if ((count KJB_bayFace) > 0) then { _steps = _steps + [["phase", "turning"], ["face", KJB_bayFace], ["wait", 1.5]] };
        _steps = _steps + [["phase", "roaring"], ["call", { call KJB_fnDoRoar }], ["wait", KJB_ROAR_SECS]];
        [_steps, "idle"] call KJB_fnSeq
    };
    if (_s == "stomp") then {
        // [integration] he turns so that''')
f.save()
