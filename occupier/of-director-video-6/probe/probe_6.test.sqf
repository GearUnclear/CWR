triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
gmForcesTune ["log", 1]
udI = 0
while {udI < gmZoneCount} do {udR = gmZone udI; logInfo format ["UDZONE %1", [udI, udR select 0, udR select 1, udR select 2, udR select 3, udR select 8, (gmZoneSim udI) select 7, gmLandmass (udR select 8)]]; udI = udI + 1}
logInfo format ["UDPROBE gen %1", gmGeneratedBases]
logInfo format ["UDPROBE forces %1", gmOccupierForces]
logInfo format ["UDPROBE ledger %1", gmOccupierLedger]
logInfo format ["UDPROBE repels %1 massing %2 ends %3", gmOccupierRepels, gmOccupierMassing, gmCounterAttackEnds]
logInfo format ["UDPROBE hq %1 %2", gmHqState, gmHqPos]
udRes = gmResistanceSide call GM_fnSideFromString
udSq = [udRes, gmFactionSquad [gmResistanceSide, 1, 6], [4400, 7600, 0]] call GM_fnSpawnSquad
logInfo format ["UDPROBE squad %1 units %2 classes %3", udSq, count (units udSq), gmFactionSquad [gmResistanceSide, 1, 6]]
{logInfo format ["UDPROBE unit %1 %2 weapons %3 mags %4", _x, typeOf _x, weapons _x, magazines _x]} forEach (units udSq)
udU = (units udSq) select 0
udM0 = magazines udU
udU removeMagazine (udM0 select 0)
logInfo format ["UDPROBE after remove %1", magazines udU]
udU addMagazine (udM0 select 0)
logInfo format ["UDPROBE after add %1", magazines udU]
{deleteVehicle _x} forEach (units udSq)
deleteGroup udSq
logInfo format ["UDPROBE tune %1", gmForcesTune ["log", 1]]
triEndTest
