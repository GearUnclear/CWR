r"""Finishing round (2026-11-12): the core and the furnace of the integrated set learn the engine's
version 2 traits (features\engine\API.md: glow, farGlow, plantBlend, pace). Exact replacements in
integration\kj; run once, kept as the record. Every native command stays inside a block the stock
binary never runs, and kjSet answers false (no error) for a trait an older kaiju binary does not
know, so the same set runs on the stock binary and on the kaiju binary before this round.

    python patch_finish_core.py
"""
from patchlib import File

# ------------------------------------------------------------------ kj\init.sqf
f = File("init.sqf")
f.rep('''KJ_traitLog = [];        // [[name, value found, value wanted, "config" or "kjSet"], ...]''',
      '''// ---- [finishing] the traits of engine version 2, held the same way but counted apart (the
// ---- creature addon's block may or may not carry them): the night floor of the far draw, so the
// ---- furnace shows from past the view distance by night, and the planted foot at the start of a walk.
if (isNil "KJ_traits2") then { KJ_traits2 = [["farGlow", 0.9], ["plantBlend", 1]] };
KJ_traits2Log = [];
KJ_traitLog = [];        // [[name, value found, value wanted, "config" or "kjSet"], ...]''')
f.rep('''KJ_unit = objNull;
if (isNil "KJ_heightM") then { KJ_heightM = 40 };''',
      '''// ---- [finishing] the furnace painted on his body (the engine's "glow" trait, version 2):
// ---- 0 dark, 1 the painted colour, to 2 white hot. On the stock binary, and on a kaiju binary
// ---- of version 1, nothing can change it and KJ_fnGlow only keeps the number.
KJ_glowNative = false;
KJ_glowNow = -1;
// [unit, 0..2] call KJ_fnGlow
KJ_fnGlow = {
    KJ_glowNow = _this select 1;
    if (KJ_glowNative) then { kjSet [_this select 0, "glow", _this select 1] }
};

KJ_unit = objNull;
if (isNil "KJ_heightM") then { KJ_heightM = 40 };''')
f.rep('''        } forEach KJ_traits;
        // the furnace point off the model''',
      '''        } forEach KJ_traits;
        // [finishing] engine version 2: its traits, and the glow of the furnace on the body
        KJ_glowNative = KJ_engineVersion >= 2;
        KJ_traits2Log = [];
        if (KJ_glowNative) then {
            {
                _have = kjGet [_k, _x select 0];
                if ((abs (_have - (_x select 1))) < 0.001) then {
                    KJ_traits2Log = KJ_traits2Log + [[_x select 0, _have, "config"]]
                } else {
                    kjSet [_k, _x select 0, _x select 1];
                    KJ_traits2Log = KJ_traits2Log + [[_x select 0, _have, "kjSet"]]
                }
            } forEach KJ_traits2
        };
        // the furnace point off the model''')
f.rep('''    logInfo format ["KJ setup: creature %1, roar class %2,''',
      '''    logInfo format ["KJ setup: engine version %1, glow on the body %2, version 2 traits %3", KJ_engineVersion, KJ_glowNative, KJ_traits2Log];
    logInfo format ["KJ setup: creature %1, roar class %2,''')
f.save()

# ------------------------------------------------------------- kj\fx\furnace.sqs
f = File(r"fx\furnace.sqs")
f.rep(''';  On the kaiju binary (KJ_native) the glow is a real point light bound to the''',
      ''';  [finishing] On a kaiju binary of version 2 the furnace painted on his body
;  follows too (the engine's "glow" trait through KJ_fnGlow): a low ember
;  dormant, breathing awake, full at the roar, white hot through the charge
;  and the burn, sunk to almost nothing spent, lower with every wound stage,
;  and guttering when the combat feature says so (KJ_gutterAt). Without it the
;  ribs are lit like the rest of his hide, as they always were.
;
;  On the kaiju binary (KJ_native) the glow is a real point light bound to the''')
f.rep('''; Embers a second while he is awake (none while dormant; the beam makes its own).
? isNil "KJF_EMBERS" : KJF_EMBERS = 1.5''',
      '''; Embers a second while he is awake (none while dormant; the beam makes its own).
? isNil "KJF_EMBERS" : KJF_EMBERS = 1.5
; [finishing] The furnace on the body, per word: 0 dark, 1 the painted colour, 2 white hot.
? isNil "KJF_GLOW_DORMANT" : KJF_GLOW_DORMANT = 0.5
? isNil "KJF_GLOW_AWAKE" : KJF_GLOW_AWAKE = 0.72
? isNil "KJF_GLOW_ROAR" : KJF_GLOW_ROAR = 1.05
? isNil "KJF_GLOW_TOP" : KJF_GLOW_TOP = 2
? isNil "KJF_GLOW_SPENT" : KJF_GLOW_SPENT = 0.1
; A gutter (the combat feature's KJ_gutterAt): a flare this long, then soot this long.
? isNil "KJF_GUT_FLARE" : KJF_GUT_FLARE = 0.2
? isNil "KJF_GUT_SOOT" : KJF_GUT_SOOT = 1.3''')
f.rep('''_tgt = KJF_DORMANT
_p = 0
_f = 0''',
      '''_tgt = KJF_DORMANT
_gl = KJF_GLOW_DORMANT
_gt = KJF_GLOW_DORMANT
_gp = 0
_gu = 0
_p = 0
_f = 0''')
f.rep('''#have
? _word != _was : KJ_furnaceLog''',
      '''#have
; [finishing] the same word on the body
_gt = KJF_GLOW_AWAKE
? _word == "dormant" : _gt = KJF_GLOW_DORMANT
? _word == "roar" : _gt = KJF_GLOW_ROAR
? _word == "spent" : _gt = KJF_GLOW_SPENT
? _word == "fire" : _gt = KJF_GLOW_TOP
? _word == "charge" : _gt = KJF_GLOW_AWAKE + (KJF_GLOW_TOP - KJF_GLOW_AWAKE) * _p * _p
? _word != _was : KJ_furnaceLog''')
f.rep('''_p = _p * _w
KJ_furnaceNow = [_p, [_cr, _cg, _cb], _word]''',
      '''_p = _p * _w
KJ_furnaceNow = [_p, [_cr, _cg, _cb], _word]
; [finishing] the furnace on the body: follows its word, breathes, burns, sinks, and gutters
_gl = _gl + (_gt - _gl) * _f
_gp = _gl
? _word == "awake" : _gp = _gl * (1 + KJF_BREATH * sin (360 * time / KJF_BREATH_SECS))
? _word == "dormant" : _gp = _gl * (1 + 0.5 * KJF_BREATH * sin (360 * time / (2 * KJF_BREATH_SECS)))
? _word == "fire" : _gp = _gl * (0.85 + random 0.15)
_gp = _gp * _w
? isNil "KJ_gutterAt" : goto "glowset"
_gu = time - KJ_gutterAt
? _gu < 0 : goto "glowset"
? _gu < KJF_GUT_FLARE : _gp = _gp * 1.6 + 0.3; goto "glowset"
? _gu < (KJF_GUT_FLARE + KJF_GUT_SOOT) : _gp = _gp * (0.15 + 0.85 * (_gu - KJF_GUT_FLARE) / KJF_GUT_SOOT)
#glowset
[_k, _gp] call KJ_fnGlow''')
f.save()

# -------------------------------------------------------------------- kj\start.sqs
f = File("start.sqs")
f.rep('''#dead
KJ_dead = true
KJ_beamAbort = true''',
      '''#dead
; [finishing] a body with no death sequence of its own (no combat feature in the set) goes dark here
? (not (isNull _k)) and (not KJ_hasCombat) : [_k, 0] call KJ_fnGlow
KJ_dead = true
KJ_beamAbort = true''')
f.save()
