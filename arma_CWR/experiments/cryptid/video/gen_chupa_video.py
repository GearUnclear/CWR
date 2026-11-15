"""Writes the chupacabra video tests (experiment, not a regression lane).

    python gen_chupa_video.py probe        # location probe
    python gen_chupa_video.py shots        # the film's shots

Each test boots experiments/cryptid/video/ChupaVideo.Eden with @udchupa
(build-udchupa.ps1 -CustomMoves) on Everon at 13:00, light overcast. Nobody
is armed, nobody fires.

Stepped capture: the world is frozen between frames (setAccTime 0) and each
frame advances it by exactly one step inside one statement:
  call { triWaitFrames 60; setAccTime A; triSimFrames 1; setAccTime 0 }
The render-only pump lets more than 0.3 s of wall time pass, AppIdle caps
its step at 0.3 s, so the one tick simulates 0.3 x A seconds (A = 1/9 gives
1/30 s). Every frame logs its sim time (CVF lines) so the step is checked.
"""

import argparse
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))

TOML = """# {title} (experiment, kept out of tests/integration). Needs
# ../addon/build-udchupa.ps1 -CustomMoves (UDChupacabraAnim) and a data dir
# in the same tmp folder as @udchupa. Written by gen_chupa_video.py. Run with
# --render gl33, quiet, no gunfire.
tags = ["cryptid", "experiment", "visual"]
binary = "PoseidonGame"
mission = "experiments/cryptid/video/ChupaVideo.Eden"
timeout = {timeout}
assert_timeout = 90
extra_args = ["--nosound", "--no-strict", "--width", "{w}", "--height", "{h}", "--mods-dir", "..", "--mod", "@udchupa"]
"""

STEP = 'call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }'


class T:
    def __init__(self):
        self.lines = []
        self.frames = 0

    def a(self, s):
        self.lines.append(s)

    def text(self):
        return '\n'.join(self.lines) + '\n'

    def shot(self, label):
        self.a('triScreenshot "%s"' % label)
        self.frames += 1


def header(t, title, overcast=0.35):
    t.a('// ' + '=' * 76)
    t.a('//  %s' % title)
    t.a('//  Written by gen_chupa_video.py (edit that, not this file).')
    t.a('//  Quiet: no weapons, nobody fires, --nosound.')
    t.a('// ' + '=' * 76)
    t.a('triSimUntil { alive player }')
    t.a('triCaptureAtPresent true')
    t.a('player allowDammage false')
    t.a('player setCaptive true')
    t.a('removeAllWeapons player')
    t.a('player setPos [4800, 9000, 0]')
    t.a('setDate [1985, 5, 10, 13, 0]')
    t.a('0 setOvercast %g' % overcast)
    t.a('0 setFog 0')
    t.a('setViewDistance 1500')
    t.a('showCinemaBorder false')
    t.a('cvC = createCenter civilian')
    t.a('cvHud = "camera" camCreate [4800, 9000, 40]')
    t.a('cvHud cameraEffect ["internal", "back"]')
    t.a('cvHud camSetTarget [4800, 12000, 40]')
    t.a('cvHud camSetFov 0.7')
    t.a('cvHud camCommit 0')
    # Ground height: an invisible pad dropped on the point (no getTerrainHeightASL here).
    t.a('cvProbe = "HeliHEmpty" createVehicle [4800, 9000, 0]')
    t.a('cvGnd = { cvProbe setPos [_this select 0, _this select 1, 0]; (getPosASL cvProbe) select 2 }')


# --------------------------------------------------------------------------
# Probe: find the runway and apron, check classes, measure the stepped clock.
# --------------------------------------------------------------------------
CLASSES = ['Hangar', 'Hangar_Hall', 'Hangar_Office', 'Barracks', 'Office', 'Shed',
           'Su25', 'A10', 'Cessna', 'UH60', 'Mi17', 'Mi24', 'Truck5tRefuel', 'UralRefuel',
           'Truck5t', 'Ural', 'Jeep', 'HMMWV', 'Barrels', 'Barrel4', 'Civilian', 'Civilian2',
           'Civilian3', 'SoldierWCrew', 'SoldierWPilot', 'UDChupacabraAnim', 'Camp', 'FieldHospital']


def probe():
    t = T()
    header(t, 'Chupacabra video probe: runway, apron, classes, stepped clock')
    for c in CLASSES:
        t.a('logInfo format ["CVP class %s %%1", gmClassExists "%s"]' % (c, c))
    t.a('triSimFrames 60')
    # Top-down tiles, 650 m up, a slight tilt north so the view has an up axis.
    tiles = [(4600, 10600), (5100, 10600), (4600, 11100), (5100, 11100), (5500, 10300), (5500, 10850), (4300, 10850)]
    t.a('cvHud camSetFov 0.7')
    t.a('cvHud camCommit 0')
    for i, (x, z) in enumerate(tiles):
        t.a('triSetView [%d, 650, %d, 0, -1, 0.02]' % (x, z))
        t.a('triSimFrames 30')
        t.a('logInfo format ["CVP tile %d %d %d ground %%1", [%d, %d] call cvGnd]' % (i, x, z, x, z))
        t.shot('top_%d_%d_%d' % (i, x, z))
    # Ground-level look round at two spots.
    for (x, z) in [(4800, 11000), (5050, 10650)]:
        for hd in (0, 90, 180, 270):
            t.a('cvG = [%d, %d] call cvGnd' % (x, z))
            t.a('triSetView [%d, cvG + 2.0, %d, %g, -0.03, %g]' % (x, z, math.sin(math.radians(hd)), math.cos(math.radians(hd))))
            t.a('triSimFrames 20')
            t.shot('look_%d_%d_h%03d' % (x, z, hd))
    # Line-up of candidate victims and the creature, for looks and the clock.
    t.a('cvG0 = createGroup civilian')
    t.a('cvMen = []')
    for i, c in enumerate(['Civilian', 'Civilian2', 'Civilian3', 'SoldierWCrew', 'SoldierWPilot']):
        t.a('"%s" createUnit [[%g, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]' % (c, 4790 + i * 1.6))
    t.a('triSimFrames 5')
    t.a('{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x setDir 180; _x disableAI "MOVE" } forEach cvMen')
    t.a('cvG0 setBehaviour "CARELESS"')
    t.a('chupa = objNull')
    t.a('"UDChupacabraAnim" createUnit [[4800, 11000, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]')
    t.a('triSimFrames 5')
    t.a('chupa setCaptive true')
    t.a('chupa setPos [4801, 10998, 0]')
    t.a('chupa setDir 180')
    t.a('triSimFrames 60')
    t.a('{ _x switchMove "Civil" } forEach cvMen')
    t.a('triSimFrames 10')
    t.a('cvG = [4794, 10990] call cvGnd')
    t.a('triSetView [4794, cvG + 1.6, 10990, 0, -0.02, 1]')
    t.a('triSimFrames 6')
    t.shot('lineup')
    t.a('cvHud camSetTarget [4794, 30, 12000]')
    t.a('cvHud camSetFov 0.35')
    t.a('cvHud camCommit 0')
    t.a('triSimFrames 6')
    t.a('triSetView [4794, cvG + 1.6, 10985, 0, -0.02, 1]')
    t.a('triSimFrames 6')
    t.shot('lineup_long')
    # The stepped clock: creature gallops off, 40 stepped frames.
    t.a('chupa setUnitPos "DOWN"')
    t.a('cvG0 setSpeedMode "FULL"')
    t.a('chupa doMove [4801, 10900, 0]')
    t.a('triSimFrames 30')
    t.a('call { setAccTime 0 }')
    for k in range(40):
        t.a(STEP)
        t.a('logInfo format ["CVF step %%1 %%2 %%3 %%4", %d, time, getMove chupa, getPosASL chupa]' % k)
        if k % 10 == 0:
            t.a('triSetView [(getPosASL chupa select 0) + 6, (getPosASL chupa select 2) + 1, (getPosASL chupa select 1), -1, -0.1, 0]')
            t.shot('step_%02d' % k)
    t.a('setAccTime 1')
    t.a('triEndTest')
    return t


# --------------------------------------------------------------------------
# The set: the east edge of the Everon airfield. Runway centre x 4777
# (north-south), taxiway x 4847, forest edge x 5075. Props are placed, then
# setPos'd onto the intended point (createVehicle shifts things).
# [class, east, north, dir]
# --------------------------------------------------------------------------
PROPS = [
    ('Hangar', 4885, 11040, 270),
    ('Hangar_Hall', 4892, 11125, 270),
    ('Su25', 4848, 11082, 180),
    ('Mi17', 4868, 11170, 225),
    ('UralRefuel', 4866, 11060, 0),
    ('Ural', 4938, 11092, 0),
    ('Barrels', 4934, 11080, 0),
    ('Barrel4', 4932, 11078, 0),
    ('Barrel4', 4933.2, 11077.3, 0),
    ('Barrel1', 4935.5, 11077.6, 0),
]


def props(t):
    t.a('cvProps = []')
    for i, (c, x, z, d) in enumerate(PROPS):
        t.a('cvO = "%s" createVehicle [%g, %g, 0]' % (c, x, z))
        t.a('cvO setDir %g' % d)
        t.a('cvO setPos [%g, %g, 0]' % (x, z))
        t.a('cvProps = cvProps + [cvO]')
        t.a('logInfo format ["CVP prop %s %%1 at %%2 dir %%3", %d, getPosASL cvO, getDir cvO]' % (c, i))


def lens(t, fov, tx=4900, tz=14000):
    """camSetFov needs a camera target (README); a far point, then the fov."""
    t.a('cvHud camSetTarget [%g, %g, 30]' % (tx, tz))
    t.a('cvHud camSetFov %g' % fov)
    t.a('cvHud camCommit 0')


def view(t, eye, at, fov, label, ground_eye=True):
    """eye/at = (east, north, height above ground); one frame."""
    ex, ez, eh = eye
    ax, az, ah = at
    lens(t, fov)
    t.a('cvGe = [%g, %g] call cvGnd' % (ex, ez))
    t.a('cvGa = [%g, %g] call cvGnd' % (ax, az))
    t.a('triSetView [%g, cvGe + %g, %g, %g, (cvGa + %g) - (cvGe + %g), %g]' % (ex, eh, ez, ax - ex, ah, eh, az - ez))
    t.a('triSimFrames 8')
    t.shot(label)


def probe2():
    t = T()
    header(t, 'Chupacabra video probe 2: the set, portrait window, lenses')
    props(t)
    for (x, z) in [(4777, 11080), (4847, 11080), (4935, 11080), (4990, 11080), (5050, 11080), (5075, 11080), (4935, 11000), (4935, 11160)]:
        t.a('logInfo format ["CVP ground %d %d %%1", [%d, %d] call cvGnd]' % (x, z, x, z))
    t.a('cvG0 = createGroup civilian')
    t.a('victim = objNull')
    t.a('chupa = objNull')
    t.a('"Civilian" createUnit [[4936, 11082, 0], cvG0, "victim = this", 0.5, "PRIVATE"]')
    t.a('"UDChupacabraAnim" createUnit [[5055, 11080, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]')
    t.a('triSimFrames 5')
    t.a('{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x disableAI "MOVE" } forEach [victim, chupa]')
    t.a('cvG0 setBehaviour "CARELESS"')
    t.a('victim setPos [4936, 11082, 0]')
    t.a('victim setDir 90')
    t.a('chupa setPos [5055, 11080, 0]')
    t.a('chupa setDir 270')
    t.a('chupa setUnitPos "DOWN"')
    t.a('triSimFrames 90')
    t.a('chupa switchMove "ChupaFreeze"')
    t.a('victim switchMove "Civil"')
    t.a('triSimFrames 20')
    view(t, (4900, 11240, 120), (4900, 11080, 0), 0.7, 'p2_top')
    view(t, (4790, 11000, 14), (4930, 11100, 2), 0.7, 'p2_est_wide')
    view(t, (4790, 11000, 14), (4930, 11100, 2), 0.45, 'p2_est_45')
    view(t, (4925, 11079, 1.8), (5055, 11080, 1.0), 0.7, 'p2_ots_70')
    view(t, (4925, 11079, 1.8), (5055, 11080, 1.0), 0.2, 'p2_ots_20')
    view(t, (4937, 11081, 1.7), (5055, 11080, 0.8), 0.06, 'p2_tight_06')
    view(t, (4950, 11070, 1.2), (4936, 11082, 1.4), 0.5, 'p2_man_med')
    view(t, (4842, 11095, 0.6), (4930, 11070, 1.5), 0.5, 'p2_apron_low')
    view(t, (5052, 11092, 0.4), (4936, 11082, 1.0), 0.35, 'p2_from_tree_line')
    view(t, (4870, 11010, 1.0), (4848, 11082, 1.5), 0.5, 'p2_su25')
    view(t, (4950, 11115, 5), (4890, 11120, 4), 0.6, 'p2_hall')
    view(t, (4930, 11025, 4), (4885, 11040, 3), 0.6, 'p2_hangar')
    t.a('triEndTest')
    return t


def stepped(t, n, label, every=1, viewcode=None, logcode=None):
    """n stepped frames (1/30 s each), a screenshot every `every` frames."""
    t.a('call { setAccTime 0 }')
    for k in range(n):
        t.a(STEP)
        if logcode:
            t.a(logcode % k)
        if k % every == 0:
            if viewcode:
                t.a(viewcode)
            t.shot('%s_%03d' % (label, k))
    t.a('setAccTime 1')


def probe3():
    t = T()
    header(t, 'Chupacabra video probe 3: heights, the perimeter road, man moves')
    # Heights: one log line per row of a 15 x 20 m grid.
    t.a('cvRow = ""')
    for z in range(10980, 11201, 20):
        t.a('cvRow = ""; { cvRow = cvRow + format [" %%1", [_x, %d] call cvGnd] } forEach [%s]; logInfo format ["CVH %d%%1", cvRow]'
            % (z, ', '.join(str(x) for x in range(4780, 5141, 15)), z))
    for (c, x, z, d) in PROPS3:
        t.a('cvO = "%s" createVehicle [%g, %g, 0]' % (c, x, z))
        t.a('cvO setDir %g' % d)
        t.a('cvO setPos [%g, %g, 0]' % (x, z))
    t.a('cvG0 = createGroup civilian')
    t.a('victim = objNull')
    t.a('chupa = objNull')
    t.a('"Civilian" createUnit [[4984, 11080, 0], cvG0, "victim = this", 0.5, "PRIVATE"]')
    t.a('"UDChupacabraAnim" createUnit [[5082, 11080, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]')
    t.a('triSimFrames 5')
    t.a('{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x disableAI "MOVE" } forEach [victim, chupa]')
    t.a('cvG0 setBehaviour "CARELESS"')
    t.a('victim setPos [4984, 11080, 0]')
    t.a('victim setDir 90')
    t.a('chupa setPos [5082, 11080, 0]')
    t.a('chupa setDir 270')
    t.a('chupa setUnitPos "DOWN"')
    t.a('triSimFrames 90')
    t.a('chupa switchMove "ChupaFreeze"')
    t.a('victim switchMove "Civil"')
    t.a('triSimFrames 20')
    view(t, (4950, 11140, 40), (4990, 11080, 0), 0.7, 'p3_top')
    view(t, (4975, 11079, 1.7), (5082, 11080, 1.0), 0.5, 'p3_ots_50')
    view(t, (4983, 11079.5, 1.75), (5082, 11080, 1.0), 0.15, 'p3_pov_15')
    view(t, (4985, 11080, 1.7), (5082, 11080, 0.9), 0.05, 'p3_pov_05')
    view(t, (4995, 11070, 1.4), (4984, 11080, 1.3), 0.45, 'p3_man')
    view(t, (5030, 11060, 1.0), (4984, 11080, 1.3), 0.35, 'p3_slope_to_man')
    view(t, (4960, 11050, 6), (4890, 11110, 2), 0.6, 'p3_base_from_road')
    view(t, (4986, 11040, 18), (4900, 11110, 0), 0.6, 'p3_est')
    # The man's candidate moves, 10 stepped frames each, a frame every 5.
    for mv in ['CivilPutDown', 'FXStandDip', 'EffectStandTalk', 'CivilStillV1', 'FXStandBug', 'FXStandSur']:
        t.a('victim switchMove "%s"' % mv)
        t.a('logInfo format ["CVP move %s -> %%1", getMove victim]' % mv)
        stepped(t, 30, 'p3_mv_' + mv, every=10,
                viewcode='triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, 0, 4]')
        t.a('victim switchMove "Civil"')
        t.a('triSimFrames 5')
    t.a('victim switchMove "CivilToCivilLying"')
    stepped(t, 45, 'p3_mv_down', every=15, viewcode='triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]')
    t.a('victim playMove "CivilLyingToCivil"')
    stepped(t, 45, 'p3_mv_up', every=15, viewcode='triSetView [4987, ([4987, 11076] call cvGnd) + 1.4, 11076, -3, -0.3, 4]')
    t.a('triEndTest')
    return t


# The set, version 2: the man works on the perimeter road (x 4985, on the
# crest), the forest edge is about 95 m east and below him, the base is
# behind him to the west: hall, parked jet on the taxiway, runway.
PROPS3 = [
    ('Hangar_Hall', 4900, 11120, 90),
    ('Barracks', 4905, 11045, 90),
    ('Su25', 4848, 11082, 180),
    ('Mi17', 4870, 11175, 225),
    ('UralRefuel', 4872, 11060, 0),
    ('Ural', 4987, 11097, 180),
    ('Barrels', 4981, 11082, 0),
    ('Barrel4', 4980.5, 11079.5, 0),
    ('Barrel4', 4981.7, 11078.8, 0),
    ('Barrel1', 4982.6, 11080.0, 0),
]


def write(name, title, t, timeout, w=2560, h=1440):
    with open(os.path.join(HERE, name + '.test.sqf'), 'w', newline='\n') as f:
        f.write(t.text())
    with open(os.path.join(HERE, name + '.test.toml'), 'w', newline='\n') as f:
        f.write(TOML.format(title=title, timeout=timeout, w=w, h=h))
    print('%s: %d statements, %d frames' % (name, len(t.lines), t.frames))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('what', choices=['probe', 'probe2', 'probe3', 'shots'])
    args = ap.parse_args()
    if args.what == 'probe':
        write('chupa_video_probe', 'Chupacabra video probe', probe(), 400)
    if args.what == 'probe2':
        write('chupa_video_probe2', 'Chupacabra video probe 2', probe2(), 400, 1080, 1920)
    if args.what == 'probe3':
        write('chupa_video_probe3', 'Chupacabra video probe 3', probe3(), 500, 1080, 1920)


if __name__ == '__main__':
    main()
