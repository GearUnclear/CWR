"""Writes the 9:16 film's shot tests (experiment, not a regression lane).

    python gen_chupa_shots.py --name <test> --only s01_establish,s02_work,...
    python gen_chupa_shots.py --list

Each slot of video\\edit\\edl.json (cryptid-notes) that is a game shot is
staged here: the units are placed, posed and ordered, the sim runs a short
real-time pre-roll, then the frames are captured with the stepped clock of
gen_chupa_video.py (exactly 1/30 s per frame). Every frame's camera is
computed in SQF from both men's smoothed positions (triSetView), with an
optional dolly (offset eased from a start to an end value) and handheld
shake (a sum of sines, in metres). Frames are labelled with the slot name;
tools\\encode_shots.py (cryptid-notes) turns them into video\\shots\\<slot>.mp4.

The set (Everon airfield, east edge): the runway x 4759..4795, taxiway
x 4838..4856, flat at 24.7 m; the ground rises east to a crest on the
perimeter road (x 4990, 33.4 m) and falls to the forest edge (x 5095,
31.4 m). The man works at barrels on the road at (4988, 11125), facing
west; the creature first stands at the forest edge (5092, 11125), 104 m
off and visible over the crest.
"""

import argparse
import os

import gen_chupa_video as g

HERE = os.path.dirname(os.path.abspath(__file__))
EXTRA = 8  # frames captured past each slot's length

M = (4988.0, 11125.0)
TL = (5092.0, 11125.0)
FEED = (4797.0, 11104.0)

PROPS = [
    ('Hangar_Hall', 4900, 11152, 90),
    ('Barracks', 4906, 11078, 90),
    ('Su25', 4848, 11100, 180),
    ('UralRefuel', 4866, 11084, 0),
    ('Mi17', 4870, 11192, 225),
    ('Ural', 4996, 11109, 0),
    ('Barrels', 4985.0, 11123.6, 0),
    ('Barrel4', 4984.8, 11126.2, 0),
    ('Barrel4', 4983.9, 11125.4, 0),
    ('Barrel1', 4985.6, 11127.4, 0),
]

LIB = [
    # [eye [x, z, hASL], aim [x, z, hASL]] call cvLook -> triSetView array
    'cvLook = { private ["_e", "_a"]; _e = _this select 0; _a = _this select 1; '
    '[_e select 0, _e select 2, _e select 1, (_a select 0) - (_e select 0), (_a select 2) - (_e select 2), (_a select 1) - (_e select 1)] }',
    # exponential smoothing of the two men, one channel each
    'cvSA = []',
    'cvSB = []',
    'cvSmA = { private ["_p", "_a"]; _p = _this select 0; _a = _this select 1; if ((count cvSA) == 0) then { cvSA = _p }; '
    'cvSA = [(cvSA select 0) + ((_p select 0) - (cvSA select 0)) * _a, (cvSA select 1) + ((_p select 1) - (cvSA select 1)) * _a, (cvSA select 2) + ((_p select 2) - (cvSA select 2)) * _a]; cvSA }',
    'cvSmB = { private ["_p", "_a"]; _p = _this select 0; _a = _this select 1; if ((count cvSB) == 0) then { cvSB = _p }; '
    'cvSB = [(cvSB select 0) + ((_p select 0) - (cvSB select 0)) * _a, (cvSB select 1) + ((_p select 1) - (cvSB select 1)) * _a, (cvSB select 2) + ((_p select 2) - (cvSB select 2)) * _a]; cvSB }',
    # [k, amplitude m] call cvShk -> handheld offset
    'cvShk = { private ["_k", "_a"]; _k = _this select 0; _a = _this select 1; '
    '[_a * ((sin (_k * 21)) + 0.5 * (sin (_k * 53 + 40))), _a * ((sin (_k * 27 + 200)) + 0.4 * (sin (_k * 47))), _a * 0.7 * ((sin (_k * 17 + 90)) + 0.5 * (sin (_k * 61 + 10)))] }',
    # t in [0, 1] -> smoothstep
    'cvEase = { _this * _this * (3 - 2 * _this) }',
    # Pursuit steering for a creature puppeteered in the forced gallop (it moves
    # on the clip's own step along its heading): every frame it turns toward his
    # position plus 0.4 s of his velocity, offset cvSide m to the north, the
    # offset shrinking to 0 as the gap closes from 10 m. It never aims past him.
    'cvSide = 2.0',
    'cvPursue = { private ["_v", "_c", "_w", "_dx", "_dz", "_g", "_k", "_lx", "_lz"]; _v = getPosASL victim; _c = getPosASL chupa; _w = velocity victim; '
    '_dx = (_v select 0) - (_c select 0); _dz = (_v select 1) - (_c select 1); _g = sqrt (_dx * _dx + _dz * _dz); '
    '_k = _g / 10; if (_k > 1) then { _k = 1 }; '
    '_lx = (_v select 0) + 0.4 * (_w select 0); _lz = (_v select 1) + 0.4 * (_w select 1) + cvSide * _k; '
    'chupa setDir (((_lx - (_c select 0)) atan2 (_lz - (_c select 1)))); cvGap = _g }',
    'cvGap = 0',
]


def sq(v):
    return '%.3f' % v


def anchor(name, alpha):
    """SQF for an anchor point [x, z, hASL] and whether it is ground based."""
    if name == 'victim':
        return '_pv', False
    if name == 'chupa':
        return '_pc', False
    if name.startswith('mid'):
        w = float(name[3:] or 0.5)
        return ('[(_pv select 0) * %g + (_pc select 0) * %g, (_pv select 1) * %g + (_pc select 1) * %g, '
                '(_pv select 2) * %g + (_pc select 2) * %g]' % (w, 1 - w, w, 1 - w, w, 1 - w)), False
    raise ValueError(name)


# Offline framing check: ground heights from the probe 3 grid (x 4780..5140
# step 15, z 10980..11200 step 20; flat 24.66 west of 4900).
HGRID_X0, HGRID_DX, HGRID_Z0, HGRID_DZ = 4780.0, 15.0, 10980.0, 20.0
HGRID = [[float(v) for v in row.split()] for row in open(os.path.join(HERE, 'heights.txt'))]
UNITS = {}
LAST = {}


def ground(x, z):
    fx = min(max((x - HGRID_X0) / HGRID_DX, 0), len(HGRID[0]) - 1.001)
    fz = min(max((z - HGRID_Z0) / HGRID_DZ, 0), len(HGRID) - 1.001)
    i, j = int(fx), int(fz)
    ax, az = fx - i, fz - j
    h0 = HGRID[j][i] * (1 - ax) + HGRID[j][i + 1] * ax
    h1 = HGRID[j + 1][i] * (1 - ax) + HGRID[j + 1][i + 1] * ax
    return h0 * (1 - az) + h1 * az


def eval_cam(k, n, eye, aim, ease=True):
    s = min(1.0, k / max(1, n - 1))
    if ease:
        s = s * s * (3 - 2 * s)

    def base(at):
        if isinstance(at, tuple):
            return (at[0], at[1], ground(*at)), True
        if at == 'victim':
            u = UNITS['victim']
        elif at == 'chupa':
            u = UNITS['chupa']
        else:
            w = float(at[3:] or 0.5)
            a, b = UNITS['victim'], UNITS['chupa']
            u = tuple(a[i] * w + b[i] * (1 - w) for i in range(3))
        return (u[0], u[1], ground(u[0], u[1])), False

    def pt(spec, is_eye):
        o0 = spec['o0']; o1 = spec.get('o1', o0)
        o = [a + (b - a) * s for a, b in zip(o0, o1)]
        b, gr = base(spec['at'])
        x, z = b[0] + o[0], b[1] + o[1]
        if is_eye or gr:
            return (x, z, ground(x, z) + o[2])
        return (x, z, b[2] + o[2])
    return pt(eye, True), pt(aim, False)


def project(eye, aim, fov, p):
    import math
    f = [aim[i] - eye[i] for i in range(3)]
    fl = math.sqrt(sum(c * c for c in f)); f = [c / fl for c in f]
    up = (0, 0, 1)
    r = [f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]]
    rl = math.sqrt(sum(c * c for c in r)); r = [c / rl for c in r]
    u = [r[1] * f[2] - r[2] * f[1], r[2] * f[0] - r[0] * f[2], r[0] * f[1] - r[1] * f[0]]
    v = [p[i] - eye[i] for i in range(3)]
    d = sum(v[i] * f[i] for i in range(3))
    if d <= 0.05:
        return None
    # x: left -1 .. right 1 (sign follows the right vector), y: bottom -1 .. top 1
    return (sum(v[i] * r[i] for i in range(3)) / d / fov, sum(v[i] * u[i] for i in range(3)) / d / (1.778 * fov), d)


def check(slot, n, fov):
    eye, aim, ease = LAST['eye'], LAST['aim'], LAST['ease']
    out = []
    for k in (0, n - 1):
        e, a = eval_cam(k, n, eye, aim, ease)
        row = []
        for who in ('victim', 'chupa'):
            x, z, h = UNITS[who]
            g0 = ground(x, z)
            pf = project(e, a, fov, (x, z, g0))
            ph = project(e, a, fov, (x, z, g0 + h))
            if pf is None:
                row.append('%s behind' % who)
                continue
            vis = all(-1.05 < q < 1.05 for q in (pf[0], pf[1], ph[1]))
            row.append('%s x%+.2f y%+.2f..%+.2f d%.0f%s' % (who[0], pf[0], pf[1], ph[1], pf[2], '' if vis else ' OUT'))
        out.append('k%d eye(%.0f,%.0f,%.1f) %s' % (k, e[0], e[1], e[2] - ground(e[0], e[1]), ' | '.join(row)))
    print('  %-14s fov %.3f  %s' % (slot, fov, '  ||  '.join(out)))


def cam_code(n, eye, aim, shake=0.0, aim_shake=0.0, alpha=0.35, ease=True):
    """eye / aim: dict with 'at' = 'victim' | 'chupa' | 'midW' | (x, z),
    'o0' = (dx, dz, dh) start offset, 'o1' end offset (default o0).
    Eye height is always ground under the eye + dh. Aim height: unit ASL + dh,
    or ground + dh for a fixed point."""
    LAST.update(eye=eye, aim=aim, ease=ease)
    lines = ['private ["_k", "_s", "_pv", "_pc", "_e", "_a", "_h", "_g"]',
             '_k = _this select 0',
             '_s = _k / %d' % max(1, n - 1),
             'if (_s > 1) then { _s = 1 }',
             ]
    if ease:
        lines.append('_s = _s call cvEase')
    lines.append('_pv = [getPosASL victim, %g] call cvSmA' % alpha)
    lines.append('_pc = [getPosASL chupa, %g] call cvSmB' % alpha)

    def pt(spec, var, is_eye):
        o0 = spec['o0']
        o1 = spec.get('o1', o0)
        off = ['(%s + (%s - (%s)) * _s)' % (sq(a), sq(b), sq(a)) for a, b in zip(o0, o1)]
        at = spec['at']
        if isinstance(at, tuple):
            base = '[%s, %s, 0]' % (sq(at[0]), sq(at[1]))
            ground = True
        else:
            base, ground = anchor(at, alpha)
        lines.append('_h = %s' % base)
        lines.append('_h = [(_h select 0) + %s, (_h select 1) + %s, _h select 2]' % (off[0], off[1]))
        if is_eye or ground:
            lines.append('_g = [_h select 0, _h select 1] call cvGnd')
            lines.append('%s = [_h select 0, _h select 1, _g + %s]' % (var, off[2]))
        else:
            lines.append('%s = [_h select 0, _h select 1, (_h select 2) + %s]' % (var, off[2]))

    pt(eye, '_e', True)
    pt(aim, '_a', False)
    if shake:
        lines.append('_h = [_k, %g] call cvShk' % shake)
        lines.append('_e = [(_e select 0) + (_h select 0), (_e select 1) + (_h select 1), (_e select 2) + (_h select 2)]')
    if aim_shake:
        lines.append('_h = [_k + 37, %g] call cvShk' % aim_shake)
        lines.append('_a = [(_a select 0) + (_h select 0), (_a select 1) + (_h select 1), (_a select 2) + (_h select 2)]')
    lines.append('[_e, _a] call cvLook')
    return 'cvCamF = { ' + '; '.join(lines) + ' }'


def setup_world(t):
    g.header(t, 'Chupacabra 9:16 film shots: ' + t.title)
    t.lines[2] = '//  Written by gen_chupa_shots.py (edit that, not this file).'
    for s in LIB:
        t.a(s)
    for c, x, z, d in PROPS:
        t.a('cvO = "%s" createVehicle [%g, %g, 0]' % (c, x, z))
        t.a('cvO setDir %g' % d)
        t.a('cvO setPos [%g, %g, 0]' % (x, z))
    t.a('cvGV = createGroup civilian')
    t.a('cvGC = createGroup civilian')
    t.a('victim = objNull')
    t.a('chupa = objNull')
    t.a('"Civilian" createUnit [[%g, %g, 0], cvGV, "victim = this", 0.5, "PRIVATE"]' % M)
    t.a('"UDChupacabraAnim" createUnit [[%g, %g, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]' % TL)
    t.a('triSimFrames 5')
    t.a('triAssert [!(isNull victim)]')
    t.a('triAssert [!(isNull chupa)]')
    t.a('{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false } forEach [victim, chupa]')
    t.a('cvGV setBehaviour "CARELESS"')
    t.a('cvGV setSpeedMode "FULL"')
    t.a('cvGC setBehaviour "SAFE"')
    t.a('cvGC setSpeedMode "FULL"')
    t.a('chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]')
    t.a('victim addEventHandler ["AnimChanged", {logInfo format ["CVA victim %1 %2", time, _this select 1]}]')
    t.a('triSimFrames 60')


def place(t, who, x, z, d, move=None, pos=None, ai_move=False, ai_anim=True, h=None):
    if h is None:
        h = 1.75 if who == 'victim' else 1.1
        if move in ('CivilPutDown',):
            h = 1.0
        if move in ('CivilLyingDead', 'ChupaFeed', 'ChupaCrawlF'):
            h = 0.4
    UNITS[who] = (x, z, h)
    t.a('%s stop false' % who)
    t.a('%s setPos [%s, %s, 0]' % (who, sq(x), sq(z)))
    t.a('%s setDir %g' % (who, d))
    t.a('%s %s "MOVE"' % (who, 'enableAI' if ai_move else 'disableAI'))
    t.a('%s %s "ANIM"' % (who, 'enableAI' if ai_anim else 'disableAI'))
    if pos:
        t.a('%s setUnitPos "%s"' % (who, pos))
    t.a('%s doMove (getPos %s)' % (who, who))
    if move:
        t.a('%s switchMove "%s"' % (who, move))


def fresh_chupa(t, x, z, d, pos='DOWN'):
    """A new creature for shots its own AI drives: after disableAI "ANIM" the
    old one never takes orders again (enableAI does not bring it back)."""
    t.a('deleteVehicle chupa')
    t.a('triSimFrames 2')
    t.a('"UDChupacabraAnim" createUnit [[%s, %s, 0], cvGC, "chupa = this", 0.5, "PRIVATE"]' % (sq(x), sq(z)))
    t.a('triSimFrames 2')
    t.a('triAssert [!(isNull chupa)]')
    t.a('removeAllWeapons chupa')
    t.a('chupa setCaptive true')
    t.a('chupa allowDammage false')
    t.a('chupa addEventHandler ["AnimChanged", {logInfo format ["CVA chupa %1 %2", time, _this select 1]}]')
    t.a('chupa setPos [%s, %s, 0]' % (sq(x), sq(z)))
    t.a('chupa setDir %g' % d)
    t.a('chupa setUnitPos "%s"' % pos)
    t.a('cvGC setBehaviour "SAFE"')
    t.a('cvGC setSpeedMode "FULL"')
    UNITS['chupa'] = (x, z, 1.1)


def preroll(t, secs):
    t.a('setAccTime 1')
    if secs > 0:
        t.a('if (true) then { cvT0 = time }')
        t.a('triSimUntil { time > cvT0 + %g }' % secs)


def capture(t, slot, n, cam, fov, events=None, lens_target=(4900, 14000), perframe=None, pre=0):
    """perframe: a statement run before every step; pre: stepped frames run
    (with perframe, no screenshot) before the capture."""
    events = events or {}
    check(slot, n, fov)
    t.a('cvSA = []')
    t.a('cvSB = []')
    t.a(cam)
    g.lens(t, fov, *lens_target)
    t.a('call { setAccTime 0 }')
    for k in range(pre):
        if perframe:
            t.a(perframe)
        t.a(g.STEP)
    t.a('logInfo format ["CVS %%1 begin %%2 frames %%3 victim %%4 %%5 chupa %%6 %%7", "%s", time, %d, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa]' % (slot, n))
    for k in range(n):
        for e in events.get(k, []):
            t.a(e)
        if perframe:
            t.a(perframe)
        t.a(g.STEP)
        t.a('triSetView ([%d] call cvCamF)' % k)
        t.a('logInfo format ["CVF %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8", "%s", %d, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa]' % (slot, k))
        t.shot(slot)
    t.a('setAccTime 1')
    t.a('logInfo format ["CVS %%1 end %%2 gap %%3", "%s", time, cvGap]' % slot)


def man_look(who_dir0, who_dir1, k0, k1):
    """setDir the victim from dir0 to dir1 over frames k0..k1 (events)."""
    ev = {}
    for k in range(k0, k1 + 1):
        s = (k - k0) / max(1, k1 - k0)
        s = s * s * (3 - 2 * s)
        d = who_dir0 + (who_dir1 - who_dir0) * s
        ev.setdefault(k, []).append('victim setDir %.2f' % d)
    return ev


def merge(*evs):
    out = {}
    for e in evs:
        for k, v in e.items():
            out.setdefault(k, []).extend(v)
    return out


# --------------------------------------------------------------------------
# The shots. Each returns nothing; it writes into t. n = slot frames + EXTRA.
# --------------------------------------------------------------------------
def s01_establish(t, n):
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    place(t, 'chupa', TL[0], TL[1], 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.4)
    # From the runway, east past the parked jet: hall, slope, the man on the crest, the forest.
    cam = cam_code(n, {'at': (4806.0, 11082.0), 'o0': (0, 0, 7.0), 'o1': (12, 5, 5.5)},
                   {'at': (4975.0, 11122.0), 'o0': (0, 0, -1.0), 'o1': (0, 0, -0.5)})
    capture(t, 's01_establish', n, cam, 0.26)


def s02_work(t, n):
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    place(t, 'chupa', TL[0], TL[1], 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.5)
    # Low, three quarter front from the base side; the slope and the forest behind him.
    cam = cam_code(n, {'at': 'victim', 'o0': (-1.2, -3.9, 0.75), 'o1': (-1.0, -3.4, 0.7)},
                   {'at': 'victim', 'o0': (0.3, 0, 0.75), 'o1': (0.3, 0, 0.7)}, aim_shake=0.004)
    capture(t, 's02_work', n, cam, 0.36)


def s03_notice(t, n):
    # He stands and turns to the forest; the camera beside him, low, looking
    # past him east (the shape is a speck on the far tree line).
    place(t, 'victim', M[0], M[1], 270, 'Civil')
    place(t, 'chupa', TL[0], TL[1], 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.4)
    ev = man_look(270, 450, 4, 30)
    ev = merge(ev, {32: ['victim doWatch [%g, %g, 1]' % TL]})
    cam = cam_code(n, {'at': 'victim', 'o0': (2.6, -3.3, 1.2), 'o1': (2.3, -2.9, 1.25)},
                   {'at': 'victim', 'o0': (0, 0, 1.35), 'o1': (0, 0, 1.45)}, aim_shake=0.006)
    capture(t, 's03_notice', n, cam, 0.42, ev)


def s04_freeze(t, n):
    # Long lens from his eye: it stands still at the tree line; the freeze clip tilts the head.
    place(t, 'victim', M[0], M[1], 90, 'Civil')
    place(t, 'chupa', TL[0] - 4, TL[1], 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 1.0)
    cam = cam_code(n, {'at': M, 'o0': (-0.6, -0.5, 1.7)},
                   {'at': 'chupa', 'o0': (0, 0, 0.75), 'o1': (0, 0, 0.8)}, shake=0.0, aim_shake=0.02)
    capture(t, 's04_freeze', n, cam, 0.011)


def s05_shrug(t, n):
    # He turns back to the barrels and kneels; the shape still there, a speck behind him.
    place(t, 'victim', M[0], M[1], 90, 'Civil')
    place(t, 'chupa', TL[0], TL[1], 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.4)
    ev = merge(man_look(90, 270, 6, 30), {34: ['victim switchMove "CivilPutDown"']})
    cam = cam_code(n, {'at': 'victim', 'o0': (-4.2, 1.8, 1.25), 'o1': (-3.7, 1.5, 1.1)},
                   {'at': 'victim', 'o0': (0, 0, 1.2), 'o1': (0, 0, 0.8)}, aim_shake=0.004)
    capture(t, 's05_shrug', n, cam, 0.30, ev)


def s06_closer(t, n):
    # He looks back (his eye, long lens): it stands on the open slope, 42 m off, still.
    place(t, 'victim', M[0], M[1], 90, 'Civil')
    place(t, 'chupa', 5030, 11125, 270, 'ChupaIdle', pos='UP', ai_anim=False)
    preroll(t, 0.8)
    cam = cam_code(n, {'at': M, 'o0': (-0.6, -0.5, 1.6)},
                   {'at': 'chupa', 'o0': (0, 0, 1.05), 'o1': (0, 0, 1.1)}, aim_shake=0.012)
    capture(t, 's06_closer', n, cam, 0.022)


def s07_gone(t, n):
    # The same long lens as s04: the tree line is empty.
    place(t, 'victim', M[0], M[1], 90, 'Civil')
    place(t, 'chupa', 5300, 11400, 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.3)
    # The same eye, aim and lens as s04, on the spot where it stood.
    cam = cam_code(n, {'at': M, 'o0': (-0.6, -0.5, 1.7)},
                   {'at': (TL[0] - 4, TL[1]), 'o0': (0, 0, 0.75), 'o1': (0, 0, 0.8)}, aim_shake=0.02)
    capture(t, 's07_gone', n, cam, 0.011)


def s08_crawl(t, n):
    # In the grass, 30 m from him: it crawls (forced clip, it moves on its own step).
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    place(t, 'chupa', M[0] + 30, M[1] - 1, 268, 'ChupaCrawlF', pos='DOWN', ai_anim=False)
    preroll(t, 0.6)
    cam = cam_code(n, {'at': 'chupa', 'o0': (3.2, -0.9, 0.35), 'o1': (2.7, -0.8, 0.35)},
                   {'at': 'mid0.12', 'o0': (0, 0, 0.45)}, aim_shake=0.006)
    capture(t, 's08_crawl', n, cam, 0.38)


def s09_stalk(t, n):
    # Stalk crouch, 14 m off: side on and low; his back in the background.
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    place(t, 'chupa', M[0] + 14, M[1] + 0.5, 268, 'ChupaStalkF', pos='UP', ai_anim=False)
    preroll(t, 0.5)
    cam = cam_code(n, {'at': 'chupa', 'o0': (3.0, -0.9, 0.55), 'o1': (2.6, -0.8, 0.55)},
                   {'at': 'mid0.2', 'o0': (0, 0, 0.7)}, aim_shake=0.004)
    capture(t, 's09_stalk', n, cam, 0.34)


def s09b_behind(t, n):
    # Low behind it: over its shoulder to his back, 6 m closing to 3.5 m; it stops dead at the end.
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    place(t, 'chupa', M[0] + 6.4, M[1] + 0.3, 268, 'ChupaStalkF', pos='UP', ai_anim=False)
    preroll(t, 0.4)
    ev = {44: ['victim switchMove "CivilPutDown"'], n - 22: ['chupa switchMove "ChupaFreeze"']}
    cam = cam_code(n, {'at': 'chupa', 'o0': (1.7, 0.75, 1.05), 'o1': (1.3, 0.6, 1.0)},
                   {'at': 'victim', 'o0': (0, 0, 0.55)}, aim_shake=0.003)
    capture(t, 's09b_behind', n, cam, 0.40, ev)


def s10_snap(t, n):
    # Low, in front of him: he works, unaware; behind him it rises to full height.
    place(t, 'victim', M[0], M[1], 270, 'CivilPutDown')
    # Lying in wait (DOWN, the AI holds the freeze); UP makes the AI play the
    # snap up and then stand in its idle.
    # Forced: the snap up, then the idle (the AI's own UP from the freeze
    # skips the snap when it is not moving).
    place(t, 'chupa', M[0] + 3.2, M[1] + 0.3, 270, 'ChupaFreeze', pos='DOWN', ai_anim=False)
    preroll(t, 0.6)
    ev = {6: ['chupa switchMove "ChupaSnapUp"'], 20: ['chupa switchMove "ChupaIdle"'], 40: ['victim switchMove "CivilPutDown"']}
    cam = cam_code(n, {'at': 'mid0.5', 'o0': (0.2, -6.4, 0.4), 'o1': (0.2, -5.9, 0.38)},
                   {'at': 'mid0.5', 'o0': (0, 0, 0.8), 'o1': (0, 0, 1.0)}, aim_shake=0.004)
    capture(t, 's10_snap', n, cam, 0.36, ev)


def s11_turn(t, n):
    # Over its shoulder, close: he stands and turns into it.
    place(t, 'victim', M[0], M[1], 270, 'Civil')
    place(t, 'chupa', M[0] + 2.9, M[1] + 0.3, 270, 'ChupaIdle', pos='UP', ai_anim=False)
    preroll(t, 0.3)
    ev = man_look(270, 90, 6, 22)
    cam = cam_code(n, {'at': 'chupa', 'o0': (1.3, 0.9, 1.75), 'o1': (1.0, 0.7, 1.7)},
                   {'at': 'victim', 'o0': (0, 0, 1.35), 'o1': (0, 0, 1.5)}, shake=0.012, aim_shake=0.01)
    capture(t, 's11_turn', n, cam, 0.34, ev)


def est(dv, dc):
    """Shift the check's unit positions by (dx, dz) after a pre-roll."""
    v, c = UNITS['victim'], UNITS['chupa']
    UNITS['victim'] = (v[0] + dv[0], v[1] + dv[1], v[2])
    UNITS['chupa'] = (c[0] + dc[0], c[1] + dc[1], c[2])


def run_orders(t, vx, vz, cx, cz, vgo, cgo):
    """He runs to vgo; it goes on all fours to cgo (a point on a line 2 m to his side)."""
    place(t, 'victim', vx, vz, 270, 'Civil', ai_move=True, pos='UP')
    fresh_chupa(t, cx, cz, 270, 'DOWN')
    t.a('victim doMove [%s, %s, 0]' % (sq(vgo[0]), sq(vgo[1])))
    t.a('chupa doMove [%s, %s, 0]' % (sq(cgo[0]), sq(cgo[1])))


def s12_bolt(t, n):
    # He bolts down the slope toward the lens; behind him it drops to all fours
    # and comes. A low fixed camera ahead of them, panning.
    place(t, 'victim', 4987.6, 11120.5, 262, 'Civil', ai_move=True, pos='UP')
    fresh_chupa(t, 4994.0, 11117.0, 262, 'UP')
    t.a('chupa doMove (getPos chupa)')
    preroll(t, 2.0)
    t.a('victim doMove [4890, 11108, 0]')
    preroll(t, 0.7)
    t.a('chupa setUnitPos "DOWN"')
    t.a('chupa doMove [4870, 11102, 0]')
    preroll(t, 0.05)
    est((-1.5, 0), (0, 0))
    cam = cam_code(n, {'at': 'chupa', 'o0': (2.6, -3.0, 1.2)},
                   {'at': 'mid0.3', 'o0': (0, 0, 0.7)}, shake=0.035, aim_shake=0.02, ease=False, alpha=0.45)
    capture(t, 's12_bolt', n, cam, 0.5)


def pursuit(t, vx, vz, gap, vgo, side=2.0):
    """He runs west to vgo; a fresh creature, `gap` m behind and `side` m north,
    is puppeteered in the forced gallop and steered every frame by cvPursue
    (no doMove, so no re-plan dips, and it never aims past him). The forced
    gallop runs at its band minimum, about 7.4 m/s against his 4.75: it closes
    about 2.7 m/s, so a 2.3 s shot that starts 11 m back ends about 4 m back."""
    place(t, 'victim', vx, vz, 270, 'Civil', ai_move=True, pos='UP')
    fresh_chupa(t, vx + gap, vz + side, 270, 'DOWN')
    t.a('cvSide = %g' % side)
    t.a('victim doMove [%s, %s, 0]' % (sq(vgo[0]), sq(vgo[1])))
    preroll(t, 0.5)
    t.a('chupa disableAI "MOVE"')
    t.a('chupa disableAI "ANIM"')
    t.a('chupa switchMove "ChupaGallop"')
    UNITS['victim'] = (vx - 2.5, vz, 1.75)
    UNITS['chupa'] = (vx + gap - 1.5, vz + side, 1.1)


def s13_lookback(t, n):
    # In front of him, looking back up the slope past him: it gains, steered
    # onto him (pursuit), never level with him.
    pursuit(t, 4964, 11122, 12.0, (4870, 11116))
    cam = cam_code(n, {'at': 'victim', 'o0': (-5.0, 1.0, 1.35), 'o1': (-4.0, 0.9, 1.35)},
                   {'at': 'mid0.5', 'o0': (0, 0, 0.9)}, shake=0.035, aim_shake=0.025, ease=False)
    capture(t, 's13_lookback', n, cam, 0.36, perframe='[] call cvPursue', pre=6)


def s14_wide(t, n):
    # Wide from the crest behind them: both running down toward the base, the
    # gap closing (the pale body reads against the tarmac, not the grass).
    run_orders(t, 4966, 11121, 4984, 11119, (4875, 11114), (4855, 11116.2))
    preroll(t, 1.0)
    est((-3.0, 0), (-2.0, 0))
    cam = cam_code(n, {'at': 'chupa', 'o0': (6.5, -2.2, 3.0)},
                   {'at': 'victim', 'o0': (0, 0, 0.8)}, shake=0.02, aim_shake=0.012, ease=False, alpha=0.35)
    capture(t, 's14_wide', n, cam, 0.4)


def s15_hide(t, n):
    # Past the parked jet: he cuts behind it.
    run_orders(t, 4884, 11114, 4900, 11116.2, (4790, 11114), (4770, 11116.2))
    preroll(t, 1.0)
    est((-3.0, 0), (-2.3, 0))
    # Low by the jet's port wing tip: he runs past behind the fuselage.
    cam = cam_code(n, {'at': 'victim', 'o0': (7.0, 1.6, 1.3), 'o1': (6.0, 1.4, 1.2)},
                   {'at': 'victim', 'o0': (-2.0, -0.4, 0.9)}, shake=0.03, aim_shake=0.02, alpha=0.3)
    capture(t, 's15_hide', n, cam, 0.36)


def s16_burst(t, n):
    # Tracking backward in front of it past the hall: full gallop at the lens,
    # him ahead (behind the camera), steered onto him (pursuit).
    pursuit(t, 4860, 11114, 11.0, (4770, 11114))
    cam = cam_code(n, {'at': 'chupa', 'o0': (-3.6, -2.4, 0.9)},
                   {'at': 'chupa', 'o0': (0, 0, 0.55)}, shake=0.035, aim_shake=0.02, ease=False, alpha=0.45)
    capture(t, 's16_burst', n, cam, 0.5, perframe='[] call cvPursue', pre=6)


def s17_heels(t, n):
    # Low behind it, him ahead, the gap closing to a few metres (pursuit).
    pursuit(t, 4836, 11114, 11.5, (4740, 11114))
    cam = cam_code(n, {'at': 'chupa', 'o0': (3.0, -1.6, 0.5)},
                   {'at': 'mid0.3', 'o0': (0, 0, 0.7)}, shake=0.035, aim_shake=0.02, ease=False, alpha=0.5)
    capture(t, 's17_heels', n, cam, 0.5, perframe='[] call cvPursue', pre=6)


def chase_take(t, n, label, cams):
    """The staged take (cv_chase.sqs): gallop, pounce at 3.5 m, pin and feed,
    recoil, he scrambles up and runs, it rushes, lunge strike. The camera follows
    CHS_STAGE: cams = {stage: (cam_code, fov)}; each stage's code is cvCamS_<stage>."""
    t.a('CHS_DX = -1')
    t.a('CHS_POUNCE_AT = 3.5')
    t.a('CHS_DROP_AFTER = 0.35')
    t.a('CHS_LUNGE_AT = 3.5')
    t.a('CHS_FEED_HOLD = 1.6')
    t.a('CHS_RUSH_DELAY = 1.4')
    t.a('CHS_CHASE_MAX = 20')
    t.a('CHS_REORDER = 30')
    t.a('CHS_SIDE = -2.2')
    t.a('CHS_RUSH_GAP = 6')
    t.a('CHS_STAGE = "pre"')
    place(t, 'victim', 4826, 11114, 270, 'Civil', ai_move=True, pos='UP')
    fresh_chupa(t, 4835, 11114 + 2.2, 270, 'DOWN')
    t.a('victim doMove [4780, 11114, 0]')
    t.a('chupa doMove [4780, 11116.2, 0]')
    preroll(t, 0.6)
    for st, (code, fov) in cams.items():
        t.a(code.replace('cvCamF = {', 'cvCamS_%s = {' % st, 1))
    # One dispatcher: the stage picks the camera code and its lens.
    t.a('cvStageLast = ""')
    disp = ['private ["_c"]', '_c = cvCamS_chase']
    for st in cams:
        disp.append('if (CHS_STAGE == "%s") then { _c = cvCamS_%s }' % (st, st))
    disp.append('if (CHS_STAGE == "end") then { _c = cvCamS_lunge }')
    disp.append('_this call _c')
    t.a('cvCamF = { ' + '; '.join(disp) + ' }')
    lens_ev = []
    for st, (code, fov) in cams.items():
        lens_ev.append('if ((CHS_STAGE == "%s") && (cvStageLast != "%s")) then { cvHud camSetFov %g; cvHud camCommit 0; cvSA = []; cvSB = []; '
                       'logInfo format ["CVS stage %%1 %%2", CHS_STAGE, time] }' % (st, st, fov))
    lens_ev.append('cvStageLast = CHS_STAGE')
    t.a('cvLensF = { ' + '; '.join(lens_ev) + ' }')
    t.a('CHS_DONE = false')
    t.a('[chupa, victim] exec "cv_chase.sqs"')
    t.a('CHS_STAGE = "chase"')
    g.lens(t, cams['chase'][1])
    t.a('logInfo format ["CVS %%1 begin %%2 frames %%3", "%s", time, %d]' % (label, n))
    t.a('call { setAccTime 0 }')
    for k in range(n):
        t.a(g.STEP)
        t.a('[] call cvLensF')
        t.a('triSetView ([%d] call cvCamF)' % k)
        t.a('logInfo format ["CVF %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8 %%9", "%s", %d, time, getPosASL victim, getMove victim, getPosASL chupa, getMove chupa, getDir chupa, CHS_STAGE]' % (label, k))
        t.shot(label)
    t.a('setAccTime 1')
    t.a('logInfo format ["CVS %%1 end %%2 done %%3", "%s", time, CHS_DONE]' % label)


def chase_a(t, n):
    cams = {
        # gallop: tracking low beside them
        'chase': (cam_code(n, {'at': 'mid0.5', 'o0': (0, -11.0, 0.7)}, {'at': 'mid0.5', 'o0': (0, 0, 0.8)},
                           shake=0.03, aim_shake=0.02, ease=False), 0.50),
        # pounce and pin: low, side on, close
        'pounce': (cam_code(n, {'at': 'victim', 'o0': (-1.0, -4.2, 0.55)}, {'at': 'mid0.5', 'o0': (0, 0, 0.6)},
                            shake=0.02, aim_shake=0.015, ease=False, alpha=0.5), 0.40),
        'feed': (cam_code(n, {'at': 'victim', 'o0': (-1.0, -2.7, 0.6)}, {'at': 'mid0.5', 'o0': (0, 0, 0.4)},
                          shake=0.012, aim_shake=0.01, ease=False, alpha=0.3), 0.40),
        # he shoves it off: from his side, it reels back
        'recoil': (cam_code(n, {'at': 'mid0.5', 'o0': (-1.0, 3.6, 0.5)}, {'at': 'mid0.5', 'o0': (0, 0, 0.5)},
                            shake=0.02, aim_shake=0.015, ease=False, alpha=0.3), 0.40),
        # it comes again: low behind it, him ahead
        'rush': (cam_code(n, {'at': 'chupa', 'o0': (2.6, -0.9, 0.9)}, {'at': 'victim', 'o0': (0, 0, 1.0)},
                          shake=0.035, aim_shake=0.02, ease=False, alpha=0.45), 0.40),
        # the strike: in front of him, it fills the frame
        # the strike: past his shoulder, it lunges into the lens
        'lunge': (cam_code(n, {'at': 'mid0.5', 'o0': (0.6, -3.8, 1.1)}, {'at': 'mid0.5', 'o0': (0, 0, 1.0)},
                           shake=0.04, aim_shake=0.03, ease=False, alpha=0.6), 0.45),
    }
    chase_take(t, n, 'chase_a', cams)


def feed_stage(t):
    # He lies at the runway edge; it is on him, feeding (held by stop + DOWN).
    place(t, 'victim', FEED[0], FEED[1], 200, 'CivilLyingDead')
    place(t, 'chupa', FEED[0] + 0.85, FEED[1] + 0.2, 250, 'ChupaFeed', pos='DOWN', ai_anim=False)
    t.a('chupa stop true')


def s22_feed(t, n):
    feed_stage(t)
    preroll(t, 1.0)
    cam = cam_code(n, {'at': FEED, 'o0': (-24, -12, 2.2), 'o1': (-17, -8.5, 1.8)},
                   {'at': FEED, 'o0': (0.4, 0, 0.45)}, aim_shake=0.002)
    capture(t, 's22_feed', n, cam, 0.10)


def s23_feedlow(t, n):
    feed_stage(t)
    preroll(t, 1.0)
    cam = cam_code(n, {'at': FEED, 'o0': (-3.5, -4.2, 0.35), 'o1': (-2.2, -4.4, 0.35)},
                   {'at': FEED, 'o0': (0.6, 0.1, 0.45)}, aim_shake=0.003)
    capture(t, 's23_feedlow', n, cam, 0.34)


def s24_lookup(t, n):
    # It feeds, then rises and looks into the lens (the idle clip's head snap).
    feed_stage(t)
    t.a('chupa setDir 230')
    preroll(t, 1.0)
    # Frame 40: feed to idle is a one-frame pop; the film uses frames 45 on,
    # slowed (video	ools\punch.py). A fresh creature here lands off its mark.
    ev = {40: ['chupa switchMove "ChupaIdle"']}
    # camera in front of it (it faces 230 deg: south west)
    cam = cam_code(n, {'at': FEED, 'o0': (0.85 - 3.0, 0.2 - 2.6, 0.75), 'o1': (0.85 - 2.3, 0.2 - 2.0, 0.85)},
                   {'at': FEED, 'o0': (0.85, 0.2, 0.55), 'o1': (0.85, 0.2, 1.1)}, aim_shake=0.003)
    capture(t, 's24_lookup', n, cam, 0.34, ev)


def s25_far(t, n):
    feed_stage(t)
    preroll(t, 1.0)
    cam = cam_code(n, {'at': FEED, 'o0': (-70, -55, 9), 'o1': (-66, -52, 8.5)},
                   {'at': FEED, 'o0': (8, 6, 1.0)}, aim_shake=0.002)
    capture(t, 's25_far', n, cam, 0.2)


SHOTS = {
    's01_establish': s01_establish, 's02_work': s02_work, 's03_notice': s03_notice,
    's04_freeze': s04_freeze, 's05_shrug': s05_shrug, 's06_closer': s06_closer,
    's07_gone': s07_gone, 's08_crawl': s08_crawl, 's09_stalk': s09_stalk,
    's09b_behind': s09b_behind, 's10_snap': s10_snap, 's11_turn': s11_turn,
    's12_bolt': s12_bolt, 's13_lookback': s13_lookback, 's14_wide': s14_wide,
    's15_hide': s15_hide, 's16_burst': s16_burst, 's17_heels': s17_heels,
    'chase_a': chase_a,
    's22_feed': s22_feed, 's23_feedlow': s23_feedlow, 's24_lookup': s24_lookup, 's25_far': s25_far,
}


def slot_frames():
    import json
    edl = json.load(open(r'D:\Arma_CWA_worktrees\cryptid-notes\video\edit\edl.json'))
    fr = {}
    for s in edl['slots']:
        fr[s['name']] = round(s['t1'] * 30) - round(s['t0'] * 30)
    fr['chase_a'] = 450
    return fr


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--name', default='chupa_shots')
    ap.add_argument('--only', default='')
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--frames', type=int, default=0, help='override frame count (tests)')
    args = ap.parse_args()
    fr = slot_frames()
    if args.list:
        for k in SHOTS:
            print(k, fr.get(k))
        return
    names = [x for x in args.only.split(',') if x] or list(SHOTS)
    t = g.T()
    t.title = ', '.join(names)
    setup_world(t)
    for nm in names:
        n = args.frames or (fr[nm] + (EXTRA if nm != 'chase_a' else 0))
        t.a('// ---- %s (%d frames)' % (nm, n))
        SHOTS[nm](t, n)
        t.a('triSimFrames 2')
    t.a('triEndTest')
    timeout = int(240 + t.frames * 1.3)
    g.write(args.name, 'Chupacabra 9:16 film shots', t, timeout, 1080, 1920)


if __name__ == '__main__':
    main()
