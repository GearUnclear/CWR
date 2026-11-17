r"""Ninth round, from the first blows run (runs\blows-proxy-1112-004956): with the collision shape
lifted nothing on the ground shoves him (0 m with a man and a tank standing in his legs), so the
stand guard is only kept for a creature whose collision shape still reaches the ground, and it
never acts during a stomp or a swat (it shoved the tank the stomping foot was coming down on).
Run:  python tools\patch_guard.py"""
from patchlib import File

f = File(r"crush\loop.sqs")
f.rep('''; [integration] nothing may stand against his feet (it would shove him): every pass
[_k] call KJC_fnStandGuard
''', '''; [integration] nothing may stand against his feet (it would shove him): every pass. Only for a
; creature whose collision shape reaches the ground (kjGeoFloor of its class under 3 m: the
; integrated addon lifts it, and then nothing on the ground can touch it), and never while a
; stomp or a swat is under way (the foot coming down is not a standing foot).
? KJC_guard and (not KJ_stomping) and (not KJ_swatting) : [_k] call KJC_fnStandGuard
''')
f.rep('''? KJ_native : KJC_nativeSteps = true
''', '''? KJ_native : KJC_nativeSteps = true
KJC_guard = ([_k, "kjGeoFloor"] call KJ_fnCfgNum) < 3
''')
f.save()
