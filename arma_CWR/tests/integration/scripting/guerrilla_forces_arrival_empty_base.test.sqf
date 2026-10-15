// ============================================================================
//  Occupier forces, the arrival census on a held base (OCCUPIER-FORCES.md
//  7.2 / D19 the base recapture, contract row 2c: an arrival goes live only
//  against live defenders at the target).  The director-video run of
//  2026-10-14 logged "defenders present" for a push-out on the Outpost after
//  the scenario deleted the cell that took it: capture.sqs had left its live
//  3-man hold squad (GM_HOLD_GRPS) there, and the census counted it, rightly.
//  This test empties the base for real and checks the abstract path.  Boots
//  the SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full
//  Remaster:
//    r1  a real camera at the default view distance in daylight; the pool
//        STRONG, the director DORMANT; test levers: director tick 1 s, the
//        first-town roll 0, one distinct town wakes the director;
//    r2  the player walks to the western hills (far from every zone and road
//        used here; the HQ stays about 500 m from the Outpost); the Outpost
//        loses its stored garrison and an invulnerable 3-man cell takes it
//        through the real capture meter;
//    r3  capture.sqs leaves its hold squad (GM_HOLD_ZONES / GM_HOLD_GRPS):
//        with the cell deleted the census (gmZoneSim slot 1) still counts the
//        hold squad; then the hold squad is deleted too and the census reads
//        0 although the HQ stands about 500 m away;
//    r4  the Outpost is made hot (heat 100) and Houdan rises remotely: the
//        director wakes and sends a BASE operation on the Outpost (abstract
//        leg at 100 m/s, gmCounterAttackTune "speed");
//    r5  the operation arrives ABSTRACT: it reaches phase HOLDING, never
//        LIVE, never born (birthReason ""), and the zone tick fills the
//        occupier's base meter (slot 14) on its abstract presence until the
//        owner goes back to the occupier (lostCount 1); the operation ends.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
eaT0 = time
triSimUntil { time > eaT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- r1: a real camera; the pool STRONG; the levers -------------------------
eaP = gmTrafficPercept (getPos player)
triAssertEq [(eaP select 0), 1]
triAssertGe [(eaP select 3), 750]
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [((gmOccupierForces) select 2), "STRONG"]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]
triAssertEq [(format ["%1", gmForcesTune ["directorInterval", 1]]), "true"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 1]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
eaRes = gmResistanceSide call GM_fnSideFromString

// ---- r2: the player leaves; a cell takes the Outpost --------------------------
eaO = gmZoneIndex "Outpost"
triAssertGe [eaO, 0]
eaOPos = (gmZone eaO) select 8
// the HQ stands about 500 m from the Outpost on the shipped template
triAssertEq [(format ["%1", (gmHqState select 0)]), "true"]
eaHqD = gmHqPos distance eaOPos
triAssertEq [(format ["%1", (eaHqD > 300) and (eaHqD < 700)]), "true"]
eaFar = [4500, 5200, 0]
if ((gmLandmass eaFar) < 0) then {eaFar = [7922, 8500, 0]}
player setPos eaFar
triSimUntil { (not ((gmZoneSim eaO) select 0)) and (not (gmGarrisonSpawned eaO)) }
gmZoneSet [eaO, "garrison", 0]
eaCell = [eaRes, gmFactionSquad [gmResistanceSide, 1, 3], eaOPos] call GM_fnSpawnSquad
eaCell setCombatMode "BLUE"
{_x allowDammage false} forEach (units eaCell)
doStop (units eaCell)
eaT1 = time
triSimUntil { (((gmZone eaO) select 2) == gmResistanceSide) or (time > eaT1 + 300) }
triAssertEq [((gmZone eaO) select 2), gmResistanceSide]

// ---- r3: the hold squad capture.sqs left counts; then it is gone too ----------
eaAt = -1
triSimUntil { eaAt = GM_HOLD_ZONES find "Outpost"; eaAt >= 0 }
eaHold = GM_HOLD_GRPS select eaAt
triSimUntil { ({alive _x} count (units eaHold)) > 0 }
{deleteVehicle _x} forEach (units eaCell)
eaT2 = time
triSimUntil { time > eaT2 + 8 }
// the cell is gone, the hold squad is still in the census (the video-run case)
triAssertGe [((gmZoneSim eaO) select 1), 1]
{deleteVehicle _x} forEach (units eaHold)
triSimUntil { ((gmZoneSim eaO) select 1) == 0 }
eaT3 = time
triSimUntil { time > eaT3 + 8 }
triAssertEq [((gmZoneSim eaO) select 1), 0]
triAssertEq [((gmZone eaO) select 2), gmResistanceSide]
triAssertEq [((gmZoneSim eaO) select 14), 0]
triAssertEq [((gmOccupierForces) select 4), "DORMANT"]

// ---- r4: the Outpost hot; Houdan rises remotely: a BASE operation -------------
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
gmZoneSet [eaO, "heatRaise", 100]
eaV = gmZoneIndex "Village"
triAssertGe [eaV, 0]
eaVPos = (gmZone eaV) select 8
triSimUntil { not (gmGarrisonSpawned eaV) }
gmZoneSet [eaV, "garrison", 0]
gmZoneSet [eaV, "support", 55]
eaSq = [eaRes, gmFactionSquad [gmResistanceSide, 1, 1], eaVPos] call GM_fnSpawnSquad
eaSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units eaSq)
doStop (units eaSq)
triSimUntil { ((gmZone eaV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units eaSq)
triSimUntil { ((gmOccupierForces) select 4) == "AWAKE" }
eaOp = []
triSimUntil { eaOp = []; {if (((_x select 1) == "BASE") and ((_x select 5) == "Outpost")) then {eaOp = _x}} forEach gmOccupierOps; (count eaOp) > 0 }
triAssertEq [(format ["%1", (eaOp select 4) != ""]), "true"]
triAssertGe [(eaOp select 7), 4]
eaId = eaOp select 0

// ---- r5: an abstract arrival; occCapture retakes the base ---------------------
eaLive = false
eaBorn = false
eaHeld = false
eaMeter = false
eaT4 = time
triSimUntil { eaOp = []; {if ((_x select 0) == eaId) then {eaOp = _x}} forEach gmOccupierOps; if ((count eaOp) > 0) then {if ((eaOp select 6) == "LIVE") then {eaLive = true}; if ((eaOp select 10) != "") then {eaBorn = true}; if ((eaOp select 6) == "HOLDING") then {eaHeld = true}}; if ((((gmZone eaO) select 2) == gmResistanceSide) and (((gmZoneSim eaO) select 14) > 0)) then {eaMeter = true}; (((gmZone eaO) select 2) == gmOccupierSide) or (time > eaT4 + 400) }
triAssertEq [((gmZone eaO) select 2), gmOccupierSide]
triAssertEq [(format ["%1", eaLive]), "false"]
triAssertEq [(format ["%1", eaBorn]), "false"]
triAssertEq [(format ["%1", eaHeld]), "true"]
triAssertEq [(format ["%1", eaMeter]), "true"]
triAssertEq [((gmZoneSim eaO) select 15), 1]
triSimUntil { eaOp = []; {if ((_x select 0) == eaId) then {eaOp = _x}} forEach gmOccupierOps; (count eaOp) == 0 }
gmSimDiagLog

triEndTest
