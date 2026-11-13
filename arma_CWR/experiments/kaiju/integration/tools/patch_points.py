r"""Eighth round: nothing in the set names a memory point of one model. The beam's measuring
particle is dropped on the creature's own furnace point (KJ_furnacePoint, from its class), not on
"zamerny".   Run:  python tools\patch_points.py"""
from patchlib import File

f = File(r"beam\fire.sqs")
f.rep('''drop ["\\kjbeam\\kjbeam_glow", "", "Billboard", 1, 0.05, "zamerny", [0, 0, 0], 0, 1.275, 1, 0, [0.01], [[0, 0, 0, 0]], [0], 1, 0, "", "kj\\beam\\probe.sqs", _k]''',
      '''drop ["\\kjbeam\\kjbeam_glow", "", "Billboard", 1, 0.05, KJ_furnacePoint, [0, 0, 0], 0, 1.275, 1, 0, [0.01], [[0, 0, 0, 0]], [0], 1, 0, "", "kj\\beam\\probe.sqs", _k]''')
f.rep('''; A particle dropped on the memory point "zamerny" (the aim point every Man model
; carries, in the chest) reports where it died: the only way a script can read a
; model point on the stock engine.''', '''; A particle dropped on the creature's furnace point (KJ_furnacePoint, from its
; class: "ded_furnace" on the final models, "zamerny" on the stand-in) reports
; where it died: the only way a script can read a model point on the stock engine.''')
f.save()

# the stock binary's last fallback for the furnace (when the particle reports nothing) is the
# creature's own metres, not the stand-in's
f = File(r"beam\fire.sqs")
f.rep('''? isNull KJ_unit : _k call KJ_fnSetup
? isNil "KJR_ready" : call compile preprocessFile''', '''? isNull KJ_unit : _k call KJ_fnSetup
KJ_beamT_chestFwd = KJ_furnaceOfs select 1
KJ_beamT_chestUp = KJ_furnaceOfs select 2
? isNil "KJR_ready" : call compile preprocessFile''')
f.save()
