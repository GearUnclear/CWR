r"""Sixth round: wounded, he walks slower. The combat feature gives KJ_woundPace (1..0.45); a script
cannot set the rate of a played clip, but the addon has the walk twice: DedStride (full rate) and
DedWalk (its slowest rate, 0.6). Under KJ_woundPace 0.8 a leg is walked in DedWalk.
Run:  python tools\patch_pace.py"""
from patchlib import File

f = File(r"brain\init.sqf")
f.after('''KJB_dormantSecs = 16;
KJB_deadSince = -1;
''', '''KJB_walkNow = "";               // [integration] the walk move of the leg he is on (the stride, or the slow walk when wounded)
KJB_paceNow = 1;
''')
f.rep('''    if (KJB_START_FIX) then { call KJB_fnStartFixBegin };
    KJB_k playMove KJB_mWalk
};''', '''    // [integration] wounded (the combat feature's KJ_woundPace under 0.8), this leg is walked in the
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
};''')
f.rep('''        _add = KJB_walkSpeed * KJB_blendSecs * 0.5 * _t * _t - KJB_fixDone''',
      '''        _add = KJB_walkSpeed * KJB_paceNow * KJB_blendSecs * 0.5 * _t * _t - KJB_fixDone''')
f.rep('''                KJB_cyc = KJB_cyc + 1;
                _k playMove KJB_mWalk''', '''                KJB_cyc = KJB_cyc + 1;
                _k playMove KJB_walkNow''')
f.rep('''        if (KJB_lastStride and (_mv != KJB_mWalk)) then { call KJB_fnEndWalk };''',
      '''        if (KJB_lastStride and (_mv != KJB_walkNow)) then { call KJB_fnEndWalk };''')
f.rep('''        if ((KJB_mv == "walk") and (!KJB_lastStride) and (_mv != KJB_mWalk) and (_now > (KJB_walkT + 2))) then {''',
      '''        if ((KJB_mv == "walk") and (!KJB_lastStride) and (_mv != KJB_walkNow) and (_now > (KJB_walkT + 2))) then {''')
f.rep('''        if ((KJB_mv == "walk") and (_now > (KJB_walkT + 3 * KJB_cyc * KJB_strideSecs + 10))) then {''',
      '''        if ((KJB_mv == "walk") and (_now > (KJB_walkT + 3 * KJB_cyc * KJB_strideSecs / KJB_paceNow + 10))) then {''')
f.save()
