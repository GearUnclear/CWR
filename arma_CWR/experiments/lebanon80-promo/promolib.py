"""Shared pieces of the Lebanon80 promo shoot tests (experiment, not a regression lane).

Each take boots PromoShoot.Lebanon80 (a copy of the Guerrilla.Lebanon80 template: the same description.ext
and gmcore bootstrap, so the native gm* services run as in a campaign; mission.sqm activates extra @LoBo
addons for staging and names the briefing "Guerrilla", the template's own title) with @LoBo mounted from
inside the data dir, W x H (2560 x 1440), --nosound.

Recording: the whole take runs at a fixed simulation step of 1/30 s with setAccTime 1 (triSetFixedStep), so
the main loop renders exactly one frame per 1/30 s of game time however long a frame takes. The object
detail is pinned at its finest (triPinObjectDetail 1, the pop-in fix: popin-diagnosis.md) and every clip
is preceded by triSettleScene [300, 30, 2] (logged with triSceneStats; check_run.py fails a take whose
settle is NOT settled or whose detail is not lod 0.010416667, pin 1). A clip is triRecord [<name>.raw,
HEAD + frames, 0] (every presented frame, post gamma, raw RGB24) armed in the same statement as the camera
move (pcGo), which holds the first pose for the HEAD frames (15) and then moves; encode_promo.py cuts the
head, so the delivered clip starts on the first moving frame. A top-level triRecordState then waits until
the dump is written. encode_promo.py turns each dump into clips\\<name>.mp4 and deletes it.

The camera is a mission camera (camCreate + cameraEffect, so no HUD), stepped every frame by
PromoShoot.Lebanon80\\promo_cam.sqs through pcStep (promo_lib.sqf): kinds lerp (eye and target eased from
A to B), orbit (around a point or object), follow (an offset in a moving object's frame, smoothed), track
(eye eased from A to B, target follows an object, smoothed) and dz (dolly zoom). With aim=True the lens is
pointed so that the filtered subject point sits at (sx, sy) on the screen (rule of thirds). While a clip
records pcStep logs one PCF line a frame (eye, target, the subject's screen position, the horizon line, the
kaiju tremor gain, the raw subject position) for gate.py (thirds) and judder.py.
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
MISSION = 'experiments/lebanon80-promo/PromoShoot.Lebanon80'
W, H = 2560, 1440
HEAD = 15  # frames held on the first pose at the start of every recording, cut by encode_promo.py
# retakes: gen_promo.py --only c02,c03 --tag r2 writes promo_<take>_r2 recording only those clips (the
# others are skipped but their time still passes, so the staging keeps its timing)
ONLY = None
SUFFIX = ''
THIRD = 1.0 / 3.0
TWO3 = 2.0 / 3.0

TOML = """# {title} (experiment, kept out of tests/integration so a sweep never runs it).
# Written by gen_promo.py (edit that, not this file). Run it through run-promo.ps1 -Name {name}.
# One game, hidden desktop, --nosound. Needs the shoot shadow (run-promo.ps1 -DataDir, see README.md).
tags = ["experiment", "video", "lebanon80"]
binary = "PoseidonGame"
mission = "{mission}"
timeout = {timeout}
assert_timeout = 900
extra_args = ["--nosound", "--no-strict", "--width", "{w}", "--height", "{h}", "--mods-dir", ".", "--mod", "{mods}"]
"""


def q(v):
    """SQF literal of a python value."""
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, (int, float)):
        return ('%g' % v) if isinstance(v, float) else str(v)
    if isinstance(v, (list, tuple)):
        return '[' + ', '.join(q(x) for x in v) + ']'
    return v  # an SQF expression already


class T:
    def __init__(self, name, title):
        self.name = name + SUFFIX
        self.title = title
        self.lines = []
        self.clips = []
        self.comp = {}

    def a(self, s):
        assert '\n' not in s, s
        self.lines.append(s)

    def c(self, s):
        """one statement wrapped in call { } (so a FAIL-shaped answer never reaches tri)"""
        self.a('call { %s }' % s)

    def log(self, fmt, *args):
        self.c('logInfo format ["PROMO %s"%s]' % (fmt, ''.join(', ' + x for x in args)))


DAY_DATE = (1985, 6, 21)
# true night (night-diagnosis.md): 16 June has a new moon below the horizon from 18:30 to 04:00 (moon light
# share 0); 21 June has a half moon high in the evening sky. Night means 18:30 or later (sun 19 degrees under).
NIGHT_DATE = (1985, 6, 16)


def header(t, hour, minute=0, overcast=0.25, fog=0.05, rain=0.0, view=3000, player_pos=None, date=DAY_DATE):
    t.a('// ' + '=' * 76)
    t.a('//  %s' % t.title)
    t.a('//  Written by gen_promo.py (edit that, not this file). --nosound.')
    t.a('// ' + '=' * 76)
    t.a('triSimUntil { GM_LIB_READY }')
    t.a('triSimUntil { alive player }')
    t.a('triCaptureAtPresent true')
    t.c('prLit = triSetLighting "remastered"; logInfo format ["PROMO lit %1", prLit]')
    # the pop-in fix (popin-diagnosis.md): the object detail held at its finest for the whole take
    t.c('prPin = triPinObjectDetail 1; logInfo format ["PROMO pin %1", prPin]')
    t.c('logInfo format ["PROMO window %1", triGetWindowSize]')
    t.a('player allowDammage false')
    t.a('player setCaptive true')
    t.a('enableRadio false')
    t.a('showCinemaBorder false')
    t.a('call compile preprocessFile "promo_lib.sqf"')
    if player_pos:
        t.c('player setPos %s' % q(list(player_pos)))
    t.date = tuple(date)
    t.c('setDate [%d, %d, %d, %d, %d]' % (t.date + (hour, minute)))
    t.c('0 setOvercast %g; 0 setFog %g; 0 setRain %g' % (overcast, fog, rain))
    t.c('setViewDistance %d' % view)
    t.a('PC_CAM = "camera" camCreate (getPos player)')
    t.a('PC_CAM cameraEffect ["internal", "back"]')
    t.a('[] exec "promo_cam.sqs"')
    t.a('[] exec "civclear.sqs"')
    t.c('prStep = triSetFixedStep 0.0333333; setAccTime 1; logInfo format ["PROMO step %1", prStep]')
    t.log('start %1 daytime %2 player %3', 'typeOf player', 'daytime', 'getPos player')
    t.a('triSimFrames 30')


def time_of_day(t, hour, minute=0, overcast=None, fog=None, rain=None, date=None):
    t.c('setDate [%d, %d, %d, %d, %d]' % (tuple(date or getattr(t, 'date', DAY_DATE)) + (hour, minute)))
    if overcast is not None:
        t.c('0 setOvercast %g' % overcast)
    if fog is not None:
        t.c('0 setFog %g' % fog)
    if rain is not None:
        t.c('0 setRain %g' % rain)


COMP_KEYS = ('subj', 'subjp', 'z', 'subj2', 'subjp2', 'z2', 'sx', 'sy', 'aim', 'fa', 'fb')


def cam(t, kind, **kw):
    """sets the camera globals for the next move and holds the first pose.

    Composition keys: subj (object expression) or subjp (position [x, y, metres up]) with z (metres above
    its feet); subj2 / subjp2 / z2 (a second subject, logged only); sx, sy (where the subject sits on the
    screen, 0..1); aim (point the lens so it sits there); fa, fb (the subject filter)."""
    comp = {k: kw.pop(k) for k in COMP_KEYS if k in kw}
    parts = ['PC_KIND = "%s"' % kind]
    defaults = dict(ease=1, asl=False, fov0=0.7, fov1=None)
    d = dict(defaults)
    d.update(kw)
    if d['fov1'] is None:
        d['fov1'] = d['fov0']
    keymap = dict(ease='PC_EASE', asl='PC_ASL', fov0='PC_FOV0', fov1='PC_FOV1', e0='PC_E0', e1='PC_E1', g0='PC_G0',
                  g1='PC_G1', c='PC_C', cobj='PC_COBJ', r0='PC_R0', r1='PC_R1', h0='PC_H0', h1='PC_H1', a0='PC_A0',
                  a1='PC_A1', tz='PC_TZ', obj='PC_OBJ', o0='PC_O0', o1='PC_O1', to='PC_TO', hda='PC_HDA', pa='PC_PA',
                  ta='PC_TA')
    if 'e0' in d and 'e1' not in d:
        d['e1'] = d['e0']
    if 'g0' in d and 'g1' not in d:
        d['g1'] = d['g0']
    if 'g0' not in d and kind == 'lerp':
        d['g0'] = d['g1'] = [0, 0, 0]
    if 'r0' in d and 'r1' not in d:
        d['r1'] = d['r0']
    if 'h0' in d and 'h1' not in d:
        d['h1'] = d['h0']
    if 'o0' in d and 'o1' not in d:
        d['o1'] = d['o0']
    if 'cobj' not in d:
        d['cobj'] = 'objNull'
    for k, v in d.items():
        parts.append('%s = %s' % (keymap[k], q(v)))
    if kind == 'follow':
        parts.append('PC_AP = getPos PC_OBJ; PC_HD = getDir PC_OBJ')
    if kind == 'track':
        parts.append('PC_TG = [(getPos PC_OBJ select 0) + (PC_TO select 0), (getPos PC_OBJ select 1) + (PC_TO select 1), PC_TO select 2]')
    # composition
    parts.append('PC_SUBJ = objNull; PC_SUBJP = []; PC_SUBJ2 = objNull; PC_SUBJP2 = []; PC_AIM = false; PC_FA = 0.3; PC_FB = 0.05')
    if 'subj' in comp:
        parts.append('PC_SUBJ = %s' % comp['subj'])
    if 'subjp' in comp:
        parts.append('PC_SUBJP = %s' % q(comp['subjp']))
    parts.append('PC_SUBJZ = %s' % q(float(comp.get('z', 1.5 if 'subj' in comp else 0.0))))
    if 'subj2' in comp:
        parts.append('PC_SUBJ2 = %s' % comp['subj2'])
    if 'subjp2' in comp:
        parts.append('PC_SUBJP2 = %s' % q(comp['subjp2']))
    parts.append('PC_SUBJZ2 = %s' % q(float(comp.get('z2', 1.5 if 'subj2' in comp else 0.0))))
    parts.append('PC_SX = %s; PC_SY = %s' % (q(float(comp.get('sx', 0.5))), q(float(comp.get('sy', 0.5)))))
    if comp.get('aim'):
        parts.append('PC_AIM = true')
    if 'fa' in comp:
        parts.append('PC_FA = %s' % q(float(comp['fa'])))
    if 'fb' in comp:
        parts.append('PC_FB = %s' % q(float(comp['fb'])))
    parts.append('[] call pcHold')
    t.comp = comp
    t.c('; '.join(parts))


def settle(t, frames=300, tries=1):
    """triSettleScene [300, 30, 2] and the scene state after it (lod and pin are checked by check_run.py). An
    UNSETTLED answer is a failed settle: it is run again, up to tries times; check_run.py reads the last."""
    # the world is paused while it settles (triPauseGame: no simulation step, the frames still draw), so a
    # moving subject or a moving camera holds still on the clip's first pose until the recorder starts
    s = 'prPz = triPauseGame; prSet = triSettleScene [%d, 30, 2]' % frames
    for _ in range(tries - 1):
        s += '; if (((toArray prSet) select 0) != 79) then { prSet = triSettleScene [%d, 30, 2] }' % frames
    t.c(s + '; prSS = triSceneStats; prPz = triUnpauseGame; logInfo format ["PROMO settle %1 %2", prSet, prSS]; logInfo format ["PROMO feat %1", triFeatureState]')


def record(t, name, seconds, pre=None, note='', gate=None, do_settle=True, sym=False):
    """settles (unless the caller already did, on this pose), then starts the camera move and the recorder
    in one statement and waits for the dump. gate: dict(x=1/3 | 2/3 | 0.5 | 'any', y=None | 1/3 | 2/3,
    tol=0.08, x2=..., y2=..., hz=False) for gate.py; sym marks a deliberately symmetric shot."""
    if ONLY is not None and name not in ONLY:
        t.a('triSimFrames %d' % (int(round(seconds * 30)) + HEAD + 40))
        return
    if do_settle:
        settle(t)
    n = int(round(seconds * 30)) + HEAD
    g = dict(gate or {})
    if not g:
        g = dict(x=t.comp.get('sx', 'any') if t.comp.get('aim') else 'any')
        if t.comp.get('aim') and 'sy' in t.comp:
            g['y'] = t.comp['sy']
    if sym:
        g.setdefault('x', 0.5)
    t.clips.append(dict(name=name, seconds=seconds, note=note, gate=g, sym=sym))
    t.c('%s"%s" call pcLogOn; %g call pcGo; prRec = triRecord ["%s.raw", %d, 0]; logInfo format ["PROMO rec %s %%1 at %%2 daytime %%3", prRec, time, daytime]'
        % ((pre + '; ') if pre else '', name, seconds, name, n, name))
    t.a('triRecordState')
    t.c('PC_LOG = false')
    t.log('done %s %%1' % name, 'triRecordState')
    t.a('PC_RUN = false')


def footer(t):
    t.a('PC_QUIT = true')
    t.c('prStep = triSetFixedStep 0')
    t.log('END')
    t.a('triEndTest')


def write(t, timeout=5400, mission=MISSION, mods='@LoBo'):
    with open(os.path.join(HERE, t.name + '.test.sqf'), 'w', newline='\n') as f:
        f.write('\n'.join(t.lines) + '\n')
    with open(os.path.join(HERE, t.name + '.test.toml'), 'w', newline='\n') as f:
        f.write(TOML.format(title=t.title, name=t.name, mission=mission, timeout=timeout, mods=mods, w=W, h=H))
    with open(os.path.join(HERE, t.name + '.shots.json'), 'w', newline='\n') as f:
        json.dump(dict(take=t.name, head=HEAD, w=W, h=H, clips=t.clips), f, indent=1)
    tot = sum(c['seconds'] for c in t.clips)
    print('%s: %d statements, %d clips, %.0f s of footage' % (t.name, len(t.lines), len(t.clips), tot))
