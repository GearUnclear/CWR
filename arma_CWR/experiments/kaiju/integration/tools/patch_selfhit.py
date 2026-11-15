r"""Eleventh round, from the blows rerun on A (runs\blows-a2-1112-021443): a helicopter he swatted
blew up against him and the engine's hit report named HIM as the attacker (the blast of a wreck is
credited to whoever killed it). The hit signal then asked for the bearing from him to himself:
"0 atan2 0" is a script error in this engine ("Zero divisor"), which ends a test run.
Now: a hit whose attacker is himself is no hit (he is healed, nothing is signalled), and the
bearing is only worked out when the shooter stands somewhere else.
Run:  python tools\patch_selfhit.py"""
from patchlib import File

f = File(r"crush\hurt.sqs")
f.rep('''; the heaviest hit of this read is the one he feels
_h = _hits select 0
{ if ((_x select 3) > (_h select 3)) then { _h = _x } } forEach _hits
KJ_hurtReport = _h
? (_h select 3) >= KJC_HURT_MIN : [_k, _h select 3, _h select 0] call KJC_fnHurt''',
      '''; the heaviest hit of this read is the one he feels. A hit he gave himself (the blast of a
; wreck he made is credited to him) is no hit.
_h = [objNull, "", [0, 0, 0], 0, 0, [0, 0, 0]]
{ if (((_x select 3) > (_h select 3)) and ((_x select 0) != _k)) then { _h = _x } } forEach _hits
? (_h select 3) <= 0 : goto "healed"
KJ_hurtReport = _h
? (_h select 3) >= KJC_HURT_MIN : [_k, _h select 3, _h select 0] call KJC_fnHurt''')
f.save()

f = File(r"crush\init.sqf")
f.rep('''    if (!(isNull _who)) then {
        _kp = getPos _k;
        _wp = getPos _who;
        _abs = ((_wp select 0) - (_kp select 0)) atan2 ((_wp select 1) - (_kp select 1));
        if (_abs < 0) then { _abs = _abs + 360 };
        _rel = _abs - (getDir _k);
        if (_rel < 0) then { _rel = _rel + 360 }
    };''', '''    // [integration] never himself, and never a shooter standing on his own spot ("0 atan2 0" is a script error)
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
    };''')
f.save()

f = File(r"brain\init.sqf")
f.rep('''KJB_fnBear = {
    private ["_b"];
    _b = (((_this select 1) select 0) - ((_this select 0) select 0)) atan2 (((_this select 1) select 1) - ((_this select 0) select 1));
    if (_b < 0) then { _b = _b + 360 };
    _b
};''', '''// [integration] 0 when the two are the same spot ("0 atan2 0" is a script error in this engine)
KJB_fnBear = {
    private ["_b", "_dx", "_dy"];
    _dx = ((_this select 1) select 0) - ((_this select 0) select 0);
    _dy = ((_this select 1) select 1) - ((_this select 0) select 1);
    _b = 0;
    if (((abs _dx) + (abs _dy)) > 0.001) then { _b = _dx atan2 _dy };
    if (_b < 0) then { _b = _b + 360 };
    _b
};''')
f.save()
