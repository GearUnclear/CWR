r"""Second round of integration changes (after patch_brain.py, patch_crush.py, patch_beam_roar.py):
small follow-ups, and what makes a creature a data entry (its own timing table and voice).
Run:  python tools\patch_misc.py"""
from patchlib import File

# ---- crush: the stock "Hit" event must not raise a second signal beside the engine's hit report
f = File(r"crush\init.sqf")
f.rep('''KJC_ehHit = { KJC_hitEvent = time; [_this select 0, _this select 2, _this select 1] call KJC_fnHurt };''',
      '''KJC_ehHit = { KJC_hitEvent = time; if (!KJ_native) then { [_this select 0, _this select 2, _this select 1] call KJC_fnHurt } };''')
f.save()

f = File(r"crush\loop.sqs")
f.rep('''? KJC_nativeSteps : goto "sleep"
? _acc < _half : goto "sleep"''', '''? KJC_nativeSteps and (_acc > _half) : _acc = _half
? KJC_nativeSteps : goto "sleep"
? _acc < _half : goto "sleep"''')
f.save()

# ---- roar: every sound goes through the creature's own voice (kj\init.sqf, KJ_fnVoice)
f = File(r"roar\snd.sqs")
f.rep('''_cls = _near
? (_far != "") and (_d > (KJR_FAR_SWITCH * KJR_H)) : _cls = _far
''', '''_cls = _near
? (_far != "") and (_d > (KJR_FAR_SWITCH * KJR_H)) : _cls = _far
; [integration] this creature's own take of the sound, when its voice has one
_cls = _cls call KJ_fnVoice
''')
f.save()

f = File(r"roar\presence.sqs")
f.rep('''_e say "KJ_Furnace"
''', '''_e say ("KJ_Furnace" call KJ_fnVoice)
''')
f.save()

# ---- brain: the timing table is the creature's own (loaded by the core)
f = File(r"brain\start.sqs")
f.rep('''? KJ_brainHas select 3 : call compile preprocessFile "kj\\anim\\ded_anim_timing.sqf"
''', '''; [integration] the timing table of THIS creature's clips was loaded by the core (KJ_fnSetup)
? (KJ_brainHas select 3) and (isNil "KJ_animLoaded") : call compile preprocessFile "kj\\anim\\ded_anim_timing.sqf"
''')
f.save()
