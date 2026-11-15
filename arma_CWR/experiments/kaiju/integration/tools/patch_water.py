r"""Tenth round, from the first full demo run (runs\demo-proxy-1112-005808): a footfall in the sea
threw up a ring of brown dust. In water the ring is spray: pale, and thrown higher.
Run:  python tools\patch_water.py"""
from patchlib import File

f = File(r"roar\init.sqf")
f.rep('''// [[x, y], radius, strength] call KJR_fnDustRing : the ring a foot throws out.''',
      '''// [[x, y], radius, strength] call KJR_fnDustRing : the ring a foot throws out.
// [integration] an optional fourth element true makes it spray (a footfall in the sea).''')
f.rep('''    private ["_c", "_r", "_s", "_n", "_i", "_a", "_v", "_g"];
    _c = _this select 0;
    _r = _this select 1;
    _s = _this select 2;
    _n = KJR_DUST_N;''', '''    private ["_c", "_r", "_s", "_n", "_i", "_a", "_v", "_g", "_wet", "_gg", "_gb", "_up"];
    _c = _this select 0;
    _r = _this select 1;
    _s = _this select 2;
    _wet = false;
    if ((count _this) > 3) then { _wet = _this select 3 };
    _gg = 0.93;
    _gb = 0.8;
    _up = 0.6;
    if (_wet) then { _gg = 1.02; _gb = 1.06; _up = 3.5 };
    _n = KJR_DUST_N;''')
f.rep('''        _g = 0.5 + random 0.1;
        drop ["cl_basic", "", "Billboard", 1, 6 + random 3, [(_c select 0) + _r * sin _a, (_c select 1) + _r * cos _a, 0.8], [_v * sin _a, _v * cos _a, 0.6 + random 0.8], 0, 1.3, 1, 0.25, [3 * _s, 8 * _s, 14 * _s], [[_g, _g * 0.93, _g * 0.8, 0], [_g, _g * 0.93, _g * 0.8, 0.8], [_g, _g * 0.93, _g * 0.8, 0.45], [_g, _g * 0.93, _g * 0.8, 0]], [0], 0.6, 0.25, "", "", ""];''',
      '''        _g = 0.5 + random 0.1;
        if (_wet) then { _g = 0.8 + random 0.1 };
        drop ["cl_basic", "", "Billboard", 1, 6 + random 3, [(_c select 0) + _r * sin _a, (_c select 1) + _r * cos _a, 0.8], [_v * sin _a, _v * cos _a, _up + random 0.8], 0, 1.3, 1, 0.25, [3 * _s, 8 * _s, 14 * _s], [[_g, _g * _gg, _g * _gb, 0], [_g, _g * _gg, _g * _gb, 0.8], [_g, _g * _gg, _g * _gb, 0.45], [_g, _g * _gg, _g * _gb, 0]], [0], 0.6, 0.25, "", "", ""];''')
f.rep('''    while { _i < 8 } do {
        _a = random 360;''', '''    while { (_i < 8) and (!_wet) } do {
        _a = random 360;''')
f.save()

f = File(r"roar\step.sqs")
f.rep('''? _d < (KJR_DUST_VIEW * KJR_H) : _puffs = [_foot, KJR_FOOT_R * KJR_H, _scale] call KJR_fnDustRing''',
      '''; [integration] in the sea the ring is spray, not dust
? _d < (KJR_DUST_VIEW * KJR_H) : _puffs = [_foot, KJR_FOOT_R * KJR_H, _scale, not ([_foot select 0, _foot select 1] call KJR_fnDry)] call KJR_fnDustRing''')
f.save()
