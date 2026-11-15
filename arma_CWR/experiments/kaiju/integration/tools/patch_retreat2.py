r"""Fifteenth round, from runs\fight3-1112-044744: he reached the sea on his retreat, was left alone,
mended, and when the retreat was raised again while he stood 20 m deep in the bay the goal was worked
out from where he stood: "past the shore" then lay inland. A retreat that begins in the water (deeper
than a tenth of his height) ends where he stands: he goes dormant there.
Run:  python tools\patch_retreat2.py"""
from patchlib import File

f = File(r"brain\init.sqf")
f.rep('''    if (_ret) then {
        _p = call KJB_fnRetreatGoal;
''', '''    if (_ret) then {
        _p = call KJB_fnRetreatGoal;
        // already in the sea when it begins: he is where he wants to be
        if ((!KJB_retreatWalking) and (!KJB_retreatStood)) then { if ((call KJB_fnDepth) > (0.1 * KJB_H)) then { KJB_retreatStood = true } };
''')
f.save()
