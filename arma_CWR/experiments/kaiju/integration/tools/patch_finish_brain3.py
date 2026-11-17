r"""Finishing round (2026-11-12), third brain patch, after the first fight on the Hound with the
finished models (run gate-fight-d-r1 of 08:53): the tank sent to its feet stood 14 m off, to one
side, while it stood spent, and was trodden on when the retreat walk began. No stomp in the fight.

1. A crewed tank in his reach is stomped wherever round his feet it stands (the arc was 70 degrees,
   then 120 for the runs gate-fight-d-r1 to r3 of 09:15 to 09:50, of which the third had no stomp): the stomp turns him onto it anyway (KJB_fnStompBear).
2. Before the retreat walk begins he looks at his feet, as he does before setting off after a hit.

    python patch_finish_brain3.py
"""
from patchlib import File

f = File(r"brain\init.sqf")
f.rep('''if (isNil "KJB_BAY_WADE") then''',
      '''if (isNil "KJB_BLOW_ARC") then { KJB_BLOW_ARC = 181 };      // [finishing] a crewed tank in his reach is stomped when it stands within this many degrees of dead ahead: all round (the stomp turns him onto it; it was 70, and a tank beside or behind his feet was trodden on instead)
if (isNil "KJB_BAY_WADE") then''')
f.rep('''                if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < 70) then { KJB_blowTgt = _x; "stomp" call KJB_fnEnter }''',
      '''                if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < KJB_BLOW_ARC) then { KJB_blowTgt = _x; "stomp" call KJB_fnEnter }''')
f.rep('''            if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < 70) then { _t = _x }''',
      '''            if ((abs ([[_kp, getPos _x] call KJB_fnBear, getDir KJB_k] call KJB_fnAng)) < KJB_BLOW_ARC) then { _t = _x }''')
f.rep('''            } else {
                "drawn" call KJB_fnEnter;
                KJB_drawI = -1;
                KJ_brainDraw = [_p select 0, _p select 1];
                KJ_brainDraw call KJB_fnGo;
                KJB_lastDrawn = time;
                KJB_retreatWalking = true
            }
        };''',
      '''            } else {
                // [finishing] before the retreat walk begins: a crewed tank at his feet is stomped, not walked over
                _e = objNull;
                if ((time - KJB_lastBlow) >= KJB_BLOW_GAP) then { _e = call KJB_fnTankAtFeet };
                if (!(isNull _e)) then {
                    KJB_blowTgt = _e;
                    "stomp" call KJB_fnEnter
                } else {
                    "drawn" call KJB_fnEnter;
                    KJB_drawI = -1;
                    KJ_brainDraw = [_p select 0, _p select 1];
                    KJ_brainDraw call KJB_fnGo;
                    KJB_lastDrawn = time;
                    KJB_retreatWalking = true
                }
            }
        };''')
f.save()
