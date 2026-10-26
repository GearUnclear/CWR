// Scratch scenario for the director video (not a regression test).  A
// fictional campaign on the shipped Guerrilla.Abel template, played at high
// speed with the real occupier forces director.  The scenario script plays the
// RESISTANCE side only (who stands where, which zone a cell takes); every
// occupier decision is the engine's own.  Levers used, all logged as UDNOTE:
// directorInterval 10 s, waveLead 120 s, birthGiveUp 60 s, setAccTime, three
// garrison zeroings that stand in for raids, one heat raise that stands in for
// a loud ambush, and invulnerable resistance fighters.  Every udTick writes a
// UDTRACE line (sim time, gmOccupierForces, gmOccupierOps, gmOccupierIntel,
// gmCounterAttacks, every zone, gmOccupierAir, the player, gmHqState).

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
udT0 = time
triSimUntil { time > udT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

udEvery = 10
udLast = -1000
udBeat = "setup"
udRoutes = []
udAuto = false
udLogNow = {udZ = []; udI = 0; while {udI < gmZoneCount} do {udR = gmZone udI; udS = gmZoneSim udI; udZ = udZ + [[udI, udR select 2, udR select 3, udR select 4, udR select 6, udS select 14, udS select 13, udS select 9, udS select 1, udS select 2, udS select 16, udS select 0]]; udI = udI + 1}; udC = []; {udC = udC + [[_x select 10, _x select 0, _x select 1, _x select 2, _x select 3, _x select 4, _x select 15, getPos (_x select 20), getPos (_x select 18), getPos (_x select 23), _x select 21, _x select 9, _x select 11, _x select 13]]} forEach gmCounterAttacks; udA = []; {udA = udA + [[_x select 8, _x select 9, _x select 10, _x select 7, _x select 11, _x select 2, _x select 4]]} forEach gmOccupierAir; logInfo format ["UDTRACE %1", [time, udBeat, gmOccupierForces, gmOccupierOps, gmOccupierIntel, udC, udZ, udA, getPos player, gmHqState, accTime]]; {udK = format ["%1>%2", _x select 4, _x select 5]; if (not (udK in udRoutes)) then {udRoutes = udRoutes + [udK]; udSi = gmZoneIndex (_x select 4); udTi = gmZoneIndex (_x select 5); if ((udSi >= 0) and (udTi >= 0)) then {logInfo format ["UDROUTE %1", [_x select 4, _x select 5, gmRoadRoute [(gmZone udSi) select 8, (gmZone udTi) select 8]]]}}} forEach gmOccupierOps}
udAcc = {if (udAuto) then {udLv = false; {if ((_x select 6) == "LIVE") then {udLv = true}} forEach gmOccupierOps; {if ((_x select 2) == 1) then {udLv = true}} forEach gmOccupierAir; if (udLv) then {if (accTime != 4) then {setAccTime 4}} else {if (accTime != 8) then {setAccTime 8}}}}
udTick = {call udAcc; if (time >= udLast + udEvery) then {udLast = time; call udLogNow}}
udNote = {logInfo format ["UDNOTE %1", [time, _this]]}

// ---- geometry: every zone, and the road routes from every occupier base -----
udI = 0
while {udI < gmZoneCount} do {udR = gmZone udI; logInfo format ["UDZONE %1", [udI, udR select 0, udR select 1, udR select 2, udR select 3, udR select 8, (gmZoneSim udI) select 7, gmLandmass (udR select 8)]]; udI = udI + 1}
udBases = ["Outpost", "Southeast Compound", "Le Port Harbour", "Airfield"]
{udSi = gmZoneIndex _x; udJ = 0; while {udJ < gmZoneCount} do {if (udJ != udSi) then {logInfo format ["UDGRAPH %1", [_x, (gmZone udJ) select 0, gmRoadRoute [(gmZone udSi) select 8, (gmZone udJ) select 8]]]}; udJ = udJ + 1}} forEach udBases
logInfo format ["UDHQ %1", [gmHqState, gmHqPos]]

// ---- compression levers (test pacing only) ----------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
gmForcesTune ["directorInterval", 10]
gmForcesTune ["waveLead", 120]
gmCounterAttackTune ["birthGiveUp", 60]
"lever: directorInterval 10 s, waveLead 120 s, birthGiveUp 60 s" call udNote
call udLogNow

udRes = gmResistanceSide call GM_fnSideFromString
udV = gmZoneIndex "Village"
udL = gmZoneIndex "Le Port"
udO = gmZoneIndex "Outpost"
udAf = gmZoneIndex "Airfield"
udVPos = (gmZone udV) select 8
udLPos = (gmZone udL) select 8
udOPos = (gmZone udO) select 8
udAfPos = (gmZone udAf) select 8

// the player is away in the western hills, far from every zone and road we use
udFar = [4500, 5200, 0]
if ((gmLandmass udFar) < 0) then {udFar = [7922, 8500, 0]}
player setPos udFar
triSimUntil { call udTick; (not ((gmZoneSim udV) select 0)) and (not (gmGarrisonSpawned udV)) }
triSimUntil { call udTick; (not ((gmZoneSim udL) select 0)) and (not (gmGarrisonSpawned udL)) }
triSimUntil { call udTick; (not ((gmZoneSim udO) select 0)) and (not (gmGarrisonSpawned udO)) }
setAccTime 4

// ---- beat 1: Houdan falls to a cell, the director stays DORMANT ------------
udBeat = "town1"
"lever: Houdan garrison 0, support 55 (the cell's earlier work)" call udNote
gmZoneSet [udV, "garrison", 0]
gmZoneSet [udV, "support", 55]
udSqV = [udRes, gmFactionSquad [gmResistanceSide, 1, 2], udVPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units udSqV)
doStop (units udSqV)
udT1 = time
triSimUntil { call udTick; (((gmZone udV) select 2) == gmResistanceSide) or (time > udT1 + 600) }
call udLogNow
{deleteVehicle _x} forEach (units udSqV)
"the Houdan cell moves on: the town is left empty" call udNote
udT1 = time
triSimUntil { call udTick; time > udT1 + 120 }

// ---- beat 2: Le Port falls, four fighters stay: the director wakes ---------
udBeat = "town2"
"lever: Le Port garrison 0, support 55; four fighters, invulnerable" call udNote
gmZoneSet [udL, "garrison", 0]
gmZoneSet [udL, "support", 55]
udSqL = [udRes, gmFactionSquad [gmResistanceSide, 1, 4], udLPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units udSqL)
doStop (units udSqL)
udT2 = time
triSimUntil { call udTick; (((gmZone udL) select 2) == gmResistanceSide) or (time > udT2 + 600) }
call udLogNow
udT2 = time
triSimUntil { call udTick; (((gmOccupierForces) select 4) == "AWAKE") or (time > udT2 + 300) }
call udLogNow

// ---- beat 3: the first answers play out (Houdan empty, Le Port defended) ---
udBeat = "answers"
udAuto = true
udT3 = time
triSimUntil { call udTick; (((gmZone udV) select 2) != gmResistanceSide) or (time > udT3 + 1200) }
udT3 = time
triSimUntil { call udTick; ((count gmOccupierOps) == 0) or (time > udT3 + 1500) }
call udLogNow

// ---- beat 4: a cell takes the Outpost and leaves it empty -------------------
udBeat = "outpost"
"lever: Outpost reserve 0 (an ambush on its relief)" call udNote
gmZoneSet [udO, "garrison", 0]
udSqO = [udRes, gmFactionSquad [gmResistanceSide, 1, 3], udOPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units udSqO)
doStop (units udSqO)
udT4 = time
triSimUntil { call udTick; (((gmZone udO) select 2) == gmResistanceSide) or (time > udT4 + 900) }
call udLogNow
{deleteVehicle _x} forEach (units udSqO)
"the Outpost cell moves on: the base is left empty" call udNote

// ---- beat 5: the next token; the base is retaken ----------------------------
udBeat = "base"
udT5 = time
triSimUntil { call udTick; (((gmZone udO) select 2) != gmResistanceSide) or (time > udT5 + 3000) }
call udLogNow
udT5 = time
triSimUntil { call udTick; time > udT5 + 60 }

// ---- beat 6: a loud ambush near Le Port; the director buys for 4 fighters ---
udBeat = "column"
triSimUntil { call udTick; not (gmGarrisonSpawned udAf) }
"lever: Airfield reserve 2 (a raid on its barracks)" call udNote
gmZoneSet [udAf, "garrison", 2]
"lever: Le Port heat +45 (stands in for a loud ambush on the coast road)" call udNote
gmZoneSet [udL, "heatRaise", 45]
udOpL = []
udT6 = time
triSimUntil { call udTick; udOpL = []; {if (((_x select 5) == "Le Port") and ((_x select 0) > 1)) then {udOpL = _x}} forEach gmOccupierOps; ((count udOpL) > 0) or (time > udT6 + 3000) }
call udLogNow

// the player scouts the column's base: 200 m down its road, facing the gate
udBeat = "watch"
udAuto = false
setAccTime 4
udSrc = "Airfield"
if ((count udOpL) > 0) then {udSrc = udOpL select 4}
udSPos = (gmZone (gmZoneIndex udSrc)) select 8
udRt = gmRoadRoute [udSPos, udLPos]
udPts = []
if ((count udRt) > 0) then {udPts = udRt select ((count udRt) - 1)}
udCum = 0
udVan = [(udSPos select 0) + (((udLPos select 0) - (udSPos select 0)) * 200 / (udSPos distance udLPos)), (udSPos select 1) + (((udLPos select 1) - (udSPos select 1)) * 200 / (udSPos distance udLPos)), 0]
udK = 1
if ((count udPts) > 1) then {udVan = [((udPts select 0) select 0), ((udPts select 0) select 1), 0]; while {(udK < (count udPts)) and (udCum < 200)} do {udP0 = udPts select (udK - 1); udP1 = udPts select udK; udCum = udCum + ([(udP0 select 0), (udP0 select 1), 0] distance [(udP1 select 0), (udP1 select 1), 0]); udVan = [(udP1 select 0), (udP1 select 1), 0]; udK = udK + 1}}
player setCaptive true
player setPos udVan
udLook = ((udSPos select 0) - (udVan select 0)) atan2 ((udSPos select 1) - (udVan select 1))
player setDir udLook
(format ["the player watches the road out of %1 from %2 m", udSrc, udVan distance udSPos]) call udNote
udId = -1
if ((count udOpL) > 0) then {udId = udOpL select 0}
udRow = []
udT7 = time
triSimUntil { call udTick; player setDir udLook; udRow = []; {if ((_x select 0) == udId) then {udRow = _x}} forEach gmOccupierOps; ((count udRow) == 0) or (((count udRow) > 0) and (((udRow select 4) != udSrc) or ((udRow select 6) == "LIVE") or ((udRow select 3) == "TRANSPORT_HELI"))) or (time > udT7 + 900) }
call udLogNow
udT7 = time
triSimUntil { call udTick; player setDir udLook; time > udT7 + 20 }
player setPos udFar
player setCaptive false
"the player leaves for the western hills" call udNote

// ---- beat 7: the re-planned force reaches Le Port ---------------------------
udBeat = "insertion"
udAuto = true
udT8 = time
triSimUntil { call udTick; udRow = []; {if ((_x select 0) == udId) then {udRow = _x}} forEach gmOccupierOps; ((count udRow) == 0) or (time > udT8 + 2400) }
call udLogNow
udT8 = time
triSimUntil { call udTick; time > udT8 + 60 }

// ---- beat 8: a raid takes the Airfield; control falls; wave 1 --------------
udBeat = "airfield"
"lever: Airfield reserve 0 (a raid); three fighters hold it, invulnerable" call udNote
triSimUntil { call udTick; not (gmGarrisonSpawned udAf) }
gmZoneSet [udAf, "garrison", 0]
udSqA = [udRes, gmFactionSquad [gmResistanceSide, 1, 3], udAfPos] call GM_fnSpawnSquad
{_x allowDammage false} forEach (units udSqA)
doStop (units udSqA)
udT9 = time
triSimUntil { call udTick; (((gmZone udAf) select 2) == gmResistanceSide) or (time > udT9 + 900) }
call udLogNow
udBeat = "wave"
udT9 = time
triSimUntil { call udTick; (((gmOccupierForces) select 18) >= 1) or (time > udT9 + 600) }
call udLogNow
udT9 = time
triSimUntil { call udTick; (((gmOccupierForces) select 22) >= 1) or (time > udT9 + 600) }
call udLogNow

// ---- beat 9: the wave's strikes play out ------------------------------------
udBeat = "strikes"
udT10 = time
triSimUntil { call udTick; (time > udT10 + 120) and (((count gmOccupierOps) == 0) or (gmHqState select 1) or (time > udT10 + 4000)) }
call udLogNow
udT10 = time
triSimUntil { call udTick; time > udT10 + 60 }
udAuto = false
setAccTime 1
call udLogNow
gmSimDiagLog

triEndTest
