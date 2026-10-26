"""Adds the probe 6 views (beam contrast, misty pine wood, headlights, a pan) to gen_test.py."""
import sys

p = sys.argv[1]
s = open(p, encoding='utf-8').read()


def rep(old, new):
    global s
    assert s.count(old) == 1, old
    s = s.replace(old, new)


rep("""    for sp in v.get('spawn', []):
        out.append('call { %s call capSpawn }\\n' % sp)
""", """    for sp in v.get('spawn', []):
        out.append('call { %s call capSpawn }\\n' % sp)
    for raw in v.get('raw', []):
        out.append(raw + '\\n')
""")
rep("""    for label, knobs in v['variants']:
        out.append(shot(v['id'], label, 'remastered', knobs, fire=fire))
    out.append('[] call capClear\\ncall { capFire = objNull }\\n')
""", """    for label, knobs in v['variants']:
        out.append(shot(v['id'], label, 'remastered', knobs, fire=fire))
    for label, knobs, steps, every, deg in v.get('pans', []):
        out.append(pan(v['id'], label, knobs, steps, every, deg))
    for raw in v.get('cleanup', []):
        out.append(raw + '\\n')
    out.append('[] call capClear\\ncall { capFire = objNull }\\n')
""")
rep("""def view_block(v):""", """def pan(view, label, knobs, steps, every, deg):
    # A camera pan: settle on the view, then turn deg per frame for steps
    # frames, a shot every `every` frames (mid-motion), then restore the heading.
    out = [shot(view, '%s_p00' % label, 'remastered', knobs)]
    for k in range(1, steps + 1):
        out.append('call { capH = capH + %s; [] call capPinView }\\n' % deg)
        out.append('triSimFrames 1\\n')
        if k % every == 0:
            out.append('triScreenshot "%s__%s_p%02d"\\n' % (view, label, k))
            out.append('"%s__%s_p%02d" call capRecord\\n' % (view, label, k))
    out.append('call { capH = capH - %s; [] call capPin }\\n' % round(deg * steps, 4))
    return ''.join(out)


def view_block(v):""")
rep("""VN = VA + ['volMist=0.5']
SPECS = {""", """VN = VA + ['volMist=0.5']

# ---- probe 6: beam contrast (volGain), misty pine wood, headlights, a pan
PINEWOOD_MIST = dict(PINEWOOD, id='pinewood_mist', fog=0.35)
G25 = AH + ['volGain=2.5']
G4 = AH + ['volGain=4']
A3G = A3H + ['volGain=3']
GLO = VA + ['volHighpass=0.3', 'volGain=2']
# Headlights: a Jeep with a SAFE AI driver on a dark forest road (Abel 7259,
# 8343, road heading 10 deg; no street lamp within 400 m). The driver is not a
# capProps entry, so capPin's setPos never pulls him out of the seat.
DRIVER = ['call {',
          '    capDrv = objNull;',
          '    "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"];',
          '    if (not (isNull capDrv)) then {',
          '        capDrv setCaptive true;',
          '        capDrv allowDamage false;',
          '        capDrv disableAI "MOVE";',
          '        capDrv moveInDriver capJeep;',
          '        capDrv setBehaviour "SAFE";',
          '        format ["view=%1;event=driver;in=%2", capView, vehicle capDrv] call capLog;',
          '    };',
          '}']
UNDRIVER = ['call { if (not (isNull capDrv)) then { deleteVehicle capDrv } }']
# Side view: camera 2 m behind the bumper line and 16 m to the right of the
# road, looking at heading 305; the beams run up the road into the right half.
HL_SIDE = dict(id='hl_side', x=7274.4, z=8338.3, eye=1.6, h=305, p=-2, date='[1985, 2, 27, 22, 0]', vd=800, fog=0.2,
               pre=300, raw=['call { capJeep = [["Jeep"], 15.35, -4.95, 65, "veh"] call capSpawn }'] + DRIVER,
               cleanup=UNDRIVER)
# Front view: 14 m up the road and 5 m to its right, looking back down it.
HL_FRONT = dict(id='hl_front', x=7266.4, z=8355.9, eye=1.6, h=200, p=-1, date='[1985, 2, 27, 22, 0]', vd=800, fog=0.2,
                pre=300, raw=['call { capJeep = [["Jeep"], 14.66, 2.49, 170, "veh"] call capSpawn }'] + DRIVER,
                cleanup=UNDRIVER)
VNS = VN + ['volSpot=1']
VNS2 = VN + ['volSpot=1', 'volLocal=2']
VNSM = VA + ['volSpot=1', 'volLocal=2']
SPECS = {
    'probe6': [
        dict(HL_SIDE, variants=[('vn', VN), ('vns', VNS), ('vns2', VNS2), ('vnsm', VNSM), ('vns2_dbg', VNS2 + ['volDebug=1'])]),
        dict(HL_FRONT, variants=[('vns', VNS), ('vns2', VNS2), ('vnsm', VNSM)]),
        dict(PINEWOOD, variants=[('vah', AH), ('vg25', G25), ('vg4', G4), ('va3g', A3G), ('vg25_dbg', G25 + ['volDebug=1'])]),
        dict(PINEWOOD_MIST, variants=[('va', VA), ('vah', AH), ('vg25', G25), ('va3g', A3G)]),
        dict(PINE_EVE, variants=[('vah', AH), ('vg25', G25), ('va3g', A3G)]),
        dict(LOWSUN, variants=[('vah', AH), ('vg25', G25)]),
        dict(RIDGE, variants=[('va', VA), ('vglo', GLO), ('vg25', G25)]),
        dict(PINEWOOD, id='pinewood_pan', classic=False, current=False, variants=[],
             pans=[('sp', G25, 24, 8, 0.3), ('tt', G25 + ['volTemporal=1', 'volBlur=0.5'], 24, 8, 0.3)]),
    ],""")
open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('patched', p)
