r"""Fourteenth round, from the fights (runs\fight2-1112-043409, runs\fight3-1112-044744): with a crewed
tank parked 12 m in front of his feet and tanks firing from far off, the brain chose the beam every
time it was free to choose (the beam was decided before the blow, and under fire its anger is always
earned), so the tank at his feet stood there unharmed through two beams. A tank under his foot now
comes before a target far off: the blow is decided first.
Run:  python tools\patch_blowfirst.py"""
from patchlib import File

f = File(r"brain\init.sqf")
blow = '''    // a blow, when something is in reach and he is standing or between strides
    if ((KJ_state in ["idle", "wander", "drawn"]) and ((time - KJB_lastBlow) >= KJB_BLOW_GAP)) then {
        {
            if ((KJ_state in ["idle", "wander", "drawn"]) and (alive _x) and ((count (crew _x)) > 0)) then {
                if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < 70) then { KJB_blowTgt = _x; "stomp" call KJB_fnEnter }
            }
        } forEach (nearestObjects [_kp, ["Tank"], KJB_STOMP_R * KJB_H]);
        {
            if ((KJ_state in ["idle", "wander", "drawn"]) and (alive _x) and (((getPos _x) select 2) > (0.25 * KJB_H))) then { KJB_blowTgt = _x; "swat" call KJB_fnEnter }
        } forEach (nearestObjects [[_kp select 0, _kp select 1, 0.8 * KJB_H], ["Air"], KJB_SWAT_R * KJB_H])
    };

'''
f.rep(blow, "")
f.before('''    // the beam, from any waking state ([integration] not while he retreats to the sea)
''', blow.replace("    // a blow, when something is in reach and he is standing or between strides",
                  "    // a blow, when something is in reach and he is standing or between strides\n    // ([integration] decided before the beam: a tank under his foot comes before a target far off)"))
f.save()
