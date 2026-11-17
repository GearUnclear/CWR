"""Writes the chupacabra animation tests (experiment, not a regression lane).

    python gen_chupa_anim_tests.py [--wall SECONDS]

Each test boots experiments/cryptid/ChupaModel.Eden with the @udchupa mod
built by ../addon/build-udchupa.ps1 -CustomMoves and spawns UDChupacabraAnim
on the Everon airfield edge in daylight. Nobody is armed, nobody fires.

Frame bursts: triScreenshot needs a literal label and one statement per
frame, so the bursts are spelled out here. The sim runs on wall time, so a
burst slows the game with setAccTime and logs the sim time of every frame
(CHF lines); the encoder turns those times into a real-speed mp4.
--wall is the measured wall time per burst frame (log, view, screenshot).
"""

import argparse
import os

HERE = os.path.dirname(os.path.abspath(__file__))

TOML = """# {title} (experiment, kept out of tests/integration). Needs
# ../addon/build-udchupa.ps1 -CustomMoves (UDChupacabraAnim, CfgMovesChupa)
# and a data dir in the same tmp folder as @udchupa, like
# tests/integration/scripting/cryptid_model_spawn. Written by
# gen_chupa_anim_tests.py. Run with --render gl33, quiet, no gunfire.
tags = ["cryptid", "experiment", "visual"]
binary = "PoseidonGame"
mission = "experiments/cryptid/ChupaModel.Eden"
timeout = {timeout}
assert_timeout = 90
extra_args = ["--nosound", "--no-strict", "--width", "1280", "--height", "720", "--mods-dir", "..", "--mod", "@udchupa"]
"""

SPOT = (4800, 11000)


class T:
    def __init__(self, wall):
        self.lines = []
        self.wall = wall
        self.frames = 0

    def a(self, s):
        self.lines.append(s)

    def wait(self, secs):
        self.a('if (true) then { t0 = time }')
        self.a('triSimUntil { time > t0 + %g }' % secs)

    def log(self, tag, extra=''):
        self.a('logInfo format ["CHUPA %s %%1 %%2 %%3 %%4%s", time, getMove chupa, getPosASL chupa, getDir chupa]'
               % (tag, extra))

    def burst(self, label, n, cover, view=None):
        """n frames spread over `cover` sim seconds; view = SQF view function
        name to re-aim each frame (None: keep the current view)."""
        wall = self.wall + (0.045 if view else 0)
        acc = cover / (n * wall)
        if acc > 1:
            acc = 1
        self.a('setAccTime %.4f' % acc)
        self.a('logInfo format ["CHB %%1 %%2 %%3 %d %.4f", "%s", time, getMove chupa]' % (n, acc, label))
        for _ in range(n):
            self.a('logInfo format ["CHF %%1 %%2 %%3 %%4 %%5", "%s", time, getMove chupa, getPosASL chupa, getDir chupa]'
                   % label)
            if view:
                self.a('triSetView ([] call %s)' % view)
            self.a('triScreenshot "%s"' % label)
        self.frames += n
        self.a('setAccTime 1')
        self.a('logInfo format ["CHE %%1 %%2 %%3", "%s", time, getMove chupa]' % label)

    def text(self):
        return '\n'.join(self.lines) + '\n'


def header(t, title):
    t.a('// ' + '=' * 76)
    t.a('//  %s' % title)
    t.a('//  Written by gen_chupa_anim_tests.py (edit that, not this file).')
    t.a('//  UDChupacabraAnim (CfgMovesChupa) on the Everon airfield edge at 13:00.')
    t.a('//  Quiet: no weapons, nobody fires, --nosound. CHUPA/CHF/CHW lines carry')
    t.a('//  sim time, move, ASL position and heading for the offline checks.')
    t.a('// ' + '=' * 76)
    t.a('triSimUntil { alive player }')
    t.a('triCaptureAtPresent true')
    t.a('player allowDammage false')
    t.a('player setCaptive true')
    t.a('removeAllWeapons player')
    # Far south, out of every frame; the creature never tracks him.
    t.a('player setPos [4800, 10000, 0]')
    t.a('setDate [1985, 5, 10, 13, 0]')
    t.a('0 setOvercast 0.3')
    t.a('0 setFog 0')
    t.a('setViewDistance 1200')
    t.a('showCinemaBorder false')
    t.a('chC = createCenter civilian')
    t.a('chG = createGroup civilian')
    t.a('chupa = objNull')
    t.a('chAt = [%d, %d, 0]' % SPOT)
    t.a('"UDChupacabraAnim" createUnit [chAt, chG, "chupa = this", 0.5, "PRIVATE"]')
    t.a('logInfo format ["CHUPA spawn %1 %2", time, getMove chupa]')
    t.a('triSimFrames 2')
    t.a('triAssert [!(isNull chupa)]')
    t.a('triAssertEq [typeOf chupa, "UDChupacabraAnim"]')
    t.a('triAssert [alive chupa]')
    t.a('chAnimN = 0')
    t.a('chupa addEventHandler ["AnimChanged", {chAnimN = chAnimN + 1; logInfo format ["CHUPA anim %1 %2", time, _this select 1]}]')
    t.a('chupa setCaptive true')
    t.a('chupa setDir 90')
    t.a('chG setBehaviour "SAFE"')
    t.a('CHW_ON = true')
    t.a('CHW_QUIT = false')
    t.a('[chupa, 0.05] exec "chupa_watch.sqs"')
    # HUD off and a long lens; triSetView does the framing.
    t.a('chCam = "camera" camCreate [4800, 10500, 30]')
    t.a('chCam cameraEffect ["internal", "back"]')
    t.a('chCam camSetTarget [4800, 10700, 30]')
    t.a('chCam camSetFov 0.5')
    t.a('chCam camCommit 0')
    # Views: [east, up, north, dirE, dirUp, dirN]. cvD metres out on the
    # south side, cvO metres ahead of the creature (east), eye cvE, aim cvA.
    t.a('cvD = 6')
    t.a('cvO = 0')
    t.a('cvE = 1.1')
    t.a('cvA = 0.8')
    t.a('cvSide = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + cvO, (_p select 2) + cvE, (_p select 1) - cvD, -cvO, cvA - cvE, cvD] }')
    # 3/4 front view from the south-east.
    t.a('cv34 = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + cvD * 0.64, (_p select 2) + cvE, (_p select 1) - cvD * 0.77, -cvD * 0.64, cvA - cvE, cvD * 0.77] }')
    t.a('triSimFrames 150')
    t.log('settled')
    t.a('triSetView ([] call cvSide)')
    t.a('triSimFrames 6')
    t.a('triScreenshot "00_warmup_discard"')


def place(t, x=None, d=90):
    if x is None:
        t.a('chupa setPos chAt')
    else:
        t.a('chupa setPos [%g, %d, 0]' % (x, SPOT[1]))
    t.a('chupa setDir %d' % d)


def cam(t, d, e, a, o=0):
    t.a('cvD = %g' % d)
    t.a('cvE = %g' % e)
    t.a('cvA = %g' % a)
    t.a('cvO = %g' % o)


def end(t, tag):
    t.a('triAssert [alive chupa]')
    t.log(tag + '_end')
    t.a('CHW_QUIT = true')
    t.a('triEndTest')


# --------------------------------------------------------------------------
# Test 1: every clip forced (switchMove / playMove), one burst each.
# --------------------------------------------------------------------------
# state, label, cycle seconds as forced (cycle / relSpeedMin: the AI's
# animation choice is off, so the wanted speed is 0 and the engine plays the
# clip at its band minimum), frames, all fours?
LOOPS = [
    ('ChupaIdle', 'idle_twitch', 4.0, 90, False),
    ('ChupaStalkF', 'stalk_crouch', 3.2, 64, False),
    ('ChupaTurnL', 'stalk_crouch_turnl', 1.0, 30, False),
    ('ChupaTurnR', 'stalk_crouch_turnr', 1.0, 30, False),
    ('ChupaLope', 'lope', 0.75 / 0.7, 32, False),
    ('ChupaRush', 'sprint_rush', 0.45 / 0.85, 24, False),
    ('ChupaFreeze', 'freeze', 6.0, 100, True),
    ('ChupaCrawlF', 'crawl', 1.2 / 0.6, 50, True),
    ('ChupaCrawlTurnL', 'crawl_turnl', 1.2, 36, True),
    ('ChupaCrawlTurnR', 'crawl_turnr', 1.2, 36, True),
    ('ChupaScuttle', 'flee_scuttle', 0.6 / 0.7, 28, True),
    ('ChupaGallop', 'gallop', 0.5, 24, True),
    ('ChupaFeed', 'feed', 3.0, 80, True),
]


def clips_test(wall):
    t = T(wall)
    header(t, 'Chupacabra clips forced one by one (check 2, loop wraps, joints)')
    t.a('chupa disableAI "MOVE"')
    t.a('chupa disableAI "ANIM"')
    t.a('chupa stop true')
    t.a('triSetView ([] call cvSide)')
    t.a('triSimFrames 6')
    t.a('triScreenshot "01_spawn_idle"')
    for state, label, cyc, n, low in LOOPS:
        place(t)
        t.a('chupa switchMove "%s"' % state)
        t.wait(0.05)
        t.log('forced_' + label)
        travel = label in ('stalk_crouch', 'lope', 'sprint_rush', 'crawl', 'flee_scuttle', 'gallop')
        if low:
            cam(t, 5.5, 0.7, 0.45)
        else:
            cam(t, 6, 1.1, 0.8)
        # One cycle plus a quarter across the wrap; travelling clips are
        # followed side on, the others held from 3/4 front.
        t.a('triSetView ([] call %s)' % ('cvSide' if travel else 'cv34'))
        t.burst('c_' + label, n, cyc * 1.25, 'cvSide' if travel else None)
        t.log('after_' + label)
    # One-shots. A switchMove'd state is an external move until its first
    # cycle ends; a playMove queues behind it and a later switchMove puts it
    # back in the queue (SoldierOldMove.cpp:377-394, 289-305). So the snap
    # and the lunge are cut in at frame 0 (their frame 0 is the freeze and
    # the idle pose), and the pounce and recoil are played once the state
    # before them has run a full cycle.
    place(t)
    cam(t, 6, 1.0, 0.7)
    t.a('triSetView ([] call cv34)')
    t.a('chupa switchMove "ChupaSnapUp"')
    t.log('call_snap_up')
    t.burst('c_snap_up', 30, 0.45 + 0.55)
    t.wait(0.5)
    place(t)
    cam(t, 6, 1.1, 0.8, 0.8)
    t.a('triSetView ([] call cvSide)')
    t.a('chupa switchMove "ChupaLungeStrike"')
    t.log('call_lunge')
    t.burst('c_lunge_strike', 36, 0.7 + 0.5)
    t.wait(1)
    # pounce from the gallop (tracked), with the feed queued behind it.
    place(t, SPOT[0] - 8)
    t.a('chupa switchMove "ChupaGallop"')
    t.wait(0.6)
    cam(t, 7, 0.9, 0.6, 1.5)
    t.a('chupa playMove "ChupaPounce"')
    t.a('chupa playMove "ChupaFeed"')
    t.log('call_pounce')
    t.burst('c_pounce', 40, 0.8 + 0.5, 'cvSide')
    # The queued feed counts as done after one 3 s cycle.
    t.wait(3.2)
    t.log('after_pounce')
    # recoil out of the feed.
    cam(t, 6, 1.0, 0.6, -0.6)
    t.a('triSetView ([] call cvSide)')
    t.a('chupa playMove "ChupaRecoil"')
    t.log('call_recoil')
    t.burst('c_recoil', 40, 0.9 + 0.5)
    t.wait(1)
    t.log('after_recoil')
    # Death last: the AI's animation choice back on, then killed.
    place(t)
    t.a('chupa enableAI "ANIM"')
    t.a('chupa switchMove "ChupaIdle"')
    t.wait(1)
    cam(t, 6, 1.0, 0.5)
    t.a('triSetView ([] call cv34)')
    t.a('chupa setDammage 1')
    t.log('call_die')
    t.burst('c_die', 50, 1.4 + 0.8)
    t.wait(1)
    t.a('logInfo format ["CHUPA dead %1 %2 alive %3", time, getMove chupa, alive chupa]')
    t.a('triAssert [!(alive chupa)]')
    t.a('triSetView ([] call cv34)')
    t.a('triSimFrames 6')
    t.a('triScreenshot "c_dead"')
    t.a('CHW_QUIT = true')
    t.a('triEndTest')
    return t


# --------------------------------------------------------------------------
# Joint close-ups in motion, for the mesh A/B (stock mesh against the
# joint-fix mesh): the stalk's knee crease, the gallop's wrists and ankles,
# the crawl's elbows, the scuttle's neck, the feed's wrists, the shoulders on
# the pounce and the recoil. Forced as in the clips test.
# --------------------------------------------------------------------------
JOINTS = [
    # state, label, forced cycle s, frames, view, distance, eye, aim, ahead
    ('ChupaStalkF', 'j_stalk_knee', 3.2, 48, 'cvSide', 2.4, 0.55, 0.45, 0),
    ('ChupaStalkF', 'j_stalk_knee34', 3.2, 48, 'cv34', 2.6, 0.7, 0.45, 0),
    ('ChupaGallop', 'j_gallop_limbs', 0.5, 24, 'cvSide', 2.8, 0.45, 0.35, 0),
    ('ChupaGallop', 'j_gallop_limbs34', 0.5, 24, 'cv34', 3.0, 0.6, 0.35, 0),
    ('ChupaCrawlF', 'j_crawl_elbow', 2.0, 40, 'cv34', 2.6, 0.7, 0.35, 0),
    ('ChupaScuttle', 'j_scuttle_neck', 0.6 / 0.7, 26, 'cv34', 2.6, 0.7, 0.45, 0),
    ('ChupaFeed', 'j_feed_wrist', 3.0, 48, 'cv34', 2.2, 0.8, 0.3, 0),
]


def joints_test(wall):
    t = T(wall)
    header(t, 'Chupacabra joints in motion (mesh A/B)')
    t.a('chupa disableAI "MOVE"')
    t.a('chupa disableAI "ANIM"')
    t.a('chupa stop true')
    for state, label, cyc, n, view, d, e, a, o in JOINTS:
        place(t)
        t.a('chupa switchMove "%s"' % state)
        t.wait(0.3)
        cam(t, d, e, a, o)
        t.a('triSetView ([] call %s)' % view)
        t.burst(label, n, cyc * 1.1, view)
        t.wait(0.3)
    # Shoulders on the pounce (3/4 front, tracked) and the recoil.
    place(t, SPOT[0] - 8)
    t.a('chupa switchMove "ChupaGallop"')
    t.wait(0.6)
    cam(t, 3.6, 1.2, 0.6, 2.2)
    t.a('chupa playMove "ChupaPounce"')
    t.a('chupa playMove "ChupaFeed"')
    t.log('call_pounce')
    t.burst('j_pounce_shoulder', 40, 0.8 + 0.3, 'cv34')
    t.wait(3.4)
    cam(t, 3.4, 1.1, 0.6, -0.4)
    t.a('triSetView ([] call cv34)')
    t.a('chupa playMove "ChupaRecoil"')
    t.log('call_recoil')
    t.burst('j_recoil_shoulder', 40, 0.9 + 0.3)
    t.wait(1)
    end(t, 'joints')
    return t


# --------------------------------------------------------------------------
# Test 2: what the AI picks, speeds, prone-turn flicker, path-end overshoot,
# planted feet against a fixed camera.
# --------------------------------------------------------------------------
MODES = [
    # setUnitPos, behaviour, speed mode
    ('AUTO', 'SAFE', 'LIMITED'), ('AUTO', 'SAFE', 'NORMAL'), ('AUTO', 'SAFE', 'FULL'),
    ('AUTO', 'CARELESS', 'LIMITED'), ('AUTO', 'CARELESS', 'NORMAL'),
    ('AUTO', 'AWARE', 'LIMITED'), ('AUTO', 'AWARE', 'NORMAL'), ('AUTO', 'AWARE', 'FULL'),
    ('AUTO', 'COMBAT', 'LIMITED'), ('AUTO', 'COMBAT', 'NORMAL'), ('AUTO', 'COMBAT', 'FULL'),
    ('AUTO', 'STEALTH', 'LIMITED'), ('AUTO', 'STEALTH', 'NORMAL'), ('AUTO', 'STEALTH', 'FULL'),
    ('UP', 'SAFE', 'LIMITED'), ('UP', 'AWARE', 'LIMITED'), ('UP', 'COMBAT', 'LIMITED'), ('UP', 'SAFE', 'FULL'),
    ('DOWN', 'SAFE', 'LIMITED'), ('DOWN', 'AWARE', 'LIMITED'), ('DOWN', 'COMBAT', 'LIMITED'), ('DOWN', 'SAFE', 'FULL'),
]

# gait, unitPos, behaviour, speed, run length (m), seconds to watch,
# foot burst: (frames, cover seconds) or None
SPEEDS = [
    ('gallop', 'DOWN', 'SAFE', 'FULL', 90, 16, (24, 0.6)),
    ('rush', 'UP', 'SAFE', 'FULL', 90, 16, (24, 0.55)),
    ('lope', 'AUTO', 'AWARE', 'LIMITED', 40, 12, (30, 0.9)),
    ('stalk', 'AUTO', 'SAFE', 'LIMITED', 20, 12, (40, 1.8)),
    ('scuttle', 'DOWN', 'AWARE', 'LIMITED', 40, 12, (24, 0.8)),
    ('crawl', 'DOWN', 'SAFE', 'LIMITED', 15, 12, (40, 1.4)),
]


def modes(t, pos, beh, spd):
    t.a('chupa setUnitPos "%s"' % pos)
    t.a('chG setBehaviour "%s"' % beh)
    t.a('chG setSpeedMode "%s"' % spd)


def ai_test(wall, group_move=False):
    """group_move: order the group (`move`) instead of the unit (`doMove`).
    doMove puts the unit in a new subgroup whose speed mode is always NORMAL
    (AIGroupCmd.cpp:223-249, AISubgroup.cpp:498), so setSpeedMode, which sets
    the main subgroup only (GameStateExtGrp.cpp:290-313), cannot reach it."""
    t = T(wall)
    header(t, 'Chupacabra AI states per mode, speeds, flicker, overshoot, feet'
           + (' (group move orders)' if group_move else ''))
    go = (lambda x, y: 'chG move [%d, %d, 0]' % (x, y)) if group_move else (lambda x, y: 'chupa doMove [%d, %d, 0]' % (x, y))
    hold = 'chG move (getPos chupa)' if group_move else 'chupa doMove (getPos chupa)'
    speeds = [sp for sp in SPEEDS if sp[0] not in ('gallop', 'rush')] if group_move else SPEEDS
    # -- 1. the mode table ---------------------------------------------------
    x0, x1 = SPOT[0] - 20, SPOT[0] + 40
    east = True
    for i, (pos, beh, spd) in enumerate(MODES):
        tag = 'mode_%s_%s_%s' % (pos, beh, spd)
        frm, to = (x0, x1) if east else (x1, x0)
        place(t, frm, 90 if east else 270)
        modes(t, pos, beh, spd)
        t.a(hold)
        t.wait(3)
        t.log(tag + '_stopped')
        t.a(go(to, SPOT[1]))
        t.log(tag + '_go')
        t.wait(7)
        t.log(tag + '_moving')
        east = not east
    # -- 2. speeds, overshoot and feet ----------------------------------------
    for gait, pos, beh, spd, length, watch, foot in speeds:
        tag = 'speed_' + gait
        start = SPOT[0] - 30
        target = start + length
        place(t, start, 90)
        modes(t, pos, beh, spd)
        t.a(hold)
        t.wait(2.5)
        t.log(tag + '_stopped')
        t.a(go(target, SPOT[1]))
        t.log(tag + '_go', ' target %d' % target)
        if foot:
            n, cover = foot
            # Fixed low camera 2.5 m ahead of where the creature is after
            # a steady stretch, at foot height.
            settle = 3 if gait in ('gallop', 'rush') else 4
            t.wait(settle)
            t.log(tag + '_foot')
            low = gait in ('gallop', 'scuttle', 'crawl')
            ahead = {'gallop': 2.0, 'rush': 1.8, 'lope': 1.2, 'stalk': 0.5, 'scuttle': 1.0, 'crawl': 0.4}[gait]
            cam(t, 4.5, 0.35 if low else 0.45, 0.15, ahead)
            t.a('chFoot = [] call cvSide')
            t.a('triSetView chFoot')
            t.burst('f_' + gait, n, cover)
            t.wait(max(0.5, watch - settle - 1))
        else:
            t.wait(watch)
        t.log(tag + '_late')
        # Let it settle at the end of the path.
        t.wait(4)
        t.log(tag + '_rest', ' target %d' % target)
    if group_move:
        end(t, 'ai2')
        return t
    # -- 3. turning in place, lying down and upright ---------------------------
    for pos, beh, label in [('DOWN', 'SAFE', 'down'), ('UP', 'SAFE', 'up'), ('AUTO', 'STEALTH', 'stealth')]:
        tag = 'turn_' + label
        place(t, SPOT[0], 90)
        modes(t, pos, beh, 'LIMITED')
        t.a('chupa doMove (getPos chupa)')
        t.wait(3)
        t.log(tag + '_still')
        t.a('chAnimN = 0')
        # A point straight behind it: it has to turn round before it moves.
        t.a('chupa doMove [%d, %d, 0]' % (SPOT[0] - 25, SPOT[1]))
        t.log(tag + '_go')
        t.wait(2)
        t.a('logInfo format ["CHUPA %s_count2 %%1 %%2 anims %%3 dir %%4", time, getMove chupa, chAnimN, getDir chupa]' % tag)
        t.wait(2)
        t.a('logInfo format ["CHUPA %s_count4 %%1 %%2 anims %%3 dir %%4", time, getMove chupa, chAnimN, getDir chupa]' % tag)
    # A watched turn with no path: doWatch behind it while lying still.
    place(t, SPOT[0], 90)
    modes(t, 'DOWN', 'SAFE', 'LIMITED')
    t.a('chupa doMove (getPos chupa)')
    t.wait(3)
    t.a('chAnimN = 0')
    t.a('chupa doWatch [%d, %d, 0]' % (SPOT[0] - 50, SPOT[1] + 20))
    t.log('turn_watch_go')
    t.wait(3)
    t.a('logInfo format ["CHUPA turn_watch_count3 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]')
    end(t, 'ai')
    return t


# --------------------------------------------------------------------------
# Test 3: the video chase, scripted beats (chupa_chase.sqs), once at real
# speed for the timings and once slowed down for the frames.
# --------------------------------------------------------------------------
def chase_test(wall, frames):
    t = T(wall)
    header(t, 'Chupacabra video chase: gallop, pounce, feed, recoil, rush, lunge')
    t.a('chV = createGroup civilian')
    t.a('victim = objNull')
    # He runs on 0.35 s into the 4 m leap (about 1.7 m), so the cue is short.
    t.a('CHS_POUNCE_AT = 3.5')
    t.a('CHS_DROP_AFTER = 0.35')
    t.a('CHS_LUNGE_AT = 3.0')
    t.a('CHS_FEED_HOLD = 3')
    t.a('CHS_RUSH_DELAY = 1.5')
    t.a('CHS_CHASE_MAX = 20')
    t.a('CHS_REORDER = 30')
    t.a('CHS_SIDE = 2.2')
    t.a('CHS_RUSH_GAP = 6')
    for run, slow in [('rt', False), ('slow', True)]:
        vx = SPOT[0] - 60 + 14
        cx = SPOT[0] - 60
        t.a('if (!(isNull victim)) then { deleteVehicle victim }')
        t.a('triSimFrames 2')
        t.a('"Civilian" createUnit [[%d, %d, 0], chV, "victim = this", 0.5, "PRIVATE"]' % (vx, SPOT[1]))
        t.a('triSimFrames 2')
        t.a('triAssert [!(isNull victim)]')
        t.a('victim setCaptive true')
        t.a('victim allowDammage false')
        t.a('victim setDir 90')
        t.a('chV setBehaviour "CARELESS"')
        t.a('chupa stop false')
        t.a('chupa enableAI "MOVE"')
        t.a('chupa setPos [%d, %g, 0]' % (cx, SPOT[1] - 2.2))
        t.a('chupa setDir 90')
        modes(t, 'DOWN', 'SAFE', 'FULL')
        t.a('chupa doMove (getPos chupa)')
        t.wait(3)
        t.log('chase_%s_ready' % run)
        t.a('logInfo format ["CHUPA chase_%s_victim %%1 %%2 %%3", time, getMove victim, getPosASL victim]' % run)
        t.a('CHS_DONE = false')
        t.a('[chupa, victim] exec "chupa_chase.sqs"')
        if not slow:
            t.a('triSimUntil { CHS_DONE }')
        else:
            # A side view that keeps both in frame.
            t.a('chMid = { private ["_a", "_b", "_mx", "_mz", "_h"]; _a = getPosASL chupa; _b = getPosASL victim; '
                '_mx = ((_a select 0) + (_b select 0)) / 2; _mz = ((_a select 1) + (_b select 1)) / 2; '
                '_h = ((_a select 2) + (_b select 2)) / 2; [_mx, _h + 1.3, _mz - 9, 0, -0.6, 9] }')
            t.a('triSetView ([] call chMid)')
            # The beats run in sim time in the script; the frames only watch.
            # The cover is a ceiling: frames after CHS_DONE film the aftermath.
            t.burst('chase', frames, 20.0, 'chMid')
            t.a('triSimUntil { CHS_DONE }')
        t.log('chase_%s_done' % run)
        t.a('logInfo format ["CHUPA chase_%s_victim_end %%1 %%2 %%3 damage %%4", time, getMove victim, getPosASL victim, damage victim]' % run)
        t.wait(2)
    end(t, 'chase')
    return t


def write(name, title, t, timeout):
    with open(os.path.join(HERE, name + '.test.sqf'), 'w', newline='\n') as f:
        f.write(t.text())
    with open(os.path.join(HERE, name + '.test.toml'), 'w', newline='\n') as f:
        f.write(TOML.format(title=title, timeout=timeout))
    print('%s: %d statements, %d frames' % (name, len(t.lines), t.frames))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--wall', type=float, default=0.13)
    ap.add_argument('--chase-frames', type=int, default=500)
    args = ap.parse_args()
    w = args.wall
    c = clips_test(w)
    write('chupa_anim_clips', 'Chupacabra clips forced one by one', c, int(240 + c.frames * w * 2.5))
    j = joints_test(w)
    write('chupa_anim_joints', 'Chupacabra joints in motion', j, int(240 + j.frames * w * 2.5))
    a = ai_test(w)
    write('chupa_anim_ai', 'Chupacabra AI states, speeds and feet', a, int(600 + a.frames * w * 2.5))
    # ai_test(w, group_move=True) was run once (2026-11-07): group `move`
    # and waypoint orders never moved the creature (scripted civilian group,
    # --nosound), so it is not written any more.
    s = chase_test(w, args.chase_frames)
    write('chupa_anim_chase', 'Chupacabra video chase beats', s, int(300 + s.frames * w * 2.5))


if __name__ == '__main__':
    main()
