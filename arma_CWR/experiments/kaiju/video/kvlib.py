"""Shared pieces of the Dedushka video shoot tests (experiment, not a regression lane).

Each test boots video/DedushkaShoot.Eden (a copy of the integration's demo mission) with the one
mount @kj_dedushka on the kaiju binary, in a portrait window (1080 x 1920), --nosound. The creature
is started by the mission's own init.sqs through kj\\start.sqs; nothing of the script set is edited.

Stepped capture (the chupacabra recipe): the world is frozen between frames (setAccTime 0) and each
frame advances it by exactly one step inside one statement:
    call { triWaitFrames W; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
The render-only pump lets more than 0.3 s of wall time pass, AppIdle caps its step at 0.3 s, so the
one tick simulates 0.3 x 1/9 = 1/30 s. While the world is frozen any number of cameras can each take
the same instant (triSetView + triScreenshot per camera). Every frame logs its sim time and the
creature's state (KVF and KVG lines); the game's own log has every sound the script set sent (KJROAR
snd lines, with sim time and place), which is what the edit lays its sounds on.

Frame files: <seq>_<slot>-<cam>.png in the run's output folder.
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))

TOML = """# {title} (experiment, kept out of tests/integration so a sweep never runs it).
# Needs the mount tmp/@kj_dedushka. Written by gen_shoot.py (edit that, not this file).
# Run it through video/run-shoot.ps1 -Name {name}. One game, hidden desktop, --nosound.
tags = ["kaiju", "experiment", "video"]
binary = "PoseidonGame"
mission = "experiments/kaiju/video/DedushkaShoot.Eden"
timeout = {timeout}
assert_timeout = 900
extra_args = ["--nosound", "--no-strict", "--width", "{w}", "--height", "{h}", "--mods-dir", "..", "--mod", "@kj_dedushka"]
"""


def step_stmt(wait=60):
    return 'call { triWaitFrames %d; setAccTime (1/9); triSimFrames 1; setAccTime 0 }' % wait


class T:
    def __init__(self):
        self.lines = []
        self.frames = 0
        self.ncam = 0

    def a(self, s):
        assert '\n' not in s
        self.lines.append(s)

    def text(self):
        return '\n'.join(self.lines) + '\n'

    def shot(self, label):
        self.a('triScreenshot "%s"' % label)
        self.frames += 1


def header(t, title, weapons=False):
    t.a('// ' + '=' * 76)
    t.a('//  %s' % title)
    t.a('//  Written by gen_shoot.py (edit that, not this file). --nosound.%s' % ('  WEAPONS ARE FIRED.' if weapons else ''))
    t.a('// ' + '=' * 76)
    t.a('triSimUntil { alive player }')
    t.a('triCaptureAtPresent true')
    t.a('player allowDammage false')
    t.a('triSimUntil { !(isNil "KJ_ready") }')
    t.a('triSimUntil { KJ_ready }')
    t.a('if (true) then { KJT_on = true; [kaiju] exec "kj\\brain\\watch.sqs"; call compile preprocessFile "kjd_cams.sqf"; KJD_probe = "HeliHEmpty" createVehicle [5100, 4100, 0] }')
    t.a('kjCamObj = "camera" camCreate [5000, 3980, 3]')
    t.a('kjCamObj cameraEffect ["internal", "back"]')
    t.a('showCinemaBorder false')
    t.a('enableRadio false')
    # values the per-frame log reads, in case a feature has not set them yet
    t.a('if (isNil "KJ_beamPhase") then { KJ_beamPhase = "" }')
    t.a('if (isNil "KJ_deathPhase") then { KJ_deathPhase = "" }')
    t.a('if (isNil "KJD_fightShots") then { KJD_fightShots = 0 }')
    t.a('if (isNil "KJ_woundHealth") then { KJ_woundHealth = -1 }')
    t.a('if (isNil "KJ_woundStage") then { KJ_woundStage = 0 }')
    t.a('if (isNil "KJ_roarCount") then { KJ_roarCount = 0 }')
    t.a('if (isNil "KJ_brainBay") then { KJ_brainBay = false }')
    t.a('if (isNil "KJ_dead") then { KJ_dead = false }')
    # ground (or sea) height above sea level under a point
    t.a('kvGnd = { KJD_probe setPos [_this select 0, _this select 1, 0]; (getPosASL KJD_probe) select 2 }')
    # points are [x, height above sea level, y] (the order triSetView takes)
    t.a('kvMix = { private ["_a", "_b", "_k"]; _a = _this select 0; _b = _this select 1; _k = _this select 2; [(_a select 0) + _k * ((_b select 0) - (_a select 0)), (_a select 1) + _k * ((_b select 1) - (_a select 1)), (_a select 2) + _k * ((_b select 2) - (_a select 2))] }')
    t.a('kvLook = { private ["_e", "_a"]; _e = _this select 0; _a = _this select 1; [_e select 0, _e select 1, _e select 2, (_a select 0) - (_e select 0), (_a select 1) - (_e select 1), (_a select 2) - (_e select 2)] }')
    # [east offset, north offset, height above his feet] call kvK : a point on him
    t.a('kvK = { private ["_p"]; _p = getPosASL kaiju; [(_p select 0) + (_this select 0), (_p select 2) + (_this select 2), (_p select 1) + (_this select 1)] }')
    # the furnace point of the model in the pose he is in
    t.a('kvF = { private ["_f", "_r"]; _r = [0, 0, 27] call kvK; if (KJ_native) then { _f = kjPoint [kaiju, KJ_furnacePoint]; if ((count _f) == 3) then { _r = [_f select 0, _f select 2, _f select 1] } }; _r }')
    # [x, y, height above the ground or the sea there] call kvP : a fixed point
    t.a('kvP = { [_this select 0, ([_this select 0, _this select 1] call kvGnd) + (_this select 2), _this select 1] }')
    # [degrees clockwise from his front, metres from him, eye height above the ground or sea there] call kvRel
    t.a('kvRel = { private ["_p", "_b", "_x", "_y"]; _p = getPosASL kaiju; _b = (getDir kaiju) + (_this select 0); _x = (_p select 0) + (_this select 1) * (sin _b); _y = (_p select 1) + (_this select 1) * (cos _b); [_x, ([_x, _y] call kvGnd) + (_this select 2), _y] }')
    # [vehicle, metres behind it as seen from him, eye height, metres to its right] call kvBehind
    t.a('kvBehind = { private ["_v", "_p", "_k", "_dx", "_dy", "_d", "_x", "_y"]; _v = _this select 0; _p = getPosASL _v; _k = getPosASL kaiju; _dx = (_p select 0) - (_k select 0); _dy = (_p select 1) - (_k select 1); _d = sqrt (_dx * _dx + _dy * _dy); if (_d < 1) then { _d = 1 }; _x = (_p select 0) + (_this select 1) * _dx / _d + (_this select 3) * _dy / _d; _y = (_p select 1) + (_this select 1) * _dy / _d - (_this select 3) * _dx / _d; [_x, ([_x, _y] call kvGnd) + (_this select 2), _y] }')
    # [least metres from him, most metres from him, vehicles] call kvPick : the nearest living one in that ring (objNull: none)
    t.a('kvPick = { private ["_best", "_bd", "_d"]; _best = objNull; _bd = 1000000; { if (alive _x) then { _d = _x distance kaiju; if ((_d > (_this select 0)) && (_d < (_this select 1)) && (_d < _bd)) then { _bd = _d; _best = _x } } } forEach (_this select 2); _best }')
    # [point, metres of height the portrait frame should hold at his distance] call kvFovFor : the lens for that
    t.a('kvFovFor = { private ["_e", "_k", "_d", "_f"]; _e = _this select 0; _k = getPosASL kaiju; _d = sqrt (((_e select 0) - (_k select 0)) ^ 2 + ((_e select 2) - (_k select 1)) ^ 2); _f = (_this select 1) / (2 * 1.778 * _d); if (_f < 0.02) then { _f = 0.02 }; if (_f > 0.6) then { _f = 0.6 }; _f }')
    t.a('kvEar = "HeliHEmpty" createVehicle [5000, 3984, 0]')
    t.a('KJ_listener = kvEar')
    t.a('kvLens = { kjCamObj camSetTarget (getPos kaiju); kjCamObj camSetFov _this; kjCamObj camCommit 0 }')
    t.a('logInfo format ["KV start: creature %1, native %2, hour %3, start %4, params %5", typeOf kaiju, KJ_native, daytime, getPos kaiju, KJD_params]')
    t.a('0.5 call kvLens')
    t.a('triSetView ([[4985, 3984, 1.7] call kvP, [0, 0, 22] call kvK] call kvLook)')
    t.a('triSimFrames 8')
    t.shot('000_warmup_discard')


def footer(t):
    t.a('setAccTime 1')
    t.a('logInfo format ["KV end: states %1", KJT_states]')
    t.a('logInfo format ["KV end: moves %1", KJT_moves]')
    t.a('logInfo format ["KV end: jumps (time, metres, seconds, from, to, men, vehicles) %1, top speed %2", KJT_jumps, KJT_maxSpeed]')
    t.a('logInfo "KV END"')
    # the engine's light and emitter must be gone before the game exits (as the integration's tests do)
    t.a('if (true) then { KJT_on = false; KJ_brainOn = false; KJ_furnaceOn = false; KJ_presenceOn = false; KJ_crushOn = false; KJ_combatOn = false; KJD_firing = false }')
    t.a('triSimFrames 30')
    t.a('if (KJ_native) then { kjClear kaiju }')
    t.a('triSimFrames 5')
    t.a('triEndTest')


class Cam:
    """One camera. eye: (x, y, height above the ground or sea there). aim: an SQF expression that
    gives the target point [x, asl, y] this frame, for example '[0, 0, 22] call kvK' (a point on
    him), 'call kvF' (the furnace), '[5057, 3925, 14] call kvP' (a fixed point). alpha: how fast the
    aim follows (1 = locked, 0.08 = a heavy tripod head). fov: horizontal half width (tan); the
    portrait frame's vertical half is 1.778 times that."""

    def __init__(self, name, eye, aim, fov, alpha=1.0, eye_expr=None, aim0=None):
        self.name = name
        self.eye = eye
        self.aim = aim
        self.fov = fov          # a number, or an SQF expression evaluated when the take starts
        self.alpha = alpha
        self.eye_expr = eye_expr
        self.aim0 = aim0        # where the aim starts (default: on the target)


def still(t, label, cam, settle=6):
    t.a('%s call kvLens' % cam.fov)
    eye = cam.eye_expr or ('[%g, %g, %g] call kvP' % cam.eye)
    t.a('triSetView ([%s, %s] call kvLook)' % (eye, cam.aim))
    t.a('triSimFrames %d' % settle)
    t.shot(label)


def stepped(t, slot, n, cams, wait=45, pre=None, every=1, extra_log=None):
    """n stepped frames of 1/30 s; every camera takes each instant. All cameras of one take share
    one lens (the lens is set before the world is frozen). pre: one SQF statement run in the same
    statement that freezes the world (an order given a tick before the first frame). The first
    camera is where the script set's listener is put (birds rise near it, sounds are timed to it)."""
    fovs = set(str(c.fov) for c in cams)
    ids = []
    for c in cams:
        t.ncam += 1
        i = t.ncam
        ids.append(i)
        eye = c.eye_expr or ('[%g, %g, %g] call kvP' % c.eye)
        t.a('kvE%d = %s' % (i, eye))
        t.a('kvA%d = %s' % (i, c.aim0 or c.aim))
        fov = c.fov % i if isinstance(c.fov, str) and '%d' in c.fov else c.fov
        t.a('kvFv%d = %s' % (i, fov))
        t.a('logInfo format ["KVC %s-%s eye %%1 aim %%2 fov %%3 alpha %g", kvE%d, kvA%d, kvFv%d]' % (slot, c.name, c.alpha, i, i, i))
    t.a('kvFv%d call kvLens' % ids[0])
    t.a('triSimFrames 2')
    t.a('kvEar setPos [kvE%d select 0, kvE%d select 2, 0]' % (ids[0], ids[0]))
    t.a('call { %s; setAccTime 0 }' % (pre or 'kvNop = 0'))
    for k in range(n):
        t.a(step_stmt(wait))
        t.a('logInfo format ["KVF %s %d %%1 %%2 %%3 %%4 %%5 %%6 %%7", time, getPosASL kaiju, getDir kaiju, KJ_state, KJ_brainPhase, KJ_footfall select 0, getMove kaiju]' % (slot, k))
        t.a('logInfo format ["KVG %s %d %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8", KJ_beamPhase, KJ_deathPhase, KJD_fightShots, KJ_woundHealth, KJ_woundStage, KJ_roarCount, if (KJ_native) then { kjShakeGain } else { 0 }, KJ_furnaceNow]' % (slot, k))
        t.a('logInfo format ["KVX %s %d %%1 %%2 %%3 %%4", KJ_crushStats select 0, KJ_crushStats select 2, if (isNil "KJD_force") then { -1 } else { { alive _x } count KJD_force }, if (isNil "KJ_beamWrecks") then { 0 } else { KJ_beamWrecks }]' % (slot, k))
        if extra_log:
            t.a(extra_log % {'slot': slot, 'k': k})
        if k % every:
            continue
        for c, i in zip(cams, ids):
            if c.alpha >= 1:
                t.a('kvA%d = %s' % (i, c.aim))
            else:
                t.a('kvA%d = [kvA%d, %s, %g] call kvMix' % (i, i, c.aim, c.alpha))
            if len(fovs) > 1:
                t.a('kvFv%d call kvLens' % i)      # a lens change takes while the world is frozen (probe 1)
            t.a('triSetView ([kvE%d, kvA%d] call kvLook)' % (i, i))
            t.shot('%s-%s' % (slot, c.name))
    t.a('setAccTime 1')


def write(name, title, t, timeout, params='', w=1080, h=1920):
    with open(os.path.join(HERE, name + '.test.sqf'), 'w', newline='\n') as f:
        f.write(t.text())
    with open(os.path.join(HERE, name + '.test.toml'), 'w', newline='\n') as f:
        f.write(TOML.format(title=title, name=name, timeout=timeout, w=w, h=h))
    p = os.path.join(HERE, name + '.params.sqf')
    with open(p, 'w', newline='\n') as f:
        f.write(params)
    print('%s: %d statements, %d frames' % (name, len(t.lines), t.frames))
