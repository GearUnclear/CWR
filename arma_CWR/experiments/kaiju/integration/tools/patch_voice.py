r"""Fifth round: each creature has its own voice (kjSnd_* entries of its class, kj\init.sqf
KJ_fnVoice), and a cry at a hard hit (the shared class KJ_Hurt, which only exists as a creature's
own take).   Run:  python tools\patch_voice.py"""
from patchlib import File

f = File(r"roar\snd.sqs")
f.rep('''_cls = _cls call KJ_fnVoice
''', '''_cls = _cls call KJ_fnVoice
; no such sound for this creature in the mounted sound set: nothing is played
? _cls == "" : exit
''')
f.save()

f = File(r"brain\init.sqf")
f.rep('''        _steps = [["phase", "stopping"], ["halt"], ["until", { time >= KJB_heldUntil }, 30], ["phase", "shudder"], ["move", KJB_mFlinch], ["wait", 3], ["phase", "staring"], ["wait", KJB_HIT_STARE]];''',
      '''        // [integration] with the shudder comes the creature's own cry at a hard hit (its kjSnd_Hurt), when its voice has one
        _steps = [["phase", "stopping"], ["halt"], ["until", { time >= KJB_heldUntil }, 30], ["phase", "shudder"], ["move", KJB_mFlinch], ["call", { if (KJB_hasRoar) then { [KJB_k, "KJ_Hurt", "KJ_HurtFar", 0, KJR_MOUTH_H] exec "kj\\roar\\snd.sqs" } }], ["wait", 3], ["phase", "staring"], ["wait", KJB_HIT_STARE]];''')
f.save()
