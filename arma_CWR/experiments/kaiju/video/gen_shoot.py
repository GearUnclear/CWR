"""Writes the Dedushka video shoot tests.

    python gen_shoot.py probe      # stepped clock, lenses, map, first motion clips
    python gen_shoot.py <take>     # see TAKES at the bottom

Run a take with run-shoot.ps1 -Name kv_<take>; encode its frames with tools/encode_shots.py.
"""
import argparse

from kvlib import Cam, T, footer, header, step_stmt, stepped, still, write

K22 = '[0, 0, 22] call kvK'


def probe():
    t = T()
    header(t, 'Dedushka video probe: stepped clock, portrait lenses, the map, first motion')
    # ---- heights: one log line per row of a 50 m grid
    xs = list(range(4300, 5501, 50))
    for y in range(3500, 4501, 50):
        t.a('kvRow = ""; { kvRow = kvRow + format [" %%1", [_x, %d] call kvGnd] } forEach [%s]; logInfo format ["KVH %d%%1", kvRow]'
            % (y, ', '.join(str(x) for x in xs), y))
    # ---- the map from above (north is up in the portrait frame)
    t.a('0.5 call kvLens')
    for (x, y, h) in [(5000, 3950, 600), (4900, 3900, 1500)]:
        t.a('triSetView [%d, %d, %d, 0, -1, 0.02]' % (x, h, y))
        t.a('triSimFrames 12')
        t.shot('top_%d_%d_%d' % (x, y, h))
    # ---- he stands dormant: candidate views
    cands = [
        ('west_35', (4690, 3990, 1.7), 0.35), ('bay_03', (4880, 2160, 1.7), 0.03), ('bay_06', (4880, 2160, 1.7), 0.06),
        ('n400_12', (4790, 4404, 1.7), 0.12), ('s400_12', (4790, 3604, 1.7), 0.12), ('w600_08', (4190, 4004, 1.7), 0.08),
        ('ne_10', (5150, 4350, 1.7), 0.10), ('street_45', (4985, 3984, 1.7), 0.45), ('street_15', (4985, 3984, 1.7), 0.15),
        ('square_30', (5050, 3975, 1.7), 0.30), ('armour_12', (5128, 3958, 2.6), 0.12), ('east_08', (5300, 4004, 1.7), 0.08),
    ]
    for name, eye, fov in cands:
        still(t, 'dorm_' + name, Cam(name, eye, K22, fov))
    # mid-frame capture against the presented frame, same view
    t.a('triCaptureAtPresent false')
    still(t, 'dorm_west_35_midframe', Cam('w', (4690, 3990, 1.7), K22, 0.35))
    t.a('triCaptureAtPresent true')
    # ---- the stepped clock at four pump lengths (no pictures)
    t.a('call { setAccTime 0 }')
    for w in (60, 30, 15, 8):
        for k in range(12):
            t.a(step_stmt(w))
            t.a('logInfo format ["KVCLOCK %d %d %%1", time]' % (w, k))
    # ---- does a lens change take while the world is frozen
    t.a('triSetView ([[4690, 3990, 1.7] call kvP, %s] call kvLook)' % K22)
    t.a('0.12 call kvLens')
    t.shot('frozen_lens_12_no_tick')
    t.a('triSimFrames 1')
    t.shot('frozen_lens_12_after_tick')
    t.a('logInfo format ["KVCLOCK tick %1", time]')
    t.a('0.35 call kvLens')
    t.a('triSimFrames 1')
    t.shot('frozen_lens_35_after_tick')
    t.a('setAccTime 1')
    # ---- wake: the order given as the world freezes; 5 s from two cameras
    stepped(t, 'wake', 150, [Cam('street', (4985, 3984, 1.7), K22, 0.35), Cam('west', (4690, 3990, 1.7), K22, 0.35)],
            pre='["wake"] exec "kjd_beat.sqs"')
    t.a('triSimUntil { ("wake" in KJD_done) || (time > 200) }')
    t.a('logInfo format ["KV wake done at %1, state %2", time, KJ_state]')
    # ---- the walk in: 6 s from three cameras once he is under way
    t.a('["through"] exec "kjd_beat.sqs"')
    t.a('triSimUntil { (((getPos kaiju) select 0) > 4860) || (time > 400) }')
    stepped(t, 'walk', 180, [
        Cam('street', (4985, 3984, 1.7), K22, 0.35, alpha=0.1),
        Cam('north', (4930, 4130, 1.7), K22, 0.35, alpha=0.1),
        Cam('fixed', (4930, 4130, 1.7), '[4900, 4004, 22] call kvP', 0.35),
    ])
    footer(t)
    write('kv_probe', 'Dedushka video probe', t, 2400, params='KJD_manual = true;\n')


KF = 'call kvF'
# The street lamps stutter when he is within nine of his heights (the roar feature's presence, tuning knob
# KJR_FLICKER_R). On film each stutter steps the brightness of the whole ground by up to 40 of 255 in one
# frame (probe 1), which reads as a fault of the picture. The dusk takes switch the stutter off through the
# knob (the lamps still go dark for good as he comes near). Nothing of the script set is edited.
NOFLICK = 'KJR_FLICKER_R = 0;\n'
MANUAL = 'KJD_manual = true;\n' + NOFLICK
SHORT = MANUAL + 'KJD_start = [4900, 4004, 0];\n'
# The furnace light breathes 18 percent up and down every 6 s (KJF_BREATH). Its reach scales with its
# brightness and the ground is lit a patch at a time, so each time the brightness crosses about 1.1 a whole
# field steps 8 to 11 of 255 brighter or darker in one frame (take main1, the north camera). Part 2 turns the
# breath of the light down to 5 percent through the knob (the glow on the body breathes less with it).
CALM = SHORT + 'KJF_BREATH = 0.05;\n'


def wait_until(t, cond, limit):
    """Real time until cond, or limit seconds from now (always under the toml's assert_timeout)."""
    t.a('kvTw = time')
    t.a('triSimUntil { (%s) || (time > kvTw + %d) }' % (cond, limit))


def beat(t, name):
    t.a('["%s"] exec "kjd_beat.sqs"' % name)


def night():
    """By night (23:30): the furnace from 5 km and 2.3 km off shore, the wake in the dark, the walk in the dark."""
    t = T()
    header(t, 'Dedushka video, the night take: the far furnace, the wake and the walk in the dark')
    boat5 = '[4790, 4, -996]'
    boat2 = '[4790, 4, 1704]'
    stepped(t, 'nfar', 150, [
        Cam('boat5', None, KF, 0.012, eye_expr=boat5),
        Cam('boat5w', None, KF, 0.12, eye_expr=boat5),
        Cam('boat2', None, KF, 0.014, eye_expr=boat2),
    ])
    stepped(t, 'nwake', 520, [Cam('street', (4985, 3984, 1.7), K22, 0.15)], pre='["wake"] exec "kjd_beat.sqs"')
    wait_until(t, '"wake" in KJD_done', 60)
    stepped(t, 'nidle', 120, [
        Cam('boat5', None, KF, 0.012, eye_expr=boat5),
        Cam('boat5w', None, KF, 0.12, eye_expr=boat5),
        Cam('boat2', None, KF, 0.014, eye_expr=boat2),
    ])
    beat(t, 'through')
    wait_until(t, '((getPos kaiju) select 0) > 4826', 120)
    stepped(t, 'nwalk', 330, [
        Cam('street', (4985, 3984, 1.7), K22, 0.2, alpha=0.1),
        Cam('boat2', None, KF, 0.014, alpha=0.2, eye_expr=boat2),
    ])
    footer(t)
    write('kv_night', 'Dedushka video, the night take', t, 3600, params='KJD_manual = true;\nKJD_hour = 23.5;\n')


def main1():
    """Dusk: dormant on the horizon, the wake, the walk in through the houses, the roar."""
    t = T()
    header(t, 'Dedushka video, dusk part 1: dormant, wake, the walk in, the roar')
    stepped(t, 'dorm', 90, [
        Cam('street', (4985, 3984, 1.7), K22, 0.15),
        Cam('n400', (4790, 4404, 1.7), K22, 0.12),
        Cam('bay', (4880, 2160, 1.7), K22, 0.03),
        Cam('east', (5300, 4004, 1.7), K22, 0.08),
    ])
    # the rise takes 17 s and nearly all of what shows is in its second half (the night take)
    beat(t, 'wake')
    t.a('kvTw = time')
    t.a('triSimUntil { time > kvTw + 7.5 }')
    stepped(t, 'wake', 310, [
        Cam('street', (4985, 3984, 1.7), K22, 0.15),
        Cam('n400', (4790, 4404, 1.7), K22, 0.12),
    ])
    wait_until(t, '"wake" in KJD_done', 60)
    beat(t, 'through')
    wait_until(t, '((getPos kaiju) select 0) > 4812', 120)
    stepped(t, 'walkA', 540, [
        Cam('low', (4876, 3989, 1.2), '[0, 0, 27] call kvK', 0.5, alpha=0.15),
        Cam('north', (4930, 4130, 1.7), '[0, 0, 20] call kvK', 0.2, alpha=0.08),
        Cam('street', (4985, 3984, 1.7), '[4850, 4004, 15] call kvP', 0.12),
    ])
    wait_until(t, '((getPos kaiju) select 0) > 4905', 120)
    stepped(t, 'walkB', 420, [
        Cam('street', (4985, 3984, 1.7), '[0, 0, 25] call kvK', 0.35, alpha=0.12),
        Cam('square', (5078, 3977, 1.7), K22, 0.12, alpha=0.08),
    ])
    wait_until(t, '"through" in KJD_done', 120)
    stepped(t, 'roar', 400, [
        Cam('square', (5050, 3975, 1.7), '[0, 0, 26] call kvK', 0.45, alpha=0.2),
        Cam('bmp', (5106, 3960, 2.4), '[0, 0, 22] call kvK', 0.2, alpha=0.2),
        Cam('civ', (5082, 4042, 1.7), '[0, 0, 26] call kvK', 0.3, alpha=0.2),
        Cam('east', (5300, 4004, 1.7), '[0, 0, 22] call kvK', 0.08, alpha=0.2),
    ], pre='["roar"] exec "kjd_beat.sqs"')
    footer(t)
    write('kv_main1', 'Dedushka video, dusk part 1', t, 5400, params=MANUAL)


def church():
    """Dusk: he walks through the church of Morton."""
    t = T()
    header(t, 'Dedushka video, the church: he walks through the church of Morton')
    beat(t, 'wake')
    wait_until(t, '"wake" in KJD_done', 90)
    t.a('KJ_brainOrder = ["drawn", [5104, 3901, 0]]')
    wait_until(t, '(sqrt ((((getPos kaiju) select 0) - 5057) ^ 2 + (((getPos kaiju) select 1) - 3925) ^ 2)) < 58', 200)
    t.a('logInfo format ["KV church: he is at %1, state %2", getPos kaiju, KJ_state]')
    stepped(t, 'church', 780, [
        Cam('se', (5097, 3888, 1.7), '[5057, 3925, 20] call kvP', 0.32),
        Cam('south', (5062, 3862, 1.7), '[5057, 3925, 20] call kvP', 0.3),
        Cam('west', (4990, 3900, 1.7), '[5057, 3925, 18] call kvP', 0.3),
    ])
    footer(t)
    write('kv_church', 'Dedushka video, the church', t, 4000, params=SHORT)


def fire_beam(t, chest):
    """The armour fires (staged by the mission: no weapon is fired), the beam, spent."""
    t.a('if (true) then { KJD_spentHold = 6; KJB_SPENT = 30; KJ_beamOverride = { KJ_beamT_spent = 30 } }')
    for b in ('wake', 'through', 'roar'):
        beat(t, b)
        wait_until(t, '"%s" in KJD_done' % b, 150)
    # (a camera behind the armour cannot see him for the houses, and 5106, 3960 is inside a wall:
    # probe 2 and the first roar take. The armour is filmed for itself; he from the square and the north.)
    stepped(t, 'fire', 240, [
        Cam('square', (5050, 3975, 1.7), '[0, 0, 25] call kvK', 0.42, alpha=0.2),
        Cam('armour', (5142, 3952, 2.6), '[5112, 3972, 5] call kvP', 0.3),
        Cam('north', (5075, 4125, 2), '[0, 0, 22] call kvK', 0.2, alpha=0.2),
    ], pre='["fire"] exec "kjd_beat.sqs"')
    wait_until(t, '"fire" in KJD_done', 120)
    beat(t, 'beam')
    wait_until(t, 'KJ_beamPhase == "charge"', 120)
    cams = [
        Cam('square', (5050, 3975, 1.7), '[0, 0, 26] call kvK', 0.45, alpha=0.2),
        Cam('north', (5075, 4125, 2), '[20, 0, 22] call kvK', 0.3, alpha=0.2),
        Cam('armour', (5142, 3950, 2.6), '[5108, 3974, 6] call kvP', 0.35),
    ]
    if chest:
        cams.append(Cam('chest', (5075, 4125, 2), KF, 0.07, alpha=0.3))
    stepped(t, 'beam', 780, cams)
    wait_until(t, 'KJ_state == "spent"', 120)
    stepped(t, 'spent', 120, [
        Cam('street', (4985, 3984, 1.7), '[0, 0, 26] call kvK', 0.5),
        Cam('north', (5075, 4125, 2), '[20, 0, 22] call kvK', 0.3),
    ])


def main2a():
    """Dusk: the armour fires, the beam, spent (kept as a take of its own for a re-shoot of only that)."""
    t = T()
    header(t, 'Dedushka video, dusk part 2a: the armour fires, the beam, spent')
    fire_beam(t, True)
    footer(t)
    write('kv_main2a', 'Dedushka video, dusk part 2a', t, 5400, params=SHORT)


def main2(lead_only=False):
    """Dusk: the armour fires, the beam, spent, then THE FIGHT (weapons are fired), the last stand, the
    death, the body, and the body the next morning. One run, because every run waits its turn for the
    machine's one game slot (the perf squad's timed runs hold it for up to three quarters of an hour).
    lead_only: the beats before the fight are played in real time and not filmed."""
    t = T()
    header(t, 'Dedushka video, dusk part 2: fire, the beam, the fight, the last stand, the death, the body', weapons=True)
    if lead_only:
        t.a('if (true) then { KJD_spentHold = 6; KJB_SPENT = 30; KJ_beamOverride = { KJ_beamT_spent = 30 } }')
        for b in ('wake', 'through', 'roar', 'fire', 'beam'):
            beat(t, b)
            wait_until(t, '"%s" in KJD_done' % b, 240)
    else:
        fire_beam(t, False)
    # ---- the fight
    beat(t, 'kill')
    wait_until(t, '!(isNil "KJD_fightOn")', 60)
    t.a('kvT1 = time')
    wait_until(t, 'KJD_fightShots > 0', 90)
    # the opening fire from behind the platoon on the west road (open fields between it and him; the
    # armour in the town stands among houses that hide him: probe 2)
    t.a('if (isNil "KJD_force2") then { KJD_force2 = [] }')
    t.a('kvV = [90, 700, KJD_force2] call kvPick')
    t.a('if (isNull kvV) then { kvV = [90, 700, KJD_force] call kvPick }')
    t.a('logInfo format ["KV fight: first shot at %1, picked %2 at %3 m, force alive %4, second platoon %5", time - kvT1, kvV, kvV distance kaiju, { alive _x } count KJD_force, KJD_force2]')
    tank_eye = 'if (isNull kvV) then { [60, 160, 2.2] call kvRel } else { [kvV, 55, 3.4, 1.2] call kvBehind }'
    stepped(t, 'guns', 240, [
        Cam('tank', None, '[0, 0, 20] call kvK', '[kvE%d, 62] call kvFovFor', alpha=0.2, eye_expr=tank_eye),
        Cam('north', (5075, 4125, 2), '[0, 0, 22] call kvK', 0.25, alpha=0.2),
        Cam('west', (4690, 3990, 1.7), '[0, 0, 22] call kvK', 0.1, alpha=0.2),
    ])
    # at bay: he turns on what is killing him
    wait_until(t, 'KJ_brainBay || KJ_dead', 700)
    t.a('kvV = [90, 600, KJD_force] call kvPick')
    t.a('logInfo format ["KV bay: %1 s into the fight, bay %2, dead %3, health %4, pos %5, depth %6, picked %7 at %8 m", time - kvT1, KJ_brainBay, KJ_dead, KJ_woundHealth, getPos kaiju, call KJB_fnDepth, kvV, kvV distance kaiju]')
    shelf_eye = '[5095, 3835, 6] call kvP'
    stepped(t, 'bay', 330, [
        Cam('tank', None, '[0, 0, 20] call kvK', '[kvE%d, 62] call kvFovFor', alpha=0.2, eye_expr=tank_eye),
        Cam('front', None, '[0, 0, 22] call kvK', 0.24, alpha=0.2, eye_expr='[25, 150, 1.8] call kvRel'),
        Cam('shelf', None, '[0, 0, 20] call kvK', '[kvE%d, 66] call kvFovFor', alpha=0.2, eye_expr=shelf_eye),
    ])
    wait_until(t, 'KJ_dead', 600)
    t.a('kvV = [90, 600, KJD_force] call kvPick')
    t.a('logInfo format ["KV dead: %1 s into the fight, by the director %2, health %3, pos %4, dir %5, depth %6, picked %7 at %8 m", time - kvT1, KJD_fightByDirector, KJ_woundHealth, getPos kaiju, getDir kaiju, call KJB_fnDepth, kvV, kvV distance kaiju]')
    stepped(t, 'death', 690, [
        Cam('front', None, '[0, 0, 15] call kvK', 0.24, alpha=0.06, eye_expr='[25, 150, 1.8] call kvRel', aim0='[0, 0, 22] call kvK'),
        Cam('tank', None, '[0, 0, 14] call kvK', '[kvE%d, 70] call kvFovFor', alpha=0.06, eye_expr=tank_eye, aim0='[0, 0, 20] call kvK'),
        Cam('shelf', None, '[0, 0, 14] call kvK', '[kvE%d, 74] call kvFovFor', alpha=0.06, eye_expr=shelf_eye, aim0='[0, 0, 20] call kvK'),
    ])
    wait_until(t, 'KJ_deathPhase == "body"', 60)
    t.a('kvTw = time')
    t.a('triSimUntil { time > kvTw + 22 }')
    t.a('logInfo format ["KV body: death log %1, pos %2, dir %3, depth %4", KJ_deathLog, getPos kaiju, getDir kaiju, call KJB_fnDepth]')
    # the middle of the body (he falls forward) and views round it
    t.a('kvMx = ((getPos kaiju) select 0) + 0.5 * KJ_heightM * (sin (getDir kaiju))')
    t.a('kvMy = ((getPos kaiju) select 1) + 0.5 * KJ_heightM * (cos (getDir kaiju))')
    t.a('kvMz = ([kvMx, kvMy] call kvGnd) + 5')
    t.a('kvBody = { private ["_b", "_x", "_y"]; _b = (getDir kaiju) + (_this select 0); _x = kvMx + (_this select 1) * (sin _b); _y = kvMy + (_this select 1) * (cos _b); [_x, ([_x, _y] call kvGnd) + (_this select 2), _y] }')
    mid = '[kvMx, kvMz, kvMy]'
    stepped(t, 'bodydusk', 45, [
        Cam('right', None, mid, 0.3, eye_expr='[90, 95, 1.8] call kvBody'),
        Cam('left', None, mid, 0.3, eye_expr='[270, 95, 1.8] call kvBody'),
    ])
    # ---- the next morning: men come to look
    t.a('skipTime (29.7 - daytime)')
    t.a('triSimFrames 40')
    t.a('kvMen = []')
    t.a('kvGrp = createGroup east')
    for (b, d) in [(84, 44), (88, 50), (93, 41), (97, 55), (264, 46), (270, 52), (277, 43)]:
        t.a('kvQ = [%d, %d, 0] call kvBody' % (b, d))
        t.a('"SoldierEB" createUnit [[kvQ select 0, kvQ select 2, 0], kvGrp, "kvMen = kvMen + [this]", 0.5, "PRIVATE"]')
    t.a('triSimFrames 10')
    t.a('{ _x setDir ((kvMx - ((getPos _x) select 0)) atan2 (kvMy - ((getPos _x) select 1))); _x disableAI "MOVE"; _x setBehaviour "SAFE"; _x setCaptive true } forEach kvMen')
    t.a('triSimFrames 90')
    t.a('logInfo format ["KV morning: hour %1, men %2, body middle [%3, %4]", daytime, count kvMen, kvMx, kvMy]')
    stepped(t, 'morning', 90, [
        Cam('right', None, mid, 0.3, eye_expr='[90, 95, 1.7] call kvBody'),
        Cam('left', None, mid, 0.3, eye_expr='[270, 95, 1.7] call kvBody'),
        Cam('high', None, mid, 0.45, eye_expr='[200, 70, 30] call kvBody'),
        Cam('head', None, mid, 0.35, eye_expr='[20, 80, 1.7] call kvBody'),
    ])
    # an hour and a bit later (07:00), in case 05:42 is too dark
    t.a('skipTime (7 - daytime)')
    t.a('triSimFrames 40')
    stepped(t, 'morning7', 45, [
        Cam('right', None, mid, 0.3, eye_expr='[90, 95, 1.7] call kvBody'),
        Cam('left', None, mid, 0.3, eye_expr='[270, 95, 1.7] call kvBody'),
        Cam('high', None, mid, 0.45, eye_expr='[200, 70, 30] call kvBody'),
    ])
    footer(t)
    write('kv_main2b' if lead_only else 'kv_main2', 'Dedushka video, dusk part 2 (WEAPONS FIRE)', t, 9000, params=CALM)


def hound():
    """By night: the other creature, in the bay, seen from the shelf above the shore."""
    t = T()
    header(t, 'Dedushka video, the last shot: a second light on the water (the other creature, by night)')
    t.a('logInfo format ["KV hound: at %1 asl %2, depth %3, state %4", getPos kaiju, getPosASL kaiju, call KJB_fnDepth, KJ_state]')
    stepped(t, 'hdorm', 90, [
        Cam('shelf', (5012, 3856, 1.7), KF, 0.1),
        Cam('wide', (5012, 3856, 1.7), KF, 0.3),
    ])
    stepped(t, 'hwake', 520, [Cam('shelf', (5012, 3856, 1.7), KF, 0.1, alpha=0.2)], pre='["wake"] exec "kjd_beat.sqs"')
    wait_until(t, '"wake" in KJD_done', 60)
    t.a('KJ_brainOrder = ["drawn", [5010, 3790, 0]]')
    t.a('triSimFrames 60')
    stepped(t, 'hwalk', 450, [
        Cam('shelf', (5012, 3856, 1.7), KF, 0.1, alpha=0.15),
        Cam('wide', (5012, 3856, 1.7), KF, 0.3, alpha=0.15),
    ])
    footer(t)
    write('kv_hound', 'Dedushka video, the other creature by night', t, 3600,
          params='KJD_manual = true;\nKJD_hour = 23.5;\nKJD_class = "UDDedushkaHound";\nKJD_start = [5010, 3640, 0];\n')


def probe2():
    """Stills only: every fixed camera of the takes, with him set down where he will be."""
    t = T()
    header(t, 'Dedushka video probe 2: the framing of every planned camera (stills)')

    def P(x, y, h):
        return '[%g, %g, %g] call kvP' % (x, y, h)
    views = [
        ('a_low', (4876, 3989, 1.2), P(4840, 4004, 27), 0.5), ('a_north', (4930, 4130, 1.7), P(4840, 4004, 20), 0.2),
        ('a_street', (4985, 3984, 1.7), P(4850, 4004, 15), 0.12),
    ]
    t.a('kaiju setPos [4840, 4004, 0]')
    t.a('triSimFrames 30')
    for n, e, a, f in views:
        still(t, n, Cam(n, e, a, f))
    t.a('kaiju setPos [4925, 4004, 0]')
    t.a('triSimFrames 30')
    for n, e, a, f in [('b_street', (4985, 3984, 1.7), P(4925, 4004, 25), 0.35), ('b_square', (5078, 3977, 1.7), P(4925, 4004, 22), 0.12)]:
        still(t, n, Cam(n, e, a, f))
    t.a('kaiju setPos [5003, 4004, 0]')
    t.a('triSimFrames 30')
    for n, e, a, f in [
        ('r_square', (5050, 3975, 1.7), P(5003, 4004, 26), 0.45), ('r_armour', (5128, 3958, 2.6), P(5003, 4004, 24), 0.2),
        ('r_civ', (5082, 4042, 1.7), P(5003, 4004, 26), 0.3), ('f_armour', (5142, 3952, 2.6), P(5003, 4004, 22), 0.16),
        ('f_north', (5075, 4125, 2), P(5003, 4004, 22), 0.2), ('m_north', (5075, 4125, 2), P(5023, 4004, 22), 0.3),
        ('m_chest', (5128, 3958, 2.6), P(5003, 4004, 28), 0.06), ('m_armour', (5142, 3950, 2.6), P(5003, 4004, 20), 0.35),
        ('s_street', (4985, 3984, 1.7), P(5003, 4004, 26), 0.5), ('r_east', (5300, 4004, 1.7), P(5003, 4004, 22), 0.08),
        ('r_south', (5010, 3895, 1.7), P(5003, 4004, 24), 0.4),
    ]:
        still(t, n, Cam(n, e, a, f))
    t.a('kaiju setPos [5046, 3931, 0]')
    t.a('kaiju setDir 117')
    t.a('triSimFrames 30')
    for n, e, a, f in [
        ('c_se', (5097, 3888, 1.7), P(5057, 3925, 20), 0.32), ('c_north', (5052, 3998, 1.7), P(5057, 3925, 17), 0.36),
        ('c_east', (5300, 3950, 1.7), P(5057, 3925, 20), 0.05), ('c_south', (5062, 3862, 1.7), P(5057, 3925, 20), 0.3),
        ('c_west', (4990, 3900, 1.7), P(5057, 3925, 18), 0.3),
    ]:
        still(t, n, Cam(n, e, a, f))
    t.a('kaiju setPos [5000, 3700, 0]')
    t.a('kaiju setDir 0')
    t.a('triSimFrames 60')
    t.a('logInfo format ["KV bay: he is at %1, asl %2, depth %3", getPos kaiju, getPosASL kaiju, call KJB_fnDepth]')
    for n, e, a, f in [('h_shelf', (5012, 3856, 1.7), KF, 0.1), ('h_wide', (5012, 3856, 1.7), KF, 0.3), ('h_town', (5050, 3975, 1.7), KF, 0.12),
                       ('h_shore', (5095, 3835, 1.7), KF, 0.2)]:
        still(t, n, Cam(n, e, a, f))
    # the top view again by a brighter hour, for the map
    t.a('skipTime -6')
    t.a('triSimFrames 30')
    t.a('0.5 call kvLens')
    t.a('triSetView [5000, 600, 3950, 0, -1, 0.02]')
    t.a('triSimFrames 12')
    t.shot('top_day_5000_3950_600')
    footer(t)
    write('kv_probe2', 'Dedushka video probe 2 (stills)', t, 1200, params=MANUAL)


TAKES = {'probe': probe, 'probe2': probe2, 'night': night, 'main1': main1, 'church': church, 'main2a': main2a, 'main2': main2, 'main2b': lambda: main2(True), 'hound': hound}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('what', choices=sorted(TAKES))
    args = ap.parse_args()
    TAKES[args.what]()


if __name__ == '__main__':
    main()
