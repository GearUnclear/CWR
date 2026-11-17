r"""Integration changes to the beam and roar features' payloads (kj\beam, kj\roar), applied once to
the COPIES of features\beam\kj\beam and features\roar\kj\roar. Run:  python tools\patch_beam_roar.py
What changes, and why, is in kaiju-notes\integration\NOTES.md ("What changed in each feature")."""
from patchlib import File

# =========================================================================================== beam\fire.sqs
f = File(r"beam\fire.sqs")
f.rep('''; Needs the kjbeam addon (features\\beam\\addon\\build-kjbeam.ps1) named in the
; mission's addOns[]. Nothing here fires a weapon: every death is setDammage.''',
      '''; Needs the kjbeam addon (in tmp\\@kj_dedushka) named in the mission's addOns[].
; Nothing here fires a weapon: every death is scripted damage.
;
; INTEGRATED set (changes marked [integration]): the beam now has its sounds (the
; roar feature's KJ_BeamCharge, KJ_Beam, KJ_BeamLoop, KJ_Exhale through snd.sqs), its
; tremor (KJ_fnShake: the engine's camera shake on the kaiju binary), its animations
; (the brain plays DedBeamRise and DedBeamLower; this script does when the brain is not
; running), the furnace light of kj\\fx\\furnace.sqs on the kaiju binary (KJ_beamCharge
; drives it; the Fire-class lights below are the stock fallback), the chest point in
; his current pose (kjPoint) and kills credited to him (KJ_fnKill). And his feet stay
; planted while the beam cuts: the line swings from the chest, the body does not turn
; with it (KJ_beamT_bodyTurn).''')
f.rep('''; A caller may change any of the above for one run: KJ_beamOverride = { KJ_beamT_still = 1 }.''',
      '''; [integration] false: his feet stay where they are and the line swings from the chest across
; the arc (he must already face the middle of the cut; the brain turns him there with its turn
; clips). true: the body is turned with the line, as the beam feature did alone (the feet slide).
KJ_beamT_bodyTurn = false
; The line may leave his facing by at most this many degrees before the body is turned after
; all (the sky phase can swing anywhere).
KJ_beamT_offMax = 62
; Which end the cut starts from: 1 his left to his right, -1 the other way, 0 the end nearer his facing.
KJ_beamT_side = 0
; Seconds the charge sound lasts: it starts that long before the beam, so it ends on the crack.
KJ_beamT_chargeSnd = 4.2
; Seconds of KJ_Beam before the loop takes over, and the loop's length.
KJ_beamT_beamSnd = 4.4
KJ_beamT_loopSnd = 2.95
; The tremor: strength while it charges (start, end), at the ignition, and while it burns.
KJ_beamT_shake = [0.2, 0.8, 1.7, 0.65]
; A caller may change any of the above for one run: KJ_beamOverride = { KJ_beamT_still = 1 }.''')
f.rep('''? isNil "KJ_beamFnsLoaded" : call compile preprocessFile "kj\\beam\\fns.sqf"
''', '''? isNil "KJ_beamFnsLoaded" : call compile preprocessFile "kj\\beam\\fns.sqf"
; [integration] the shared core and the roar feature's sound and shake
? isNil "KJ_coreReady" : call compile preprocessFile "kj\\init.sqf"
? isNull KJ_unit : _k call KJ_fnSetup
? isNil "KJR_ready" : call compile preprocessFile "kj\\roar\\init.sqf"
? isNil "KJR_sized" : [_k] call KJR_fnSize
? isNil "KJ_brainOn" : KJ_brainOn = false
? isNil "KJ_furnaceOn" : KJ_furnaceOn = false
KJ_beamCharge = 0
KJ_beamSnd = []
_hgt = KJ_heightM
_useFire = not (KJ_native and KJ_furnaceOn)
_sndAt = 0
_loopAt = 0
_shakeAt = 0
_off = 0
''')
f.rep('''if (([getDir _k, _b0] call KJ_beam_fnTurn) < 0) then { _side = -1 }
''', '''if (([getDir _k, _b0] call KJ_beam_fnTurn) < 0) then { _side = -1 }
? KJ_beamT_side != 0 : _side = KJ_beamT_side
''')
f.rep('''; ---- 1. he stops. He turns to the cut, and then nothing moves ----
_k stop true
_last = time
_faced = -1
''', '''; ---- 1. he stops. He turns to the cut, and then nothing moves ----
_k stop true
_last = time
_faced = -1
; [integration] where he must face: the start of the cut when the body turns with the line;
; else wherever he faces now, as long as both ends of the cut are inside KJ_beamT_offMax of it
; (the brain has turned him to the target), else the middle of the cut
_face = _azS
? KJ_beamT_bodyTurn : goto "still"
_face = getDir _k
? ((abs ([_face, _azS] call KJ_beam_fnTurn)) > KJ_beamT_offMax) or ((abs ([_face, _azS + _sweep] call KJ_beam_fnTurn)) > KJ_beamT_offMax) : _face = _azS + _sweep / 2
''')
f.rep('''_turn = [getDir _k, _azS] call KJ_beam_fnTurn
_stepMax = KJ_beamT_turnRate * _dt
if (_turn > _stepMax) then { _turn = _stepMax }
if (_turn < (0 - _stepMax)) then { _turn = 0 - _stepMax }
? (abs _turn) > 0.001 : _k setDir ((getDir _k) + _turn)
? ((abs ([getDir _k, _azS] call KJ_beam_fnTurn)) < 1.5) and (_faced < 0) : _faced = time''',
      '''_turn = [getDir _k, _face] call KJ_beam_fnTurn
_stepMax = KJ_beamT_turnRate * _dt
if (_turn > _stepMax) then { _turn = _stepMax }
if (_turn < (0 - _stepMax)) then { _turn = 0 - _stepMax }
? (abs _turn) > 0.001 : _k setDir ((getDir _k) + _turn)
? ((abs ([getDir _k, _face] call KJ_beam_fnTurn)) < 1.5) and (_faced < 0) : _faced = time''')
f.rep('''{ _lights = _lights + [_x createVehicle [_kx, _ky, 0]] } forEach ["KJBeamGlowA", "KJBeamGlowB", "KJBeamGlowC", "KJBeamGlowD"]
KJ_beamLights = _lights''',
      '''; [integration] on the kaiju binary the furnace light is kj\\fx\\furnace.sqs (one smooth light on
; the furnace point); these four Fire-class lights are the stock binary's steps
? _useFire : { _lights = _lights + [_x createVehicle [_kx, _ky, 0]] } forEach ["KJBeamGlowA", "KJBeamGlowB", "KJBeamGlowC", "KJBeamGlowD"]
KJ_beamLights = _lights
? not KJ_brainOn : _k playMove "DedBeamRise"''')
f.rep('''_t0 = time
_tick = 0
_n = 0
#charge
~0.01
? (not (alive _k)) or KJ_beamAbort : goto "abort"
_p = (time - _t0) / KJ_beamT_charge
if (_p > 1) then { _p = 1 }
? time < _tick : goto "chargeNext"
_tick = time + KJ_beamT_fxTick
_n = _n + 1
''', '''_t0 = time
_tick = 0
_n = 0
_sndAt = _t0 + KJ_beamT_charge - KJ_beamT_chargeSnd
_shakeAt = _t0
#charge
~0.01
? (not (alive _k)) or KJ_beamAbort : goto "abort"
_p = (time - _t0) / KJ_beamT_charge
if (_p > 1) then { _p = 1 }
KJ_beamCharge = _p
; [integration] the whine of the charge, timed to end on the crack; the ground trembling harder as it builds
? (_sndAt >= 0) and (time >= _sndAt) : _sndAt = -1; KJ_beamSnd = KJ_beamSnd + [["KJ_BeamCharge", time]]; [_k, "KJ_BeamCharge", "", 4, 0.6] exec "kj\\roar\\snd.sqs"
? time >= _shakeAt : _shakeAt = time + 0.8; [getPos _k, (KJ_beamT_shake select 0) + ((KJ_beamT_shake select 1) - (KJ_beamT_shake select 0)) * _p, 1.2, 1.5 * _hgt, 12 * _hgt, 0] call KJ_fnShake
? time < _tick : goto "chargeNext"
_tick = time + KJ_beamT_fxTick
_n = _n + 1
''')
f.rep('''KJ_beamPhase = "fire"
KJ_beamShots = KJ_beamShots + 1
logInfo format ["KJBEAM fire at %1", time]
''', '''KJ_beamPhase = "fire"
KJ_beamShots = KJ_beamShots + 1
KJ_beamCharge = 1
logInfo format ["KJBEAM fire at %1", time]
; [integration] the crack and the beam's voice, then its loop for as long as it burns; one hard jolt
KJ_beamSnd = KJ_beamSnd + [["KJ_Beam", time]]
[_k, "KJ_Beam", "", 4, 0.6] exec "kj\\roar\\snd.sqs"
_loopAt = time + KJ_beamT_beamSnd
[getPos _k, KJ_beamT_shake select 2, 1.6, 2.5 * _hgt, 18 * _hgt, 0] call KJ_fnShake
_shakeAt = time + 1
''')
f.rep('''_az = _azS + _sweep * _e
_r = _rS + (_rE - _rS) * _e
_k setDir _az
_kp = getPosASL _k''', '''_az = _azS + _sweep * _e
_r = _rS + (_rE - _rS) * _e
; [integration] his feet stay planted: the line swings, the body does not (unless it must)
_off = [getDir _k, _az] call KJ_beam_fnTurn
? KJ_beamT_bodyTurn : _k setDir _az
? (not KJ_beamT_bodyTurn) and (_off > KJ_beamT_offMax) : _k setDir (_az - KJ_beamT_offMax)
? (not KJ_beamT_bodyTurn) and (_off < (0 - KJ_beamT_offMax)) : _k setDir (_az + KJ_beamT_offMax)
? time >= _loopAt : _loopAt = time + KJ_beamT_loopSnd; KJ_beamSnd = KJ_beamSnd + [["KJ_BeamLoop", time]]; [_k, "KJ_BeamLoop", "", 7, 0.6] exec "kj\\roar\\snd.sqs"
? time >= _shakeAt : _shakeAt = time + 0.9; [getPos _k, KJ_beamT_shake select 3, 1.1, 1.5 * _hgt, 14 * _hgt, 0] call KJ_fnShake
_kp = getPosASL _k''')
f.rep('''_el = _el + _turn
_k setDir _az
_c = _k call KJ_beam_fnChest''', '''_el = _el + _turn
_off = [getDir _k, _az] call KJ_beam_fnTurn
? KJ_beamT_bodyTurn : _k setDir _az
? (not KJ_beamT_bodyTurn) and (_off > KJ_beamT_offMax) : _k setDir (_az - KJ_beamT_offMax)
? (not KJ_beamT_bodyTurn) and (_off < (0 - KJ_beamT_offMax)) : _k setDir (_az + KJ_beamT_offMax)
? time >= _loopAt : _loopAt = time + KJ_beamT_loopSnd; KJ_beamSnd = KJ_beamSnd + [["KJ_BeamLoop", time]]; [_k, "KJ_BeamLoop", "", 7, 0.6] exec "kj\\roar\\snd.sqs"
? time >= _shakeAt : _shakeAt = time + 0.9; [getPos _k, KJ_beamT_shake select 3, 1.1, 1.5 * _hgt, 14 * _hgt, 0] call KJ_fnShake
_c = _k call KJ_beam_fnChest''')
f.rep('''_red setPosASL _c
_red inflame true
KJ_beamPhase = "spent"
KJ_beamSpentUntil = time + KJ_beamT_spent
KJ_beamActive = false
_k stop false''', '''_red setPosASL _c
? _useFire : _red inflame true
KJ_beamPhase = "spent"
KJ_beamSpentUntil = time + KJ_beamT_spent
KJ_beamActive = false
KJ_beamCharge = 0
; [integration] the spent breath; he lowers himself (the brain plays it when it runs)
KJ_beamSnd = KJ_beamSnd + [["KJ_Exhale", time]]
[_k, "KJ_Exhale", "", 8, 0.6] exec "kj\\roar\\snd.sqs"
? not KJ_brainOn : _k playMove "DedBeamLower"
_k stop false''')
f.rep('''KJ_beamPhase = "idle"
KJ_beamActive = false
? alive _k : _k stop false
exit''', '''KJ_beamPhase = "idle"
KJ_beamActive = false
KJ_beamCharge = 0
? (alive _k) and (not KJ_brainOn) : _k playMove "DedBeamLower"
? alive _k : _k stop false
exit''')
f.save()

# =========================================================================================== beam\fns.sqf
f = File(r"beam\fns.sqf")
f.rep('''KJ_beam_fnChest = {
    private ["_p", "_d"];
    _p = getPosASL _this;
    _d = getDir _this;
    [(_p select 0) + KJ_beamChestFwd * sin _d, (_p select 1) + KJ_beamChestFwd * cos _d, (_p select 2) + KJ_beamChestUp]
};''', '''// [integration] On the kaiju binary: the furnace point of his model in his CURRENT pose (he
// straightens for the beam, and the point goes with him).
KJ_beam_fnChest = {
    private ["_p", "_d", "_c"];
    _c = [];
    if (KJ_native) then { _c = kjPoint [_this, KJ_furnacePoint] };
    if ((count _c) != 3) then {
        _p = getPosASL _this;
        _d = getDir _this;
        _c = [(_p select 0) + KJ_beamChestFwd * sin _d, (_p select 1) + KJ_beamChestFwd * cos _d, (_p select 2) + KJ_beamChestUp]
    };
    _c
};''')
f.rep('''            if (alive _o) then { _o setDammage 1; KJ_beamKills = KJ_beamKills + 1 };''',
      '''            if (alive _o) then { [_o, KJ_beamOwner] call KJ_fnKill; KJ_beamKills = KJ_beamKills + 1 };''')
f.rep('''            if (alive _o) then {
                _o setDammage 1;
                KJ_beamWrecks = KJ_beamWrecks + 1;''', '''            if (alive _o) then {
                [_o, KJ_beamOwner] call KJ_fnKill;
                KJ_beamWrecks = KJ_beamWrecks + 1;''')
f.rep('''                if (_d2 < _r2) then {
                    _x setDammage 1;''', '''                if (_d2 < _r2) then {
                    [_x, KJ_beamOwner] call KJ_fnKill;''')
f.rep('''                if ((damage _o) < 1) then {
                    _o setDammage 1;
                    if ((damage _o) >= 1) then {''', '''                if ((damage _o) < 1) then {
                    [_o, KJ_beamOwner] call KJ_fnKill;
                    if ((damage _o) >= 1) then {''')
f.save()

# =========================================================================================== roar\roar.sqs
f = File(r"roar\roar.sqs")
f.rep('''_mine = units (group player)
_i = 0
''', '''_mine = units (group player)
_i = 0
; [integration] kaiju binary: the engine's roar tells every group led from inside the alert
; radius exactly where he is (kjRoar's reveal, at full accuracy, script-made groups included).
; Its fear is NOT used: it takes every group in its radius, townspeople too, and a civilian under
; the engine's flee walks (measured by this feature) where the script makes him run for a house.
; The sound stays with snd.sqs, which picks the near or the far class.
KJ_roarNative = [0, 0]
? KJ_native : KJ_roarNative = kjRoar [_k, "", KJR_ALERT_R * KJR_H, 0, 0]
? KJ_native : logInfo format ["KJROAR native reveal: %1 groups told where he is", KJ_roarNative select 0]
''')
f.save()

# =========================================================================================== roar\init.sqf
f = File(r"roar\init.sqf")
f.rep('''// ---- functions -----------------------------------------------------------------
''', '''// [integration] the shared core (which engine: shake.sqs uses the engine's camera shake when KJ_native)
if (isNil "KJ_coreReady") then { call compile preprocessFile "kj\\init.sqf" };

// ---- functions -----------------------------------------------------------------
''')
f.save()
