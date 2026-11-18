"""Writes the Lebanon80 promo shoot takes (Uslu dur! Guerrilla Mode promo, IDF occupier vs Hizballah).

    python gen_promo.py <take> [<take> ...]      # or: all
    bash run-batch.sh <take> [<take> ...]          # one game at a time, encodes each take's dumps
    python check_run.py <run folder>               # pin and settle of every clip
    python gate.py <run folder>                    # thirds gate and clips\\thirds-sheet-<take>.jpg
    python judder.py <run folder>                  # per-frame camera log and picture judder check

The reshoot (2026-11-17, build 850fce64): every take pins the object detail, settles before every clip,
records 15 head frames that the encoder cuts, records at 2560 x 1440, aims the lens so that the subject sits
on a third (cam(..., subj=, sx=, sy=, aim=True)) and logs the camera every frame (PCF lines). Kaiju takes run
with PROMO_NOSHAKE: his camera tremor was the first shoot's judder (see clips\\shoot-notes.md).

Places (Lebanon80, scouted 2026-11-17 with promo_probe / promo_scout, maps from plot_scout.py):
  Tyre       coastal town with a harbour, houses x 8600..9250, y 9650..10100.
  Ghajar     hillside village, houses x 14080..14350, y 10840..11100. Straight roads: north road x = 14115 from
             y 11100 to 11550 (along a steep valley side); east road y = 11098 from x 14100 to 14550 (open
             meadow, lamp lit at night).
  Coast hwy  dead straight along y = 8307 from x 9433 to 11158, in a wide sandy cut.
  North rd   x = 9695 crosses a river on a bridge near y 11000..11300: not used.
  Olive plain east of Tyre around [10450, 9250]: open ground with scattered trees, slopes down to the north.
  Marjayoun  hilltop post [13043, 12140]; the Litani valley below to the west.
Sun (June 1985 on this world): sunrise about 07:15, sunset about 16:50, golden 15:45..16:35, blue hour
17:05..17:25, dark from about 17:30.
Moving men: kjSteer walking (psPatrol, about 1.6 m/s); a doMove makes them jog. Vehicles: psGo (doMove each).
"""
import sys

from promolib import THIRD, TWO3, T, cam, footer, header, q, record, settle, time_of_day, write

IDF_PATROL = '["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWLAW", "LoBoGolaniWB"]'
HEZ_SQUAD = '["LoBo_HizballahLeader", "LoBo_HizballahRifle4", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG", "LoBo_HizballahRifle1", "LoBo_HizballahRifle4"]'
NO_TRAFFIC = 'gmTrafficTune ["maxCiv", 0]; gmTrafficTune ["maxPatrols", 0]; gmTrafficTune ["maxConvoys", 0]; gmTrafficTune ["maxParked", 0]'
NO_CIVS = '{ if ((side _x) == civilian) then { if ((vehicle _x) != _x) then { deleteVehicle (vehicle _x) }; deleteVehicle _x } } forEach (nearestObjects [%s, ["Man"], %d])'


def wait_s(t, seconds):
    t.a('triSimFrames %d' % int(seconds * 30))


def until(t, cond, limit=120):
    t.a('prTw = time')
    t.a('triSimUntil { (%s) || (time > prTw + %d) }' % (cond, limit))


def free(x, y, z, r=1.5):
    """an eye spot nudged clear of objects"""
    return '[[%g, %g, %g], %g] call psFreeNear' % (x, y, z, r)


def ahead(start, heading, metres):
    import math
    return [round(start[0] + metres * math.sin(math.radians(heading)), 1), round(start[1] + metres * math.cos(math.radians(heading)), 1), 0]


def walkers(t, var, side, classes, start, heading, dest, spacing=3.5, lateral=0.8, beh='SAFE', rate=0.65):
    """a file of men walking (kjSteer) from start to dest: group var, its men varU"""
    t.c('%s = [%s, %s, %s, %g, %s, %g, %g, "%s", %g] call psPatrol; %sU = units %s' % (
        var, side, classes, q(list(start)), heading, q(list(dest)), spacing, lateral, beh, rate, var, var))
    t.c('logInfo format ["PROMO walkers %s %%1 men at %%2", count %sU, getPos (%sU select 0)]' % (var, var, var))


def clear_civs(t, centre, radius=900):
    t.c(NO_CIVS % (q(list(centre)), radius))


# ======================================================================================== landscapes
def land():
    t = T('promo_land', 'Lebanon80 promo: landscape establishing shots, dawn, day, golden hour, blue hour')
    header(t, 7, 25, overcast=0.2, fog=0.0, view=3500)
    t.c(NO_TRAFFIC)
    # dawn: orbit of Tyre and its harbour from the sea side, sun rising inland
    cam(t, 'orbit', c=[8950, 9800, 0], r0=760, r1=700, h0=170, h1=155, a0=215, a1=250, asl=True, fov0=0.5,
        subjp=[8900, 9850, 0], sx=TWO3, sy=TWO3, aim=True)
    record(t, 'c01_dawn_tyre_harbour_orbit', 10, note='dawn orbit of Tyre and its harbour from the sea, sun rising inland')
    time_of_day(t, 7, 35)
    # low push over the olive plain toward Tyre (130 m in 8 s: peak about 24 m/s at 12 to 14 m up)
    cam(t, 'lerp', e0=[10330, 9240, 14], e1=[10215, 9300, 12], fov0=0.55, subjp=[9050, 9800, 15], sx=TWO3, sy=0.36, aim=True)
    record(t, 'c02_dawn_olive_plain_push', 8, note='low push over the olive plain toward Tyre at dawn')
    time_of_day(t, 7, 45, fog=0.25)
    cam(t, 'lerp', e0=[14115, 11750, 170], e1=[14117, 11530, 120], fov0=0.5, subjp=[14215, 10975, 6], sx=THIRD, sy=0.6, aim=True)
    record(t, 'c08_dawn_ghajar_road_misty_descent', 10, note='descending down the north road toward Ghajar in morning haze, the village on the left third')
    time_of_day(t, 10, 30, fog=0.0)
    cam(t, 'lerp', e0=[13850, 10650, 110], e1=[13940, 10740, 70], fov0=0.55, subjp=[14215, 10970, 6], sx=TWO3, sy=0.6, aim=True)
    record(t, 'c03_day_ghajar_village_dolly', 10, note='descending aerial dolly onto Ghajar on its hillside, mountains behind')
    time_of_day(t, 10, 45)
    cam(t, 'lerp', e0=[8560, 9420, 125], e1=[8645, 9385, 118], asl=True, fov0=0.45, subjp=[8900, 9850, 0], sx=THIRD, sy=0.6, aim=True)
    record(t, 'c04_day_tyre_shore_track', 8, note='lateral track off the shore with Tyre and its harbour on the left third')
    # golden hour: sun setting over the sea behind Tyre
    time_of_day(t, 16, 25)
    cam(t, 'lerp', e0=[9700, 9550, 170], e1=[9560, 9640, 155], asl=True, fov0=0.55, subjp=[8950, 9820, 8], sx=THIRD, sy=0.6, aim=True)
    record(t, 'c05_golden_tyre_sunset_over_sea', 10, note='golden hour, Tyre against the sun setting over the sea')
    time_of_day(t, 16, 35, overcast=0.1)
    cam(t, 'lerp', e0=[13350, 11750, 480], e1=[13180, 11660, 455], asl=True, fov0=0.42, subjp=[11600, 11450, 0], sx=TWO3, sy=0.66, aim=True)
    record(t, 'c07_golden_litani_valley_pan', 10, note='the Litani valley in low sun from the Marjayoun heights')
    time_of_day(t, 16, 45)
    cam(t, 'orbit', c=[14215, 10970, 0], r0=330, r1=300, h0=360, h1=345, a0=60, a1=105, asl=True, fov0=0.6,
        subjp=[14215, 10975, 4], sx=THIRD, sy=0.62, aim=True)
    record(t, 'c09_golden_ghajar_orbit', 10, note='orbit of Ghajar at sunset, long shadows')
    # blue hour
    time_of_day(t, 17, 10)
    cam(t, 'lerp', e0=[9350, 9480, 100], e1=[9300, 9530, 92], asl=True, fov0=0.5, subjp=[8950, 9820, 8], sx=TWO3, sy=0.58, aim=True)
    record(t, 'c27_bluehour_tyre_lights_coming_on', 9, note='blue hour over Tyre, lamps on, last light on the sea')
    time_of_day(t, 17, 27)
    cam(t, 'orbit', c=[14215, 10990, 0], r0=260, r1=240, h0=330, h1=320, a0=120, a1=160, asl=True, fov0=0.55,
        subjp=[14250, 11000, 4], sx=TWO3, sy=0.6, aim=True)
    record(t, 'c06_bluehour_ghajar_lamps_orbit', 10, note='orbit of Ghajar at the end of blue hour, the lamp-lit streets')
    footer(t)
    write(t)


def checkpoint_stage(t, night=False):
    """The checkpoint on the paved road into Ghajar (the straight east-west stretch at y 11068.8, read from the road
    net by psRoadHeading): the bar ACROSS the carriageway (the bar model's long axis is its local x, so its setDir is
    the road heading), sandbag walls on both shoulders running along the road, the M113 pulled off on the north
    shoulder at an angle covering the approach, soldiers at the bar and on the shoulders. cpR is the bar's road
    point, cpA the heading of the approach (the oncoming lane, from the SE curve). By day a civilian van comes
    in along the road and stops at the bar; at night a Humvee stands in the approach lane, headlights on."""
    t.c('cpRH = [14300, 11068.8, 0] call psRoadHeading; cpR = [(cpRH select 1) select 0, (cpRH select 1) select 1, 0]; cpH = cpRH select 0; cpA = cpH; logInfo format ["PROMO checkpoint at %1 road heading %2", cpR, cpH]')
    t.c('pBar = ["MAP_Barrier", cpR, cpH] call psProp; pSb1 = ["LoBo_SandbagX4", [cpR, cpA, 3.0, 5.4] call psOff, cpH - 90] call psProp; pSb2 = ["LoBo_SandbagX4", [cpR, cpA, 4.5, -6.6] call psOff, cpH - 90] call psProp; logInfo format ["PROMO props %1 %2 %3 dirs %4 %5", typeOf pBar, typeOf pSb1, typeOf pSb2, getDir pBar, getDir pSb1]')
    t.c('pApc = ["LoBo_Zelda", [cpR, cpA, 13, -10] call psOff, cpA - 40, west, "LoBoGolaniWB", true, false] call psVehC; (driver pApc) disableAI "MOVE"; (group (driver pApc)) setBehaviour "SAFE"')
    t.c('pCp = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWB", "LoBoGolaniWMG"], cpR, cpA, 0.7] call psInf; pCp setBehaviour "SAFE"; pCpU = units pCp')
    t.c('[pCpU select 0, [cpR, cpA, 1.3, 1.4] call psOff, cpA, "UP"] call psPost; [pCpU select 1, [cpR, cpA, 1.3, -1.9] call psOff, cpA - 10, "UP"] call psPost; [pCpU select 2, [cpR, cpA, 1.5, 5.6] call psOff, cpA + 15, "UP"] call psPost; [pCpU select 3, [cpR, cpA, 4.0, -7.4] call psOff, cpA - 10, "MIDDLE"] call psPost')
    if night:
        t.c('pHum = ["LoBoHMWV_Mag_Patrol", [cpR, cpA, 16, 1.8] call psOff, cpA + 180, west, "LoBoGolaniWB", true, false] call psVehC; (driver pHum) disableAI "MOVE"; (group (driver pHum)) setBehaviour "SAFE"; pHum engineOn true')
    else:
        t.c('pVan = ["LoBo_S1203", [14350.6, 11058.1, 0], 300, civilian, "LoBo_Civ_01", false, false] call psVehC; [pVan] exec "vanstop.sqs"')
    t.a('triSimFrames 3')
    # the overhead check still of the layout (not for the cut)
    t.c('PC_RUN = false; PC_CAM camSetPos [(cpR select 0), (cpR select 1) - 0.5, 34]; PC_CAM camSetDir 0; PC_CAM camSetDive -89; PC_CAM camSetFov 0.7; PC_CAM camCommit 0')
    settle(t)
    t.a('triScreenshot "top_checkpoint_%s"' % ('night' if night else 'day'))
    if not night:
        t.c('[pVan, [cpR, cpA, 8.5, 1.8] call psOff, "LIMITED", "CARELESS"] call psGo')


# ======================================================================================== the occupier
def occ1():
    t = T('promo_occ1', 'Lebanon80 promo: an IDF patrol walks into Ghajar, a staffed checkpoint, a garrison post')
    header(t, 9, 50, overcast=0.2, fog=0.0, view=2500, player_pos=[14700, 11500, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [14200, 11100, 0])
    # ---- the patrol walks south down the north road into Ghajar (six Golani, a file, about 1.6 m/s)
    walkers(t, 'pPat', 'west', IDF_PATROL, [14115, 11214, 0], 180, [14115, 11080, 0], spacing=3.5, lateral=0.7, beh='SAFE')
    wait_s(t, 1)
    cam(t, 'lerp', e0=[14117.5, 11186, 1.8], e1=[14117.7, 11186.4, 1.85], fov0=0.2, subj='pPatU select 1', z=1.65, sx=TWO3, sy=THIRD, aim=True)
    record(t, 'c10_day_idf_patrol_ghajar_front', 9, note='IDF patrol walks down the road into Ghajar toward a long lens, second man on the right third',
           gate=dict(x=TWO3, y=THIRD, hz=False))
    cam(t, 'follow', obj='pPatU select 5', o0=[0.9, -3.2, 1.9], o1=[1.1, -3.8, 1.95], to=[0, 14, 1.3], pa=0.2, hda=0.05, fov0=0.55,
        subj='pPatU select 2', z=1.6, sx=THIRD, sy=0.38, aim=True)
    record(t, 'c11_day_idf_patrol_ghajar_behind', 8, note='over the shoulder of the last man as the patrol walks into Ghajar', gate=dict(x=THIRD, y=0.38, hz=False))
    cam(t, 'follow', obj='pPatU select 2', o0=[-6.5, 2.5, 2.1], o1=[-6.5, -1.0, 2.1], to=[0, 0, 1.4], pa=0.15, hda=0.04, fov0=0.5,
        subj='pPatU select 2', z=1.65, sx=TWO3, sy=THIRD, aim=True)
    record(t, 'c12_day_idf_patrol_ghajar_side_dolly', 8, note='side dolly alongside the walking patrol at the edge of the village', gate=dict(x=TWO3, y=THIRD, hz=False))
    t.c('pPat call psDelGrp')
    # ---- the checkpoint on the paved road into Ghajar; a civilian van comes in along it and stops at the bar
    time_of_day(t, 11, 20)
    clear_civs(t, [14300, 11068, 0], 500)
    checkpoint_stage(t)
    cam(t, 'lerp', e0='[cpR, cpA, 24, 2.5, 3.6] call psOffZ', e1='[cpR, cpA, 20, 2.0, 3.0] call psOffZ', fov0=0.32, subj='pCpU select 0', z=1.65, sx=THIRD, sy=0.38, aim=True)
    record(t, 'c13_day_idf_checkpoint_van_arrives', 10, note='slow push on the checkpoint on the paved road into Ghajar as a civilian van comes in and stops at the closed bar')
    cam(t, 'lerp', e0='[cpR, cpA, -2.2, 0.6, 1.9] call psOffZ', e1='[cpR, cpA, -1.8, 0.8, 1.9] call psOffZ', fov0=0.4, subj='pVan', z=1.4, sx=TWO3, sy=0.45, aim=True)
    record(t, 'c14_day_idf_checkpoint_low_behind_soldier', 7, note='behind the checkpoint soldiers at the bar, the stopped van in front of it', gate=dict(x=TWO3, y=0.45, hz=False))
    cam(t, 'lerp', e0='[cpR, cpA, -55, 2.5, 2.3] call psOffZ', e1='[cpR, cpA, -55, 0.5, 2.3] call psOffZ', fov0=0.16, subj='pCpU select 0', z=1.65, sx=THIRD, sy=THIRD, aim=True)
    record(t, 'c15_day_idf_checkpoint_telephoto', 7, note='long lens down the paved road onto the checkpoint, the bar across it and the van beyond', gate=dict(x=THIRD, y=THIRD, hz=False))
    cam(t, 'orbit', c='cpR', r0=10, r1=9.5, h0=4.2, h1=3.6, a0=40, a1=100, fov0=0.6, subj='pCpU select 0', z=1.5, sx=TWO3, sy=0.5, aim=True)
    record(t, 'c16_day_idf_checkpoint_orbit', 9, note='low arc round the checkpoint from the approach side: the bar across the road, the soldiers at it, the M113 on the grass')
    t.c('pCp call psDelGrp; (group (driver pApc)) call psDelGrp; deleteVehicle pApc; (group (driver pVan)) call psDelGrp; deleteVehicle pVan; { deleteVehicle _x } forEach [pBar, pSb1, pSb2]')
    # ---- a garrison post on open ground in the olive plain: sandbags, an M113, soldiers on watch
    time_of_day(t, 12, 10)
    t.c('player setPos [11200, 9800, 0]')
    t.c('gS1 = [["LoBo_SandbagX4", "MAP_Sandbag_W11"] call psPick, [10617, 9126, 0], 250] call psProp; gS2 = [["MAP_Sandbag_W12", "LoBo_SandbagX2"] call psPick, [10625, 9128, 0], 200] call psProp; gZ = ["LoBo_Zelda", [10631, 9114, 0], 230, west, "LoBoGolaniWB", true, false] call psVehC; (driver gZ) disableAI "MOVE"')
    t.c('gG = [west, ["LoBoGolaniWMG", "LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWB", "LoBoGolaniWLAW"], [10620, 9120, 0], 250, 0.7] call psInf; gG setBehaviour "SAFE"; gU = units gG; [gU select 0, [10616, 9124, 0], 250, "MIDDLE"] call psPost; [gU select 1, [10621, 9121, 0], 230, "UP"] call psPost; [gU select 2, [10624, 9126, 0], 210, "UP"] call psPost; [gU select 3, [10613, 9116, 0], 270, "UP"] call psPost; [gU select 4, [10627, 9120, 0], 160, "UP"] call psPost')
    wait_s(t, 2)
    cam(t, 'lerp', e0=free(10598, 9106, 3.5), e1=free(10604, 9111, 2.9), fov0=0.4, subj='gU select 0', z=1.2, sx=THIRD, sy=0.45, aim=True)
    record(t, 'c17_day_idf_garrison_post_push', 8, note='slow push on an IDF garrison post in the olive plain, a machine gunner on watch behind sandbags', gate=dict(x=THIRD, y=0.45, hz=False))
    footer(t)
    write(t)


def occ2():
    t = T('promo_occ2', 'Lebanon80 promo: IDF armour and a convoy on the Ghajar east road, helicopters over the olive plain, the convoy at night')
    header(t, 15, 30, overcast=0.25, fog=0.0, view=2500, player_pos=[10300, 8600, 0])
    t.c(NO_TRAFFIC)
    # ---- armour west along the Ghajar east road (y 11098) toward the village: each tank its own crew and doMove
    t.c('player setPos [14700, 11600, 0]')
    clear_civs(t, [14350, 11098, 0], 600)
    t.c('aT1 = ["LoBoMerkava2", [14540, 11098, 0], 270, west, "LoBoGolaniWB", true, true] call psVehC; aT2 = ["LoBoMerkava3", [14585, 11098, 0], 270, west, "LoBoGolaniWB", true, true] call psVehC; aT3 = ["LoBo_Magach7A", [14630, 11098, 0], 270, west, "LoBoGolaniWB", true, true] call psVehC')
    t.c('{ [_x, [14150, 11098, 0], "LIMITED", "SAFE"] call psGo } forEach [aT1, aT2, aT3]')
    until(t, '((getPos aT1) select 0) < 14515', 90)
    cam(t, 'lerp', e0=free(14470, 11085, 2.4), fov0=0.3, subj='aT1', z=2.2, sx=TWO3, sy=0.55, aim=True)
    record(t, 'c20_day_idf_armour_roadside', 10, note='Merkava column rolls down the Ghajar road past a roadside camera, the lead tank on the right third', gate=dict(x=TWO3, y=0.55, hz=False))
    cam(t, 'follow', obj='aT2', o0=[18, -20, 12], o1=[14, -6, 9], pa=0.12, hda=0.03, fov0=0.6, subj='aT2', z=2, sx=THIRD, sy=0.6, aim=True)
    record(t, 'c21_day_idf_armour_aerial_follow', 10, note='aerial follow of the armour column on the road into Ghajar')
    cam(t, 'follow', obj='aT1', o0=[3, 22, 1.8], o1=[2.5, 15, 1.7], pa=0.25, hda=0.05, fov0=0.55, subj='aT1', z=2.6, sx=TWO3, sy=0.45, aim=True)
    record(t, 'c22_day_idf_merkava_front_dolly', 7, note='low in front of the lead Merkava as it advances', gate=dict(x=TWO3, y=0.45, hz=False))
    t.c('{ (group (driver _x)) call psDelGrp; deleteVehicle _x } forEach [aT1, aT2, aT3]')
    # ---- a convoy west along the highway in late light: Humvee, M35 with troops, covered truck, M113
    convoy = ('vV1 = ["LoBoHMWV_Mag_Patrol", [14540, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC; vV2 = ["LoBo_M35", [14568, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; '
              'vV3 = ["LoBo_Truck5tcovered", [14596, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; vV4 = ["LoBo_Zelda", [14624, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC; '
              '[vV2, group (driver vV2), ["LoBoGolaniWB", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"]] call psCargo; { [_x, [14150, 11098, 0], "LIMITED", "SAFE"] call psGo } forEach [vV1, vV2, vV3, vV4]')
    time_of_day(t, 16, 5, overcast=0.2)
    t.c(convoy)
    until(t, '((getPos vV1) select 0) < 14520', 60)
    cam(t, 'follow', obj='vV3', o0=[-34, -24, 20], o1=[-30, -2, 16], pa=0.1, hda=0.03, fov0=0.62, subj='vV2', z=2, sx=TWO3, sy=0.6, aim=True)
    record(t, 'c23b_golden_idf_convoy_aerial_side', 10, note='aerial tracking alongside an IDF convoy on the road into Ghajar in late light')
    t.c('{ (group (driver _x)) call psDelGrp; deleteVehicle _x } forEach [vV1, vV2, vV3, vV4]')
    t.c(convoy)
    until(t, '((getPos vV1) select 0) < 14480', 60)
    cam(t, 'lerp', e0=free(14425, 11086, 2.2), fov0=0.25, subj='vV1', z=1.8, sx=TWO3, sy=0.5, aim=True)
    record(t, 'c24_golden_idf_convoy_roadside_pass', 10, note='the convoy comes down the road into Ghajar past a roadside camera, low sun', gate=dict(x=TWO3, y=0.5, hz=False))
    t.c('{ (group (driver _x)) call psDelGrp; deleteVehicle _x } forEach [vV1, vV2, vV3, vV4]')
    # ---- a Bell 212 and a Cobra low over the olive plain, flying west into the low sun (scripted paths)
    time_of_day(t, 16, 20)
    t.a('[] exec "helifly.sqs"')
    t.c('hH = ["LoBoB212", [10850, 9050, 38]] call hfHeli; hC = ["LoBo_AH1", [10870, 9010, 48]] call hfHeli; HF_HOLD = true; [[hH, "wline", [10850, 9050, 38], [9950, 9500, 38], 20, 0], [hC, "wline", [10870, 9010, 48], [9970, 9460, 48], 20, 0.6]] call hfStart; PC_EXT = true')
    cam(t, 'lerp', e0=free(10330, 9330, 1.8), fov0=0.6, subj='hH', z=0, sx=TWO3, sy=0.4, aim=True)
    record(t, 'c25_golden_idf_helicopters_flyover', 9, pre='HF_HOLD = false; HF_T0 = time - 6 + PC_HEAD', note='a Bell 212 and an AH-1 Cobra fly low over the olive plain toward the low sun', gate=dict(x=TWO3, tol=0.12, hz=False))
    t.c('PC_EXT = false; HF_ON = false; HF_QUIT = true; (group (driver hH)) call psDelGrp; deleteVehicle hH; (group (driver hC)) call psDelGrp; deleteVehicle hC')
    # ---- the convoy at night, headlights
    time_of_day(t, 17, 35)
    t.c(convoy)
    until(t, '((getPos vV1) select 0) < 14520', 60)
    cam(t, 'follow', obj='vV2', o0=[-26, -10, 14], o1=[-22, 10, 11], pa=0.1, hda=0.03, fov0=0.62, subj='vV2', z=2, sx=TWO3, sy=0.6, aim=True)
    record(t, 'c23_night_idf_convoy_aerial_headlights', 10, note='aerial alongside the IDF convoy at night, headlights on the road', gate=dict(x=TWO3, y=0.6, hz=False))
    footer(t)
    write(t)


# ======================================================================================== the resistance
def guer1():
    t = T('promo_guer1', 'Lebanon80 promo: a Hizballah squad crosses the olive plain at dawn, a technical on the Ghajar road')
    header(t, 7, 45, overcast=0.2, fog=0.0, view=2500, player_pos=[11420, 11950, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10450, 9250, 0])
    start = [10520, 9180, 0]
    walkers(t, 'gSq', 'east', HEZ_SQUAD, start, 320, ahead(start, 320, 250), spacing=3.2, lateral=0.9, beh='AWARE')
    wait_s(t, 8)
    cam(t, 'lerp', e0=[10501, 9217, 2.2], e1=[10500.5, 9217.5, 2.25], fov0=0.3, subj='gSqU select 0', z=1.65, sx=THIRD, sy=THIRD, aim=True)
    record(t, 'c30_dawn_hezb_squad_olive_grove_pass', 9, note='a Hizballah squad walks across the olive plain past the camera at dawn, the leader on the left third', gate=dict(x=THIRD, y=THIRD, hz=False))
    cam(t, 'follow', obj='gSqU select 3', o0=[-1.2, -5, 2.0], o1=[-1.4, -6, 2.1], pa=0.15, hda=0.04, fov0=0.4, subj='gSqU select 0', z=1.6, sx=TWO3, sy=0.38, aim=True)
    record(t, 'c31_dawn_hezb_squad_behind', 8, note='behind the squad through the trees, the leader ahead on the right third', gate=dict(x=TWO3, y=0.38, hz=False))
    cam(t, 'follow', obj='gSqU select 2', o0=[0, -9, 3], o1=[-6, -45, 28], pa=0.12, hda=0.03, fov0=0.6, fov1=0.7, subj='gSqU select 2', z=1, sx=THIRD, sy=0.62, aim=True)
    record(t, 'c32_dawn_hezb_squad_aerial_reveal', 10, note='rising pull-back over the squad and the olive plain')
    t.c('gSq call psDelGrp')
    # ---- a technical on the Ghajar east road, eastbound, morning
    tech = ('if (!(isNil "tV")) then { (group (driver tV)) call psDelGrp; deleteVehicle tV }; tV = ["LoBo_JeepMG_Hizballah", [14150, 11098, 0], 90, east, "LoBo_HizballahRifle4", true, false] call psVehC; '
            '[tV, group (driver tV), ["LoBo_HizballahRifle1"]] call psCargo; [tV, [14560, 11098, 0], "NORMAL", "CARELESS"] call psGo')
    time_of_day(t, 9, 40)
    t.c('player setPos [14700, 11600, 0]')
    clear_civs(t, [14300, 11098, 0], 600)
    t.c(tech)
    until(t, '((getPos tV) select 0) > 14185', 40)
    cam(t, 'follow', obj='tV', o0=[6, 5, 1.8], o1=[6, -3, 1.8], to=[0, 3.5, 1.3], pa=0.3, hda=0.08, fov0=0.6, subj='tV', z=1.0)
    # the aimed lens put this fast vehicle far below its line here (the logged projection and the picture disagree
    # by several metres of height); framed in the vehicle's own frame instead and checked by eye
    record(t, 'c33_day_hezb_technical_side', 8, note='alongside a Hizballah technical with a mounted gun on the Ghajar road', gate=dict(x='none', hz=False))
    t.c(tech)
    until(t, '((getPos tV) select 0) > 14185', 40)
    cam(t, 'follow', obj='tV', o0=[-3, -12, 3.2], o1=[-2.5, -9, 2.8], to=[1.5, 15, 0.8], pa=0.25, hda=0.06, fov0=0.65, subj='tV', z=1.0)
    record(t, 'c34_day_hezb_technical_chase', 8, note='chasing the technical down the Ghajar road', gate=dict(x='none', hz=False))
    footer(t)
    write(t)


def guer2():
    t = T('promo_guer2', 'Lebanon80 promo: a weapons cache at the edge of Ghajar, a Hizballah file walks the olive plain into the low sun')
    header(t, 15, 40, overcast=0.2, fog=0.0, view=2500, player_pos=[14700, 11600, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [14376, 11031, 0], 400)
    # a cache at the east edge of Ghajar: crates, a technical, fighters around it
    t.c('cB1 = ["MAP_Box_IronC", [14373, 11032, 0], 20] call psProp; cB2 = ["MAP_Crate", [14376, 11034, 0], 70] call psProp; cB3 = ["MAP_Box_IronO", [14371, 11028, 0], 140] call psProp; cB4 = ["MAP_Pallet", [14378, 11029, 0], 10] call psProp; cT = ["LoBo_JeepMG_Hizballah", [14383, 11024, 0], 300] call psVeh')
    t.c('cSq = [east, %s, [14370, 11036, 0], 120, 0.7] call psInf; cU = units cSq; [cU select 0, [14370, 11036, 0], 140, "UP"] call psPost; [cU select 1, [14374, 11028, 0], 30, "MIDDLE"] call psPost; [cU select 2, [14379, 11035, 0], 200, "UP"] call psPost; [cU select 3, [14381, 11027, 0], 300, "UP"] call psPost; [cU select 4, [14368, 11031, 0], 90, "MIDDLE"] call psPost; [cU select 5, [14386, 11031, 0], 250, "UP"] call psPost; cSq setBehaviour "SAFE"' % HEZ_SQUAD)
    wait_s(t, 2)
    cam(t, 'orbit', c=[14376, 11031, 0], r0=10, r1=9, h0=3.0, h1=2.6, a0=100, a1=160, fov0=0.62, subjp=[14374, 11031, 0.8], sx=TWO3, sy=0.6, aim=True)
    record(t, 'c35b_golden_hezb_cache_orbit', 10, note='orbit of a Hizballah weapons cache and technical at the edge of Ghajar, fighters around it', gate=dict(x=TWO3, y=0.6, hz=False))
    cam(t, 'lerp', e0=free(14390, 11046, 2.0), e1=free(14385, 11041, 1.8), fov0=0.5, subjp=[14374, 11031, 0.8], sx=THIRD, sy=0.62, aim=True)
    record(t, 'c36_golden_hezb_cache_push_in', 8, note='push in on the cache, crates and the technical, Ghajar behind', gate=dict(x=THIRD, y=0.62, hz=False))
    t.c('cSq call psDelGrp')
    # a file walking west across the plain, the low sun ahead of them
    time_of_day(t, 16, 5)
    t.c('player setPos [11200, 9800, 0]')
    clear_civs(t, [10450, 9250, 0])
    start = [10520, 9200, 0]
    walkers(t, 'qSq', 'east', HEZ_SQUAD, start, 290, ahead(start, 290, 250), spacing=3.2, lateral=0.9, beh='AWARE')
    wait_s(t, 3)
    cam(t, 'follow', obj='qSqU select 2', o0=[-6.5, 2.5, 1.9], o1=[-6.5, -1, 1.9], pa=0.15, hda=0.04, fov0=0.5, subj='qSqU select 2', z=1.65, sx=TWO3, sy=THIRD, aim=True)
    record(t, 'c37_golden_hezb_file_side_dolly', 9, note='side dolly alongside a Hizballah file walking into the low sun', gate=dict(x=TWO3, y=THIRD, hz=False))
    cam(t, 'follow', obj='qSqU select 0', o0=[1.2, 8, 1.9], o1=[1.0, 6.5, 1.9], pa=0.2, hda=0.05, fov0=0.5, subj='qSqU select 0', z=1.65, sx=THIRD, sy=THIRD, aim=True)
    record(t, 'c38_golden_hezb_file_front_backpedal', 8, note='in front of the squad leader, the file walking toward the camera behind him', gate=dict(x=THIRD, y=THIRD, hz=False))
    cam(t, 'follow', obj='qSqU select 5', o0=[0.8, -3.0, 1.85], o1=[0.9, -3.5, 1.9], pa=0.15, hda=0.04, fov0=0.6, subj='qSqU select 2', z=1.6, sx=THIRD, sy=0.4, aim=True)
    record(t, 'c39_golden_hezb_file_over_shoulder', 8, note='over the shoulder of the last fighter, the file ahead in the golden light', gate=dict(x=THIRD, y=0.4, hz=False))
    footer(t)
    write(t)


def night():
    t = T('promo_night', 'Lebanon80 promo: blue hour in Ghajar, a Hizballah squad crosses the road, a firefight with tracers')
    header(t, 17, 16, overcast=0.15, fog=0.0, view=2000, player_pos=[14800, 11600, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [14350, 11098, 0], 600)
    # an IDF patrol walks west into Ghajar along the lamp-lit east road
    walkers(t, 'nPat', 'west', IDF_PATROL, [14345, 11098, 0], 270, [14170, 11098, 0], spacing=3.5, lateral=0.7, beh='SAFE')
    wait_s(t, 1)
    cam(t, 'follow', obj='nPatU select 2', o0=[7, 2.5, 2.0], o1=[7, -1, 2.0], pa=0.15, hda=0.04, fov0=0.5, subj='nPatU select 2', z=1.65, sx=THIRD, sy=THIRD, aim=True)
    record(t, 'c80_bluehour_idf_patrol_side_lamplit_village', 9, note='side dolly with an IDF patrol walking along the lamp-lit road at the edge of Ghajar, blue hour', gate=dict(x=THIRD, y=THIRD, hz=False))
    cam(t, 'follow', obj='nPatU select 5', o0=[1.0, -3.4, 1.9], o1=[1.2, -4.0, 1.95], pa=0.15, hda=0.04, fov0=0.6, subj='nPatU select 2', z=1.6, sx=THIRD, sy=0.4, aim=True)
    record(t, 'c81_bluehour_idf_patrol_behind_into_village', 8, note='behind the patrol walking under the street lamps', gate=dict(x=THIRD, y=0.4, hz=False))
    t.c('nPat call psDelGrp')
    # a Hizballah squad crosses the road beyond the lamps, north
    walkers(t, 'nSq', 'east', HEZ_SQUAD, [14395, 11076, 0], 0, [14395, 11230, 0], spacing=2.8, lateral=0.8, beh='AWARE')
    wait_s(t, 1)
    cam(t, 'lerp', e0=free(14410, 11092, 1.9), e1=free(14410, 11092.5, 1.9), fov0=0.35, subj='nSqU select 0', z=1.6, sx=THIRD, sy=0.38, aim=True)
    record(t, 'c82_bluehour_hezb_squad_crosses_road', 8, note='a Hizballah squad crosses the road close to the camera at blue hour', gate=dict(x=THIRD, y=0.38, hz=False))
    t.c('nSq call psDelGrp')
    # the firefight: tracers across the olive plain, the lines 80 m apart; nobody is hurt (allowDammage false)
    t.c('setDate [1985, 6, 21, 17, 22]')
    t.c('player setPos [11000, 9800, 0]')
    clear_civs(t, [10450, 9290, 0])
    t.c('gI = [west, ["LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWB"], [10450, 9250, 0], 0, 0.7] call psInf; gH = [east, ["LoBo_HizballahRifle4", "LoBo_HizballahMG1", "LoBo_HizballahRifle1", "LoBo_HizballahMG1", "LoBo_HizballahRifle4", "LoBo_HizballahLeader"], [10450, 9330, 0], 180, 0.7] call psInf; gIU = units gI; gHU = units gH')
    t.c('_i = 0; { _x allowDammage false; _x setPos [10430 + 8 * _i, 9250 + 3 * (_i mod 2), 0]; _x setDir 0; _x setUnitPos "MIDDLE"; _x disableAI "MOVE"; _i = _i + 1 } forEach gIU; _i = 0; { _x allowDammage false; _x setPos [10432 + 8 * _i, 9330 - 3 * (_i mod 2), 0]; _x setDir 180; _x setUnitPos "MIDDLE"; _x disableAI "MOVE"; _i = _i + 1 } forEach gHU')
    t.c('{ _x setCombatMode "RED"; _x setBehaviour "COMBAT" } forEach [gI, gH]; { _u = _x; { _u reveal _x } forEach gHU } forEach gIU; { _u = _x; { _u reveal _x } forEach gIU } forEach gHU')
    wait_s(t, 5)
    cam(t, 'lerp', e0='[(getPos (gIU select 1) select 0) + 1.2, (getPos (gIU select 1) select 1) - 2.6, 1.5]', e1='[(getPos (gIU select 1) select 0) + 1.0, (getPos (gIU select 1) select 1) - 2.4, 1.5]', fov0=0.45, subj='gIU select 1', z=1.0, sx=THIRD, sy=0.55, subjp2=[10452, 9330, 1], aim=True)
    record(t, 'c83_bluehour_firefight_behind_idf_machine_gun', 12, note='blue hour, behind an IDF machine gunner firing across the olive plain, tracers going out and coming back', gate=dict(x=THIRD, hz=False))
    cam(t, 'lerp', e0='[(getPos (gHU select 1) select 0) - 1.2, (getPos (gHU select 1) select 1) + 2.6, 1.5]', e1='[(getPos (gHU select 1) select 0) - 1.0, (getPos (gHU select 1) select 1) + 2.4, 1.5]', fov0=0.45, subj='gHU select 1', z=1.0, sx=THIRD, sy=0.55, subjp2=[10452, 9250, 1], aim=True)
    record(t, 'c84_bluehour_firefight_behind_hezb_machine_gun', 10, note='behind a Hizballah machine gunner firing into the dusk, the IDF tracers answering', gate=dict(x=THIRD, hz=False))
    footer(t)
    write(t)


# ======================================================================================== the ambush
AMB_FIGHTERS = '["LoBo_HizballahLeader", "LoBo_Hizballah_RPG", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG3", "LoBo_HizballahRifle4", "LoBo_HizballahRifle1", "LoBo_Hizballah_RPG"]'
# the fighters lie in the meadow 12 to 16 m north of the road (y 11098), facing south (the meadow rises away
# from the road, so further back the road drops out of sight); the RPG gunners (1, 3, 6) at the east end
AMB_SPOTS = [(14450, 11112), (14480, 11114), (14457, 11115), (14488, 11112), (14465, 11112), (14472, 11115), (14495, 11115)]


def amb_stage(t, qrf=True, leadtoo=False, blow=2.5):
    """the convoy (Humvee, an empty M35 the RPGs aim at, an M35 with six troops, an M113) driving west on the
    Ghajar east road at NORMAL speed, and the hidden fighters; ambush2.sqs opens fire when the Humvee is 30 m
    from x 14470, about 4 s after the staging (the settle pauses the world, so the recorder starts with them)"""
    t.c('xV1 = ["LoBoHMWV_Mag_Patrol", [14545, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC; xV2 = ["LoBo_M35", [14567, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; xV3 = ["LoBo_M35", [14589, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; xV4 = ["LoBo_Zelda", [14611, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC')
    t.c('[xV3, group (driver xV3), ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWLAW", "LoBoGolaniWB"]] call psCargo; { if (_x != driver xV3) then { _x allowDammage false } } forEach (crew xV3); xQ = (crew xV3) - [driver xV3]; AM_DISMOUNT = []')
    t.c('{ [_x, [14150, 11098, 0], "NORMAL", "SAFE"] call psGo; _x setVelocity [-6 * 1, 0, 0] } forEach [xV1, xV2, xV3, xV4]')
    t.c('xA = [east, %s, [14470, 11112, 0], 180, 0.8] call psInf; xA setCombatMode "BLUE"; xA setBehaviour "STEALTH"; xAU = units xA; { _x allowDammage false } forEach xAU' % AMB_FIGHTERS)
    for j, (x, y) in enumerate(AMB_SPOTS):
        t.c('_u = xAU select %d; _u setPos [%d, %d, 0]; _u setDir 180; _u setUnitPos "DOWN"; _u disableAI "MOVE"' % (j, x, y))
    t.c('AM_LEAD = xV1; AM_PT = [14470, 11098, 0]; AM_R = 30; AM_GRP = xA; AM_VEHS = [xV1, xV2, xV3, xV4]; AM_TRUCK = xV2; AM_RPGS = [xAU select 1, xAU select 3, xAU select 6]; AM_RISE = xAU; AM_BLOW = %g; AM_LEADTOO = %s; AM_QRF = %s'
        % (blow, 'true' if leadtoo else 'false', 'xV3' if qrf else 'objNull'))
    t.a('[] exec "ambush2.sqs"')


# one pass per take: a fresh mission each time (in one mission the later convoys stalled behind the first
# passes' wrecks and bodies), golden hour 16:10 to 16:30, blue hour 17:18 to 17:24
AMB_PASSES = {
    'g1': ((16, 10), 'c40_golden_ambush_wide_over_fighters', 12,
           dict(kind='lerp', e0=free(14466, 11124, 4.0), e1=free(14467, 11123.5, 3.6), fov0=0.6, subj='xV2', z=1.5, sx=THIRD, sy=0.62, subj2='xAU select 4', z2=0.4, aim=True),
           dict(x=THIRD, tol=0.1, hz=False), dict(),
           'wide over the backs of the fighters lying in the grass as the convoy rolls into the ambush; they rise and fire, the truck is hit'),
    'g2': ((16, 13), 'c41_golden_ambush_roadside_truck_hit', 11,
           dict(kind='lerp', e0=free(14505, 11086, 1.6), e1=free(14505, 11086.5, 1.55), fov0=0.5, subj='xV2', z=1.6, sx=TWO3, sy=0.62, aim=True),
           dict(x=TWO3, y=0.62, hz=False), dict(qrf=False),
           'low across the road 12 m from the truck as the RPGs open up and it blows, fireball and black smoke'),
    'g3': ((16, 16), 'c42_golden_ambush_rpg_gunner_close', 11,
           dict(kind='lerp', e0=free(14478.6, 11116.4, 1.7, 0.5), e1=free(14478.4, 11116.7, 1.75, 0.5), fov0=0.55, subj='xV2', z=1.5, sx=TWO3, sy=0.58, subj2='xAU select 1', z2=1.0, aim=True),
           dict(x=TWO3, tol=0.1, hz=False), dict(qrf=False),
           'over the shoulder of an RPG gunner as he rises from the grass and fires on the truck'),
    'g4': ((16, 19), 'c47_golden_ambush_fighters_rise_from_cover', 10,
           dict(kind='lerp', e0=free(14440, 11106, 1.5, 0.6), e1=free(14440.6, 11106.4, 1.5, 0.6), fov0=0.42, subj='xAU select 2', z=0.8, sx=THIRD, sy=0.5, aim=True),
           dict(x=THIRD, tol=0.1, hz=False), dict(),
           'medium shot along the ambush line from its flank: the fighters rise from the grass and open fire'),
    'g5': ((16, 22), 'c46_golden_ambush_idf_dismount_return_fire', 11,
           dict(kind='lerp', e0=free(14552, 11085, 1.8, 0.8), e1=free(14551, 11085.5, 1.8, 0.8), fov0=0.55, subj='xV3', z=1.5, sx=TWO3, sy=0.5, aim=True),
           dict(x=TWO3, tol=0.12, hz=False), dict(),
           'the troop truck halts, the IDF soldiers jump down, kneel at the roadside and return fire'),
    'g6': ((16, 25), 'c48_golden_ambush_truck_hit_high_angle', 12,
           dict(kind='lerp', e0=free(14484, 11060, 24), e1=free(14478, 11062, 22), fov0=0.55, subj='xV2', z=1.5, sx=TWO3, sy=0.62, subjp2=[14470, 11113, 0.5], aim=True),
           dict(x=TWO3, y=0.62), dict(qrf=False, leadtoo=True),
           'high from across the road: the convoy, the fighters rising in the grass, the truck and the Humvee hit'),
    'n1': ((17, 18), 'c43_bluehour_ambush_tracers_close', 12,
           dict(kind='lerp', e0=free(14486, 11091, 1.8), e1=free(14485, 11091.4, 1.8), fov0=0.55, subjp=[14470, 11113, 1.0], sx=THIRD, sy=0.5, subj2='xV2', z2=1.5, aim=True),
           dict(x=THIRD, hz=False), dict(),
           'blue hour, at the roadside: tracers both ways across the road close to the lens, the truck hit'),
    'n3': ((17, 28), 'c43b_bluehour_ambush_behind_fighters_tracers', 12,
           dict(kind='lerp', e0=free(14477, 11121, 1.9, 0.6), e1=free(14476.5, 11120.6, 1.9, 0.6), fov0=0.5, subj='xV2', z=1.5, sx=TWO3, sy=0.55, subj2='xAU select 5', z2=0.8, aim=True),
           dict(x=TWO3, tol=0.1, hz=False), dict(),
           'after sunset, close behind the fighters as they rise and fire on the convoy: muzzle flashes, tracers going out, the truck blows'),
    'n2': ((17, 22), 'c44_bluehour_ambush_low_road', 11,
           dict(kind='lerp', e0=free(14432, 11096, 1.3), e1=free(14433, 11096, 1.3), fov0=0.5, subj='xV1', z=1.2, sx=TWO3, sy=0.55, aim=True),
           dict(x=TWO3, tol=0.1, hz=False), dict(leadtoo=True),
           'low on the road ahead of the convoy: headlights, the ambush lights them up, the lead Humvee is hit'),
}


def amb_pass(tag):
    (hh, mm), name, secs, camspec, gate, stage, note = AMB_PASSES[tag]
    t = T('promo_amb_%s' % tag, 'Lebanon80 promo: Hizballah ambush an IDF convoy on the Ghajar east road, pass %s' % tag)
    header(t, hh, mm, overcast=0.2, fog=0.0, view=2500, player_pos=[13600, 12100, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [14470, 11098, 0], 1000)
    amb_stage(t, **stage)
    spec = dict(camspec)
    kind = spec.pop('kind')
    cam(t, kind, **spec)
    record(t, name, secs, note=note, gate=gate)
    if tag == 'g1':
        # the burning wreck: push in once it has burned for a few seconds
        wait_s(t, 5)
        cam(t, 'lerp', e0=free(14522, 11084, 2.6), e1=free(14528, 11087, 2.2), fov0=0.5, subj='xV2', z=1.5, sx=TWO3, sy=0.55, aim=True)
        record(t, 'c45_golden_ambush_burning_wreck_smoke', 8, note='push in on the burning truck, black smoke rising, the convoy halted behind it', gate=dict(x=TWO3, y=0.55, hz=False))
    if tag == 'n1':
        wait_s(t, 2)
        cam(t, 'lerp', e0='[(getPos xV3 select 0) - 3, 11085, 1.7]', e1='[(getPos xV3 select 0) - 2, 11085.5, 1.7]', fov0=0.55, subj='xV2', z=1.6, sx=THIRD, sy=0.55, aim=True)
        record(t, 'c49_bluehour_ambush_burning_truck_return_fire', 10, note='behind the dismounted IDF soldiers returning fire, the burning truck lighting the road', gate=dict(x=THIRD, hz=False))
    footer(t)
    write(t)


def amb_g1(): amb_pass('g1')
def amb_g2(): amb_pass('g2')
def amb_g3(): amb_pass('g3')
def amb_g4(): amb_pass('g4')
def amb_g5(): amb_pass('g5')
def amb_g6(): amb_pass('g6')
def amb_n1(): amb_pass('n1')
def amb_n2(): amb_pass('n2')
def amb_n3(): amb_pass('n3')


# ======================================================================================== first person, journal
def fp():
    t = T('promo_fp', 'Lebanon80 promo: first-person gameplay with a Hizballah squad, a firefight, the field journal')
    header(t, 10, 20, overcast=0.2, fog=0.0, view=2000, player_pos=[10520, 9180, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10450, 9250, 0])
    t.c('player setDir 290; PC_RUN = false; PC_CAM cameraEffect ["terminate", "back"]; player selectWeapon (primaryWeapon player)')
    wait_s(t, 5)
    t.c('fSq = [east, ["LoBo_HizballahRifle4", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG", "LoBo_HizballahRifle1"], [10528, 9176, 0], 320, 0.7] call psInf; fSq setBehaviour "AWARE"; _i = 0; { _x setPos ([[10520, 9180, 0], 320, 4 + 3 * (random 3), -8 + 5 * _i] call psOff); _i = _i + 1 } forEach (units fSq); [fSq, [10350, 9340, 0], 320, 4, 6, PS_WALKRATE] call psSteer')
    wait_s(t, 1)
    t.c('hint ""')
    t.a('triKeyDown 26')
    record(t, 'c70_day_fp_advance_with_squad', 8, note='first person: advancing through the olive grove with a Hizballah squad', gate=dict(x='none', hz=False))
    t.a('triKeyUp 26')
    t.c('fSq call psDelGrp')
    # a firefight: an IDF section dug in 90 m north-west; nobody is hurt (allowDammage false), the fire is real
    t.c('fI = [west, %s, [10460, 9262, 0], 140, 0.7] call psInf; { _x allowDammage false; _x setUnitPos "MIDDLE"; _x disableAI "MOVE" } forEach (units fI); fI setCombatMode "RED"; fI setBehaviour "COMBAT"' % IDF_PATROL)
    t.c('fH = [east, ["LoBo_HizballahMG1", "LoBo_HizballahRifle4", "LoBo_HizballahRifle1"], [10515, 9185, 0], 320, 0.7] call psInf; { _x allowDammage false; _x setUnitPos "MIDDLE"; _x disableAI "MOVE" } forEach (units fH); (units fH select 0) setPos [10512, 9184, 0]; (units fH select 1) setPos [10526, 9180, 0]; (units fH select 2) setPos [10518, 9190, 0]; fH setCombatMode "RED"; fH setBehaviour "COMBAT"')
    t.c('player setCaptive false; player allowDammage false; player setPos [10519, 9182, 0]; player setDir ([[10519, 9182, 0], [10460, 9262, 0]] call psDir); { _u = _x; { _u reveal _x } forEach (units fI) } forEach ((units fH) + [player]); { _u = _x; { _u reveal _x } forEach ((units fH) + [player]) } forEach (units fI)')
    wait_s(t, 6)
    t.c('hint ""')
    record(t, 'c71_day_fp_firefight_olive_grove', 9, pre='FP_SECS = 8.5; [] exec "fpfire.sqs"', note='first person: firing on an IDF section across the olive grove, fighters beside him', gate=dict(x='none', hz=False))
    t.c('fI call psDelGrp; fH call psDelGrp; player setCaptive true')
    # the field journal and the Resistance Dossier on the map screen (title: the template's own, "Guerrilla")
    t.c('prMap = triOpenMap; logInfo format ["PROMO map %1", prMap]')
    wait_s(t, 3)
    t.c('hint ""; JN_SECS = 3.2; JN_PAGES = ["Main", "GM_DISPATCH", "GM_PEOPLE", "GM_PLACES", "GM_CHRONICLES", "GM_REFERENCE"]')
    record(t, 'c72_fieldjournal_pages', 19, pre='[] exec "journal.sqs"', note='the field journal and Resistance Dossier pages on the map screen', gate=dict(x='none', hz=False))
    footer(t)
    write(t)


# ======================================================================================== Dedushka
KAIJU_MISSION = 'experiments/lebanon80-promo/PromoKaiju.Lebanon80'
KAIJU_MODS = '@LoBo;@kj_dedushka'


def kaiju_spawn(t, x, y, h, noshake=True):
    """Dedushka (UDDedushka) on the Resistance side (his config side), Resistance made the IDF's friend and
    Hizballah's enemy: the IDF never engages him, Hizballah fires on him. Unkillable (no combat feature),
    does only what he is told (KJ_brainAuto false). No camera tremor (PROMO_NOSHAKE) unless noshake=False."""
    t.c('kjCtr = createCenter resistance; resistance setFriend [west, 1]; west setFriend [resistance, 1]; resistance setFriend [east, 0]; east setFriend [resistance, 0]; kjG = createGroup resistance; kaiju = objNull; "UDDedushka" createUnit [[%g, %g, 0], kjG, "kaiju = this", 0.5, "PRIVATE"]' % (x, y))
    t.c('kaiju setDir %g; kaiju setPos [%g, %g, 0]; logInfo format ["PROMO kaiju %%1 side %%2 at %%3", typeOf kaiju, side kaiju, getPos kaiju]' % (h, x, y))
    if noshake:
        # no camera tremor on film (sync_kaiju_patch.py: the shoot copy of kj\roar\shake.sqs exits)
        t.c('PROMO_NOSHAKE = true')
    t.c('KJ_brainAuto = false; KJB_WAKE_ROAR = false; KJ_combatFile = false; KJ_ready = false; KJR_FLICKER_R = 0; KJF_BREATH = 0.03; [kaiju, "idle"] exec "kj\\start.sqs"')
    until(t, 'KJ_ready', 120)
    t.log('kaiju ready %1 state %2 native %3', 'KJ_ready', 'KJ_state', 'KJ_native')


def order(t, word, arg=None):
    t.c('KJ_brainOrder = ["%s"%s]' % (word, (', ' + arg) if arg else ''))


def kaiju_escort():
    t = T('promo_kaiju_escort', 'Lebanon80 promo: Dedushka walks the coast highway with an IDF escort, an IDF roadblock lets him through')
    header(t, 16, 5, overcast=0.2, fog=0.0, view=3000, player_pos=[10300, 8700, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10400, 8307, 0], 1200)
    kaiju_spawn(t, 9850, 8307, 90)
    # the escort: two Merkavas at his flanks, an M113 behind, a Golani squad well out on his flanks
    t.c('eT1 = ["LoBoMerkava2", [9828, 8281, 0], 90, west, "LoBoGolaniWB", true, true] call psVehC; eT2 = ["LoBoMerkava3", [9820, 8333, 0], 90, west, "LoBoGolaniWB", true, true] call psVehC; eA = ["LoBo_Zelda", [9790, 8307, 0], 90, west, "LoBoGolaniWB", true, false] call psVehC; { (group (driver _x)) setBehaviour "SAFE"; (group (driver _x)) setSpeedMode "NORMAL" } forEach [eT1, eT2, eA]')
    t.c('eSq = [west, %s, [9830, 8345, 0], 90, 0.7] call psInf; eSq setBehaviour "AWARE"; eSq setSpeedMode "NORMAL"; eU = units eSq' % IDF_PATROL)
    t.c('ES_LIST = [[eT1, -12, -34, true], [eT2, -6, 34, true], [eA, 60, 0, true], [eU select 0, 15, -40, false], [eU select 1, 22, -43, false], [eU select 2, 29, -39, false], [eU select 3, 18, 40, false], [eU select 4, 25, 43, false], [eU select 5, 32, 39, false]]; ES_ON = true; [] exec "escort.sqs"')
    order(t, 'drawn', '[11100, 8307, 0]')
    wait_s(t, 10)
    t.log('kaiju walking %1 at %2', 'KJ_state', 'getPos kaiju')
    # walking straight at a low camera, the tanks at his flanks: symmetric, deliberate
    cam(t, 'lerp', e0='[((getPos kaiju) select 0) + 150, 8301, 1.8]', e1='[((getPos kaiju) select 0) + 149, 8301, 1.8]', fov0=0.42, fov1=0.5, subj='kaiju', z=24, sx=0.5, sy=0.42, subj2='eT1', z2=2, aim=True, fa=0.25, fb=0.03)
    record(t, 'c50_kaiju_golden_advance_with_armour_low_front', 10, note='Dedushka advances down the highway toward a low camera, Merkavas at his flanks', sym=True, gate=dict(x=0.5, y=0.42, hz=False))
    cam(t, 'follow', obj='kaiju', o0=[95, -25, 32], o1=[85, 15, 28], pa=0.1, hda=0.03, fov0=0.7, subj='kaiju', z=22, sx=THIRD, sy=0.5, subj2='eT1', z2=2, aim=True, fa=0.25, fb=0.03)
    record(t, 'c51_kaiju_golden_escort_aerial_scale', 10, note='aerial alongside: his scale over the IDF tanks escorting him')
    # IDF soldiers posted at the roadside watch him pass, unharmed
    t.c('wX = ((getPos kaiju) select 0) + 110; wSq = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWB", "LoBoGolaniWMG"], [wX, 8338, 0], 200, 0.7] call psInf; wU = units wSq; [wU select 0, [wX, 8336, 0], 210, "UP"] call psPost; [wU select 1, [wX + 4, 8338, 0], 200, "UP"] call psPost; [wU select 2, [wX - 4, 8339, 0], 215, "UP"] call psPost; [wU select 3, [wX + 2, 8342, 0], 205, "MIDDLE"] call psPost; wSq setBehaviour "SAFE"')
    until(t, '((getPos kaiju) select 0) > wX - 75', 90)
    cam(t, 'lerp', e0='[wX + 3, 8352, 2.0]', e1='[wX + 1, 8351, 1.9]', fov0=0.62, subj='kaiju', z=26, sx=TWO3, sy=0.38, subj2='wU select 0', z2=1.6, aim=True, fa=0.25, fb=0.03)
    record(t, 'c52_kaiju_golden_idf_soldiers_watch_him_pass', 10, note='behind IDF soldiers at the roadside as Dedushka walks past them', gate=dict(x=TWO3, y=0.38, tol=0.1, hz=False))
    cam(t, 'lerp', e0='[((getPos kaiju) select 0) + 45, 8292, 0.5]', fov0=0.5, subj='kaiju', z=4, sx=THIRD, sy=0.6, aim=True, fa=0.25, fb=0.03)
    record(t, 'c53_kaiju_golden_legs_footfall_low', 8, note='teaser: ground-level view of his legs and footfalls passing', gate=dict(x=THIRD, y=0.6, tol=0.1, hz=False))
    t.c('wSq call psDelGrp')
    # the IDF roadblock across the coast highway at x 10900, laid out from the road net (psRoadHeading): the bar
    # across the track (its setDir is the road heading), sandbags on both shoulders running along it, the M113
    # pulled off on the north shoulder covering the approach, two soldiers at the bar and one at the sandbags.
    # rbA faces the oncoming kaiju (west); the bar's post is at its south end (its local +x).
    t.c('rbRH = [10900, 8307, 0] call psRoadHeading; rbR = [(rbRH select 1) select 0, (rbRH select 1) select 1, 0]; rbH = rbRH select 0; rbA = rbH + 180; RB_ARM = 4.6; rbP = [rbR, rbH, 0, RB_ARM] call psOff')
    t.c('rBar = ["MAP_Barrier", rbR, rbH] call psProp; rSb1 = ["LoBo_SandbagX4", [rbR, rbA, -1.5, 7.5] call psOff, rbH - 90] call psProp; rSb2 = ["LoBo_SandbagX4", [rbR, rbA, -1.5, -7.5] call psOff, rbH - 90] call psProp; rApc = ["LoBo_Zelda", [rbR, rbA, -10, -12] call psOff, rbA + 35, west, "LoBoGolaniWB", true, false] call psVehC; (driver rApc) disableAI "MOVE"; logInfo format ["PROMO kaiju roadblock at %1 heading %2 bar dir %3", rbR, rbH, getDir rBar]')
    t.c('rSq = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWB"], rbR, rbA, 0.7] call psInf; rU = units rSq; [rU select 0, [rbR, rbA, 1.2, 1.5] call psOff, rbA, "UP"] call psPost; [rU select 1, [rbR, rbA, 1.2, -1.8] call psOff, rbA + 8, "UP"] call psPost; [rU select 2, [rbR, rbA, -0.5, -8.2] call psOff, rbA - 10, "MIDDLE"] call psPost; rSq setBehaviour "SAFE"; RB_STEP1 = [rbR, rbA, 0, 9.5] call psOff; RB_STEP2 = [rbR, rbA, -2, -10.5] call psOff')
    t.a('triSimFrames 3')
    t.c('PC_RUN = false; PC_CAM camSetPos [(rbR select 0), (rbR select 1) - 0.5, 34]; PC_CAM camSetDir 0; PC_CAM camSetDive -89; PC_CAM camSetFov 0.7; PC_CAM camCommit 0')
    settle(t)
    t.a('triScreenshot "top_kaiju_roadblock"')
    # a "drawn" order stops him well short of its point: aim it past the roadblock
    order(t, 'drawn', '[11300, 8307, 0]')
    until(t, '((getPos kaiju) select 0) > 10480', 120)
    # he walks slower than the shot needs: bring him to 430 m from the roadblock camera (out of its view)
    t.c('if (((getPos kaiju) select 0) < 10480) then { kaiju setPos [10480, 8307, 0] }')
    wait_s(t, 1)
    cam(t, 'lerp', e0='[rbR, rbA, -30, -4, 1.9] call psOffZ', e1='[rbR, rbA, -28, -4, 1.85] call psOffZ', fov0=0.2, fov1=0.24, subj='kaiju', z=26, sx=TWO3, sy=0.6, subj2='rU select 0', z2=1.6, aim=True, fa=0.25, fb=0.03)
    record(t, 'c54_kaiju_golden_reveal_behind_idf_roadblock', 10, pre='RB_DELAY = 4 + PC_HEAD; [] exec "rbopen.sqs"',
           note='behind the IDF roadblock across the highway as Dedushka walks out of the low sun toward it; the soldiers swing the bar open and step off the road for him')
    t.c('ES_ON = false')
    # the bar stands open for c55 (it swung open in c54; a retake that skips c54 opens it here)
    t.c('if ((getDir rBar) < rbH + 60) then { rBar setDir (rbH + 80); rBar setPos [(rbP select 0) - RB_ARM * (cos (rbH + 80)), (rbP select 1) + RB_ARM * (sin (rbH + 80)), 0]; { _x setPos ([rbR, rbA, 0, 9.5] call psOff) } forEach [rU select 0]; (rU select 1) setPos RB_STEP2 }')
    t.c('KJ_brainOrder = ["idle"]')
    wait_s(t, 1)
    t.c('kaiju setPos [10818, 8307, 0]; kaiju setDir 90; RB_STOPX = [rbR, rbA, 60, 0] call psOff; RB_STOPX = RB_STOPX select 0')
    wait_s(t, 1)
    cam(t, 'lerp', e0='[rbR, rbA, -16, 6, 2.0] call psOffZ', e1='[rbR, rbA, -15, 5.6, 1.9] call psOffZ', fov0=0.42, subj='kaiju', z=32, sx=THIRD, sy=THIRD, subj2='rU select 0', z2=1.6, aim=True, fa=0.25, fb=0.03)
    record(t, 'c55_kaiju_golden_roar_over_roadblock', 13, pre='[] exec "rbroar.sqs"', note='Dedushka walks up to the open roadblock and roars over it, the IDF soldiers standing aside on the shoulders', gate=dict(x=THIRD, y=THIRD, tol=0.1, hz=False))
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)


def probe1():
    """Pipeline probe: 1440p window, detail pin and settles, the 15 head frames, per-frame camera log and the
    thirds aim, walking patrols (old per-unit doMove against one-man groups ordered with move), journal title."""
    t = T('promo_probe1', 'Lebanon80 promo: pipeline probe (1440p, pin, settle, head, walk, aim, journal title)')
    header(t, 9, 50, overcast=0.2, fog=0.0, view=2500, player_pos=[14700, 11500, 0])
    t.c(NO_TRAFFIC)
    # A: one group, every man his own doMove (the first shoot's way); B: one group per man, group move
    t.c('wA = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG"], [14480, 11122, 0], 270, 0.7] call psInf; wA setBehaviour "SAFE"; wA setSpeedMode "LIMITED"; { _x setSpeedMode "LIMITED" } forEach (units wA); [wA, [14300, 11122, 0], 270, 3, 1.0] call psFile')
    t.c('wB = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"], [14480, 11098, 0], 270, 0.7] call psInfSolo; wBU = wB call psUnits; [wB, [14300, 11098, 0], 270, 3, 1.0, "LIMITED", "SAFE"] call psWalk')
    t.c('wP0 = []; { wP0 set [count wP0, getPos _x] } forEach ((units wA) + wBU)')
    wait_s(t, 8)
    t.c('_i = 0; { _p = wP0 select _i; logInfo format ["PROMO walk %1 %2 speed %3 km/h, moved %4 m in 8 s, %5 m/s", if (_i < 3) then {"A"} else {"B"}, _i, speed _x, [_p, getPos _x] call psDist2, ([_p, getPos _x] call psDist2) / 8]; _i = _i + 1 } forEach ((units wA) + wBU)')
    t.c('wA call psDelGrp')
    cam(t, 'lerp', e0=free(14352, 11093, 1.35), e1=free(14350, 11092, 1.4), fov0=0.42, subj='wBU select 1', z=1.6, sx=THIRD, sy=0.4, aim=True)
    record(t, 'p01_day_walk_probe_front', 6, note='probe: one-man-group patrol walking at a low camera, aimed on the left third')
    t.c('_i = 0; { _p = wP0 select (_i + 3); logInfo format ["PROMO walk B %1 speed %2 km/h, moved %3 m", _i, speed _x, [_p, getPos _x] call psDist2]; _i = _i + 1 } forEach wBU')
    t.c('wB call psDelGrps')
    # the journal title on the map screen
    t.c('prMap = triOpenMap; logInfo format ["PROMO map %1", prMap]')
    wait_s(t, 3)
    t.c('_r = triBriefingSwitch "Main"; logInfo format ["PROMO journal Main %1", _r]')
    wait_s(t, 1)
    t.a('triScreenshot "probe_journal_contents"')
    record(t, 'p02_journal_title_probe', 3, note='probe: the Contents page title', gate=dict(x='none', hz=False))
    footer(t)
    write(t)


def walkprobe():
    """Walking: which order makes a man walk (about 1.4 m/s)? Six variants on parallel lines in the meadow
    east of Ghajar, speeds logged after 4, 8 and 12 s. No recording."""
    t = T('promo_walkprobe', 'Lebanon80 promo: walking-order probe (no recording)')
    header(t, 9, 50, overcast=0.2, fog=0.0, view=1500, player_pos=[14700, 11500, 0])
    t.c(NO_TRAFFIC)
    C2 = '["LoBoGolaniWBo", "LoBoGolaniWB"]'
    C4 = '["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"]'
    # V1 one-man groups, doMove, SAFE LIMITED FILE
    t.c('v1 = [west, %s, [14480, 11060, 0], 270, 0.7] call psInfSolo; { _x setBehaviour "SAFE"; _x setSpeedMode "LIMITED"; _x setFormation "FILE"; (leader _x) doMove ([[getPos (leader _x) select 0, getPos (leader _x) select 1, 0], 270, 120, 0] call psOff) } forEach v1' % C2)
    # V2 one-man groups, waypoint LIMITED SAFE
    t.c('v2 = [west, %s, [14480, 11075, 0], 270, 0.7] call psInfSolo; { _x setBehaviour "SAFE"; _x setSpeedMode "LIMITED"; _w = _x addWaypoint [[(getPos (leader _x) select 0) - 120, getPos (leader _x) select 1, 0], 0]; _w setWaypointSpeed "LIMITED"; _w setWaypointBehaviour "SAFE"; _w setWaypointType "MOVE"; logInfo format ["PROMO wp %%1", _w] } forEach v2' % C2)
    # V3 a four-man group, leader doMove only (scout 2's A)
    t.c('v3 = [west, %s, [14480, 11090, 0], 270, 0.7] call psInf; v3 setFormation "FILE"; v3 setBehaviour "SAFE"; v3 setSpeedMode "LIMITED"; (leader v3) doMove [14360, 11090, 0]' % C4)
    # V4 a four-man group, waypoint
    t.c('v4 = [west, %s, [14480, 11108, 0], 270, 0.7] call psInf; v4 setFormation "FILE"; v4 setBehaviour "SAFE"; v4 setSpeedMode "LIMITED"; _w = v4 addWaypoint [[14360, 11108, 0], 0]; _w setWaypointSpeed "LIMITED"; _w setWaypointBehaviour "SAFE"; _w setWaypointType "MOVE"' % C4)
    # V5 one-man groups, doMove first, then the speed mode
    t.c('v5 = [west, %s, [14480, 11125, 0], 270, 0.7] call psInfSolo; { (leader _x) doMove ([[getPos (leader _x) select 0, getPos (leader _x) select 1, 0], 270, 120, 0] call psOff); _x setBehaviour "SAFE"; _x setSpeedMode "LIMITED" } forEach v5' % C2)
    # V6 one-man groups, CARELESS LIMITED doMove
    t.c('v6 = [west, %s, [14480, 11140, 0], 270, 0.7] call psInfSolo; { _x setBehaviour "CARELESS"; _x setSpeedMode "LIMITED"; (leader _x) doMove ([[getPos (leader _x) select 0, getPos (leader _x) select 1, 0], 270, 120, 0] call psOff) } forEach v6' % C2)
    # V7 a four-man group, every man doMove, then each man's group speed (the first shoot's way), AWARE
    t.c('v7 = [west, %s, [14480, 11155, 0], 270, 0.7] call psInf; v7 setBehaviour "AWARE"; v7 setSpeedMode "LIMITED"; [v7, [14360, 11155, 0], 270, 3, 1.0] call psFile' % C4)
    t.c('vAll = []; { vAll = vAll + [["V1", _x]] } forEach (v1 call psUnits); { vAll = vAll + [["V2", _x]] } forEach (v2 call psUnits); { vAll = vAll + [["V3", _x]] } forEach (units v3); { vAll = vAll + [["V4", _x]] } forEach (units v4); { vAll = vAll + [["V5", _x]] } forEach (v5 call psUnits); { vAll = vAll + [["V6", _x]] } forEach (v6 call psUnits); { vAll = vAll + [["V7", _x]] } forEach (units v7); vP = []; { vP set [count vP, getPos (_x select 1)] } forEach vAll')
    for s in (4, 8, 12):
        wait_s(t, 4)
        t.c('_i = 0; { _p = vP select _i; logInfo format ["PROMO walk t%1 %2 #%3 speed %4 km/h, %5 m from start", SECS, _x select 0, _i, speed (_x select 1), [_p, getPos (_x select 1)] call psDist2]; _i = _i + 1 } forEach vAll'.replace('SECS', str(s)))
    footer(t)
    write(t)


def jdiag():
    """Judder diagnosis: Dedushka walks the coast highway; three of the first shoot's set-ups (c50 static low
    pan, c52 static pan near him, c93 orbit at helicopter height with a scripted Cobra) are recorded first with
    his tremor on (as in the first shoot: the player 400 to 600 m off, so every footfall starts a tremor), then
    with PROMO_NOSHAKE. The PCF log carries kjShakeGain at the camera every frame; judder.py measures the picture."""
    t = T('promo_jdiag', 'Lebanon80 promo: judder diagnosis, his camera tremor on and off')
    header(t, 16, 5, overcast=0.2, fog=0.0, view=3000, player_pos=[10300, 8700, 0])
    t.c(NO_TRAFFIC)
    kaiju_spawn(t, 9850, 8307, 90, noshake=False)
    t.a('PC_EXT = true')
    t.a('[] exec "helifly.sqs"')
    t.c('hA = ["LoBo_AH1", [9700, 8200, 40]] call hfHeli; KJ_crushOn = false; [[hA, "escort", [-300, -500, 70], 0, 0]] call hfStart')
    order(t, 'drawn', '[11100, 8307, 0]')
    wait_s(t, 8)
    for tag in ('on', 'off'):
        if tag == 'off':
            t.c('PROMO_NOSHAKE = true')
            wait_s(t, 3)
        t.c('PC_EXT = false')
        cam(t, 'lerp', e0='[((getPos kaiju) select 0) + 140, 8301, 1.6]', subj='kaiju', z=22, sx=0.5, sy=0.42, aim=True, fov0=0.5, fa=1, fb=0)
        record(t, 'j50_kaiju_low_front_shake_%s' % tag, 6, note='judder probe: c50 set-up (static low, pan up with him), raw subject aim', sym=True)
        cam(t, 'lerp', e0='[((getPos kaiju) select 0) + 30, 8352, 1.6]', e1='[((getPos kaiju) select 0) + 28, 8351, 1.5]', g0='[((getPos kaiju) select 0) - 10, 8307, 12]', g1='[((getPos kaiju) select 0) + 40, 8307, 18]', fov0=0.62)
        record(t, 'j52_kaiju_roadside_pan_shake_%s' % tag, 6, note='judder probe: c52 set-up (fixed eye and target path, no subject read)', gate=dict(x='none', hz=False))
        t.c('[[hA, "orbit", 62, 30, 200, 11, 0]] call hfStart; PC_EXT = true')
        cam(t, 'orbit', cobj='kaiju', c=[0, 0, 0], r0=92, r1=92, h0=32, h1=32, a0=190, a1=250, tz=24, fov0=0.6, ease=0)
        record(t, 'j93_kaiju_heli_orbit_shake_%s' % tag, 6, pre='HF_T0 = time + PC_HEAD', note='judder probe: c93 set-up (orbit round him at helicopter height, a Cobra circling)', gate=dict(x='none', hz=False))
        t.c('[[hA, "escort", [-300, -500, 70], 0, 0]] call hfStart')
    t.a('HF_QUIT = true')
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)


def walkprobe2():
    """Walking with the kaiju engine's direct steer (kjSteer [unit, pos, gait, arrive, rate]): any man walks
    a straight line at the gait's own clip speed, past the planner and the formation. Speeds logged."""
    t = T('promo_walkprobe2', 'Lebanon80 promo: kjSteer walking probe (no recording)')
    header(t, 9, 50, overcast=0.2, fog=0.0, view=1500, player_pos=[14700, 11500, 0])
    t.c(NO_TRAFFIC)
    C2 = '["LoBoGolaniWBo", "LoBoGolaniWB"]'
    H2 = '["LoBo_HizballahLeader", "LoBo_HizballahRifle4"]'
    rows = [('S1', 'west', C2, 11060, 'SAFE', 'walk', 1), ('S2', 'west', C2, 11075, 'AWARE', 'walk', 1), ('S3', 'west', C2, 11090, 'SAFE', 'slow', 1),
            ('S4', 'east', H2, 11105, 'AWARE', 'walk', 1), ('S5', 'east', H2, 11120, 'SAFE', 'walk', 1), ('S6', 'west', C2, 11135, 'COMBAT', 'walk', 1),
            ('S7', 'west', C2, 11150, 'SAFE', 'walk', 0.85)]
    for tag, side, cl, y, beh, gait, rate in rows:
        t.c('%s = [%s, %s, [14480, %d, 0], 270, 0.7] call psInf; %s setBehaviour "%s"; { kjSteer [_x, [(getPos _x select 0) - 150, getPos _x select 1, 0], "%s", 2, %g] } forEach (units %s)' % (tag, side, cl, y, tag, beh, gait, rate, tag))
    t.c('vAll = []; { _g = _x; { vAll = vAll + [[_g, _x]] } forEach (units (call compile _g)) } forEach ["S1", "S2", "S3", "S4", "S5", "S6", "S7"]; vP = []; { vP set [count vP, getPos (_x select 1)] } forEach vAll')
    for s in (4, 8, 12):
        wait_s(t, 4)
        t.c('_i = 0; { _p = vP select _i; logInfo format ["PROMO walk t%1 %2 #%3 speed %4 km/h, %5 m from start, %6", SECS, _x select 0, _i, speed (_x select 1), [_p, getPos (_x select 1)] call psDist2, behaviour (_x select 1)]; _i = _i + 1 } forEach vAll'.replace('SECS', str(s)))
    footer(t)
    write(t)


def walkprobe3():
    """kjSteer walking without an enemy in sight (sides 600 m apart): SAFE and AWARE, clip rates 1, 0.75, 0.62;
    one short clip of the IDF pair at rate 1 and 0.62 side by side to judge the legs."""
    t = T('promo_walkprobe3', 'Lebanon80 promo: kjSteer walking probe, one side at a time, clip rates')
    header(t, 9, 50, overcast=0.2, fog=0.0, view=1500, player_pos=[14700, 11500, 0])
    t.c(NO_TRAFFIC)
    C2 = '["LoBoGolaniWBo", "LoBoGolaniWB"]'
    H2 = '["LoBo_HizballahLeader", "LoBo_HizballahRifle4"]'
    rows = [('S1', 'west', C2, 14480, 11060, 'SAFE', 1), ('S2', 'west', C2, 14480, 11068, 'SAFE', 0.62), ('S3', 'west', C2, 14480, 11076, 'SAFE', 0.75),
            ('S4', 'west', C2, 14480, 11120, 'AWARE', 0.62), ('S5', 'east', H2, 14480, 11700, 'SAFE', 0.62), ('S6', 'east', H2, 14480, 11720, 'AWARE', 0.62),
            ('S7', 'east', H2, 14480, 11740, 'AWARE', 1)]
    for tag, side, cl, x, y, beh, rate in rows:
        t.c('%s = [%s, %s, [%d, %d, 0], 270, 0.7] call psInf; %s setBehaviour "%s"; { _x setPos [getPos _x select 0, %d + 4 * ((getPos _x select 1) - %d), 0]; kjSteer [_x, [(getPos _x select 0) - 150, getPos _x select 1, 0], "walk", 2, %g] } forEach (units %s)' % (tag, side, cl, x, y, tag, beh, y, y, rate, tag))
    t.c('vAll = []; { _g = _x; { vAll = vAll + [[_g, _x]] } forEach (units (call compile _g)) } forEach ["S1", "S2", "S3", "S4", "S5", "S6", "S7"]; vP = []; { vP set [count vP, getPos (_x select 1)] } forEach vAll')
    wait_s(t, 3)
    cam(t, 'lerp', e0='[(getPos ((units S2) select 0) select 0) - 12, 11040, 1.9]', subj='(units S2) select 0', z=1.0, sx=0.5, sy=0.55, aim=True, fov0=0.42)
    record(t, 'p03_walk_rates_probe', 4, note='probe: IDF SAFE walk, clip rate 1 (back) and 0.62 (front)', gate=dict(x='none', hz=False))
    for s in (12,):
        t.c('_i = 0; { _p = vP select _i; logInfo format ["PROMO walk t%1 %2 #%3 speed %4 km/h, %5 m from start, %6", SECS, _x select 0, _i, speed (_x select 1), [_p, getPos (_x select 1)] call psDist2, behaviour (_x select 1)]; _i = _i + 1 } forEach vAll'.replace('SECS', str(s)))
    footer(t)
    write(t)


def beam_stage(t):
    """the Hizballah position in the olive plain, restaged before every beam: crates, a technical, a truck, six
    fighters dug in facing him (armed, firing on him); civilians are cleared at the start of the take"""
    t.c(('if (!(isNil "bSq")) then { bSq call psDelGrp; { deleteVehicle _x } forEach [bTech, bUral, bB1, bB2] }; '
         'bB1 = ["MAP_Box_IronC", [10441, 9284, 0], 10] call psProp; bB2 = ["MAP_Crate", [10437, 9280, 0], 60] call psProp; '
         'bTech = ["LoBo_JeepMG_Hizballah", [10462, 9268, 0], 190] call psVeh; bUral = ["LoBo_Ural_Hizballah", [10425, 9290, 0], 160] call psVeh; '
         'bSq = [east, %s, [10450, 9262, 0], 180, 0.7] call psInf; bU = units bSq; '
         '{ _x setUnitPos "MIDDLE"; _x disableAI "MOVE"; _x setDir 180 } forEach bU; '
         '(bU select 0) setPos [10444, 9262, 0]; (bU select 1) setPos [10452, 9258, 0]; (bU select 2) setPos [10466, 9262, 0]; (bU select 3) setPos [10433, 9266, 0]; (bU select 4) setPos [10458, 9272, 0]; (bU select 5) setPos [10470, 9276, 0]; '
         'bSq setCombatMode "RED"; bSq setBehaviour "COMBAT"; { _x reveal kaiju } forEach bU; KJ_beamSpentUntil = 0') % HEZ_SQUAD)


def beam_shot(t, name, secs, note, camspec, gate=None, sym=False):
    """the camera on its first pose and settled, then the beam order; at the still phase (1.2 s before the
    charge) a second full settle on the same pose with the world paused (the beam script waits with it), then the recorder: the delivered clip
    opens 0.7 s before the charge (the chest glow building), the fire about 4.1 s in. phaselog.sqs logs every phase change with
    the clip's frame count."""
    kind = camspec.pop('kind')
    cam(t, kind, **camspec)
    settle(t)
    order(t, 'beam', 'bTech')
    until(t, 'KJ_beamPhase in ["still", "charge"]', 40)
    record(t, name, secs, note=note, gate=gate, sym=sym)
    until(t, 'KJ_beamPhase in ["spent", "idle"]', 40)
    wait_s(t, 4)


def kaiju_beam():
    t = T('promo_kaiju_beam', 'Lebanon80 promo: Dedushka turns his beam on a Hizballah position in the olive plain, dusk to night, and Ghajar at night')
    header(t, 16, 50, overcast=0.2, fog=0.0, view=3000, player_pos=[11200, 9800, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10450, 9150, 0])
    kaiju_spawn(t, 10450, 8960, 0)
    t.c('KJ_beamOverride = { KJ_beamT_still = 1.2; KJ_beamT_charge = 3.4; KJ_beamT_hold = 0.6; KJ_beamT_sweepMin = 5 }')
    t.a('[] exec "phaselog.sqs"')
    # wide: high behind his shoulder, the beam crossing the valley onto the position (left third)
    time_of_day(t, 16, 50)
    beam_stage(t)
    wait_s(t, 3)
    beam_shot(t, 'c64_kaiju_beam_wide_over_shoulder_valley', 13, 'BEAM wide: high behind his shoulder, the beam crosses the olive valley onto the Hizballah position; wind-up, impact, sweep, fire',
              dict(kind='lerp', e0=[10545, 8878, 36], e1=[10540, 8884, 34], fov0=0.72, subjp=[10452, 9268, 0], sx=TWO3, sy=0.6, subj2='kaiju', z2=30, aim=True),
              gate=dict(x=TWO3, y=0.6))
    # hero: low behind and under him, tilt from the furnace chest down the beam (symmetric, deliberate)
    time_of_day(t, 16, 58)
    beam_stage(t)
    wait_s(t, 3)
    beam_shot(t, 'c65_kaiju_beam_hero_under_chest', 12, 'BEAM hero: low behind and under him, the furnace chest lighting up and firing down the valley, the beam down the centre',
              dict(kind='lerp', e0=[10463, 8938, 1.4], e1=[10461, 8941, 1.5], g0=[10450, 8962, 27], g1=[10450, 9060, 14], fov0=0.8, fov1=0.74, subj='kaiju', z=26),
              gate=dict(x=0.5, tol=0.12, until=3.0, hz=False), sym=True)
    # the fighters fire on him, then the reverse up the beam (from behind them)
    time_of_day(t, 17, 6)
    beam_stage(t)
    wait_s(t, 3)
    cam(t, 'lerp', e0=[10476, 9300, 1.8], e1=[10474, 9298, 1.7], fov0=0.45, subj='kaiju', z=26, sx=TWO3, sy=THIRD, subj2='bU select 0', z2=1.0, aim=True)
    record(t, 'c68_kaiju_dusk_fighters_fire_on_him', 8, note='behind the Hizballah line as they fire on Dedushka across the plain', gate=dict(x=TWO3, y=THIRD, hz=False))
    beam_shot(t, 'c66_kaiju_beam_reverse_up_the_beam', 12, 'BEAM reverse: from behind the fighters, looking up the beam at him as it comes down on them',
              dict(kind='lerp', e0=[10472, 9312, 2.2], e1=[10471, 9309, 2.1], fov0=0.4, subj='kaiju', z=26, sx=TWO3, sy=THIRD, aim=True),
              gate=dict(x=TWO3, y=THIRD, hz=False))
    # night hero: beside his legs, the chest glow on the upper right third, the beam into the dark
    time_of_day(t, 18, 20)
    beam_stage(t)
    wait_s(t, 3)
    beam_shot(t, 'c67_kaiju_beam_night_hero', 12, 'BEAM night hero: beside his legs at night, the chest glow builds and the beam fires into the dark',
              dict(kind='lerp', e0=[10478, 8946, 2.0], e1=[10476, 8949, 2.0], fov0=0.75, subj='kaiju', z=26, sx=TWO3, sy=THIRD, aim=True),
              gate=dict(x=TWO3, y=THIRD, hz=False))
    # he walks through the burning position, at night
    t.c('KJ_beamSpentUntil = 0')
    order(t, 'drawn', '[10450, 9330, 0]')
    wait_s(t, 6)
    cam(t, 'follow', obj='kaiju', o0=[60, 30, 14], o1=[55, 60, 12], pa=0.1, hda=0.03, fov0=0.7, subj='kaiju', z=22, sx=THIRD, sy=0.45, aim=True, fa=0.25, fb=0.03)
    record(t, 'c61_kaiju_night_walks_through_burning_position', 10, note='Dedushka walks through the burning Hizballah position at night', gate=dict(x=THIRD, y=0.45, hz=False))
    # scale over Ghajar at night: he stands on the hill east of the village, the furnace glowing
    t.c('KJ_brainOrder = ["idle"]')
    wait_s(t, 3)
    t.c('player setPos [14800, 11600, 0]')
    clear_civs(t, [14450, 11050, 0])
    t.c('kaiju setPos [14600, 11140, 0]; kaiju setDir 255')
    wait_s(t, 4)
    cam(t, 'lerp', e0=[14240, 10990, 30], e1=[14270, 11000, 26], fov0=0.24, subj='kaiju', z=24, sx=TWO3, sy=0.42, aim=True)
    record(t, 'c62_kaiju_night_glow_over_ghajar', 10, note='scale: his furnace glow on the hill above Ghajar at night', gate=dict(x=TWO3, y=0.42, hz=False))
    cam(t, 'lerp', e0=[14330, 10950, 1.7], e1=[14335, 10955, 1.7], fov0=0.22, subj='kaiju', z=30, sx=TWO3, sy=THIRD, aim=True)
    record(t, 'c63_kaiju_night_silhouette_from_village_street', 9, note='teaser: his silhouette and chest glow above the village roofs', gate=dict(x=TWO3, y=THIRD, hz=False))
    t.a('PC_QUIT = true')
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)


def kaiju_heli():
    """IDF helicopters around Dedushka for scale. The helicopters are not flown by their AI: hfStep (promo_lib.sqf)
    sets each one's position, heading and velocity every frame along a path relative to him (crew in, engine on,
    undamageable), and the camera is stepped right after them in the same loop (PC_EXT). HF_HOLD freezes the
    paths at their start while the scene settles."""
    t = T('promo_kaiju_heli', 'Lebanon80 promo: IDF helicopters fly with and around Dedushka (scale), golden hour and dusk')
    header(t, 16, 12, overcast=0.2, fog=0.0, view=3000, player_pos=[10300, 8700, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10400, 8307, 0], 1200)
    kaiju_spawn(t, 9850, 8307, 90)
    t.a('PC_EXT = true')
    t.a('[] exec "helifly.sqs"')
    t.c('hA = ["LoBo_AH1", [9700, 8200, 40]] call hfHeli; hB = ["LoBo_AH1", [9700, 8180, 40]] call hfHeli; hU = ["LoBoB212", [9700, 8160, 40]] call hfHeli; logInfo format ["PROMO helis %1 %2 %3", typeOf hA, typeOf hB, typeOf hU]')
    t.c('KJ_crushOn = false; [[hA, "escort", [-300, -500, 70], 0, 0], [hB, "escort", [-340, -520, 75], 0, 0], [hU, "escort", [-380, -540, 80], 0, 0]] call hfStart')
    order(t, 'drawn', '[11100, 8307, 0]')
    wait_s(t, 8)
    shots = [
        ('c90_kaiju_heli_cobras_flypast_chest_golden', 9,
         '[[hA, "line", [-16, 90, 27], [-16, -90, 27], 6, 0], [hB, "line", [-26, 120, 36], [-26, -60, 36], 6, 0], [hU, "escort", [0, -400, 60], 0, 0]] call hfStart',
         dict(kind='lerp', e0='[[(getPos kaiju) select 0, (getPos kaiju) select 1, 0], getDir kaiju, 70, -60, 18] call psOff', fov0=0.55, subj='kaiju', z=24, sx=TWO3, sy=0.42, subj2='hA', z2=0, aim=True, fa=0.25, fb=0.03),
         'PC_E0 set [2, 18]; PC_E1 = +PC_E0',
         'two IDF Cobras rush past his chest and head, tiny against him, golden hour'),
        ('c91_kaiju_heli_escort_pair_at_shoulders', 10,
         '[[hA, "escort", [-32, 6, 34], 1.2, 40], [hU, "escort", [34, 2, 31], 1.5, 33], [hB, "escort", [-60, -40, 45], 1, 25]] call hfStart',
         dict(kind='follow', obj='kaiju', o0=[-55, -75, 38], o1=[-48, -62, 34], pa=0.1, hda=0.05, fov0=0.62, subj='kaiju', z=30, sx=TWO3, sy=0.4, subj2='hA', z2=0, aim=True, fa=0.25, fb=0.03),
         '',
         'an IDF escort pair flies at his shoulders as he walks the highway, a third helicopter behind'),
        ('c92_kaiju_heli_chase_bell_overtakes_him', 10,
         '[[hU, "line", [22, -170, 30], [22, 22, 33], 10, 0], [hA, "escort", [-34, 4, 34], 1.2, 40], [hB, "escort", [-60, -40, 45], 1, 25]] call hfStart',
         dict(kind='follow', obj='hU', o0=[9, -26, 38], o1=[8, -20, 36], pa=1, hda=0.3, fov0=0.72, subj='hU', z=0, sx=THIRD, sy=0.55, subj2='kaiju', z2=30, aim=True),
         '',
         'chase camera behind a Bell 212 as it overtakes him, the Bell on the left third, he fills the right'),
        ('c93_kaiju_heli_orbit_cobra_foreground', 10,
         '[[hA, "orbit", 62, 30, 200, 11, 0], [hU, "escort", [34, 2, 31], 1.5, 33], [hB, "orbit", 85, 46, 20, 11, 0]] call hfStart',
         dict(kind='orbit', cobj='kaiju', c=[0, 0, 0], r0=92, r1=92, h0=32, h1=32, a0=190, a1=300, tz=24, fov0=0.6, ease=0, subj='kaiju', z=26, sx=TWO3, sy=0.45, subj2='hA', z2=0, aim=True, fa=0.25, fb=0.03),
         '',
         'orbit at helicopter height, a Cobra circling him in the foreground for scale'),
        ('c94_kaiju_heli_cobra_past_his_head_closeup', 8,
         '[[hA, "line", [-12, 80, 37], [-12, -80, 37], 5.5, 0.3], [hU, "escort", [34, 2, 31], 1.5, 33], [hB, "escort", [-60, -40, 45], 1, 25]] call hfStart',
         dict(kind='follow', obj='kaiju', o0=[-75, 18, 36], o1=[-72, 10, 36], pa=0.15, hda=0.05, fov0=0.28, subj='kaiju', z=34, sx=THIRD, sy=THIRD, subj2='hA', z2=0, aim=True, fa=0.25, fb=0.03),
         '',
         'long lens on his head as a Cobra flies past it close'),
    ]
    for name, secs, heli, camspec, fix, note in shots:
        t.c('HF_HOLD = true; ' + heli)
        kind = camspec.pop('kind')
        cam(t, kind, **camspec)
        if fix:
            t.c(fix + '; [] call pcHold')
        record(t, name, secs, pre='HF_HOLD = false; HF_T0 = time + PC_HEAD', note=note, gate=dict(x=camspec.get('sx', 'any'), y=camspec.get('sy'), tol=0.1, hz=False))
    # dusk: he stands, the furnace glowing; helicopters circle above his head
    time_of_day(t, 16, 58)
    order(t, 'idle')
    wait_s(t, 4)
    circle = '[[hA, "orbit", 48, 58, 0, 14, 0], [hB, "orbit", 60, 66, 120, 14, 0], [hU, "orbit", 55, 52, 240, 14, 0]] call hfStart'
    dusk = [
        ('c95_kaiju_heli_dusk_lookup_circling_glow', 10, circle,
         dict(kind='lerp', e0='[[(getPos kaiju) select 0, (getPos kaiju) select 1, 0], getDir kaiju, 34, 6, 0.6] call psOff', fov0=0.85, subj='kaiju', z=30, sx=THIRD, sy=0.42, subj2='hA', z2=0, aim=True),
         'PC_E0 set [2, 0.6]; PC_E1 = +PC_E0',
         'low ground camera looking up at his glowing chest and head, three helicopters circling above him at dusk'),
        ('c96_kaiju_heli_dusk_orbit_glow_wide', 10, circle,
         dict(kind='orbit', cobj='kaiju', c=[0, 0, 0], r0=130, r1=125, h0=45, h1=42, a0=30, a1=90, tz=28, fov0=0.6, ease=1, subj='kaiju', z=26, sx=TWO3, sy=0.5, aim=True),
         '',
         'wide orbit at dusk: his furnace glow and three IDF helicopters circling him'),
    ]
    for name, secs, heli, camspec, fix, note in dusk:
        t.c('HF_HOLD = true; ' + heli)
        kind = camspec.pop('kind')
        cam(t, kind, **camspec)
        if fix:
            t.c(fix + '; [] call pcHold')
        record(t, name, secs, pre='HF_HOLD = false; HF_T0 = time + PC_HEAD', note=note, gate=dict(x=camspec.get('sx', 'any'), y=camspec.get('sy'), tol=0.1, hz=False))
    t.a('HF_QUIT = true')
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)


def dollyzoom():
    """Dolly zoom on a Hizballah fighter with Dedushka standing 300 m behind him on the coast highway, composed
    on thirds (dzSetup2): the fighter's head holds the left and upper third lines while the eye backs from 3.5
    to 15 m and the lens narrows (fov x distance constant); Dedushka's chest starts near the middle and grows out
    to the right third. Zoom 0 to 3.5 s; the fighter turns and sprints off at 4.3 s (DZ_DELAY counts the head)."""
    t = T('promo_dollyzoom', 'Lebanon80 promo: dolly zoom on a Hizballah fighter as Dedushka looms behind him, then he runs')
    header(t, 16, 28, overcast=0.2, fog=0.0, view=3000, player_pos=[10600, 8700, 0])
    t.c(NO_TRAFFIC)
    clear_civs(t, [10150, 8307, 0], 1000)
    kaiju_spawn(t, 10000, 8307, 90)
    t.c('KJ_crushOn = false')
    takes = [((16, 28), 'c97_dollyzoom_fighter_golden', ''),
             ((16, 34), 'c98_dollyzoom_fighter_golden_roar', 'KJ_brainOrder = ["roar"]; '),
             ((16, 57), 'c99_dollyzoom_fighter_dusk_glow', '')]
    for (hh, mm), name, extra in takes:
        time_of_day(t, hh, mm)
        t.c('if (!(isNil "dzG")) then { dzG call psDelGrp }; dzG = createGroup east; "LoBo_HizballahRifle4" createUnit [[10300, 8307, 0], dzG, "DZ_F = this", 0.6, "PRIVATE"]; DZ_F setPos [10300, 8307, 0]; DZ_F setDir 90; DZ_F setCaptive true; DZ_F disableAI "TARGET"; DZ_F disableAI "AUTOTARGET"; DZ_F disableAI "MOVE"; DZ_F stop true; dzG setBehaviour "SAFE"; DZ_TO = [10270, 8385, 0]; DZ_DELAY = 4.3 + PC_HEAD')
        t.a('triSimFrames 45')
        t.c('PC_SUBJ = DZ_F; PC_SUBJP = []; PC_SUBJZ = 1.55; PC_SUBJ2 = kaiju; PC_SUBJP2 = []; PC_SUBJZ2 = 24; PC_AIM = false; [DZ_F, 1.55, 24, 3.5, 15, 0.27, %g, %g, %g, %g] call dzSetup2; PC_EASE = 1; [] call pcHold' % (THIRD, THIRD, TWO3, 0.42))
        t.comp = dict(sx=THIRD, sy=THIRD)
        settle(t)
        t.clips.append(dict(name=name, seconds=8.5, note='', gate=dict(x=THIRD, y=THIRD, until=4.0, x2=TWO3, tol2=0.18, hz=False), sym=False))
        t.c('%s[] exec "dzrun.sqs"; "%s" call pcLogOn; 3.5 call pcGo; prRec = triRecord ["%s.raw", %d, 0]; logInfo format ["PROMO rec %s %%1 at %%2", prRec, time]' % (extra, name, name, 255 + 15, name))
        t.a('triRecordState')
        t.c('PC_LOG = false')
        t.log('done %s %%1' % name, 'triRecordState')
        t.a('PC_RUN = false')
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)


def nightprobe():
    """night-diagnosis.md probe: the c06 Ghajar pose held still, three settled stills with triFeatureState: A the
    old night (21 June 18:25, half moon up), B the new moon (16 June 20:00), C a full moon (28 June 20:00). Also a
    survey of the street lamps and the camp-fire class for the night staging. No recording."""
    t = T('promo_nightprobe', 'Lebanon80 promo: night probe, moon on and off, lamp survey')
    header(t, 18, 25, overcast=0.2, fog=0.0, view=3500, player_pos=[14800, 11600, 0])
    t.c(NO_TRAFFIC)
    cam(t, 'orbit', c=[14215, 10990, 0], r0=260, r1=260, h0=330, h1=330, a0=140, a1=140, asl=True, fov0=0.55, subjp=[14250, 11000, 4], sx=TWO3, sy=0.6, aim=True)
    for tag, date in (('A', (1985, 6, 21, 18, 25)), ('B', (1985, 6, 16, 20, 0)), ('C', (1985, 6, 28, 20, 0))):
        t.c('setDate %s' % q(list(date)))
        settle(t)
        t.c('logInfo format ["PROMO probe %s daytime %%1 feat %%2", daytime, triFeatureState]' % tag)
        t.a('triScreenshot "night_%s"' % tag)
    # street lamps and other typed objects along the Ghajar east road
    for x in range(14150, 14560, 50):
        t.c('{ if ((typeOf _x) != "") then { logInfo format ["PROMO obj %%1 %%2", typeOf _x, getPos _x] } } forEach (nearestObjects [[%d, 11098, 0], [], 30])' % x)
    t.c('{ logInfo format ["PROMO lamp %%1 %%2", typeOf _x, getPos _x] } forEach (nearestObjects [[14300, 11050, 0], ["StreetLamp"], 400])')
    t.c('{ _f = _x createVehicle [14300, 11200, 0]; logInfo format ["PROMO fireclass %%1 -> %%2", _x, typeOf _f]; deleteVehicle _f } forEach ["Fire", "FireLit", "Fireplace"]')
    footer(t)
    write(t)


# ======================================================================================== true night
# night-diagnosis.md: 16 June, 20:00 and later (new moon below the horizon, sun 37 degrees under, night share 1);
# triSetLighting "remastered" and nothing else; the settle before every clip logs triFeatureState (the
# moonShadows gate must read off). Every shot is lit from inside the story and framed on its light pool.
NIGHT = (1985, 6, 16)


def night_ghajar():
    t = T('promo_night_ghajar', 'Lebanon80 promo: true night in Ghajar (new moon): a lamp-lit street, a patrol against lit walls, a camp fire at the cache, a checkpoint in headlights')
    header(t, 20, 0, overcast=0.2, fog=0.0, view=2000, player_pos=[14800, 11600, 0], date=NIGHT)
    t.c(NO_TRAFFIC)
    clear_civs(t, [14300, 11050, 0], 600)
    # a slow low orbit round the street lamp at the village edge (the only typed lamp on the east road)
    cam(t, 'orbit', c=[14203.4, 11073.5, 0], r0=22, r1=20, h0=7, h1=6, a0=20, a1=60, fov0=0.6, subjp=[14203.4, 11073.5, 2], sx=TWO3, sy=0.6, aim=True)
    record(t, 'c06n_night_ghajar_street_lamp_orbit', 10, note='true night in Ghajar: a slow low orbit round a street lamp, its pool on the walls and the street, the village black beyond', gate=dict(x=TWO3, y=0.6, hz=False))
    # an IDF patrol walks along the road at the village edge, silhouetted against the lit walls
    walkers(t, 'nPat', 'west', IDF_PATROL, [14345, 11098, 0], 270, [14170, 11098, 0], spacing=3.5, lateral=0.7, beh='SAFE')
    wait_s(t, 1)
    cam(t, 'follow', obj='nPatU select 2', o0=[7, 2.5, 2.0], o1=[7, -1, 2.0], pa=0.15, hda=0.04, fov0=0.5, subj='nPatU select 2', z=1.65, sx=THIRD, sy=THIRD, aim=True)
    record(t, 'c80n_night_idf_patrol_silhouettes_lit_walls', 9, note='true night: an IDF patrol walks the road at the edge of Ghajar, black silhouettes against the lamp-lit walls', gate=dict(x=THIRD, y=THIRD, hz=False))
    t.c('nPat call psDelGrp')
    # the cache at the east edge of Ghajar round a camp fire
    t.c('cB1 = ["MAP_Box_IronC", [14373, 11032, 0], 20] call psProp; cB2 = ["MAP_Crate", [14376, 11034, 0], 70] call psProp; cB3 = ["MAP_Box_IronO", [14371, 11028, 0], 140] call psProp; cT = ["LoBo_JeepMG_Hizballah", [14385, 11023, 0], 300] call psVeh; cF = "Fire" createVehicle [14377, 11029.5, 0]; cF setPos [14377, 11029.5, 0]; cF inflame true; logInfo format ["PROMO fire %1 %2", typeOf cF, getPos cF]')
    t.c('cSq = [east, %s, [14370, 11036, 0], 120, 0.7] call psInf; cU = units cSq; [cU select 0, [14374.5, 11031.5, 0], 140, "UP"] call psPost; [cU select 1, [14379.5, 11027.5, 0], 320, "MIDDLE"] call psPost; [cU select 2, [14380, 11031.5, 0], 220, "UP"] call psPost; [cU select 3, [14374, 11027, 0], 45, "MIDDLE"] call psPost; [cU select 4, [14377, 11033, 0], 180, "MIDDLE"] call psPost; [cU select 5, [14382, 11028, 0], 290, "UP"] call psPost; cSq setBehaviour "SAFE"' % HEZ_SQUAD)
    wait_s(t, 3)
    cam(t, 'orbit', c=[14377, 11029.5, 0], r0=8, r1=7.5, h0=2.4, h1=2.2, a0=150, a1=210, fov0=0.6, subjp=[14377, 11029.5, 0.6], sx=TWO3, sy=0.62, aim=True)
    record(t, 'c35n_night_hezb_cache_campfire_orbit', 10, note='true night: Hizballah fighters round a camp fire at their cache, the crates and the technical in its light', gate=dict(x=TWO3, y=0.62, hz=False))
    cam(t, 'lerp', e0=free(14365, 11041, 1.6), e1=free(14368, 11038, 1.5), fov0=0.42, subjp=[14377, 11029.5, 0.6], sx=THIRD, sy=0.6, aim=True)
    record(t, 'c36n_night_hezb_cache_campfire_push', 8, note='true night: slow push on the fire, the fighters lit from below', gate=dict(x=THIRD, y=0.6, hz=False))
    t.c('cSq call psDelGrp')
    # the checkpoint at night: the bar across the paved road, the soldiers black against the headlights of a
    # Humvee standing in the approach lane
    clear_civs(t, [14300, 11068, 0], 500)
    checkpoint_stage(t, night=True)
    wait_s(t, 3)
    cam(t, 'lerp', e0='[cpR, cpA, -12, 3.0, 1.7] call psOffZ', e1='[cpR, cpA, -11.5, 2.6, 1.7] call psOffZ', fov0=0.38, subj='pCpU select 0', z=1.65, sx=THIRD, sy=THIRD, subj2='pHum', z2=1.0, aim=True)
    record(t, 'c13n_night_idf_checkpoint_in_headlights', 9, note='true night: IDF soldiers at the bar across the road, black against the headlights of their Humvee', gate=dict(x=THIRD, y=THIRD, hz=False))
    footer(t)
    write(t)


def night_fight():
    t = T('promo_night_fight', 'Lebanon80 promo: true night firefight and the convoy in its headlights (new moon)')
    header(t, 20, 0, overcast=0.2, fog=0.0, view=2000, player_pos=[11000, 9800, 0], date=NIGHT)
    t.c(NO_TRAFFIC)
    clear_civs(t, [10450, 9290, 0])
    # the firefight: two lines 60 m apart on the olive plain, nobody is hurt (allowDammage false)
    t.c('gI = [west, ["LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB", "LoBoGolaniWB"], [10450, 9250, 0], 0, 0.7] call psInf; gH = [east, ["LoBo_HizballahRifle4", "LoBo_HizballahMG1", "LoBo_HizballahRifle1", "LoBo_HizballahMG1", "LoBo_HizballahRifle4", "LoBo_HizballahLeader"], [10450, 9310, 0], 180, 0.7] call psInf; gIU = units gI; gHU = units gH')
    t.c('_i = 0; { _x allowDammage false; _x setPos [10436 + 5 * _i, 9250 + 2 * (_i mod 2), 0]; _x setDir 0; _x setUnitPos "MIDDLE"; _x disableAI "MOVE"; _i = _i + 1 } forEach gIU; _i = 0; { _x allowDammage false; _x setPos [10438 + 5 * _i, 9310 - 2 * (_i mod 2), 0]; _x setDir 180; _x setUnitPos "MIDDLE"; _x disableAI "MOVE"; _i = _i + 1 } forEach gHU')
    t.c('{ _x setCombatMode "RED"; _x setBehaviour "COMBAT" } forEach [gI, gH]; { _u = _x; { _u reveal _x } forEach gHU } forEach gIU; { _u = _x; { _u reveal _x } forEach gIU } forEach gHU')
    wait_s(t, 5)
    cam(t, 'lerp', e0='[(getPos (gIU select 1) select 0) + 2.0, (getPos (gIU select 1) select 1) - 3.0, 1.4]', e1='[(getPos (gIU select 1) select 0) + 1.8, (getPos (gIU select 1) select 1) - 2.8, 1.4]', fov0=0.5, subj='gIU select 1', z=0.6, sx=THIRD, sy=0.6, subjp2=[10455, 9310, 1], aim=True)
    record(t, 'c85_night_firefight_behind_idf_machine_gun', 12, note='true night firefight: behind an IDF machine gunner, his muzzle flash lights him, tracers go out and come back across 60 m', gate=dict(x=THIRD, hz=False))
    cam(t, 'lerp', e0=free(10425, 9282, 1.5), e1=free(10425.5, 9281, 1.5), fov0=0.45, subj='gHU select 1', z=0.6, sx=TWO3, sy=0.55, subj2='gIU select 1', z2=0.6, aim=True)
    record(t, 'c85b_night_firefight_between_the_lines', 10, note='true night: low between the two lines, tracers crossing both ways within 30 m of the lens', gate=dict(x=TWO3, hz=False))
    cam(t, 'lerp', e0='[(getPos (gHU select 1) select 0) - 2.0, (getPos (gHU select 1) select 1) + 3.0, 1.4]', e1='[(getPos (gHU select 1) select 0) - 1.8, (getPos (gHU select 1) select 1) + 2.8, 1.4]', fov0=0.5, subj='gHU select 1', z=0.6, sx=THIRD, sy=0.6, subjp2=[10455, 9250, 1], aim=True)
    record(t, 'c86_night_firefight_behind_hezb_machine_gun', 10, note='true night: behind a Hizballah machine gunner, his muzzle light on the grass, the IDF tracers answering', gate=dict(x=THIRD, hz=False))
    t.c('gI call psDelGrp; gH call psDelGrp')
    # the convoy at night, headlights: low on the Ghajar east road ahead of it
    t.c('player setPos [14700, 11600, 0]')
    clear_civs(t, [14350, 11098, 0], 600)
    t.c('vV1 = ["LoBoHMWV_Mag_Patrol", [14540, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC; vV2 = ["LoBo_M35", [14568, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; vV3 = ["LoBo_Truck5tcovered", [14596, 11098, 0], 270, west, "LoBoGolaniWB", false, false] call psVehC; vV4 = ["LoBo_Zelda", [14624, 11098, 0], 270, west, "LoBoGolaniWB", true, false] call psVehC; { [_x, [14150, 11098, 0], "LIMITED", "SAFE"] call psGo } forEach [vV1, vV2, vV3, vV4]')
    until(t, '((getPos vV1) select 0) < 14500', 60)
    cam(t, 'lerp', e0=free(14440, 11095.5, 0.9), e1=free(14441, 11095.5, 0.9), fov0=0.4, subj='vV1', z=1.2, sx=TWO3, sy=0.55, aim=True)
    record(t, 'c23n_night_idf_convoy_headlights_low_road', 10, note='true night: the IDF convoy comes down the road at a low camera, the headlights and the lit road fill the frame', gate=dict(x=TWO3, tol=0.1, hz=False))
    t.c('{ (group (driver _x)) call psDelGrp; deleteVehicle _x } forEach [vV1, vV2, vV3, vV4]')
    footer(t)
    write(t)


def night_ambush():
    """the ambush at true night: the convoy's headlights, the fighters' muzzle flashes, the truck blows; then the
    burning wreck lights the road"""
    t = T('promo_night_ambush', 'Lebanon80 promo: the ambush at true night (new moon): headlights, muzzle flashes, the burning truck')
    header(t, 20, 10, overcast=0.2, fog=0.0, view=2500, player_pos=[13600, 12100, 0], date=NIGHT)
    t.c(NO_TRAFFIC)
    clear_civs(t, [14470, 11098, 0], 1000)
    amb_stage(t)
    cam(t, 'lerp', e0=free(14432, 11096, 1.2), e1=free(14433, 11096, 1.2), fov0=0.45, subj='xV1', z=1.2, sx=TWO3, sy=0.55, aim=True)
    record(t, 'c87_night_ambush_headlights_low_road', 11, note='true night, low on the road: the convoy headlights come on, the ambush opens on them, the truck blows', gate=dict(x=TWO3, tol=0.12, hz=False))
    wait_s(t, 4)
    cam(t, 'lerp', e0=free(14518, 11084, 1.8), e1=free(14522, 11086, 1.7), fov0=0.45, subj='xV2', z=1.5, sx=TWO3, sy=0.55, subj2='xQ select 1', z2=1.0, aim=True)
    record(t, 'c88_night_ambush_burning_truck_return_fire', 10, note='true night: the burning truck lights the road, IDF soldiers return fire beside it', gate=dict(x=TWO3, tol=0.1, hz=False))
    footer(t)
    write(t)


def kaiju_night():
    """Dedushka at true night: the beam hero, the walk through the burning position, his glow over Ghajar and his
    silhouette from a village street. His furnace is his own key light: KJF_BREATH 0.08 at night (0.03 by day)."""
    t = T('promo_kaiju_night', 'Lebanon80 promo: Dedushka at true night (new moon): beam hero, burning position, Ghajar')
    header(t, 20, 0, overcast=0.2, fog=0.0, view=3000, player_pos=[11200, 9800, 0], date=NIGHT)
    t.c(NO_TRAFFIC)
    clear_civs(t, [10450, 9150, 0])
    kaiju_spawn(t, 10450, 8960, 0)
    t.c('KJF_BREATH = 0.08; KJ_beamOverride = { KJ_beamT_still = 1.2; KJ_beamT_charge = 3.4; KJ_beamT_hold = 0.6; KJ_beamT_sweepMin = 5 }')
    t.a('[] exec "phaselog.sqs"')
    beam_stage(t)
    wait_s(t, 3)
    beam_shot(t, 'c67_kaiju_beam_night_hero', 12, 'BEAM night hero: beside his legs on a moonless night, the chest glow builds and the beam fires into the dark',
              dict(kind='lerp', e0=[10478, 8946, 2.0], e1=[10476, 8949, 2.0], fov0=0.75, subj='kaiju', z=26, sx=TWO3, sy=THIRD, aim=True),
              gate=dict(x=TWO3, y=THIRD, hz=False))
    t.c('KJ_beamSpentUntil = 0')
    order(t, 'drawn', '[10450, 9600, 0]')
    wait_s(t, 6)
    cam(t, 'follow', obj='kaiju', o0=[45, 25, 10], o1=[42, 45, 9], pa=0.1, hda=0.03, fov0=0.65, subj='kaiju', z=22, sx=THIRD, sy=0.45, aim=True, fa=0.25, fb=0.03)
    record(t, 'c61_kaiju_night_walks_through_burning_position', 10, note='Dedushka walks through the burning Hizballah position on a moonless night, lit by the fires and his chest', gate=dict(x=THIRD, y=0.45, hz=False))
    # over Ghajar: he stands on the hill east of the village; low camera among the lit roofs
    t.c('KJ_brainOrder = ["idle"]')
    wait_s(t, 3)
    t.c('player setPos [14800, 11600, 0]')
    clear_civs(t, [14450, 11050, 0])
    t.c('kaiju setPos [14600, 11140, 0]; kaiju setDir 255')
    wait_s(t, 4)
    cam(t, 'lerp', e0=free(14250, 10985, 18, 3), e1=free(14256, 10989, 17, 3), fov0=0.26, subj='kaiju', z=24, sx=TWO3, sy=0.42, aim=True)
    record(t, 'c62_kaiju_night_glow_over_ghajar', 10, note='scale: his furnace glow on the hill above the lamp-lit roofs of Ghajar, a moonless night', gate=dict(x=TWO3, y=0.42, hz=False))
    cam(t, 'lerp', e0=[14330, 10950, 1.7], e1=[14335, 10955, 1.7], fov0=0.22, subj='kaiju', z=30, sx=TWO3, sy=THIRD, aim=True)
    record(t, 'c63_kaiju_night_silhouette_from_village_street', 9, note='teaser: his silhouette and chest glow above the village roofs, a moonless night', gate=dict(x=TWO3, y=THIRD, hz=False))
    t.a('PC_QUIT = true')
    footer(t)
    write(t, mission=KAIJU_MISSION, mods=KAIJU_MODS)



def rbprobe():
    """Roadblock probe: road heading at the checkpoint and roadblock points (road-segment centres from
    nearestRoads and the road objects under them), overhead stills of each spot, and the barrier model's
    long axis (a MAP_Barrier at setDir 0 seen from straight above). No recording."""
    t = T('promo_rbprobe', 'Lebanon80 promo: roadblock probe, road headings and the barrier axis')
    header(t, 12, 0, overcast=0.2, fog=0.0, view=1500, player_pos=[14700, 11600, 0])
    t.c(NO_TRAFFIC)
    pts = dict(rbA=[14300, 11098], rbB=[14330, 11098], rbC=[14442, 11098], rbK=[10900, 8307])
    for tag, (x, y) in pts.items():
        t.c('{ logInfo format ["PROMO road %s %%1 %%2", _x, [_x, [%g, %g]] call psDist2] } forEach ([%g, %g, 0] nearestRoads 45)' % (tag, x, y, x, y))
        t.c('{ logInfo format ["PROMO obj %s %%1 dir %%2 at %%3", typeOf _x, getDir _x, getPos _x] } forEach (nearestObjects [[%g, %g, 0], [], 6])' % (tag, x, y))
    # the barrier at setDir 0, a soldier beside it for scale, seen from straight above
    t.c('bT = ["MAP_Barrier", [14300, 11140, 0], 0] call psProp; logInfo format ["PROMO barrier %1 dir %2 at %3", typeOf bT, getDir bT, getPos bT]')
    for tag, (x, y) in list(pts.items()) + [('bar', [14300, 11140])]:
        h = 14 if tag == 'bar' else 60
        t.c('PC_RUN = false; PC_CAM camSetPos [%g, %g - 0.5, %d]; PC_CAM camSetDir 0; PC_CAM camSetDive -89; PC_CAM camSetFov 0.7; PC_CAM camCommit 0' % (x, y, h))
        settle(t)
        t.a('triScreenshot "top_%s"' % tag)
    footer(t)
    write(t)


TAKES = dict(land=land, occ1=occ1, occ2=occ2, guer1=guer1, guer2=guer2, night=night, amb_g1=amb_g1, amb_g2=amb_g2, amb_g3=amb_g3, amb_g4=amb_g4, amb_g5=amb_g5, amb_g6=amb_g6, amb_n1=amb_n1, amb_n2=amb_n2, amb_n3=amb_n3, fp=fp,
             kaiju_escort=kaiju_escort, kaiju_beam=kaiju_beam, kaiju_heli=kaiju_heli, dollyzoom=dollyzoom,
             probe1=probe1, walkprobe=walkprobe, walkprobe2=walkprobe2, walkprobe3=walkprobe3, jdiag=jdiag, nightprobe=nightprobe,
             night_ghajar=night_ghajar, night_fight=night_fight, night_ambush=night_ambush, kaiju_night=kaiju_night, rbprobe=rbprobe)

if __name__ == '__main__':
    import promolib
    args = sys.argv[1:]
    if '--only' in args:
        i = args.index('--only'); promolib.ONLY = set(args[i + 1].split(',')); del args[i:i + 2]
    if '--tag' in args:
        i = args.index('--tag'); promolib.SUFFIX = '_' + args[i + 1]; del args[i:i + 2]
    names = args or ['all']
    if names == ['all']:
        names = [n for n in TAKES if 'probe' not in n and n != 'jdiag']
    for n in names:
        TAKES[n]()
