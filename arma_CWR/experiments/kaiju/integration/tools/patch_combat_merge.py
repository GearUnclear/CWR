r"""Run by merge-combat.ps1 right after the combat feature's payload is copied into the set. One fix
to the COPY (the feature's own file is not touched; the finding is reported to its owner in the
integration NOTES): the hit signal's bearing. The blast of a helicopter he has swatted is credited
to HIM by the engine, the signal then asks for the bearing from him to himself, and "0 atan2 0" is
a script error ("Zero divisor") that ends a test run (runs\blows-d-1112-025016). Same trap, same
cure, as in the crush feature's signal (patch_selfhit.py): himself is nobody, and the bearing is
only worked out for a shooter standing somewhere else.
If the feature has fixed it itself, the text below is no longer there and this does nothing."""
import sys

from patchlib import File

f = File(r"combat\init.sqf")
old = '''        if (!(isNull _who)) then {
            _kp = getPos _k;
            _wp = getPos _who;
            _abs = ((_wp select 0) - (_kp select 0)) atan2 ((_wp select 1) - (_kp select 1));
            if (_abs < 0) then { _abs = _abs + 360 };
            _rel = _abs - (getDir _k);
            if (_rel < 0) then { _rel = _rel + 360 }
        };'''
new = '''        // [integration] never himself, and never a shooter on his own spot ("0 atan2 0" is a script error)
        if (!(isNull _who)) then { if (_who == _k) then { _who = objNull } };
        if (!(isNull _who)) then {
            _kp = getPos _k;
            _wp = getPos _who;
            if (((abs ((_wp select 0) - (_kp select 0))) + (abs ((_wp select 1) - (_kp select 1)))) > 0.01) then {
                _abs = ((_wp select 0) - (_kp select 0)) atan2 ((_wp select 1) - (_kp select 1));
                if (_abs < 0) then { _abs = _abs + 360 };
                _rel = _abs - (getDir _k);
                if (_rel < 0) then { _rel = _rel + 360 }
            }
        };'''
if f.text.count(old) == 1:
    f.rep(old, new)
    f.save()
else:
    print("combat\\init.sqf: the unguarded bearing is not there (fixed in the feature, or changed): nothing patched")
    sys.exit(0)
