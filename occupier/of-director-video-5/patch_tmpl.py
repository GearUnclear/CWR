"""One-off: derive the video 5 scenario.tmpl from the video 4 template."""
import os

here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, "..", "of-director-video-4", "scenario.tmpl"), encoding="utf-8").read()


def rep(a, b):
    global src
    c = src.count(a)
    assert c == 1, (c, a[:80])
    src = src.replace(a, b)


rep("// Scratch scenario for the fourth director video (not a regression test).",
    "// Scratch scenario for the fifth director video (not a regression test).")
rep("""// gmCampaignEnd, band edges seen by the sampler, gmTrafficDiag skippedBudget].
//""", """// gmCampaignEnd, band edges seen by the sampler, gmTrafficDiag skippedBudget].
// UDFINE rows every 2 s of game time while a born record has a hull or an
// escort: [time, op, town, phase, hull pos, escort pos, leader pos, seated
// infantry, living infantry, package].
//
// Video 5 (round 3.5, 1ae2dc9 / 06414a4 / 0c43877) adds: Arudy stays held by
// three and must repel two operations, then be re-armed and attacked a third
// time; a column arrives live at defended Le Port (born in a hull, the escort
// at the edge); the scenario's own spent squads leave with deleteGroup, the
// Houdan, Dourdan and Chapoi cells with deleteVehicle only and their handles
// dropped (emptyGroups rises, then the zone tick releases them); at least 45
// minutes of free running at the end, longer until Arudy's third operation.
//""")
rep("// control, the HQ assault, then 45 minutes of free running.",
    "// control, the HQ assault, then at least 45 minutes of free running.")

rep("""{{deleteVehicle _x} forEach (units (udXe select 1)); (format ["the cell in %1 moves on: the point is left to its own", udXe select 0]) call udNote}""",
    """{{deleteVehicle _x} forEach (units (udXe select 1)); deleteGroup (udXe select 1); (format ["the cell in %1 moves on: the point is left to its own (deleteGroup)", udXe select 0]) call udNote}""")
rep("""udTick = {call udAcc; call udBandSample; call udXClean; if (time >= udLast + udEvery) then {udLast = time; call udLogNow}}""",
    """udTick = {call udAcc; call udBandSample; call udXClean; call udArTrack; call udFine; if (time >= udLast + udEvery) then {udLast = time; call udLogNow}}
udArOn = false
udArSeen = []
udArLive = []
udArRep = 0
udArTrack = {if (udArOn) then {udArNow = []; {if ((_x select 5) == "Arudy") then {udArNow = udArNow + [_x select 0]}} forEach gmOccupierOps; {if (not (_x in udArNow)) then {if (((gmZone udAr) select 2) == gmResistanceSide) then {udArRep = udArRep + 1}; (format ["Arudy: op %1 is over, the town is held by %2; operations on Arudy repelled so far %3", _x, (gmZone udAr) select 2, udArRep]) call udNote}} forEach udArLive; {if (not (_x in udArSeen)) then {udArSeen = udArSeen + [_x]; (format ["Arudy: op %1 launched on it, operation number %2 on Arudy; re-armed zones now %3", _x, count udArSeen, (gmOccupierForces) select 30]) call udNote}} forEach udArNow; udArLive = udArNow}}
udFineLast = -1000
udFine = {if (time >= udFineLast + 2) then {udFineLast = time; {if ((not (isNull (_x select 18))) or (not (isNull (_x select 23)))) then {logInfo format ["UDFINE %1", [time, _x select 10, _x select 0, _x select 2, getPos (_x select 18), getPos (_x select 23), getPos (_x select 20), _x select 19, _x select 21, _x select 13]]}} forEach gmCounterAttacks}}""")

rep('''"lever: companions off (GM_COMP_ALIVE), the legacy script QRF parked (GM_QRF_TICK), the player invulnerable and kept in the hills, time x8 (x4 while a force is live or airborne)" call udNote''',
    '''"lever: companions off (GM_COMP_ALIVE), the legacy script QRF parked (GM_QRF_TICK), the player invulnerable and kept in the hills, time x8 (x4 while a force is live or airborne)" call udNote
(format ["round 3.5 surface: re-armed zones (gmOccupierForces slot 30) %1, gmSimDiag empty/released %2", (gmOccupierForces) select 30, gmSimDiag select 11]) call udNote''')

rep('''{deleteVehicle _x} forEach (units udSqV)
"the Houdan cell moves on: the town is left empty" call udNote''',
    '''{deleteVehicle _x} forEach (units udSqV)
udSqV = grpNull
GM_tmpG = grpNull
(format ["the Houdan cell moves on: the town is left empty; its two fighters deleteVehicle'd, the group NOT deleted and the script's handles dropped; gmSimDiag empty/released %1", gmSimDiag select 11]) call udNote''')

rep('''{deleteVehicle _x} forEach (units udSqO)
if (not (isNull udHg)) then {{deleteVehicle _x} forEach (units udHg)}
(format ["lever: the Outpost hold squad (%1) is deleted, GM_HOLD_GRPS select (GM_HOLD_ZONES find Outpost), a lost patrol; the cell moves on: the base is empty", udHn]) call udNote''',
    '''{deleteVehicle _x} forEach (units udSqO)
deleteGroup udSqO
udSqO = grpNull
if (not (isNull udHg)) then {{deleteVehicle _x} forEach (units udHg)}
udHg = grpNull
(format ["lever: the Outpost hold squad (%1) is deleted, GM_HOLD_GRPS select (GM_HOLD_ZONES find Outpost), a lost patrol (its units only: capture.sqs keeps the group in GM_HOLD_GRPS); the cell moves on with deleteGroup: the base is empty", udHn]) call udNote''')

rep('''{deleteVehicle _x} forEach ((units udSqD) + (units udSqC))
"the Dourdan and Chapoi cells move on: both towns are left empty" call udNote''',
    '''{deleteVehicle _x} forEach ((units udSqD) + (units udSqC))
udSqD = grpNull
udSqC = grpNull
GM_tmpG = grpNull
(format ["the Dourdan and Chapoi cells move on: both towns are left empty; deleteVehicle only, both groups left registered and the script's handles dropped; gmSimDiag empty/released %1", gmSimDiag select 11]) call udNote''')

rep('''udBeat = "arudy"
@WAIT 600 Arudy stored | | not (gmGarrisonSpawned udAr)''', '''udBeat = "arudy"
@WAIT 600 Arudy stored | | not (gmGarrisonSpawned udAr)
udArOn = true''')
rep('''(format ["after Arudy: owner %1, pool %2 of %3, band %4", (gmZone udAr) select 2, (gmOccupierForces) select 0, (gmOccupierForces) select 1, (gmOccupierForces) select 2]) call udNote''',
    '''(format ["after Arudy's first operation: owner %1, pool %2 of %3, band %4, rests %5; the three stay for the rest of the run", (gmZone udAr) select 2, (gmOccupierForces) select 0, (gmOccupierForces) select 1, (gmOccupierForces) select 2, (gmOccupierForces) select 12]) call udNote''')

rep('''{deleteVehicle _x} forEach (units udSqF)
if (not (isNull udHg)) then {{deleteVehicle _x} forEach (units udHg)}''', '''{deleteVehicle _x} forEach (units udSqF)
deleteGroup udSqF
udSqF = grpNull
if (not (isNull udHg)) then {{deleteVehicle _x} forEach (units udHg)}
udHg = grpNull''')

rep('''"lever: FALLBACK, no natural strike in 1800 s: gmForcesForce strike on Le Port" call udNote''',
    '''"lever: FALLBACK, no director-bought strike in 1800 s: gmForcesForce strike on Le Port (the test lever, exempt from the band rule)" call udNote''')

rep('''@WAIT 2700 free running | | false
call udLogNow''', '''@WAIT 2700 free running | | false
call udLogNow
(format ["45 free minutes done: operations on Arudy %1 (repelled %2), re-armed %3, pool %4 of %5, band %6, tokens %7", udArSeen, udArRep, (gmOccupierForces) select 30, (gmOccupierForces) select 0, (gmOccupierForces) select 1, (gmOccupierForces) select 2, (gmOccupierForces) select 3]) call udNote
@WAIT 2400 free running until Arudy's third operation has gone | | ((count udArSeen) >= 3) and (udArRep >= 2)
@WAIT 300 free running a last five minutes | | false
call udLogNow''')
rep('''(format ["free running from here: no levers for 45 minutes of game time;''',
    '''(format ["free running from here: no levers for at least 45 minutes of game time;''')
rep('''(format ["end: pool %1 of %2, band %3, tokens %4, operation counter %5, waves fired %6, control %7", (gmOccupierForces) select 0, (gmOccupierForces) select 1, (gmOccupierForces) select 2, (gmOccupierForces) select 3, (gmOccupierForces) select 13, (gmOccupierForces) select 18, (gmOccupierForces) select 5]) call udNote''',
    '''(format ["end: pool %1 of %2, band %3, tokens %4, operation counter %5, waves fired %6, control %7, operations on Arudy %8 (repelled %9)", (gmOccupierForces) select 0, (gmOccupierForces) select 1, (gmOccupierForces) select 2, (gmOccupierForces) select 3, (gmOccupierForces) select 13, (gmOccupierForces) select 18, (gmOccupierForces) select 5, udArSeen, udArRep]) call udNote
(format ["end census: gmSimDiag %1", gmSimDiag]) call udNote''')

if chr(0x2014) in src:
    raise SystemExit("em dash")
open(os.path.join(here, "scenario.tmpl"), "w", encoding="utf-8", newline="\n").write(src)
print("ok", len(src.splitlines()))
